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
#include "mitkPlaneDecorationProperty.h"
#include "mitkGeometry2DDataToSurfaceFilter.h"
#include "mitkSurfaceMapper2D.h"
#include "mitkLine.h"
#include "mitkNodePredicateDataType.h"


mitk::Geometry2DDataMapper2D::Geometry2DDataMapper2D()
: m_SurfaceMapper( NULL ),
  m_RenderOrientationArrows( false ),
  m_ArrowOrientationPositive( true ),
  m_ParentNode(NULL),
  m_DataStorage(NULL)
{
}


mitk::Geometry2DDataMapper2D::~Geometry2DDataMapper2D()
{
}


const mitk::Geometry2DData* mitk::Geometry2DDataMapper2D::GetInput(void)
{
  return static_cast<const Geometry2DData * > ( GetData() );
}


void mitk::Geometry2DDataMapper2D::GenerateData()
{
  // collect all Geometry2DDatas accessible from the DataStorage
  m_OtherGeometry2Ds.clear();
  if (m_DataStorage.IsNull())
    return;
  
  mitk::NodePredicateDataType::Pointer p = mitk::NodePredicateDataType::New("Geometry2DData");
  mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetDerivations(m_ParentNode, p, false);
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    if(it->Value().IsNull())
      continue;

    BaseData* data = it->Value()->GetData();
    if (data == NULL)
      continue;

    Geometry2DData* geometry2dData = dynamic_cast<Geometry2DData*>(data);
    if(geometry2dData == NULL)
      continue;

    PlaneGeometry* planegeometry = dynamic_cast<PlaneGeometry*>(geometry2dData->GetGeometry2D());
    if (planegeometry != NULL)
      m_OtherGeometry2Ds.push_back(it->Value());
  }
}


void mitk::Geometry2DDataMapper2D::Paint(BaseRenderer *renderer)
{
  if ( !this->IsVisible(renderer) )
  {
    return;
  }
  
  Geometry2DData::Pointer input = const_cast< Geometry2DData * >(this->GetInput());

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
      BoundingBox::PointType boundingBoxMin, boundingBoxMax;
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
      Point3D point1, point2;

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

        Line< ScalarType, 2 > line, otherLine;
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

              if ( fabs( norm ) > eps )
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

            if ( (i == 1) && (m_RenderOrientationArrows) )
            {
              // Draw orientation arrow for first line segment
              this->DrawOrientationArrow( p1, p2, 
                inputPlaneGeometry, worldPlaneGeometry, displayGeometry,
                m_ArrowOrientationPositive );
            }
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


        // Draw orientation arrows
        if ( m_RenderOrientationArrows )
        {
          this->DrawOrientationArrow( p2, p1, 
            inputPlaneGeometry, worldPlaneGeometry, displayGeometry,
            m_ArrowOrientationPositive );
          if ( preLastLineParam < 2 )
          {
            // If we only have one line segment, draw other arrow, too
            this->DrawOrientationArrow( p1, p2, 
              inputPlaneGeometry, worldPlaneGeometry, displayGeometry,
              m_ArrowOrientationPositive );
          }
        }
      }
    }
  }
  else
  {
    Geometry2DDataToSurfaceFilter::Pointer surfaceCreator;
    SmartPointerProperty::Pointer surfacecreatorprop;
    surfacecreatorprop = dynamic_cast< SmartPointerProperty * >(
      GetDataTreeNode()->GetProperty(
        "surfacegeometry", renderer));

    if( (surfacecreatorprop.IsNull()) || 
        (surfacecreatorprop->GetSmartPointer().IsNull()) ||
        ((surfaceCreator = dynamic_cast< Geometry2DDataToSurfaceFilter * >(
          surfacecreatorprop->GetSmartPointer().GetPointer())).IsNull())
      )
    {
      surfaceCreator = Geometry2DDataToSurfaceFilter::New();
      surfacecreatorprop = SmartPointerProperty::New(surfaceCreator);
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
    bool usegeometryparametricbounds = true;
    if ( GetDataTreeNode()->GetIntProperty("xresolution", res, renderer))
    {
      surfaceCreator->SetXResolution(res);
      usegeometryparametricbounds=false;        
    }
    if (GetDataTreeNode()->GetIntProperty("yresolution", res, renderer))
    {
      surfaceCreator->SetYResolution(res);
      usegeometryparametricbounds=false;        
    }
    surfaceCreator->SetUseGeometryParametricBounds(usegeometryparametricbounds);

    // Calculate the surface of the Geometry2D
    surfaceCreator->Update();
    
    if (m_SurfaceMapper.IsNull())
    {
      m_SurfaceMapper=SurfaceMapper2D::New();
    }
    m_SurfaceMapper->SetSurface(surfaceCreator->GetOutput());
    m_SurfaceMapper->SetDataTreeNode(GetDataTreeNode());
    
    m_SurfaceMapper->Paint(renderer);
  }
}

