/**************************************************************************
                fb.c 0.2 Copyright (C) 2002, hito
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include "fb.h"

#define FB_ENV		"FRAMEBUFFER"
#define	FB_DEFDEV	"/dev/fb0"

#define FALSE 0
#define TRUE  1

static struct fb_cmap* fb_cmap_create(struct fb_fix_screeninfo*, struct fb_var_screeninfo *);
static void  fb_cmap_destroy(struct fb_cmap *cmap);
static int   fb_fscrn_get(int fbfp, struct fb_fix_screeninfo *scinfo);
static void *fb_mmap(int fbfp, struct fb_fix_screeninfo *scinfo);
static int   fb_munmap(void* buf,struct fb_fix_screeninfo *scinfo);
static int   fb_vscrn_get(int fbfp, struct fb_var_screeninfo *scinfo);

static struct fb_fix_screeninfo	fscinfo;
static struct fb_var_screeninfo	vscinfo;
static struct fb_cmap *cmap = NULL;
static int is_open = FALSE;
static int fbfp = -1;
static unsigned char *buf=NULL;

int fb_open(void){
  char *fbdev = {FB_DEFDEV};

  if(is_open == TRUE)
    return 1;

  if(getenv(FB_ENV)){
    fbdev=getenv(FB_ENV);
  }

  if((fbfp = open(fbdev,O_RDWR))==-1){
    fprintf(stderr, "open %s error\n",fbdev);
    goto ERR_END;
  }

  if(fb_fscrn_get(fbfp,&fscinfo)){
    goto ERR_END;
  }

  if(fb_vscrn_get(fbfp,&vscinfo)){
    goto ERR_END;
  }

  if((cmap = fb_cmap_create(&fscinfo,&vscinfo)) == (struct fb_cmap*) -1){
    goto ERR_END;
  }

  if(!(buf = fb_mmap(fbfp,&fscinfo))){
    fprintf(stderr, "Can't allocate memory.\n");
    goto ERR_END;
  }

  if(fscinfo.type!=FB_TYPE_PACKED_PIXELS){
    fprintf(stderr, "This type of framebuffer is not supported.\n");
    goto ERR_END;
  }

    /*
  if(fscinfo.visual == FB_VISUAL_PSEUDOCOLOR){
    printf("FB_VISUAL_PSEUDOCOLOR\n");
    if(vscinfo.bits_per_pixel!=8){
      fprintf(stderr, "未対応フレームバッファ\n");
      goto ERR_END;
    }

    if(fb_cmap_get(fbfp,cmap)){
      fprintf(stderr, "カラーマップ獲得失敗\n");
      //      fb_cmap_destroy(cmap);
      goto ERR_END;
    }
    //    fb_cmap_disp(cmap);

    if(cmap->len <(LINUX_LOGO_COLORS + LOGO_COLOR_OFFSET)){
      fprintf(stderr, "色の割付領域が不足しています\n");
      goto ERR_END;
    }

    cmap->start = LOGO_COLOR_OFFSET;
    cmap->len   = LINUX_LOGO_COLORS;

    for(lp = 0; lp < LINUX_LOGO_COLORS; lp++){
      if(cmap->red){
	*(cmap->red+lp) = (linux_logo_red[lp]<<CHAR_BIT)+linux_logo_red[lp];
      }
      if(cmap->green){
	*(cmap->green+lp)= (linux_logo_green[lp]<<CHAR_BIT)+linux_logo_green[lp];
      }
      if(cmap->blue){
	*(cmap->blue+lp)= (linux_logo_blue[lp]<<CHAR_BIT)+linux_logo_blue[lp];
      }
    }
    if(fb_cmap_set(fbfp,cmap)){
      fb_cmap_destroy(cmap);
      fprintf(stderr, "カラーマップ獲得失敗\n");
      goto ERR_END;
    }
  }
    */

  if(!(fscinfo.visual == FB_VISUAL_TRUECOLOR &&
     (vscinfo.bits_per_pixel == 15 ||
      vscinfo.bits_per_pixel == 16 ||
      vscinfo.bits_per_pixel == 24 ||
      vscinfo.bits_per_pixel == 32))){
    fprintf(stderr,"This type of framebuffer is not supported.\n");
    goto ERR_END;
  }

  is_open = TRUE;
  return 0;

 ERR_END:
  fb_close();
  return 1;
}

