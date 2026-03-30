/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUnstructuredGrid_h
#define mitkUnstructuredGrid_h

#include <MitkDataTypesExtExports.h>
#include <itkImageRegion.h>
#include <mitkBaseData.h>

#include <vtkSmartPointer.h>

class vtkUnstructuredGrid;

namespace mitk
{
  /**
   * \brief MITK data wrapper for vtkUnstructuredGrid with time-step support.
   *
   * Stores a time series of vtkUnstructuredGrid instances and provides the
   * standard BaseData pipeline interface (requested regions, bounding box
   * computation, grafting).
   *
   * \sa BaseData, Surface, UnstructuredGridSource
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT UnstructuredGrid : public BaseData
  {
  public:
    /** \brief Region type used for pipeline region negotiation. */
    typedef itk::ImageRegion<5> RegionType;

    mitkClassMacro(UnstructuredGrid, BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /**
     * \brief Set the VTK unstructured grid for a given time step.
     *
     * The container is expanded if necessary.
     *
     * \param[in] grid The vtkUnstructuredGrid to store. Ownership is shared.
     * \param[in] t The time step index (default 0).
     */
    virtual void SetVtkUnstructuredGrid(vtkUnstructuredGrid *grid, unsigned int t = 0);

    /**
     * \brief Get the VTK unstructured grid for a given time step.
     *
     * If the data is not yet available and a source is connected, the
     * pipeline is updated.
     *
     * \param[in] t The time step index (default 0).
     * \return The vtkUnstructuredGrid, or nullptr if the time step is out of range.
     */
    virtual vtkUnstructuredGrid *GetVtkUnstructuredGrid(unsigned int t = 0);

    /** \brief Update bounding box and time geometry information. */
    void UpdateOutputInformation() override;

    /** \brief Set the requested region to the largest possible region. */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Check whether the requested region lies outside the buffered region.
     * \return true if any requested time step has no data.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Verify the requested region is within valid bounds.
     * \return true if the requested region is valid.
     */
    bool VerifyRequestedRegion() override;

    /**
     * \brief Set the requested region from another data object.
     * \param[in] data The data object whose region is copied.
     * \throw mitk::Exception if the data cannot be cast to UnstructuredGrid.
     */
    void SetRequestedRegion(const itk::DataObject *data) override;

    /**
     * \brief Set the requested region directly.
     * \param[in] region Pointer to the region to set.
     */
    virtual void SetRequestedRegion(UnstructuredGrid::RegionType *region);

    /**
     * \brief Graft the data and information from another UnstructuredGrid.
     * \param[in] data The source data object.
     * \throw mitk::Exception if data is not an UnstructuredGrid.
     */
    void Graft(const DataObject *data) override;

    /**
     * \brief Copy information (metadata) from another data object.
     * \param[in] data The source data object.
     */
    void CopyInformation(const itk::DataObject *data) override;

    /** \brief Trigger a pipeline update. */
    void Update() override;

    /**
     * \brief Expand the internal time series to hold at least the given number of time steps.
     * \param[in] timeSteps The minimum number of time steps.
     */
    void Expand(unsigned int timeSteps = 1) override;

    /**
     * \brief Get the largest possible region.
     * \return Const reference to the region spanning all time steps.
     */
    const RegionType &GetLargestPossibleRegion() const
    {
      m_LargestPossibleRegion.SetIndex(3, 0);
      m_LargestPossibleRegion.SetSize(3, GetTimeGeometry()->CountTimeSteps());
      return m_LargestPossibleRegion;
    }

    /**
     * \brief Get the currently requested region.
     * \return Const reference to the requested region.
     */
    virtual const RegionType &GetRequestedRegion() const { return m_RequestedRegion; }

    /**
     * \brief Recompute the bounding box from all time-step grids.
     */
    void CalculateBoundingBox();

  protected:
    mitkCloneMacro(Self);

    typedef std::vector<vtkSmartPointer<vtkUnstructuredGrid>> VTKUnstructuredGridSeries;

    UnstructuredGrid();

    UnstructuredGrid(const mitk::UnstructuredGrid &other);

    ~UnstructuredGrid() override;

    void ClearData() override;

    void InitializeEmpty() override;

    VTKUnstructuredGridSeries m_GridSeries;

    mutable RegionType m_LargestPossibleRegion;

    RegionType m_RequestedRegion;

    bool m_CalculateBoundingBox;
  };

} // namespace mitk

#endif
