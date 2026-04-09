/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImage_h
#define mitkImage_h

#include <mitkBaseData.h>
#include <mitkImageAccessorBase.h>
#include <mitkImageDataItem.h>
#include <mitkImageDescriptor.h>
#include <mitkImageVtkWriteAccessor.h>
#include <mitkLevelWindow.h>
#include <mitkPlaneGeometry.h>
#include <mitkSlicedData.h>
#include <MitkCoreExports.h>
#include <mitkProportionalTimeGeometry.h>

#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

class vtkImageData;

namespace itk
{
  template <class T>
  class MutexLockHolder;
}

namespace mitk
{
  class SubImageSelector;
  class ImageTimeSelector;

  class ImageStatisticsHolder;

  /**
   * \brief Image class for storing multi-dimensional medical image data.
   *
   * Image is the primary data class for pixel-based image data in MITK. It can
   * be queried for header information, raw data vectors, or vtkImageData objects.
   * If access to only part of the data is required, use the appropriate accessor
   * classes (ImageReadAccessor, ImageWriteAccessor, ImagePixelReadAccessor,
   * ImagePixelWriteAccessor) rather than direct data access.
   *
   * Image organizes data hierarchically as:
   * - \b Slices (s x 2D): individual 2D planes
   * - \b Volumes (t x 3D): stacks of slices for each time step
   * - \b Channels (n x ND): separate data arrays per channel (e.g., morphology
   *   in channel 0, velocity in channel 1)
   *
   * All channels must share the same geometry (including dimensions). Only the
   * pixel type may differ between channels.
   *
   * For importing ITK images, use of mitk::ITKImageImport is recommended.
   * See \ref Adaptor for details.
   *
   * For more information, see \ref MitkImagePage.
   *
   * \sa mitk::SlicedData, mitk::BaseData
   * \sa mitk::ImageReadAccessor, mitk::ImageWriteAccessor
   * \sa mitk::ImagePixelReadAccessor, mitk::ImagePixelWriteAccessor
   * \sa mitk::ImageVtkReadAccessor, mitk::ImageVtkWriteAccessor
   * \ingroup Data
   */
  class MITKCORE_EXPORT Image : public SlicedData
  {
    friend class SubImageSelector;

    friend class ImageAccessorBase;
    friend class ImageVtkReadAccessor;
    friend class ImageVtkWriteAccessor;
    friend class ImageReadAccessor;
    friend class ImageWriteAccessor;

