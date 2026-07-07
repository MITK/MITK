/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageReadAccessor_h
#define mitkImageReadAccessor_h

#include <mitkImageAccessorBase.h>

namespace mitk
{
  class Image;
  class ImageDataItem;

  /**
   * \brief Provides locked read-only access to a particular region of image data.
   *
   * ImageReadAccessor acquires a read lock on the specified image data region
   * upon construction and releases it upon destruction (RAII pattern). Multiple
   * read accessors can co-exist for the same region, but a write accessor
   * for an overlapping region will either block or throw, depending on the
   * option flags.
   *
   * Use GetData() to obtain a const pointer to the raw data.
   *
   * \code
   * mitk::ImageReadAccessor accessor(image, image->GetSliceData(0));
   * const void* data = accessor.GetData();
   * \endcode
   *
   * \sa mitk::ImageWriteAccessor, mitk::ImagePixelReadAccessor
   * \sa mitk::ImageAccessorBase
   * \ingroup Data
   */
  class MITKCORE_EXPORT ImageReadAccessor : public ImageAccessorBase
  {
    friend class Image;

    template <class TPixel, unsigned int VDimension>
    friend class ImagePixelReadAccessor;

  public:
    /** \brief Smart pointer type for a non-const Image. */
    typedef itk::SmartPointer<Image> ImagePointer;

    /**
     * \brief Construct a read accessor for a slice, volume, or entire image.
     *
     * \param[in] image The image to access (const smart pointer).
     * \param[in] iDI Optional ImageDataItem specifying the data region. If nullptr,
     *            the entire first channel is accessed.
     * \param[in] OptionFlags Combination of mitk::ImageAccessorBase::Options flags.
     * \throw mitk::Exception if construction parameters are invalid.
     * \throw mitk::MemoryIsLockedException if the region is exclusively locked
     *        and ExceptionIfLocked is set.
     */
    ImageReadAccessor(ImageConstPointer image,
                      const ImageDataItem *iDI = nullptr,
                      int OptionFlags = ImageAccessorBase::DefaultBehavior);

    /**
     * \brief Construct a read accessor from a non-const image smart pointer.
     *
     * \param[in] image The image to access.
     * \param[in] iDI Optional ImageDataItem specifying the data region.
     * \param[in] OptionFlags Combination of mitk::ImageAccessorBase::Options flags.
     */
    ImageReadAccessor(ImagePointer image,
                      const ImageDataItem *iDI = nullptr,
                      int OptionFlags = ImageAccessorBase::DefaultBehavior);

    /**
     * \brief Construct a read accessor from a raw image pointer (default options).
     *
     * \param[in] image The image to access.
     * \param[in] iDI Optional ImageDataItem specifying the data region.
     */
    ImageReadAccessor(const Image *image, const ImageDataItem *iDI = nullptr);

    /**
     * \brief Destructor releases the read lock on the image data.
     */
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
