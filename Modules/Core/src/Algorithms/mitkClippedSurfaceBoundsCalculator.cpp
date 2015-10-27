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
#include "mitkClippedSurfaceBoundsCalculator.h"
#include "mitkLine.h"

#define ROUND_P(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

mitk::ClippedSurfaceBoundsCalculator::ClippedSurfaceBoundsCalculator(
    const mitk::PlaneGeometry* geometry,
    mitk::Image::Pointer image)
: m_PlaneGeometry(nullptr)
, m_Geometry3D(nullptr)
, m_Image(nullptr)
{
  this->InitializeOutput();

  this->SetInput(geometry, image);
}

mitk::ClippedSurfaceBoundsCalculator::ClippedSurfaceBoundsCalculator(
    const mitk::BaseGeometry* geometry,
    mitk::Image::Pointer image)
: m_PlaneGeometry(nullptr)
, m_Geometry3D(nullptr)
, m_Image(nullptr)
{
  this->InitializeOutput();

  this->SetInput(geometry, image);
}

mitk::ClippedSurfaceBoundsCalculator::ClippedSurfaceBoundsCalculator( const PointListType pointlist, mitk::Image::Pointer image )
: m_PlaneGeometry(nullptr)
, m_Geometry3D(nullptr)
, m_Image(image)
{
  this->InitializeOutput();

  m_ObjectPointsInWorldCoordinates = pointlist;
}

void mitk::ClippedSurfaceBoundsCalculator::InitializeOutput()
{
  // initialize with meaningless slice indices
  m_MinMaxOutput.clear();
  m_MinMaxOutput.reserve(3);
  for(int i = 0; i < 3; i++)
  {
    m_MinMaxOutput.push_back(
      OutputType( std::numeric_limits<int>::max() ,
      std::numeric_limits<int>::min() ));
  }
}

mitk::ClippedSurfaceBoundsCalculator::~ClippedSurfaceBoundsCalculator()
{
}

void
  mitk::ClippedSurfaceBoundsCalculator::SetInput(
  const mitk::PlaneGeometry* geometry,
  mitk::Image* image)
{
  if(geometry && image)
  {
    this->m_PlaneGeometry = geometry;
    this->m_Image = image;
    this->m_Geometry3D = nullptr;        //Not possible to set both
    m_ObjectPointsInWorldCoordinates.clear();
  }
}

void
  mitk::ClippedSurfaceBoundsCalculator::SetInput(
  const mitk::BaseGeometry* geometry,
  mitk::Image* image)
{
  if(geometry && image)
  {
    this->m_Geometry3D = geometry;
    this->m_Image = image;
    this->m_PlaneGeometry = nullptr;     //Not possible to set both
    m_ObjectPointsInWorldCoordinates.clear();
  }
}

void mitk::ClippedSurfaceBoundsCalculator::SetInput( const std::vector<mitk::Point3D> pointlist, mitk::Image *image )
{
  if ( !pointlist.empty() && image )
  {
    m_Geometry3D = nullptr;
    m_PlaneGeometry = nullptr;
    m_Image = image;
    m_ObjectPointsInWorldCoordinates = pointlist;
  }
}

mitk::ClippedSurfaceBoundsCalculator::OutputType
  mitk::ClippedSurfaceBoundsCalculator::GetMinMaxSpatialDirectionX()
{
  return this->m_MinMaxOutput[0];
}

mitk::ClippedSurfaceBoundsCalculator::OutputType
  mitk::ClippedSurfaceBoundsCalculator::GetMinMaxSpatialDirectionY()
{
  return this->m_MinMaxOutput[1];
}

mitk::ClippedSurfaceBoundsCalculator::OutputType
  mitk::ClippedSurfaceBoundsCalculator::GetMinMaxSpatialDirectionZ()
{
  return this->m_MinMaxOutput[2];
}

void mitk::ClippedSurfaceBoundsCalculator::Update()
{
  this->m_MinMaxOutput.clear();
  m_MinMaxOutput.reserve(3);
  for(int i = 0; i < 3; i++)
  {
    this->m_MinMaxOutput.push_back(OutputType( std::numeric_limits<int>::max() , std::numeric_limits<int>::min() ));
  }

  if(m_PlaneGeometry.IsNotNull())
  {
    this->CalculateIntersectionPoints(m_PlaneGeometry);
  }
  else if(m_Geometry3D.IsNotNull())
  {
    // go through all slices of the image, ...
    const mitk::SlicedGeometry3D* slicedGeometry3D = dynamic_cast<const mitk::SlicedGeometry3D*>( m_Geometry3D.GetPointer() );
    int allSlices = slicedGeometry3D->GetSlices();
    this->CalculateIntersectionPoints(dynamic_cast<mitk::PlaneGeometry*>(slicedGeometry3D->GetPlaneGeometry(0)));
    this->CalculateIntersectionPoints(dynamic_cast<mitk::PlaneGeometry*>(slicedGeometry3D->GetPlaneGeometry(allSlices-1)));
  }
  else if( !m_ObjectPointsInWorldCoordinates.empty() )
  {
    this->CalculateIntersectionPoints( m_ObjectPointsInWorldCoordinates );
    this->EnforceImageBounds();
  }
}

