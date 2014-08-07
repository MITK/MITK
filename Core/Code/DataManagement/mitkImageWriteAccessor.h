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

#ifndef MITKIMAGEWRITEACCESSOR_H
#define MITKIMAGEWRITEACCESSOR_H

#include "mitkImageAccessorBase.h"
#include "mitkImage.h"

namespace mitk {

/**
 * @brief ImageWriteAccessor class to get locked write-access for a particular image part.
 * @ingroup Data
 */
class MITK_CORE_EXPORT ImageWriteAccessor : public ImageAccessorBase
{

  template <class TPixel, unsigned int VDimension>
  friend class ImagePixelReadAccessor;

public:

  typedef Image::Pointer ImagePointer;

  /** \brief Orders write access for a slice, volume or 4D-Image
   *  \param Image::Pointer specifies the associated Image
   *  \param ImageDataItem* specifies the allocated image part
   *  \param OptionFlags properties from mitk::ImageAccessorBase::Options can be chosen and assembled with bitwise unification.
   *  \throws mitk::Exception if the Constructor was created inappropriately
   *  \throws mitk::MemoryIsLockedException if requested image area is exclusively locked and mitk::ImageAccessorBase::ExceptionIfLocked is set in OptionFlags
   */
  ImageWriteAccessor(
      ImagePointer image,
      const ImageDataItem* iDI = NULL,
      int OptionFlags = ImageAccessorBase::DefaultBehavior
      );

  /** \brief Gives full data access. */
  inline void * GetData()
  {
    return m_AddressBegin;
  }

  /** \brief informs Image to unlock the represented image part */
  virtual ~ImageWriteAccessor();

protected:

  virtual const Image* GetImage() const;

private:

  /** \brief manages a consistent write access and locks the ordered image part */
  void OrganizeWriteAccess();

  ImageWriteAccessor& operator=(const ImageWriteAccessor&);  // Not implemented on purpose.
  ImageWriteAccessor(const ImageWriteAccessor&);

  ImagePointer m_Image;
};

}
#endif // MITKIMAGEWRITEACCESSOR_H
