/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_UNSTRUCTURED_GRID_H_
#define _MITK_UNSTRUCTURED_GRID_H_

#include "MitkDataTypesExtExports.h"
#include "itkImageRegion.h"
#include "mitkBaseData.h"

class vtkUnstructuredGrid;

namespace mitk
{
  //##Documentation
  //## @brief Class for storing unstructured grids (vtkUnstructuredGrid)
  //## @ingroup Data
  class MITKDATATYPESEXT_EXPORT UnstructuredGrid : public BaseData
  {
  public:
    // not yet the best choice of a region-type for surfaces, but it works for the time being
    typedef itk::ImageRegion<5> RegionType;

    mitkClassMacro(UnstructuredGrid, BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      virtual void SetVtkUnstructuredGrid(vtkUnstructuredGrid *grid, unsigned int t = 0);

    virtual vtkUnstructuredGrid *GetVtkUnstructuredGrid(unsigned int t = 0);

    void UpdateOutputInformation() override;

    void SetRequestedRegionToLargestPossibleRegion() override;

    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    bool VerifyRequestedRegion() override;

    void SetRequestedRegion(const itk::DataObject *data) override;

    virtual void SetRequestedRegion(UnstructuredGrid::RegionType *region);

    void Graft(const DataObject *data) override;

    void CopyInformation(const itk::DataObject *data) override;

    void Update() override;

    // Initialize should not be called manually;
    // The polydata vector is initialized automatically when enlarged;
    void Expand(unsigned int timeSteps = 1) override;

    const RegionType &GetLargestPossibleRegion() const
    {
      m_LargestPossibleRegion.SetIndex(3, 0);
      m_LargestPossibleRegion.SetSize(3, GetTimeGeometry()->CountTimeSteps());
      return m_LargestPossibleRegion;
    }

    //##Documentation
    //## Get the region object that defines the size and starting index
    //## for the region of the image requested (i.e., the region of the
    //## image to be operated on by a filter).
    virtual const RegionType &GetRequestedRegion() const { return m_RequestedRegion; }
    void CalculateBoundingBox();

  protected:
    mitkCloneMacro(Self);

    typedef std::vector<vtkUnstructuredGrid *> VTKUnstructuredGridSeries;

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

#endif /* _MITK_UNSTRUCTURED_GRID_H_ */
