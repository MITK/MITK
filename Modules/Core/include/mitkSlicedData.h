/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSlicedData_h
#define mitkSlicedData_h

#include <itkImageRegion.h>
#include <itkIndex.h>
#include <itkOffset.h>
#include <itkSize.h>
#include <mitkBaseData.h>
#include <mitkSlicedGeometry3D.h>
#include <MitkCoreExports.h>

namespace mitk
{
  class SlicedGeometry3D;

  /**
   * \brief Super class of data objects consisting of slices.
   *
   * SlicedData is the common base class for data objects that consist of slices,
   * such as images (mitk::Image) or stacks of contours. GetGeometry() returns a
   * BaseGeometry containing PlaneGeometry objects, and the data geometry is of
   * type SlicedGeometry3D or a sub-class thereof.
   *
   * SlicedData extends BaseData with region-based pipeline support using
   * itk::ImageRegion<5>, where the five dimensions represent x, y, slices (z),
   * time steps, and channels.
   *
   * \sa mitk::Image, mitk::BaseData
   * \sa mitk::SlicedGeometry3D, mitk::PlaneGeometry
   * \ingroup Data
   */
  class MITKCORE_EXPORT SlicedData : public BaseData
  {
  public:
    mitkClassMacro(SlicedData, BaseData);

    itkStaticConstMacro(RegionDimension, unsigned int, 5);

    /** Region typedef support. A region is used to specify a subset of a @a SlicedData. */
    typedef itk::ImageRegion<RegionDimension> RegionType;

    /** Index typedef support. An index is used to access pixel values. */
    typedef itk::Index<RegionDimension> IndexType;
    typedef IndexType::IndexValueType IndexValueType;

    /** Offset typedef support. An offset represent relative position
    * between indices. */
    typedef itk::Offset<RegionDimension> OffsetType;
    typedef OffsetType::OffsetValueType OffsetValueType;

    /** Size typedef support. A size is used to define region bounds. */
    typedef itk::Size<RegionDimension> SizeType;
    typedef SizeType::SizeValueType SizeValueType;

    /**
     * \brief Update the information for this DataObject so it can be used as
     * an output of a ProcessObject.
     *
     * Propagates information through the pipeline and initializes metadata.
     * Calls the source's UpdateOutputInformation() and, if there is no source,
     * sets the use-largest-possible-region flag. Also initializes the requested
     * region if it has not yet been set.
     */
    void UpdateOutputInformation() override;

    /**
     * \brief Prepare the data object to receive new data.
     *
     * Releases existing data if the pipeline time has changed, ensuring a
     * clean state for subsequent writes.
     */
    void PrepareForNewData() override;

    /**
     * \brief Set the RequestedRegion to the LargestPossibleRegion.
     *
     * Forces a filter to produce all of the output in one execution
     * (i.e., no streaming) on the next call to Update().
     */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Determine whether the RequestedRegion is outside of the BufferedRegion.
     *
     * Returns true if at least one requested slice, volume, or channel is not
     * currently buffered. Used by the pipeline mechanism to determine whether
     * re-execution is needed.
     *
     * \return True if the requested region is not fully contained in the buffered region.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Verify that the RequestedRegion is within the LargestPossibleRegion.
     *
     * If the RequestedRegion is not within the LargestPossibleRegion, the
     * filter cannot satisfy the request.
     *
     * \return True if the request can be satisfied, false otherwise.
     */
    bool VerifyRequestedRegion() override;

    /**
     * \brief Set the requested region from another data object.
     *
     * \param[in] data The data object whose requested region will be matched.
     *            Must be castable to SlicedData.
     * \throw itk::ExceptionObject if \a data cannot be cast to SlicedData.
     */
    void SetRequestedRegion(const itk::DataObject *data) override;

    /**
     * \brief Set the requested region from a RegionType pointer.
     *
     * \param[in] region Pointer to the region to use. Must not be nullptr.
     * \throw itk::ExceptionObject if \a region is nullptr.
     */
    virtual void SetRequestedRegion(SlicedData::RegionType *region);

    /**
     * \brief Set the largest possible region.
     *
     * The largest possible region is the entire region occupied by the data
     * object. The largest possible region should always be bigger than or equal
     * to the requested region of any operation.
     *
     * \param[in] region Pointer to the region to set. Must not be nullptr.
     * \throw itk::ExceptionObject if \a region is nullptr.
     */
    void SetLargestPossibleRegion(SlicedData::RegionType *region);

    /**
     * \brief Get the largest possible region.
     *
     * \return Const reference to the largest possible region.
     */
    const RegionType &GetLargestPossibleRegion() const { return m_LargestPossibleRegion; }

    /**
     * \brief Get the region object that defines the size and starting index
     * for the requested region (i.e., the part of the data to be operated on).
     *
     * \return Const reference to the requested region.
     */
    virtual const RegionType &GetRequestedRegion() const { return m_RequestedRegion; }

