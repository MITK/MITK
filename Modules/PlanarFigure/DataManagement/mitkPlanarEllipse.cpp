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
#include "mitkGeometry2D.h"
#include <itkEllipseSpatialObject.h>
#include "mitkProperties.h"


mitk::PlanarEllipse::PlanarEllipse()
    : FEATURE_ID_RADIUS( this->AddFeature( "Radius", "mm" ) ),
      FEATURE_ID_DIAMETER( this->AddFeature( "Diameter", "mm" ) ),
      FEATURE_ID_AREA( this->AddFeature( "Area", "mm2" ) ),
      m_MinRadius(0),
      m_MaxRadius(100),
      m_MinMaxRadiusContraintsActive(false),
      m_TreatAsCircle(true)
{
    // Ellipse has three control points
    this->ResetNumberOfControlPoints( 3 );
    this->SetNumberOfPolyLines( 1 );
    this->SetProperty( "closed", mitk::BoolProperty::New(true) );
}


mitk::PlanarEllipse::~PlanarEllipse()
{
}

bool mitk::PlanarEllipse::SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist )
{
    if(index == 0) // moving center point and control points accordingly
    {
        const Point2D &centerPoint = GetControlPoint( 0 );
        Point2D boundaryPoint1 = GetControlPoint( 1 );
        Point2D boundaryPoint2 = GetControlPoint( 2 );
        vnl_vector<float> vec = (point.GetVnlVector() - centerPoint.GetVnlVector());

        boundaryPoint1[0] += vec[0];
        boundaryPoint1[1] += vec[1];
        boundaryPoint2[0] += vec[0];
        boundaryPoint2[1] += vec[1];
        PlanarFigure::SetControlPoint( 0, point, createIfDoesNotExist );
        PlanarFigure::SetControlPoint( 1, boundaryPoint1, createIfDoesNotExist );
        PlanarFigure::SetControlPoint( 2, boundaryPoint2, createIfDoesNotExist );
    }
    else
    {
        PlanarFigure::SetControlPoint( index, point, createIfDoesNotExist );
        int otherIndex = index+1;
        if (otherIndex > 2)
            otherIndex = 1;

        const Point2D &centerPoint = GetControlPoint( 0 );
        Point2D otherPoint = GetControlPoint( otherIndex );

        Vector2D vec1 = centerPoint - point;
        Vector2D vec2;
        float radius1 = centerPoint.EuclideanDistanceTo(otherPoint);

        if (index == 1 && m_TreatAsCircle )
        {
            float x = vec1[0];
            vec2[0] = vec1[1];
            vec2[1] = x;

            if (index==1)
                vec2[0] *= -1;
            else
                vec2[1] *= -1;

            otherPoint = centerPoint+vec2;
            PlanarFigure::SetControlPoint( otherIndex, otherPoint, createIfDoesNotExist );
        }
        else
        {
            float x = vec1[0];
            vec2[0] = vec1[1];
            vec2[1] = x;

            if (index==1)
                vec2[0] *= -1;
            else
                vec2[1] *= -1;

            vec2.Normalize(); vec2 *= radius1;

            if (vec2.Length>0)
            {
                otherPoint = centerPoint+vec2;
                PlanarFigure::SetControlPoint( otherIndex, otherPoint, createIfDoesNotExist );
            }

            m_TreatAsCircle = false;
        }
    }
}

void mitk::PlanarEllipse::PlaceFigure( const mitk::Point2D &point )
{
  PlanarFigure::PlaceFigure( point );
  m_SelectedControlPoint = 1;
}

mitk::Point2D mitk::PlanarEllipse::ApplyControlPointConstraints(unsigned int index, const Point2D &point)
{
    if ( this->GetGeometry2D() ==  NULL )
    {
        return point;
    }

    Point2D indexPoint;
    this->GetGeometry2D()->WorldToIndex( point, indexPoint );

    BoundingBox::BoundsArrayType bounds = this->GetGeometry2D()->GetBounds();
    if ( indexPoint[0] < bounds[0] ) { indexPoint[0] = bounds[0]; }
    if ( indexPoint[0] > bounds[1] ) { indexPoint[0] = bounds[1]; }
    if ( indexPoint[1] < bounds[2] ) { indexPoint[1] = bounds[2]; }
    if ( indexPoint[1] > bounds[3] ) { indexPoint[1] = bounds[3]; }

    Point2D constrainedPoint;
    this->GetGeometry2D()->IndexToWorld( indexPoint, constrainedPoint );

    if(m_MinMaxRadiusContraintsActive)
    {
        if( index != 0)
        {
            const Point2D &centerPoint = this->GetControlPoint(0);
            double euclideanDinstanceFromCenterToPoint1 = centerPoint.EuclideanDistanceTo(point);

            Vector2D vectorProjectedPoint;
            vectorProjectedPoint = point - centerPoint;
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
    // TODO: start circle at specified boundary point...

    // clear the PolyLine-Contrainer, it will be reconstructed soon enough...
    this->ClearPolyLines();

    const Point2D &centerPoint = GetControlPoint( 0 );
    const Point2D &boundaryPoint1 = GetControlPoint( 1 );
    const Point2D &boundaryPoint2 = GetControlPoint( 2 );

    Vector2D dir = centerPoint-boundaryPoint1; dir.Normalize();
    vnl_matrix_fixed<float, 2, 2> rot;

    if (dir[1]<0)
        dir[0] = -dir[0];

    rot[0][0] = dir[0];
    rot[1][1] = rot[0][0];
    rot[1][0] = sin(acos(rot[0][0]));
    rot[0][1] = -rot[1][0];

    MITK_INFO << acos(rot[0][0])*180.0/vnl_math::pi;

    double radius1 = centerPoint.EuclideanDistanceTo( boundaryPoint1 );
    double radius2 = centerPoint.EuclideanDistanceTo( boundaryPoint2 );

    // Generate poly-line with 64 segments
    for ( int t = 0; t < 64; ++t )
    {
        double alpha = (double) t * vnl_math::pi / 32.0;

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
        AppendPointToPolyLine( 0, PolyLineElement( polyLinePoint, 0 ) );
    }
}

void mitk::PlanarEllipse::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
    // A circle does not require a helper object
}

void mitk::PlanarEllipse::EvaluateFeaturesInternal()
{
    // Calculate circle radius and area
    const Point3D &p0 = this->GetWorldControlPoint( 0 );
    const Point3D &p1 = this->GetWorldControlPoint( 1 );

    double radius = p0.EuclideanDistanceTo( p1 );
    double area = vnl_math::pi * radius * radius;

    this->SetQuantity( FEATURE_ID_RADIUS, radius );
    this->SetQuantity( FEATURE_ID_DIAMETER, 2*radius );
    this->SetQuantity( FEATURE_ID_AREA, area );
}


void mitk::PlanarEllipse::PrintSelf( std::ostream& os, itk::Indent indent) const
{
    Superclass::PrintSelf( os, indent );
}
