#include <assert.h>
#include "texture.h"
#include "picimage.h"
#include <cmath>

iil4mitkPicImage::iil4mitkPicImage (unsigned int size)
        : iil4mitkImage(size), _pic (NULL), _min (0.0), _max (0.0), _colors (NULL), _binary (false), _mask (false), _outline(false), _outlineWidth(1.0)
{
}

iil4mitkPicImage::~iil4mitkPicImage ()
{
}

void
iil4mitkPicImage::setImage (mitkIpPicDescriptor* pic, int model)
{
    assert (pic);
    assert (pic->dim >= 2);

    _pic = pic;
    iil4mitkImage::setImage (pic->n[0], pic->n[1], model, (unsigned char *) pic->data);
}

mitkIpPicDescriptor*
iil4mitkPicImage::image () const
{
	return _pic;
}

void
iil4mitkPicImage::setExtrema (const float minimum, const float maximum)
{
    // assert (minimum < maximum);

    _min = minimum;
    _max = maximum;
    invalidateTextures ();
}

void
iil4mitkPicImage::setWindow (const float level, const float window)
{
    // assert (window > 0);
    _min = level - window / 2.0;
    _max = _min + window;
    invalidateTextures ();
}

float
iil4mitkPicImage::minimum () const
{
    return _min;
}

float
iil4mitkPicImage::maximum () const
{
    return _max;
}

float
iil4mitkPicImage::level () const
{
    return (_min + _max) / 2.0;
}

float
iil4mitkPicImage::window () const
{
    return (_max - _min);
}

void
iil4mitkPicImage::setColors (const unsigned char* colors)
{
    _colors = colors;
}

const unsigned char*
iil4mitkPicImage::colors () const
{
    return _colors;
}

void 
iil4mitkPicImage::setBinary (const bool on)
{
	if (_binary != on) {
		if (on) {
			_mask = false;
		}
		_binary = on;
		invalidateTextures ();
	}
}

bool 
iil4mitkPicImage::binary () const
{
	return _binary;
}

void 
iil4mitkPicImage::setOutline (const bool on)
{
	if (_binary) {
		_outline = on;
	}
}

void 
iil4mitkPicImage::setOutlineWidth(float width)
{
	if (_binary) {
		_outlineWidth = width;
	}
}


bool 
iil4mitkPicImage::outline () const
{
	return _outline;
}

void 
iil4mitkPicImage::setMask (const bool on)
{
	if (_mask != on) {
		if (on) {
			_binary = false;
		}
		_mask = on;
		invalidateTextures ();
	}
}

bool 
iil4mitkPicImage::mask () const
{
	return _mask;
}

void
iil4mitkPicImage::clear ()
{
    _pic = NULL;
    _min = _max = 0.0;
    _colors = NULL;
    _binary = false;
    _mask = false; 
    _outline = false;
    iil4mitkImage::clear ();
}

iil4mitkPicImage* 
iil4mitkPicImage::find (const iil4mitkItem* item) 
{
    iil4mitkPicImage* result = NULL;

    if (!item) return NULL;

    if ( dynamic_cast<const iil4mitkPicImage*>(item)!=NULL ) {
        result = const_cast<iil4mitkPicImage*>(dynamic_cast<const iil4mitkPicImage*>(item));
    }
    return result;
}

#define MASK_INTENSITIES(TYPE, PIC)					\
{									\
TYPE* source = (TYPE *) src;						\
unsigned char* dest = dst;						\
register float a;							\
if (model () == INTENSITY) 						\
while (dest < eol) {							\
a = source [0] * scale - bias;						\
*dest = (a > 255.0 ? 0 : (a < 0.0 ? 0 : 255));				\
source++;								\
dest++;									\
} else									\
if (model () == INTENSITY_ALPHA)		 			\
while (dest < eol) {							\
a = source [0] * scale - bias;						\
dest [1] = dest [0] = (a > 255.0 ? 0 : (a < 0.0 ? 0 : 255));		\
source++;								\
dest += 2;								\
}  									\
}

#define BINARY_INTENSITIES(TYPE, PIC)					\
{									\
TYPE* source = (TYPE *) src;						\
unsigned char* dest = dst;						\
register float a;							\
if (model () == INTENSITY) 						\
while (dest < eol) {							\
a = source [0] * scale - bias;						\
*dest = (a > 255.0 ? 255 : (a < 0.0 ? 0 : 255));			\
source++;								\
dest++;									\
} else									\
if (model () == INTENSITY_ALPHA)		 			\
while (dest < eol) {							\
a = source [0] * scale - bias;						\
dest [0] = (a > 255.0 ? 255 : (a < 0.0 ? 0 : 255));			\
dest [1] = (a > 0.0 ? 255 : 0);						\
source++;								\
dest += 2;								\
}  									\
}

