// bladerf.c v1 Sandra Berndt
// spectrum analyzer server;

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <termios.h>
#include <regex.h>
#include <assert.h>
#include <math.h>

#include <fftw3.h>
#include "bladerf_priv.h"
#include "dc_cal_table.h"
#include "metadata.h"

// config;

#define PORT 0x4252
#define SZ_RXB 1024
#define SZ_TXB (SZ_RXB + 256)

#define SDR_ASYNC
#define SDR_RETRY
#define SDR_BPS (2 * sizeof(short))
#define SDR_SPBUF (1024)	// samples per buffer;
#define SDR_NXFER 16		// number of xfers;
#define SDR_FUDGE 16		// # of ignored buffers after starting;

#define SDR_IQ_MIN -2048
#define SDR_IQ_MAX 2047
#define SDR_IQ_RND 0.5

#define SDR_REP_OK NULL
#define SDR_REP_DATA ((void *)1)

// types;

typedef struct tmr tmr_t;
typedef struct sstats sstats_t;
typedef struct sparam sparam_t;
typedef struct cal_entry cal_entry_t;
typedef struct cal cal_t;
typedef struct spwr spwr_t;
typedef struct g g_t;

// generic timer data;
    
struct tmr {
	int fd;			// timer fd;
	struct itimerspec ts; 
	void (*hdl)(void *);	// timeout handler;
	void *priv;		// private data;
}; 

// sampel stats;

struct sstats {
	int min[2];		// IQ min;
	int max[2];		// IQ max;
	double avg[2];		// IQ averages;
};

// sample params;

struct sparam {
	void *sbuf;		// sample buffer;
	int64_t freq;		// frequency;
	unsigned srate[2];	// sample rate, want/actual;
	unsigned bwidth[2];	// filter bandwidth, want/actual;
	unsigned ns[2];		// # of samples, want/got;
	int rxgain;		// rx gain;
};

// calibration data;

struct cal_entry {
	cal_entry_t *next;	// next frequency;
	int64_t freq;		// frequency;
	uint32_t lnbr;		// cal file line number;
	float rxgain;		// rx gain;
	float rxiqdco[2];	// rx i/q dc offsets;
};

struct cal {
	int sdr;		// calibrate SDR;
	char file[60];		// calibration file name;
	cal_entry_t *ce;	// frequency/gain pairs;
};

// spectrum power;

struct spwr {
	double mma[3];		// min/max/avg power;
};

// global data;

struct g {
	char *cmd;			// argv[0];
	int verbose;			// verbose level;
	unsigned delay;			// delay for sdr to get ready;
	int nerr;			// # of errors;
	int stop;			// stop and exit;
	tmr_t tmr;			// timer;
	int sock;			// socket;
	int cso;			// client socket;
	struct sockaddr_in csa;		// client sa;
	socklen_t clen;			// client sa length;
	struct bladerf *dev;		// device handle;
	struct bladerf_devinfo info;	// device info;
	cal_t cal;			// calibration data;
	unsigned srate;			// sample rate in samples/sec;
	unsigned bwidth;		// SDR bandwidth;
	unsigned nsamples;		// # of samples in buffer;
	unsigned szsb;			// size of sample buffer;
	int rxgain;			// combined rx gain;
	unsigned spmsg;			// samples per msg;
	unsigned spbuf;			// samples per buffer;
	void **sbufs;			// array of sample buffers;
	void *sbuf;			// sample buffer;
	unsigned nbufs;			// number of sample buffers;
	unsigned bidx;			// buffer index;
	uint64_t ts;			// current timestamp;
	int64_t freq;			// <0 means illegal, in Hz;
	int npe;			// # of power envelope points;
	int nspwr;			// # of spwr_t entries;
	spwr_t *spwr;			// last spectrum power;
	int nrx;			// # of chars in rxb[];
	u_char rxb[SZ_RXB];		// rx buffer;
	u_char txb[SZ_TXB];		// tx buffer;
};

g_t G;

// print error or warning;
// @ at beginning is fatal exit;

void
Msg(char *fmt, ...)
{
	FILE *fp = stdout;
	va_list args;
	int fatal, len;
	char *s, *p;

	// check fatal;

	fatal = (fmt[0] == '@');
	if(fatal) {
		fmt++;
		fp = stderr;
	}
	fprintf(fp, "%s: ", G.cmd);
	if(fatal)
		fprintf(fp, "error: ");

	// handle actual message;

	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);
	fflush(fp);

	if(fatal) {
		fprintf(fp, "%s: aborting ...\n", G.cmd);
		exit(1);
	}
}

#define Fatal(fmt...) Msg("@" fmt)

// skip over white space;

char *
skip_spc(char *s)
{
	while((*s == ' ') || (*s == '\t'))
		s++;
	return(s);
}

// print usage;

void
usage(g_t *g, FILE *out)
{
	fprintf(out, "usage: %s [options]\n", g->cmd);
	fprintf(out, "  -d   - incread debug verbosity\n");
	fprintf(out, "  -h   - print this help\n");
	exit(1);
}

// set single-shot timer;

int
timer_set(tmr_t *t, unsigned ms, void (*hdl)(void *), void *priv)
{
	int ret;

	t->hdl = hdl;
	t->priv = priv;
	t->ts.it_value.tv_sec = ms / 1000;
	t->ts.it_value.tv_nsec = (ms % 1000) * 1000000;
	ret = timerfd_settime(t->fd, 0, &t->ts, NULL);
	return(ret);
}