void mitk::ClippedSurfaceBoundsCalculator::CalculateIntersectionPoints(const mitk::PlaneGeometry* geometry)
{
  // SEE HEADER DOCUMENTATION for explanation

  const mitk::BaseGeometry::Pointer imageGeometry = m_Image->GetGeometry()->Clone();

  //  the cornerpoint(0) is the corner based Origin, which is original center based
  Point3D origin = imageGeometry->GetCornerPoint(0);           //Left, bottom, front

  //Get axis vector for the spatial directions
  const Vector3D xDirection = imageGeometry->GetAxisVector(0);
  const Vector3D yDirection = imageGeometry->GetAxisVector(1);
  const Vector3D zDirection = imageGeometry->GetAxisVector(2);

  const Point3D leftBottomFront = origin;
  const Point3D leftTopFront = origin + yDirection;
  const Point3D leftBottomBack = origin + zDirection;
  const Point3D leftTopBack = origin + yDirection + zDirection;
  const Point3D rightBottomFront = origin + xDirection;
  const Point3D rightTopFront = origin + xDirection + yDirection;
  const Point3D rightBottomBack = origin + xDirection + zDirection;
  const Point3D rightTopBack = origin + xDirection + yDirection + zDirection;

  typedef std::vector< std::pair<mitk::Point3D, mitk::Point3D> > EdgesVector;
  EdgesVector edgesOf3DBox;
  edgesOf3DBox.reserve(12);

  edgesOf3DBox.push_back(std::make_pair(leftBottomFront,     // x = left=xfront, y=bottom=yfront, z=front=zfront
    leftTopFront));     // left, top, front

  edgesOf3DBox.push_back(std::make_pair(leftBottomFront,    // left, bottom, front
    leftBottomBack));   // left, bottom, back

  edgesOf3DBox.push_back(std::make_pair(leftBottomFront,    // left, bottom, front
    rightBottomFront)); // right, bottom, front

  edgesOf3DBox.push_back(std::make_pair(leftTopFront,       // left, top, front
    rightTopFront));    // right, top, front

  edgesOf3DBox.push_back(std::make_pair(leftTopFront,       // left, top, front
    leftTopBack));      // left, top, back

  edgesOf3DBox.push_back(std::make_pair(rightTopFront,      // right, top, front
    rightTopBack));     // right, top, back

  edgesOf3DBox.push_back(std::make_pair(rightTopFront,      // right, top, front
    rightBottomFront)); // right, bottom, front

  edgesOf3DBox.push_back(std::make_pair(rightBottomFront,   // right, bottom, front
    rightBottomBack));  // right, bottom, back

  edgesOf3DBox.push_back(std::make_pair(rightBottomBack,    // right, bottom, back
    leftBottomBack));   // left, bottom, back

  edgesOf3DBox.push_back(std::make_pair(rightBottomBack,    // right, bottom, back
    rightTopBack));     // right, top, back

  edgesOf3DBox.push_back(std::make_pair(rightTopBack,       // right, top, back
    leftTopBack));      // left, top, back

  edgesOf3DBox.push_back(std::make_pair(leftTopBack,        // left, top, back
    leftBottomBack));   // left, bottom, back

  for ( auto iterator = edgesOf3DBox.cbegin(); iterator != edgesOf3DBox.cend(); ++iterator )
  {
    const Point3D startPoint = (*iterator).first;   // start point of the line
    const Point3D endPoint   = (*iterator).second;  // end point of the line
    const Vector3D lineDirection = endPoint - startPoint;

    const mitk::Line3D line(startPoint, lineDirection);

    // Get intersection point of line and plane geometry
    Point3D intersectionWorldPoint(std::numeric_limits<int>::min());

    double t = -1.0;
    bool doesLineIntersectWithPlane(false);

    const double norm = line.GetDirection().GetNorm();
    const double dist = geometry->Distance(line.GetPoint1());
    if( norm < mitk::eps && dist < mitk::sqrteps)
    {
      t = 1.0;
      doesLineIntersectWithPlane = true;
      intersectionWorldPoint = line.GetPoint1();
    }
    else
    {
      geometry->IntersectionPoint(line, intersectionWorldPoint);
      doesLineIntersectWithPlane = geometry->IntersectionPointParam(line, t);
    }

    //Get index point
    mitk::Point3D intersectionIndexPoint;
    imageGeometry->WorldToIndex(intersectionWorldPoint, intersectionIndexPoint);

    const bool lowerBoundGood = (0-mitk::sqrteps) <= t;
    const bool upperBoundGood = t <= 1.0 + mitk::sqrteps;
    if ( doesLineIntersectWithPlane && lowerBoundGood && upperBoundGood )
    {
      for( int dim = 0; dim < 3; ++dim )
      {
        m_MinMaxOutput[dim].first = std::min( m_MinMaxOutput[dim].first, ROUND_P(intersectionIndexPoint[dim]) );
        m_MinMaxOutput[dim].second = std::max( m_MinMaxOutput[dim].second, ROUND_P(intersectionIndexPoint[dim]) );
      }
      this->EnforceImageBounds();
    }
  }
}

