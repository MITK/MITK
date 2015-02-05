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

#include "mitkImageDataItem.h"
#include "mitkImage.h"

namespace mitk {

/**
 * @brief Provides templated image access for all inheriting classes
 * @tparam TPixel defines the PixelType
 * @tparam VDimension defines the dimension for accessing data
 * @ingroup Data
 */
template <class TPixel, unsigned int VDimension = 3>
class ImagePixelAccessor
{
  friend class Image;

public:
  typedef itk::Index<VDimension> IndexType;
  typedef ImagePixelAccessor<TPixel,VDimension> ImagePixelAccessorType;
  typedef Image::ConstPointer ImageConstPointer;

  /** Get Dimensions from ImageDataItem */
  int GetDimension (int i) const
  {
    return m_ImageDataItem->GetDimension(i);
  }

protected:

   /**  \param ImageDataItem* specifies the allocated image part */
  ImagePixelAccessor(ImageConstPointer iP, const mitk::ImageDataItem* iDI)
    : m_ImageDataItem(iDI)
  {
    if(iDI == NULL)
    {
        m_ImageDataItem = iP->GetChannelData();
    }
    CheckData(iP.GetPointer());
  }

  /** Destructor */
  virtual ~ImagePixelAccessor()
  {
  }

  void CheckData(const Image* image)
  {
    // Check if Dimensions are correct
    if(m_ImageDataItem == NULL)
    {
      if(image->GetDimension() != VDimension)
      {
        mitkThrow() << "Invalid ImageAccessor: The Dimensions of ImageAccessor and Image are not equal. They have to be equal if an entire image is requested";
      }
    }
    else
    {
      if(m_ImageDataItem->GetDimension() != VDimension)
      {
        mitkThrow() << "Invalid ImageAccessor: The Dimensions of ImageAccessor and ImageDataItem are not equal.";
      }
    }

    // Check if PixelType is correct
    if(!(image->GetPixelType() ==  mitk::MakePixelType< itk::Image<TPixel, VDimension> >()) )
    {
      mitkThrow() << "Invalid ImageAccessor: PixelTypes of Image and ImageAccessor are not equal";
    }
  }

protected:

  // protected members

  /** Holds the specified ImageDataItem */
  const ImageDataItem* m_ImageDataItem;

  /** \brief Pointer to the used Geometry.
   *  Since Geometry can be different to the Image (if memory was forced to be coherent) it is necessary to store Geometry separately.
   */
  BaseGeometry::Pointer m_Geometry;

  /** \brief A Subregion defines an arbitrary area within the image.
   *  If no SubRegion is defined, the whole ImageDataItem or Image is regarded.
   *  A subregion (e.g. subvolume) can lead to non-coherent memory access where every dimension has a start- and end-offset.
   */
  itk::ImageRegion<VDimension>* m_SubRegion;

  /** \brief Stores all extended properties of an ImageAccessor.
   * The different flags in mitk::ImageAccessorBase::Options can be unified by bitwise operations.
   */
  int m_Options;

  /** Get memory offset for a given image index */
  unsigned int GetOffset(const IndexType & idx) const
  {
    const unsigned int * imageDims = m_ImageDataItem->m_Dimensions;

    unsigned int offset = 0;
    switch(VDimension)
    {
    case 4:
      offset += idx[3]*imageDims[0]*imageDims[1]*imageDims[2];
    case 3:
      offset += idx[2]*imageDims[0]*imageDims[1];
    case 2:
      offset += idx[0] + idx[1]*imageDims[0];
      break;
    }
    return offset;
  }

};

}

#endif // MITKIMAGEACCESSOR_H
