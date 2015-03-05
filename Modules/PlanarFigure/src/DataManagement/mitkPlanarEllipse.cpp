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

#include "mitkPlanarEllipse.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"
#include "mitkImage.h"
#include "itkImage.h"
#include <mitkImageCast.h>

#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

mitk::PlanarEllipse::PlanarEllipse()
    : FEATURE_ID_MAJOR_AXIS(Superclass::AddFeature("Major Axis", "mm")),
      FEATURE_ID_MINOR_AXIS(Superclass::AddFeature("Minor Axis", "mm")),
      FEATURE_ID_AREA(Superclass::AddFeature("Area", "mm2")),
      m_MinRadius(0),
      m_MaxRadius(100),
      m_MinMaxRadiusContraintsActive(false),
      m_TreatAsCircle(true)
{
    // Ellipse has three control points
    this->ResetNumberOfControlPoints( 4 );
    this->SetNumberOfPolyLines( 2 );
    this->SetProperty( "closed", mitk::BoolProperty::New(true) );
}

bool mitk::PlanarEllipse::SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist )
{
    if(index == 0) // moving center point and control points accordingly
    {
        const Point2D &centerPoint = GetControlPoint( 0 );
        Point2D boundaryPoint1 = GetControlPoint( 1 );
        Point2D boundaryPoint2 = GetControlPoint( 2 );
        Point2D boundaryPoint3 = GetControlPoint( 3 );
        const vnl_vector<ScalarType> vec = (point.GetVnlVector() - centerPoint.GetVnlVector());

        boundaryPoint1[0] += vec[0];
        boundaryPoint1[1] += vec[1];
        boundaryPoint2[0] += vec[0];
        boundaryPoint2[1] += vec[1];
        boundaryPoint3[0] += vec[0];
        boundaryPoint3[1] += vec[1];
        PlanarFigure::SetControlPoint( 0, point, createIfDoesNotExist );
        PlanarFigure::SetControlPoint( 1, boundaryPoint1, createIfDoesNotExist );
        PlanarFigure::SetControlPoint( 2, boundaryPoint2, createIfDoesNotExist );
        PlanarFigure::SetControlPoint( 3, boundaryPoint3, createIfDoesNotExist );
        return true;
    }
    else if (index < 3)
    {
        PlanarFigure::SetControlPoint( index, point, createIfDoesNotExist );
        int otherIndex = index+1;
        if (otherIndex > 2)
            otherIndex = 1;

        const Point2D &centerPoint = GetControlPoint( 0 );
        Point2D otherPoint = GetControlPoint( otherIndex );
        Point2D point3 = GetControlPoint( 3 );

        const Vector2D vec1 = point - centerPoint;
        Vector2D vec2;

        if (index == 1 && m_TreatAsCircle )
        {
            const float x = vec1[0];
            vec2[0] = vec1[1];
            vec2[1] = x;

            if (index==1)
                vec2[0] *= -1;
            else
                vec2[1] *= -1;

            otherPoint = centerPoint+vec2;
            PlanarFigure::SetControlPoint( otherIndex, otherPoint, createIfDoesNotExist );
            const float r = centerPoint.EuclideanDistanceTo(otherPoint);

            // adjust additional third control point
            const Point2D p3 = this->GetControlPoint(3);
            Vector2D vec3;
            vec3[0] = p3[0]-centerPoint[0];
            vec3[1] = p3[1]-centerPoint[1];
            if (vec3[0]!=0 || vec3[1]!=0)
            {
                vec3.Normalize();
                vec3 *= r;
            }
            else
            {
                vec3[0] = r;
                vec3[1] = 0;
            }
            point3 = centerPoint + vec3;
            PlanarFigure::SetControlPoint( 3, point3, createIfDoesNotExist );
        }
        else if ( vec1.GetNorm() > 0 )
        {
            const float r = centerPoint.EuclideanDistanceTo(otherPoint);
            const float x = vec1[0];
            vec2[0] = vec1[1];
            vec2[1] = x;

            if (index==1)
                vec2[0] *= -1;
            else
                vec2[1] *= -1;

            vec2.Normalize(); vec2 *= r;

            if ( vec2.GetNorm() > 0 )
            {
                otherPoint = centerPoint+vec2;
                PlanarFigure::SetControlPoint( otherIndex, otherPoint, createIfDoesNotExist );
            }

            // adjust third control point
            Vector2D vec3 = point3 - centerPoint;
            vec3.Normalize();
            const double r1 = centerPoint.EuclideanDistanceTo( GetControlPoint( 1 ) );
            const double r2 = centerPoint.EuclideanDistanceTo( GetControlPoint( 2 ) );
            const Point2D newPoint = centerPoint + vec3*std::max(r1, r2);
            PlanarFigure::SetControlPoint( 3, newPoint, createIfDoesNotExist );

            m_TreatAsCircle = false;
        }
        return true;
    }
    else if (index == 3)
    {
        const Point2D centerPoint = GetControlPoint( 0 );
        Vector2D vec3 = point - centerPoint;
        vec3.Normalize();
        const double r1 = centerPoint.EuclideanDistanceTo( GetControlPoint( 1 ) );
        const double r2 = centerPoint.EuclideanDistanceTo( GetControlPoint( 2 ) );
        const Point2D newPoint = centerPoint + vec3*std::max(r1, r2);
        PlanarFigure::SetControlPoint( index, newPoint, createIfDoesNotExist );
        m_TreatAsCircle = false;
        return true;
    }
    return false;
}