void mitk::Geometry2DDataMapper2D::DrawOrientationArrow( mitk::Point2D &outerPoint, mitk::Point2D &innerPoint, 
  const mitk::PlaneGeometry *planeGeometry,
  const mitk::PlaneGeometry *rendererPlaneGeometry,
  const mitk::DisplayGeometry *displayGeometry,
  bool positiveOrientation )
{
  // Draw arrows to indicate plane orientation
  // Vector along line
  Vector2D v1 = innerPoint - outerPoint;
  v1.Normalize();
  v1 *= 7.0;

  // Orthogonal vector
  Vector2D v2;
  v2[0] = v1[1];
  v2[1] = -v1[0];

  // Calculate triangle tip for one side and project it back into world
  // coordinates to determine whether it is above or below the plane
  Point2D worldPoint2D;
  Point3D worldPoint;
  displayGeometry->DisplayToWorld( outerPoint + v1 + v2, worldPoint2D );
  rendererPlaneGeometry->Map( worldPoint2D, worldPoint );

  // Initialize remaining triangle coordinates accordingly
  // (above/below state is XOR'ed with orientation flag)
  Point2D p1 = outerPoint + v1 * 2.0;
  Point2D p2 = outerPoint + v1 
    + ((positiveOrientation ^ planeGeometry->IsAbove( worldPoint )) 
    ? v2 : -v2);

  // Draw the arrow (triangle)
  glBegin( GL_TRIANGLES );
  glVertex2f( outerPoint[0], outerPoint[1] );
  glVertex2f( p1[0], p1[1] );
  glVertex2f( p2[0], p2[1] );
  glEnd();
}


void mitk::Geometry2DDataMapper2D::ApplyProperties( BaseRenderer *renderer )
{
  Superclass::ApplyProperties(renderer);

  PlaneDecorationProperty* decorationProperty;
  this->GetDataTreeNode()->GetProperty( decorationProperty, "decoration", renderer );
  if ( decorationProperty != NULL )
  {
    if ( decorationProperty->GetPlaneDecoration() == 
      PlaneDecorationProperty::PLANE_DECORATION_POSITIVE_ORIENTATION )
    {
      m_RenderOrientationArrows = true;
      m_ArrowOrientationPositive = true;
    }
    else if ( decorationProperty->GetPlaneDecoration() == 
      PlaneDecorationProperty::PLANE_DECORATION_NEGATIVE_ORIENTATION )
    {
      m_RenderOrientationArrows = true;
      m_ArrowOrientationPositive = false;
    }
    else
    {
      m_RenderOrientationArrows = false;
    }
  }
}


void mitk::Geometry2DDataMapper2D::SetDatastorageAndGeometryBaseNode( mitk::DataStorage::Pointer ds, mitk::DataTreeNode::Pointer parent )
{
  if (ds.IsNotNull())
  {
    m_DataStorage = ds;
  }
  if (parent.IsNotNull())
  {
    m_ParentNode = parent;
  }
}
