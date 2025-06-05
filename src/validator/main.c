#include "validator.h"

static int validate_path(const char *path);
static int color_search(uint32_t color);

struct g g={0};

/* Validate image or tile.
 * (tileid<0) if full image, for logging only.
 */
 
static int validate_sub_image(const uint8_t *src,int w,int h,int stride,const char *path,int tileid) {
  char sfx[6];
  if ((tileid>=0)&&(tileid<=0xff)) {
    sfx[0]=':';
    sfx[1]='0';
    sfx[2]='x';
    sfx[3]="0123456789abcdef"[tileid>>4];
    sfx[4]="0123456789abcdef"[tileid&15];
    sfx[5]=0;
  } else {
    sfx[0]=0;
  }
  uint32_t foundv[64]; // There should only be 4, but allow some head room so we can track some of the extras.
  int y=0,result=0,foundc=0,totalc=0;
  for (;y<h;y++,src+=stride) {
    const uint8_t *p=src;
    int x=0;
    for (;x<w;x++,p+=4) {
      uint32_t color=0;
      if (p[3]) { // transparent colors stay full zero and don't check the palette
        if (p[3]!=0xff) {
          fprintf(stderr,"%s%s@%d,%d: Intermediate alpha 0x%02x\n",path,sfx,x,y,p[3]);
          result=-2;
        }
        color=(p[0]<<24)|(p[1]<<16)|(p[2]<<8)|0xff;
        if (g.colorc) {
          if (color_search(color)<0) {
            fprintf(stderr,"%s%s@%d,%d: Color 0x%08x not in the global palette.\n",path,sfx,x,y,color);
            result=-2;
          }
        }
      }
      int fi=foundc,found=0;
      while (fi-->0) {
        if (foundv[fi]==color) {
          found=1;
          break;
        }
      }
      if (!found) {
        totalc++;
        if (foundc<64) foundv[foundc++]=color;
      }
    }
  }
  if (totalc>4) {
    fprintf(stderr,"%s%s: Too many colors (%d)\n",path,sfx,totalc);
    result=-2;
  }
  return result;
}

/* Validate decoded image.
 */
 
static int validate_image(struct image *image,const char *path) {
  if (image->pixelsize!=32) {
    fprintf(stderr,"%s: Images must be 32-bit RGBA. I know, I know, i8 would make more sense, but Egg currently doesn't read those.\n",path);
    return -2;
  }
  if (image->stride!=image->w<<2) {
    fprintf(stderr,"%s: Expected minimal stride.\n",path);
    return -2;
  }
  int result=0;
  if ((image->w==image->h)&&(image->w>16)&&!(image->w&15)) {
    fprintf(stderr,"%s: Looks like a tilesheet (%dx%d)\n",path,image->w,image->h);
    int tilesize=image->w>>4;
    int longstride=image->stride*tilesize;
    int shortstride=tilesize<<2;
    const uint8_t *rowp=image->v;
    int yi=16,tileid=0;
    for (;yi-->0;rowp+=longstride) {
      const uint8_t *colp=rowp;
      int xi=16;
      for (;xi-->0;colp+=shortstride,tileid++) {
        if (validate_sub_image(colp,tilesize,tilesize,image->stride,path,tileid)<0) result=-2;
      }
    }
  } else {
    fprintf(stderr,"%s: Generic image (%dx%d)\n",path,image->w,image->h);
    if (validate_sub_image(image->v,image->w,image->h,image->stride,path,-1)<0) result=-2;
  }
  return result;
}

/* Read PNG file, decode it, validate colors.
 */
 
static int validate_png_path(const char *path) {
  void *src=0;
  int srcc=file_read(&src,path);
  if (srcc<0) {
    fprintf(stderr,"%s: Failed to read file.\n",path);
    return -2;
  }
  struct image *image=image_decode(src,srcc);
  free(src);
  if (!image) {
    fprintf(stderr,"%s: Failed to decode image.\n",path);
    return 0;
  }
  int err=validate_image(image,path);
  image_del(image);
  return err;
}

/* Validate a file or directory.
 */
 