void fb_close(void)
{
  if(is_open != TRUE)
    return;

  if(cmap != NULL){
    fb_cmap_destroy(cmap);
    cmap = NULL;
  }
  if(buf != NULL){
    fb_munmap(buf,&fscinfo);
    buf = NULL;
  }

  if(fbfp >= 0){
    close(fbfp);
  }

  is_open = FALSE;
}

void fb_pset(int x, int y, int r, int g, int b)
{
  unsigned long work;
  int offset;
  static size_t size = 0;

  if(is_open != TRUE || x >= vscinfo.xres || y >= vscinfo.yres)
    return;

  if(size == 0)
    size = (vscinfo.bits_per_pixel + 7) / CHAR_BIT;

  offset = fscinfo.line_length * y + size * x;

  if(offset >= fscinfo.smem_len)
    return;

  work=
    ((r >> (CHAR_BIT - vscinfo.red.length  )) << vscinfo.red.offset)+
    ((g >> (CHAR_BIT - vscinfo.green.length)) << vscinfo.green.offset)+
    ((b >> (CHAR_BIT - vscinfo.blue.length )) << vscinfo.blue.offset);
  memcpy(buf + offset, &work, size);
}

int fb_get_color(int x, int y, int *r, int *g, int *b)
{
  unsigned long work = 0;
  int offset;
  static size_t size = 0;

  if(is_open != TRUE || x >= vscinfo.xres || y >= vscinfo.yres)
    return 1;

  if(size == 0)
    size = (vscinfo.bits_per_pixel + 7) / CHAR_BIT;

  offset = fscinfo.line_length * y + size * x;
  if(offset >= fscinfo.smem_len)
    return 1;

  memcpy(&work, buf + offset, size);

  *r = ((work >> vscinfo.red.offset) & (0x000000ff >> (CHAR_BIT - vscinfo.red.length)))
	      << (CHAR_BIT - vscinfo.red.length);
  *g = ((work >> vscinfo.green.offset) & (0x000000ff >> (CHAR_BIT - vscinfo.green.length)))
	      <<(CHAR_BIT - vscinfo.green.length);
  *b = ((work >> vscinfo.blue.offset)  & (0x000000ff >> (CHAR_BIT - vscinfo.blue.length)))
	      << (CHAR_BIT - vscinfo.blue.length);
  return 0;
}

void fb_clear(void)
{
  if(is_open != TRUE)
    return;

  memset(buf, 0, (vscinfo.xres * vscinfo.yres * vscinfo.bits_per_pixel) / CHAR_BIT);
}

int fb_width(void)
{
   if(is_open != TRUE)
    return 0;

   return vscinfo.xres;
}

int fb_height(void)
{
   if(is_open != TRUE)
    return 0;

   return vscinfo.yres;
}

