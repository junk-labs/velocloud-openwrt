// swtest.c v1 Sandra Berndt
// sierra wireless cellular modem tests;
// usable for colo-testing and factory test;

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

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

// config;

#define SZ_RBUF 1000		// max size tty recv buffer;
#define SZ_ARGV 20		// max # of argv[] lines;

#define SZ_SDR_REQ 200
#define SZ_SDR_REP 200

#define SDR_SRATE 20000000	// default sample rate;
#define SDR_DFTSZ 1024		// dft size;
#define SDR_RXGAIN 0		// rx gain;
#define RFM_DB_BIN 1.2		// max db/bin outside limits;

#define N_REX	7
#define REG_DEF (REG_EXTENDED | REG_ICASE)

#define PORT 0x4252
#define PLOT_DIR "plots"

// types;

typedef struct smap smap_t;
typedef struct tmr tmr_t;
typedef struct rea rea_t;
typedef struct atc atc_t;
typedef struct ats ats_t;
typedef struct atl atl_t;
typedef struct rfpe rfpe_t;
typedef struct rfm rfm_t;
typedef struct sdr sdr_t;
typedef struct sdreq sdreq_t;
typedef struct g g_t;

// generic timer data;
    
struct tmr {
	int fd;			// timer fd;
	struct itimerspec ts; 
	void (*hdl)(void *);	// timeout handler;
	void *priv;		// private data;
}; 

// regex action;

struct rea {
	int mflag;		// regex flags;
	char *pat;		// pattern to match;
	char *(*hdl)(g_t *, atc_t *, rea_t *);	// handler on match;
	regex_t *re;		// compiled regex;
};

// AT command descriptor;

struct atc {
	int (*comp)(g_t *, atc_t *); // cmd compose handler;
	char *cmd;		// command to send;
	int rto;		// reply timeout (msec);
	unsigned delay;		// delay after response;
	rea_t *re[N_REX];	// regex list;
};

// AT command sequence;

struct ats {
	atc_t *atc;		// command descriptor;
	char rei[N_REX];	// atc.re[] jump targets;
};

enum rei {
	ATS_DONE = -1,
};

// command list;

struct atl {
	rfm_t *rfm;		// rf data;
	ats_t *ats;		// at sequence;
	int nats;		// # of cmds in sequence;
};

// expected rf power envelope;

struct rfpe {
	rfpe_t *next;		// next in list;
	int ofc;		// offset from center frequency;
	float r[2];		// min/max range;
};

// rf measurement config;

struct rfm {
	rfm_t *next;		// next in list;
	char *msg;		// message to print;
	smap_t *band;		// band;
	smap_t *chan;		// channel;
	smap_t *bwidth;		// bandwidth;
	smap_t *ufreq;		// uplink frequency;
	smap_t *dfreq;		// downlink frequency;
	smap_t *txmod;		// tx modulation;
	rfpe_t *pe;		// expected power envelope;
	int channel;		// actual channel;
	int bandwidth;		// SDR bandwidth;
	int tech;		// technology index;
	unsigned sr;		// sample rate in samples/sec;
	int64_t uf, df;		// <0 means illegal, in Hz;
	float dbin[3];		// max db/bin, act db, act db/bin power outside of limits;
	unsigned nbin;		// # of bins outside of limits;
};

// spectrum analyzer info;

struct sdr {
	char *host;		// SDR host;
	struct hostent *svr;	// server ip;
	int sock;		// socket;
	struct sockaddr_in sa;	// socket address;
};

// sdr request info;

struct sdreq {
	void *rxb;		// receive buffer;
	unsigned szrb;		// sizeof rxb[];
	unsigned ndata;		// # of bytes of data blob;
	void *data;		// ptr to data;
};

// global data;

struct g {
	char *cmd;		// argv[0];
	char *vcid;		// vc board id;
	char *profile;		// execute profile;
	char *tty;		// tty for at commands;
	int debug;		// debug flags;
	int dur;		// test duration, or <=0 wait for enter;
	int tfd;		// tty fildes;
	int stop;		// stop the main loop;
	int model;		// modem model;
	unsigned nerr;		// # of errors;
	unsigned failed;	// # of faield tests;
	tmr_t rrt;		// req/reply timer;
	ats_t *ats;		// current AT sequence state;
	atl_t *atl;		// current AT seq list;
	int nbuf;		// # of chars in buf[];
	regex_t regex;		// compiled regex;
	char buf[SZ_RBUF];	// tty recv buffer;
	char *argv[SZ_ARGV];	// split response lines;
	rfm_t *rfm;		// all measurment descriptors;
	rfm_t *rfmc;		// current measurement;
	rfm_t rfmo;		// parameters from options;
	sdr_t sdr;		// analyzer info;
};

g_t G;

enum debug {
	DBG_TTY = 0x01,		// debug tty messages;
};

enum model {
	MODEL_NONE = 0,
	MODEL_EM7455,
	MODEL_EM7430,
};

// band to dasband arg mapping;

struct smap {
	char *str;		// string value
	union {
		int i;		// integer value;
		int ir[2];	// integer range;
		float f;	// float value;
		float fr[2];	// float range;
		int64_t (*freq)(int ch); // frequency map;
	};
	int flags;		// type of band;
	void *priv;		// private data;
};

enum sflag {
	SM_GSM = 1 << 0,
	SM_WCDMA = 1 << 1,
	SM_LTE = 1 << 2,
	SM_ALL = SM_GSM | SM_WCDMA | SM_LTE,

	SM_MASK = 0xff,
};

enum sprint {
	SP_INT = 0,
	SP_INT_RANGE,
	SP_FLOAT,
	SP_FLOAT_RANGE,
};

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

// frequency mapper functions;

int64_t
ufreq_utra_1(int uarfcn)
{
	int64_t f = uarfcn * 1000000LL / 5LL;

	switch(uarfcn) {
	default:
		if(uarfcn < 9612)
			break;
		if(uarfcn > 9888)
			break;
		return(f);
	}
	return(-1);
}

int64_t
ufreq_utra_2(int uarfcn)
{
	int64_t f = uarfcn * 1000000LL / 5LL;

	switch(uarfcn) {
	default:
		if(uarfcn < 9262)
			break;
		if(uarfcn > 9538)
			break;
		return(f);
	}
	return(-1);
}

int64_t
ufreq_utra_5(int uarfcn)
{
	int64_t f = uarfcn * 1000000LL / 5LL;

	switch(uarfcn) {
	case 782:
	case 787:
	case 807:
	case 812:
	case 837:
	case 862:
		return(f + 670100000LL);
	default:
		if(uarfcn < 4132)
			break;
		if(uarfcn > 4233)
			break;
		return(f);
	}
	return(-1);
}

int64_t
ufreq_utra_8(int uarfcn)
{
	int64_t f = uarfcn * 1000000LL / 5LL;

	if(uarfcn < 2712)
		return(-1);
	if(uarfcn > 2863)
		return(-1);
	return(f + 340000000LL);
}

int64_t
freq_XXX(int uarfcn)
{
	return(-1); //XXX
}

// uplink frequencies;

smap_t ufreq_map[] = {
	{ "GSM-850", .freq = freq_XXX, SM_GSM },
	{ "GSM-900", .freq = freq_XXX, SM_GSM },
	{ "GSM-900", .freq = freq_XXX, SM_GSM },
	{ "GSM-1800", .freq = freq_XXX, SM_GSM },
	{ "GSM-1900", .freq = freq_XXX, SM_GSM },
	{ "WCDMA-850", .freq = ufreq_utra_5, SM_WCDMA },
	{ "WCDMA-900", .freq = ufreq_utra_8, SM_WCDMA },
	{ "WCDMA-1900", .freq = ufreq_utra_2, SM_WCDMA },
	{ "WCDMA-2100", .freq = ufreq_utra_1, SM_WCDMA },
	{ "LTE-B1", .freq = freq_XXX, SM_LTE },
	{ "LTE-B3", .freq = freq_XXX, SM_LTE },
	{ "LTE-B4", .freq = freq_XXX, SM_LTE },
	{ "LTE-B7", .freq = freq_XXX, SM_LTE },
	{ "LTE-B8", .freq = freq_XXX, SM_LTE },
	{ "LTE-B13", .freq = freq_XXX, SM_LTE },
	{ "LTE-B17", .freq = freq_XXX, SM_LTE },
	{ "LTE-B20", .freq = freq_XXX, SM_LTE },
	{ 0 },
};