  public:
    mitkClassMacro(Image, SlicedData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Smart Pointer type to an ImageDataItem. */
    typedef itk::SmartPointer<ImageDataItem> ImageDataItemPointer;

    /** \brief Histogram type used for image statistics (double-precision). */
    typedef itk::Statistics::Histogram<double> HistogramType;

    /** \brief Pointer type to the ImageStatisticsHolder. */
    typedef mitk::ImageStatisticsHolder *StatisticsHolderPointer;

    /**
     * \brief Defines how imported memory is managed when setting data on an Image.
     *
     * This enum is evaluated by SetImportSlice, SetImportVolume, and
     * SetImportChannel to decide ownership of the provided data buffer.
     */
    enum ImportMemoryManagementType
    {
      CopyMemory, /**< \brief Data is copied to a new memory block owned by the Image. The original buffer is not freed by Image. */
      ManageMemory, /**< \brief Data is referenced by the Image. The Image takes ownership and will free the buffer on deletion. */
      ReferenceMemory, /**< \brief Data is referenced by the Image. The Image does \em not free the buffer on deletion; the caller retains ownership. */
      DontManageMemory = ReferenceMemory /**< \brief Alias for ReferenceMemory. */
    };

    /**
     * \brief Vector container of SmartPointers to ImageDataItems.
     *
     * For internal usage to allow convenient access to all slices via iterators.
     * \sa ImageDataItem
     */
    typedef std::vector<ImageDataItemPointer> ImageDataItemPointerArray;

  public:
    /**
     * \brief Returns the PixelType of channel \a n.
     *
     * \param[in] n The channel index (default: 0).
     * \return The PixelType describing the voxel data type and number of components.
     * \sa PixelType
     */
    const mitk::PixelType GetPixelType(int n = 0) const;

    /**
     * \brief Get the number of dimensions of the image.
     *
     * \return The image dimensionality (e.g., 2, 3, or 4).
     */
    unsigned int GetDimension() const;

    /**
     * \brief Get the size of dimension \a i.
     *
     * For example, i=0 gives the number of pixels in x-direction, i=1 in
     * y-direction, i=2 in z-direction (number of slices), and i=3 the
     * number of time steps.
     *
     * \param[in] i The dimension index.
     * \return The size of the requested dimension, or 1 if \a i is out of range.
     * \sa GetDimensions
     */
    unsigned int GetDimension(int i) const;

  public:
    /**
     * \brief Get a volume at a specific time step \a t of channel \a n as a vtkImageData.
     *
     * If the image is not initialized, the method will attempt to trigger an
     * update from the pipeline source.
     *
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return Pointer to the vtkImageData, or nullptr if the data is unavailable.
     * \sa ImageVtkReadAccessor, ImageVtkWriteAccessor
     */
    virtual vtkImageData *GetVtkImageData(int t = 0, int n = 0);

    /** \copydoc GetVtkImageData(int, int) */
    virtual const vtkImageData *GetVtkImageData(int t = 0, int n = 0) const;

    /**
     * \brief Check whether slice \a s at time \a t in channel \a n is set.
     *
     * \param[in] s Slice index (default: 0).
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return True if the slice data is available.
     */
    bool IsSliceSet(int s = 0, int t = 0, int n = 0) const override;

    /**
     * \brief Check whether the volume at time \a t in channel \a n is set.
     *
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return True if the volume data is available.
     */
    bool IsVolumeSet(int t = 0, int n = 0) const override;

    /**
     * \brief Check whether channel \a n is set.
     *
     * \param[in] n Channel number (default: 0).
     * \return True if the channel data is available.
     */
    bool IsChannelSet(int n = 0) const override;

    /**
     * \brief Set \a data as slice \a s at time \a t in channel \a n.
     *
     * The data is copied into an array managed by the Image. If the image
     * should reference external data instead, use SetImportSlice with
     * ImportMemoryManagementType set to ReferenceMemory.
     *
     * \pre The caller must ensure \a data points to a buffer at least as large
     *      as one slice.
     *
     * \param[in] data Pointer to the slice data.
     * \param[in] s Slice index (default: 0).
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return True on success.
     * \sa SetImportSlice, SetVolume
     */
    virtual bool SetSlice(const void *data, int s = 0, int t = 0, int n = 0);

    /**
     * \brief Set \a data as volume at time \a t in channel \a n.
     *
     * The data is copied into an array managed by the Image. If the image
     * should reference external data instead, use SetImportVolume with
     * ImportMemoryManagementType set to ReferenceMemory.
     *
     * \pre The caller must ensure \a data points to a buffer at least as large
     *      as one volume.
     *
     * \param[in] data Pointer to the volume data.
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return True on success.
     * \sa SetImportVolume, SetSlice
     */
    virtual bool SetVolume(const void *data, int t = 0, int n = 0);

    /**
     * \brief Set \a data as the data for channel \a n.
     *
     * The data is copied into an array managed by the Image. If the image
     * should reference external data instead, use SetImportChannel with
     * ImportMemoryManagementType set to ReferenceMemory.
     *
     * \pre The caller must ensure \a data points to a buffer at least as large
     *      as one channel.
     *
     * \param[in] data Pointer to the channel data.
     * \param[in] n Channel number (default: 0).
     * \return True on success.
     * \sa SetImportChannel
     */
    virtual bool SetChannel(const void *data, int n = 0);

    /**
     * \brief Set \a data as slice \a s at time \a t in channel \a n with
     * configurable memory management.
     *
     * \pre The caller must ensure \a data points to a buffer at least as large
     *      as one slice.
     *
     * \param[in] data Pointer to the slice data.
     * \param[in] s Slice index (default: 0).
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \param[in] importMemoryManagement Specifies how the data buffer is managed
     *            (default: CopyMemory).
     * \return True on success.
     * \sa SetSlice, ImportMemoryManagementType
     */
    virtual bool SetImportSlice(
      void *data, int s = 0, int t = 0, int n = 0, ImportMemoryManagementType importMemoryManagement = CopyMemory);

    /**
     * \brief Set \a data as volume at time \a t in channel \a n with
     * configurable memory management.
     *
     * \pre The caller must ensure \a data points to a buffer at least as large
     *      as one volume.
     *
     * \param[in] data Pointer to the volume data.
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \param[in] importMemoryManagement Specifies how the data buffer is managed
     *            (default: CopyMemory).
     * \return True on success.
     * \sa SetVolume, ImportMemoryManagementType
     */
    virtual bool SetImportVolume(void *data,
                                 int t = 0,
                                 int n = 0,
                                 ImportMemoryManagementType importMemoryManagement = CopyMemory);

    /**
     * \brief Set \a const_data as volume at time \a t in channel \a n (copy-only).
     *
     * This overload always copies the data (equivalent to CopyMemory).
     *
     * \param[in] const_data Pointer to the const volume data to copy.
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return True on success.
     */
    virtual bool SetImportVolume(const void *const_data, int t = 0, int n = 0);

    /**
     * \brief Allocate a zero-initialized volume at time \a t in channel \a n.
     *
     * The image must already be initialized with a pixel type and dimensions
     * (via any Initialize() overload) but must not yet have volume data set
     * for the given time step and channel.
     *
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \pre Image must be initialized (IsInitialized() == true).
     * \pre Image dimension must be between 1 and 3.
     * \pre No volume data may already exist at (t, n).
     * \throw mitk::Exception if any precondition is violated.
     */
    virtual void AllocateZeroedVolume(int t = 0, int n = 0);

    /**
     * \brief Set \a data as the data for channel \a n with configurable
     * memory management.
     *
     * \pre The caller must ensure \a data points to a buffer at least as large
     *      as one channel.
     *
     * \param[in] data Pointer to the channel data.
     * \param[in] n Channel number (default: 0).
     * \param[in] importMemoryManagement Specifies how the data buffer is managed
     *            (default: CopyMemory).
     * \return True on success.
     * \sa SetChannel, ImportMemoryManagementType
     */
    virtual bool SetImportChannel(void *data,
                                  int n = 0,
                                  ImportMemoryManagementType importMemoryManagement = CopyMemory);

    /**
     * \brief Initialize (or re-initialize) image with pixel type and dimensions.
     *
     * Creates a plane, evenly spaced geometry starting at origin (0,0,0).
     *
     * \param[in] type The pixel type for the image.
     * \param[in] dimension The number of dimensions (2, 3, or 4).
     * \param[in] dimensions Array of sizes for each dimension.
     * \param[in] channels Number of channels (default: 1).
     * \warning Assumes a plane, evenly spaced geometry starting at (0,0,0).
     */
    virtual void Initialize(const mitk::PixelType &type,
                            unsigned int dimension,
                            const unsigned int *dimensions,
                            unsigned int channels = 1);

    /**
     * \brief Initialize (or re-initialize) image from a BaseGeometry.
     *
     * \param[in] type The pixel type for the image.
     * \param[in] geometry The spatial geometry to use.
     * \param[in] channels Number of channels (default: 1).
     * \param[in] tDim Number of time steps (default: 1).
     */
    virtual void Initialize(const mitk::PixelType &type,
                            const mitk::BaseGeometry &geometry,
                            unsigned int channels = 1,
                            int tDim = 1);

    /**
     * \brief Initialize (or re-initialize) image from a TimeGeometry.
     *
     * \param[in] type The pixel type for the image.
     * \param[in] geometry The time geometry providing spatial and temporal information.
     * \param[in] channels Number of channels (default: 1).
     * \param[in] tDim Override the time dimension if the value is > 0 (default: -1,
     *            meaning use the number of time steps from the TimeGeometry).
     */
    virtual void Initialize(const mitk::PixelType &type,
                            const mitk::TimeGeometry &geometry,
                            unsigned int channels = 1,
                            int tDim = -1);

    /**
     * \brief Initialize (or re-initialize) image from a PlaneGeometry and number
     * of slices.
     *
     * The bounding box is initialized according to the width/height of the
     * PlaneGeometry and \a sDim via SlicedGeometry3D::InitializeEvenlySpaced.
     * The spacing is calculated from the PlaneGeometry.
     *
     * \param[in] type The pixel type for the image.
     * \param[in] sDim Number of slices.
     * \param[in] geometry2d The 2D plane geometry.
     * \param[in] channels Number of channels (default: 1).
     * \param[in] tDim Number of time steps (default: 1).
     * \sa SlicedGeometry3D::InitializeEvenlySpaced
     */
    virtual void Initialize(const mitk::PixelType &type,
                            int sDim,
                            const mitk::PlaneGeometry &geometry2d,
                            unsigned int channels = 1,
                            int tDim = 1);

    /**
     * \brief Initialize (or re-initialize) image from another mitk::Image.
     *
     * Only the header information is copied, not the data vector.
     *
     * \param[in] image The source image whose header to copy.
     */
    virtual void Initialize(const mitk::Image *image);

    /**
     * \brief Initialize (or re-initialize) image from an ImageDescriptor.
     *
     * \param[in] inDesc The image descriptor providing type and dimension information.
     */
    virtual void Initialize(const mitk::ImageDescriptor::Pointer inDesc);

    /**
     * \brief Initialize (or re-initialize) image from a vtkImageData.
     *
     * Only the header is used, not the data vector. Use
     * SetVolume(vtkimage->GetScalarPointer()) to transfer the pixel data.
     *
     * \param[in] vtkimagedata The VTK image providing header information.
     * \param[in] channels Number of channels (default: 1).
     * \param[in] tDim Override time dimension (default: -1, use VTK value).
     * \param[in] sDim Override z-space dimension (default: -1, use VTK value).
     * \param[in] pDim Override y-space dimension (default: -1, use VTK value).
     */
    virtual void Initialize(vtkImageData *vtkimagedata, int channels = 1, int tDim = -1, int sDim = -1, int pDim = -1);

    /**
     * \brief Initialize (or re-initialize) image from a templated ITK image.
     *
     * Only the header (spacing, origin, direction, dimensions) is used, not the
     * data vector. Use SetVolume(itkimage->GetBufferPointer()) to transfer data.
     *
     * \tparam itkImageType The ITK image type.
     * \param[in] itkimage The ITK image providing header information.
     * \param[in] channels Number of channels (default: 1).
     * \param[in] tDim Override time dimension (default: -1, use ITK value).
     * \param[in] sDim Override z-space dimension (default: -1, use ITK value).
     */
    template <typename itkImageType>
    void InitializeByItk(const itkImageType *itkimage, int channels = 1, int tDim = -1, int sDim = -1)
    {
      if (itkimage == nullptr)
        return;

      MITK_DEBUG << "Initializing MITK image from ITK image.";
      // build array with dimensions in each direction with at least 4 entries
      m_Dimension = itkimage->GetImageDimension();
      unsigned int i, *tmpDimensions = new unsigned int[m_Dimension > 4 ? m_Dimension : 4];
      for (i = 0; i < m_Dimension; ++i)
        tmpDimensions[i] = itkimage->GetLargestPossibleRegion().GetSize().GetSize()[i];
      if (m_Dimension < 4)
      {
        unsigned int *p;
        for (i = 0, p = tmpDimensions + m_Dimension; i < 4 - m_Dimension; ++i, ++p)
          *p = 1;
      }

      // overwrite number of slices if sDim is set
      if ((m_Dimension > 2) && (sDim >= 0))
        tmpDimensions[2] = sDim;
      // overwrite number of time points if tDim is set
      if ((m_Dimension > 3) && (tDim >= 0))
        tmpDimensions[3] = tDim;

      // rough initialization of Image
      // mitk::PixelType importType = ImportItkPixelType( itkimage::PixelType );

      Initialize(
        MakePixelType<itkImageType>(itkimage->GetNumberOfComponentsPerPixel()), m_Dimension, tmpDimensions, channels);
      const typename itkImageType::SpacingType &itkspacing = itkimage->GetSpacing();

      MITK_DEBUG << "ITK spacing " << itkspacing;
      // access spacing of itk::Image
      Vector3D spacing;
      FillVector3D(spacing, itkspacing[0], 1.0, 1.0);
      if (m_Dimension >= 2)
        spacing[1] = itkspacing[1];
      if (m_Dimension >= 3)
        spacing[2] = itkspacing[2];

      // access origin of itk::Image
      Point3D origin;
      const typename itkImageType::PointType &itkorigin = itkimage->GetOrigin();
      MITK_DEBUG << "ITK origin " << itkorigin;
      FillVector3D(origin, itkorigin[0], 0.0, 0.0);
      if (m_Dimension >= 2)
        origin[1] = itkorigin[1];
      if (m_Dimension >= 3)
        origin[2] = itkorigin[2];

      // access direction of itk::Imagm_PixelType = new mitk::PixelType(type);e and include spacing
      const typename itkImageType::DirectionType &itkdirection = itkimage->GetDirection();
      MITK_DEBUG << "ITK direction " << itkdirection;
      mitk::Matrix3D matrix;
      matrix.SetIdentity();
      unsigned int j, itkDimMax3 = (m_Dimension >= 3 ? 3 : m_Dimension);
      // check if spacing has no zero entry and itkdirection has no zero columns
      bool itkdirectionOk = true;
      mitk::ScalarType columnSum;
      for (j = 0; j < itkDimMax3; ++j)
      {
        columnSum = 0.0;
        for (i = 0; i < itkDimMax3; ++i)
        {
          columnSum += fabs(itkdirection[i][j]);
        }
        if (columnSum < mitk::eps)
        {
          itkdirectionOk = false;
        }
        if ((spacing[j] < -mitk::eps) // (normally sized) negative value
            &&
            (j == 2) && (m_Dimensions[2] == 1))
        {
          // Negative spacings can occur when reading single DICOM slices with ITK via GDCMIO
          // In these cases spacing is not determined by ITK correctly (because it distinguishes correctly
          // between slice thickness and inter slice distance -- slice distance is meaningless for
          // single slices).
          // I experienced that ITK produced something meaningful nonetheless because it is
          // evaluating the tag "(0018,0088) Spacing between slices" as a fallback. This tag is not
          // reliable (http://www.itk.org/pipermail/insight-users/2005-September/014711.html)
          // but gives at least a hint.
          // In real world cases I experienced that this tag contained the correct inter slice distance
          // with a negative sign, so we just invert such negative spacings.
          MITK_WARN << "Illegal value of itk::Image::GetSpacing()[" << j << "]=" << spacing[j]
                    << ". Using inverted value " << -spacing[j];
          spacing[j] = -spacing[j];
        }
        else if (spacing[j] < mitk::eps) // value near zero
        {
          MITK_ERROR << "Illegal value of itk::Image::GetSpacing()[" << j << "]=" << spacing[j]
                     << ". Using 1.0 instead.";
          spacing[j] = 1.0;
        }
      }
      if (itkdirectionOk == false)
      {
        MITK_ERROR << "Illegal matrix returned by itk::Image::GetDirection():" << itkdirection
                   << " Using identity instead.";
        for (i = 0; i < itkDimMax3; ++i)
          for (j = 0; j < itkDimMax3; ++j)
            if (i == j)
              matrix[i][j] = spacing[j];
            else
              matrix[i][j] = 0.0;
      }
      else
      {
        for (i = 0; i < itkDimMax3; ++i)
          for (j = 0; j < itkDimMax3; ++j)
            matrix[i][j] = itkdirection[i][j] * spacing[j];
      }

      // re-initialize PlaneGeometry with origin and direction
      PlaneGeometry *planeGeometry = static_cast<PlaneGeometry *>(GetSlicedGeometry(0)->GetPlaneGeometry(0));
      planeGeometry->SetOrigin(origin);
      planeGeometry->GetIndexToWorldTransform()->SetMatrix(matrix);

      // re-initialize SlicedGeometry3D
      SlicedGeometry3D *slicedGeometry = GetSlicedGeometry(0);
      slicedGeometry->InitializeEvenlySpaced(planeGeometry, m_Dimensions[2]);
      slicedGeometry->SetSpacing(spacing);

      // re-initialize TimeGeometry
      ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
      timeGeometry->Initialize(slicedGeometry, m_Dimensions[3]);
      SetTimeGeometry(timeGeometry);

      // clean-up
      delete[] tmpDimensions;

      this->Initialize();
    }

    /**
     * \brief Check whether slice \a s at time \a t in channel \a n is valid.
     *
     * A valid slice is one whose indices are within the image bounds.
     *
     * \param[in] s Slice index (default: 0).
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return True if the specified slice is within the image bounds.
     */
    virtual bool IsValidSlice(int s = 0, int t = 0, int n = 0) const;

    /**
     * \brief Check whether the volume at time \a t in channel \a n is valid.
     *
     * A valid volume has indices within the image bounds.
     *
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return True if the specified volume is within the image bounds.
     */
    virtual bool IsValidVolume(int t = 0, int n = 0) const;

    /**
     * \brief Check whether channel \a n is valid.
     *
     * A valid channel has an index within the image bounds.
     *
     * \param[in] n Channel number (default: 0).
     * \return True if the specified channel is within the image bounds.
     */
    virtual bool IsValidChannel(int n = 0) const;

    /**
     * \brief Check whether the image has a rotated geometry.
     *
     * Returns true if the geometry's transformation matrix has non-zero
     * off-diagonal elements larger than 1/1000 of the matrix trace.
     *
     * \return True if the image geometry is rotated.
     */
    bool IsRotated() const;

    /**
     * \brief Get the sizes of all dimensions as an integer array.
     *
     * \return Pointer to the internal array of dimension sizes.
     * \sa GetDimension(int)
     */
    unsigned int *GetDimensions() const;

    /**
     * \brief Get the image descriptor containing type and dimension information.
     *
     * \return Smart pointer to the ImageDescriptor.
     */
    ImageDescriptor::Pointer GetImageDescriptor() const { return m_ImageDescriptor; }

    /**
     * \brief Get the channel descriptor for channel \a id.
     *
     * \param[in] id The channel index (default: 0).
     * \return The ChannelDescriptor for the specified channel.
     */
    ChannelDescriptor GetChannelDescriptor(int id = 0) const { return m_ImageDescriptor->GetChannelDescriptor(id); }

    /**
     * \brief Set the geometry of the image.
     *
     * Overrides BaseData::SetGeometry to handle SlicedGeometry3D and
     * PlaneGeometry conversion.
     *
     * \param[in] aGeometry3D The geometry to set.
     */
    void SetGeometry(BaseGeometry *aGeometry3D) override;

    /**
     * \brief Get the ImageDataItem for slice \a s at time \a t in channel \a n.
     *
     * \warning For internal use only. Use ImageReadAccessor or ImageWriteAccessor
     * for safe data access.
     *
     * \param[in] s Slice index (default: 0).
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \param[in] data Optional pre-allocated data buffer (default: nullptr).
     * \param[in] importMemoryManagement Memory management policy (default: CopyMemory).
     * \return Smart pointer to the ImageDataItem, or nullptr if invalid.
     */
    virtual ImageDataItemPointer GetSliceData(int s = 0,
                                              int t = 0,
                                              int n = 0,
                                              void *data = nullptr,
                                              ImportMemoryManagementType importMemoryManagement = CopyMemory) const;

    /**
     * \brief Get the ImageDataItem for the volume at time \a t in channel \a n.
     *
     * \warning For internal use only. Use ImageReadAccessor or ImageWriteAccessor
     * for safe data access.
     *
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \param[in] data Optional pre-allocated data buffer (default: nullptr).
     * \param[in] importMemoryManagement Memory management policy (default: CopyMemory).
     * \return Smart pointer to the ImageDataItem, or nullptr if invalid.
     */
    virtual ImageDataItemPointer GetVolumeData(int t = 0,
                                               int n = 0,
                                               void *data = nullptr,
                                               ImportMemoryManagementType importMemoryManagement = CopyMemory) const;

    /**
     * \brief Get the ImageDataItem for channel \a n.
     *
     * \warning For internal use only. Use ImageReadAccessor or ImageWriteAccessor
     * for safe data access.
     *
     * \param[in] n Channel number (default: 0).
     * \param[in] data Optional pre-allocated data buffer (default: nullptr).
     * \param[in] importMemoryManagement Memory management policy (default: CopyMemory).
     * \return Smart pointer to the ImageDataItem, or nullptr if invalid.
     */
    virtual ImageDataItemPointer GetChannelData(int n = 0,
                                                void *data = nullptr,
                                                ImportMemoryManagementType importMemoryManagement = CopyMemory) const;

    /**
     * \brief Get a raw pointer to the volume data at \a t in channel \a n without locking.
     *
     * Returns the underlying voxel buffer for direct, unlocked access. No
     * ImageReadAccessor or ImageWriteAccessor is created and the
     * accessor lock list is left untouched. The caller is responsible
     * for ensuring no concurrent SetVolume(), SetSlice(), or
     * Initialize() call runs while the returned pointer is in use.
     *
     * Use the accessor classes (ImageReadAccessor, ImageWriteAccessor)
     * when lock-based concurrency control is required instead.
     *
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return Pointer to the start of the volume buffer, or \c nullptr
     *         if no volume data is available at (t, n).
     * \sa GetVolumeData, ImageReadAccessor, ImageWriteAccessor
     */
    void *GetData(int t = 0, int n = 0);

    /** \copydoc GetData(int, int) */
    const void *GetData(int t = 0, int n = 0) const;

    /**
     * \brief Get the ImageStatisticsHolder that provides statistics for this image.
     *
     * All statistics query methods (min, max, scalar 2nd, etc.) are available
     * through the returned ImageStatisticsHolder object.
     *
     * \return Pointer to the ImageStatisticsHolder.
     * \sa ImageStatisticsHolder
     */
    StatisticsHolderPointer GetStatistics() const { return m_ImageStatistics; }

  protected:
    mitkCloneMacro(Self);

    typedef std::lock_guard<std::mutex> MutexHolder;

    int GetSliceIndex(int s = 0, int t = 0, int n = 0) const;

    int GetVolumeIndex(int t = 0, int n = 0) const;

    void ComputeOffsetTable();

    virtual bool IsValidTimeStep(int t) const;

    void Expand(unsigned int timeSteps) override;

    virtual ImageDataItemPointer AllocateSliceData(
      int s = 0,
      int t = 0,
      int n = 0,
      void *data = nullptr,
      ImportMemoryManagementType importMemoryManagement = CopyMemory) const;

    virtual ImageDataItemPointer AllocateVolumeData(
      int t = 0, int n = 0, void *data = nullptr, ImportMemoryManagementType importMemoryManagement = CopyMemory) const;

    virtual ImageDataItemPointer AllocateChannelData(
      int n = 0, void *data = nullptr, ImportMemoryManagementType importMemoryManagement = CopyMemory) const;

    Image();

    Image(const Image &other);

    ~Image() override;

    void Clear() override;

    /** @warning Has to be called by every Initialize method! */
    void Initialize() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    mutable ImageDataItemPointerArray m_Channels;
    mutable ImageDataItemPointerArray m_Volumes;
    mutable ImageDataItemPointerArray m_Slices;
    mutable std::mutex m_ImageDataArraysLock;

    unsigned int m_Dimension;

    unsigned int *m_Dimensions;

    ImageDescriptor::Pointer m_ImageDescriptor;

    size_t *m_OffsetTable;
    ImageDataItemPointer m_CompleteData;

    // Image statistics Holder replaces the former implementation directly inside this class
    friend class ImageStatisticsHolder;
    StatisticsHolderPointer m_ImageStatistics;

  private:
    ImageDataItemPointer GetSliceData_unlocked(
      int s, int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const;
    ImageDataItemPointer GetVolumeData_unlocked(int t,
                                                int n,
                                                void *data,
                                                ImportMemoryManagementType importMemoryManagement) const;
    ImageDataItemPointer GetChannelData_unlocked(int n,
                                                 void *data,
                                                 ImportMemoryManagementType importMemoryManagement) const;

    ImageDataItemPointer AllocateSliceData_unlocked(
      int s, int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const;
    ImageDataItemPointer AllocateVolumeData_unlocked(int t,
                                                     int n,
                                                     void *data,
                                                     ImportMemoryManagementType importMemoryManagement) const;
    ImageDataItemPointer AllocateChannelData_unlocked(int n,
                                                      void *data,
                                                      ImportMemoryManagementType importMemoryManagement) const;

    bool IsSliceSet_unlocked(int s, int t, int n) const;
    bool IsVolumeSet_unlocked(int t, int n) const;
    bool IsChannelSet_unlocked(int n) const;

    /** Stores all existing ImageReadAccessors */
    mutable std::vector<ImageAccessorBase *> m_Readers;
    /** Stores all existing ImageWriteAccessors */
    mutable std::vector<ImageAccessorBase *> m_Writers;
    /** Stores all existing ImageVtkAccessors */
    mutable std::vector<ImageAccessorBase *> m_VtkReaders;

    /** A mutex, which needs to be locked to manage m_Readers and m_Writers */
    mutable std::mutex m_ReadWriteLock;
    /** A mutex, which needs to be locked to manage m_VtkReaders */
    mutable std::mutex m_VtkReadersLock;
  };


  using ImageDimensionVectorType = std::vector<unsigned int>;
  MITKCORE_EXPORT ImageDimensionVectorType DetermineImageDimensionsFromTimeGeometry(const TimeGeometry* timeGeometry);

  /**
  * @brief Equal A function comparing two images for being equal in meta- and imagedata
  *
  * @ingroup MITKTestingAPI
  *
  * Following aspects are tested for equality:
  *  - dimension of the images
  *  - size of the images
  *  - pixel type
  *  - pixel values : pixel values are expected to be identical at each position ( for other options see
  * mitk::CompareImageFilter )
  *
  * @param rightHandSide An image to be compared
  * @param leftHandSide An image to be compared
  * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return true, if all subsequent comparisons are true, false otherwise
  */
  MITKCORE_EXPORT bool Equal(const mitk::Image &leftHandSide,
                             const mitk::Image &rightHandSide,
                             ScalarType eps,
                             bool verbose);

} // namespace mitk

#endif
