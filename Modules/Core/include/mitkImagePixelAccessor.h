/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImagePixelAccessor_h
#define mitkImagePixelAccessor_h

#include <mitkImage.h>
#include <mitkImageDataItem.h>

#include <typeinfo>

namespace mitk
{
  /**
   * \brief Provides templated image access for all inheriting classes.
   *
   * \tparam TPixel Defines the pixel type used for accessing image data.
   * \tparam VDimension Defines the dimension for accessing data. Defaults to 3.
   *
   * \ingroup Data
   * \sa ImageReadAccessor
   * \sa ImageWriteAccessor
   */
  template <class TPixel, unsigned int VDimension = 3>
  class ImagePixelAccessor
  {
  public:
    typedef itk::Index<VDimension> IndexType;
    typedef ImagePixelAccessor<TPixel, VDimension> ImagePixelAccessorType;
    typedef Image::ConstPointer ImageConstPointer;

    /**
     * \brief Get the size of the specified dimension from the underlying ImageDataItem.
     *
     * \param i The dimension index to query.
     * \return The size of the requested dimension.
     */
    int GetDimension(int i) const { return m_ImageDataItem->GetDimension(i); }

  private:
  friend class Image;

  protected:
    /**
     * \brief Construct the accessor from an image and an optional ImageDataItem.
     *
     * If \p iDI is nullptr, the full channel data of the image is used instead.
     *
     * \param iP Pointer to the image to access.
     * \param iDI The allocated image data item (e.g. a slice, volume, or channel).
     *            If nullptr, the full channel data is retrieved from the image.
     */
    ImagePixelAccessor(ImageConstPointer iP, const mitk::ImageDataItem *iDI) : m_ImageDataItem(iDI)
    {
      if (iDI == nullptr)
      {
        m_ImageDataItem = iP->GetChannelData();
      }
      CheckData(iP.GetPointer());
    }

    /** \brief Destructor. */
    virtual ~ImagePixelAccessor() {}

    /**
     * \brief Validate that the accessor dimensions and pixel type match the image.
     *
     * \param image The image to validate against.
     * \throw mitk::Exception if dimensions or pixel types do not match.
     */
    void CheckData( const Image *image )
    {
      // Check if Dimensions are correct
      if ( m_ImageDataItem == nullptr )
      {
        if ( image->GetDimension() != VDimension )
        {
          mitkThrow() << "Invalid ImageAccessor: The Dimensions of ImageAccessor and Image are not equal."
          << " They have to be equal if an entire image is requested."
          << " image->GetDimension(): " << image->GetDimension() << " , VDimension: " << VDimension;
        }
      }
      else
      {
        if ( m_ImageDataItem->GetDimension() != VDimension )
        {
          mitkThrow() << "Invalid ImageAccessor: The Dimensions of ImageAccessor and ImageDataItem are not equal."
          << " m_ImageDataItem->GetDimension(): " << m_ImageDataItem->GetDimension() << " , VDimension: " << VDimension;
        }
      }

      if (!( image->GetPixelType() == mitk::MakePixelType< itk::Image< TPixel, VDimension > >() ||
             image->GetPixelType() == mitk::MakePixelType< itk::VectorImage< TPixel, VDimension > >
               ( image->GetPixelType().GetNumberOfComponents() )
         ) )
      {
        mitkThrow() << "Invalid ImageAccessor: PixelTypes of Image and ImageAccessor are not equal."
        << " image->GetPixelType(): " << typeid(image->GetPixelType()).name()
        << "\n m_ImageDataItem->GetDimension(): " << m_ImageDataItem->GetDimension()
        << " , VDimension: " << VDimension
        << " , TPixel: " << typeid(TPixel).name()
        << " , NumberOfComponents: " << image->GetPixelType().GetNumberOfComponents() << std::endl;
      }
    }

    /** \brief Holds the specified ImageDataItem for pixel data access. */
    const ImageDataItem *m_ImageDataItem;

    /** \brief Pointer to the used Geometry.
     *  Since Geometry can be different to the Image (if memory was forced to be coherent) it is necessary to store
     * Geometry separately.
     */
    BaseGeometry::Pointer m_Geometry;

    /** \brief A Subregion defines an arbitrary area within the image.
     *  If no SubRegion is defined, the whole ImageDataItem or Image is regarded.
     *  A subregion (e.g. subvolume) can lead to non-coherent memory access where every dimension has a start- and
     * end-offset.
     */
    itk::ImageRegion<VDimension> *m_SubRegion;

    /** \brief Stores all extended properties of an ImageAccessor.
     * The different flags in mitk::ImageAccessorBase::Options can be unified by bitwise operations.
     */
    int m_Options;

    /**
     * \brief Compute the linear memory offset for a given multi-dimensional image index.
     *
     * \param idx The multi-dimensional index into the image.
     * \return The linear memory offset corresponding to the index.
     */
    unsigned int GetOffset(const IndexType &idx) const
    {
      const unsigned int *imageDims = m_ImageDataItem->m_Dimensions;

      unsigned int offset = 0;
      switch (VDimension)
      {
        case 4:
          offset += idx[3] * imageDims[0] * imageDims[1] * imageDims[2];
          /* FALLTHRU */
        case 3:
          offset += idx[2] * imageDims[0] * imageDims[1];
          /* FALLTHRU */
        case 2:
          offset += idx[0] + idx[1] * imageDims[0];
          /* FALLTHRU */
        default:
          break;
      }
      return offset;
    }
  };
}

#endif