// downlink frequencies;

smap_t dfreq_map[] = {
	{ "GSM-850", .freq = freq_XXX, SM_GSM },
	{ "GSM-900", .freq = freq_XXX, SM_GSM },
	{ "GSM-900", .freq = freq_XXX, SM_GSM },
	{ "GSM-1800", .freq = freq_XXX, SM_GSM },
	{ "GSM-1900", .freq = freq_XXX, SM_GSM },
	{ "WCDMA-850", .freq = freq_XXX, SM_WCDMA },
	{ "WCDMA-900", .freq = freq_XXX, SM_WCDMA },
	{ "WCDMA-1900", .freq = freq_XXX, SM_WCDMA },
	{ "WCDMA-2100", .freq = freq_XXX, SM_WCDMA },
	{ "LTE-B1", .freq = freq_XXX, SM_LTE },
	{ "LTE-B3", .freq = freq_XXX, SM_LTE },
	{ "LTE-B4", .freq = freq_XXX, SM_LTE },
	{ "LTE-B7", .freq = freq_XXX, SM_LTE },
	{ "LTE-B8", .freq = freq_XXX, SM_LTE },
	{ "LTE-B13", .freq = freq_XXX, SM_LTE },
	{ "LTE-B17", .freq = freq_XXX, SM_LTE },
	{ "LTE-B20", .freq = freq_XXX, SM_LTE },
	{ 0 },
};

// string maps;

smap_t channel_map[] = {
	{ "GSM-850", .ir = { 128, 251 }, SM_GSM },
	{ "GSM-900", .ir = { 1, 24 }, SM_GSM },
	{ "GSM-900", .ir = { 975, 1023 }, SM_GSM },
	{ "GSM-1800", .ir = { 512, 885 }, SM_GSM },
	{ "GSM-1900", .ir = { 512, 810 }, SM_GSM },
	{ "WCDMA-850", .ir = { 4132, 4233 }, SM_WCDMA },
	{ "WCDMA-900", .ir = { 2712, 2863 }, SM_WCDMA },
	{ "WCDMA-1900", .ir = { 9262, 9538 }, SM_WCDMA },
	{ "WCDMA-2100", .ir = { 9612, 9888 }, SM_WCDMA },
	{ "LTE-B1", .ir = { 18000, 18599 }, SM_LTE },
	{ "LTE-B3", .ir = { 19200, 19949 }, SM_LTE },
	{ "LTE-B4", .ir = { 19950, 20399 }, SM_LTE },
	{ "LTE-B7", .ir = { 20750, 21449 }, SM_LTE },
	{ "LTE-B8", .ir = { 21450, 21799 }, SM_LTE },
	{ "LTE-B13", .ir = { 23180, 23279 }, SM_LTE },
	{ "LTE-B17", .ir = { 23730, 23849 }, SM_LTE },
	{ "LTE-B20", .ir = { 24150, 24449 }, SM_LTE },
	{ 0 },
};

smap_t band_map[] = {
	{ "GSM-850", 18, SM_GSM },
	{ "GSM-900", 10, SM_GSM },
	{ "GSM-1800", 11, SM_GSM },
	{ "GSM-1900", 12, SM_GSM },
	{ "WCDMA-850", 22, SM_WCDMA },
	{ "WCDMA-900", 29, SM_WCDMA },
	{ "WCDMA-1900", 16, SM_WCDMA },
	{ "WCDMA-2100", 9, SM_WCDMA },
	{ "LTE-B1", 34, SM_LTE },
	{ "LTE-B3", 44, SM_LTE },
	{ "LTE-B4", 42, SM_LTE },
	{ "LTE-B7", 35, SM_LTE },
	{ "LTE-B8", 47, SM_LTE },
	{ "LTE-B13", 36, SM_LTE },
	{ "LTE-B17", 37, SM_LTE },
	{ "LTE-B20", 56, SM_LTE },
	{ 0 },
};

// .ir[0] is module argument;
// .ir[1] is SDR measurement bandwidth;

#define BWIDTH_DEF (bwidth_map + 3)
smap_t bwidth_map[] = {
	{ "1.4", .ir = { 0, 2000000 }, SM_LTE },
	{ "3", .ir = { 1, 3500000 }, SM_LTE },
	{ "5", .ir = { 2, 6000000 }, SM_LTE },
	{ "10", .ir = { 3, 12000000 }, SM_LTE },
	{ "15", .ir = { 4, 18000000 }, SM_LTE },
	{ "20", .ir = { 5, 24000000 }, SM_LTE },
	{ 0 },
};

#define TXMOD_DEF (txmod_map + 0)
smap_t txmod_map[] = {
	{ "WCDMA", 0, SM_WCDMA },
	{ "CW", 1, SM_WCDMA },
	{ "QPSK", 0, SM_LTE },
	{ "QAM16", 1, SM_LTE },
	{ "QAM256", 2, SM_LTE },
	{ 0 },
};

// print a string map strings;

void
smap_list(FILE *out, smap_t *map, char *name)
{
	fprintf(out, " %s:", name);
	for(; map->str; map++)
		fprintf(out, " %s", map->str);
	fprintf(out, "\n");
}

// print map;

void
smap_print(FILE *out, smap_t *map, char *name, int type)
{
	fprintf(out, " %s:\n", name);
	for(; map->str; map++) {
		switch(type) {
		case SP_INT:
			fprintf(out, "  %s: %d\n", map->str, map->i);
			break;
		case SP_INT_RANGE:
			fprintf(out, "  %s: %d-%d\n", map->str, map->ir[0], map->ir[1]);
			break;
		case SP_FLOAT:
			fprintf(out, "  %s: %f\n", map->str, map->f);
			break;
		case SP_FLOAT_RANGE:
			fprintf(out, "  %s: %f-%f\n", map->str, map->fr[0], map->fr[1]);
			break;
		}
	}
}

// lookup mapped value;

smap_t *
smap_lookup(smap_t *map, char *str)
{
	for(; map->str; map++) {
		if( !strcmp(map->str, str))
			return(map);
	}
	return(NULL);
}

// return channel map for band;

smap_t *
smap_channel(smap_t *band)
{
	smap_t *chan;

	assert(band);
	for(chan = channel_map; chan->str; chan++) {
		if( !strcmp(band->str, chan->str))
			return(chan);
	}
	return(NULL);
}

// map uplink/downlink frequency;

smap_t *
smap_freq(smap_t *band, smap_t *freq)
{
	assert(band);
	for(; freq->str; freq++) {
		if( !strcmp(band->str, freq->str))
			return(freq);
	}
	return(NULL);
}


// split response into lines;
// returns # of lines;

int
resp_split(g_t *g)
{
	char c, *s;
	char **argv;
	int i = 1;

	s = g->buf;
	argv = g->argv;
	*argv++ = s;

	for(; c = *s; s++) {
		if(c == '\r') {
			*s = 0;
			c = *++s;
		}
		if(c == '\n') {
			*s = 0;
			if(i < (SZ_ARGV - 1))
				*argv++ = s + 1;
			i++;
		}
	}
	*argv = NULL;
	return(i);
}

// pseudo commands;
// processed within atc list but do not send anything to modem;
// compose handler used to do other stuff;

// prolog to test mode;

int
mfg_info(g_t *g, atc_t *atc)
{
	Msg("putting modem into test mode\n");
	return(1);
}

atc_t exe_mfg = { mfg_info, "", 100, 0 };

// post cleanup;

int
mfg_post(g_t *g, atc_t *atc)
{
	Msg("cleaning up modem modes\n");
	return(1);
}

atc_t exe_post = { mfg_post, "", 100, 0 };

// print rf measurement summary;

void
rfm_summary(g_t *g, rfm_t *rfm)
{
	smap_t *band;
	smap_t *bwidth;
	smap_t *txmod;
	int n;
	char sdr[200];

	band = rfm->band;
	assert(band);
	bwidth = rfm->bwidth;
	txmod = rfm->txmod;

	n = snprintf(sdr, sizeof(sdr) - 1, " sdr bw %.2fMhz",
		rfm->bandwidth / 1000000.0);
	sdr[n] = 0;

	Msg("==> %s: %s, bw %sMHz, ch #%d, %s [ul %.2fMHz, dl %.2fMHz]%s\n",
		rfm->msg, band->str,
		bwidth? bwidth->str : "-",
		rfm->channel,
		txmod? txmod->str : "?",
		(float)rfm->uf / 1000000.0,
		(float)rfm->df / 1000000.0,
		sdr);
}

