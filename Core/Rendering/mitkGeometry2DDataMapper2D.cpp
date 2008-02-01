/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkGL.h"
#include "mitkGeometry2DDataMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkSmartPointerProperty.h"
#include "mitkGeometry2DDataToSurfaceFilter.h"
#include "mitkSurfaceMapper2D.h"
#include "mitkLine.h"

mitk::Geometry2DDataMapper2D::Geometry2DDataMapper2D()
: m_SurfaceMapper(NULL)
{
}


mitk::Geometry2DDataMapper2D::~Geometry2DDataMapper2D()
{
}


const mitk::Geometry2DData *
mitk::Geometry2DDataMapper2D::GetInput(void)
{
  return static_cast<const mitk::Geometry2DData * > ( GetData() );
}


void 
mitk::Geometry2DDataMapper2D::SetDataIteratorToOtherGeometry2Ds(
  const mitk::DataTreeIteratorBase *iterator )
{
  if (m_IteratorToOtherGeometry2Ds != iterator)
  {
    m_IteratorToOtherGeometry2Ds = iterator;
    this->Modified();
  }
}


void 
mitk::Geometry2DDataMapper2D::GenerateData()
{
  // collect all Geometry2DDatas accessible by traversing
  // m_IteratorToOtherGeometry2Ds
  m_OtherGeometry2Ds.clear();
  if(m_IteratorToOtherGeometry2Ds.IsNull()) return;

  mitk::DataTreeIteratorClone it = m_IteratorToOtherGeometry2Ds;
  while(!it->IsAtEnd())
  {
    if(it->Get().IsNotNull())
    {
      mitk::BaseData* data = it->Get()->GetData();
      if(data != NULL)
      {
        mitk::Geometry2DData* geometry2dData = 
          dynamic_cast<mitk::Geometry2DData*>(data);
        if(geometry2dData!=NULL)
        {
          mitk::PlaneGeometry* planegeometry = 
            dynamic_cast<mitk::PlaneGeometry*>(geometry2dData->GetGeometry2D());

          if(planegeometry!=NULL)
            m_OtherGeometry2Ds.push_back(it->Get());
        }
      }
    }
    ++it;
  }
}


