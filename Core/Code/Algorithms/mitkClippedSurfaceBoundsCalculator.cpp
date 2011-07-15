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
    this->m_PlaneGeometry = geometry;
    this->m_Image = image;
  }
}

mitk::ClippedSurfaceBoundsCalculator::OutputType mitk::ClippedSurfaceBoundsCalculator::GetMinMaxSpatialDirectionX()
{
  return this->m_MinMaxOutput[0];
}

mitk::ClippedSurfaceBoundsCalculator::OutputType mitk::ClippedSurfaceBoundsCalculator::GetMinMaxSpatialDirectionY()
{
  return this->m_MinMaxOutput[1];
}

mitk::ClippedSurfaceBoundsCalculator::OutputType mitk::ClippedSurfaceBoundsCalculator::GetMinMaxSpatialDirectionZ()
{
  return this->m_MinMaxOutput[2];
}

void mitk::ClippedSurfaceBoundsCalculator::Update()
{
  typedef std::vector< std::pair<mitk::Point3D, mitk::Point3D> > EdgesVector;

  this->m_MinMaxOutput.clear();
  for(int i = 0; i < 3; i++)
  {
    this->m_MinMaxOutput.push_back(OutputType( std::numeric_limits<int>::max() , std::numeric_limits<int>::min() ));
  }

  Point3D origin;
  Vector3D xDirection, yDirection, zDirection;
  const Vector3D spacing = m_Image->GetGeometry()->GetSpacing();

  origin = m_Image->GetGeometry()->GetOrigin();           //Left, bottom, front

  //Get axis vector for the spatial directions
  xDirection = m_Image->GetGeometry()->GetAxisVector(1);
  yDirection = m_Image->GetGeometry()->GetAxisVector(0);
  zDirection = m_Image->GetGeometry()->GetAxisVector(2);

  /* For the calculation of the intersection points we need as corner points the center-based image coordinates.
   * With the method GetCornerPoint() of the class Geometry3D we only get the corner-based coordinates.
   * Therefore we need to calculate the center-based corner points here. For that we add/substract the corner-
   * based coordinates with the spacing of the geometry3D.
   */
  for( int i = 0; i < 3; i++ )
  {
    if(xDirection[i] < 0)
    {
      xDirection[i] += spacing[i];
    }
    else if( xDirection[i] > 0 )
    {
      xDirection[i] -= spacing[i];
    }

    if(yDirection[i] < 0)
    {
      yDirection[i] += spacing[i];
    }
    else if( yDirection[i] > 0 )
    {
      yDirection[i] -= spacing[i];
    }

    if(zDirection[i] < 0)
    {
      zDirection[i] += spacing[i];
    }
    else if( zDirection[i] > 0 )
    {
      zDirection[i] -= spacing[i];
    }
  }

  Point3D LeftTopFront, LeftBottomBack, LeftTopBack;
  Point3D RightBottomFront, RightTopFront, RightBottomBack, RightTopBack;

  LeftTopFront = origin + yDirection;
  LeftBottomBack = origin + zDirection;
  LeftTopBack = origin + yDirection + zDirection;
  RightBottomFront = origin + xDirection;
  RightTopFront = origin + xDirection + yDirection;
  RightBottomBack = origin + xDirection + zDirection;
  RightTopBack = origin + xDirection + yDirection + zDirection;

  EdgesVector edgesOf3DBox;

  // connections/edges from origin
  edgesOf3DBox.push_back(std::make_pair(origin,             // x = left=xfront, y=bottom=yfront, z=front=zfront
                                        LeftTopFront));     // left, top, front

  edgesOf3DBox.push_back(std::make_pair(origin,             // left, bottom, front
                                        LeftBottomBack));   // left, bottom, back

  edgesOf3DBox.push_back(std::make_pair(origin,             // left, bottom, front
                                        RightBottomFront)); // right, bottom, front

  edgesOf3DBox.push_back(std::make_pair(LeftTopFront,       // left, top, front
                                        RightTopFront));    // right, top, front

  edgesOf3DBox.push_back(std::make_pair(LeftTopFront,       // left, top, front
                                        LeftTopBack));      // left, top, back

  edgesOf3DBox.push_back(std::make_pair(RightTopFront,      // right, top, front
                                        RightTopBack));     // right, top, back

  edgesOf3DBox.push_back(std::make_pair(RightTopFront,      // right, top, front
                                        RightBottomFront)); // right, bottom, front

  edgesOf3DBox.push_back(std::make_pair(RightBottomFront,   // right, bottom, front
                                        RightBottomBack));  // right, bottom, back

  edgesOf3DBox.push_back(std::make_pair(RightBottomBack,    // right, bottom, back
                                        LeftBottomBack));   // left, bottom, back

  edgesOf3DBox.push_back(std::make_pair(RightBottomBack,    // right, bottom, back
                                        RightTopBack));     // right, top, back

  edgesOf3DBox.push_back(std::make_pair(RightTopBack,       // right, top, back
                                        LeftTopBack));      // left, top, back

  edgesOf3DBox.push_back(std::make_pair(LeftTopBack,        // left, top, back
                                        LeftBottomBack));   // left, bottom, back



  for (EdgesVector::iterator iterator = edgesOf3DBox.begin(); iterator != edgesOf3DBox.end();iterator++)
  {
    Point3D startPoint = (*iterator).first;   // start point of the line
    Point3D endPoint   = (*iterator).second;  // end point of the line
    Vector3D lineDirection = endPoint - startPoint;

    mitk::Line3D line(startPoint, lineDirection);

    Point3D intersectionWorldPoint;
    intersectionWorldPoint.Fill(std::numeric_limits<int>::min());

    m_PlaneGeometry->IntersectionPoint(line, intersectionWorldPoint);  // Get intersection point of line and plane geometry

    double t = -1.0;
    bool isIntersectionPointOnLine;
    isIntersectionPointOnLine = m_PlaneGeometry->IntersectionPointParam(line, t);

    mitk::Point3D intersectionIndexPoint;
    m_Image->GetGeometry()->WorldToIndex(intersectionWorldPoint, intersectionIndexPoint);    //Get index point

    if(0.0 <= t && t <= 1.0 && isIntersectionPointOnLine)
    {
      for(int dim = 0; dim < 3; dim++)
      {
        // minimum
        if( this->m_MinMaxOutput[dim].first > ROUND_P(intersectionIndexPoint[dim]) )    //If new point value is lower than old
        {
            this->m_MinMaxOutput[dim].first = ROUND_P(intersectionIndexPoint[dim]);     //set new value
        }

        // maximum
        if( this->m_MinMaxOutput[dim].second < ROUND_P(intersectionIndexPoint[dim]) ) //If new point value is higher than old
        {
            this->m_MinMaxOutput[dim].second = ROUND_P(intersectionIndexPoint[dim]);     //set new value
        }
      }
    }
  }
}
