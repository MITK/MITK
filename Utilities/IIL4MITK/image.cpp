#include <assert.h>
#include <ipFunc/ipFunc.h>
#include "widget.h"
#include "texture.h"
#include "switch.h"
#include "image.h"

static unsigned int
power2 (const unsigned int x) {
    unsigned int y = 1;

    while (y < x) {
        y *= 2;
    }
    return y;
}

const unsigned int iilImage::_bytes [] = {1, 2, 3, 4};

iilImage::iilImage (QObject* parent, const char* name)
        : iilItem (parent, name), _width (0), _height (0), _rx (0), _ry (0), _rw (0), _rh (0), _model (INTENSITY), _interpolation (false), _pixels (NULL), _internal (GL_LUMINANCE)
{
    _textures.setAutoDelete (true);
}

iilImage::~iilImage ()
{
    clear ();
}

void
iilImage::setImage (const unsigned int width, const unsigned int height, const int model, unsigned char *data)
{
    assert (width > 0);
    assert (height > 0);

    _width = width;
    _height = height;
    _model = model;
    _pixels = data;
    invalidateTextures ();
}

unsigned int
iilImage::imageWidth() const
{
    return _width;
}

unsigned int
iilImage::imageHeight () const
{
    return _height;
}

void
iilImage::setRegion (const unsigned int x, const unsigned y, const unsigned w, const unsigned h)
{
    assert (x + w <= _width);
    assert (y + h <= _height);

    if ((_rx != x) || (_ry != y) || (_rw != w) || (_rh != h)) {
        _rx = x; _ry = y; _rw = w; _rh = h;
        invalidateTextures ();
    }
}

unsigned int
iilImage::regionX () const
{
    return _rx;
}

unsigned int
iilImage::regionY () const
{
    return _ry;
}

unsigned int
iilImage::regionWidth () const
{
    return _rw;
}

unsigned int
iilImage::regionHeight () const
{
    return _rh;
}

void
iilImage::clear ()
{
    _width = _height = 0;
    _rx = _ry = _rw = _rh = 0;
    _model = INTENSITY;
    _pixels = NULL;
    _internal = GL_LUMINANCE;
    _textures.clear ();
}

float
iilImage::width () const
{
    return (float) _rw;
}

float
iilImage::height () const
{
    return (float) _rh;
}

int
iilImage::model () const
{
    return _model;
}

void
iilImage::setInterpolation (const bool on)
{
    _interpolation = on;
}

bool
iilImage::interpolation () const
{
    return _interpolation;
}

void
iilImage::display (iilWidget* widget)
{
    GLdouble planeX [] = {-1, 0, 0, regionWidth ()};
    GLdouble planeY [] = {0, -1, 0, regionHeight ()};

    if (!visible () || !widget->isVisible (this) || (constraint () && (widget != constraint ()))) {
	return;
    }

    if (_pixels) {
        assert (_rx + _rw <= _width);
        assert (_ry + _rh <= _height);
	
        bool texturing = glIsEnabled (GL_TEXTURE_2D);
        bool blending = glIsEnabled (GL_BLEND);

	glClipPlane (GL_CLIP_PLANE1, planeX);
	glEnable (GL_CLIP_PLANE1);
	glClipPlane (GL_CLIP_PLANE2, planeY);
	glEnable (GL_CLIP_PLANE2);

    	if ((_model == INTENSITY_ALPHA) || (_model == COLOR_ALPHA)) {
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    	}
        glEnable (GL_TEXTURE_2D);
	glMatrixMode (GL_MODELVIEW);
        glPushMatrix ();
        glColor4f (red (), green (), blue (), alpha ());
        glTranslatef (x (), y (), 0.0);
	drawTextures (widget);
        glPopMatrix ();

	glDisable (GL_CLIP_PLANE1);
	glDisable (GL_CLIP_PLANE2);
        if (!texturing) glDisable (GL_TEXTURE_2D);
        if (!blending) glDisable (GL_BLEND);
    }
}

