/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageWriteAccessor_h
#define mitkImageWriteAccessor_h

#include "mitkImage.h"
#include "mitkImageAccessorBase.h"

namespace mitk
{
  /**
   * @brief ImageWriteAccessor class to get locked write-access for a particular image part.
   * @ingroup Data
   */
  class MITKCORE_EXPORT ImageWriteAccessor : public ImageAccessorBase
  {
    template <class TPixel, unsigned int VDimension>
    friend class ImagePixelReadAccessor;

  public:
    typedef Image::Pointer ImagePointer;

    /** \brief Orders write access for a slice, volume or 4D-Image
     *  \param image specifies the associated Image
     *  \param iDI specifies the allocated image part
     *  \param OptionFlags properties from mitk::ImageAccessorBase::Options can be chosen and assembled with bitwise
     * unification.
     *  \throws mitk::Exception if the Constructor was created inappropriately
     *  \throws mitk::MemoryIsLockedException if requested image area is exclusively locked and
     * mitk::ImageAccessorBase::ExceptionIfLocked is set in OptionFlags
     */
    ImageWriteAccessor(ImagePointer image,
                       const ImageDataItem *iDI = nullptr,
                       int OptionFlags = ImageAccessorBase::DefaultBehavior);

    /** \brief Gives full data access. */
    inline void *GetData() { return m_AddressBegin; }
    /** \brief informs Image to unlock the represented image part */
    ~ImageWriteAccessor() override;

  protected:
    const Image *GetImage() const override;

  private:
    /** \brief manages a consistent write access and locks the ordered image part */
    void OrganizeWriteAccess();

    ImageWriteAccessor &operator=(const ImageWriteAccessor &); // Not implemented on purpose.
    ImageWriteAccessor(const ImageWriteAccessor &);

    ImagePointer m_Image;
  };
}
#endif