#define LIMIT_INTENSITIES(TYPE, PIC)					\
{									\
TYPE* source = (TYPE *) src;						\
unsigned char* dest = dst;						\
register unsigned int i; 						\
register float a,b,c,d;							\
if (model () == INTENSITY) 						\
while (dest < eol) {							\
a = source [0] * scale - bias;						\
*dest = (a > 255.0 ? 255 : (a < 0.0 ? 0 : (unsigned char) a));		\
source++;								\
dest++;									\
} else									\
if (model () == INTENSITY_ALPHA)		 			\
while (dest < eol) {							\
a = source [0] * scale - bias;						\
dest [0] = (a > 255.0 ? 255 : (a < 0.0 ? 0 : (unsigned char) a));	\
dest [1] = (a > 0.0 ? 255 : 0);						\
source++;								\
dest += 2;								\
} else 									\
if ((model () == COLOR) && _colors)					\
while (dest < eol) {							\
a = source [0] * scale - bias;						\
i = (a > 255.0 ? 255 : (a < 0.0 ? 0 : (unsigned char) a));		\
dest [0] = _colors [i*3];						\
dest [1] = _colors [i*3+1];						\
dest [2] = _colors [i*3+2];						\
source++;								\
dest += 3;								\
} else									\
if ((model () == COLOR_ALPHA) && _colors)					\
while (dest < eol) {							\
a = source [0] * scale - bias;						\
i = (a > 255.0 ? 255 : (a < 0.0 ? 0 : (unsigned char) a));		\
dest [0] = _colors [i*4  ];						\
dest [1] = _colors [i*4+1];						\
dest [2] = _colors [i*4+2];						\
dest [3] = _colors [i*4+3];						\
source++;								\
dest += 4;								\
} else									\
if ((model () == COLOR) && !_colors)					\
while (dest < eol) {							\
a = source [0] * scale - bias;						\
b = source [slice] * scale - bias;					\
c = source [2*slice] * scale - bias;					\
dest [0] = (a > 255.0 ? 255 : (a < 0.0 ? 0 : (unsigned char) a));	\
dest [1] = (b > 255.0 ? 255 : (b < 0.0 ? 0 : (unsigned char) b));	\
dest [2] = (c > 255.0 ? 255 : (c < 0.0 ? 0 : (unsigned char) c));	\
source++;								\
dest += 3;								\
}									\
if ((model () == COLOR_ALPHA) && !_colors)				\
{									\
a = source [0] * scale - bias;						\
b = source [slice] * scale - bias;					\
c = source [2*slice] * scale - bias;					\
d = source [3*slice] * scale - bias;					\
dest [0] = (a > 255.0 ? 255 : (a < 0.0 ? 0 : (unsigned char) a));	\
dest [1] = (b > 255.0 ? 255 : (b < 0.0 ? 0 : (unsigned char) b));	\
dest [2] = (c > 255.0 ? 255 : (c < 0.0 ? 0 : (unsigned char) c));	\
dest [3] = (d > 255.0 ? 255 : (d < 0.0 ? 0 : (unsigned char) d));	\
source++;								\
dest += 4;								\
}									\
}									

// prototype
#ifdef USE_MMX
#define MAXSHORT 32767
static void extrema (unsigned char* dst, short* src, const unsigned int num, const short minimum, const short maximum)
{
	const short low = -1024;
	const short up = MAXSHORT - (maximum - low);
	const short down = (minimum - low) + up;
	const short scale = (255.0f/(float) (maximum-minimum)) * 256; 
	const short values [] = {
		low,	// lowest value
		low,
		low,
		low,
		up, 	// saturate above
		up,
		up,
		up,
		down,	// saturate below
		down,
		down,
		down,
		scale,	// scale 				
		scale,
		scale,
		scale
	};
	const unsigned int n = num / 16;

		__asm__ __volatile__ (
			".align 16			\n\t"
			"l1:				\n\t"
			"prefetcht0 64(%1)		\n\t" 

			"movq (%1),%%mm0    		\n\t" 
			"movq 8(%1),%%mm1		\n\t"
			"psubsw (%2),%%mm0		\n\t"
			"psubsw (%2),%%mm1		\n\t"
			"paddsw 8(%2),%%mm0		\n\t"
			"paddsw 8(%2),%%mm1		\n\t" 
			"psubusw 16(%2),%%mm0		\n\t"
			"psubusw 16(%2),%%mm1		\n\t"
			"pmullw 24(%2),%%mm0		\n\t" 
			"pmullw 24(%2),%%mm1		\n\t" 
			"psrlw $8,%%mm0			\n\t"
			"psrlw $8,%%mm1			\n\t" 
			"packuswb %%mm1,%%mm0		\n\t"
			"movntq %%mm0,(%0)  		\n\t"

			"movq 16(%1),%%mm2	  	\n\t" 
			"movq 24(%1),%%mm3		\n\t"
			"psubsw (%2),%%mm2		\n\t"
			"psubsw (%2),%%mm3		\n\t"
			"paddsw 8(%2),%%mm2		\n\t" 
			"paddsw 8(%2),%%mm3		\n\t" 
			"psubusw 16(%2),%%mm2		\n\t"
			"psubusw 16(%2),%%mm3		\n\t"
			"pmullw 24(%2),%%mm2		\n\t"
			"pmullw 24(%2),%%mm3		\n\t" 
			"psrlw $8,%%mm2			\n\t"
			"psrlw $8,%%mm3			\n\t" 
			"packuswb %%mm3,%%mm2		\n\t"
			"movntq %%mm2,8(%0)		\n\t" 
			
			"emms				\n\t"
			"add $16,%0			\n\t"
			"add $32,%1			\n\t"
			"dec %3				\n\t"
			"jnz l1				\n\t"
			"emms				\n\t"
			:
			:"r" (dst), "r" (src), "r" (values), "r" (n)
		);
}
#endif

