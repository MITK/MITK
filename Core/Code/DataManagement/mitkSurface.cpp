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

#include "mitkSurface.h"
#include "mitkInteractionConst.h"
#include "mitkSurfaceOperation.h"

#include <algorithm>
#include <vtkPolyData.h>

static vtkPolyData* DeepCopy(vtkPolyData* other)
{
  if (other == NULL)
    return NULL;

  vtkPolyData* copy = vtkPolyData::New();
  copy->DeepCopy(other);

  return copy;
}

static void Delete(vtkPolyData* polyData)
{
  if (polyData != NULL)
    polyData->Delete();
}

static void Update(vtkPolyData* polyData)
{
  if (polyData != NULL)
    polyData->Update();
}

mitk::Surface::Surface()
  : m_CalculateBoundingBox(false)
{
  this->InitializeEmpty();
}

mitk::Surface::Surface(const mitk::Surface& other)
  : BaseData(other),
    m_LargestPossibleRegion(other.m_LargestPossibleRegion),
    m_RequestedRegion(other.m_RequestedRegion),
    m_CalculateBoundingBox(other.m_CalculateBoundingBox)
{
  if(!other.m_PolyDatas.empty())
  {
    m_PolyDatas.resize(other.m_PolyDatas.size());
    std::transform(other.m_PolyDatas.begin(), other.m_PolyDatas.end(), m_PolyDatas.begin(), DeepCopy);
  }
  else
  {
    this->InitializeEmpty();
  }
}

void mitk::Surface::Swap(mitk::Surface& other)
{
  std::swap(m_PolyDatas, other.m_PolyDatas);
  std::swap(m_LargestPossibleRegion, other.m_LargestPossibleRegion);
  std::swap(m_RequestedRegion, other.m_RequestedRegion);
  std::swap(m_CalculateBoundingBox, other.m_CalculateBoundingBox);
}

mitk::Surface& mitk::Surface::operator=(Surface other)
{
  this->Swap(other);
  return *this;
}

mitk::Surface::~Surface()
{
  this->ClearData();
}

void mitk::Surface::ClearData()
{
  using ::Delete;

  std::for_each(m_PolyDatas.begin(), m_PolyDatas.end(), Delete);
  m_PolyDatas.clear();

  Superclass::ClearData();
}

const mitk::Surface::RegionType& mitk::Surface::GetLargestPossibleRegion() const
{
  m_LargestPossibleRegion.SetIndex(3, 0);
  m_LargestPossibleRegion.SetSize(3, GetTimeSlicedGeometry()->GetTimeSteps());

  return m_LargestPossibleRegion;
}

const mitk::Surface::RegionType& mitk::Surface::GetRequestedRegion() const
{
  return m_RequestedRegion;
}

void mitk::Surface::InitializeEmpty()
{
  if (!m_PolyDatas.empty())
    this->ClearData();

  Superclass::InitializeTimeSlicedGeometry();

  m_PolyDatas.push_back(NULL);
  m_Initialized = true;
}

void mitk::Surface::SetVtkPolyData(vtkPolyData* polyData, unsigned int t)
{
  this->Expand(t + 1);

  if (m_PolyDatas[t] != NULL)
  {
    if (m_PolyDatas[t] == polyData)
      return;

    m_PolyDatas[t]->Delete();
  }

  m_PolyDatas[t] = polyData;

  if(polyData != NULL)
    polyData->Register(NULL);

  m_CalculateBoundingBox = true;

  this->Modified();
  this->UpdateOutputInformation();
}

bool mitk::Surface::IsEmptyTimeStep(unsigned int t) const
{
  if(!IsInitialized())
    return false;

  vtkPolyData* polyData = const_cast<Surface*>(this)->GetVtkPolyData(t);

  return polyData == NULL || (
    polyData->GetNumberOfLines() == 0 &&
    polyData->GetNumberOfPolys() == 0 &&
    polyData->GetNumberOfStrips() == 0 &&
    polyData->GetNumberOfVerts() == 0
  );
}

vtkPolyData* mitk::Surface::GetVtkPolyData(unsigned int t)
{
  if (t < m_PolyDatas.size())
  {
    if(m_PolyDatas[t] == NULL && this->GetSource().IsNotNull())
    {
      RegionType requestedRegion;
      requestedRegion.SetIndex(3, t);
      requestedRegion.SetSize(3, 1);

      this->SetRequestedRegion(&requestedRegion);
      this->GetSource()->Update();
    }

    return m_PolyDatas[t];
  }

  return NULL;
}

void mitk::Surface::UpdateOutputInformation()
{
  if (this->GetSource().IsNotNull())
    this->GetSource()->UpdateOutputInformation();

  if (m_CalculateBoundingBox == true && !m_PolyDatas.empty())
    this->CalculateBoundingBox();
  else
    this->GetTimeSlicedGeometry()->UpdateInformation();
}

