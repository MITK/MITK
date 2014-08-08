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
#include "mitkCoreServices.h"
#include "mitkISurfaceCutterFactory.h"

#include <algorithm>
#include <vtkPolyData.h>
#include <vtkIdList.h>
#include <vtkTriangleFilter.h>
#include <vtkSmartPointer.h>
#include <vtkCell.h>

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

static void Update(vtkPolyData* /*polyData*/)
{
//  if (polyData != NULL)
//    polyData->Update(); //VTK6_TODO vtk pipeline
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
  m_LargestPossibleRegion.SetSize(3, GetTimeGeometry()->CountTimeSteps());

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

  Superclass::InitializeTimeGeometry();

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
    m_Cutters[t].reset();
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

vtkSmartPointer<vtkPolyData> mitk::Surface::CutWithPlane(mitk::Point3D planePoints[4], unsigned int t /* = 0 */)
{
    if (!GetVtkPolyData(t)) {
        return nullptr;
    }

    if (m_Cutters.size() <= t) {
        m_Cutters.resize(t + 1);
    }

    if (!m_Cutters[t]) {
        ISurfaceCutterFactory* cutterFactory = CoreServices::GetSurfaceCutterFactory();
        if (cutterFactory) {
            m_Cutters[t].reset(cutterFactory->createSurfaceCutter());
            if (m_Cutters[t]) {
                m_Cutters[t]->setPolyData(m_PolyDatas[t]);
            }
        }
    }
    return m_Cutters[t] ? m_Cutters[t]->cutWithPlane(planePoints) : vtkSmartPointer<vtkPolyData>::New();
}

void mitk::Surface::UpdateOutputInformation()
{
  if (this->GetSource().IsNotNull())
    this->GetSource()->UpdateOutputInformation();

  if (m_CalculateBoundingBox == true && !m_PolyDatas.empty())
    this->CalculateBoundingBox();
  else
    this->GetTimeGeometry()->Update();
}

void mitk::Surface::CalculateBoundingBox()
{
  TimeGeometry* timeGeometry = this->GetTimeGeometry();

  if (timeGeometry->CountTimeSteps() != m_PolyDatas.size())
    mitkThrow() << "Number of geometry time steps is inconsistent with number of poly data pointers.";

  for (unsigned int i = 0; i < m_PolyDatas.size(); ++i)
  {
    vtkPolyData* polyData = m_PolyDatas[i];
    double bounds[6] = {0};

    if (polyData != NULL && polyData->GetNumberOfPoints() > 0)
    {
//      polyData->Update(); //VTK6_TODO vtk pipeline
      polyData->ComputeBounds();
      polyData->GetBounds(bounds);
    }

    Geometry3D::Pointer geometry = timeGeometry->GetGeometryForTimeStep(i);

    if (geometry.IsNull())
      mitkThrow() << "Time-sliced geometry is invalid (equals NULL).";

    geometry->SetFloatBounds(bounds);
  }

  timeGeometry->Update();
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

void mitk::Surface::SetRequestedRegion(const itk::DataObject* data )
{
  const mitk::Surface *surface = dynamic_cast<const mitk::Surface*>(data);

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
    m_Cutters.resize(timeSteps);
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

bool mitk::Equal( vtkPolyData* leftHandSide, vtkPolyData* rightHandSide, mitk::ScalarType eps, bool verbose )
{
  if(( leftHandSide == NULL ) || ( rightHandSide == NULL ))
  {
    MITK_ERROR << "mitk::Equal( vtkPolyData* leftHandSide, vtkPolyData* rightHandSide, mitk::ScalarType eps, bool verbose ) does not work for NULL pointer input.";
    return false;
  }
  return Equal( *leftHandSide, *rightHandSide, eps, verbose);
}


bool mitk::Equal( vtkPolyData& leftHandSide, vtkPolyData& rightHandSide, mitk::ScalarType eps, bool verbose )
{
  bool noDifferenceFound = true;

  if( ! mitk::Equal( leftHandSide.GetNumberOfCells(), rightHandSide.GetNumberOfCells(), eps, verbose ) )
  {
    if(verbose)
      MITK_INFO << "[Equal( vtkPolyData*, vtkPolyData* )] Number of cells not equal";
    noDifferenceFound = false;
  }

  if( ! mitk::Equal( leftHandSide.GetNumberOfVerts(), rightHandSide.GetNumberOfVerts(), eps, verbose ))
  {
    if(verbose)
      MITK_INFO << "[Equal( vtkPolyData*, vtkPolyData* )] Number of vertices not equal";
    noDifferenceFound = false;
  }

  if( ! mitk::Equal( leftHandSide.GetNumberOfLines(), rightHandSide.GetNumberOfLines(), eps, verbose ) )
  {
    if(verbose)
      MITK_INFO << "[Equal( vtkPolyData*, vtkPolyData* )] Number of lines not equal";
    noDifferenceFound = false;
  }

  if( ! mitk::Equal( leftHandSide.GetNumberOfPolys(), rightHandSide.GetNumberOfPolys(), eps, verbose ) )
  {
    if(verbose)
      MITK_INFO << "[Equal( vtkPolyData*, vtkPolyData* )] Number of polys not equal";
    noDifferenceFound = false;
  }

  if( ! mitk::Equal( leftHandSide.GetNumberOfStrips(), rightHandSide.GetNumberOfStrips(), eps, verbose ) )
  {
    if(verbose)
      MITK_INFO << "[Equal( vtkPolyData*, vtkPolyData* )] Number of strips not equal";
    noDifferenceFound = false;
  }

  {
    unsigned int numberOfPointsRight = rightHandSide.GetPoints()->GetNumberOfPoints();
    unsigned int numberOfPointsLeft = leftHandSide.GetPoints()->GetNumberOfPoints();
    if(! mitk::Equal( numberOfPointsLeft, numberOfPointsRight, eps, verbose ))
    {
      if(verbose)
        MITK_INFO << "[Equal( vtkPolyData*, vtkPolyData* )] Number of points not equal";
      noDifferenceFound = false;
    }
    else
    {
      for( unsigned int i( 0 ); i < numberOfPointsRight; i++ )
      {
        bool pointFound = false;
        double pointOne[3];
        rightHandSide.GetPoints()->GetPoint(i, pointOne);

        for( unsigned int j( 0 ); j < numberOfPointsLeft; j++ )
        {
          double pointTwo[3];
          leftHandSide.GetPoints()->GetPoint(j, pointTwo);

          double x = pointOne[0] - pointTwo[0];
          double y = pointOne[1] - pointTwo[1];
          double z = pointOne[2] - pointTwo[2];
          double distance = x*x + y*y + z*z;

          if( distance < eps )
          {
            pointFound = true;
            break;
          }
        }
        if( !pointFound )
        {
          if(verbose)
          {
            MITK_INFO << "[Equal( vtkPolyData*, vtkPolyData* )] Right hand side point with id " << i << " and coordinates ( "
                    << std::setprecision(12)
                    << pointOne[0] << " ; " << pointOne[1] << " ; " << pointOne[2]
                    << " ) could not be found in left hand side with epsilon " << eps << ".";
          }
          noDifferenceFound = false;
          break;
        }
      }
    }
  }
  return noDifferenceFound;
}

bool mitk::Equal( mitk::Surface* leftHandSide, mitk::Surface* rightHandSide, mitk::ScalarType eps, bool verbose )
{
  if(( leftHandSide == NULL ) || ( rightHandSide == NULL ))
  {
    MITK_ERROR << "mitk::Equal( mitk::Surface* leftHandSide, mitk::Surface* rightHandSide, mitk::ScalarType eps, bool verbose ) does not work with NULL pointer input.";
    return false;
  }
  return Equal( *leftHandSide, *rightHandSide, eps, verbose);
}


bool mitk::Equal( mitk::Surface& leftHandSide, mitk::Surface& rightHandSide, mitk::ScalarType eps, bool verbose )
{
  bool noDifferenceFound = true;

  if( ! mitk::Equal( leftHandSide.GetSizeOfPolyDataSeries(), rightHandSide.GetSizeOfPolyDataSeries(), eps, verbose ) )
  {
    if(verbose)
      MITK_INFO << "[Equal( mitk::surface&, mitk::surface& )] Size of PolyData series not equal.";
    return false;
  }

  // No mitk::Equal for TimeGeometry implemented.
  //if( ! mitk::Equal( leftHandSide->GetTimeGeometry(), rightHandSide->GetTimeGeometry(), eps, verbose ) )
  //{
  //  if(verbose)
  //    MITK_INFO << "[Equal( mitk::surface&, mitk::surface& )] Time sliced geometries not equal";
  //  noDifferenceFound = false;
  //}

  for( unsigned int i( 0 ); i < rightHandSide.GetSizeOfPolyDataSeries(); i++ )
  {
    if( ! mitk::Equal( *leftHandSide.GetVtkPolyData( i ), *rightHandSide.GetVtkPolyData( i ), eps, verbose ) )
    {
      if(verbose)
        MITK_INFO << "[Equal( mitk::surface&, mitk::surface& )] Poly datas not equal.";
      noDifferenceFound = false;
    }
  }

  return noDifferenceFound;
}
