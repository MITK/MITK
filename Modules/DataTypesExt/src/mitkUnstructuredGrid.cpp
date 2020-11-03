/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUnstructuredGrid.h"

#include <vtkUnstructuredGrid.h>

void mitk::UnstructuredGrid::SetVtkUnstructuredGrid(vtkUnstructuredGrid *grid, unsigned int t)
{
  this->Expand(t + 1);

  if (m_GridSeries[t] != nullptr)
  {
    m_GridSeries[t]->Delete();
  }

  m_GridSeries[t] = grid;

  // call m_VtkPolyData->Register(nullptr) to tell the reference counting that we
  // want to keep a reference on the object
  if (m_GridSeries[t] != nullptr)
    m_GridSeries[t]->Register(grid);

  this->Modified();
  m_CalculateBoundingBox = true;
}

void mitk::UnstructuredGrid::Expand(unsigned int timeSteps)
{
  // check if the vector is long enough to contain the new element
  // at the given position. If not, expand it with sufficient zero-filled elements.
  if (timeSteps > m_GridSeries.size())
  {
    Superclass::Expand(timeSteps);
    vtkUnstructuredGrid *pdnull = nullptr;
    m_GridSeries.resize(timeSteps, pdnull);
    m_CalculateBoundingBox = true;
  }
}

void mitk::UnstructuredGrid::ClearData()
{
  for (auto it = m_GridSeries.begin(); it != m_GridSeries.end(); ++it)
  {
    if ((*it) != nullptr)
      (*it)->Delete();
  }
  m_GridSeries.clear();

  Superclass::ClearData();
}

void mitk::UnstructuredGrid::InitializeEmpty()
{
  vtkUnstructuredGrid *pdnull = nullptr;
  m_GridSeries.resize(1, pdnull);
  Superclass::InitializeTimeGeometry(1);

  m_Initialized = true;
}

vtkUnstructuredGrid *mitk::UnstructuredGrid::GetVtkUnstructuredGrid(unsigned int t)
{
  if (t < m_GridSeries.size())
  {
    vtkUnstructuredGrid *grid = m_GridSeries[t];
    if ((grid == nullptr) && (GetSource().GetPointer() != nullptr))
    {
      RegionType requestedregion;
      requestedregion.SetIndex(3, t);
      requestedregion.SetSize(3, 1);
      SetRequestedRegion(&requestedregion);
      GetSource()->Update();
    }
    grid = m_GridSeries[t];
    return grid;
  }
  else
    return nullptr;
}

void mitk::UnstructuredGrid::Graft(const DataObject *data)
{
  const auto *grid = dynamic_cast<const UnstructuredGrid *>(data);

  if (grid == nullptr)
    mitkThrow() << "Data object used to graft surface is not a mitk::Surface.";

  this->CopyInformation(data);
  m_GridSeries.clear();

  for (unsigned int i = 0; i < grid->m_GridSeries.size(); ++i)
  {
    m_GridSeries.push_back(vtkUnstructuredGrid::New());
    m_GridSeries.back()->DeepCopy(const_cast<mitk::UnstructuredGrid *>(grid)->GetVtkUnstructuredGrid(i));
  }
}

mitk::UnstructuredGrid::UnstructuredGrid() : m_CalculateBoundingBox(false)
{
  this->InitializeEmpty();
}

mitk::UnstructuredGrid::UnstructuredGrid(const mitk::UnstructuredGrid &other)
  : BaseData(other),
    m_LargestPossibleRegion(other.m_LargestPossibleRegion),
    m_CalculateBoundingBox(other.m_CalculateBoundingBox)
{
  if (!other.m_Initialized)
  {
    this->InitializeEmpty();
  }
  else
  {
    m_GridSeries = other.m_GridSeries;
    m_Initialized = other.m_Initialized;
  }
  this->SetRequestedRegion(&other);
}

mitk::UnstructuredGrid::~UnstructuredGrid()
{
  this->ClearData();
}

void mitk::UnstructuredGrid::UpdateOutputInformation()
{
  if (this->GetSource())
  {
    this->GetSource()->UpdateOutputInformation();
  }
  if ((m_CalculateBoundingBox) && (m_GridSeries.size() > 0))
    CalculateBoundingBox();
  else
    GetTimeGeometry()->Update();
}