void mitk::Surface::CalculateBoundingBox()
{
  mitk::TimeSlicedGeometry* timeSlicedGeometry = this->GetTimeSlicedGeometry();

  if (timeSlicedGeometry->GetTimeSteps() != m_PolyDatas.size())
    mitkThrow() << "Number of geometry time steps is inconsistent with number of poly data pointers.";

  for (unsigned int i = 0; i < m_PolyDatas.size(); ++i)
  {
    vtkPolyData* polyData = m_PolyDatas[i];
    vtkFloatingPointType bounds[6] = {0};

    if (polyData != NULL && polyData->GetNumberOfPoints() > 0)
    {
      polyData->Update();
      polyData->ComputeBounds();
      polyData->GetBounds(bounds);
    }

    mitk::Geometry3D::Pointer geometry = timeSlicedGeometry->GetGeometry3D(i);

    if (geometry.IsNull())
      mitkThrow() << "Time-sliced geometry is invalid (equals NULL).";

    geometry->SetFloatBounds(bounds);
  }

  timeSlicedGeometry->UpdateInformation();
  m_CalculateBoundingBox = false;
}

void mitk::Surface::SetRequestedRegionToLargestPossibleRegion()
{
  m_RequestedRegion = GetLargestPossibleRegion();
}

bool mitk::Surface::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  RegionType::IndexValueType end = m_RequestedRegion.GetIndex(3) + m_RequestedRegion.GetSize(3);

  if(static_cast<RegionType::IndexValueType>(m_PolyDatas.size()) < end)
    return true;

  for(RegionType::IndexValueType t = m_RequestedRegion.GetIndex(3); t < end; ++t)
  {
    if(m_PolyDatas[t] == NULL)
      return true;
  }

  return false;
}

bool mitk::Surface::VerifyRequestedRegion()
{
  if(m_RequestedRegion.GetIndex(3) >= 0 && m_RequestedRegion.GetIndex(3) + m_RequestedRegion.GetSize(3) <= m_PolyDatas.size())
    return true;

  return false;
}

void mitk::Surface::SetRequestedRegion(itk::DataObject* data)
{
  mitk::Surface* surface = dynamic_cast<mitk::Surface*>(data);

  if (surface != NULL)
    m_RequestedRegion = surface->GetRequestedRegion();
  else
    mitkThrow() << "Data object used to get requested region is not a mitk::Surface.";
}

void mitk::Surface::SetRequestedRegion(Surface::RegionType* region)
{
  if (region == NULL)
    mitkThrow() << "Requested region is invalid (equals NULL)";

  m_RequestedRegion = *region;
}

void mitk::Surface::CopyInformation(const itk::DataObject* data)
{
  Superclass::CopyInformation(data);

  const mitk::Surface* surface = dynamic_cast<const mitk::Surface*>(data);

  if (surface == NULL)
    mitkThrow() << "Data object used to get largest possible region is not a mitk::Surface.";

  m_LargestPossibleRegion = surface->GetLargestPossibleRegion();
}

void mitk::Surface::Update()
{
  using ::Update;

  if (this->GetSource().IsNull())
    std::for_each(m_PolyDatas.begin(), m_PolyDatas.end(), Update);

  Superclass::Update();
}

void mitk::Surface::Expand(unsigned int timeSteps)
{
  if (timeSteps > m_PolyDatas.size())
  {
    Superclass::Expand(timeSteps);

    m_PolyDatas.resize(timeSteps);
    m_CalculateBoundingBox = true;
  }
}

void mitk::Surface::ExecuteOperation(Operation* operation)
{
  switch (operation->GetOperationType())
  {
    case OpSURFACECHANGED:
    {
      mitk::SurfaceOperation* surfaceOperation = dynamic_cast<mitk::SurfaceOperation*>(operation);

      if(surfaceOperation == NULL)
        break;

      unsigned int timeStep = surfaceOperation->GetTimeStep();

      if(m_PolyDatas[timeStep] != NULL)
      {
        vtkPolyData* updatedPolyData = surfaceOperation->GetVtkPolyData();

        if(updatedPolyData != NULL)
        {
          this->SetVtkPolyData(updatedPolyData, timeStep);
          this->CalculateBoundingBox();
          this->Modified();
        }
      }

      break;
    }

    default:
      return;
  }
}

unsigned int mitk::Surface::GetSizeOfPolyDataSeries() const
{
  return m_PolyDatas.size();
}

void mitk::Surface::Graft(const DataObject* data)
{
  const Surface* surface = dynamic_cast<const Surface*>(data);

  if(surface == NULL)
    mitkThrow() << "Data object used to graft surface is not a mitk::Surface.";

  this->CopyInformation(data);
  m_PolyDatas.clear();

  for (unsigned int i = 0; i < surface->GetSizeOfPolyDataSeries(); ++i)
  {
    m_PolyDatas.push_back(vtkPolyData::New());
    m_PolyDatas.back()->DeepCopy(const_cast<mitk::Surface*>(surface)->GetVtkPolyData(i));
  }
}

void mitk::Surface::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "\nNumber PolyDatas: " << m_PolyDatas.size() << "\n";

  unsigned int count = 0;

  for (std::vector<vtkPolyData*>::const_iterator it = m_PolyDatas.begin(); it != m_PolyDatas.end(); ++it)
  {
    os << "\n";

    if(*it != NULL)
    {
      os << indent << "PolyData at time step " << count << ":\n";
      os << indent << "Number of cells: " << (*it)->GetNumberOfCells() << "\n";
      os << indent << "Number of points: " << (*it)->GetNumberOfPoints() << "\n\n";
      os << indent << "VTKPolyData:\n";

      (*it)->Print(os);
    }
    else
    {
      os << indent << "Empty PolyData at time step " << count << "\n";
    }

    ++count;
  }
}