// cancel a timer;

void
timer_cancel(tmr_t *t)
{
	t->hdl = NULL;
	t->ts.it_value.tv_sec = 0;
	t->ts.it_value.tv_nsec = 0;
	timerfd_settime(t->fd, 0, &t->ts, NULL);
}

// handle timer;
// remove handler only for single-shot timers;

void
timer_hdl(tmr_t *t)
{
	int ret;
	unsigned long long missed;

	ret = read(t->fd, &missed, sizeof(missed));
	if(t->hdl)
		t->hdl(t->priv);
	if( !t->ts.it_interval.tv_sec && !t->ts.it_interval.tv_nsec)
		t->hdl = NULL;
}

// set timer fd for select;

static inline int
set_fd(int fd, fd_set *set, int max)
{
	FD_SET(fd, set);
	if(fd > max)
		max = fd;
	return(max);
}

// accept new connection;

void
do_accept(g_t *g)
{
	// close previous connection;

	if(g->cso >= 0)
		close(g->cso);

	// accept new connection;

	g->clen = sizeof(sizeof(g->csa));
	g->cso = accept(g->sock, (struct sockaddr *)&g->csa, &g->clen);
	if(g->cso < 0) {
                Msg("accept failed: %s\n", strerror(errno));
		return;
	}
	Msg("connection from %s\n", inet_ntoa(g->csa.sin_addr));
}

// read commands;
// returns new # of bytes in rxb[];

int
do_read(g_t *g)
{
	int nb, left;
	u_char *buf;

	left = sizeof(g->rxb) - g->nrx;
	if(left <= 0)
		return(0);

	// read as much as we can;

	buf = g->rxb;
	nb = read(g->cso, buf + g->nrx, left);
	if(nb < 0) {
		g->nrx = 0;	// socket closed;
		return(0);
	}
	if(nb == 0)
		return(0);
	//Msg("#r %d\n", nb);
	nb += g->nrx;
	g->nrx = nb;
	return(nb);
}

// compute sample stats;

void
stats_compute(sstats_t *stats, short *sp, unsigned ns)
{
	int n;
	short i, q, imin, imax, qmin, qmax;
	int64_t iavg, qavg;

	imin = qmin = 0x7fff;
	imax = qmax = 0x8000;
	iavg = qavg = 0;
	for(n = 0; n < ns; n++) {
		i = *sp++;
		q = *sp++;
		if(i < imin) imin = i;
		if(i > imax) imax = i;
		if(q < qmin) qmin = q;
		if(q > qmax) qmax = q;
		iavg += i;
		qavg += q;
	}
	stats->min[0] = imin;
	stats->min[1] = qmin;
	stats->max[0] = imax;
	stats->max[1] = qmax;
	stats->avg[0] = (double)iavg / (double)ns;
	stats->avg[1] = (double)qavg / (double)ns;
}

// print sample stats;

void
stats_print(sstats_t *stats)
{
	Msg("stats: i %d/%d q %d/%d avg %f/%f\n",
		stats->min[0], stats->max[0],
		stats->min[1], stats->max[1],
		stats->avg[0], stats->avg[1]);
}

// allocate sample buffer;

void *
sdr_sbuf(g_t *g, unsigned ns)
{
	unsigned szsb;

	szsb = ns * SDR_BPS;
	if(szsb > g->szsb) {
		g->szsb = szsb;
		if(g->sbuf)
			free(g->sbuf);
		g->sbuf = malloc(g->szsb);
	}
	return(g->sbuf);
}

// print sample params;

void
sparam_print(sparam_t *sp)
{
	Msg("sample: f=%ld rxgain=%d sr=%u/%u bw=%u/%u ns=%u\n",
		sp->freq, sp->rxgain, sp->srate[0], sp->srate[1],
		sp->bwidth[0], sp->bwidth[1], sp->ns[0]);
}

// sdr async rx buffer callback;

int ncb;

void *
sdr_rx_async_cb(struct bladerf *dev, struct bladerf_stream *stream, struct bladerf_metadata *meta, void *samples, size_t ns, void *priv)
{
	g_t *g = priv;
	uint64_t ts, tsexp;
	unsigned bidx;

	// set initial timestamp;
	// shutdown stream if buffer timestamp out of sync;
	// there are a number of [meta,sample] messages in the buffer;

	bidx = g->bidx;
	ts = metadata_get_timestamp(samples);
	if(bidx >= 16) {
		tsexp = g->ts + g->spbuf;
		if(ts != tsexp)
			return(BLADERF_STREAM_SHUTDOWN);
	}
	g->ts = ts;

	// shutdown if buffers out of order;

	if(samples != g->sbufs[bidx])
		return(BLADERF_STREAM_SHUTDOWN);

	// done if all buffers full;

	g->bidx = ++bidx;
	if(bidx >= g->nbufs)
		return(BLADERF_STREAM_SHUTDOWN);

	// give it next buffer, if more to fill;
	// pipe is SDR_NXFER ahead;

	bidx += (SDR_NXFER - 1);
	if(bidx >= g->nbufs)
		return(BLADERF_STREAM_NO_DATA);
	return(g->sbufs[bidx]);
}

// sdr async rx;

