#ifndef _PIC_IMAGE_H_
#define _PIC_IMAGE_H_

#include <mitkIpPic.h>
#include "image.h"

/*!
\brief The class adds support for PIC images by providing a proper interface
to map the intensity range to the physical one of the display. 
*/
class iil4mitkPicImage : public iil4mitkImage {

public:
    /*!
    \brief The constructor.
    */
    iil4mitkPicImage (unsigned int size = 128);

    /*!
    \brief The destructor.
    */
    virtual ~iil4mitkPicImage ();

    /*!
    \brief Sets the PIC image.
    @param pic the PIC image
    @param model the color model

    \note Is the model needed here?
    */
    void setImage (ipPicDescriptor* pic, int model = INTENSITY);

    /*!
    \brief Gets the PIC image.
    */
    ipPicDescriptor* image () const;

    /*!
    \brief Sets the range of the intensities which will be displayed.
    @param minimum the minimal intensity
    @param maximum the maximal intensity
    */
    void setExtrema (const float minimum, const float maximum);

    /*!
    \brief Sets the range of the intensities which will be displayed.
    @param level the level of the window
    @param window the width of the window 
    */
    void setWindow (const float level, const float window);

    /*!
    \brief Gets the minimal intensity which will be displayed.
    */
    float minimum () const;

    /*!
    \brief Gets the maximal intensity which will be displayed.
    */
    float maximum () const;

    /*!
    \brief Gets the level of the window which limits the 
    displayed intensities.
    */
    float level () const;

    /*!
    \brief Gets the width of the window which limits the 
    displayed intensities.
    */
    float window () const;

    /*!
    \brief Sets the color map which assigns each intensity
    a color according to the color model.
    @param colors the array of colors using either the
    RGB format or RGBA format which depends on the color
    model.
    */
    void setColors (const unsigned char* colors);

    /*!
    \brief Gets the color map.
    */
    const unsigned char* colors () const;

    /*!
	\brief Sets the binary flag which forces the data 
	to be converted into a binary image. 
    */
    void setBinary (const bool on);

    /*!
	\brief Checks if the binary flag is set.
    */
    bool binary () const;

    /*!
	\brief Sets the outline flag which forces the data 
	to be displayed as outline. Only possible with binary data!
    */
    void setOutline (const bool on);

    /*!
	\brief Checks if the outline flag is set.
    */
    bool outline () const;

    /*!
	\brief Sets the mask flag which ensures that
	only pixels are displayed which are in the
	intensity window. 
    */ 
    void setMask (const bool on);

    /*!
    \brief Checks if the mask flag is set.
    */
    bool mask () const;

public:

    virtual void clear ();
	virtual void display (iil4mitkWidget* widget);

public:

	/*!
	\brief Gets the first image of the item tree.
	*/
	static iil4mitkPicImage* find (const iil4mitkItem* item);

protected:

	virtual void copyImage (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char* data, unsigned int width, unsigned int height, unsigned int xoffset, unsigned int yoffset);

private:

    /*!
    \brief The PIC image.
    */
    ipPicDescriptor* _pic;

    /*!
    \brief The extremal values.
    */
    float _min, _max;

    /*!
    \brief The color map.
    */
    const unsigned char* _colors;

	/*!
	\brief The binary flag. If the flag is set, the data
	will be displayed as a binary image.
	*/
	bool _binary;

	/*!
	\brief The mask flag. If the flag is set, the data
	which is in the defined intensity window will be shown
	as a binary image. In contrast to the binary flag,
	the upper values are set to zero.
	*/
	bool _mask;

	/*!
	\brief The outline flag. If the flag is set, the binary data
	will be displayed with lines showing the outline.
	*/
	bool _outline;
};

#endif