// start a measurement;

int
rfm_info(g_t *g, atc_t *atc)
{
	rfm_t *rfm = g->rfmc;

	assert(rfm);
	rfm_summary(g, rfm);
	return(1);
}

atc_t exe_info = { rfm_info, "", 100, 0 };

// send a request;
// wait for reply;

char *
sdr_req(g_t *g, sdreq_t *sdx, char *fmt, ...)
{
	va_list args;
	int nrx, nb, ret, flags, left, to, len;
	unsigned ndata;
	char *ep;
	u_char *rxp, *nl;
	u_char req[SZ_SDR_REQ];

	assert(sdx);
	sdx->ndata = 0;
	sdx->data = NULL;

	// compose message;

	flags = 0;
	va_start(args, fmt);
	nb = vsnprintf(req, sizeof(req), fmt, args);
	va_end(args);

	// send it;

	ret = send(g->sdr.sock, req, nb, flags);
	//Msg("#s %d '%s'\n", ret, req);
	if(ret != nb)
		return("send to sdr failed");

	// wait for response;

	rxp = sdx->rxb;
	nrx = 0;
	to = 2000000;
	while(1) {
		left = sdx->szrb - 1 - nrx;
		if(left <= 0)
			return("too much sdr data");
		if(to <= 0)
			return("sdr receive timeout");

		nb = recv(g->sdr.sock, rxp, left, MSG_DONTWAIT);
		if(nb <= 0) {
			if(errno == EWOULDBLOCK) {
				usleep(10000);
				to -= 10000;
				continue;
			}
			return("sdr receive error");
		}
		rxp[nb] = 0;
		nrx += nb;
		//Msg("#r %d '%s'\n", nb, sdx->rxb);

		// "ok\n" is returned when no error;

		if( !strcmp(sdx->rxb, "ok\n"))
			return(NULL);

		// wait for at least a newline;

		nl = strchr(rxp, '\n');
		rxp += nb;
		if( !nl)
			continue;
		*nl = 0;

		// an unsigned number is size of returned data;
		// else an error string;

		ndata = strtoul(sdx->rxb, &ep, 0);
		len = ep - (char *)sdx->rxb;
		if(len && !*ep) {
			len++;
			len += ndata;
			if(nrx >= len) {
				sdx->ndata = ndata;
				sdx->data = ep + 1;
				return(NULL);
			}
		} else
			break;
	}
	return(sdx->rxb);
}

// plot power spectrum;

char *
plot_ps(g_t *g, rfm_t *rfm, float *data, int dftsz, char *msg)
{
	smap_t *band;
	smap_t *txmod;
	int ret, n;
	FILE *pf;
	float freq, bw, fbin, f;
	rfpe_t *pe;
	char dir[200];
	char file[200];
	char path[500];

	band = rfm->band;
	assert(band);
	txmod = rfm->txmod;
	assert(txmod);

	// make sure dirs exist;

	ret = mkdir(PLOT_DIR, 0755);
	if(ret && (errno != EEXIST))
		return("cannot make " PLOT_DIR " directory");
	n = snprintf(dir, sizeof(dir) - 1, "%s/%s", PLOT_DIR, g->vcid);
	dir[n] = 0;
	ret = mkdir(dir, 0755);
	if(ret && (errno != EEXIST))
		return("cannot make plot directory");

	// open plot file;

	n = snprintf(file, sizeof(file) - 1, "%s-#%d-%s.plot",
		band->str, rfm->channel, txmod->str);
	file[n] = 0;
	n = snprintf(path, sizeof(path) - 1, "%s/%s", dir, file);
	path[n] = 0;

	Msg("saving into plot file: %s\n", path);
	pf = fopen(path, "w");
	if( !pf)
		return("cannot open plot file");

	// frequency parameters;

	freq = (float)rfm->uf / 1000000.0;
	bw = (float)rfm->sr / 1000000.0;
	fbin = (float)rfm->sr / (float)dftsz;

	// plot script;

	fprintf(pf, "# %s #%d %s - %s\n", band->str, rfm->channel, txmod->str, msg);
	fprintf(pf, "set terminal gif size 1280,720\n");
	fprintf(pf, "set output '%s.gif'\n", file);
	fprintf(pf, "set title \"%s #%d %s - %s (%.1f/%u %.1f)\"\n",
		band->str, rfm->channel, txmod->str,
		msg, rfm->dbin[1], rfm->nbin, rfm->dbin[2]);
	fprintf(pf, "set yrange [-90:5]\n");
	fprintf(pf, "set ylabel \"dbm\"\n");
	fprintf(pf, "set ytics 10\n");
	fprintf(pf, "set grid ytics\n");
	fprintf(pf, "set xrange [%.3f:%.3f]\n", freq - 0.5 * bw, freq + 0.5 * bw);
	fprintf(pf, "set xlabel \"MHz\"\n");
	fprintf(pf, "set xtics 1\n");
	fprintf(pf, "set grid xtics\n");
	fprintf(pf, "set arrow from %.3f,-80 to %.3f,0 nohead ls 0 lc rgb 'red'\n", freq, freq);
	fprintf(pf, "set label 1 at %.3f,-88\n", freq);
	fprintf(pf, "set label 1 '%.3f' rotate by 90\n", freq);

	// plot limits, filled;

	fprintf(pf, "$plim << EOD\n");
	for(pe = rfm->pe; pe; pe = pe->next) {
		fprintf(pf, "%.3f %.2f %.2f\n",
			(double)(rfm->uf + pe->ofc) / 1000000.0,
			pe->r[0], pe->r[1]);
	}
	fprintf(pf, "EOD\n");

	// power spectrum data;

	fprintf(pf, "$pavg << EOD\n");
	freq = (float)rfm->uf;
	for(n = 0; n < dftsz; n++) {
		f = freq + (float)(n - dftsz/2) * fbin;
		fprintf(pf, "%f %.2f\n", f / 1000000.0, data[n]);
	}
	fprintf(pf, "EOD\n");
	fprintf(pf, "set style fill pattern 2 border\n");
	fprintf(pf, "plot $plim using 1:2:3 w filledcu lc '#f0d0d0', $pavg using 1:2 w lines\n");

	fflush(pf);
	fclose(pf);
	return(NULL);
}

// check power envelope;

char *
pe_check(g_t *g, rfm_t *rfm, float *data, int dftsz)
{
	rfpe_t *pe, *pn;
	int dft2 = (dftsz + 1) / 2;
	int nbin;
	int64_t ofe, ofn, ofc;
	double dofc, df, dmin, dmax;
	float lmin, lmax, pwr, pmin, pmax, pout;

	// sweep through spectrum;
	// measure power outside limits per bin;

	pout = 0.0;
	nbin = 0;
	for(pe = rfm->pe; pe; pe = pn) {
		pn = pe->next;
		if( !pn)
			break;		// end of envelope;

		ofe = (int64_t)pe->ofc * (int64_t)dftsz / rfm->sr;
		ofn = (int64_t)pn->ofc * (int64_t)dftsz / rfm->sr;
		dofc = (double)(pn->ofc - pe->ofc);
		dmin = (pn->r[0] - pe->r[0]) / dofc;
		dmax = (pn->r[1] - pe->r[1]) / dofc;
		assert(dofc >= 0);

		for(; ofe < ofn; ofe++) {
			if(ofe < -dft2)
				continue;
			if(ofe >= dft2)
				break;
			ofc = ofe * (int64_t)rfm->sr / dftsz;
			df = (double)(ofc - pe->ofc);
			lmin = pe->r[0] + df * dmin;
			lmax = pe->r[1] + df * dmax;

			pwr = data[ofe + dft2];
			pmin = lmin - pwr;
			if(pmin > 0.0)			// pwr below lmin;
				pout += pmin;
			pmax = pwr - lmax;
			if(pmax > 0.0)			// pwr above lmax;
				pout += pmax;
			if((pmin > 0.0) || (pmax > 0.0)) {
				nbin++;
			}
		}
	}
	rfm->nbin = nbin;
	rfm->dbin[1] = pout;
	rfm->dbin[2] = pout / (float)nbin;

	return(NULL);
}