char *
sdr_rx_async(g_t *g, sparam_t *sp)
{
	struct bladerf *dev = g->dev;
	int ret;
	char *err;
	struct bladerf_stream *stream;
	uint64_t ts, tsexp;
	unsigned bidx, nb;
	int n, len;
	uint8_t *sd, *ss;

	// round bufs to multiple of sample buffer size;
	// it seems, occasionally, the bladerf messes up the
	// timestamps of the first 16 usb packets, so stream them,
	// but irgnore them for samples;

	g->spmsg = (dev->msg_size - METADATA_HEADER_SIZE) / SDR_BPS;
	g->spbuf = (SDR_SPBUF * (dev->msg_size - METADATA_HEADER_SIZE)) / dev->msg_size;
	g->nbufs = (sp->ns[0] / g->spbuf) + 1;
	g->nbufs += SDR_FUDGE;
	g->ts = 0;
	g->bidx = 0;
	sp->ns[1] = 0;

	// init stream;

	ret = bladerf_init_stream(&stream, dev, sdr_rx_async_cb,
		&g->sbufs, g->nbufs, BLADERF_FORMAT_SC16_Q11_META,
		SDR_SPBUF, SDR_NXFER, g);
	if(ret)
		return("async stream init failed");

	// set stream timeout;

	ret = bladerf_set_stream_timeout(dev, BLADERF_MODULE_RX, 1000);
	err = "error setting stream timeout";
	if(ret)
		goto out;

	// enable rx module;

	ret = bladerf_enable_module(dev, BLADERF_MODULE_RX, 1);
	if(ret)
		return("cannot enable SDR rx");

	// enable stream from rx;
	// callbacks are being done;

	ret = bladerf_stream(stream, BLADERF_MODULE_RX);
	err = "error streaming rx samples";
	if(ret)
		goto out;
	err = "sampling truncated";
	if(g->bidx != g->nbufs)
		goto out;

	// copy async msg bufs to sample buffer;

	ss = g->sbufs[SDR_FUDGE];
	tsexp = metadata_get_timestamp(ss);

	sd = sp->sbuf;
	nb = sp->ns[0] * SDR_BPS;
	err = "sample discontinuity";

	for(bidx = SDR_FUDGE; nb && (bidx < g->nbufs); bidx++) {
		ss = g->sbufs[bidx];
		for(n = 0; n < (SDR_SPBUF * SDR_BPS); n += dev->msg_size) {
			ts = metadata_get_timestamp(ss);
			if(ts != tsexp)
				goto out;

			len = dev->msg_size - METADATA_HEADER_SIZE;
			if(nb < len)
				len = nb;
			if( !len)
				break;
			memcpy(sd, ss + METADATA_HEADER_SIZE, len);
			ss += dev->msg_size;
			sd += len;
			nb -= len;
			len /= SDR_BPS;
			sp->ns[1] += len;
			tsexp += g->spmsg;
		}
	}
	err = NULL;
out:
	bladerf_deinit_stream(stream);
	return(err);
}

// sdr sync rx;

char *
sdr_rx_sync(g_t *g, sparam_t *sp)
{
	int ret;
	char *err;
	struct bladerf_metadata meta;

	// configure sample format;

	ret = bladerf_sync_config(g->dev, BLADERF_MODULE_RX,
		BLADERF_FORMAT_SC16_Q11_META,
		1024, SDR_SPBUF, SDR_NXFER, 1000);
	err = "invalid SDR rx config";
	if(ret)
		goto out;

	// enable rx module for streaming;

	ret = bladerf_enable_module(g->dev, BLADERF_MODULE_RX, 1);
	if(ret)
		return("cannot enable SDR rx");

	// stream samples;

	memset(&meta, 0, sizeof(meta));
	meta.flags = BLADERF_META_FLAG_RX_NOW;

	ret = bladerf_sync_rx(g->dev, sp->sbuf, sp->ns[0], &meta, 1000);
	err = "error streaming SDR data";
	if(ret)
		goto out;
	err = "SDR overrun";
	if(meta.flags & BLADERF_META_STATUS_OVERRUN)
		goto out;
	err = "SDR underrun";
	if(meta.flags & BLADERF_META_STATUS_UNDERRUN)
		goto out;

	sp->ns[1] = meta.actual_count;
	err = NULL;
out:
	return(err);
}

// sample the sdr rx;
// optionally, compute sample statistics;

char *
sdr_rx(g_t *g, sparam_t *sp, sstats_t *stats)
{
	int ret;
	char *err;
	struct bladerf_metadata meta;

	// disable rx just in case;

	bladerf_enable_module(g->dev, BLADERF_MODULE_RX, 0);

	// set frequency;

	ret = bladerf_set_frequency(g->dev, BLADERF_MODULE_RX, sp->freq);
	if(ret)
		return("cannot set SDR frequency");

	// set RX LNA gain;

	ret = bladerf_set_gain(g->dev, BLADERF_MODULE_RX, sp->rxgain);
	if(ret)
		return("cannot set RX gain");

	// set sample rate;

	ret = bladerf_set_sample_rate(g->dev, BLADERF_MODULE_RX, sp->srate[0], sp->srate + 1);
	if(ret)
		return("cannot set SDR sample rate");

	// set bandwidth;

	ret = bladerf_set_bandwidth(g->dev, BLADERF_MODULE_RX, sp->bwidth[0], sp->bwidth + 1);
	if(ret)
		return("cannot set SDR bandwidth");

	if(g->verbose)
		sparam_print(sp);

	// stream samples;

	sp->ns[1] = 0;
#ifdef SDR_ASYNC
	err = sdr_rx_async(g, sp);
#else // SDR_ASYNC
	err = sdr_rx_sync(g, sp);
#endif // SDR_ASYNC
	if(err)
		goto out;

	err = "not enough samples";
	if(sp->ns[1] != sp->ns[0]) {
		if(g->verbose)
			Msg("# only %u of %u samples\n", sp->ns[1], sp->ns[0]);
		goto out;
	}

	// compute sample statistics;

	if(stats)
		stats_compute(stats, sp->sbuf, sp->ns[0]);

	// error handling;
	err = NULL;
out:
	bladerf_enable_module(g->dev, BLADERF_MODULE_RX, 0);
	return(err);
}

