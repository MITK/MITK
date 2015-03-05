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

#include "mitkImageReadAccessor.h"
#include "mitkImagePixelAccessor.h"

namespace mitk {

class Image;

/**
 * @brief Gives locked and index-based read access for a particular image part.
 * The class provides several set- and get-methods, which allow an easy pixel access.
 * It needs to know about pixel type and dimension of its image at compile time.
 * @tparam TPixel defines the PixelType
 * @tparam VDimension defines the dimension for accessing data
 * @ingroup Data
 */
template <class TPixel, unsigned int VDimension = 3>
class ImagePixelReadAccessor : public ImagePixelAccessor<TPixel, VDimension>
{
  friend class Image;

public:

  typedef ImagePixelAccessor<TPixel,VDimension> ImagePixelAccessorType;
  typedef itk::SmartPointer<mitk::Image> ImagePointer;
  typedef itk::SmartPointer<const mitk::Image> ImageConstPointer;

  /** \brief Instantiates a mitk::ImageReadAccessor (see its doxygen page for more details)
   *  \param Image::Pointer specifies the associated Image
   *  \param ImageDataItem* specifies the allocated image part
   *  \param OptionFlags properties from mitk::ImageAccessorBase::Options can be chosen and assembled with bitwise unification.
   *  \throws mitk::Exception if the Constructor was created inappropriately
   *  \throws mitk::MemoryIsLockedException if requested image area is exclusively locked and mitk::ImageAccessorBase::ExceptionIfLocked is set in OptionFlags
   *
   *   Includes a check if typeid of PixelType coincides with templated TPixel
   *   and a check if VDimension equals to the Dimension of the Image.
   */
  ImagePixelReadAccessor(
      ImageConstPointer iP,
      const ImageDataItem* iDI = NULL,
      int OptionFlags = ImageAccessorBase::DefaultBehavior
      )
    : ImagePixelAccessor<TPixel, VDimension>(iP,iDI)
    , m_ReadAccessor(iP, iDI, OptionFlags)
  {
  }

  ImagePixelReadAccessor(
      ImagePointer iP,
      const ImageDataItem* iDI = NULL,
      int OptionFlags = ImageAccessorBase::DefaultBehavior
      )
    : ImagePixelAccessor<TPixel, VDimension>(iP.GetPointer(),iDI)
    , m_ReadAccessor(iP, iDI, OptionFlags)
  {
  }

  ImagePixelReadAccessor(
      Image* iP,
      const ImageDataItem* iDI = NULL,
      int OptionFlags = ImageAccessorBase::DefaultBehavior
      )
    : ImagePixelAccessor<TPixel, VDimension>(iP,iDI)
    , m_ReadAccessor(mitk::Image::ConstPointer(iP), iDI, OptionFlags)
  {
  }

  ImagePixelReadAccessor(
      const Image* iP,
      const ImageDataItem* iDI = NULL,
      int OptionFlags = ImageAccessorBase::DefaultBehavior
      )
    : ImagePixelAccessor<TPixel, VDimension>(iP,iDI)
    , m_ReadAccessor(iP, iDI, OptionFlags)
  {
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
    itk::Index<3> itkIndex;
    m_ReadAccessor.GetImage()->GetGeometry()->WorldToIndex(position, itkIndex);

    return GetPixelByIndex( itkIndex);
  }


  /** Returns a const reference to the pixel at given world coordinate - works only with four-dimensional ImageAccessor */
  const TPixel & GetPixelByWorldCoordinates(mitk::Point3D position, unsigned int timestep)
  {
    itk::Index<3> itkIndex;
    m_ReadAccessor.GetImage()->GetGeometry()->WorldToIndex(position, itkIndex);
    if (m_ReadAccessor.GetImage()->GetTimeSteps() < timestep)
    {
      timestep = m_ReadAccessor.GetImage()->GetTimeSteps();
    }
    itk::Index<4> itk4Index;
    for(int i=0; i<3; ++i)
      itk4Index[i] = itkIndex[i];

    itk4Index[3] = timestep;
    return GetPixelByIndex( itk4Index );
  }


  /** \brief Gives const access to the data. */
  inline const TPixel* GetData() const
  {
    return static_cast<const TPixel*>(m_ReadAccessor.m_AddressBegin);
  }

protected:

  // protected members

private:

  ImageReadAccessor m_ReadAccessor;

  ImagePixelReadAccessor& operator=(const ImagePixelReadAccessor&);  // Not implemented on purpose.
  ImagePixelReadAccessor(const ImagePixelReadAccessor&);

};

}

#endif // MITKIMAGEPIXELREADACCESSOR_H



