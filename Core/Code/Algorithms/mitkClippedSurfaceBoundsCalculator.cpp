#include "mitkClippedSurfaceBoundsCalculator.h"
#include "vtkPolyData.h"
#include "vtkPlaneSource.h"
#include "vtkPPolyDataNormals.h"
#include "mitkGeometry2D.h"
#include "mitkSliceNavigationController.h"
#include "mitkLine.h"
#include "vtkSmartPointer.h"

#define ROUND_P(x) ((int)((x)+0.5))

mitk::ClippedSurfaceBoundsCalculator::ClippedSurfaceBoundsCalculator(const mitk::PlaneGeometry* geometry, mitk::Image::Pointer image)
{
  this->SetInput(geometry, image);
}

mitk::ClippedSurfaceBoundsCalculator::~ClippedSurfaceBoundsCalculator()
{
}

void mitk::ClippedSurfaceBoundsCalculator::SetInput(const mitk::PlaneGeometry* geometry, mitk::Image::Pointer image)
{
  if(geometry != NULL && image.IsNotNull())
  {
    this->m_Geometry2D = geometry;
    this->m_Image = image;
    this->m_MinMaxOutput.clear();
    for(int i = 0; i < 3; i++)
    {
      this->m_MinMaxOutput.push_back(outputType( std::numeric_limits<int>::max() , std::numeric_limits<int>::min() ));
    }
  }
}

void mitk::ClippedSurfaceBoundsCalculator::CalculateSurfaceBounds()
{

  //Get the corner points of the geometry3D
  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(false, false, false)
                         , m_Image->GetGeometry()->GetCornerPoint(true, false, false));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(false, false, false)
                         , m_Image->GetGeometry()->GetCornerPoint(false, true, false));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(false, false, false)
                         , m_Image->GetGeometry()->GetCornerPoint(false, false, true));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(false, true, false)
                         , m_Image->GetGeometry()->GetCornerPoint(true, true, false));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(false, true, false)
                         , m_Image->GetGeometry()->GetCornerPoint(false, true, true));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(false, false, true)
                         , m_Image->GetGeometry()->GetCornerPoint(true, false, true));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(false, false, true)
                         , m_Image->GetGeometry()->GetCornerPoint(false, true, true));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(false, true, true)
                         , m_Image->GetGeometry()->GetCornerPoint(true, true, true));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(true, false, true)
                         , m_Image->GetGeometry()->GetCornerPoint(true, true, true));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(true, false, false)
                         , m_Image->GetGeometry()->GetCornerPoint(true, false, true));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(true, false, false)
                         , m_Image->GetGeometry()->GetCornerPoint(true, true, false));

  CalculateSurfaceBounds(  m_Image->GetGeometry()->GetCornerPoint(true, true, false)
                         , m_Image->GetGeometry()->GetCornerPoint(true, true, true));
}

void mitk::ClippedSurfaceBoundsCalculator::CalculateSurfaceBounds(Point3D startPoint, Point3D endPoint)
{
  if(this->GetIntersectionPoint(startPoint, endPoint) != NULL)
  {
    mitk::Point3D* point3D = this->GetIntersectionPoint(startPoint, endPoint);
    mitk::Point3D indexPoint3D;
    m_Image->GetGeometry()->WorldToIndex(*point3D, indexPoint3D);

    for(int dim = 0; dim < 3; dim++)
    {
      // minimum
      if(this->m_MinMaxOutput[dim].first > ROUND_P(indexPoint3D[dim]))
      {
        this->m_MinMaxOutput[dim].first = ROUND_P(indexPoint3D[dim]);
      }

      // maximum
      if(this->m_MinMaxOutput[dim].second < ROUND_P(indexPoint3D[dim]))
      {
        this->m_MinMaxOutput[dim].second = ROUND_P(indexPoint3D[dim]);
      }
    }
    delete point3D;
  }

//  }
}

mitk::Point3D* mitk::ClippedSurfaceBoundsCalculator::GetIntersectionPoint(Point3D startPoint, Point3D endPoint)
{
  Point3D* intersectionPoint = new Point3D();
  vtkSmartPointer<vtkPoints> points = vtkPoints::New();
  points->SetNumberOfPoints(1);

  endPoint[0] =- startPoint[0];
  endPoint[1] =- startPoint[1];
  endPoint[2] =- startPoint[2];

  Vector3D bottomVec;

  FillVector3D(bottomVec, endPoint[0], endPoint[1], endPoint[2]);

  mitk::Line3D line(startPoint, bottomVec);

  m_Geometry2D->IntersectionPoint(line, *intersectionPoint);

  if(m_Image->GetGeometry()->IsInside(*intersectionPoint))
  {
    return intersectionPoint;
  }
  else
  {
    return NULL;
  }
}

mitk::ClippedSurfaceBoundsCalculator::outputType mitk::ClippedSurfaceBoundsCalculator::GetMinMaxSpatialDirectionX()
{
  return this->m_MinMaxOutput[0];
}

mitk::ClippedSurfaceBoundsCalculator::outputType mitk::ClippedSurfaceBoundsCalculator::GetMinMaxSpatialDirectionY()
{
  return this->m_MinMaxOutput[1];
}

mitk::ClippedSurfaceBoundsCalculator::outputType mitk::ClippedSurfaceBoundsCalculator::GetMinMaxSpatialDirectionZ()
{
  return this->m_MinMaxOutput[2];
}

void mitk::ClippedSurfaceBoundsCalculator::Update()
{
  this->CalculateSurfaceBounds();
}