// command descriptor;

typedef struct req req_t;
struct req {
	char *name;		// request;
	char *(*hdl)(g_t *, req_t *, char *);
	void *data;		// extra data to send;
	unsigned ndata;		// # of extra bytes;
};

// get scale of frequency;

unsigned
f_scale(char *s)
{
	int scale;

	if( !*s)
		scale = 1;
	else if( !strcasecmp(s, "Hz"))
		scale = 1;
	else if( !strcasecmp(s, "k"))
		scale = 1000;
	else if( !strcasecmp(s, "kHz"))
		scale = 1000;
	else if( !strcasecmp(s, "M"))
		scale = 1000000;
	else if( !strcasecmp(s, "MHz"))
		scale = 1000000;
	else
		scale = 0;
	return(scale);
}

// set sample rate;

char *
hdl_sr(g_t *g, req_t *req, char *s)
{
	unsigned srate;
	int scale;
	char *ep;

	srate = strtoul(s, &ep, 0);
	if(ep <= s)
		return("missing sample rate");
	scale = f_scale(ep);
	if( !scale)
		return("invalid sample rate");
	g->srate = srate * scale;
	return(SDR_REP_OK);
}

// set bandwidth;

char *
hdl_bw(g_t *g, req_t *req, char *s)
{
	unsigned bw;
	int scale;
	char *ep;

	bw = strtoul(s, &ep, 0);
	if(ep <= s)
		return("missing bandwidth");
	scale = f_scale(ep);
	if( !scale)
		return("invalid bandwidth");
	g->bwidth = bw * scale;
	return(SDR_REP_OK);
}

// set center frequency;

char *
hdl_f(g_t *g, req_t *req, char *s)
{
	int64_t f;
	int scale;
	char *ep;

	f = strtoll(s, &ep, 0);
	if(ep <= s)
		return("missing frequency");
	scale = f_scale(ep);
	if( !scale)
		return("invalid frequency");
	g->freq = f * scale;
	return(SDR_REP_OK);
}

// set rx gain;
// assumes automatic selection if multiple gain stages;

char *
hdl_rxgain(g_t *g, req_t *req, char *s)
{
	int rxgain;
	char *ep;

	rxgain = strtol(s, &ep, 0);
	if(ep <= s)
		return("missing rx gain");
	if(*ep || (rxgain < 0) || (rxgain > 66))
		return("invalid rx gain");
	g->rxgain = rxgain;
	return(SDR_REP_OK);
}

// set # of samples;

char *
hdl_ns(g_t *g, req_t *req, char *s)
{
	unsigned ns;
	char *ep;

	ns = strtoul(s, &ep, 0);
	if((ep <= s) || *ep)
		return("invalid # of samples");
	g->nsamples = ns;
	return(SDR_REP_OK);
}

// sample the SDR input;

char *
hdl_sample(g_t *g, req_t *req, char *s)
{
	char *err;
	int clip;
	sparam_t sparam;
	sstats_t stats;

	// must have a bladerf;

	if( !g->dev)
		return("no SDR");

	// allocate sample buffer;
	// only reallocate if more is needed;

	if( !sdr_sbuf(g, g->nsamples))
		return("cannot allocate sample buffer");

	// get samples;

	sparam.sbuf = g->sbuf;
	sparam.freq = g->freq;
	sparam.rxgain = g->rxgain;
	sparam.srate[0] = g->srate;
	sparam.bwidth[0] = g->bwidth;
	sparam.ns[0] = g->nsamples;

again:
	err = sdr_rx(g, &sparam, &stats);
#ifdef SDR_RETRY
	if(err && !strcmp(err, "sampling truncated"))
		goto again;
#endif // SDR_RETRY
	if(err)
		return(err);

	// report likely clipping;

	clip = 0;
	clip |= (stats.min[0] <= SDR_IQ_MIN);
	clip |= (stats.min[1] <= SDR_IQ_MIN);
	clip |= (stats.max[0] >= SDR_IQ_MAX);
	clip |= (stats.max[1] >= SDR_IQ_MAX);
	if(clip)
		return("clipped samples");

	return(SDR_REP_OK);
}

// lookup dc calibration;
// look up from table generated by bladeRF-cli;
// these are values for the lms6002;
// due to limited # of bits, there is residual error,
// that needs to be handled separately;

