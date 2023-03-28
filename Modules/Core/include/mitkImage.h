/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImage_h
#define mitkImage_h

#include "mitkBaseData.h"
#include "mitkImageAccessorBase.h"
#include "mitkImageDataItem.h"
#include "mitkImageDescriptor.h"
#include "mitkImageVtkAccessor.h"
#include "mitkLevelWindow.h"
#include "mitkPlaneGeometry.h"
#include "mitkSlicedData.h"
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
    * @brief Image class for storing images
    *
    * Can be asked for header information, the data vector, or vtkImageData objects.
    * If not the complete data is required, the appropriate SubImageSelector class
    * should be used for access.
    * Image organizes sets of slices (s x 2D), volumes (t x 3D) and channels (n
    * x ND). Channels are for different kind of data, e.g., morphology in
    * channel 0, velocities in channel 1. All channels must have the same Geometry! In
    * particular, the dimensions of all channels are the same, only the pixel-type
    * may differ between channels.
    *
    * For importing ITK images use of mitk::ITKImageImport is recommended, see
    * \ref Adaptor.
    *
    * For ITK v3.8 and older: Converting coordinates from the ITK physical
    * coordinate system (which does not support rotated images) to the MITK world
    * coordinate system should be performed via the BaseGeometry of the Image, see
    * BaseGeometry::WorldToItkPhysicalPoint.
    *
    * For more information, see \ref MitkImagePage .
    * @ingroup Data
    */
  class MITKCORE_EXPORT Image : public SlicedData
  {
    friend class SubImageSelector;

    friend class ImageAccessorBase;
    friend class ImageVtkAccessor;
    friend class ImageVtkReadAccessor;
    friend class ImageVtkWriteAccessor;
    friend class ImageReadAccessor;
    friend class ImageWriteAccessor;

  public:
    mitkClassMacro(Image, SlicedData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** Smart Pointer type to a ImageDataItem. */
    typedef itk::SmartPointer<ImageDataItem> ImageDataItemPointer;
    typedef itk::Statistics::Histogram<double> HistogramType;
    typedef mitk::ImageStatisticsHolder *StatisticsHolderPointer;

    /** This enum is evaluated when setting new data to an image.
      */
    enum ImportMemoryManagementType
    {
      CopyMemory, /**< Data to be set is copied and assigned to a new memory block. Data memory block will be freed on deletion of mitk::Image. */
      ManageMemory, /**< Data to be set will be referenced, and Data memory block will be freed on deletion of mitk::Image. */
      ReferenceMemory, /**< Data to be set will be referenced, but Data memory block will not be freed on deletion of mitk::Image. */
      DontManageMemory = ReferenceMemory
    };

    /**
      * @brief Vector container of SmartPointers to ImageDataItems;
      * Class is only for internal usage to allow convenient access to all slices over iterators;
      * See documentation of ImageDataItem for details.
      */
    typedef std::vector<ImageDataItemPointer> ImageDataItemPointerArray;

  public:
    /**
      * @brief Returns the PixelType of channel @a n.
      */
    const mitk::PixelType GetPixelType(int n = 0) const;

    /**
      * @brief Get dimension of the image
      */
    unsigned int GetDimension() const;

    /**
      * @brief Get the size of dimension @a i (e.g., i=0 results in the number of pixels in x-direction).
      *
      * @sa GetDimensions()
      */
    unsigned int GetDimension(int i) const;

  public:
    /**
      * @brief Get a volume at a specific time @a t of channel @a n as a vtkImageData.
      */
    virtual vtkImageData *GetVtkImageData(int t = 0, int n = 0);
    virtual const vtkImageData *GetVtkImageData(int t = 0, int n = 0) const;

    /**
      * @brief Check whether slice @a s at time @a t in channel @a n is set
      */
    bool IsSliceSet(int s = 0, int t = 0, int n = 0) const override;

    /**
      * @brief Check whether volume at time @a t in channel @a n is set
      */
    bool IsVolumeSet(int t = 0, int n = 0) const override;

    /**
      * @brief Check whether the channel @a n is set
      */
    bool IsChannelSet(int n = 0) const override;

    /**
      * @brief Set @a data as slice @a s at time @a t in channel @a n. It is in
      * the responsibility of the caller to ensure that the data vector @a data
      * is really a slice (at least is not smaller than a slice), since there is
      * no chance to check this.
      *
      * The data is copied to an array managed by the image. If the image shall
      * reference the data, use SetImportSlice with ImportMemoryManagementType
      * set to ReferenceMemory. For importing ITK images use of mitk::
      * ITKImageImport is recommended.
      * @sa SetPicSlice, SetImportSlice, SetImportVolume
      */
    virtual bool SetSlice(const void *data, int s = 0, int t = 0, int n = 0);

    /**
      * @brief Set @a data as volume at time @a t in channel @a n. It is in
      * the responsibility of the caller to ensure that the data vector @a data
      * is really a volume (at least is not smaller than a volume), since there is
      * no chance to check this.
      *
      * The data is copied to an array managed by the image. If the image shall
      * reference the data, use SetImportVolume with ImportMemoryManagementType
      * set to ReferenceMemory. For importing ITK images use of mitk::
      * ITKImageImport is recommended.
      * @sa SetPicVolume, SetImportVolume
      */
    virtual bool SetVolume(const void *data, int t = 0, int n = 0);

    /**
      * @brief Set @a data in channel @a n. It is in
      * the responsibility of the caller to ensure that the data vector @a data
      * is really a channel (at least is not smaller than a channel), since there is
      * no chance to check this.
      *
      * The data is copied to an array managed by the image. If the image shall
      * reference the data, use SetImportChannel with ImportMemoryManagementType
      * set to ReferenceMemory. For importing ITK images use of mitk::
      * ITKImageImport is recommended.
      * @sa SetPicChannel, SetImportChannel
      */
    virtual bool SetChannel(const void *data, int n = 0);

    /**
      * @brief Set @a data as slice @a s at time @a t in channel @a n. It is in
      * the responsibility of the caller to ensure that the data vector @a data
      * is really a slice (at least is not smaller than a slice), since there is
      * no chance to check this.
      *
      * The data is managed according to the parameter \a importMemoryManagement.
      * @sa SetPicSlice
      */
    virtual bool SetImportSlice(
      void *data, int s = 0, int t = 0, int n = 0, ImportMemoryManagementType importMemoryManagement = CopyMemory);

    /**
      * @brief Set @a data as volume at time @a t in channel @a n. It is in
      * the responsibility of the caller to ensure that the data vector @a data
      * is really a volume (at least is not smaller than a volume), since there is
      * no chance to check this.
      *
      * The data is managed according to the parameter \a importMemoryManagement.
      * @sa SetPicVolume
      */
    virtual bool SetImportVolume(void *data,
                                 int t = 0,
                                 int n = 0,
                                 ImportMemoryManagementType importMemoryManagement = CopyMemory);

    virtual bool SetImportVolume(const void *const_data, int t = 0, int n = 0);

    /**
      * @brief Set @a data in channel @a n. It is in
      * the responsibility of the caller to ensure that the data vector @a data
      * is really a channel (at least is not smaller than a channel), since there is
      * no chance to check this.
      *
      * The data is managed according to the parameter \a importMemoryManagement.
      * @sa SetPicChannel
      */
    virtual bool SetImportChannel(void *data,
                                  int n = 0,
                                  ImportMemoryManagementType importMemoryManagement = CopyMemory);

    /**
      * initialize new (or re-initialize) image information
      * @warning Initialize() by pic assumes a plane, evenly spaced geometry starting at (0,0,0).
      */
    virtual void Initialize(const mitk::PixelType &type,
                            unsigned int dimension,
                            const unsigned int *dimensions,
                            unsigned int channels = 1);

    /**
      * initialize new (or re-initialize) image information by a BaseGeometry
      *
      * \param type
      * \param geometry
      * \param channels
      * @param tDim defines the number of time steps for which the Image should be initialized
      */
    virtual void Initialize(const mitk::PixelType &type,
                            const mitk::BaseGeometry &geometry,
                            unsigned int channels = 1,
                            int tDim = 1);

    /**
    * \brief Initialize new (or re-initialize) image information by a TimeGeometry
    *
    * \param type
    * \param geometry
    * \param channels
    * \param tDim override time dimension if the value is bigger than 0 (Default -1)
    */
    virtual void Initialize(const mitk::PixelType &type,
                            const mitk::TimeGeometry &geometry,
                            unsigned int channels = 1,
                            int tDim = -1);

    /**
      * initialize new (or re-initialize) image information by a PlaneGeometry and number of slices
      *
      * Initializes the bounding box according to the width/height of the
      * PlaneGeometry and @a sDim via SlicedGeometry3D::InitializeEvenlySpaced.
      * The spacing is calculated from the PlaneGeometry.
      * \sa SlicedGeometry3D::InitializeEvenlySpaced
      */
    virtual void Initialize(const mitk::PixelType &type,
                            int sDim,
                            const mitk::PlaneGeometry &geometry2d,
                            unsigned int channels = 1,
                            int tDim = 1);

    /**
      * initialize new (or re-initialize) image information by another
      * mitk-image.
      * Only the header is used, not the data vector!
      */
    virtual void Initialize(const mitk::Image *image);

    virtual void Initialize(const mitk::ImageDescriptor::Pointer inDesc);

    /**
      * initialize new (or re-initialize) image information by @a vtkimagedata,
      * a vtk-image.
      * Only the header is used, not the data vector! Use
      * SetVolume(vtkimage->GetScalarPointer()) to set the data vector.
      *
      * @param vtkimagedata
      * @param channels
      * @param tDim override time dimension in @a vtkimagedata (if >0 and <)
      * @param sDim override z-space dimension in @a vtkimagedata (if >0 and <)
      * @param pDim override y-space dimension in @a vtkimagedata (if >0 and <)
      */
    virtual void Initialize(vtkImageData *vtkimagedata, int channels = 1, int tDim = -1, int sDim = -1, int pDim = -1);

    /**
      * initialize new (or re-initialize) image information by @a itkimage,
      * a templated itk-image.
      * Only the header is used, not the data vector! Use
      * SetVolume(itkimage->GetBufferPointer()) to set the data vector.
      *
      * @param itkimage
      * @param channels
      * @param tDim override time dimension in @a itkimage (if >0 and <)
      * @param sDim override z-space dimension in @a itkimage (if >0 and <)
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
      * @brief Check whether slice @a s at time @a t in channel @a n is valid, i.e.,
      * is (or can be) inside of the image
      */
    virtual bool IsValidSlice(int s = 0, int t = 0, int n = 0) const;

    /**
      * @brief Check whether volume at time @a t in channel @a n is valid, i.e.,
      * is (or can be) inside of the image
      */
    virtual bool IsValidVolume(int t = 0, int n = 0) const;

    /**
      * @brief Check whether the channel @a n is valid, i.e.,
      * is (or can be) inside of the image
      */
    virtual bool IsValidChannel(int n = 0) const;

    /**
      * @brief Returns true if an image is rotated, i.e. its geometry's
      * transformation matrix has nonzero elements besides the diagonal.
      * Non-diagonal elements are checked if larger then 1/1000 of the matrix' trace.
      */
    bool IsRotated() const;

    /**
      * @brief Get the sizes of all dimensions as an integer-array.
      *
      * @sa GetDimension(int i);
      */
    unsigned int *GetDimensions() const;

    ImageDescriptor::Pointer GetImageDescriptor() const { return m_ImageDescriptor; }
    ChannelDescriptor GetChannelDescriptor(int id = 0) const { return m_ImageDescriptor->GetChannelDescriptor(id); }
    /** \brief Sets a geometry to an image.
      */
    void SetGeometry(BaseGeometry *aGeometry3D) override;

    /**
    * @warning for internal use only
    */
    virtual ImageDataItemPointer GetSliceData(int s = 0,
                                              int t = 0,
                                              int n = 0,
                                              void *data = nullptr,
                                              ImportMemoryManagementType importMemoryManagement = CopyMemory) const;

    /**
    * @warning for internal use only
    */
    virtual ImageDataItemPointer GetVolumeData(int t = 0,
                                               int n = 0,
                                               void *data = nullptr,
                                               ImportMemoryManagementType importMemoryManagement = CopyMemory) const;

    /**
    * @warning for internal use only
    */
    virtual ImageDataItemPointer GetChannelData(int n = 0,
                                                void *data = nullptr,
                                                ImportMemoryManagementType importMemoryManagement = CopyMemory) const;

    /**
      \brief Returns a pointer to the ImageStatisticsHolder object that holds all statistics information for the image.

      All Get-methods for statistics properties formerly accessible directly from an Image object are now moved to the
      new \a ImageStatisticsHolder object.
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
