#include "mitkContour.h"

#include <mitkPointOperation.h>
#include <mitkOperationActor.h>

//#include <qapplication.h>//@todo because of quickimplementation UpdateAllWidgets
//#include <qwidgetlist.h>//@todo because of quickimplementation UpdateAllWidgets

#include <mitkInteractionConst.h>

mitk::Contour::Contour() :
m_ContourPath (PathType::New()),
m_CurrentWindow ( NULL ),
m_BoundingBox (BoundingBoxType::New()),
m_Vertices ( BoundingBoxType::PointsContainer::New() )
{
  m_Geometry3D->Initialize();
}

mitk::Contour::~Contour()
{}

void mitk::Contour::AddVertex(mitk::ITKPoint3D newPoint)
{
  BoundingBoxType::PointType p;
  p.CastFrom(newPoint);
  m_Vertices->InsertElement(m_Vertices->Size(), p);

  ContinuousIndexType idx;
  idx.CastFrom(newPoint);
  m_ContourPath->AddVertex(idx);
}

void mitk::Contour::ExecuteOperation(Operation* op)
{
  mitk::PointOperation * pointOp = dynamic_cast<mitk::PointOperation*>( op );
  if ( pointOp != NULL )
  {
    switch (op->GetOperationType())
    {
    case mitk::OpADD:
      {
      m_ContourPath = PathType::New();
      m_ContourPath->Initialize();    
      }

    case mitk::OpMOVE:
      {
        //std::cout << "add vertex to contour" << std::endl;
        AddVertex(pointOp->GetPoint());
        UpdateOutputInformation();
        if (m_CurrentWindow != NULL) {
          m_CurrentWindow->Update();
        }
      }
    }
  }
}

void mitk::Contour::UpdateOutputInformation()
{
  float mitkBounds[6];
  if (m_Vertices->Size() == 0)  {
    mitkBounds[0] = 0.0;
    mitkBounds[1] = 0.0;
    mitkBounds[2] = 0.0;
    mitkBounds[3] = 0.0;
    mitkBounds[4] = 0.0;
    mitkBounds[5] = 0.0;
  }
  else
  {
    m_BoundingBox->ComputeBoundingBox();
    BoundingBoxType::BoundsArrayType tmp = m_BoundingBox->GetBounds();
    mitkBounds[0] = tmp[0];
    mitkBounds[1] = tmp[1];
    mitkBounds[2] = tmp[2];
    mitkBounds[3] = tmp[3];
    mitkBounds[4] = tmp[4];
    mitkBounds[5] = tmp[5];
  }
  m_Geometry3D->SetBoundingBox(mitkBounds);
}

void mitk::Contour::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::Contour::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}

bool mitk::Contour::VerifyRequestedRegion()
{
    return false;
}

void mitk::Contour::SetRequestedRegion(itk::DataObject*)
{
}

mitk::Contour::PathType::Pointer 
mitk::Contour::GetContourPath()
{
  return m_ContourPath;
}

void 
mitk::Contour::SetCurrentWindow(mitk::RenderWindow* rw)
{
  m_CurrentWindow = rw;
}

mitk::RenderWindow*
mitk::Contour::GetCurrentWindow()
  {
  return m_CurrentWindow;
  }

void
mitk::Contour::Initialize()
  {
  m_ContourPath = PathType::New();
  m_ContourPath->Initialize();
  m_BoundingBox = BoundingBoxType::New();
  m_Vertices = BoundingBoxType::PointsContainer::New();
  m_Geometry3D->Initialize();
  }

void
mitk::Contour::Continue(mitk::ITKPoint3D newPoint)
  {
  //std::cout << "add vertex to contour" << std::endl;
  AddVertex(newPoint);
  UpdateOutputInformation();
  if (m_CurrentWindow != NULL) {
    m_CurrentWindow->Update();
    }
  }

unsigned int 
mitk::Contour::GetNumberOfPoints()
  {
  return m_Vertices->Size();
  }

mitk::Contour::PointsContainerPointer
mitk::Contour::GetPoints()
  {
  return m_Vertices;
  }

void
mitk::Contour::SetPoints(mitk::Contour::PointsContainerPointer points)
  {
  m_Vertices = points;
  }