char *
dc_cal_lookup(struct dc_cal_tbl *dcct, int64_t freq, struct dc_cal_entry **dccp)
{
	struct dc_cal_entry *dcce, *dccm;
	int64_t df, mf;
	int n;

	if( !dcct)
		return("no rx dc calibrations");

	// find min delta freq entry,
	// even though table is sorted;

	mf = (1LL << 40);
	dccm = NULL;
	for(n = 0; n < dcct->n_entries; n++) {
		dcce = dcct->entries + n;
		df = llabs(freq - dcce->freq);
		if(df < mf) {
			dccm = dcce;
			mf = df;
		}
	}
	if( !dccm)
		return("no rx dc calibration for frequency");
	if(mf > 10000000LL)
		return("rx dc calibration frequency offset too large");
	if(dccp)
		*dccp = dccm;
	return(NULL);
}

// lookup fine calibration;

cal_entry_t *
cal_lookup(g_t *g, int64_t freq, float rxgain)
{
	cal_entry_t *ce, *e;
	int64_t df, mf;

	// find min frequency delta;

	mf = 1LL << 40;
	e = NULL;
	for(ce = g->cal.ce; ce; ce = ce->next) {
		if(ce->rxgain != rxgain)
			continue;	// not wanted gain;
		df = llabs(freq - ce->freq);
		if(df < mf) {
			e = ce;
			mf = df;
		}
		if(df > mf)
			break;		// table is sorted by freq;
	}
	return(e);
}

// window functions;

double
wf_rect(int n, double sc)
{
	return(1.0);
}

double
wf_hann(int n, double sc)
{
	return(0.5 * (1.0 - cos((double)n * sc)));
}

double
wf_hamming(int n, double sc)
{
	double a, b;

	a = 0.53836;
	b = 0.46164;
	return(a - b * cos((double)n * sc));
}

double
wf_blackman(int n, double sc)
{
	double a0, a1, a2;

	a0 = 0.42659;
	a1 = 0.49656;
	a2 = 0.076849;
	return(a0 - a1 * cos((double)n * sc) + a2 * cos((double)n * 2.0 * sc));
}

// compute power spectrum;
// "ps dftsz window-func"

char *
hdl_comps(g_t *g, req_t *req, char *s)
{
	struct bladerf *dev = g->dev;
	cal_entry_t *ce;
	fftw_complex *in, *out, *cp;
	double *wf, *dp;
	spwr_t *spwr;
	fftw_plan plan;
	int bins, dftsz;
	int win, n;
	struct timeval t0, t1;
	short *sp;
	char *ep, *xp;
	char *err = "out of memory";
	double dcoff[2];
	double sc, r, i, m;
	double (*winf)(int, double);

	// must have a bladerf;

	if( !dev)
		return("no SDR");
	if(g->nsamples <= 0)
		return("no samples");

	// get dft size;

	dftsz = strtol(s, &ep, 0);
	xp = skip_spc(ep);
	if((ep <= s) || (xp <= ep) || (dftsz < 10))
		return("invalid dft size");

	// get window function;

	if( !strcmp(xp, "rect"))
		winf = wf_rect;
	else if( !strcmp(xp, "hann"))
		winf = wf_hann;
	else if( !strcmp(xp, "hamming"))
		winf = wf_hamming;
	else if( !strcmp(xp, "blackman"))
		winf = wf_blackman;
//XXX more winf
	else
		return("unknown window function");

	bins = g->nsamples / dftsz;
	if(bins <= 0)
		return("not enough samples for dft size");

	// lms6002 dc calibration came from cal file;
	// complain if no or not good enough lms6002 calibration data;
	// SDR_IQ_RND for SDR_IQ_MIN..SDR_IQ_MAX scale;

	dcoff[0] = SDR_IQ_RND;
	dcoff[1] = SDR_IQ_RND;

	err = dc_cal_lookup(dev->cal.dc_rx, g->freq, NULL);
	if(err)
		return(err);

	// apply fine dc calibration;

	ce = cal_lookup(g, g->freq, g->rxgain);
	if( !ce)
		return("no/unusable fine calibration data");
	dcoff[0] -= ce->rxiqdco[0];
	dcoff[1] -= ce->rxiqdco[1];

	// (re)allocate power spectrum buffer;
	// keep old buffers around, so we can send them to clients;

	if(g->nspwr != dftsz) {
		if(g->spwr)
			free(g->spwr);
		g->nspwr = dftsz;
		g->spwr = malloc(sizeof(spwr_t) * dftsz);
		if( !g->spwr)
			return(err);
	}

	// allocate window function / dft buffers;

	wf = malloc(sizeof(double) * dftsz);
	in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * dftsz);
	out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * dftsz);
	if( !wf || !in || !out)
		goto out;

	// create dft plan;

	plan = fftw_plan_dft_1d(dftsz, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	err = "dft plan failure";
	if( !plan)
		goto out;

	// init min/max/avg power spectrum;

	for(spwr = g->spwr, n = 0; n < dftsz; n++, spwr++) {
		spwr->mma[0] = __DBL_MAX__;
		spwr->mma[1] = __DBL_MIN__;
		spwr->mma[2] = 0.0;
	}

	// init windowing function;
	// combine sample normalization and wf coefficient;

	sc = 2.0 * M_PI / (double)(dftsz - 1);
	for(dp = wf, n = 0; n < dftsz; n++, dp++)
		*dp = 1.0 / (double)SDR_IQ_MAX * winf(n, sc);

	// do 1d DFT over windows;

	gettimeofday(&t0, NULL);
	sp = g->sbuf;
	for(win = 0; win < bins; win++) {
		for(dp = wf, cp = in, n = dftsz; n--; cp++) {
			sc = *dp++;
			cp[0][0] = ((double)*sp++ + dcoff[0]) * sc;
			cp[0][1] = ((double)*sp++ + dcoff[1]) * sc;
		}
		fftw_execute(plan);

		cp = out;
		spwr = g->spwr;
		for(n = 0; n < dftsz; n++, cp++, spwr++) {
			r = cp[0][0];
			i = cp[0][1];
			m = r*r + i*i;
			spwr->mma[2] += m;
			if(m < spwr->mma[0])
				spwr->mma[0] = m;
			if(m > spwr->mma[1])
				spwr->mma[1] = m;
		}
	}
	gettimeofday(&t1, NULL);
	if(g->verbose)
		Msg("dft took %ldusec\n", (t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec));

	m = 0.5 / (double)dftsz;
	sc = 0.5 / (double)(dftsz * bins);
	spwr = g->spwr;
	for(n = 0; n < dftsz; n++, spwr++) {
		spwr->mma[0] = 10.0 * log10(spwr->mma[0] * m);
		spwr->mma[1] = 10.0 * log10(spwr->mma[1] * m);
		spwr->mma[2] = 10.0 * log10(spwr->mma[2] * sc);
	}

	// free buffers;

	err = SDR_REP_OK;
out:
	if(plan)
		fftw_destroy_plan(plan);
	if(out)
		fftw_free(out);
	if(in)
		fftw_free(in);
	if(wf)
		free(wf);

	return(err);
}