void mitk::PlanarEllipse::PlaceFigure( const mitk::Point2D &point )
{
    PlanarFigure::PlaceFigure( point );
    m_SelectedControlPoint = 1;
}

mitk::Point2D mitk::PlanarEllipse::ApplyControlPointConstraints(unsigned int index, const Point2D &point)
{
    return point;

    Point2D indexPoint;
    this->GetPlaneGeometry()->WorldToIndex( point, indexPoint );

    BoundingBox::BoundsArrayType bounds = this->GetPlaneGeometry()->GetBounds();
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

void mitk::PlanarEllipse::GeneratePolyLine()
{
    // clear the PolyLine-Contrainer, it will be reconstructed soon enough...
    this->ClearPolyLines();

    const Point2D &centerPoint = GetControlPoint( 0 );
    const Point2D &boundaryPoint1 = GetControlPoint( 1 );
    const Point2D &boundaryPoint2 = GetControlPoint( 2 );

    Vector2D dir = boundaryPoint1 - centerPoint;
    dir.Normalize();
    vnl_matrix_fixed<float, 2, 2> rot;

    // differentiate between clockwise and counterclockwise rotation
    int start = 0;
    int end = 64;
    if (dir[1]<0)
    {
        dir[0] = -dir[0];
        start = -32;
        end = 32;
    }
    // construct rotation matrix to align ellipse with control point vector
    rot[0][0] = dir[0];
    rot[1][1] = rot[0][0];
    rot[1][0] = sin(acos(rot[0][0]));
    rot[0][1] = -rot[1][0];

    const double radius1 = centerPoint.EuclideanDistanceTo( boundaryPoint1 );
    const double radius2 = centerPoint.EuclideanDistanceTo( boundaryPoint2 );

    // Generate poly-line with 64 segments
    for ( int t = start; t < end; ++t )
    {
        const double alpha = (double) t * vnl_math::pi / 32.0;

        // construct the new polyline point ...
        vnl_vector_fixed< float, 2 > vec;
        vec[0] = radius1 * cos( alpha );
        vec[1] = radius2 * sin( alpha );
        vec = rot*vec;

        Point2D polyLinePoint;
        polyLinePoint[0] = centerPoint[0] + vec[0];
        polyLinePoint[1] = centerPoint[1] + vec[1];

        // ... and append it to the PolyLine.
        // No extending supported here, so we can set the index of the PolyLineElement to '0'
        this->AppendPointToPolyLine(0, polyLinePoint);
    }

    this->AppendPointToPolyLine(1, centerPoint);
    this->AppendPointToPolyLine(1, this->GetControlPoint(3));
}

void mitk::PlanarEllipse::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
    // A circle does not require a helper object
}

std::string mitk::PlanarEllipse::EvaluateAnnotation()
{
  double major_axis = GetQuantity(FEATURE_ID_MAJOR_AXIS);
  double minor_axis = GetQuantity(FEATURE_ID_MINOR_AXIS);
  std::stringstream ss;
  char stmp[20];

  MeasurementStatistics* stats = EvaluateStatistics();
  if (stats) {
    ss << "Mean=";
    sprintf(stmp, "%.2f", stats->Mean);
    ss << stmp;    
    sprintf(stmp, "%.2f", stats->SD);
    ss << " SD=" << stmp;    
    ss << "\nMax=" << std::to_string(stats->Max);
    ss << " Min=" << std::to_string(stats->Min);
  }
  delete stats;

  ss << "\nMajor axis=";
  sprintf(stmp, "%.2fmm\n", major_axis);
  ss << stmp << "Minor axis=";
  sprintf(stmp, "%.2fmm\n", minor_axis);
  ss << stmp;
  return ss.str();
}

