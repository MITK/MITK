#include <assert.h>
#include "texture.h"

_iilTexture::_iilTexture (iilWidget* parent, const char* name ) 
	: QObject (parent, name), _width (0), _height (0), _model (0), _internal (0), _valid (false), _interpolation (false), _red (1.0), _green (1.0), _blue (1.0), _alpha (1.0)
{	
	assert (parent);

	parent->makeCurrent ();
	glGenTextures (1, &_name);
	glBindTexture (GL_TEXTURE_2D, _name);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}	

/*
Pay attention that the context which the widget share with another widget
will not be deleted.
*/
_iilTexture::~_iilTexture () {
	assert((QGLWidget *)parent());
	((QGLWidget *) parent ())->makeCurrent ();
	glDeleteTextures (1, &_name);

	// Are the textures deleted automatically?

}

void 
_iilTexture::bind () 
{
	glBindTexture (GL_TEXTURE_2D, _name);
}	
	

void 
_iilTexture::setSize (const unsigned int width, const unsigned int height)
{
	assert (width > 0);
	assert (height > 0);

	if ((width != _width) || (height != _height)) {
		_valid = false;
	}
	_width = width;
	_height = height;
}

unsigned int 
_iilTexture::width () const
{
	return _width;
}

unsigned int 
_iilTexture::height () const
{
	return _height;
}
	
void 
_iilTexture::setModel (int model)
{
	switch (model) {
	case INTENSITY:
		_model = GL_LUMINANCE;
		break;
	case INTENSITY_ALPHA:
		_model = GL_LUMINANCE_ALPHA;
		break;
	case COLOR:
		_model = GL_RGB;
		break;
	case COLOR_ALPHA:
		_model = GL_RGBA;
		break;
	}
		if (_internal != _model) {
			_internal = _model;
			_valid = false;
		}	
}

void 
_iilTexture::setData (const unsigned char* data)
{
	glTexImage2D (GL_TEXTURE_2D, 0, _internal, _width, _height, 0, _model, GL_UNSIGNED_BYTE, data);
	_valid = true;
}

void 
_iilTexture::setInterpolation (const bool on) {
	if (on) {
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {	
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	_interpolation = on; 
}

void 
_iilTexture::invalidate ()
{

	_valid = false;
}

bool 
_iilTexture::isValid ()
{
	return _valid;
}