void fb_cmap_disp(void)
{
  int lp;

  if(is_open != TRUE)
    return;

  printf("cmap DUMP\n");
  printf("start		:[%08x]\n", cmap->start);
  printf("len		:[%08x]\n", cmap->len);
  printf("red		:[%8p]\n", cmap->red);
  if(cmap->red){
    for(lp=0;lp<cmap->len;lp++){
      if((lp+1)%16==0) printf("%04x\n",*(cmap->red+lp));
      else printf("%04x ",*(cmap->red+lp));
    }
    if(lp%16) printf("\n");
  }
  printf("green		:[%8p]\n",cmap->green);
  if(cmap->green){
    for(lp=0;lp<cmap->len;lp++){
      if((lp+1)%16==0) printf("%04x\n",*(cmap->green+lp));
      else printf("%04x ",*(cmap->green+lp));
    }
    if(lp%16) printf("\n");
  }
  printf("blue		:[%8p]\n",cmap->blue);
  if(cmap->blue){
    for(lp=0;lp<cmap->len;lp++){
      if((lp+1)%16==0) printf("%04x\n",*(cmap->blue+lp));
      else printf("%04x ",*(cmap->blue+lp));
    }
    if(lp%16) printf("\n");
  }
  printf("transp		:[%8p]\n",cmap->transp);
  if(cmap->transp){
    for(lp=0;lp<cmap->len;lp++){
      if((lp+1)%16==0) printf("%04x\n",*(cmap->transp+lp));
      else printf("%04x ",*(cmap->transp+lp));
    }
    if(lp%16) printf("\n");
  }
  return;
}


