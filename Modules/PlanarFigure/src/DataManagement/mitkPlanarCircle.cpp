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


#include "mitkPlanarCircle.h"
#include "mitkPlaneGeometry.h"

#include "mitkProperties.h"
#include "mitkImage.h"
#include "itkImage.h"
#include <mitkImageCast.h>


mitk::PlanarCircle::PlanarCircle()
: FEATURE_ID_RADIUS( this->AddFeature( "Radius", "mm" ) ),
  FEATURE_ID_DIAMETER( this->AddFeature( "Diameter", "mm" ) ),
  FEATURE_ID_AREA( this->AddFeature( "Area", "mm2" ) ),
  m_MinRadius(0),
  m_MaxRadius(100),
  m_MinMaxRadiusContraintsActive(false)
{
  // Circle has two control points
  this->ResetNumberOfControlPoints( 2 );
  this->SetNumberOfPolyLines( 1 );
  this->SetProperty( "closed", mitk::BoolProperty::New(true) );
}

bool mitk::PlanarCircle::SetControlPoint( unsigned int index, const Point2D &point, bool /*createIfDoesNotExist*/ )
{
  // moving center point
  if(index == 0)
  {
    const Point2D &centerPoint = GetControlPoint( 0 );
    Point2D boundaryPoint = GetControlPoint( 1 );
    const vnl_vector<ScalarType> vec = (point.GetVnlVector() - centerPoint.GetVnlVector());

    boundaryPoint[0] += vec[0];
    boundaryPoint[1] += vec[1];
    PlanarFigure::SetControlPoint( 0, point );
    PlanarFigure::SetControlPoint( 1, boundaryPoint );
    return true;
  }
  else if ( index == 1 )
  {
    PlanarFigure::SetControlPoint( index, point );
    return true;
  }
  return false;
}

mitk::Point2D mitk::PlanarCircle::ApplyControlPointConstraints(unsigned int index, const Point2D &point)
{
  if ( this->GetPlaneGeometry() ==  nullptr )
  {
    return point;
  }

  Point2D indexPoint;
  this->GetPlaneGeometry()->WorldToIndex( point, indexPoint );

  const BoundingBox::BoundsArrayType bounds = this->GetPlaneGeometry()->GetBounds();
  if ( indexPoint[0] < bounds[0] ) { indexPoint[0] = bounds[0]; }
  if ( indexPoint[0] > bounds[1] ) { indexPoint[0] = bounds[1]; }
  if ( indexPoint[1] < bounds[2] ) { indexPoint[1] = bounds[2]; }
  if ( indexPoint[1] > bounds[3] ) { indexPoint[1] = bounds[3]; }

  Point2D constrainedPoint;
  this->GetPlaneGeometry()->IndexToWorld( indexPoint, constrainedPoint );

  if(m_MinMaxRadiusContraintsActive)
  {
    if( index != 0)
    {
      const Point2D &centerPoint = this->GetControlPoint(0);
      const double euclideanDinstanceFromCenterToPoint1 = centerPoint.EuclideanDistanceTo(point);

      Vector2D vectorProjectedPoint = point - centerPoint;
      vectorProjectedPoint.Normalize();

      if( euclideanDinstanceFromCenterToPoint1 > m_MaxRadius )
      {
        vectorProjectedPoint *= m_MaxRadius;
        constrainedPoint = centerPoint;
        constrainedPoint += vectorProjectedPoint;
      }
      else if( euclideanDinstanceFromCenterToPoint1 < m_MinRadius )
      {
        vectorProjectedPoint *= m_MinRadius;
        constrainedPoint = centerPoint;
        constrainedPoint += vectorProjectedPoint;
      }
    }
  }

  return constrainedPoint;
}

void mitk::PlanarCircle::GeneratePolyLine()
{
  // TODO: start circle at specified boundary point...

  // clear the PolyLine-Contrainer, it will be reconstructed soon enough...
  this->ClearPolyLines();

  const Point2D &centerPoint = GetControlPoint( 0 );
  const Point2D &boundaryPoint = GetControlPoint( 1 );

  const double radius = centerPoint.EuclideanDistanceTo( boundaryPoint );

  // Generate poly-line with 64 segments
  for ( int t = 0; t < 64; ++t )
  {
    const double alpha = (double) t * vnl_math::pi / 32.0;

    // construct the new polyline point ...
    Point2D polyLinePoint;
    polyLinePoint[0] = centerPoint[0] + radius * cos( alpha );
    polyLinePoint[1] = centerPoint[1] + radius * sin( alpha );

    // ... and append it to the PolyLine.
    // No extending supported here, so we can set the index of the PolyLineElement to '0'
    this->AppendPointToPolyLine(0, polyLinePoint);
  }
}

void mitk::PlanarCircle::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // A circle does not require a helper object
}

