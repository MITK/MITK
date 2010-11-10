#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <mitkIpPic.h>
#include "texture.h"
#include "item.h"
#include <vector>
#include <map>

/*!
\todo Manage the texture in a common way for all instances.
\todo Implement scaling of images here.
*/
class iil4mitkImage : public iil4mitkItem {

public:

    /*!
    \brief The constructor.
    */
    iil4mitkImage (unsigned int size = 128);

    /*!
    \brief The destructor.
    */
    virtual ~iil4mitkImage ();

    /*!
    \brief The supported color models.
    */
    enum {INTENSITY = 0, INTENSITY_ALPHA, COLOR, COLOR_ALPHA, RGB, RGBA};

    /*!
    \brief Sets the image.
    @param width,height the image size. Make sure that the
    dimensions have a power of two. Note that the image
    size is limited (see \c maxDimensions).
    @param model the color model
    @param data the data
    \note Make sure that the data keeps valid 
    until this instance is destroyed.
    */
    virtual void setImage (const unsigned int width, const unsigned int height, const int model, unsigned char *data);

    /*!
    \brief Gets the image size.
    */
    unsigned int imageWidth () const;
    unsigned int imageHeight () const;

    /*!
    \brief Limits the image to the specified region.
    @param x,y,w,h the position and size of the region
    */
    virtual void setRegion (const unsigned int x, const unsigned y, const unsigned w, const unsigned h);

    /*!
    \brief Gets the position and the size of the region.
    */
    unsigned int regionX () const;
    unsigned int regionY () const;
    unsigned int regionWidth () const;
    unsigned int regionHeight () const;

    /*!
    \brief Clears the image. 
    */
    virtual void clear ();

    /*!
    \brief Gets the maximal size of an image.
    */
    // unsigned int maxSize () const;

    /*!
    Gets the color model of the image.
    */
    int model () const;

    /*!
    \brief Switches interpolation for scaled images on or off.
    */
    virtual void setInterpolation (const bool on = true);

    /*!
    \brief Gets the interpolation method.
    */
    bool interpolation () const;

    /*!
    \brief Invalidates the content of the textures.
    */
    virtual void invalidateTextures ();

public:

    virtual float width () const;
    virtual float height () const;
    virtual void display (iil4mitkWidget* widget);
    virtual void remove (iil4mitkWidget* widget);

public:

    /*!
    \brief Gets the first image of the item tree.
    */
    static iil4mitkImage* find (const iil4mitkItem* item);

protected:

    /*!
    \brief Draws the image by means of the textures.
    */
    void drawTextures (iil4mitkWidget* widget);

    /*!
    \brief Updates the content of the given texture.
    */
    void updateTexture (_iil4mitkTexture* texture, unsigned int x, unsigned int y, unsigned int w, unsigned int h);

    /*!
    \brief Copies a specified region of the image.
    @param x,y,w,h the geometry of the region
    @param data the image data where the region is copied to
    @param width,height the geometry of the image data
    @param xoffset,yoffset the image point where the region is copied to
    */
    virtual void copyImage (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char* data, unsigned int width, unsigned int height, unsigned int xoffset, unsigned int yoffset);

    /*!
    \brief Sets the color model of the image.

    \note method needed?
    */
    // void setModel (int model);

    /*!
    \brief Gets the number of bits per element of the image.
    */
    unsigned int bpe ();

    /*!
    \brief Gets the pixels of the image.
    */
    unsigned char* pixels ();


private:

    /*!
    \brief The dimensions of the image.
    */
    unsigned int _width, _height;

    /*!
    \brief The region of the image which will be displayed.
    */
    unsigned int _rx, _ry, _rw, _rh;

    /*!
    \brief The color model of the image.
    */
    int _model;

    /*!
    \brief The interpolation method.
    */
    bool _interpolation;

    /*!
    \brief The bytes per element of the image.
    */
    static const unsigned int _bytes [6];

    /*!
    \brief The pixels of the image.
    */
    unsigned char *_pixels;

    /*!
    \brief The internal color model of the texture.
    */
    GLenum _internal;

    /*!
    \brief The list of allocated textures. 
    */
    typedef std::vector<_iil4mitkTexture*> Textures;
    std::map<void*,Textures*> _textures;

    unsigned int _size;

};

#endif