    /**
     * \brief Check whether slice \a s at time \a t in channel \a n is set.
     *
     * \param[in] s Slice index (default: 0).
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return True if the slice data is available.
     */
    virtual bool IsSliceSet(int s = 0, int t = 0, int n = 0) const = 0;

    /**
     * \brief Check whether volume at time \a t in channel \a n is set.
     *
     * \param[in] t Time step (default: 0).
     * \param[in] n Channel number (default: 0).
     * \return True if the volume data is available.
     */
    virtual bool IsVolumeSet(int t = 0, int n = 0) const = 0;

    /**
     * \brief Check whether channel \a n is set.
     *
     * \param[in] n Channel number (default: 0).
     * \return True if the channel data is available.
     */
    virtual bool IsChannelSet(int n = 0) const = 0;

    /**
     * \brief Copy information from the specified data set.
     *
     * Copies the TimeGeometry, PropertyList, and LargestPossibleRegion from
     * the given data object. Calls the superclass version.
     *
     * \param[in] data The data object to copy from. Must be castable to SlicedData.
     * \throw itk::ExceptionObject if \a data cannot be cast to SlicedData.
     */
    void CopyInformation(const itk::DataObject *data) override;

    /**
     * \brief Get the number of channels.
     *
     * The number of channels is derived from the fifth dimension of the
     * LargestPossibleRegion.
     *
     * \return The number of channels.
     */
    unsigned int GetNumberOfChannels() const { return m_LargestPossibleRegion.GetSize(4); }

    /**
     * \brief Convenience access method for the geometry, which is of type
     * SlicedGeometry3D (or a sub-class of it).
     *
     * \warning No update will be called. Normally used in GenerateOutputInformation
     * of subclasses of BaseProcess.
     *
     * \param[in] t The time step for which to retrieve the geometry (default: 0).
     * \return Pointer to the SlicedGeometry3D, or nullptr if the TimeGeometry is not set.
     * \sa GetUpdatedSlicedGeometry
     */
    SlicedGeometry3D *GetSlicedGeometry(unsigned int t = 0) const;

    /**
     * \brief Convenience access method that returns an up-to-date SlicedGeometry3D.
     *
     * Before returning, this method ensures the geometry is up-to-date by
     * calling SetRequestedRegionToLargestPossibleRegion() and
     * UpdateOutputInformation().
     *
     * \param[in] t The time step for which to retrieve the geometry (default: 0).
     * \return Const pointer to the up-to-date SlicedGeometry3D.
     * \sa GetSlicedGeometry
     * \todo Appropriate setting of the update extent is missing.
     */
    const SlicedGeometry3D *GetUpdatedSlicedGeometry(unsigned int t = 0);

    /**
     * \brief Set the BaseGeometry of the data, which will be referenced (not copied!).
     *
     * The geometry must be a SlicedGeometry3D or a PlaneGeometry that can be wrapped
     * in a SlicedGeometry3D. If a PlaneGeometry is provided, it will be used to
     * initialize an evenly spaced SlicedGeometry3D.
     *
     * \warning This method will normally be called internally by sub-classes of
     * SlicedData during initialization.
     *
     * \param[in] aGeometry3D The geometry to set. May be nullptr to clear the geometry.
     */
    void SetGeometry(BaseGeometry *aGeometry3D) override;

    /**
     * \brief Convenience method for setting the origin of the SlicedGeometry3D
     * instances of all time steps.
     *
     * In case the SlicedGeometry3D is evenly spaced, the origin of the first
     * slice is set to \a origin and the sliced geometry is re-initialized.
     *
     * \param[in] origin The new origin in world coordinates.
     * \sa mitk::BaseData::SetOrigin
     */
    void SetOrigin(const Point3D &origin) override;

    /**
     * \brief Convenience method for setting the spacing of the SlicedGeometry3D
     * instances of all time steps.
     *
     * \param[in] aSpacing Array of three spacing values [x, y, z].
     * \sa SetSpacing(mitk::Vector3D)
     */
    virtual void SetSpacing(const ScalarType aSpacing[]);

    /**
     * \brief Convenience method for setting the spacing of the SlicedGeometry3D
     * instances of all time steps.
     *
     * \param[in] aSpacing Vector of three spacing values [x, y, z].
     * \sa SetSpacing(const ScalarType[])
     */
    virtual void SetSpacing(mitk::Vector3D aSpacing);

  protected:
    SlicedData();
    SlicedData(const SlicedData &other);
    ~SlicedData() override;
    RegionType m_LargestPossibleRegion;

    mutable RegionType m_RequestedRegion;
    mutable bool m_RequestedRegionInitialized;

    RegionType m_BufferedRegion;
    bool m_UseLargestPossibleRegion;
  };

} // namespace mitk

#endif