mitk::PlanarCircle::MeasurementStatistics* mitk::PlanarCircle::EvaluateStatistics() 
{
  if (m_ImageNode) {
    mitk::BaseData* data = m_ImageNode->GetData();
    if (data) {
      mitk::Image* image = dynamic_cast<mitk::Image*>(data);
      if (image) {

        const int CENTRAL_POINT_NUM = 0;
        const int RADIUS_POINT_NUM = 1;
        const int X = 0;
        const int Y = 1;
        const int Z = 2;        

        double circleRadius = GetWorldControlPoint(CENTRAL_POINT_NUM).EuclideanDistanceTo(GetWorldControlPoint(RADIUS_POINT_NUM));
        double circleRadiusSqr = circleRadius*circleRadius;

        mitk::Point3D centerIndex;
        image->GetGeometry()->WorldToIndex(this->GetWorldControlPoint(CENTRAL_POINT_NUM), centerIndex);

        mitk::Point3D center = GetWorldControlPoint(CENTRAL_POINT_NUM);

        int minValue = INT32_MAX;
        int maxValue = INT32_MIN;

        typedef itk::Image<short, 3> ImageType3D;
        ImageType3D::Pointer itkImage;
        mitk::CastToItkImage(image, itkImage);

        ImageType3D::IndexType currentIndex;
        currentIndex[Z] = centerIndex[Z];

        long long sum = 0;
        double sd(0), mean(0);
        long long pixCount = 0;
        double dx;
        int lIndex, rIndex;
        mitk::Point3D currentPoint; 
        std::vector<short> values;
        for (double dy = circleRadius; dy > - circleRadius; dy--) {
          dx = sqrt(circleRadiusSqr - dy*dy);
          currentPoint[X] = center[X] - dx;
          currentPoint[Y] = center[Y] + dy;
          currentPoint[Z] = 0;
          image->GetGeometry()->WorldToIndex(currentPoint, centerIndex);
          lIndex = centerIndex[X];

          currentPoint[X] = center[X] + dx;
          image->GetGeometry()->WorldToIndex(currentPoint, centerIndex);
          rIndex = centerIndex[X];
          
          currentIndex[Y] = centerIndex[Y];
          for (int rowX = lIndex; rowX <= rIndex; rowX++) {
            currentIndex[X] = rowX;
            short val = itkImage->GetPixel(currentIndex);
            values.push_back(val);
            if (val < minValue) {
              minValue = val;
            } else if (val > maxValue) {
              maxValue = val;
            }
            sum += val;
            pixCount++;
          } 
        }

        if (pixCount > 0) {
          mean = (double)sum/pixCount;
          sum = 0;
          for (std::vector<short>::const_iterator it = values.begin(); it != values.end(); ++it) {
            sum += (*it - mean)*(*it - mean);
          }
          sd = sqrt((double)sum/pixCount);
        } else {
          maxValue = 0;
          minValue = 0;
        }

        MeasurementStatistics* stats = new MeasurementStatistics();
        stats->Mean = mean;
        stats->Max = maxValue;
        stats->Min = minValue;
        stats->SD = sd;

        return stats;
      }
    }
  }
  return NULL;
}



std::string mitk::PlanarCircle::EvaluateAnnotation()
{
  double diameter = GetQuantity(FEATURE_ID_DIAMETER);
  double area = GetQuantity(FEATURE_ID_AREA);

  MeasurementStatistics* stats = EvaluateStatistics();
  char str[20];
  std::string res;
  if (stats) {
    res = "Mean=";
    sprintf(str, "%.2f", stats->Mean);
    res += str;    
    sprintf(str, "%.2f", stats->SD);
    res += " SD=";
    res += str;    
    res += "\nMax=" + std::to_string(stats->Max);
    res += " Min=" + std::to_string(stats->Min);
  }
  delete stats;
  
  sprintf(str, "%.2f", diameter);
  res += "\nD=";
  res += str;
  res += " mm";
  res += "\n";

  sprintf(str, "%.2f", area);
  res += "Area=";
  res += str;
  res += " mm\xC2\xB2";

  return res;
}

void mitk::PlanarCircle::EvaluateFeaturesInternal()
{
  // Calculate circle radius and area
  const Point3D &p0 = this->GetWorldControlPoint( 0 );
  const Point3D &p1 = this->GetWorldControlPoint( 1 );

  const double radius = p0.EuclideanDistanceTo( p1 );
  const double area = vnl_math::pi * radius * radius;

  this->SetQuantity( FEATURE_ID_RADIUS, radius );
  this->SetQuantity( FEATURE_ID_DIAMETER, 2*radius );
  this->SetQuantity( FEATURE_ID_AREA, area );
}


void mitk::PlanarCircle::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
}

bool mitk::PlanarCircle::SetCurrentControlPoint( const Point2D& point )
{
  if ( m_SelectedControlPoint < 0 )
  {
    m_SelectedControlPoint = 1;
  }

  return this->SetControlPoint( m_SelectedControlPoint, point, false);
}

 bool mitk::PlanarCircle::Equals(const PlanarFigure &other) const
 {
   const mitk::PlanarCircle* otherCircle = dynamic_cast<const mitk::PlanarCircle*>(&other);
   if ( otherCircle )
   {
     return Superclass::Equals(other);
   }
   else
   {
     return false;
   }
 }