// rf power output;
// timed or waiting for enter;

void
rf_pwr_timed(g_t *g)
{
	char line[8];

	if(g->dur > 0) {
		Msg("rf power on for %u seconds...\n", g->dur);
		sleep(g->dur);
	} else {
		Msg("rf power on until <ENTER>...\n");
		while(getchar() != '\n');
	}
}

// measure rf power on analyzer;

char rfpm_rxb[SZ_SDR_REP + SDR_DFTSZ * sizeof(float)];

int
rf_pwr_measure(g_t *g, atc_t *atc)
{
	char *err, *sts;
	rfm_t *rfm;
	rfpe_t *pe, *pn;
	sdreq_t sdx;
	int rxgain;

	// if no SDR then timed or interactive rf output;

	if(g->sdr.sock < 0) {
		rf_pwr_timed(g);
		return(1);
	}

	Msg("measuring rf power ...\n");
	rfm = g->rfmc;
	if( !rfm)
		return(0);

	// prepare sdr transactions;

	sdx.rxb = rfpm_rxb;
	sdx.szrb = sizeof(rfpm_rxb);

	// sample for 1/2 sec;
	// noise of bladerf is weird for gain > 0db;
	// modem 24db - attenuator 20db - combiner 12db - window function max 3db;

	if( !rfm->sr)
		rfm->sr = SDR_SRATE;
	rxgain = SDR_RXGAIN;

	// send parameters to SDR;

	err = sdr_req(g, &sdx, "sr=%d\n", rfm->sr);
	if(err)
		goto out;
	err = sdr_req(g, &sdx, "bw=%d\n", rfm->bandwidth);
	if(err)
		goto out;
	err = sdr_req(g, &sdx, "f=%ld\n", rfm->uf);
	if(err)
		goto out;
	err = sdr_req(g, &sdx, "rxgain=%d\n", rxgain);
	if(err)
		goto out;
	err = sdr_req(g, &sdx, "ns=%d\n", rfm->sr / 2);
	if(err)
		goto out;

	// sample sdr receiver;

	err = sdr_req(g, &sdx, "sample\n");
	if(err)
		goto out;

	// compute power spectrum;

	err = sdr_req(g, &sdx, "comps %d hann\n", SDR_DFTSZ);
	if(err)
		goto out;

	// get avg power spectrum;

	err = sdr_req(g, &sdx, "getps avg\n");
	if(err)
		goto out;
	err = "bad data reply";
	if(sdx.ndata != (SDR_DFTSZ * sizeof(float)))
		goto out;

	// check power envelope;

	err = pe_check(g, rfm, sdx.data, SDR_DFTSZ);
	if(err)
		goto out;
	sts = "passed";
	if(rfm->dbin[2] > rfm->dbin[0]) {
		Msg("rf power check FAILED, %.1fdb/%ubin %.2f > max (%.2f)\n",
			rfm->dbin[1], rfm->nbin, rfm->dbin[2], rfm->dbin[0]);
		g->failed++;
		sts = "FAILED";
	}
	err = NULL;

	// save power spectrum into gnuplot file;

	err = plot_ps(g, rfm, sdx.data, SDR_DFTSZ, sts);
	if(err)
		goto out;

	// handle error cases;
out:
	if(err)
		Msg("error: %s\n", err);
	return(err? 0 : 1);
}

atc_t exe_rfpm = { rf_pwr_measure, "", 2000, 0, };

// reply 'OK' handler;
// assign ok target handler;

char *
reh_ok(g_t *g, atc_t *atc, rea_t *rea)
{
	return(NULL);
}

// reply 'ERROR' handler;

char *
reh_error(g_t *g, atc_t *atc, rea_t *rea)
{
	return("generic error");
}

// often used replies;

rea_t rea_ok = { REG_DEF, ".*^OK", reh_ok };
rea_t rea_error = { REG_DEF, ".*^ERROR", reh_error };

// AT commands;

// turn on/off tx PA;

atc_t at_dastxon = { 0, "!DASTXON", 1000, 0, { &rea_error, &rea_ok }, };
atc_t at_dastxoff = { 0, "!DASTXOFF", 1000, 0, { &rea_error, &rea_ok }, };

// set/enable wcdma modulation;

int
atc_dawstxcw(g_t *g, atc_t *atc)
{
	rfm_t *rfm = g->rfmc;
	smap_t *txmod;

	assert(rfm);
	txmod = rfm->txmod;
	assert(txmod);
	sprintf(atc->cmd, "!DAWSTXCW=%d", txmod->i);
	return(-1);
}

char buf_dawstxcw[32];
atc_t at_dawstxcw = { atc_dawstxcw, buf_dawstxcw, 1000, 0, { &rea_error, &rea_ok }, };

// set/enable wcdma pa power;

int
atc_dawstxpwr(g_t *g, atc_t *atc)
{
	sprintf(atc->cmd, "!DAWSTXPWR=1,%d", 23); //XXX
	return(-1);
}

char buf_dawstxpwr[32];
atc_t at_dawstxpwr = { atc_dawstxpwr, buf_dawstxpwr, 1000, 0, { &rea_error, &rea_ok }, };

// set wcdma pa range;

int
atc_dawsparange(g_t *g, atc_t *atc)
{
	sprintf(atc->cmd, "!DAWSPARANGE=%d", 3); //XXX
	return(-1);
}

char buf_dawsparange[32];
atc_t at_dawsparange = { atc_dawsparange, buf_dawsparange, 1000, 0, { &rea_error, &rea_ok }, };

// set NS value;

int
atc_dalsnsval(g_t *g, atc_t *atc)
{
	sprintf(atc->cmd, "!DALSNSVAL=%d", 1); //XXX
	return(-1);
}

char buf_dalsnsval[32];
atc_t at_dalsnsval = { atc_dalsnsval, buf_dalsnsval, 1000, 0, { &rea_error, &rea_ok }, };

// set waveform;

int
atc_dalswaveform(g_t *g, atc_t *atc)
{
	sprintf(atc->cmd, "!DALSWAVEFORM=%d", 0); //XXX
	return(-1);
}

char buf_dalswaveform[32];
atc_t at_dalswaveform = { atc_dalswaveform, buf_dalswaveform, 1000, 0, { &rea_error, &rea_ok }, };

// set tx mode;

int
atc_dalstxmod(g_t *g, atc_t *atc)
{
	rfm_t *rfm = g->rfmc;
	smap_t *txmod;

	assert(rfm);
	txmod = rfm->txmod;
	assert(txmod);
	sprintf(atc->cmd, "!DALSTXMOD=%d", txmod->i);
	return(-1);
}

char buf_dalstxmod[32];
atc_t at_dalstxmod = { atc_dalstxmod, buf_dalstxmod, 1000, 0, { &rea_error, &rea_ok }, };

// set channel;

int
atc_daschan(g_t *g, atc_t *atc)
{
	rfm_t *rfm = g->rfmc;

	assert(rfm);
	sprintf(atc->cmd, "!DASCHAN=%d", rfm->channel);
	return(-1);
}

char *
reh_daschan(g_t *g, atc_t *atc, rea_t *rea)
{
	rfm_t *rfm = g->rfmc;
	int argc;
	int channel;
	char *ep;

	assert(rfm);
	argc = resp_split(g);
	channel = strtol(g->argv[1], &ep, 0);
	if(channel == rfm->channel)
		return(NULL);

	Msg("real channel is %d\n", channel);

//XXX 
	return(NULL);
}

rea_t rea_daschan = { REG_DEF, ".*^[0-9]*.*^OK", reh_daschan, };
char buf_daschan[32];
atc_t at_daschan = { atc_daschan, buf_daschan, 2000, 1000, { &rea_error, &rea_daschan }, };

// set rx bandwidth;

int
atc_dalsrxbw(g_t *g, atc_t *atc)
{
	rfm_t *rfm;
	smap_t *bw;

	rfm = g->rfmc;
	assert(rfm);
	bw = rfm->bwidth;
	assert(bw);
	sprintf(atc->cmd, "!DALSRXBW=%d", bw->ir[0]);
	return(-1);
}

char buf_dalsrxbw[32];
atc_t at_dalsrxbw = { atc_dalsrxbw, buf_dalsrxbw, 1000, 0, { &rea_error, &rea_ok }, };

