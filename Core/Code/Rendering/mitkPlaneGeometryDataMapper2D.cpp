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


#include "mitkGL.h"
#include "mitkPlaneGeometryDataMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkSmartPointerProperty.h"
#include "mitkPlaneOrientationProperty.h"
#include "mitkPlaneGeometryDataToSurfaceFilter.h"
#include "mitkSurfaceGLMapper2D.h"
#include "mitkLine.h"
#include "mitkNodePredicateDataType.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkResliceMethodProperty.h"
#include "mitkAbstractTransformGeometry.h"


mitk::PlaneGeometryDataMapper2D::PlaneGeometryDataMapper2D()
: m_SurfaceMapper( NULL ), m_DataStorage(NULL), m_ParentNode(NULL),
  m_OtherPlaneGeometries(), m_RenderOrientationArrows( false ),
  m_ArrowOrientationPositive( true )
{
}


mitk::PlaneGeometryDataMapper2D::~PlaneGeometryDataMapper2D()
{
}


const mitk::PlaneGeometryData* mitk::PlaneGeometryDataMapper2D::GetInput(void)
{
  return static_cast<const PlaneGeometryData * > ( GetDataNode()->GetData() );
}

void mitk::PlaneGeometryDataMapper2D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
  BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  if(!ls->IsGenerateDataRequired(renderer,this,GetDataNode()))
    return;

  ls->UpdateGenerateDataTime();

  // collect all PlaneGeometryDatas accessible from the DataStorage
  m_OtherPlaneGeometries.clear();
  if (m_DataStorage.IsNull())
    return;

  mitk::NodePredicateDataType::Pointer p = mitk::NodePredicateDataType::New("PlaneGeometryData");
  mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetDerivations(m_ParentNode, p, false);
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    if(it->Value().IsNull())
      continue;

    BaseData* data = it->Value()->GetData();
    if (data == NULL)
      continue;

    PlaneGeometryData* geometry2dData = dynamic_cast<PlaneGeometryData*>(data);
    if(geometry2dData == NULL)
      continue;

    PlaneGeometry* planegeometry = dynamic_cast<PlaneGeometry*>(geometry2dData->GetPlaneGeometry());
    if (planegeometry != NULL && dynamic_cast<AbstractTransformGeometry*>(geometry2dData->GetPlaneGeometry())==NULL)
      m_OtherPlaneGeometries.push_back(it->Value());
  }
}


