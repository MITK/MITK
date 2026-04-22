/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurface_h
#define mitkSurface_h

#include <itkImageRegion.h>
#include <mitkBaseData.h>
#include <vtkSmartPointer.h>

class vtkPolyData;

namespace mitk
{
  /**
   * \brief Data class for storing polygonal surfaces (vtkPolyData).
   *
   * Surface wraps a time series of vtkPolyData objects, providing one
   * vtkPolyData per time step. It integrates into the MITK pipeline by
   * implementing the region-based pipeline methods from BaseData and supports
   * undo/redo via ExecuteOperation.
   *
   * \sa mitk::BaseData, vtkPolyData
   * \sa mitk::SurfaceOperation
   * \ingroup Data
   */
  class MITKCORE_EXPORT Surface : public BaseData
  {
  public:
    /** \brief Region type used for pipeline region negotiation (5-dimensional). */
    typedef itk::ImageRegion<5> RegionType;

    mitkClassMacro(Surface, BaseData);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Compute the bounding box from all vtkPolyData objects across time steps.
     *
     * Updates the geometry bounds for each time step based on the vtkPolyData content.
     */
    void CalculateBoundingBox();

    /**
     * \brief Copy information (geometry, region) from another data object.
     *
     * \param[in] data The data object to copy from. Must be castable to Surface.
     * \throw mitk::Exception if \a data cannot be cast to Surface.
     */
    void CopyInformation(const itk::DataObject *data) override;

    /**
     * \brief Execute an operation on this surface (e.g., OpSURFACECHANGED).
     *
     * \param[in] operation The operation to execute. If the operation type is
     *            OpSURFACECHANGED, the contained vtkPolyData replaces the
     *            existing data at the specified time step.
     * \sa mitk::SurfaceOperation
     */
    void ExecuteOperation(Operation *operation) override;

    /**
     * \brief Expand the data to the given number of time steps.
     *
     * Resizes the internal vtkPolyData vector and the TimeGeometry.
     * Does nothing if the current size already equals or exceeds \a timeSteps.
     *
     * \param[in] timeSteps The desired number of time steps (default: 1).
     */
    void Expand(unsigned int timeSteps = 1) override;

    /**
     * \brief Get the largest possible region.
     *
     * \return Const reference to the largest possible region. The time dimension
     *         is set to the number of time steps.
     */
    const RegionType &GetLargestPossibleRegion() const;

    /**
     * \brief Get the requested region.
     *
     * \return Const reference to the currently requested region.
     */
    virtual const RegionType &GetRequestedRegion() const;

    /**
     * \brief Get the number of vtkPolyData objects in the time series.
     *
     * \return The number of time steps stored.
     */
    unsigned int GetSizeOfPolyDataSeries() const;

    /**
     * \brief Get the vtkPolyData at time step \a t.
     *
     * If the data is not available but a source exists, the pipeline is updated
     * to produce the requested time step.
     *
     * \param[in] t Time step (default: 0).
     * \return Pointer to the vtkPolyData, or nullptr if not available.
     */
    virtual vtkPolyData *GetVtkPolyData(unsigned int t = 0) const;

    /**
     * \brief Graft data and information from another data object.
     *
     * Deep-copies all vtkPolyData objects from the source surface.
     *
     * \param[in] data The data object to graft from. Must be castable to Surface.
     * \throw mitk::Exception if \a data cannot be cast to Surface.
     */
    void Graft(const DataObject *data) override;

    /**
     * \brief Check whether time step \a t is empty (has no vtkPolyData).
     *
     * \param[in] t The time step to check.
     * \return True if the vtkPolyData at time step \a t is nullptr.
     */
    bool IsEmptyTimeStep(unsigned int t) const override;

    /**
     * \brief Print the surface information to the output stream.
     *
     * \param[in,out] os The output stream.
     * \param[in] indent The indentation level.
     */
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    /**
     * \brief Determine whether the requested region is outside the buffered region.
     *
     * \return True if any requested time step has no vtkPolyData.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Set the requested region to match another data object's requested region.
     *
     * \param[in] data The data object whose region to match. Must be castable to Surface.
     * \throw mitk::Exception if \a data cannot be cast to Surface.
     */
    void SetRequestedRegion(const itk::DataObject *data) override;

    /**
     * \brief Set the requested region from a RegionType pointer.
     *
     * \param[in] region Pointer to the region. Must not be nullptr.
     * \throw mitk::Exception if \a region is nullptr.
     */
    virtual void SetRequestedRegion(Surface::RegionType *region);

    /**
     * \brief Set the requested region to the largest possible region.
     */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Set the vtkPolyData for time step \a t.
     *
     * The surface takes a reference to the provided polydata. If the same
     * polydata object is already set at the given time step, no action is taken.
     *
     * \param[in] polydata The vtkPolyData to set.
     * \param[in] t Time step (default: 0). The surface is expanded if necessary.
     */
    virtual void SetVtkPolyData(vtkPolyData *polydata, unsigned int t = 0);

    /**
     * \brief Swap the contents of this surface with another surface.
     *
     * \param[in,out] other The surface to swap with.
     */
    virtual void Swap(Surface &other);

    /**
     * \brief Trigger an update of all vtkPolyData objects in the pipeline.
     */
    void Update() override;

    /**
     * \brief Update the output information (bounding box, geometry).
     */
    void UpdateOutputInformation() override;

    /**
     * \brief Verify that the requested region is within the largest possible region.
     *
     * \return True if the requested region is valid.
     */
    bool VerifyRequestedRegion() override;

  protected:
    mitkCloneMacro(Self);

    Surface();
    ~Surface() override;

    Surface(const Surface &other);
    Surface &operator=(Surface other);

    void ClearData() override;
    void InitializeEmpty() override;

  private:
    std::vector<vtkSmartPointer<vtkPolyData>> m_PolyDatas;
    mutable RegionType m_LargestPossibleRegion;
    mutable RegionType m_RequestedRegion;
    bool m_CalculateBoundingBox;
  };

  /**
  * @brief Equal Compare two surfaces for equality, returns true if found equal.
  * @ingroup MITKTestingAPI
  * @param rightHandSide Surface to compare.
  * @param leftHandSide Surface to compare.
  * @param eps Epsilon to use for floating point comparison. Most of the time mitk::eps will be sufficient.
  * @param verbose Flag indicating if the method should give a detailed console output.
  * @return True if every comparison is true, false in any other case.
  */
  MITKCORE_EXPORT bool Equal(mitk::Surface &leftHandSide,
                             mitk::Surface &rightHandSide,
                             mitk::ScalarType eps,
                             bool verbose);

  /**
  * @brief Equal Compare two vtk PolyDatas for equality, returns true if found equal.
  * @ingroup MITKTestingAPI
  * @param rightHandSide Surface to compare.
  * @param leftHandSide Surface to compare.
  * @param eps Epsilon to use for floating point comparison. Most of the time mitk::eps will be sufficient.
  * @param verbose Flag indicating if the method should give a detailed console output.
  * @return True if every comparison is true, false in any other case.
  *
  * This will only check if the number of cells, vertices, polygons, stripes and lines is the same and whether
  * all the two poly datas have the same number of points with the same coordinates. It is not checked whether
  * all points are correctly connected.
  */
  MITKCORE_EXPORT bool Equal(vtkPolyData &leftHandSide, vtkPolyData &rightHandSide, mitk::ScalarType eps, bool verbose);
}

#endif
