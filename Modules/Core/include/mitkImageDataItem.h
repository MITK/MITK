/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageDataItem_h
#define mitkImageDataItem_h

#include <mitkCommon.h>
#include <MitkCoreExports.h>
#include <mitkImageDescriptor.h>

class vtkImageData;

namespace mitk
{
  class PixelType;
  class ImageVtkReadAccessor;
  class ImageVtkWriteAccessor;

  class Image;

  /**
   * \brief Internal container for image data used by mitk::Image.
   *
   * ImageDataItem manages raw pixel buffers and bridges between MITK's Image
   * representation and VTK's vtkImageData. It is primarily used to represent
   * sub-images (e.g. individual slices or volumes of a time series) inside
   * mitk::Image.
   *
   * This class is an implementation detail; it should not be used directly
   * outside of mitk::Image and its accessor classes.
   *
   * \sa Image, ImageReadAccessor, ImageWriteAccessor, ImageVtkReadAccessor
   * \ingroup Data
   */
  class MITKCORE_EXPORT ImageDataItem : public itk::LightObject
  {
    friend class ImageAccessorBase;
    friend class ImageWriteAccessor;
    friend class ImageReadAccessor;

    template <class TPixel, unsigned int VDimension>
    friend class ImagePixelAccessor;

    friend class Image;

    //  template<class TOutputImage>
    //  friend class ImageToItk;

  public:
    typedef itk::SmartPointer<mitk::Image> ImagePointer;           ///< Smart pointer to Image.
    typedef itk::SmartPointer<const mitk::Image> ImageConstPointer; ///< Smart pointer to const Image.

    mitkClassMacroItkParent(ImageDataItem, itk::LightObject);

    /**
     * \brief Construct a sub-image data item as a child of an existing item.
     *
     * The child shares the parent's data buffer at the given offset.
     *
     * \param[in] aParent       The parent ImageDataItem whose buffer is shared.
     * \param[in] desc          Image descriptor providing dimension information.
     * \param[in] timestep      Time step index associated with this item.
     * \param[in] dimension     Number of dimensions for this sub-image.
     * \param[in] data          Optional external data pointer. If non-null and different
     *                          from the computed parent offset, data is memcpy'd.
     * \param[in] manageMemory  If true and \p data is provided, the external buffer
     *                          is deleted after copying.
     * \param[in] offset        Byte offset into the parent's data buffer.
     */
    ImageDataItem(const ImageDataItem &aParent,
                  const mitk::ImageDescriptor::Pointer desc,
                  int timestep,
                  unsigned int dimension,
                  void *data = nullptr,
                  bool manageMemory = false,
                  size_t offset = 0);

    /** \brief Destructor. Frees managed memory and VTK resources. */
    ~ImageDataItem() override;

    /**
     * \brief Construct a top-level data item from an image descriptor.
     *
     * Allocates a new data buffer if \p data is nullptr.
     *
     * \param[in] desc          Image descriptor providing pixel type and dimensions.
     * \param[in] timestep      Time step index associated with this item.
     * \param[in] data          Optional external data buffer.
     * \param[in] manageMemory  If true, the provided buffer will be deleted on destruction.
     */
    ImageDataItem(const mitk::ImageDescriptor::Pointer desc, int timestep, void *data, bool manageMemory);

    /**
     * \brief Construct a data item from explicit pixel type and dimension information.
     *
     * Allocates a new data buffer if \p data is nullptr.
     *
     * \param[in] type          The pixel type.
     * \param[in] timestep      Time step index.
     * \param[in] dimension     Number of spatial dimensions.
     * \param[in] dimensions    Array of dimension sizes (must have at least \p dimension elements).
     * \param[in] data          Optional external data buffer.
     * \param[in] manageMemory  If true, the provided buffer will be deleted on destruction.
     */
    ImageDataItem(const mitk::PixelType &type,
                  int timestep,
                  unsigned int dimension,
                  unsigned int *dimensions,
                  void *data,
                  bool manageMemory);

    ImageDataItem(const ImageDataItem &other) = delete;
    ImageDataItem &operator=(const ImageDataItem &other) = delete;

    /**
     * \brief Check whether all sub-items of this data item have been filled.
     * \return True if complete.
     */
    bool IsComplete() const { return m_IsComplete; }

    /**
     * \brief Set the completion status of this data item.
     * \param[in] complete True if all data has been loaded.
     */
    void SetComplete(bool complete) { m_IsComplete = complete; }

