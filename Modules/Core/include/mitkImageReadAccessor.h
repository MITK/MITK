/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageReadAccessor_h
#define mitkImageReadAccessor_h

#include "mitkImageAccessorBase.h"

namespace mitk
{
  class Image;
  class ImageDataItem;

  /**
   * @brief ImageReadAccessor class to get locked read access for a particular image part
   * @ingroup Data
   */
  class MITKCORE_EXPORT ImageReadAccessor : public ImageAccessorBase
  {
    friend class Image;

    template <class TPixel, unsigned int VDimension>
    friend class ImagePixelReadAccessor;

  public:
    typedef itk::SmartPointer<Image> ImagePointer;

    /** \brief Orders read access for a slice, volume or 4D-Image
     *  \param image specifies the associated Image
     *  \param iDI specifies the allocated image part
     *  \param OptionFlags properties from mitk::ImageAccessorBase::Options can be chosen and assembled with bitwise
     * unification.
     *  \throws mitk::Exception if the Constructor was created inappropriately
     *  \throws mitk::MemoryIsLockedException if requested image area is exclusively locked and
     * mitk::ImageAccessorBase::ExceptionIfLocked is set in OptionFlags
     */
    ImageReadAccessor(ImageConstPointer image,
                      const ImageDataItem *iDI = nullptr,
                      int OptionFlags = ImageAccessorBase::DefaultBehavior);

    ImageReadAccessor(ImagePointer image,
                      const ImageDataItem *iDI = nullptr,
                      int OptionFlags = ImageAccessorBase::DefaultBehavior);

    ImageReadAccessor(const Image *image, const ImageDataItem *iDI = nullptr);

    /** Destructor informs Image to unlock memory. */
    ~ImageReadAccessor() override;

  protected:
    const Image *GetImage() const override;

  private:
    /** \brief manages a consistent read access and locks the ordered image part */
    void OrganizeReadAccess();

    ImageReadAccessor &operator=(const ImageReadAccessor &); // Not implemented on purpose.
    ImageReadAccessor(const ImageReadAccessor &);

    ImageConstPointer m_Image;
  };
}

#endif