void mitk::PlaneGeometryDataMapper2D::Paint(BaseRenderer *renderer)
{
  bool visible = true;

  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if(!visible) return;

  PlaneGeometryData::Pointer input = const_cast< PlaneGeometryData * >(this->GetInput());

  // intersecting with ourself?
  if ( input.IsNull() || (this->GetInput()->GetPlaneGeometry() ==
       renderer->GetCurrentWorldPlaneGeometry()) )
  {
    return; // do nothing!
  }

  const PlaneGeometry *inputPlaneGeometry =
    dynamic_cast< const PlaneGeometry * >( input->GetPlaneGeometry() );

  const PlaneGeometry *worldPlaneGeometry =
    dynamic_cast< const PlaneGeometry* >(
    renderer->GetCurrentWorldPlaneGeometry() );

  if ( worldPlaneGeometry && dynamic_cast<const AbstractTransformGeometry*>(renderer->GetCurrentWorldPlaneGeometry())==NULL
    && inputPlaneGeometry && dynamic_cast<const AbstractTransformGeometry*>(input->GetPlaneGeometry() )==NULL
    && inputPlaneGeometry->GetReferenceGeometry() )
  {
    DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();
    assert( displayGeometry );

    const BaseGeometry *referenceGeometry =
      inputPlaneGeometry->GetReferenceGeometry();

    // calculate intersection of the plane data with the border of the
    // world geometry rectangle
    Point2D lineFrom, lineTo;

    const Geometry3D::TransformType *transform = dynamic_cast< const Geometry3D::TransformType * >(
      referenceGeometry->GetIndexToWorldTransform() );

    Geometry3D::TransformType::Pointer inverseTransform = Geometry3D::TransformType::New();
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

        Line< ScalarType, 2 > mainLine, otherLine;
        Line< ScalarType, 2 > primaryHelperLine, secondaryHelperLine;
        mainLine.SetPoints( lineFrom, lineTo );
        primaryHelperLine.SetPoints( lineFrom, lineTo );
        secondaryHelperLine.SetPoints( lineFrom, lineTo );

        displayGeometry->WorldToDisplay( lineFrom, lineFrom );
        displayGeometry->WorldToDisplay( lineTo, lineTo );

        ScalarType lengthInDisplayUnits = (lineTo - lineFrom).GetNorm();

        Vector2D mainLineDirectionOrthogonal;
        mainLineDirectionOrthogonal[0] = -mainLine.GetDirection()[1];
        mainLineDirectionOrthogonal[1] = mainLine.GetDirection()[0];


        // lineParams stores the individual segments of the line, which are
        // separated by a gap each (to mark the intersection with another
        // displayed line)
        std::vector< ScalarType > mainLineParams;
        std::vector< ScalarType > primaryHelperLineParams;
        std::vector< ScalarType > secondaryHelperLineParams;
        mainLineParams.reserve( m_OtherPlaneGeometries.size() + 2 );
        mainLineParams.push_back( 0.0 );
        mainLineParams.push_back( 1.0 );

        primaryHelperLineParams.reserve( m_OtherPlaneGeometries.size() + 2 );
        primaryHelperLineParams.push_back( 0.0 );
        primaryHelperLineParams.push_back( 1.0 );

        secondaryHelperLineParams.reserve( m_OtherPlaneGeometries.size() + 2 );
        secondaryHelperLineParams.push_back( 0.0 );
        secondaryHelperLineParams.push_back( 1.0 );


        // Now iterate through all other lines displayed in this window and
        // calculate the positions of intersection with the line to be
        // rendered; these positions will be stored in lineParams to form a
        // gap afterwards.
        NodesVectorType::iterator otherPlanesIt = m_OtherPlaneGeometries.begin();
        NodesVectorType::iterator otherPlanesEnd = m_OtherPlaneGeometries.end();

        DataNode* dataNodeOfInputPlaneGeometry = NULL;

        // Now we have to find the DataNode that contains the inputPlaneGeometry
        // in order to determine the state of the thick-slice rendering
        while ( otherPlanesIt != otherPlanesEnd )
        {
          PlaneGeometry *otherPlane = static_cast< PlaneGeometry * >(
            static_cast< PlaneGeometryData * >(
            (*otherPlanesIt)->GetData() )->GetPlaneGeometry() );

          // if we have found the correct node
          if ( (otherPlane == inputPlaneGeometry)
            && worldPlaneGeometry->IntersectionLine(
            otherPlane, otherCrossLine ) )
          {
            dataNodeOfInputPlaneGeometry = (*otherPlanesIt);
//            if( dataNodeOfInputPlaneGeometry )
//            {
//              mainLineThickSlicesMode = this->DetermineThickSliceMode(dataNodeOfInputPlaneGeometry, mainLineThickSlicesNum);
//            }
            break;
          }
          otherPlanesIt++;
        }

        // if we did not find a dataNode for the inputPlaneGeometry there is nothing we can do from here
        if ( dataNodeOfInputPlaneGeometry == NULL )
          return;

        // Determine if we should draw the area covered by the thick slicing, default is false.
        // This will also show the area of slices that do not have thick slice mode enabled
        bool showAreaOfThickSlicing = false;
        dataNodeOfInputPlaneGeometry->GetBoolProperty( "reslice.thickslices.showarea", showAreaOfThickSlicing );

        // get the normal of the inputPlaneGeometry
        Vector3D normal = inputPlaneGeometry->GetNormal();
        // determine the pixelSpacing in that direction
        double thickSliceDistance = SlicedGeometry3D::CalculateSpacing( referenceGeometry->GetSpacing(), normal );

        IntProperty *intProperty=0;
        if( dataNodeOfInputPlaneGeometry->GetProperty( intProperty, "reslice.thickslices.num" ) && intProperty )
            thickSliceDistance *= intProperty->GetValue()+0.5;
        else
            showAreaOfThickSlicing = false;

        // not the nicest place to do it, but we have the width of the visible bloc in MM here
        // so we store it in this fancy property
        dataNodeOfInputPlaneGeometry->SetFloatProperty( "reslice.thickslices.sizeinmm", thickSliceDistance*2 );

        if ( showAreaOfThickSlicing )
        {
          // vectorToHelperLine defines how to reach the helperLine from the mainLine
          Vector2D vectorToHelperLine;
          vectorToHelperLine = mainLineDirectionOrthogonal;
          vectorToHelperLine.Normalize();
          // got the right direction, so we multiply the width
          vectorToHelperLine *= thickSliceDistance;

          // and create the corresponding points
          primaryHelperLine.SetPoints( primaryHelperLine.GetPoint1() - vectorToHelperLine,
            primaryHelperLine.GetPoint2() - vectorToHelperLine );

          secondaryHelperLine.SetPoints( secondaryHelperLine.GetPoint1() + vectorToHelperLine,
            secondaryHelperLine.GetPoint2() + vectorToHelperLine );
        }


        //int otherLineThickSlicesMode = 0;
        int otherLineThickSlicesNum = 0;

        // by default, there is no gap for the helper lines
        ScalarType gapSize = 0.0;

        otherPlanesIt = m_OtherPlaneGeometries.begin();
        while ( otherPlanesIt != otherPlanesEnd )
        {
          PlaneGeometry *otherPlane = static_cast< PlaneGeometry * >(
            static_cast< PlaneGeometryData * >(
              (*otherPlanesIt)->GetData() )->GetPlaneGeometry() );


          // Just as with the original line, calculate the intersection with
          // the world geometry...
          if ( (otherPlane != inputPlaneGeometry)
            && worldPlaneGeometry->IntersectionLine(
                 otherPlane, otherCrossLine ) )
          {
            //otherLineThickSlicesMode = this->DetermineThickSliceMode((*otherPlanesIt), otherLineThickSlicesNum);
            Vector3D normal = otherPlane->GetNormal();

            double otherLineThickSliceDistance = SlicedGeometry3D::CalculateSpacing( referenceGeometry->GetSpacing(), normal );
            otherLineThickSliceDistance *= (otherLineThickSlicesNum)*2;

            Point2D otherLineFrom, otherLineTo;

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
                transform->TransformPoint( point1 ), otherLineFrom );
              worldPlaneGeometry->Map(
                transform->TransformPoint( point2 ), otherLineTo );

              otherLine.SetPoints( otherLineFrom, otherLineTo );

              // then we have to determine the gap position of the main line
              // by finding the position at which the two lines cross
              this->DetermineParametricCrossPositions( mainLine, otherLine, mainLineParams );


              // if the other line is also in thick slice mode, we have to determine the
              // gapsize considering the width of that other line and the spacing in its direction
              if ( showAreaOfThickSlicing )
              {
                Vector2D otherLineDirection = otherLine.GetDirection();
                otherLineDirection.Normalize();
                mainLineDirectionOrthogonal.Normalize();

                // determine the gapsize
                gapSize = fabs( otherLineThickSliceDistance / ( otherLineDirection*mainLineDirectionOrthogonal ) );
                gapSize = gapSize / displayGeometry->GetScaleFactorMMPerDisplayUnit();

                // determine the gap positions for the helper lines as well
                this->DetermineParametricCrossPositions( primaryHelperLine, otherLine, primaryHelperLineParams );
                this->DetermineParametricCrossPositions( secondaryHelperLine, otherLine, secondaryHelperLineParams );
              }
            }
          }
          ++otherPlanesIt;
        }

        // If we have to draw the helperlines, the mainline will be drawn as a dashed line
        // with a fixed gapsize of 10 pixels
        this->DrawLine(renderer,
          lengthInDisplayUnits,
          mainLine,
          mainLineParams,
          inputPlaneGeometry,
          showAreaOfThickSlicing,
          10.0
          );


        // If drawn, the helperlines are drawn as a solid line. The gapsize depends on the
        // width of the crossed line.
        if ( showAreaOfThickSlicing )
        {
          this->DrawLine(renderer,
            lengthInDisplayUnits,
            primaryHelperLine,
            primaryHelperLineParams,
            inputPlaneGeometry,
            false,
            gapSize
            );

          this->DrawLine(renderer,
            lengthInDisplayUnits,
            secondaryHelperLine,
            secondaryHelperLineParams,
            inputPlaneGeometry,
            false,
            gapSize
            );
        }
      }
    }
  }
  else
  {
    PlaneGeometryDataToSurfaceFilter::Pointer surfaceCreator;
    SmartPointerProperty::Pointer surfacecreatorprop;
    surfacecreatorprop = dynamic_cast< SmartPointerProperty * >(
      GetDataNode()->GetProperty(
        "surfacegeometry", renderer));

    if( (surfacecreatorprop.IsNull()) ||
        (surfacecreatorprop->GetSmartPointer().IsNull()) ||
        ((surfaceCreator = dynamic_cast< PlaneGeometryDataToSurfaceFilter * >(
          surfacecreatorprop->GetSmartPointer().GetPointer())).IsNull())
      )
    {
      surfaceCreator = PlaneGeometryDataToSurfaceFilter::New();
      surfacecreatorprop = SmartPointerProperty::New(surfaceCreator);
      surfaceCreator->PlaceByGeometryOn();
      GetDataNode()->SetProperty( "surfacegeometry", surfacecreatorprop );
    }

    surfaceCreator->SetInput( input );

    // Clip the PlaneGeometry with the reference geometry bounds (if available)
    if ( input->GetPlaneGeometry()->HasReferenceGeometry() )
    {
      surfaceCreator->SetBoundingBox(
        input->GetPlaneGeometry()->GetReferenceGeometry()->GetBoundingBox()
      );
    }

    int res;
    bool usegeometryparametricbounds = true;
    if ( GetDataNode()->GetIntProperty("xresolution", res, renderer))
    {
      surfaceCreator->SetXResolution(res);
      usegeometryparametricbounds=false;
    }
    if (GetDataNode()->GetIntProperty("yresolution", res, renderer))
    {
      surfaceCreator->SetYResolution(res);
      usegeometryparametricbounds=false;
    }
    surfaceCreator->SetUseGeometryParametricBounds(usegeometryparametricbounds);

    // Calculate the surface of the PlaneGeometry
    surfaceCreator->Update();

    if (m_SurfaceMapper.IsNull())
    {
      m_SurfaceMapper=SurfaceGLMapper2D::New();
    }
    m_SurfaceMapper->SetSurface(surfaceCreator->GetOutput());
    m_SurfaceMapper->SetDataNode(GetDataNode());

    m_SurfaceMapper->Paint(renderer);
  }
}

