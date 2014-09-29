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

#ifndef MITKIMAGEREADACCESSOR_H
#define MITKIMAGEREADACCESSOR_H

#include "mitkImageAccessorBase.h"

namespace mitk {

class Image;
class ImageDataItem;


/**
 * @brief ImageReadAccessor class to get locked read access for a particular image part
 * @ingroup Data
 */
class MITK_CORE_EXPORT ImageReadAccessor : public ImageAccessorBase
{
  friend class Image;

  template <class TPixel, unsigned int VDimension>
  friend class ImagePixelReadAccessor;

public:

  typedef itk::SmartPointer<Image> ImagePointer;

  /** \brief Orders read access for a slice, volume or 4D-Image
   *  \param Image::Pointer specifies the associated Image
   *  \param ImageDataItem* specifies the allocated image part
   *  \param OptionFlags properties from mitk::ImageAccessorBase::Options can be chosen and assembled with bitwise unification.
   *  \throws mitk::Exception if the Constructor was created inappropriately
   *  \throws mitk::MemoryIsLockedException if requested image area is exclusively locked and mitk::ImageAccessorBase::ExceptionIfLocked is set in OptionFlags
   */
  ImageReadAccessor(
      ImageConstPointer image,
      const ImageDataItem* iDI = NULL,
      int OptionFlags = ImageAccessorBase::DefaultBehavior
      );

  ImageReadAccessor(
      ImagePointer image,
      const ImageDataItem* iDI = NULL,
      int OptionFlags = ImageAccessorBase::DefaultBehavior
      );

  ImageReadAccessor(const Image* image, const ImageDataItem* iDI = NULL);

  /** Destructor informs Image to unlock memory. */
  virtual ~ImageReadAccessor();

protected:

  virtual const Image* GetImage() const;

private:

  /** \brief manages a consistent read access and locks the ordered image part */
  void OrganizeReadAccess();

  ImageReadAccessor& operator=(const ImageReadAccessor&);  // Not implemented on purpose.
  ImageReadAccessor(const ImageReadAccessor&);

  ImageConstPointer m_Image;
};

}

#endif // MITKIMAGEACCESSOR_H



