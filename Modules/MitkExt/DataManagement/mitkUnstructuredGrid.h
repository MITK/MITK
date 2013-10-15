/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef _MITK_UNSTRUCTURED_GRID_H_
#define _MITK_UNSTRUCTURED_GRID_H_

#include "mitkBaseData.h"
#include "MitkExtExports.h"
#include "itkImageRegion.h"

class vtkUnstructuredGrid;

namespace mitk {

//##Documentation
//## @brief Class for storing unstructured grids (vtkUnstructuredGrid)
//## @ingroup Data
class MitkExt_EXPORT UnstructuredGrid : public BaseData
{

public:
  // not yet the best choice of a region-type for surfaces, but it works for the time being
  typedef itk::ImageRegion< 5 >  RegionType;

  mitkClassMacro(UnstructuredGrid, BaseData);

  itkNewMacro(Self);

  mitkCloneMacro(UnstructuredGrid);

  virtual void SetVtkUnstructuredGrid(vtkUnstructuredGrid* grid, unsigned int t = 0);

  virtual vtkUnstructuredGrid* GetVtkUnstructuredGrid(unsigned int t = 0);

  virtual void UpdateOutputInformation();

  virtual void SetRequestedRegionToLargestPossibleRegion();

  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

  virtual bool VerifyRequestedRegion();

  virtual void SetRequestedRegion( const itk::DataObject *data);

  virtual void SetRequestedRegion(UnstructuredGrid::RegionType *region);

  virtual void CopyInformation(const itk::DataObject *data);

  virtual void Update();

  // Initialize should not be called manually;
  // The polydata vector is initialized automatically when enlarged;
  virtual void Expand( unsigned int timeSteps = 1 );

  const RegionType& GetLargestPossibleRegion() const
  {
    m_LargestPossibleRegion.SetIndex(3, 0);
    m_LargestPossibleRegion.SetSize(3, GetTimeGeometry()->CountTimeSteps());
    return m_LargestPossibleRegion;
  }

  //##Documentation
  //## Get the region object that defines the size and starting index
  //## for the region of the image requested (i.e., the region of the
  //## image to be operated on by a filter).
  virtual const RegionType& GetRequestedRegion() const
  {
    return m_RequestedRegion;
  }

  void CalculateBoundingBox();

protected:

  typedef std::vector< vtkUnstructuredGrid* > VTKUnstructuredGridSeries;

  UnstructuredGrid();

  UnstructuredGrid(const mitk::UnstructuredGrid & other);

  virtual ~UnstructuredGrid();

  virtual void ClearData();

  virtual void InitializeEmpty();

  VTKUnstructuredGridSeries m_GridSeries;

  mutable RegionType m_LargestPossibleRegion;

  RegionType m_RequestedRegion;

  bool m_CalculateBoundingBox;
};

} // namespace mitk

#endif /* _MITK_UNSTRUCTURED_GRID_H_ */