// get something;

char *
hdl_getps(g_t *g, req_t *req, char *s)
{
	int pi, dftsz, k, n;
	spwr_t *spwr;
	float *fp;

	// which to get;

	if( !strcmp(s, "min"))
		pi = 0;
	else if( !strcmp(s, "max"))
		pi = 1;
	else if( !strcmp(s, "avg"))
		pi = 2;
	else
		return("unknown power index");

	// must have a spectrum;

	dftsz = g->nspwr;
	spwr = g->spwr;
	if( !dftsz || !spwr)
		return("no spectrum data");

	// compute data blob size;
	// (re)allocate 

	n = dftsz * sizeof(float);
	if(n != req->ndata) {
		if(req->data)
			free(req->data);
		req->ndata = n;
		req->data = malloc(n);
	}
	if( !req->data)
		return("out of memory");

	// fill in data blob;
	// unfold to make linear spectrum;

	k = (dftsz + 1) / 2;
	fp = req->data;
	spwr = g->spwr + k;
	for(n = k; n < dftsz; n++, spwr++)
		*fp++ = spwr->mma[pi];
	spwr = g->spwr;
	for(n = 0; n < k; n++, spwr++)
		*fp++ = spwr->mma[pi];

	return(SDR_REP_DATA);
}

// SDR commands;

req_t sdr_req[] = {
	{ "sr=", hdl_sr },
	{ "bw=", hdl_bw },
	{ "f=", hdl_f },
	{ "ns=", hdl_ns },
	{ "rxgain=", hdl_rxgain },
	{ "sample", hdl_sample },
	{ "comps ", hdl_comps },
	{ "getps ", hdl_getps },
	{ 0 },
};

// lookup request;

char *
req_lookup(char *name, req_t **reqp)
{
	req_t *req;
	char *s;
	int n;

	for(req = sdr_req; s = req->name; req++) {
		n = strlen(s);
		if( !strncmp(s, name, n)) {
			*reqp = req;
			return(name + n);
		}
	}
	return(NULL);
}

// process new input from client;

void
do_proc(g_t *g)
{
	u_char *buf = g->rxb;
	int nb = g->nrx;
	int ntx, ret;
	u_char *s, *arg, *err;
	req_t *req;

	// process all \n terminated commands in rxb[];

	for(s = buf; nb > 0; nb--, s++) {
		if(*s != '\n')
			continue;
		*s = 0;
		arg = req_lookup(buf, &req);
		if(arg && req->hdl)
			err = req->hdl(g, req, arg);
		else
			err = "unknown request or no handler";

		if(err == SDR_REP_OK)
			ntx = snprintf(g->txb, sizeof(g->txb) - 1, "ok\n");
		else if(err == SDR_REP_DATA)
			ntx = snprintf(g->txb, sizeof(g->txb) - 1, "%u\n", req->ndata);
		else {
			ntx = snprintf(g->txb, sizeof(g->txb) - 1, "%s: %s\n", buf, err);
			Msg("%s\n", g->txb);
		}
		g->txb[ntx] = 0;

		// send reply;

		ret = send(g->cso, g->txb, ntx, 0);
		if(ret != ntx)
			Msg("'%s' error sending reply\n", buf);

		// send extra data;

		ntx = req->ndata;
		if(ntx && req->data) {
			ret = send(g->cso, req->data, ntx, 0);
			if(ret != ntx)
				Msg("'%s' error sending %u data\n", buf, ntx);
		}
		buf = s + 1;
	}
	nb = buf - g->rxb;
	if(nb > 0) {
		assert(g->nrx >= nb);
		nb = g->nrx - nb;
		memcpy(g->rxb, buf, nb);
		g->nrx = nb;
	}
}

// make calibration file name;
// buffer must be BLADERF_SERIAL_LENGTH + 5;