#ifndef PIC_IMAGE_PI
   #define PIC_IMAGE_PI 3.141592653589
#endif

// Convert color pixels from (R,G,B) to (H,S,I).
// Reference: "Digital Image Processing, 2nd. edition", R. Gonzalez and R. Woods. Prentice Hall, 2002.
template<class T>
void RGBtoHSI(T* RGB, T* HSI) {
  T R = RGB[0],
    G = RGB[1],
    B = RGB[2],
    nR = (R<0?0:(R>255?255:R))/255,
    nG = (G<0?0:(G>255?255:G))/255,
    nB = (B<0?0:(B>255?255:B))/255,
    m = nR<nG?(nR<nB?nR:nB):(nG<nB?nG:nB),
    theta = (T)(std::acos(0.5f*((nR-nG)+(nR-nB))/std::sqrt(std::pow(nR-nG,2)+(nR-nB)*(nG-nB)))*180/PIC_IMAGE_PI),
    sum = nR + nG + nB;
  T H = 0, S = 0, I = 0;
  if (theta>0) H = (nB<=nG)?theta:360-theta;
  if (sum>0) S = 1 - 3/sum*m;
  I = sum/3;
  HSI[0] = (T)H;
  HSI[1] = (T)S;
  HSI[2] = (T)I;
}

// Convert color pixels from (H,S,I) to (R,G,B).
template<class T>
void HSItoRGB(T* HSI, T* RGB) {
  T H = (T)HSI[0],
    S = (T)HSI[1],
    I = (T)HSI[2],
    a = I*(1-S),
    R = 0, G = 0, B = 0;
  if (H<120) {
    B = a;
    R = (T)(I*(1+S*std::cos(H*PIC_IMAGE_PI/180)/std::cos((60-H)*PIC_IMAGE_PI/180)));
    G = 3*I-(R+B);
  } else if (H<240) {
    H-=120;
    R = a;
    G = (T)(I*(1+S*std::cos(H*PIC_IMAGE_PI/180)/std::cos((60-H)*PIC_IMAGE_PI/180)));
    B = 3*I-(R+G);
  } else {
    H-=240;
    G = a;
    B = (T)(I*(1+S*std::cos(H*PIC_IMAGE_PI/180)/std::cos((60-H)*PIC_IMAGE_PI/180)));
    R = 3*I-(G+B);
  }
  R*=255; G*=255; B*=255;
  RGB[0] = (T)(R<0?0:(R>255?255:R));
  RGB[1] = (T)(G<0?0:(G>255?255:G));
  RGB[2] = (T)(B<0?0:(B>255?255:B));
}