// set tx bandwidth;

int
atc_dalstxbw(g_t *g, atc_t *atc)
{
	rfm_t *rfm;
	smap_t *bw;

	rfm = g->rfmc;
	assert(rfm);
	bw = rfm->bwidth;
	assert(bw);
	sprintf(atc->cmd, "!DALSTXBW=%d", bw->ir[0]);
	return(-1);
}

char buf_dalstxbw[32];
atc_t at_dalstxbw = { atc_dalstxbw, buf_dalstxbw, 1000, 0, { &rea_error, &rea_ok }, };

// set band (and mode);

int
atc_dasband(g_t *g, atc_t *atc)
{
	rfm_t *rfm;
	smap_t *band;

	rfm = g->rfmc;
	assert(rfm);
	band = rfm->band;
	assert(band);
	sprintf(atc->cmd, "!DASBAND=%d", band->i);
	return(-1);
}

char *
reh_dasband(g_t *g, atc_t *atc, rea_t *rea)
{
	rfm_t *rfm;
	smap_t *band;
	int argc, num;
	char *ep;

	rfm = g->rfmc;
	assert(rfm);
	band = rfm->band;
	assert(band);

	argc = resp_split(g);
	if(argc < 2)
		return("DASBAND reply error");

	num = strtol(g->argv[1], &ep, 0);
	if(num != band->i)
		return("band not set");

	return(NULL);
}

rea_t rea_dasband = { REG_DEF, ".*[0-9].*^OK", reh_dasband, };
char buf_dasband[32];
atc_t at_dasband = { atc_dasband, buf_dasband, 1000, 1000, { &rea_error, &rea_dasband }, };

// put modem into factory test mode;

rea_t rea_daftmact = { REG_DEF, ".*290300.*^OK", };
atc_t at_daftmact = { 0, "!DAFTMACT" , 1000, 2000, { &rea_error, &rea_daftmact } };

// leave factory test mode;

atc_t at_daftmdeact = { 0, "!DAFTMDEACT" , 1000, 0, { &rea_error, &rea_ok } };

// reset modem;

atc_t at_reset = { 0, "!RESET" , 1000, 0, { &rea_error, &rea_ok } };

// enter password to enter factory test mode;
// password is SKU assigned by sierra wireless;

atc_t at_entercnd = { 0, "!ENTERCND=\"A710\"" , 1000, 0, { &rea_error, &rea_ok }, };

// get modem status;
// if in normal mode, then enter mfg mode;
// if in mfg mode already, then go on;

rea_t rea_gstatus_fact = { REG_DEF, ".*Mode.*FACTORY.*TEST.*^OK", };
rea_t rea_gstatus_norm = { REG_DEF, ".*Mode.*^OK", };
atc_t at_gstatus = { 0, "!GSTATUS?", 1000, 0, { &rea_error, &rea_gstatus_fact, &rea_gstatus_norm }, };

// get modem info;

char *
reh_i9(g_t *g, atc_t *atc, rea_t *rea)
{
	int argc;
	char **argv;
	char *s;

	// print model info;

	argc = resp_split(g);
	if(argc < 7)
		Fatal("not enough data from '%s'\n", atc->cmd);
	g->argv[7] = 0;
	for(argv = g->argv + 1; s = *argv++; )
		Msg("%s\n", s);

	// check supported model;

	s = g->argv[2] + 7;
	if( !strcmp(s, "EM7455"))
		g->model = MODEL_EM7455;
	else if( !strcmp(s, "EM7430"))
		g->model = MODEL_EM7430;
	else
		Fatal("unknown model: %s\n", s);

//XXX model specific bands;

	return(NULL);
}

rea_t rea_7430 = { REG_DEF, ".*Model: EM7430.*^OK", reh_i9 };
rea_t rea_7455 = { REG_DEF, ".*Model: EM7455.*^OK", reh_i9 };
atc_t at_i9 = { 0, "I9", 1000, 0, { &rea_error, &rea_7455, &rea_7430 }, };

// AT sequences;

// check model, enter mfg test mode;

ats_t ats_mode[] = {
	{ &at_i9, { 5, 1, 1 }, },
	{ &exe_mfg, { 1, 1 }, },
	{ &at_gstatus, { 3, 4, 1 }, },
	{ &at_entercnd, { 2, 1 }, },
	{ &at_daftmact, { 1, 2 }, },
	{ &at_reset, { -1, -1 }, },
	{ 0 },
};

// LTE AT command sequence;

ats_t ats_lte[] = {
	{ &exe_info, { 1, 1 }, },
	{ &at_dasband, { 9, 1 }, },
	{ &at_dalstxbw, { 8, 1 }, },
	{ &at_dalsrxbw, { 7, 1 }, },
	{ &at_daschan, { 6, 1 }, },
	{ &at_dalstxmod, { 5, 1 }, },
	{ &at_dalswaveform, { 4, 1 }, },
	{ &at_dalsnsval, { 3, 1 }, },
	{ &at_dastxon, { 2, 1 }, },
	{ &exe_rfpm, { 1, 1 }, },
	{ &at_dastxoff, { 1, 1 }, },
	{ 0 },
};

// GSM AT command sequence;

ats_t ats_gsm[] = {
	{ 0 },
};

// WCDMA AT command sequence;

ats_t ats_wcdma[] = {
	{ &exe_info, { 1, 1 }, },
	{ &at_dasband, { 7, 1 }, },
	{ &at_daschan, { 6, 1 }, },
	{ &at_dawsparange, { 5, 1 }, },
	{ &at_dastxon, { 4, 1 }, },
	{ &at_dawstxpwr, { 3, 1 }, },
	{ &at_dawstxcw, { 2, 1 }, },
	{ &exe_rfpm, { 1, 1 }, },
	{ &at_dastxoff, { 1, 1 }, },
	{ 0 },
};

// at list per technology;
// first entry is for unknown;

#define N_ATS(name) (sizeof(name) / sizeof(ats_t))

ats_t *tech_ats[] = { NULL, ats_gsm, ats_wcdma, ats_lte };
int tech_nats[] = { 0, N_ATS(ats_gsm), N_ATS(ats_wcdma), N_ATS(ats_lte) };

// post-test cleanup;

ats_t ats_end[] = {
	{ &exe_post, { 1, 1 }, },
	{ &at_daftmdeact, { 1, 1 }, },
	{ &at_reset, { -1, -1 }, },
	{ 0 },
};

// print usage;