void fb_fscrn_disp(void)
{
  if(is_open != TRUE)
    return;

  printf("scinfo[%8p] DUMP\n",      &fscinfo);
  printf("id		:[%s]\n",    fscinfo.id);
  printf("smem_start	:[%08lx]\n", fscinfo.smem_start);
  printf("smem_len	:[%d]\n",    fscinfo.smem_len);
  printf("type		:[%d] ",     fscinfo.type);
  switch(fscinfo.type){
  case FB_TYPE_PACKED_PIXELS:
    printf("FB_TYPE_PACKED_PIXELS\n");break;
  case FB_TYPE_PLANES:
    printf("FB_TYPE_PLANES\n");break;
  case FB_TYPE_INTERLEAVED_PLANES:
    printf("FB_TYPE_INTERLEAVED_PLANES\n");break;
  case FB_TYPE_TEXT:
    printf("FB_TYPE_TEXT\n");break;
  default:printf("Unknown type.\n");
  }
  printf("type_aux	:[%d] ",fscinfo.type_aux);
  switch(fscinfo.type_aux){
  case FB_AUX_TEXT_MDA:
    printf("FB_AUX_TEXT_MDA\n");break;
  case FB_AUX_TEXT_CGA:
    printf("FB_AUX_TEXT_CGA\n");break;
  case FB_AUX_TEXT_S3_MMIO:
    printf("FB_AUX_TEXT_S3_MMIO\n");break;
  case FB_AUX_TEXT_MGA_STEP16:
    printf("FB_AUX_TEXT_MGA_STEP16\n");break;
  case FB_AUX_TEXT_MGA_STEP8:
    printf("FB_AUX_TEXT_MGA_STEP8\n");break;
  default:printf("Unknown type_aux.\n");
  }
  printf("visual		:[%d] ",fscinfo.visual);
  switch(fscinfo.visual){
  case FB_VISUAL_MONO01:
    printf("FB_VISUAL_MONO01\n");break;
  case FB_VISUAL_MONO10:
    printf("FB_VISUAL_MONO10\n");break;
  case FB_VISUAL_TRUECOLOR:
    printf("FB_VISUAL_TRUECOLOR\n");break;
  case FB_VISUAL_PSEUDOCOLOR:
    printf("FB_VISUAL_PSEUDOCOLOR\n");break;
  case FB_VISUAL_DIRECTCOLOR:
    printf("FB_VISUAL_DIRECTCOLOR\n");break;
  case FB_VISUAL_STATIC_PSEUDOCOLOR:
    printf("FB_VISUAL_STATIC_PSEUDOCOLOR\n");break;
  default:printf("Unknown Visual mode.\n");
  }
  printf("xpanstep	:[%d]\n",fscinfo.xpanstep);
  printf("ypanstep	:[%d]\n",fscinfo.ypanstep);
  printf("ywrapstep	:[%d]\n",fscinfo.ywrapstep);
  printf("line_length	:[%d]\n",fscinfo.line_length);
  printf("mmio_start	:[%08lx]\n",fscinfo.mmio_start);
  printf("mmio_len	:[%d]\n",fscinfo.mmio_len);
  printf("accel		:[%d] ",fscinfo.accel);
  switch(fscinfo.accel){
  case FB_ACCEL_NONE:
    printf("FB_ACCEL_NONE\n");break;
  case FB_ACCEL_ATARIBLITT:
    printf("FB_ACCEL_ATARIBLITT\n");break;
  case FB_ACCEL_AMIGABLITT:
    printf("FB_ACCEL_AMIGABLITT\n");break;
  case FB_ACCEL_S3_TRIO64:
    printf("FB_ACCEL_S3_TRIO64\n");break;
  case FB_ACCEL_NCR_77C32BLT:
    printf("FB_ACCEL_NCR_77C32BLT\n");break;
  case FB_ACCEL_S3_VIRGE:
    printf("FB_ACCEL_S3_VIRGE\n");break;
  case FB_ACCEL_ATI_MACH64GX:
    printf("FB_ACCEL_ATI_MACH64GX\n");break;
  case FB_ACCEL_DEC_TGA:
    printf("FB_ACCEL_DEC_TGA\n");break;
  case FB_ACCEL_ATI_MACH64CT:
    printf("FB_ACCEL_ATI_MACH64CT\n");break;
  case FB_ACCEL_ATI_MACH64VT:
    printf("FB_ACCEL_ATI_MACH64VT\n");break;
  case FB_ACCEL_ATI_MACH64GT:
    printf("FB_ACCEL_ATI_MACH64GT\n");break;
  case FB_ACCEL_SUN_CREATOR:
    printf("FB_ACCEL_SUN_CREATOR\n");break;
  case FB_ACCEL_SUN_CGSIX:
    printf("FB_ACCEL_SUN_CGSIX\n");break;
  case FB_ACCEL_SUN_LEO:
    printf("FB_ACCEL_SUN_LEO\n");break;
  case FB_ACCEL_IMS_TWINTURBO:
    printf("FB_ACCEL_IMS_TWINTURBO\n");break;
  case FB_ACCEL_3DLABS_PERMEDIA2:
    printf("FB_ACCEL_3DLABS_PERMEDIA2\n");break;
  case FB_ACCEL_MATROX_MGA2064W:
    printf("FB_ACCEL_MATROX_MGA2064W\n");break;
  case FB_ACCEL_MATROX_MGA1064SG:
    printf("FB_ACCEL_MATROX_MGA1064SG\n");break;
  case FB_ACCEL_MATROX_MGA2164W:
    printf("FB_ACCEL_MATROX_MGA2164W\n");break;
  case FB_ACCEL_MATROX_MGA2164W_AGP:
    printf("FB_ACCEL_MATROX_MGA2164W_AGP\n");break;
  case FB_ACCEL_MATROX_MGAG100:
    printf("FB_ACCEL_MATROX_MGAG100\n");break;
  case FB_ACCEL_MATROX_MGAG200:
    printf("FB_ACCEL_MATROX_MGAG200\n");break;
  case FB_ACCEL_SUN_CG14:
    printf("FB_ACCEL_SUN_CG14\n");break;
  case FB_ACCEL_SUN_BWTWO:
    printf("FB_ACCEL_SUN_BWTWO\n");break;
  case FB_ACCEL_SUN_CGTHREE:
    printf("FB_ACCEL_SUN_CGTHREE\n");break;
  case FB_ACCEL_SUN_TCX:
    printf("FB_ACCEL_SUN_TCX\n");break;
  default:printf("Unknown Visual mode.\n");
  }
  return;
}