void iilImage::drawTextures (iilWidget* widget)
{
	const unsigned int s = 256; 	// size of the tiles
	unsigned int n, m;		// number of the tiles

	n = (unsigned int) ceilf ((float) _rw / (float) (s - 2));
	m = (unsigned int) ceilf ((float) _rh / (float) (s - 2));

	/* Allocate memory for the textures */
	
	Textures* textures;
	{
	        iilWidget* w;
		unsigned int available, total;
	
		w = (widget->isSharing () ? widget->sharedWidget () : widget);
        	textures = _textures.find (w);
		if (!textures) {
			textures = new Textures ();
			textures->setAutoDelete (true);
			_textures.insert (w, textures);
		}
		available = textures->count (); 
		total = n * m;
		textures->resize (total);
		for (unsigned int i = available; i < total; i++) {
			textures->insert (i, new _iilTexture (w));
		}
		widget->makeCurrent ();
	}	

	/* Render the textures */

        glScalef ((float) (s-2), (float) (s-2), 1.0);
	for (unsigned int i = 0; i < n; i++) {
		unsigned int pos [2]; // left-top corner of the region
		unsigned int len [2]; // extent of the region
		unsigned int tex [2]; // size of the texture
		float res [2];	      // resolution of the texture
		
		pos [0] = _rx+i*(s-2);
	        len [0] = (pos[0]+(s-2)>_rw ? _rw-pos[0] : s-2);	
		tex [0] = power2(s);
		res [0] = 1.0f/tex[0];
		for (unsigned int j = 0; j < m; j++) {
			_iilTexture* texture;
			
			texture = (*textures)[i*m+j];
			pos [1] = _ry+j*(s-2);
			len [1] = (pos[1]+(s-2)>_rh ? _rh-pos[1] : s-2);
			tex [1] = power2(s);
			res [1] = 1.0f/tex[1];
	
			if (widget->isVisible (pos [0], pos [1], len [0], len [1])) { 
			// widget->makeCurrent ();	
			texture->bind ();
			// widget->makeCurrent ();	
			texture->setSize (tex[0], tex[1]);
			texture->setModel (_model);
			texture->setInterpolation (_interpolation);
        		if (!texture->isValid ()) {
				updateTexture (texture, pos[0], pos[1], len[0], len[1]);
			}	

			glBegin (GL_QUADS);
			glTexCoord2f (res[0], res[1]); glVertex3f (0.0, 0.0, 0.0);
			glTexCoord2f (1.0-res[0], res[1]); glVertex3f (1.0, 0.0, 0.0);
			glTexCoord2f (1.0-res[0], 1.0-res[1]); glVertex3f (1.0, 1.0, 0.0);
			glTexCoord2f (res[0], 1.0-res[1]); glVertex3f (0.0, 1.0, 0.0);
			glEnd ();
			}
			glTranslatef (0.0, 1.0, 0.0);
		}
		glTranslatef (1.0, -((float) m), 0.0);
	}
}

void 
iilImage::remove (iilWidget* widget)
{
   _textures.remove (widget);
}

void
iilImage::invalidateTextures ()
{
    for (QPtrDictIterator<Textures> i (_textures); i.current (); ++i) {
        for (unsigned int j = 0; j < (*i).count (); j++) {
		(*i)[j]->invalidate ();
	}	
    }
}

void
iilImage::updateTexture (_iilTexture* texture, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
    unsigned int p2w = texture->width ();
    unsigned int p2h = texture->height ();
    unsigned int size = p2w * p2h * _bytes [_model];
    unsigned char *region = (unsigned char *) malloc (size * sizeof (unsigned char));
    unsigned int left = (x == 0 ? 0 : 1);
    unsigned int right = (x+w == _rw ? 0 : 1);
    unsigned int top = (y == 0 ? 0 : 1);
    unsigned int bottom = (y + h == _rh ? 0 : 1);

    if (x == 0) {
	copyImage (x, y-top, 1, h+top+bottom, region, p2w, p2h, 0, 1-top);
    } else {
	copyImage (x-1, y-top, 1, h+top+bottom, region, p2w, p2h, 0, 1-top);
    }
    if (x+w == _rw) {
	copyImage (x+w-1, y-top, 1, h+top+bottom, region, p2w, p2h, w+1, 1-top);
    } else {
	copyImage (x+w, y-top, 1, h+top+bottom, region, p2w, p2h, w+1, 1-top);
    }
    if (y == 0) {
	copyImage (x-left, y, w+left+right, 1, region, p2w, p2h, 1-left, 0);
    } else {
	copyImage (x-left, y-1, w+left+right, 1, region, p2w, p2h, 1-left, 0);
    }
    if (y+h == _rh) {
	copyImage (x-left, y+h-1, w+left+right, 1, region, p2w, p2h, 1-left, h+1);
    } else {
	copyImage (x-left, y+h, w+left+right, 1, region, p2w, p2h, 1-left, h+1);
    }
    copyImage (x, y, w, h, region, p2w, p2h, 1, 1);
    texture->setData (region);

    /*
    char filename [255];
    sprintf (filename, "tile_%u_%u.pic", x, y);
    ipPicDescriptor* pic = ipPicNew ();
    pic->type = ipPicUInt;
    pic->bpe = 8;
    pic->dim = 2;
    pic->n [0] = p2w;
    pic->n [1] = p2h;
    pic->data = region;
    ipPicPut (filename, pic);
    pic->data = NULL;
    ipPicFree (pic);
    */

    free (region);
}

void iilImage::copyImage (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char* data, unsigned int width, unsigned int , unsigned int xoffset, unsigned int yoffset)
{
    const unsigned int bytes = _bytes [_model];
    const unsigned char *src = _pixels + (y * _width + x) * bytes;
    unsigned char *dst = data + (yoffset * width + xoffset) * bytes;

    for (unsigned int i = 0; i < h; i++) {
        memcpy (dst, src, w * bytes);
        src += _width * bytes;
        dst += width * bytes;
    }
}

unsigned int
iilImage::bpe ()
{
    return _bytes [_model] * 8;
}

unsigned char*
iilImage::pixels ()
{
    return _pixels;
}

iilImage* 
iilImage::find (const iilItem* item) 
{
    iilImage* result = NULL;

    if (!item) return (iilImage *) NULL;

    if (item->isA ("iilGroup")) {
        QPtrListIterator<iilItem> i (((iilGroup *) item)->members ());
        do {
            result = iilImage::find (i ());
        } while (i.current () && !result);
    }

    if (item->isA ("iilSwitch")) {
        result = iilImage::find (((iilSwitch *) item)->current ());
    }

    if (item->inherits ("iilImage")) {
        result = (iilImage *) item;
    }
    return (iilImage *) result;
}
