/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageWriteAccessor_h
#define mitkImageWriteAccessor_h

#include <mitkImage.h>
#include <mitkImageAccessorBase.h>

namespace mitk
{
  /**
   * \brief Provides locked write access to a particular region of image data.
   *
   * ImageWriteAccessor acquires an exclusive write lock on the specified image
   * data region upon construction and releases it upon destruction (RAII pattern).
   * While a write accessor exists, no other read or write accessor can access an
   * overlapping region (they will either block or throw, depending on option flags).
   *
   * Use GetData() to obtain a non-const pointer to the raw data for writing.
   *
   * \code
   * mitk::ImageWriteAccessor accessor(image);
   * void* data = accessor.GetData();
   * // ... modify data ...
   * \endcode
   *
   * \note After writing, you should call image->Modified() to notify the pipeline
   * that the data has changed.
   *
   * \sa mitk::ImageReadAccessor, mitk::ImagePixelWriteAccessor
   * \sa mitk::ImageAccessorBase
   * \ingroup Data
   */
  class MITKCORE_EXPORT ImageWriteAccessor : public ImageAccessorBase
  {
    template <class TPixel, unsigned int VDimension>
    friend class ImagePixelReadAccessor;

  public:
    /** \brief Smart pointer type for the associated Image. */
    typedef Image::Pointer ImagePointer;

    /**
     * \brief Construct a write accessor for a slice, volume, or entire image.
     *
     * \param[in] image The image to access (must be non-const).
     * \param[in] iDI Optional ImageDataItem specifying the data region. If nullptr,
     *            the entire first channel is accessed.
     * \param[in] OptionFlags Combination of mitk::ImageAccessorBase::Options flags.
     * \throw mitk::Exception if construction parameters are invalid.
     * \throw mitk::MemoryIsLockedException if the region is exclusively locked
     *        and ExceptionIfLocked is set.
     */
    ImageWriteAccessor(ImagePointer image,
                       const ImageDataItem *iDI = nullptr,
                       int OptionFlags = ImageAccessorBase::DefaultBehavior);

    /**
     * \brief Get a non-const pointer to the raw image data for writing.
     *
     * \return Pointer to the beginning of the writable memory region.
     */
    inline void *GetData() { return m_AddressBegin; }

    /**
     * \brief Destructor releases the write lock on the image data.
     */
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
