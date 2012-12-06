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

#ifndef MITKIMAGEPIXELREADACCESSOR_H
#define MITKIMAGEPIXELREADACCESSOR_H

#include <algorithm>
#include <itkIndex.h>
#include <itkPoint.h>
#include <itkSmartPointer.h>
#include "mitkImageDataItem.h"
#include "mitkPixelType.h"
#include "mitkImage.h"
#include "mitkImageReadAccessor.h"
#include "mitkImagePixelAccessor.h"

namespace mitk {

class Image;

typedef itk::SmartPointer<mitk::Image> ImagePointer;

//##Documentation
//## @brief Gives locked and index-based read access for a particular image part.
//## The class provides several set- and get-methods, which allow an easy pixel access.
//## It needs to know about pixel type and dimension of its image at compile time.
//## @tparam TPixel defines the PixelType
//## @tparam VDimension defines the dimension for accessing data
//## @ingroup Data
template <class TPixel, unsigned int VDimension = 3>
class ImagePixelReadAccessor : public ImagePixelAccessor<TPixel, VDimension>
{
  friend class Image;

public:
   typedef ImagePixelAccessor<TPixel,VDimension> ImagePixelAccessorType;

  /** \brief Instantiates a mitk::ImageReadAccessor (see its doxygen page for more details)
     *  \param Image::Pointer specifies the associated Image
     *  \param ImageDataItem* specifies the allocated image part
     *  \param OptionFlags properties from mitk::ImageAccessorBase::Options can be chosen and assembled with bitwise unification.
     *  \throws mitk::Exception if the Constructor was created inappropriately
     *  \throws mitk::MemoryIsLockedException if requested image area is exclusively locked and mitk::ImageAccessorBase::ExceptionIfLocked is set in OptionFlags
     *
     *   Includes a check if typeid of PixelType coincides with templated TPixel
     *   and a check if VDimension equals to the Dimension of the Image.*/
  ImagePixelReadAccessor(
      ImagePointer iP,
      ImageDataItem* iDI = NULL,
      int OptionFlags = ImageAccessorBase::DefaultBehavior
      ) :
    ImagePixelAccessor<TPixel, VDimension>(iP,iDI),
    m_ReadAccessor(iP, iDI, OptionFlags)
  {

    // Check if Dimensions are correct
    if(ImagePixelAccessor<TPixel,VDimension>::m_ImageDataItem == NULL) {
      if(m_ReadAccessor.m_Image->GetDimension() != VDimension)
        mitkThrow() << "Invalid ImageAccessor: The Dimensions of ImageAccessor and Image are not equal. They have to be equal if an entire image is requested";
    }
    else {
      if(ImagePixelAccessor<TPixel,VDimension>::m_ImageDataItem->GetDimension() != VDimension)
        mitkThrow() << "Invalid ImageAccessor: The Dimensions of ImageAccessor and ImageDataItem are not equal.";
    }

    // Check if PixelType is correct
    if(!(m_ReadAccessor.m_Image->GetPixelType() ==  mitk::MakePixelType< itk::Image<TPixel, VDimension> >()) )
    {
      mitkThrow() << "Invalid ImageAccessor: PixelTypes of Image and ImageAccessor are not equal";
    }


  }

  /** Destructor informs Image to unlock memory. */
  virtual ~ImagePixelReadAccessor()
  {
  }

  /** Returns a const reference to the pixel at given index. */
  const TPixel & GetPixelByIndex(const itk::Index<VDimension>& idx) const
  {
     unsigned int offset = ImagePixelAccessorType::GetOffset(idx);

    return *(((TPixel*)m_ReadAccessor.m_AddressBegin) + offset);
  }

  /** Extends GetPixel by integrating index validation to prevent overflow.
    * \throws mitk::Exception in case of overflow
    */
  const TPixel & GetPixelByIndexSafe(const itk::Index<VDimension>& idx) const
  {
    unsigned int offset = ImagePixelAccessorType::GetOffset(idx);

    TPixel* targetAddress = ((TPixel*)m_ReadAccessor.m_AddressBegin) + offset;

    if(!(targetAddress >= m_ReadAccessor.m_AddressBegin && targetAddress < m_ReadAccessor.m_AddressEnd))
    {
      mitkThrow() << "ImageAccessor Overflow: image access exceeds the requested image area at " << idx << ".";
    }

    return *targetAddress;
  }


  /** Returns a const reference to the pixel at given world coordinate - works only with three-dimensional ImageAccessor */
  const TPixel & GetPixelByWorldCoordinates(mitk::Point3D position)
  {
    Index3D itkIndex;
    m_ReadAccessor.m_Image->GetGeometry()->WorldToIndex(position, itkIndex);

    return GetPixelByIndex( itkIndex);
  }


  /** Returns a const reference to the pixel at given world coordinate - works only with four-dimensional ImageAccessor */
  const TPixel & GetPixelByWorldCoordinates(mitk::Point3D position, unsigned int timestep)
  {
    Index3D itkIndex;
    m_ReadAccessor.m_Image->GetGeometry()->WorldToIndex(position, itkIndex);
    if (m_ReadAccessor.m_Image->GetTimeSteps() < timestep)
    {
      timestep = m_ReadAccessor.m_Image->GetTimeSteps();
    }
    itk::Index<4> itk4Index;
    for(int i=0; i<3; ++i)
      itk4Index[i] = itkIndex[i];

    itk4Index[3] = timestep;
    return GetPixelByIndex( itk4Index );
  }


  /** \brief Gives const access to the data. */
  virtual inline const TPixel * GetConstData()
  {
    return (TPixel*) m_ReadAccessor.m_AddressBegin;
  }

protected:

  // protected members

private:

  ImageReadAccessor m_ReadAccessor;

};

}

#endif // MITKIMAGEPIXELREADACCESSOR_H