void mitk::PlaneGeometryDataMapper2D::DrawOrientationArrow( mitk::Point2D &outerPoint, mitk::Point2D &innerPoint,
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


void mitk::PlaneGeometryDataMapper2D::ApplyAllProperties( BaseRenderer *renderer )
{
  Superclass::ApplyColorAndOpacityProperties(renderer);

  PlaneOrientationProperty* decorationProperty;
  this->GetDataNode()->GetProperty( decorationProperty, "decoration", renderer );
  if ( decorationProperty != NULL )
  {
    if ( decorationProperty->GetPlaneDecoration() ==
      PlaneOrientationProperty::PLANE_DECORATION_POSITIVE_ORIENTATION )
    {
      m_RenderOrientationArrows = true;
      m_ArrowOrientationPositive = true;
    }
    else if ( decorationProperty->GetPlaneDecoration() ==
      PlaneOrientationProperty::PLANE_DECORATION_NEGATIVE_ORIENTATION )
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


void mitk::PlaneGeometryDataMapper2D::SetDatastorageAndGeometryBaseNode( mitk::DataStorage::Pointer ds, mitk::DataNode::Pointer parent )
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

void mitk::PlaneGeometryDataMapper2D::DrawLine( BaseRenderer* renderer,
                                            ScalarType lengthInDisplayUnits,
                                            Line<ScalarType,2> &line,
                                            std::vector<ScalarType> &gapPositions,
                                            const PlaneGeometry* inputPlaneGeometry,
                                            bool drawDashed,
                                            ScalarType gapSizeInPixel
                                            )
{
  DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();
  const PlaneGeometry *worldPlaneGeometry =
    dynamic_cast< const PlaneGeometry* >( renderer->GetCurrentWorldPlaneGeometry() );

  // Apply color and opacity read from the PropertyList.
  this->ApplyAllProperties( renderer );

  ScalarType gapSizeInParamUnits =
    1.0 / lengthInDisplayUnits * gapSizeInPixel;

  std::sort( gapPositions.begin(), gapPositions.end() );

  Point2D p1, p2;
  ScalarType p1Param, p2Param;

  p1Param = gapPositions[0];
  p1 = line.GetPoint( p1Param );
  displayGeometry->WorldToDisplay( p1, p1 );

  //Workaround to show the crosshair always on top of a 2D render window
  //The image is usually located at depth = 0 or negative depth values, and thus,
  //the crosshair with depth = 1 is always on top.
  float depthPosition = 1.0f;

  if ( drawDashed )
  {
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xF0F0);
  }
  glEnable(GL_DEPTH_TEST);

  // Iterate over all line segments and display each, with a gap
  // in between.
  unsigned int i, preLastLineParam = gapPositions.size() - 1;
  for ( i = 1; i < preLastLineParam; ++i )
  {
    p2Param = gapPositions[i] - gapSizeInParamUnits * 0.5;
    p2 = line.GetPoint( p2Param );

    if ( p2Param > p1Param )
    {
      // Convert intersection points (until now mm) to display
      // coordinates (units).
      displayGeometry->WorldToDisplay( p2, p2 );

      // draw
      glBegin (GL_LINES);
      glVertex3f(p1[0],p1[1], depthPosition);
      glVertex3f(p2[0],p2[1], depthPosition);
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
  p2Param = gapPositions[i];
  p2 = line.GetPoint( p2Param );
  displayGeometry->WorldToDisplay( p2, p2 );
  glBegin( GL_LINES );
  glVertex3f( p1[0], p1[1], depthPosition);
  glVertex3f( p2[0], p2[1], depthPosition);
  glEnd();

  if ( drawDashed )
  {
    glDisable(GL_LINE_STIPPLE);
  }

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

int mitk::PlaneGeometryDataMapper2D::DetermineThickSliceMode( DataNode * dn, int &thickSlicesNum )
{
  int thickSlicesMode = 0;
  // determine the state and the extend of the thick-slice mode
  mitk::ResliceMethodProperty *resliceMethodEnumProperty=0;
  if( dn->GetProperty( resliceMethodEnumProperty, "reslice.thickslices" ) && resliceMethodEnumProperty )
    thickSlicesMode = resliceMethodEnumProperty->GetValueAsId();

  IntProperty *intProperty=0;
  if( dn->GetProperty( intProperty, "reslice.thickslices.num" ) && intProperty )
  {
    thickSlicesNum = intProperty->GetValue();
    if(thickSlicesNum < 1) thickSlicesNum=0;
    if(thickSlicesNum > 10) thickSlicesNum=10;
  }

  if ( thickSlicesMode == 0 )
    thickSlicesNum = 0;

  return thickSlicesMode;
}

void mitk::PlaneGeometryDataMapper2D::DetermineParametricCrossPositions( Line< mitk::ScalarType, 2 > &mainLine,
                                                                     Line< mitk::ScalarType, 2 > &otherLine,
                                                                     std::vector< mitk::ScalarType > &crossPositions )
{
  Vector2D direction, dOrth;
  // By means of the dot product, calculate the gap position as
  // parametric value in the range [0, 1]
  direction = otherLine.GetDirection();
  dOrth[0] = -direction[1]; dOrth[1] = direction[0];


  ScalarType gapPosition = ( otherLine.GetPoint1() - mainLine.GetPoint1() ) * dOrth;
  ScalarType norm = mainLine.GetDirection() * dOrth;

  if ( fabs( norm ) > eps )
  {
    gapPosition /= norm;
    if ( (gapPosition > 0.0) && (gapPosition < 1.0) )
    {
      crossPositions.push_back(gapPosition);
    }
  }
}