mitk::PlanarEllipse::MeasurementStatistics* mitk::PlanarEllipse::EvaluateStatistics() 
{
  if (m_ImageNode) {
   mitk::BaseData* data = m_ImageNode->GetData();
    if (data) {
      mitk::Image* image = dynamic_cast<mitk::Image*>(data);
      if (image) {

        const int CENTRAL_POINT_NUM = 0;
        const int RADIUS_POINT_NUM1 = 1;
        const int RADIUS_POINT_NUM2 = 2;
        const int X = 0;
        const int Y = 1;
        const int Z = 2;
        mitk::Point3D center = GetWorldControlPoint(CENTRAL_POINT_NUM);
        double circleRadius1 = center.EuclideanDistanceTo(GetWorldControlPoint(RADIUS_POINT_NUM1));
        double circleRadius2 = center.EuclideanDistanceTo(GetWorldControlPoint(RADIUS_POINT_NUM2));

        if (circleRadius1 == 0 || circleRadius2 == 0) return NULL;

        double ymax = sqrt( ( 1 - (pow(0,2) / pow(circleRadius2,2) )) * pow(circleRadius1,2) );
        /* rotation matrix:
        x' = x \cos \theta - y \sin \theta\,,
        y' = x \sin \theta + y \cos \theta\,.
        */
        
        if (ymax < 0) return NULL;

        mitk::Point3D centerIndex;
        image->GetGeometry()->WorldToIndex(this->GetWorldControlPoint(CENTRAL_POINT_NUM), centerIndex);

        int minValue = INT32_MAX;
        int maxValue = INT32_MIN;

        typedef itk::Image<short, 3> ImageType3D;
        ImageType3D::Pointer itkImage;
		    mitk::CastToItkImage(image, itkImage);

        ImageType3D::IndexType currentIndex;
        currentIndex[Z] = centerIndex[Z];

        BoundingBox::BoundsArrayType bounds = this->GetPlaneGeometry()->GetBounds();

        //unsigned short layout = 0;
        mitk::Point3D cosPoint;
        cosPoint[X] = GetWorldControlPoint(RADIUS_POINT_NUM1)[X];
        cosPoint[Y] = center[Y];
        cosPoint[Z] = center[Z];
        double cosline = center.EuclideanDistanceTo(cosPoint);
        if (cosline == 0) {
          //layout = 1;
          cosPoint[X] = center[X];
          cosPoint[Y] = GetWorldControlPoint(RADIUS_POINT_NUM1)[Y];
          cosPoint[Z] = center[Z];
          cosline = center.EuclideanDistanceTo(GetWorldControlPoint(RADIUS_POINT_NUM1));
        }
        if (cosline == 0) return NULL;

        double theta = acos ( cosline / circleRadius1 );

        long long sum = 0;
        double sd(0), mean(0);
        long long pixCount = 0;
        double dx;
        int lIndex, rIndex;
        mitk::Point3D currentPoint; 
        std::vector<short> values;
        double tx, ty;

        double theta_cos = cos( theta );
        double theta_sin = sin( theta );

        for (double dy = ymax; dy > -ymax; dy--) {
          dx = sqrt( ( 1 - (pow(dy,2) / pow(circleRadius1,2) )) * pow(circleRadius2,2) );
          /// Matrix rotation when center is (0, 0)
          tx = dx * theta_cos - dy * theta_sin;
          ty = dx * theta_sin + dy * theta_cos;
          currentPoint[X] = center[X] - tx;
          currentPoint[Y] = center[Y] + ty;
          currentPoint[Z] = center[Z];
          image->GetGeometry()->WorldToIndex(currentPoint, centerIndex);
          lIndex = centerIndex[X];

          currentPoint[X] = center[X] + dx;
          image->GetGeometry()->WorldToIndex(currentPoint, centerIndex);
          rIndex = centerIndex[X];
          
          currentIndex[Y] = centerIndex[Y];
          for (int rowX = lIndex; rowX <= rIndex; rowX++) {
            currentIndex[X] = rowX;
            
            if ( currentIndex[X] <= bounds[0]
                || currentIndex[X] >= bounds[1]
                || currentIndex[Y] <= bounds[0]
                || currentIndex[Y] >= bounds[1]
                || currentIndex[Z] <= bounds[2]
                || currentIndex[Z] >= bounds[3]) return NULL;

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

void mitk::PlanarEllipse::EvaluateFeaturesInternal()
{
  const Point2D centerPoint = this->GetControlPoint(0);

  const auto longAxisLength = centerPoint.EuclideanDistanceTo(this->GetControlPoint(1));
  const auto shortAxisLength = centerPoint.EuclideanDistanceTo(this->GetControlPoint(2));

  this->SetQuantity(FEATURE_ID_MAJOR_AXIS, 2 * longAxisLength);
  this->SetQuantity(FEATURE_ID_MINOR_AXIS, 2 * shortAxisLength);
  this->SetQuantity(FEATURE_ID_AREA, longAxisLength * shortAxisLength * M_PI );


}


void mitk::PlanarEllipse::PrintSelf( std::ostream& os, itk::Indent indent) const
{
    Superclass::PrintSelf( os, indent );
}

 bool mitk::PlanarEllipse::Equals(const mitk::PlanarFigure& other) const
 {
   const mitk::PlanarEllipse* otherEllipse = dynamic_cast<const mitk::PlanarEllipse*>(&other);
   if ( otherEllipse )
   {
     if(this->m_TreatAsCircle != otherEllipse->m_TreatAsCircle)
       return false;

     return Superclass::Equals(other);
   }
   else
   {
     return false;
   }
 }
