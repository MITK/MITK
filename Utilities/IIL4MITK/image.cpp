#include <assert.h>
#include <ipFunc/ipFunc.h>
#include "widget.h"
#include "texture.h"
#include "image.h"

static unsigned int
power2 (const unsigned int x) 
{
  unsigned int y = 1;

  while (y < x) 
  {
    y *= 2;
  }
  return y;
}

const unsigned int iil4mitkImage::_bytes [] = {1, 2, 3, 4, 3};

iil4mitkImage::iil4mitkImage (unsigned int size)
: _width (0), _height (0), _rx (0), _ry (0), _rw (0), _rh (0),
  _model (INTENSITY), _interpolation (false), _pixels (NULL),
  _internal (GL_LUMINANCE), _size(size)
{

}

iil4mitkImage::~iil4mitkImage ()
{
  clear ();
}

void
iil4mitkImage::setImage (const unsigned int width, const unsigned int height,
  const int model, unsigned char *data)
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
iil4mitkImage::imageWidth() const
{
  return _width;
}

unsigned int
iil4mitkImage::imageHeight () const
{
  return _height;
}

void
iil4mitkImage::setRegion (const unsigned int x, const unsigned y, 
  const unsigned w, const unsigned h)
{
  assert (x + w <= _width);
  assert (y + h <= _height);

  if ((_rx != x) || (_ry != y) || (_rw != w) || (_rh != h)) {
    _rx = x; _ry = y; _rw = w; _rh = h;
    invalidateTextures ();
  }
}

unsigned int
iil4mitkImage::regionX () const
{
  return _rx;
}

unsigned int
iil4mitkImage::regionY () const
{
  return _ry;
}

unsigned int
iil4mitkImage::regionWidth () const
{
  return _rw;
}

unsigned int
iil4mitkImage::regionHeight () const
{
  return _rh;
}

void
iil4mitkImage::clear ()
{
  _width = _height = 0;
  _rx = _ry = _rw = _rh = 0;
  _model = INTENSITY;
  _pixels = NULL;
  _internal = GL_LUMINANCE;

  std::map<void*,Textures*>::iterator i;
  for (i=_textures.begin(); i!=_textures.end(); ++i) {
    for (unsigned int j = 0; j < (*i).second->size (); j++) {
      delete (*(*i).second)[j];
    }
    delete i->second;
  }
  _textures.clear ();
}

float
iil4mitkImage::width () const
{
  return (float) _rw;
}

float
iil4mitkImage::height () const
{
  return (float) _rh;
}

int
iil4mitkImage::model () const
{
  return _model;
}

void
iil4mitkImage::setInterpolation (const bool on)
{
  _interpolation = on;
}

bool
iil4mitkImage::interpolation () const
{
  return _interpolation;
}