void mitk::ClippedSurfaceBoundsCalculator::CalculateIntersectionPoints( PointListType pointList )
{
  PointListType::const_iterator pointIterator;

  const mitk::SlicedGeometry3D::Pointer imageGeometry = m_Image->GetSlicedGeometry();
  for ( pointIterator = pointList.cbegin(); pointIterator != pointList.cend(); ++pointIterator )
  {
    mitk::Point3D pntInIndexCoordinates;
    imageGeometry->WorldToIndex( (*pointIterator), pntInIndexCoordinates );

    m_MinMaxOutput[0].first  = pntInIndexCoordinates[0] < m_MinMaxOutput[0].first  ? ROUND_P(pntInIndexCoordinates[0]) : m_MinMaxOutput[0].first;
    m_MinMaxOutput[0].second = pntInIndexCoordinates[0] > m_MinMaxOutput[0].second ? ROUND_P(pntInIndexCoordinates[0]) : m_MinMaxOutput[0].second;

    m_MinMaxOutput[1].first  = pntInIndexCoordinates[1] < m_MinMaxOutput[1].first  ? ROUND_P(pntInIndexCoordinates[1]) : m_MinMaxOutput[1].first;
    m_MinMaxOutput[1].second = pntInIndexCoordinates[1] > m_MinMaxOutput[1].second ? ROUND_P(pntInIndexCoordinates[1]) : m_MinMaxOutput[1].second;

    m_MinMaxOutput[2].first  = pntInIndexCoordinates[2] < m_MinMaxOutput[2].first  ? ROUND_P(pntInIndexCoordinates[2]) : m_MinMaxOutput[2].first;
    m_MinMaxOutput[2].second = pntInIndexCoordinates[2] > m_MinMaxOutput[2].second ? ROUND_P(pntInIndexCoordinates[2]) : m_MinMaxOutput[2].second;
  }

  //this->EnforceImageBounds();

}

void mitk::ClippedSurfaceBoundsCalculator::EnforceImageBounds()
{
  m_MinMaxOutput[0].first = std::max( m_MinMaxOutput[0].first, 0 );
  m_MinMaxOutput[1].first = std::max( m_MinMaxOutput[1].first, 0 );
  m_MinMaxOutput[2].first = std::max( m_MinMaxOutput[2].first, 0 );

  m_MinMaxOutput[0].first = std::min( m_MinMaxOutput[0].first, (int) m_Image->GetDimension(0)-1 );
  m_MinMaxOutput[1].first = std::min( m_MinMaxOutput[1].first, (int) m_Image->GetDimension(1)-1 );
  m_MinMaxOutput[2].first = std::min( m_MinMaxOutput[2].first, (int) m_Image->GetDimension(2)-1 );

  m_MinMaxOutput[0].second = std::min( m_MinMaxOutput[0].second, (int) m_Image->GetDimension(0)-1 );
  m_MinMaxOutput[1].second = std::min( m_MinMaxOutput[1].second, (int) m_Image->GetDimension(1)-1 );
  m_MinMaxOutput[2].second = std::min( m_MinMaxOutput[2].second, (int) m_Image->GetDimension(2)-1 );

  m_MinMaxOutput[0].second = std::max( m_MinMaxOutput[0].second, 0 );
  m_MinMaxOutput[1].second = std::max( m_MinMaxOutput[1].second, 0 );
  m_MinMaxOutput[2].second = std::max( m_MinMaxOutput[2].second, 0 );
}
