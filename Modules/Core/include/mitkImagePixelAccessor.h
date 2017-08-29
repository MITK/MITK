/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKIMAGEPIXELACCESSOR_H
#define MITKIMAGEPIXELACCESSOR_H

#include "mitkImage.h"
#include "mitkImageDataItem.h"

#include <typeinfo>

namespace mitk
{
  /**
   * @brief Provides templated image access for all inheriting classes
   * @tparam TPixel defines the PixelType
   * @tparam VDimension defines the dimension for accessing data
   * @ingroup Data
   */
  template <class TPixel, unsigned int VDimension = 3>
  class ImagePixelAccessor
  {
  public:
    typedef itk::Index<VDimension> IndexType;
    typedef ImagePixelAccessor<TPixel, VDimension> ImagePixelAccessorType;
    typedef Image::ConstPointer ImageConstPointer;

    /** Get Dimensions from ImageDataItem */
    int GetDimension(int i) const { return m_ImageDataItem->GetDimension(i); }

  private:
  friend class Image;

  protected:
    /**  \param ImageDataItem* specifies the allocated image part */
    ImagePixelAccessor(ImageConstPointer iP, const mitk::ImageDataItem *iDI) : m_ImageDataItem(iDI)
    {
      if (iDI == nullptr)
      {
        m_ImageDataItem = iP->GetChannelData();
      }
      CheckData(iP.GetPointer());
    }

    /** Destructor */
    virtual ~ImagePixelAccessor() {}

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

    /** Holds the specified ImageDataItem */
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

    /** Get memory offset for a given image index */
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

#endif // MITKIMAGEACCESSOR_H