void
iil4mitkImage::display (iil4mitkWidget* widget)
{
  GLdouble planeX [] = {-1, 0, 0, regionWidth ()};
  GLdouble planeY [] = {0, -1, 0, regionHeight ()};

  if (!visible () 
    || (constraint () && (widget != constraint ())))
  {
    return;
  }

  if (_pixels)
  {
    assert (_rx + _rw <= _width);
    assert (_ry + _rh <= _height);

    GLboolean texturing = glIsEnabled (GL_TEXTURE_2D);
    GLboolean blending = glIsEnabled (GL_BLEND);

    glClipPlane (GL_CLIP_PLANE4, planeX);
    glEnable (GL_CLIP_PLANE4);
    glClipPlane (GL_CLIP_PLANE5, planeY);
    glEnable (GL_CLIP_PLANE5);

    if ((_model == INTENSITY_ALPHA) || (_model == COLOR_ALPHA) || (_model == RGB))
    {
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

    glDisable (GL_CLIP_PLANE4);
    glDisable (GL_CLIP_PLANE5);

    if (texturing == GL_FALSE) glDisable (GL_TEXTURE_2D);
    if (blending  == GL_FALSE) glDisable (GL_BLEND);
  }
}

void iil4mitkImage::drawTextures (iil4mitkWidget* widget)
{
  const unsigned int s = _size; // size of the tiles
  unsigned int n, m;  // number of the tiles

  n = (unsigned int) ceilf ((float) _rw / (float) (s - 2));
  m = (unsigned int) ceilf ((float) _rh / (float) (s - 2));

  /* Allocate memory for the textures */
  
  Textures* textures;
  {
    iil4mitkWidget* w;
    unsigned int available, total;
  
    //w = (widget->IsSharing () ? widget->SharedWidget () : widget);
    w = (false ? NULL : widget);
    std::map<void*,Textures*>::iterator tex_it = _textures.find (w);
    if(tex_it!=_textures.end())
      textures = tex_it->second;
    else
      textures = NULL;

    if (!textures) {
      textures = new Textures ();
      typedef std::pair <void*, std::vector<_iil4mitkTexture*>* > TexturePair;
      _textures.insert (TexturePair(w, textures));
    }
    available = textures->size (); 
    total = n * m;
    //textures->resize (total);
    int iii;
    for (unsigned int i = available; i < total; i++)
    {
      textures->push_back(new _iil4mitkTexture (w));
      iii=textures->size ();
    }
    widget->MakeCurrent ();
  }  

  /* Render the textures */

  glScalef ((float) (s-2), (float) (s-2), 1.0);
  for (unsigned int i = 0; i < n; i++)
  {
    unsigned int pos [2]; // left-top corner of the region
    unsigned int len [2]; // extent of the region
    unsigned int tex [2]; // size of the texture
    float res [2];        // resolution of the texture
    
    pos [0] = _rx+i*(s-2);
          len [0] = (pos[0]+(s-2)>_rw ? _rw-pos[0] : s-2);  
    tex [0] = power2(s);
    res [0] = 1.0f/tex[0];
    for (unsigned int j = 0; j < m; j++) 
    {
      _iil4mitkTexture* texture;
      
      texture = (*textures)[i*m+j];
      pos [1] = _ry+j*(s-2);
      len [1] = (pos[1]+(s-2)>_rh ? _rh-pos[1] : s-2);
      tex [1] = power2(s);
      res [1] = 1.0f/tex[1];
  
      //if (widget->isVisible (pos [0], pos [1], len [0], len [1])) {
      if (true) {
      // widget->makeCurrent ();  
      texture->bind ();
      // widget->makeCurrent ();  
      texture->setSize (tex[0], tex[1]);
      texture->setModel (_model);
      texture->setInterpolation (_interpolation);
      if (!texture->isValid ()) 
      {
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
iil4mitkImage::remove (iil4mitkWidget* widget)
{
  std::map<void*,Textures*>::iterator i =
   _textures.find (widget);
  for (unsigned int j = 0; j < (*i).second->size (); j++) 
  {
    delete (*(*i).second)[j];
  }  
  delete i->second;
  _textures.erase(i);
}

void
iil4mitkImage::invalidateTextures ()
{
  std::map<void*,Textures*>::iterator i;
  for (i=_textures.begin(); i!=_textures.end(); ++i) 
  {
    for (unsigned int j = 0; j < (*i).second->size (); j++) 
    {
      (*(*i).second)[j]->invalidate ();
    }  
  }
}

void
iil4mitkImage::updateTexture (_iil4mitkTexture* texture, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
  unsigned int p2w = texture->width ();
  unsigned int p2h = texture->height ();
  unsigned int size = p2w * p2h * _bytes [_model];
  unsigned char *region = (unsigned char *) malloc (size * sizeof (unsigned char));
  unsigned int left = (x == 0 ? 0 : 1);
  unsigned int right = (x+w == _rw ? 0 : 1);
  unsigned int top = (y == 0 ? 0 : 1);
  unsigned int bottom = (y + h == _rh ? 0 : 1);

  if (x == 0) 
  {
    copyImage (x, y-top, 1, h+top+bottom, region, p2w, p2h, 0, 1-top);
  }
  else 
  {
    copyImage (x-1, y-top, 1, h+top+bottom, region, p2w, p2h, 0, 1-top);
  }
  if (x+w == _rw) 
  {
    copyImage (x+w-1, y-top, 1, h+top+bottom, region, p2w, p2h, w+1, 1-top);
  }
  else
  {
    copyImage (x+w, y-top, 1, h+top+bottom, region, p2w, p2h, w+1, 1-top);
  }
  if (y == 0) 
  {
    copyImage (x-left, y, w+left+right, 1, region, p2w, p2h, 1-left, 0);
  }
  else
  {
    copyImage (x-left, y-1, w+left+right, 1, region, p2w, p2h, 1-left, 0);
  }
  if (y+h == _rh)
  {
    copyImage (x-left, y+h-1, w+left+right, 1, region, p2w, p2h, 1-left, h+1);
  }
  else 
  {
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

void iil4mitkImage::copyImage(unsigned int x, unsigned int y, unsigned int w,
  unsigned int h, unsigned char* data, unsigned int width, unsigned int,
  unsigned int xoffset, unsigned int yoffset)
{
  const unsigned int bytes = _bytes [_model];
  const unsigned char *src = _pixels + (y * _width + x) * bytes;
  unsigned char *dst = data + (yoffset * width + xoffset) * bytes;

  for (unsigned int i = 0; i < h; i++) 
  {
    memcpy (dst, src, w * bytes);
    src += _width * bytes;
    dst += width * bytes;
  }
}

unsigned int
iil4mitkImage::bpe ()
{
  return _bytes [_model] * 8;
}

unsigned char*
iil4mitkImage::pixels ()
{
  return _pixels;
}

iil4mitkImage* 
iil4mitkImage::find (const iil4mitkItem* item) 
{
  iil4mitkImage* result = NULL;

  if (!item) return (iil4mitkImage *) NULL;

  if ( dynamic_cast<const iil4mitkImage*>(item)!=NULL ) {
      result = const_cast<iil4mitkImage*>(dynamic_cast<const iil4mitkImage*>(item));
  }
  return (iil4mitkImage *) result;
}
