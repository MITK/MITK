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
#include "mitkProperties.h"


mitk::PlanarEllipse::PlanarEllipse()
    : m_MinRadius(0),
      m_MaxRadius(100),
      m_MinMaxRadiusContraintsActive(false),
      m_TreatAsCircle(true)
{
    // Ellipse has three control points
    this->ResetNumberOfControlPoints( 4 );
    this->SetNumberOfPolyLines( 2 );
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
        Point2D boundaryPoint3 = GetControlPoint( 3 );
        vnl_vector<float> vec = (point.GetVnlVector() - centerPoint.GetVnlVector());

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

        Vector2D vec1 = point - centerPoint;
        Vector2D vec2;
        float r = centerPoint.EuclideanDistanceTo(otherPoint);

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

            // adjust additional third control point
            Vector2D vec3;
            vec3[0] = r;
            vec3[1] = 0;
            point3 = centerPoint + vec3;
            PlanarFigure::SetControlPoint( 3, point3, createIfDoesNotExist );
        }
        else if ( vec1.GetNorm() > 0 )
        {
            float x = vec1[0];
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
            Vector2D vec3 = point3 - centerPoint; vec3.Normalize();
            double r1 = centerPoint.EuclideanDistanceTo( GetControlPoint( 1 ) );
            double r2 = centerPoint.EuclideanDistanceTo( GetControlPoint( 2 ) );
            Point2D newPoint = centerPoint + vec3*std::max(r1, r2);
            PlanarFigure::SetControlPoint( 3, newPoint, createIfDoesNotExist );

            m_TreatAsCircle = false;
        }
        return true;
    }
    else if (index == 3)
    {
        Point2D centerPoint = GetControlPoint( 0 );
        Vector2D vec3 = point - centerPoint; vec3.Normalize();
        double r1 = centerPoint.EuclideanDistanceTo( GetControlPoint( 1 ) );
        double r2 = centerPoint.EuclideanDistanceTo( GetControlPoint( 2 ) );
        Point2D newPoint = centerPoint + vec3*std::max(r1, r2);
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
    // clear the PolyLine-Contrainer, it will be reconstructed soon enough...
    this->ClearPolyLines();

    const Point2D &centerPoint = GetControlPoint( 0 );
    const Point2D &boundaryPoint1 = GetControlPoint( 1 );
    const Point2D &boundaryPoint2 = GetControlPoint( 2 );

    Vector2D dir = boundaryPoint1 - centerPoint; dir.Normalize();
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

    double radius1 = centerPoint.EuclideanDistanceTo( boundaryPoint1 );
    double radius2 = centerPoint.EuclideanDistanceTo( boundaryPoint2 );

    // Generate poly-line with 64 segments
    for ( int t = start; t < end; ++t )
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

    AppendPointToPolyLine( 1, PolyLineElement( centerPoint, 0 ) );
    AppendPointToPolyLine( 1, PolyLineElement( GetControlPoint( 3 ), 0 ) );
}

void mitk::PlanarEllipse::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
    // A circle does not require a helper object
}

void mitk::PlanarEllipse::EvaluateFeaturesInternal()
{

}


void mitk::PlanarEllipse::PrintSelf( std::ostream& os, itk::Indent indent) const
{
    Superclass::PrintSelf( os, indent );
}