void 
mitk::Geometry2DDataMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  if ( !this->IsVisible(renderer) )
  {
    return;
  }
  
  mitk::Geometry2DData::Pointer input =
    const_cast< mitk::Geometry2DData * >(this->GetInput());

  // intersecting with ourself?
  if ( input.IsNull() || (this->GetInput()->GetGeometry2D() == 
       renderer->GetCurrentWorldGeometry2D()) )
  {
    return; // do nothing!
  }
  
  const PlaneGeometry *inputPlaneGeometry = 
    dynamic_cast< const PlaneGeometry * >( input->GetGeometry2D() );

  const PlaneGeometry *worldPlaneGeometry =
    dynamic_cast< const PlaneGeometry* >( 
      renderer->GetCurrentWorldGeometry2D() );

  if ( worldPlaneGeometry && inputPlaneGeometry 
    && inputPlaneGeometry->GetReferenceGeometry() )
  {
    DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();
    assert( displayGeometry );

    const Geometry3D *referenceGeometry = 
      inputPlaneGeometry->GetReferenceGeometry();

    // calculate intersection of the plane data with the border of the 
    // world geometry rectangle
    Point2D lineFrom, lineTo;
    
    typedef Geometry3D::TransformType TransformType;
    const TransformType *transform = dynamic_cast< const TransformType * >(
      referenceGeometry->GetIndexToWorldTransform() );

    TransformType::Pointer inverseTransform = TransformType::New();
    transform->GetInverse( inverseTransform );

    Line3D crossLine, otherCrossLine;

    // Calculate the intersection line of the input plane with the world plane
    if ( worldPlaneGeometry->IntersectionLine( 
          inputPlaneGeometry, crossLine ) )
    {
      mitk::BoundingBox::PointType boundingBoxMin, boundingBoxMax;
      boundingBoxMin = referenceGeometry->GetBoundingBox()->GetMinimum();
      boundingBoxMax = referenceGeometry->GetBoundingBox()->GetMaximum();
      if(referenceGeometry->GetImageGeometry())
      {
        for(unsigned int i = 0; i < 3; ++i)
        {
          boundingBoxMin[i]-=0.5;
          boundingBoxMax[i]-=0.5;
        }
      }

      crossLine.Transform( *inverseTransform );
      mitk::Point3D point1, point2;

      // Then, clip this line with the (transformed) bounding box of the 
      // reference geometry.
      if ( crossLine.BoxLineIntersection(
        boundingBoxMin[0], boundingBoxMin[1], boundingBoxMin[2],
        boundingBoxMax[0], boundingBoxMax[1], boundingBoxMax[2],
        crossLine.GetPoint(), crossLine.GetDirection(),
        point1, point2 ) == 2 )
      {
        // Transform the resulting line start and end points into display
        // coordinates.
        worldPlaneGeometry->Map( 
          transform->TransformPoint( point1 ), lineFrom );
        worldPlaneGeometry->Map( 
          transform->TransformPoint( point2 ), lineTo );

        mitk::Line< ScalarType, 2 > line, otherLine;
        line.SetPoints( lineFrom, lineTo );

        displayGeometry->WorldToDisplay( lineFrom, lineFrom );
        displayGeometry->WorldToDisplay( lineTo, lineTo );

        ScalarType lengthInDisplayUnits = (lineTo - lineFrom).GetNorm();

        // lineParams stores the individual segments of the line, which are
        // separated by a gap each (to mark the intersection with another
        // displayed line)
        std::vector< ScalarType > lineParams;
        lineParams.reserve( m_OtherGeometry2Ds.size() + 2 );
        lineParams.push_back( 0.0 );
        lineParams.push_back( 1.0 );

        Vector2D d, dOrth;
        
        // Now iterate through all other lines displayed in this window and
        // calculate the positions of intersection with the line to be
        // rendered; these positions will be stored in lineParams to form a
        // gap afterwards.
        NodesVectorType::iterator otherPlanesIt = m_OtherGeometry2Ds.begin();
        NodesVectorType::iterator otherPlanesEnd = m_OtherGeometry2Ds.end();
        while ( otherPlanesIt != otherPlanesEnd )
        {
          PlaneGeometry *otherPlane = static_cast< PlaneGeometry * >(
            static_cast< Geometry2DData * >(
              (*otherPlanesIt)->GetData() )->GetGeometry2D() );

          // Just as with the original line, calculate the intersaction with
          // the world geometry...
          if ( (otherPlane != inputPlaneGeometry)
            && worldPlaneGeometry->IntersectionLine( 
                 otherPlane, otherCrossLine ) )
          {
            // ... and clip the resulting line segment with the reference 
            // geometry bounding box.
            otherCrossLine.Transform( *inverseTransform );
            if ( otherCrossLine.BoxLineIntersection(
              boundingBoxMin[0], boundingBoxMin[1], boundingBoxMin[2],
              boundingBoxMax[0], boundingBoxMax[1], boundingBoxMax[2],
              otherCrossLine.GetPoint(), otherCrossLine.GetDirection(),
              point1, point2 ) == 2 )
            {
              worldPlaneGeometry->Map( 
                transform->TransformPoint( point1 ), lineFrom );
              worldPlaneGeometry->Map( 
                transform->TransformPoint( point2 ), lineTo );

              // By means of the dot product, calculate the gap position as
              // parametric value in the range [0, 1]
              otherLine.SetPoints( lineFrom, lineTo );
              d = otherLine.GetDirection();
              dOrth[0] = -d[1]; dOrth[1] = d[0];
              
              ScalarType t, norm; 
              t = ( otherLine.GetPoint1() - line.GetPoint1() ) * dOrth;
              norm = line.GetDirection() * dOrth;

              if ( fabs( norm ) > mitk::eps )
              {
                t /= norm;
                if ( (t > 0.0) && (t < 1.0) )
                {
                  lineParams.push_back(t);
                }
              }
            }
          }
          ++otherPlanesIt;
        }

        // Apply color and opacity read from the PropertyList.
        this->ApplyProperties( renderer );

        ScalarType gapSizeInPixel = 10.0;
        ScalarType gapSizeInParamUnits = 
          1.0 / lengthInDisplayUnits * gapSizeInPixel;

        std::sort( lineParams.begin(), lineParams.end() );

        Point2D p1, p2;
        ScalarType p1Param, p2Param; 
        
        p1Param = lineParams[0];
        p1 = line.GetPoint( p1Param );
        displayGeometry->WorldToDisplay( p1, p1 );

        // Iterate over all line segments and display each, with a gap
        // inbetween.
        unsigned int i, preLastLineParam = lineParams.size() - 1;
        for ( i = 1; i < preLastLineParam; ++i )
        {
          p2Param = lineParams[i] - gapSizeInParamUnits * 0.5;
          p2 = line.GetPoint( p2Param );

          if ( p2Param > p1Param )
          {
            // Convert intersection points (until now mm) to display 
            // coordinates (units).
            displayGeometry->WorldToDisplay( p2, p2 );

            // draw
            glBegin (GL_LINES);
            glVertex2f(p1[0],p1[1]);
            glVertex2f(p2[0],p2[1]);
            glEnd ();
          }

          p1Param = p2Param + gapSizeInParamUnits;
          p1 = line.GetPoint( p1Param );
          displayGeometry->WorldToDisplay( p1, p1 );
        }

        // Draw last line segment
        p2Param = lineParams[i];
        p2 = line.GetPoint( p2Param );
        displayGeometry->WorldToDisplay( p2, p2 );
        glBegin( GL_LINES );
        glVertex2f( p1[0], p1[1] );
        glVertex2f( p2[0], p2[1] );
        glEnd();
      }
    }
  }
  else
  {
    mitk::Geometry2DDataToSurfaceFilter::Pointer surfaceCreator;
    mitk::SmartPointerProperty::Pointer surfacecreatorprop;
    surfacecreatorprop = dynamic_cast< mitk::SmartPointerProperty * >(
      GetDataTreeNode()->GetProperty(
        "surfacegeometry", renderer));

    if( (surfacecreatorprop.IsNull()) || 
        (surfacecreatorprop->GetSmartPointer().IsNull()) ||
        ((surfaceCreator = dynamic_cast< mitk::Geometry2DDataToSurfaceFilter * >(
          surfacecreatorprop->GetSmartPointer().GetPointer())).IsNull())
      )
    {
      surfaceCreator = mitk::Geometry2DDataToSurfaceFilter::New();
      surfacecreatorprop = new mitk::SmartPointerProperty(surfaceCreator);
      surfaceCreator->PlaceByGeometryOn();
      GetDataTreeNode()->SetProperty( "surfacegeometry", surfacecreatorprop );
    }
    
    surfaceCreator->SetInput( input );

    // Clip the Geometry2D with the reference geometry bounds (if available)
    if ( input->GetGeometry2D()->HasReferenceGeometry() )
    {
      surfaceCreator->SetBoundingBox(
        input->GetGeometry2D()->GetReferenceGeometry()->GetBoundingBox()
      );
    }

    int res;
    bool usegeometryparametricbounds=true;
    if(GetDataTreeNode()->GetIntProperty("xresolution", res, renderer))
    {
      surfaceCreator->SetXResolution(res);
      usegeometryparametricbounds=false;        
    }
    if(GetDataTreeNode()->GetIntProperty("yresolution", res, renderer))
    {
      surfaceCreator->SetYResolution(res);
      usegeometryparametricbounds=false;        
    }
    surfaceCreator->SetUseGeometryParametricBounds(usegeometryparametricbounds);

    // Calculate the surface of the Geometry2D
    surfaceCreator->Update();
    
    if (m_SurfaceMapper.IsNull())
    {
      m_SurfaceMapper=mitk::SurfaceMapper2D::New();
    }
    m_SurfaceMapper->SetSurface(surfaceCreator->GetOutput());
    m_SurfaceMapper->SetDataTreeNode(GetDataTreeNode());
    
    m_SurfaceMapper->Paint(renderer);
  }
}