void fb_vscrn_disp(void)
{
  if(is_open != TRUE)
    return;
  printf("vscinfo DUMP\n");
  printf("xres		:[%d]\n",vscinfo.xres);
  printf("yres		:[%d]\n",vscinfo.yres);
  printf("xres_virtual	:[%d]\n",vscinfo.xres_virtual);
  printf("yres_virtual	:[%d]\n",vscinfo.yres_virtual);
  printf("xoffset		:[%d]\n",vscinfo.xoffset);
  printf("yoffset		:[%d]\n",vscinfo.yoffset);
  printf("bits_per_pixel	:[%d]\n",vscinfo.bits_per_pixel);
  printf("grayscale	:[%d]\n",vscinfo.grayscale);
  printf("red.offset	:[%d]\n",vscinfo.red.offset);
  printf("red.length	:[%d]\n",vscinfo.red.length);
  printf("red.msb_right	:[%d]\n",vscinfo.red.msb_right);
  printf("green.offset	:[%d]\n",vscinfo.green.offset);
  printf("green.length	:[%d]\n",vscinfo.green.length);
  printf("green.msb_right	:[%d]\n",vscinfo.green.msb_right);
  printf("blue.offset	:[%d]\n",vscinfo.blue.offset);
  printf("blue.length	:[%d]\n",vscinfo.blue.length);
  printf("blue.msb_right	:[%d]\n",vscinfo.blue.msb_right);
  printf("transp.offset	:[%d]\n",vscinfo.transp.offset);
  printf("transp.length	:[%d]\n",vscinfo.transp.length);
  printf("transp.msb_right:[%d]\n",vscinfo.transp.msb_right);
  printf("nonstd		:[%d]\n",vscinfo.nonstd);
  printf("activate	:[%d]\n",vscinfo.activate);
  printf("height		:[%d]\n",vscinfo.height);
  printf("width		:[%d]\n",vscinfo.width);
  printf("accel_flags	:[%d]\n",vscinfo.accel_flags);
  printf("pixclock	:[%d]\n",vscinfo.pixclock);
  printf("left_margin	:[%d]\n",vscinfo.left_margin);
  printf("right_margin	:[%d]\n",vscinfo.right_margin);
  printf("upper_margin	:[%d]\n",vscinfo.upper_margin);
  printf("lower_margin	:[%d]\n",vscinfo.lower_margin);
  printf("hsync_len	:[%d]\n",vscinfo.hsync_len);
  printf("vsync_len	:[%d]\n",vscinfo.vsync_len);
  printf("sync		:[%d]\n",vscinfo.sync);
  printf("vmode		:[%d]\n",vscinfo.vmode);
  return;
}

/********* static functions **************/

/*
	(struct fb_cmap)デバイスに依存しないカラーマップ情報

	fb_cmap_create()	新規のカラーマップ情報
	fb_cmap_destroy()	カラーマップ情報の破棄
	fb_cmap_disp()		情報の表示
	fb_cmap_get()		情報の獲得
	fb_cmap_set()		情報の設定
*/

#define	LUT_MAX		(256)

static struct fb_cmap* fb_cmap_create(struct fb_fix_screeninfo* fscinfo,
				struct fb_var_screeninfo *vscinfo)
{
	struct	fb_cmap*	cmap;
	int			cmaplen=LUT_MAX;

	/* カラーマップの存在チェック */
	if(fscinfo->visual==FB_VISUAL_MONO01 ||
		fscinfo->visual==FB_VISUAL_MONO10 ||
		fscinfo->visual==FB_VISUAL_TRUECOLOR) return NULL;

	cmap=(struct fb_cmap*)malloc(sizeof(struct fb_cmap));
	if(!cmap){
		perror("cmap malloc error\n");
		return (struct fb_cmap*)-1;
	}
	memset(cmap,0,sizeof(struct fb_cmap));

