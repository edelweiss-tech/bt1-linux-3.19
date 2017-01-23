#ifndef LYNXDRV_H_
#define LYNXDRV_H_

#define FB_ACCEL_SMI 0xab

#define MHZ(x) ((x) * 1000000)

#define DEFAULT_SM750_CHIP_CLOCK	290
#define DEFAULT_SM750LE_CHIP_CLOCK	333
#ifndef SM750LE_REVISION_ID
#define SM750LE_REVISION_ID ((unsigned char)0xfe)
#endif

enum sm750_pnltype {
	sm750_24TFT = 0,	/* 24bit tft */
	sm750_dualTFT = 2,	/* dual 18 bit tft */
	sm750_doubleTFT = 1,	/* 36 bit double pixel tft */
};

/* vga channel is not concerned  */
enum sm750_dataflow {
	sm750_simul_pri,	/* primary => all head */
	sm750_simul_sec,	/* secondary => all head */
	sm750_dual_normal,	/* primary => panel head and secondary => crt */
	sm750_dual_swap,	/* primary => crt head and secondary => panel */
};

enum sm750_channel {
	sm750_primary = 0,
	/* enum value equal to the register filed data */
	sm750_secondary = 1,
};

enum sm750_path {
	sm750_panel = 1,
	sm750_crt = 2,
	sm750_pnc = 3,	/* panel and crt */
};

struct init_status {
	ushort powerMode;
	/* below three clocks are in unit of MHZ*/
	ushort chip_clk;
	ushort mem_clk;
	ushort master_clk;
	ushort setAllEngOff;
	ushort resetMemory;
};

struct lynx_accel {
    volatile unsigned char __iomem *dprBase;	/* base virtual address of DPR registers */
    volatile unsigned char __iomem *dpPortBase;	/* base virtual address of de data port */
    void (*de_init)(struct lynx_accel *);	/* function pointers */
    int (*de_wait)(void);			/* see if hardware ready to work */
    int (*de_fillrect)(struct lynx_accel *, u32, u32, u32, u32, u32, u32, u32, u32, u32);
    int (*de_copyarea)(struct lynx_accel *, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32);
    int (*de_imageblit)(struct lynx_accel *, const char *, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32);
};

struct sm750_dev {
	/* common members */
	u16 devid;
	u8 revid;
	struct pci_dev *pdev;
	struct fb_info *fbinfo[2];
	struct lynx_accel accel;
	int accel_off;
	int fb_count;
	int mtrr_off;
	struct{
		int vram;
	} mtrr;
	/* all smi graphic adaptor got below attributes */
	unsigned long vidmem_start;
	unsigned long vidreg_start;
	__u32 vidmem_size;
	__u32 vidreg_size;
	void __iomem *pvReg;
	unsigned char __iomem *pvMem;
	/* locks*/
	spinlock_t slock;

	struct init_status initParm;
	enum sm750_pnltype pnltype;
	enum sm750_dataflow dataflow;
	int nocrt;

	/*
	 * 0: no hardware cursor
	 * 1: primary crtc hw cursor enabled,
	 * 2: secondary crtc hw cursor enabled
	 * 3: both ctrc hw cursor enabled
	 */
	int hwCursor;
};

struct lynx_cursor {
	/* cursor width ,height and size */
	int w;
	int h;
	int size;
	/* hardware limitation */
	int maxW;
	int maxH;
	/* base virtual address and offset  of cursor image */
	char __iomem *vstart;
	int offset;

	volatile char __iomem *mmio;	/* mmio addr of hw cursor */
};

struct lynxfb_crtc {
	unsigned char __iomem *vCursor; /* virtual address of cursor */
	unsigned char __iomem *vScreen; /* virtual address of on_screen */
	int oCursor; /* cursor address offset in vidmem */
	int oScreen; /* onscreen address offset in vidmem */
	int channel;/* which channel this crtc stands for*/
	resource_size_t vidmem_size;/* this view's video memory max size */

	/* below attributes belong to info->fix, their value depends on specific adaptor*/
	u16 line_pad;/* padding information:0,1,2,4,8,16,... */
	u16 xpanstep;
	u16 ypanstep;
	u16 ywrapstep;

	void *priv;

	struct lynx_cursor cursor;	/* cursor information */
};

struct lynxfb_output {
    int dpms;
    int paths;
    /* which paths(s) this output stands for,for sm750:
    paths=1:means output for panel paths
    paths=2:means output for crt paths
    paths=3:means output for both panel and crt paths
    */

    int *channel;
    /* which channel these outputs linked with,for sm750:
    channel=0 means primary channel
    channel=1 means secondary channel
    output->channel ==> &crtc->channel
    */
    void *priv;

    int (*proc_setBLANK)(struct lynxfb_output*, int);
};

struct lynxfb_par {
    /* either 0 or 1 for dual head adaptor,0 is the older one registered */
    int index;
    unsigned int pseudo_palette[256];
    struct lynxfb_crtc crtc;
    struct lynxfb_output output;
    struct fb_info *info;
    struct sm750_dev *dev;
};

static inline unsigned long ps_to_hz(unsigned int psvalue)
{
    unsigned long long numerator = 1000*1000*1000*1000ULL;
    /* 10^12/picosecond period gives frequency in Hz */
    do_div(numerator, psvalue);
    return (unsigned long)numerator;
}

int hw_map(struct sm750_dev *sm750_dev, struct pci_dev *pdev);
int hw_init_chip(struct sm750_dev*, struct pci_dev *);
void hw_init_accel(struct sm750_dev *);
int hw_deWait(void);
int hw_le_deWait(void);

int hw_output_setMode(struct lynxfb_output*, struct fb_var_screeninfo*, struct fb_fix_screeninfo*);
int hw_crtc_checkMode(struct lynxfb_crtc*, struct fb_var_screeninfo*);
int hw_crtc_setMode(struct lynxfb_crtc*, struct fb_var_screeninfo*, struct fb_fix_screeninfo*);
int hw_setColReg(struct lynxfb_crtc*, ushort, ushort, ushort, ushort);
int hw_setBLANK(struct lynxfb_output*, int);
int hw_le_setBLANK(struct lynxfb_output*, int);
int hw_pan_display(struct lynxfb_crtc *crtc, const struct fb_var_screeninfo *var, const struct fb_info *info);

#endif