    /**
     * \brief Get the byte offset of this item within its parent's buffer.
     * \return The offset in bytes, or 0 for top-level items.
     */
    int GetOffset() const { return m_Offset; }

    /**
     * \brief Get the pixel type of this data item.
     * \return A copy of the PixelType.
     */
    PixelType GetPixelType() const { return *m_PixelType; }

    /**
     * \brief Set the time step index for this data item.
     * \param[in] t The time step index.
     */
    void SetTimestep(int t) { m_Timestep = t; }

    /**
     * \brief Set whether this item owns (and will delete) its data buffer.
     * \param[in] b True if this item should manage the memory.
     */
    void SetManageMemory(bool b) { m_ManageMemory = b; }

    /**
     * \brief Get the number of spatial dimensions.
     * \return The dimension count.
     */
    int GetDimension() const { return m_Dimension; }

    /**
     * \brief Get the size along a specific dimension.
     * \param[in] i The dimension index (0-based).
     * \return The size along dimension \p i, or 0 if \p i >= GetDimension().
     */
    int GetDimension(int i) const
    {
      int returnValue = 0;

      // return the true size if dimension available
      if (i < (int)m_Dimension)
        returnValue = m_Dimensions[i];

      return returnValue;
    }

    /**
     * \brief Get the parent data item, if any.
     * \return Smart pointer to the parent, or null for top-level items.
     */
    ImageDataItem::ConstPointer GetParent() const { return m_Parent; }

    /**
     * \brief Get or create a VTK read accessor for this data item.
     *
     * If no vtkImageData has been constructed yet, one is created via
     * ConstructVtkImageData(). For 3D images the VTK origin is set to
     * (0, 0, 0) for historical compatibility (see MITK bug 5050).
     *
     * \param[in] iP Smart pointer to the owning Image (used for geometry queries).
     * \return Pointer to an ImageVtkReadAccessor wrapping the VTK data.
     */
    ImageVtkReadAccessor *GetVtkImageAccessor(ImageConstPointer iP) const;

    /**
     * \brief Get or create a VTK write accessor for this data item.
     * \param[in] iP Smart pointer to the owning Image.
     * \return Pointer to an ImageVtkWriteAccessor wrapping the VTK data.
     */
    ImageVtkWriteAccessor *GetVtkImageAccessor(ImagePointer iP);

    /**
     * \brief Query whether this item will delete its data buffer on destruction.
     * \return True if memory is managed by this item.
     */
    bool GetManageMemory() const { return m_ManageMemory; }

    /**
     * \brief Build a vtkImageData representation from the raw pixel buffer.
     *
     * Sets up dimensions, spacing, origin, and scalar arrays matching the
     * pixel type and dimension information of this item.
     *
     * \param[in] iP Smart pointer to the owning Image (used for spacing).
     */
    virtual void ConstructVtkImageData(ImageConstPointer iP) const;

    /**
     * \brief Get the total size of this data item in bytes.
     * \return The byte size (pixel size * number of pixels).
     */
    size_t GetSize() const { return m_Size; }

    /**
     * \brief Mark the internal vtkImageData as modified.
     *
     * Call this after changing the raw data buffer so that VTK pipelines
     * recognize the update.
     */
    virtual void Modified() const;

  protected:
    /**Helper function to allow friend classes to access m_Data without changing their code.
    * Moved to protected visibility because only friends are allowed to access m_Data directly.
    * Other classes should used ImageWriteAccessor::GetData() or ImageReadAccessor::GetData()
    * to get access.*/
    void* GetData() const { return m_Data; }

    unsigned char *m_Data;

    PixelType *m_PixelType;

    bool m_ManageMemory;

    mutable vtkImageData *m_VtkImageData;
    mutable ImageVtkReadAccessor *m_VtkImageReadAccessor;
    ImageVtkWriteAccessor *m_VtkImageWriteAccessor;
    int m_Offset;

    bool m_IsComplete;

    size_t m_Size;

  private:
    void ComputeItemSize(const unsigned int *dimensions, unsigned int dimension);

    ImageDataItem::ConstPointer m_Parent;

    unsigned int m_Dimension;

    unsigned int m_Dimensions[MAX_IMAGE_DIMENSIONS];

    int m_Timestep;
  };

} // namespace mitk

#endif
