/*
 *  ATI Frame Buffer Device Driver Core Definitions
 */

#define PLL_CRTC_DECODE 0

#define EINVAL -1

#define max(x,y) (x>=y)?x:y

#if CONFIG_CONSOLE_BTEXT
    /*
     *  Elements of the hardware specific atyfb_par structure
     */
struct crtc {
    u32 vxres;
    u32 vyres;
    u32 xoffset;
    u32 yoffset;
    u32 bpp;
    u32 h_tot_disp;
    u32 h_sync_strt_wid;
    u32 v_tot_disp;
    u32 v_sync_strt_wid;
    u32 off_pitch;
    u32 gen_cntl;
    u32 dp_pix_width;	/* acceleration */
    u32 dp_chain_mask;	/* acceleration */
};
#endif /* CONFIG_CONSOLE_BTEXT */

struct pll_ct {
    u8 pll_ref_div;
    u8 pll_gen_cntl;
    u8 mclk_fb_div;
    u8 mclk_fb_mult;    /* 2 or 4 */
    u8 sclk_fb_div;
    u8 pll_vclk_cntl;
    u8 vclk_post_div;
    u8 vclk_fb_div;
    u8 pll_ext_cntl;
    u8 spll_cntl2;
    u32 dsp_config;	/* Mach64 GTB DSP */
    u32 dsp_on_off;	/* Mach64 GTB DSP */
    u8 mclk_post_div_real;
    u8 xclk_post_div_real;
    u8 vclk_post_div_real;
};

union aty_pll {
    struct pll_ct ct;
};


    /*
     *  The hardware parameters for each card
     */
struct atyfb_par {
#if CONFIG_CONSOLE_BTEXT
    struct crtc crtc;
#endif
    union aty_pll pll;
    u32 accel_flags;
};

struct fb_info_aty {
#if CONFIG_CONSOLE_BTEXT
#if PLL_CRTC_DECODE==1
    struct fb_info fb_info;
#endif
#endif

    unsigned long ati_regbase;
    unsigned long frame_buffer;
    unsigned long clk_wr_offset;
    struct aty_cmap_regs *aty_cmap_regs;
    struct atyfb_par default_par;
#if PLL_CRTC_DECODE==1
    struct atyfb_par current_par;
#endif

    u32 features;
    u32 total_vram;
    u32 ref_clk_per;
    u32 pll_per;
    u32 mclk_per;
    u32 xclk_per;
    u8 bus_type;
    u8 ram_type;
    u8 mem_refresh_rate;
    u8 blitter_may_be_busy;
};


    /*
     *  ATI Mach64 features
     */

#define M64_HAS(feature)	((info)->features & (M64F_##feature))

#define M64F_RESET_3D		0x00000001
#define M64F_MAGIC_FIFO		0x00000002
#define M64F_GTB_DSP		0x00000004
#define M64F_FIFO_24		0x00000008
#define M64F_SDRAM_MAGIC_PLL	0x00000010
#define M64F_MAGIC_POSTDIV	0x00000020
#define M64F_INTEGRATED		0x00000040
#define M64F_CT_BUS		0x00000080
#define M64F_VT_BUS		0x00000100
#define M64F_MOBIL_BUS		0x00000200
#define M64F_GX			0x00000400
#define M64F_CT			0x00000800
#define M64F_VT			0x00001000
#define M64F_GT			0x00002000
#define M64F_MAGIC_VRAM_SIZE	0x00004000
#define M64F_G3_PB_1_1		0x00008000
#define M64F_G3_PB_1024x768	0x00010000
#define M64F_EXTRA_BRIGHT	0x00020000
#define M64F_LT_SLEEP		0x00040000
#define M64F_XL_DLL		0x00080000
#define M64F_MFB_TIMES_4	0x00100000


    /*
     *  Register access
     */

static inline u32 aty_ld_le32(int regindex,
			      const struct fb_info_aty *info)
{
    /* Hack for bloc 1, should be cleanly optimized by compiler */
    if (regindex >= 0x400)
	regindex -= 0x800;

#ifdef ATARI
    return in_le32((volatile u32 *)(info->ati_regbase+regindex));
#else
    return read32 (info->ati_regbase + regindex);
#endif
}

static inline void aty_st_le32(int regindex, u32 val,
			       const struct fb_info_aty *info)
{
    /* Hack for bloc 1, should be cleanly optimized by compiler */
    if (regindex >= 0x400)
	regindex -= 0x800;

#ifdef ATARI
    out_le32 (info->ati_regbase+regindex, val);
#else
    write32 (info->ati_regbase + regindex, val);
#endif
}

static inline u16 aty_ld_le16(int regindex,
			      const struct fb_info_aty *info)
{
    /* Hack for bloc 1, should be cleanly optimized by compiler */
    if (regindex >= 0x400)
    	regindex -= 0x800;

#if defined(__mc68000__)
    return le16_to_cpu(*((volatile u16 *)(info->ati_regbase+regindex)));
#else
    return read16 (info->ati_regbase + regindex);
#endif
}

static inline void aty_st_le16(int regindex, u16 val,
			       const struct fb_info_aty *info)
{
    /* Hack for bloc 1, should be cleanly optimized by compiler */
    if (regindex >= 0x400)
    	regindex -= 0x800;

#if defined(__mc68000__)
    *((volatile u16 *)(info->ati_regbase+regindex)) = cpu_to_le16(val);
#else
    write16 (info->ati_regbase + regindex, val);
#endif
}

static inline u8 aty_ld_8(int regindex,
			  const struct fb_info_aty *info)
{
    /* Hack for bloc 1, should be cleanly optimized by compiler */
    if (regindex >= 0x400)
	regindex -= 0x800;

#ifdef ATARI
    return in_8 (info->ati_regbase + regindex);
#else
    return read8 (info->ati_regbase + regindex);
#endif
}

static inline void aty_st_8(int regindex, u8 val,
			    const struct fb_info_aty *info)
{
    /* Hack for bloc 1, should be cleanly optimized by compiler */
    if (regindex >= 0x400)
	regindex -= 0x800;

#ifdef ATARI
    out_8 (info->ati_regbase + regindex, val);
#else
    write8 (info->ati_regbase + regindex, val);
#endif
}

static inline u8 aty_ld_pll(int offset, const struct fb_info_aty *info)
{
    u8 res;

    /* write addr byte */
    aty_st_8(CLOCK_CNTL + 1, (offset << 2), info);
    /* read the register value */
    res = aty_ld_8(CLOCK_CNTL + 2, info);
    return res;
}

/*
 * CT family only.
 */
static inline void aty_st_pll(int offset, u8 val,
			      const struct fb_info_aty *info)
{
    /* write addr byte */
    aty_st_8(CLOCK_CNTL + 1, (offset << 2) | PLL_WR_EN, info);
    /* write the register value */
    aty_st_8(CLOCK_CNTL + 2, val, info);
    aty_st_8(CLOCK_CNTL + 1, (offset << 2) & ~PLL_WR_EN, info);
}


static inline void wait_for_fifo(u16 entries, const struct fb_info_aty *info)
{
    while ((aty_ld_le32(FIFO_STAT, info) & 0xffff) >
	   ((u32)(0x8000 >> entries)));
}

static inline void wait_for_idle(struct fb_info_aty *info)
{
    wait_for_fifo(16, info);
    while ((aty_ld_le32(GUI_STAT, info) & 1)!= 0);
    info->blitter_may_be_busy = 0;
}