void
usage(g_t *g, FILE *out)
{
	smap_t *map;

	fprintf(out, "usage: %s [options]\n", g->cmd);
	fprintf(out, "  -t tty      - tty for AT commands\n");
	fprintf(out, "  -f profile  - test profiles\n");
	fprintf(out, "  -o dur      - non-SDR test duration in sec (0=forever)\n");
	fprintf(out, "  -B host     - SDR host\n");
	fprintf(out, "  -N name     - name to put in plots\n");
	fprintf(out, "  -h          - print this help\n");

	smap_list(out, band_map, "bands");
	smap_list(out, bwidth_map, "bandwidths");
	smap_print(out, channel_map, "channels", SP_INT_RANGE);
	smap_list(out, txmod_map, "tx modulation");

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

// send command to tty;
// returns: 0 if ok, else errno;

int
tty_send(g_t *g, char *cmd)
{
	int nb, ret;
	char msg[100];

	nb = strlen(cmd);
	if(nb <= 0)
		return(0);
	if(nb >= sizeof(msg))
		return(EINVAL);

	nb = snprintf(msg, sizeof(msg), "AT%s\r\n", cmd);
	ret = write(g->tfd, msg, nb);

	if(g->debug & DBG_TTY) {
		msg[nb-2] = 0;
		printf("#> %d '%s'\n", nb, msg);
	}

	if(ret < 0)
		return(errno);
	if(ret != nb)
		return(EIO);

	return(0);
}

// reply timeout handler;

void
tty_rto(void *arg)
{
	atc_t *atc = arg;
	g_t *g = &G;

	Fatal("command timeout: %s [%dms]\n", atc->cmd, atc->rto);
}

// issue AT command;
// returns -1 issued, else compose handler rei;

int
tty_issue(g_t *g, atc_t *atc)
{
	int ret, i;
	rea_t *rea;

	assert(atc);
	g->nbuf = 0;

	// call handler to compose AT command, or run function;

	if(atc->comp) {
		ret = atc->comp(g, atc);
		if(ret >= 0)
			return(ret);
	}

	// compile regex if not already;

	for(i = 0; i < N_REX; i++) {
		rea = atc->re[i];
		if( !rea)
			break;
		if(rea->re)
			continue;

		rea->re = malloc(sizeof(regex_t));
		ret = regcomp(rea->re, rea->pat, rea->mflag);
		if(ret)
			Fatal("at regex '%s' for '%s'\n", rea->pat, atc->cmd);
	}

	// set reply timeout;

	timer_set(&g->rrt, atc->rto, tty_rto, atc);

	// send command;

	ret = tty_send(g, atc->cmd);
	if(ret)
		Fatal("sending %s\n", atc->cmd);

	return(-1);
}

// read tty messages;
// returns # of bytes in buffer available for parsing;

int
tty_read(g_t *g)
{
	int nb, left;

	// there can be partial data from previous read in buf[];
	// left=0 when buffer filled with garbage, such as with wrong baud rate;

	nb = g->nbuf;
	left = sizeof(g->buf) - 1 - nb;
	if(left <= 0)
		return(nb);

	// read() can return 0 for special tty control chars,
	// which we should never get, except when baudrate is wrong;

	nb = read(g->tfd, g->buf + nb, left);
	if(nb < 0)
		Fatal("reading from tty: %s\n", strerror(errno));
	nb += g->nbuf;
	g->nbuf = nb;
	g->buf[nb] = 0;
	return(nb);
}

// process tty receive data;
// returns -1 for stop, 0 for more data, else jump target for match;

int
tty_proc(g_t *g)
{
	int ret, rei, i;
	char *err;
	ats_t *ats;
	atc_t *atc;
	rea_t *rea;
	char msg[SZ_RBUF];

	// stop if no more at commands;

	ats = g->ats;
	if( !ats || !ats->atc) {
		g->stop = 1;
		return(-1);
	}

	// read chunk of tty data;

	ret = tty_read(g);
	if( !ret)
		return(0);

	// try to match expressions;
	// processed in order;

	atc = ats->atc;
	assert(atc);
	for(rei = -1, i = 0; i < N_REX; i++) {
		rea = atc->re[i];
		if( !rea)
			break;
		ret = regexec(rea->re, g->buf, 0, NULL, 0);
		if( !ret) {
			rei = i;	// found match;
			break;
		}
	}

	// rei<0 if no match, or unexpected reply;
	// wait for more data, or time out when no more coming;

	if(rei < 0)
		return(0);

	assert(rea);
	timer_cancel(&g->rrt);

	// found matching reply;

	if(g->debug & DBG_TTY)
		printf("#< '%s'\n", g->buf);

	// call reply handler, in case we need to look at it;

	err = NULL;
	if(rea->hdl)
		err = rea->hdl(g, atc, rea);
	if(err) {
		g->nerr++;
		Msg("error: at '%s' reply pattern '%s' reason '%s'\n", atc->cmd, rea->pat, err);
	}

	// finish at command;

	// wait minimal cmd delay;

	if(atc->delay)
		usleep(atc->delay * 1000);

	return(ats->rei[rei]);
}

// proceed to next target;
// returns -1 for stop;

int
ats_next(g_t *g, int tgt)
{
	atl_t *list;
	ats_t *ats;
	int ret;

	list = g->atl;
	assert(tgt > 0);

	// make sure to not jump outside sequence list;

again:
	ats = g->ats + tgt;
	if(ats > (list->ats + list->nats)) {
		Msg("error: at '%s' jump %d outside sequence\n", g->ats->atc->cmd, tgt);
		return(-1);
	}

	// advance to next list if end of this is reached;

	if( !ats->atc) {
		list++;
		g->atl = list;
		g->rfmc = list->rfm;
		ats = list->ats;
		if( !ats)
			return(-1);
	}
	g->ats = ats;
	if(ats) {
		ret = tty_issue(g, ats->atc);
		if(ret >= 0) {
			assert(ret < N_REX);
			tgt = ats->rei[ret];
			goto again;
		}
	}

	return(0);
}

// start an at sequence list;

void
ats_start(g_t *g, atl_t *list)
{
	ats_t *ats;

	if(list) {
		Msg("starting test sequence\n");
		ats = list->ats;
		g->rfmc = list->rfm;
		g->ats = ats;
		if(ats)
			tty_issue(g, ats->atc);
	} else {
		g->rfmc = NULL;
		g->ats = NULL;
	}
}

// init SDR;

int
sdr_init(g_t *g)
{
	int ret;
	struct hostent *svr;

	// nothing to do if SDR is not requested;

	if( !g->sdr.host)
		return(0);

	// open socket;

	g->sdr.sock = socket(PF_INET, SOCK_STREAM, 0);
	if(g->sdr.sock < 0)
		Fatal("cannot open socket: %s\n", strerror(errno));

	// lookup server;

	svr = gethostbyname(g->sdr.host);
	g->sdr.svr = svr;
	if( !svr)
		Fatal("no such host: %s\n", g->sdr.host);

	g->sdr.sa.sin_family = PF_INET;
	g->sdr.sa.sin_port = htons(PORT);
	bcopy(svr->h_addr, &g->sdr.sa.sin_addr.s_addr, svr->h_length);

        if(connect(g->sdr.sock, (struct sockaddr *)&g->sdr.sa, sizeof(g->sdr.sa)) < 0)
                Fatal("%s connect error: %s\n", g->sdr.host, strerror(errno));

	Msg("connected to SDR '%s'\n", g->sdr.host);
	return(0);
}

// skip over white space;

char *
skip_spc(char *s)
{
	while((*s == ' ') || (*s == '\t'))
		s++;
	return(s);
}

// save string;

char *
save_str(char *s)
{
	char *p;

	int len;
	if(s) {
		len = strlen(s);
		p = malloc(len + 1);
		if(p)
			strcpy(p, s);
		s = p;
	}
	return(s);
}

// check if valid channel;

int
channel_valid(smap_t *chan, int channel)
{
	return((channel >= chan->ir[0]) & (channel <= chan->ir[1]));
}

// profile commands;

typedef struct pfc pfc_t;
struct pfc {
	char *cmd;
	char *(*hdl)(g_t *, pfc_t *, char *);
	int flags;
};

// new rf power measurement;

char *
prof_new(g_t *g, pfc_t *pfc, char *s)
{
	rfm_t *rfm, *rfmc;

	rfm = malloc(sizeof(rfm_t));
	if( !rfm)
		return("out of memory");
	bzero(rfm, sizeof(*rfm));
	rfm->msg = save_str(s);
	rfm->dbin[0] = RFM_DB_BIN;
	Msg("profile '%s'\n", rfm->msg);

	rfmc = g->rfmc;
	if(rfmc)
		rfmc->next = rfm;
	else
		g->rfm = rfm;
	g->rfmc = rfm;

	return(NULL);
}

// set band;

char *
prof_band(g_t *g, pfc_t *pfc, char *s)
{
	rfm_t *rfm = g->rfmc;
	smap_t *band;

	if( !rfm)
		return("no rf measurement declared");
	if(rfm->band)
		return("redefinition of band");
	band = smap_lookup(band_map, s);
	if( !band)
		return("unknown band");
	rfm->band = band;

	return(NULL);
}

// set channel;

char *
prof_chan(g_t *g, pfc_t *pfc, char *s)
{
	rfm_t *rfm = g->rfmc;
	smap_t *chan;
	char *ep, *xp;
	long int channel;

	if( !rfm)
		return("no rf measurement declared");
	if( !rfm->band)
		return("no band defined yet");
	if(rfm->chan)
		return("redefinition of channel");

	chan = smap_channel(rfm->band);
	if( !chan)
		return("no channel list for band");

	if( !strcmp(s, "low"))
		channel = chan->ir[0];
	else if( !strcmp(s, "mid"))
		channel = (chan->ir[0] + chan->ir[1]) / 2;
	else if( !strcmp(s, "high"))
		channel = chan->ir[1];
	else {
		channel = strtol(s, &ep, 0);
		xp = skip_spc(ep);
		if((xp <= s) || *xp)
			return("invalid channel number");
		if( !channel_valid(chan, channel))
			return("channel not valid for band");
	}
	rfm->chan = chan;
	rfm->channel = channel;
	return(NULL);
}

// set sample rate;

char *
prof_srate(g_t *g, pfc_t *pfc, char *s)
{
	rfm_t *rfm = g->rfmc;
	unsigned sr;
	char *ep;

	if( !rfm)
		return("no rf measurement declared");
	sr = strtoul(s, &ep, 0);
	if((ep <= s) || *ep || !sr)
		return("invalid sample rate value");
	rfm->sr = sr;
	return(NULL);
}

// set bandwidth;
// signal occupied bandwidth;

char *
prof_bwidth(g_t *g, pfc_t *pfc, char *s)
{
	rfm_t *rfm = g->rfmc;
	smap_t *bwidth;
	int bw;
	char *ep;

	if( !rfm)
		return("no rf measurement declared");
	if( !rfm->band)
		return("no band defined yet");
	if(rfm->bwidth)
		return("redefinition of bandwidth");
	bwidth = smap_lookup(bwidth_map, s);
	rfm->bwidth = bwidth;
	if(bwidth)
		rfm->bandwidth = bwidth->ir[1];
	else {
		bw = strtol(s, &ep, 0);
		if((ep <= s) || *ep)
			return("invalid bandwidth value");
		rfm->bandwidth = bw;
	}
	return(NULL);
}

// set tx modulation;

char *
prof_txmod(g_t *g, pfc_t *pfc, char *s)
{
	rfm_t *rfm = g->rfmc;
	smap_t *band;
	smap_t *txmod;

	if( !rfm)
		return("no rf measurement declared");
	band = rfm->band;
	if( !band)
		return("no band defined yet");
	if(rfm->txmod)
		return("redefinition of txmod");
	txmod = smap_lookup(txmod_map, s);
	rfm->txmod = txmod;
	if( !txmod)
		return("invalid tx modulation");
	if( !(band->flags & txmod->flags))
		return("invalid txmod for band");

	return(NULL);
}

// set power envelope point;

char *
prof_pe(g_t *g, pfc_t *pfc, char *s)
{
	rfm_t *rfm = g->rfmc;
	rfpe_t *pe, **pn, **pq;
	int bandwidth;
	long int ofc;
	float r[2];
	char *ep, *xp;

	if( !rfm)
		return("no rf measurement declared");
	bandwidth = rfm->bandwidth;
	if(bandwidth <= 0)
		return("no bandwidth declared yet");

	// get offset, min, max;

	ofc = strtol(s, &ep, 0);
	xp = skip_spc(ep);
	if((ep <= s) || (xp <= ep))
		return("invalid frequency offset");
	if(abs(ofc) > bandwidth/2)
		return("point outside bandwidth");

	s = xp;
	r[0] = strtof(s, &ep);
	xp = skip_spc(ep);
	if((ep <= s) || (xp <= ep))
		return("invalid min dbm");

	s = xp;
	r[1] = strtof(s, &ep);
	xp = skip_spc(ep);
	if((ep <= s) || *xp)
		return("invalid max dbm");
	if(r[1] < r[0])
		return("invalid dbm range");

	// check if ofc is unique;
	// also holds queue position to add;
	// this adds envelope points sorted;

	for(pq = &rfm->pe, pn = pq; pe = *pq; pq = &pe->next) {
		if(pe->ofc == ofc)
			return("duplicate pwr envelope point");
		if(pe->ofc < ofc)
			pn = &pe->next;
	}

	// add point;

	pe = malloc(sizeof(rfpe_t));
	if( !pe)
		return("out of memory");
	pe->ofc = ofc;
	pe->r[0] = r[0];
	pe->r[1] = r[1];
	pe->next = *pn;
	*pn = pe;
	return(NULL);
}

// set max db/bin power limit;

char *
prof_dbin(g_t *g, pfc_t *pfc, char *s)
{
	rfm_t *rfm = g->rfmc;
	float dbin;
	char *ep;

	if( !rfm)
		return("no rf measurement declared");
	dbin = strtof(s, &ep);
	if((ep <= s) || *ep)
		return("invalid db/bin");
	rfm->dbin[0] = dbin;

	return(NULL);
}

// all profile commands;
// put longer commands first, due to sub-string matches;

pfc_t profile_cmds[] = {
	{ "new", prof_new, SM_ALL },
	{ "srate", prof_srate, SM_ALL },
	{ "bandwidth", prof_bwidth, SM_ALL },
	{ "band", prof_band, SM_ALL },
	{ "channel", prof_chan, SM_ALL },
	{ "txmod", prof_txmod, SM_ALL },
	{ "pe", prof_pe, SM_ALL },
	{ "dbin", prof_dbin, SM_ALL },
	{ 0 },
};

enum {
	PROF_NEW = 0,
	PROF_SRATE,
	PROF_BWIDTH,
	PROF_BAND,
	PROF_CHAN,
	PROF_TXMOD,
	PROF_PE,
	N_PROF,
};

// lookup profile command;

char *
prof_lookup(char *str, pfc_t **pfcp)
{
	char *cmd, *s;
	pfc_t *pfc;
	int n;

	for(pfc = profile_cmds; cmd = pfc->cmd; pfc++) {
		n = strlen(cmd);
		if(strncmp(cmd, str, n))
			continue;
		str += n;
		s = skip_spc(str);
		if(s > str) {
			*pfcp = pfc;
			return(s);
		}
	}
	return(NULL);
}

// fill rfm from option;
// fatal on errors;

void
rfm_opt(g_t *g, char *opt, int pfcoff, char *arg)
{
	pfc_t *pfc;
	rfm_t *rfm;
	smap_t *band;
	char *err;

	assert(pfcoff < N_PROF);
	pfc = profile_cmds + pfcoff;
	g->rfmo.msg = "cmdline";

	if( !pfc->hdl)
		Fatal("%s: '%s' not implemented\n", opt, pfc->cmd);

	// check if parameter is valid for band;

	rfm = g->rfmc;
	if(rfm) {
		band = rfm->band;
		if(band) {
			if( !(band->flags & pfc->flags))
				Fatal("%s: '%s' invalid for band '%s'\n", opt, pfc->cmd, band->str);
		}
	}

	// call handler to process arguments;

	err = pfc->hdl(g, pfc, arg);
	if(err)
		Fatal("%s: %s\n", opt, err);
}

// parse profile;

int
parse_profile(g_t *g)
{
	FILE *fp;
	char *s, *p;
	unsigned lnbr;
	rfm_t *rfm;
	pfc_t *pfc;
	smap_t *band;
	char line[256];

	// handle file;

	fp = fopen(g->profile, "r");
	if( !fp)
		Fatal("cannot open profile '%s'\n", g->profile);

	// read commands;

	lnbr = 0;
	while(fgets(line, sizeof(line) - 1, fp)) {
		lnbr++;
		s = skip_spc(line);
		if(*s == '#')
			continue;	// skip comments;
		if(*s == '\n')
			continue;	// blank line;
		p = strchr(s, '\n');
		if(p)
			*p = 0;		// remove \n;

		// lookup handler for parameter;

		s = prof_lookup(s, &pfc);
		if( !s)
			Fatal("%s:%u: unknown command '%s'\n", g->profile, lnbr, line);
		if( !pfc->hdl)
			Fatal("%s:%u: command '%s' not implemented\n", g->profile, lnbr, pfc->cmd);

		// check if parameter is valid for band;

		rfm = g->rfmc;
		if(rfm) {
			band = rfm->band;
			if(band) {
				if( !(band->flags & pfc->flags))
					Fatal("%s:%u: '%s' invalid for band '%s'\n", g->profile, lnbr, pfc->cmd, band->str);
			}
		}

		// call handler to process arguments;

		s = pfc->hdl(g, pfc, s);
		if(s)
			Fatal("%s:%u: %s: %s\n", g->profile, lnbr, pfc->cmd, s);
	}
	fclose(fp);
	return(0);
}

// validate rf measurement;
// also assigns defaults;

void
rfm_validate(g_t *g, rfm_t *rfm)
{
	smap_t *band;
	smap_t *chan;
	smap_t *freq;
	int flags;
	int64_t f;

	band = rfm->band;
	if( !band)
		Fatal("%s: no band specified\n", rfm->msg);
	flags = band->flags;
	if(flags & SM_GSM)
		rfm->tech = 1;
	else if(flags & SM_WCDMA)
		rfm->tech = 2;
	else if(flags & SM_LTE)
		rfm->tech = 3;
	else
		Fatal("%s: unknown band technology\n", rfm->msg);

	if(rfm->bandwidth <= 0)
		Fatal("%s: no bandwidth specified\n", rfm->msg);

	if(rfm->channel <= 0) {
		chan = smap_channel(rfm->band);
		if( !chan)
			Fatal("%s: no channel list for band '%s'\n", rfm->msg, band->str);
		rfm->chan = chan;
		rfm->channel = (chan->ir[0] + chan->ir[1]) / 2;
		Msg("%s: using '%s' mid-band channel %d\n", rfm->msg, band->str, rfm->channel);
	}
	freq = smap_freq(band, ufreq_map);
	rfm->ufreq = freq;
	if( !freq)
		Fatal("%s: no uplink frequency map for band '%s'\n", rfm->msg, band->str);
	if( !freq->freq)
		Fatal("%s: no uplink frequency handler for band '%s'\n", rfm->msg, band->str);
	f = freq->freq(rfm->channel);
	if(f <= 0)
		Fatal("%s: invalid frequency for band '%s' uarfcn '%d'\n", rfm->msg, band->str, rfm->channel);
	rfm->uf = f;

	freq = smap_freq(band, dfreq_map);
	rfm->dfreq = freq;
	if( !freq)
		Fatal("%s: no downlink frequency map for band '%s'\n", rfm->msg, band->str);
	if( !freq->freq)
		Fatal("%s: no downlink frequency handler for band '%s'\n", rfm->msg, band->str);
	f = freq->freq(rfm->channel);
//XXX valid F
	rfm->df = f;

	if( !rfm->txmod)
		Fatal("%s: no tx modulation specified\n", rfm->msg);
}

// main entry;

int
main(int argc, char **argv)
{
	g_t *g = &G;
	int c, ret, fd;
	int baud, nrfm;
	rfm_t *rfm;
	atl_t *atl;
	struct timeval tv;
	struct termios tios;
	char date[32];
	fd_set rdset, wrset;

	g->cmd = argv[0];
	g->sdr.sock = -1;

	// report startup time;

	ret = gettimeofday(&tv, NULL);
	if(ret)
		Fatal("cannot get time of day: %s\n", strerror(errno));
	strftime(date, sizeof(date), "%Y%m%d-%H%M%S", gmtime(&tv.tv_sec));
	Msg("starting: %s\n", date);

	// defaults;

	g->rfmc = &g->rfmo;
	g->vcid = "";

	// get options;

	while((c = getopt(argc, argv, "t:f:b:c:m:s:w:o:B:N:hd")) != EOF) {
		switch(c) {
		case 't':
			g->tty = optarg;
			break;
		case 'f':
			g->profile = optarg;
			break;
		case 'b':
			rfm_opt(g, "-b", PROF_BAND, optarg);
			break;
		case 'c':
			rfm_opt(g, "-c", PROF_CHAN, optarg);
			break;
		case 'm':
			rfm_opt(g, "-m", PROF_TXMOD, optarg);
			break;
		case 's':
			rfm_opt(g, "-s", PROF_SRATE, optarg);
			break;
		case 'w':
			rfm_opt(g, "-w", PROF_BWIDTH, optarg);
			break;
		case 'o':
			g->dur = strtol(optarg, NULL, 0);
			break;
		case 'B':
			g->sdr.host = optarg;
			break;
		case 'N':
			g->vcid = optarg;
			break;
		case 'd':
			g->debug |= DBG_TTY;
			break;
		case 'h':
			usage(g, stdout);
		case '?':
		default:
			usage(g, stderr);
		}
	}

	// if no cmdline measurement, remove it from list;

	if( !g->rfmo.msg)
		g->rfmc = NULL;
	g->rfm = g->rfmc;

	// parse test profile;
	// validate rf measurements;

	if(g->profile)
		parse_profile(g);
	nrfm = 0;
	for(rfm = g->rfm; rfm; rfm = rfm->next, nrfm++)
		rfm_validate(g, rfm);
	Msg("%d measurements to take\n", nrfm);

	// open SDR;

	sdr_init(g);

	// open tty;

	if( !g->tty)
		Fatal("no tty for AT commands\n");
	g->tfd = open(g->tty, O_RDWR);
	if(g->tfd < 0)
		Fatal("cannot open %s\n", g->tty);

	// get tty modes;
	// allows to inherit some tty config;

	ret = tcgetattr(g->tfd, &tios);
	if(ret < 0)
		Fatal("cannot get termio: %s\n", strerror(errno));

	// init tty modes;

	tios.c_iflag = 0;		// read BREAK as \0;
	tios.c_oflag = 0;		// nothing special;
	tios.c_cflag = 0
		| CS8			// 8 bits;
		| CREAD			// enable receiver;
		| CLOCAL;		// ignore modem controls;
	tios.c_lflag = 0
		| ICANON;		// canonical mode, buffer lines;
//XXX set c_cc

	// set speeds;

	baud = B115200;
	ret = cfsetospeed(&tios, baud);
	if(ret < 0)
		Fatal("cannot set output baud: %s\n", strerror(errno));
	ret = cfsetispeed(&tios, baud);
	if(ret < 0)
		Fatal("cannot set input baud: %s\n", strerror(errno));

	// set tty;
	// changes apply after output is drained,
	// at which point all unread input is discarded;

	ret = tcsetattr(g->tfd, TCSAFLUSH, &tios);
	if(ret < 0)
		Fatal("cannot set temio: %s\n", strerror(errno));

    	// setup single-shot request/reply timer;
	// first tick in one interval;
	// any failure is fatal;

	g->rrt.ts.it_interval.tv_sec = 0;
	g->rrt.ts.it_interval.tv_nsec = 0;

	g->rrt.fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
	if(g->rrt.fd < 0)
		Fatal("cannot create rr timer: %s\n", strerror(errno));

	// start AT command sequence;

	g->stop = 0;

	// determine command sequence, which depends on mode (gsm, wcdma, lte);

	atl = malloc((nrfm + 2 + 1) * sizeof(atl_t));
	if( !atl)
		Fatal("cannot allocate rfm at list\n");
	g->atl = atl;

	// build list of commands;
	// prolog, n tests ..., post cleanup;

	atl->rfm = NULL;
	atl->ats = ats_mode;
	atl->nats = sizeof(ats_mode) / sizeof(ats_t);
	atl++;

	for(rfm = g->rfm; rfm; rfm = rfm->next) {
		atl->rfm = rfm;
		assert(rfm);
		assert(rfm->tech);
		atl->ats = tech_ats[rfm->tech];
		atl->nats = tech_nats[rfm->tech];
		atl++;
	}

	atl->rfm = NULL;
	atl->ats = ats_end;
	atl->nats = sizeof(ats_end) / sizeof(ats_t);
	atl++;

	atl->ats = NULL;
	atl->nats = 0;

	ats_start(g, g->atl);

	// main loop;

	while( !g->stop) {
		// setup for select;

		FD_ZERO(&rdset);
		FD_ZERO(&wrset);

		fd = set_fd(g->tfd, &rdset, 0);
		fd = set_fd(g->rrt.fd, &rdset, fd);

		// max sleep to limit poll rate;

		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		ret = select(fd + 1, &rdset, &wrset, NULL, &tv);
		if(ret < 0) {
			if(errno == EINTR)
				continue;
			Fatal("select failed: %s\n", strerror(errno));
		}
		if(ret == 0)
			continue;

		// handle tty receive data;

		ret = 0;
		if(FD_ISSET(g->tfd, &rdset))
			ret = tty_proc(g);

		if(ret > 0)
			ret = ats_next(g, ret);
		if(ret < 0)
			break;

		// handle request/reply timeout;

		if(FD_ISSET(g->rrt.fd, &rdset))
			timer_hdl(&g->rrt);
	}

	// complain about failed tests;

	if(g->failed)
		Msg("warning: %d tests FAILED\n", g->failed);
	else
		Msg("all tests passed\n");

	// close fildes;

	if(g->sdr.sock >= 0)
		close(g->sdr.sock);
	close(g->rrt.fd);
	close(g->tfd);

	return(g->nerr + g->failed);
}