char *
cal_file(char *serial)
{
	int len;
	char *file;

	len = strlen(serial) + 5;
	file = malloc(len);
	if(file)
		snprintf(file, len, "%s.brf", serial);
	return(file);
}

// do sdr calibration;
// the lms6002 calibration values are limited,
// so we do residual DC offset calibration ourselves;

char *
cal_sdr(g_t *g, char *file)
{
	struct bladerf *dev = g->dev;
	FILE *cfp = NULL;
	char *err;
	int n;
	struct dc_cal_tbl *dcct;
	struct dc_cal_entry *dcce;
	float rxgain;
	double rxiqdco[2];
	sparam_t sparam;
	sstats_t stats;

	// must have a default calibration file from bladeRF-cli;

	dcct = dev->cal.dc_rx;
	err = "no rx dc calibrations";
	if( !dcct)
		goto out;

	// important message;

	Msg("calibrating: make sure RX input is terminated\n");

	// open our cal file;

	cfp = fopen(file, "w");
	err = "cannot open file";
	if( !cfp)
		goto out;

	// allocate sample buffer;
	// only reallocate if more is needed;
	// use fastest sampling, no gain, but limit bandwidth to 25MHz;

	sparam.rxgain = 0;
	sparam.srate[0] = 28000000;
	sparam.bwidth[0] = 28000000;
	sparam.ns[0] = sparam.srate[0] / 2;

	if( !sdr_sbuf(g, sparam.ns[0]))
		return("cannot allocate sample buffer");
	sparam.sbuf = g->sbuf;

	// make calibrations for every entry in default cal file;

	for(n = 0; n < dcct->n_entries; n++) {
		dcce = dcct->entries + n;

		sparam.freq = dcce->freq;
	again:
		err = sdr_rx(g, &sparam, &stats);
#ifdef SDR_RETRY
		if(err && !strcmp(err, "sampling truncated"))
			goto again;
#endif // SDR_RETRY
		if(err)
			goto out;

		rxiqdco[0] = stats.avg[0];
		rxiqdco[1] = stats.avg[1];
		fprintf(cfp, "%u %.2f %.3f %.3f\n", dcce->freq, rxgain, rxiqdco[0], rxiqdco[1]);
		if(g->verbose) {
			Msg("cal: %.3fMHz, gain %.2f, IQ dc off %.2f/%.2f\n",
				(double)dcce->freq / 1000000.0, rxgain,
				rxiqdco[0], rxiqdco[1]);
		}
	}
	err = NULL;
out:
	if(cfp)
		fclose(cfp);
	return(err);
}

// get calibration data from file;

char *
cal_get(g_t *g, char *file)
{
	FILE *cfp;
	char *s, *ep, *err;
	int lnbr, n;
	int64_t freq;
	float gain, idcoff, qdcoff;
	cal_entry_t *ce, **q, *e;
	struct bladerf *dev = g->dev;
	struct dc_cal_tbl *dcct;
	struct dc_cal_entry *dcce;
	char line[1000];

	// open file;

	cfp = fopen(file, "r");
	lnbr = 0;
	err = "cannot open file";
	if( !cfp)
		goto out;

	// "freq [wspace] gain [wpspace] idcoff [wspace] qdcoff [wspace] \n"

	while(fgets(line, sizeof(line) - 1, cfp)) {
		lnbr++;
		s = strchr(line, '\n');
		if(s)
			*s = 0;
		s = skip_spc(line);
		if( !*s)
			continue;
		if(*s == '#')
			continue;

		freq = strtoll(s, &ep, 0);
		err = "invalid frequency";
		if((freq <= 0) || (ep <= s))
			goto out;

		s = skip_spc(ep);
		gain = strtof(s, &ep);
		err = "invalid gain";
		if(ep <= s)
			goto out;

		s = skip_spc(ep);
		idcoff = strtof(s, &ep);
		err = "invalid I dc offset";
		if(ep <= s)
			goto out;

		s = skip_spc(ep);
		qdcoff = strtof(s, &ep);
		err = "invalid Q dc offset";
		if(ep <= s)
			goto out;

		// allocate new calibration entry;

		ce = malloc(sizeof(cal_entry_t));
		err = "out of memory";
		if( !ce)
			goto out;
		ce->lnbr = lnbr;
		ce->freq = freq;
		ce->rxgain = gain;
		ce->rxiqdco[0] = idcoff;
		ce->rxiqdco[1] = qdcoff;

		// add in sorted order;
		// freq -> then gain;

		for(q = &g->cal.ce; e = *q; q = &e->next) {
			if(e->freq > ce->freq)
				break;
			if(e->freq == ce->freq) {
				err = "duplicate entry";
				if(e->rxgain == ce->rxgain)
					goto out;
				if(e->rxgain > ce->rxgain)
					break;
			}
		}
		ce->next = *q;
		*q = ce;
	}

	// check alignment with bladeRF-cli calibration file;
	// each cal file entry must have corresponding entry from our file;

	dcct = dev->cal.dc_rx;
	err = "no rx dc calibrations";
	if( !dcct)
		goto out;

	err = "calibration mismatch, rerun calibration";
	lnbr = 0;
	e = g->cal.ce;
	for(n = 0; n < dcct->n_entries; n++) {
		dcce = dcct->entries + n;
		if( !e)
			goto out;		// missing entries;
		lnbr = e->lnbr;
		if(dcce->freq != e->freq)
			goto out;		// other freq points;
		while(e && (dcce->freq == e->freq))
			e = e->next;		// skip over same freq;
	}
	err = NULL;
out:
	if(err)
		Msg("%s:%d: %s\n", file, lnbr, err);
	if(cfp)
		fclose(cfp);
	return(err);
}