void mitk::UnstructuredGrid::CalculateBoundingBox()
{
  //
  // first make sure, that the associated time sliced geometry has
  // the same number of geometry 3d's as vtkUnstructuredGrids are present
  //
  TimeGeometry *timeGeometry = GetTimeGeometry();
  if (timeGeometry->CountTimeSteps() != m_GridSeries.size())
  {
    itkExceptionMacro(<< "timeGeometry->CountTimeSteps() != m_GridSeries.size() -- use Initialize(timeSteps) with "
                         "correct number of timeSteps!");
  }

  //
  // Iterate over the vtkUnstructuredGrids and update the Geometry
  // information of each of the items.
  //
  for (unsigned int i = 0; i < m_GridSeries.size(); ++i)
  {
    vtkUnstructuredGrid *grid = m_GridSeries[i];
    double bounds[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    if ((grid != nullptr) && (grid->GetNumberOfCells() > 0))
    {
      //      grid->Update(); //VTK6_TODO
      grid->ComputeBounds();
      grid->GetBounds(bounds);
    }
    mitk::BaseGeometry::Pointer g3d = timeGeometry->GetGeometryForTimeStep(i);
    assert(g3d.IsNotNull());
    g3d->SetFloatBounds(bounds);
  }
  timeGeometry->Update();

  mitk::BoundingBox::Pointer bb = timeGeometry->GetBoundingBoxInWorld();
  itkDebugMacro(<< "boundingbox min: " << bb->GetMinimum());
  itkDebugMacro(<< "boundingbox max: " << bb->GetMaximum());
  m_CalculateBoundingBox = false;
}

void mitk::UnstructuredGrid::SetRequestedRegionToLargestPossibleRegion()
{
  m_RequestedRegion = GetLargestPossibleRegion();
}

bool mitk::UnstructuredGrid::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  RegionType::IndexValueType end = m_RequestedRegion.GetIndex(3) + m_RequestedRegion.GetSize(3);

  if (((RegionType::IndexValueType)m_GridSeries.size()) < end)
    return true;

  for (RegionType::IndexValueType t = m_RequestedRegion.GetIndex(3); t < end; ++t)
    if (m_GridSeries[t] == nullptr)
      return true;

  return false;
}

bool mitk::UnstructuredGrid::VerifyRequestedRegion()
{
  if ((m_RequestedRegion.GetIndex(3) >= 0) &&
      (m_RequestedRegion.GetIndex(3) + m_RequestedRegion.GetSize(3) <= m_GridSeries.size()))
    return true;

  return false;
}

void mitk::UnstructuredGrid::SetRequestedRegion(const itk::DataObject *data)
{
  const mitk::UnstructuredGrid *gridData;

  gridData = dynamic_cast<const mitk::UnstructuredGrid *>(data);

  if (gridData)
  {
    m_RequestedRegion = gridData->GetRequestedRegion();
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro(<< "mitk::UnstructuredGrid::SetRequestedRegion(DataObject*) cannot cast " << typeid(data).name()
                      << " to "
                      << typeid(UnstructuredGrid *).name());
  }
}

void mitk::UnstructuredGrid::SetRequestedRegion(UnstructuredGrid::RegionType *region) // by arin
{
  if (region != nullptr)
  {
    m_RequestedRegion = *region;
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro(<< "mitk::UnstructuredGrid::SetRequestedRegion(UnstructuredGrid::RegionType*) cannot cast "
                      << typeid(region).name()
                      << " to "
                      << typeid(UnstructuredGrid *).name());
  }
}

void mitk::UnstructuredGrid::CopyInformation(const itk::DataObject *data)
{
  Superclass::CopyInformation(data);
}

void mitk::UnstructuredGrid::Update()
{
  if (GetSource().IsNull())
  {
    for (auto it = m_GridSeries.begin(); it != m_GridSeries.end(); ++it)
    {
      //      if ( ( *it ) != 0 )
      //        ( *it )->Update(); //VTK6_TODO
    }
  }
  Superclass::Update();
}