	/* 各分色が存在しそうだったらカラーマップ用の領域を確保 */
	if(vscinfo->red.length){
		cmap->red=(__u16*)malloc(sizeof(__u16)*cmaplen);
		if(!cmap->red){
			perror("red lut malloc error\n");
			return (struct fb_cmap*)-1;
		}
	}
	if(vscinfo->green.length){
		cmap->green=(__u16*)malloc(sizeof(__u16)*cmaplen);
		if(!cmap->green){
			if(vscinfo->red.length) free(cmap->red);
			perror("green lut malloc error\n");
			return (struct fb_cmap*)-1;
		}
	}
	if(vscinfo->blue.length){
		cmap->blue=(__u16*)malloc(sizeof(__u16)*cmaplen);
		if(!cmap->blue){
			if(vscinfo->red.length) free(cmap->red);
			if(vscinfo->green.length) free(cmap->green);
			perror("blue lut malloc error\n");
			return (struct fb_cmap*)-1;
		}
	}
	if(vscinfo->transp.length){
		cmap->transp=(__u16*)malloc(sizeof(__u16)*cmaplen);
		if(!cmap->transp){
			if(vscinfo->red.length) free(cmap->red);
			if(vscinfo->green.length) free(cmap->green);
			if(vscinfo->blue.length) free(cmap->blue);
			perror("transp lut malloc error\n");
			return (struct fb_cmap*)-1;
		}
	}
	cmap->len=cmaplen;
	return cmap;
}

static void fb_cmap_destroy(struct fb_cmap* cmap)
{
	if(cmap->red) free(cmap->red);
	if(cmap->green) free(cmap->green);
	if(cmap->blue) free(cmap->blue);
	if(cmap->transp) free(cmap->transp);
	free(cmap);
}

/*
static int fb_cmap_get(int fbfp,struct fb_cmap* cmap)
{
	if(ioctl(fbfp,FBIOGETCMAP,cmap)){
		perror("ioctl FBIOGETCMAP error\n");
		return -1;
	}
	return 0;
}

static int fb_cmap_set(int fbfp,struct fb_cmap* cmap)
{
	if(ioctl(fbfp,FBIOPUTCMAP,cmap)){
		perror("ioctl FBIOPUTCMAP error\n");
		return -1;
	}
	return 0;
}
*/
/*
	フレームバッファに対するアクセス

	fb_mmap()		フレームバッファのメモリ上へのマップ
	fb_munmap()		フレームバッファのメモリ上からのアンマップ
*/

static void *fb_mmap(int fbfp, struct fb_fix_screeninfo* scinfo)
{
  void	*buf;
  if((buf=(unsigned char*)
      mmap(NULL, scinfo->smem_len, PROT_READ|PROT_WRITE,MAP_SHARED, fbfp, (off_t)0))
     ==MAP_FAILED){
    perror("mmap error");
    return NULL;
  }
  return buf;
}

static int fb_munmap(void* buf,struct fb_fix_screeninfo* scinfo)
{
	return munmap(buf,scinfo->smem_len);
}

/*
	(struct fb_fix_screeninfo)デバイスに依存しない固定された情報

	fb_fscrn_disp()		情報の表示
	fb_fscrn_get()		情報の獲得
*/


static int fb_fscrn_get(int fbfp,struct fb_fix_screeninfo* scinfo)
{
	if(ioctl(fbfp,FBIOGET_FSCREENINFO,scinfo)){
		perror("ioctl FBIOGET_FSCREENINFO error\n");
		return -1;
	}
	return 0;
}

/*
	(struct fb_var_screeninfo)デバイスに依存しない変更可能な情報

	fb_vscrn_disp()		情報の表示
	fb_vscrn_get()		情報の獲得
	fb_vscrn_set()		情報の設定
*/


static int fb_vscrn_get(int fbfp,struct fb_var_screeninfo* scinfo)
{
	if(ioctl(fbfp,FBIOGET_VSCREENINFO,scinfo)){
		perror("ioctl FBIOGET_VSCREENINFO error\n");
		return -1;
	}
	return 0;
}

/*
static int fb_vscrn_set(int fbfp,struct fb_var_screeninfo* scinfo)
{
	if(ioctl(fbfp,FBIOPUT_VSCREENINFO,scinfo)){
		perror("ioctl FBIOPUT_VSCREENINFO error\n");
		return -1;
	}
	return 0;
}
*/