// main entry;

int
main(int argc, char **argv)
{
	g_t *g = &G;
	int c, ret, fd, nb;
	char *ep, *err, *cal;
	struct timeval tv;
	struct sockaddr_in sa;
	fd_set rdset, wrset;
	char date[32];
	char serial[BLADERF_SERIAL_LENGTH];

	g->cmd = argv[0];

	// report startup time;

	ret = gettimeofday(&tv, NULL);
	if(ret)
		Fatal("cannot get time of day: %s\n", strerror(errno));
	strftime(date, sizeof(date), "%Y%m%d-%H%M%S", gmtime(&tv.tv_sec));
	Msg("starting: %s\n", date);

	// get options;

	while((c = getopt(argc, argv, "cvd:h")) != EOF) {
		switch(c) {
		case 'c':
			g->cal.sdr = 1;
			break;
		case 'v':
			g->verbose++;
			break;
		case 'd':
			g->delay = strtoul(optarg, NULL, 0);
			break;
		case 'h':
			usage(g, stdout);
		case '?':
		default:
			usage(g, stderr);
		}
	}

	// open bladerf SDR;
	// optionally delay, in case of hotplug;

	if(g->delay) {
		Msg("delaying for %u seconds, for SDR to get ready\n", g->delay);
		sleep(g->delay);
	}

	bladerf_init_devinfo(&g->info);
	ret = bladerf_open_with_devinfo(&g->dev, &g->info);
	if(ret)
		Fatal("no bladerf SDR found: %s\n", bladerf_strerror(ret));

	// get serial number;

	ret = bladerf_get_serial(g->dev, serial);
	if(ret)
		Fatal("could not get bladerf serial: %s\n", bladerf_strerror(ret));
	Msg("bladerf found, serial: %s\n", serial);

	// check if fpga is configured;

	ret = bladerf_is_fpga_configured(g->dev);
	if( !ret) {
		Msg("bladerf FPGA not configured\n");
		Fatal("implement FPGA loading\n");
	}

	// disable transmitter;

	ret = bladerf_enable_module(g->dev, BLADERF_MODULE_TX, 0);
	if(ret)
		Fatal("cannot disable transmitter: %s\n", bladerf_strerror(ret));

    	// setup single-shot request/reply timer;
	// first tick in one interval;
	// any failure is fatal;

	g->tmr.ts.it_interval.tv_sec = 0;
	g->tmr.ts.it_interval.tv_nsec = 0;

	g->tmr.fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
	if(g->tmr.fd < 0)
		Fatal("cannot create timer: %s\n", strerror(errno));

	// make calibration file name;

	cal = cal_file(serial);
	if( !cal)
		Fatal("cannot make calibration file name for %s\n", serial);

	// do calibration if requested;

	if(g->cal.sdr) {
		err = cal_sdr(g, cal);
		if(err)
			Fatal("calibration failed: %s\n", err);
		Msg("calibration saved to %s\n", cal);
	}

	// get/check calibration data;

	err = cal_get(g, cal);
	if(err)
		Fatal("invalid calibration data: %s\n", cal);

	// open socket;

	g->sock = socket(PF_INET, SOCK_STREAM, 0);
	if(g->sock < 0)
		Fatal("cannot open socket: %s\n", strerror(errno));

	// bind socket;

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(PORT);
	
	if(bind(g->sock, (struct sockaddr *)&sa, sizeof(sa)) < 0)
		Fatal("bind failed: %s\n", strerror(errno));
	if(listen(g->sock, 5))
		Fatal("listen failed: %s\n", strerror(errno));

	// main loop;

	while( !g->stop) {
		// setup for select;

		FD_ZERO(&rdset);
		FD_ZERO(&wrset);

		fd = set_fd(g->tmr.fd, &rdset, 0);
		fd = set_fd(g->sock, &rdset, fd);
		if(g->cso >= 0)
			fd = set_fd(g->cso, &rdset, fd);

		// max sleep to limit poll rate;

		tv.tv_sec = 0;
		tv.tv_usec = 10000;

		ret = select(fd + 1, &rdset, &wrset, NULL, &tv);
		if(ret < 0) {
			if(errno == EINTR)
				continue;
			Fatal("select failed: %s\n", strerror(errno));
		}
		if(ret == 0)
			continue;

		// handle timer;

		if(FD_ISSET(g->tmr.fd, &rdset))
			timer_hdl(&g->tmr);

		// handle new connections;

		if(FD_ISSET(g->sock, &rdset))
			do_accept(g);

		// handle incoming commands;

		if((g->cso >= 0) && FD_ISSET(g->cso, &rdset)) {
			nb = do_read(g);
			if(nb > 0)
				do_proc(g);
			else {
				close(g->cso);
				g->cso = -1;
			}
		}
			
	}

	// close fildes;

	if(g->sock >= 0)
		close(g->sock);
	if(g->cso >= 0)
		close(g->cso);
brf_out:
	if(g->dev)
		bladerf_close(g->dev);

	return(g->nerr);
}