static int validate_dir_member(const char *path,const char *base,char ftype,void *userdata) {
  if (!ftype) ftype=file_get_type(path);
  if (ftype=='d') {
    return dir_read(path,validate_dir_member,0);
  } else if (ftype=='f') {
    int basec=0;
    while (base[basec]) basec++;
    if ((basec>7)&&!memcmp(base+basec-7,".a1.png",7)) {
      fprintf(stderr,"%s: Ignoring 1-bit image, presumably used only by font.\n",path);
      return 0;
    }
    if ((basec>4)&&!memcmp(base+basec-4,".png",4)) {
      return validate_png_path(path);
    }
    fprintf(stderr,"%s: Skipping file (we only do '*.png').\n",path);
  }
  return 0;
}
 
static int validate_path(const char *path) {
  char ftype=file_get_type(path);
  if (ftype=='d') {
    return dir_read(path,validate_dir_member,0);
  } else if (ftype=='f') {
    return validate_png_path(path);
  } else {
    fprintf(stderr,"%s: Unexpected file type or file not found.\n",path);
    return -2;
  }
}

/* Add color to the global palette.
 */
 
static int color_search(uint32_t color) {
  int lo=0,hi=g.colorc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    uint32_t q=g.colorv[ck];
         if (color<q) hi=ck;
    else if (color>q) lo=ck+1;
    else return ck;
  }
  return -lo-1;
}
 
static int add_color(uint32_t color) {
  int p=color_search(color);
  if (p>=0) return 0;
  p=-p-1;
  if (g.colorc>=64) {
    fprintf(stderr,"Too many colors.\n");
    return -1;
  }
  memmove(g.colorv+p+1,g.colorv+p,sizeof(uint32_t)*(g.colorc-p));
  g.colorc++;
  g.colorv[p]=color;
  return 0;
}

/* Main.
 */
 
int main(int argc,char **argv) {

  int argi=1;
  while (argi<argc) {
    const char *arg=argv[argi++];
    if (!arg||!arg[0]) continue;
    
    if (!memcmp(arg,"--palette=",10)) {
      g.palpath=arg+10;
      continue;
    }
    
    if (arg[0]=='-') {
      fprintf(stderr,"%s: Unexpected argument '%s'\n",argv[0],arg);
      return 1;
    }
    
    if (g.srcpathc>=g.srcpatha) {
      int na=g.srcpatha+8;
      if (na>INT_MAX/sizeof(void*)) return 1;
      void *nv=realloc(g.srcpathv,sizeof(void*)*na);
      if (!nv) return 1;
      g.srcpathv=nv;
      g.srcpatha=na;
    }
    g.srcpathv[g.srcpathc++]=arg;
  }
  if (!g.srcpathc) {
    fprintf(stderr,"Usage: %s [--palette=PNG] DIR_OR_PNG...\n",argv[0]);
    return 1;
  }
  
  /* Load the palette if requested.
   */
  if (g.palpath) {
    void *src=0;
    int srcc=file_read(&src,g.palpath);
    if (srcc<0) {
      fprintf(stderr,"%s: Failed to read file.\n",g.palpath);
      return 1;
    }
    struct image *image=image_decode(src,srcc);
    free(src);
    if (!image) {
      fprintf(stderr,"%s: Failed to decode image.\n",g.palpath);
      return 1;
    }
    if (image->pixelsize!=32) {
      fprintf(stderr,"%s: Expected 32-bit pixels for palette, found %d.\n",g.palpath,image->pixelsize);
      return 1;
    }
    // Examine each pixel of (image). Alpha must be zero or one. Ignore zero alphas. Add the rest to (g.colorv).
    // Decoded images will always have minimal stride, so it's OK to read them as 1-dimensional.
    uint8_t *p=image->v;
    int i=image->w*image->h;
    for (;i-->0;p+=4) {
      if (!p[3]) continue; // alpha zero
      if (p[3]!=0xff) {
        fprintf(stderr,"%s: Palette contains intermediate alpha (0x%02x%02x%02x%02x).\n",g.palpath,p[0],p[1],p[2],p[3]);
        return 1;
      }
      if (add_color((p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3])<0) return 1;
    }
    image_del(image);
    if (!g.colorc) {
      fprintf(stderr,"%s: Palette is empty.\n",g.palpath);
      return 1;
    }
    fprintf(stderr,"%s: Acquired %d-color palette.\n",g.palpath,g.colorc);
  }
  
  /* Enter each directory or image.
   */
  int result=0;
  int i=0; for (;i<g.srcpathc;i++) {
    const char *path=g.srcpathv[i];
    int err=validate_path(path);
    if (err<0) {
      fprintf(stderr,"%s: Validation failed.\n",path);
      result=1;
    }
  }
  
  return result;
}