void iil4mitkPicImage::copyImage (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char* data, unsigned int width, unsigned int, unsigned int xoffset, unsigned int yoffset)
{
    assert (_pic);
    // assert (_min <= _max);

    unsigned int slice = _pic->n[0] * _pic->n[1] * (_pic->bpe / 8);
    float scale = (_max -_min > 0 ? 255.0 / (_max - _min) : 0.0);
    float bias = _min * scale;
    unsigned char *src = (unsigned char *) _pic->data + (y * _pic->n[0] + x) * (_pic->bpe/8);
    unsigned char *dst = data + (yoffset * width + xoffset) * (bpe () / 8);
    unsigned char *eol = dst + w * (bpe () / 8);

    //printf ("updateTexture: start = (%u/%u), end = (%u/%u), length = (%u/%u)\n", x, y, x+w-1, y+h-1, w, h); 
    for (unsigned int i = 0; i < h; i++) {

        // copy current line

    if (model () == RGB)
    {
      unsigned char* source = (unsigned char *) src;
      unsigned char* dest = dst;
      while (dest < eol) 
      {
        if(_min!=0 || _max!=255)
        {
          // level/window mechanism for intensity in HSI space
          double rgb[3], hsi[3];
          rgb[0] = source[0];
          rgb[1] = source[1];
          rgb[2] = source[2];
          RGBtoHSI<double>(rgb,hsi);
          hsi[2] = hsi[2] * 255.0 * scale - bias;
          hsi[2] = (hsi[2] > 255.0 ? 255 : (hsi[2] < 0.0 ? 0 : hsi[2]));
          hsi[2] /= 255.0;
          HSItoRGB<double>(hsi,rgb);
          dest[0] = (unsigned char)rgb[0];
          dest[1] = (unsigned char)rgb[1];
          dest[2] = (unsigned char)rgb[2];
          source+=3;
          dest+=3;
        }
        else
        {
          *dest = *source;
          ++source;
          ++dest;
        }
      }
    }
    else if (model () == RGBA)
    {
      unsigned char* source = (unsigned char *) src;
      unsigned char* dest = dst;
      while (dest < eol) 
      {
        if(_min!=0 || _max!=255)
        {
          // level/window mechanism for intensity in HSI space
          double rgb[3], hsi[3];
          rgb[0] = source[0];
          rgb[1] = source[1];
          rgb[2] = source[2];
          RGBtoHSI<double>(rgb,hsi);
          hsi[2] = hsi[2] * 255.0 * scale - bias;
          hsi[2] = (hsi[2] > 255.0 ? 255 : (hsi[2] < 0.0 ? 0 : hsi[2]));
          hsi[2] /= 255.0;
          HSItoRGB<double>(hsi,rgb);
          dest[0] = (unsigned char)rgb[0];
          dest[1] = (unsigned char)rgb[1];
          dest[2] = (unsigned char)rgb[2];

          dest[3] = source[3];

          source+=4;
          dest+=4;
        }
        else
        {
          *dest = *source;
          ++source;
          ++dest;
        }
      }
    } else
	if (mask ()) {
		mitkIpPicFORALL(MASK_INTENSITIES, _pic);
	} else 
	if (binary ()) {
		mitkIpPicFORALL(BINARY_INTENSITIES, _pic);
	} else {
#ifdef USE_MMX
		if (mitkIpPicDR(_pic->type, _pic->bpe) == mitkIpPicDR(mitkIpPicInt, 16)) {
			unsigned char* d = dst;
			unsigned char* s = src;
			if (w / 16) {
				extrema (dst, (short *) src, w, (short) _min, (short) _max);
			}
			if (w % 16) {
	        		mitkIpPicFORALL(LIMIT_INTENSITIES, _pic);
				dst = d;
				src = s;
			}
		} else {
	        	mitkIpPicFORALL(LIMIT_INTENSITIES, _pic);
		}
#else
	        mitkIpPicFORALL(LIMIT_INTENSITIES, _pic);
#endif
	}

        // go to next line

        src += _pic->n[0] * (_pic->bpe/8);
        dst += width * (bpe () / 8);
        eol = dst + w * (bpe () / 8);
    }
}


void
iil4mitkPicImage::display (iil4mitkWidget* widget)
{
	if (!_outline) {
		iil4mitkImage::display( widget );
	}
	else {
		glMatrixMode (GL_MODELVIEW);
        glPushMatrix ();
        glColor4f ( red(), green(), blue(), alpha() );
        //glColor4f( 1.0, 0.0, 0.0, 1.0 );
        glTranslatef( x(), y(), 0.0 );
        glLineWidth(_outlineWidth);
        glBegin( GL_LINES );
		
		int line = _pic->n[0];
		float fLine = (float)line;
		float x=0.0, y=0.0;
		mitkIpInt1_t *current;
		mitkIpInt1_t *end = ((mitkIpInt1_t*)_pic->data) + (_pic->n[0]*_pic->n[1]);
		for (current = (mitkIpInt1_t*)_pic->data; current<end; current++) {
			if (*current != 0) {
				if (*(current-line) == 0) {
					glVertex3f( x,     y, 0.0 );
					glVertex3f( x+1.0, y, 0.0 );
				}
				if (*(current+line) == 0) {
					glVertex3f( x,     y+1.0, 0.0 );
					glVertex3f( x+1.0, y+1.0, 0.0 );
				}
				if (*(current-1) == 0) {
					glVertex3f( x, y,     0.0 );
					glVertex3f( x, y+1.0, 0.0 );
				}
				if (*(current+1) == 0) {
					glVertex3f( x+1.0, y,     0.0 );
					glVertex3f( x+1.0, y+1.0, 0.0 );
				}
			}
			x += 1.0;
			if (x >= fLine) {
				x = 0.0;
				y += 1.0;
			}
		}
        glEnd ();
        glLineWidth(1.0f);
        glPopMatrix ();
	}
}
