#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "widget.h"

/*
The class holds a texture which is associated 
to a particular widget. It is not responsible for
the rendering of images.

\todo Remove the makeCurrent calls
*/
class _iil4mitkTexture {

public:

	/*!
	\brief The constructor.
	\note Make sure that the desired OpenGL context is selected.
	*/
	_iil4mitkTexture (iil4mitkWidget* parent); 
	
	/*! 
	\brief The destructor.

	Frees the texture in the OpenGL context, if it is still
	\note Make sure that the desired OpenGL context is selected.
	existing.
	*/
	virtual ~_iil4mitkTexture ();

	/*!
	\brief Binds the texture to the OpenGL context of the widget.
	\note Make sure that the OpenGL context is made current.
	*/
	void bind ();

	/*!
	\brief Sets the size of the texture.
	*/
	void setSize (const unsigned int width, const unsigned int height);

	/*!
	\brief Gets the width of the texture.
	*/
	unsigned int width () const;

	/*!
	\brief Gets the height of the texture.
	*/
	unsigned int height () const;
	
	/*!
	\brief The color models of the image.
	*/
	enum {INTENSITY = 0, INTENSITY_ALPHA, COLOR, COLOR_ALPHA, RGB, RGBA};
	
	/*!
	\brief Sets the color model of the data. If the color is different from white,
	the RGBA color model is used.
	@param model the color model which is requested
	*/
	void setModel (int model);

	/*!
	\brief Sets the image data for the texture. If some of the above 
	parameters have been changed, the method brings the texture 
	to a valid state. 
	\note Make sure that the OpenGL context is made current and
	the texture has been bound.
	\see valid()
	*/
	void setData (const unsigned char* data);

	/*!
	\brief Turns the bilinear interpolation of the texture on/off. 
	\note Make sure that the OpenGL context is made current and
	the texture has been bound.
	*/
	void setInterpolation (const bool on = true);

	/*!
	\brief Makes the texture invalid.
	*/
	void invalidate ();

	/*!
	\brief Checks if the texture is still valid. For example, the texture
	gets invalid if its parameters are changed without setting new data.
	*/
	bool isValid ();
	
private:

	/*!
	\brief The texture name.
	*/
	GLuint _name;

	/*!
	\brief The size of the texture.
	*/
	unsigned int _width, _height;

	/*!
	\brief The color model of the data.
	*/
	GLenum _model;

	/*!
	\brief The data.
	*/
	const unsigned char* _data;
	
	/*!
	\brief The internal format of the texture.
	*/
	GLenum _internal;
	
	/*!
	\brief Flags if the texture is valid.
	*/
	bool _valid;

	/*!
	\brief Flags if the texture will be interpolated. 
	*/
	bool _interpolation;

	/*!
	\brief The color which is used for intensity images. 
	*/
	float _red, _green, _blue, _alpha;

  iil4mitkWidget* parent;
};	

#endif
