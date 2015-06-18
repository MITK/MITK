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

#include "mitkPlaneGeometryDataMapper2D.h"

//mitk includes
#include "mitkVtkPropRenderer.h"
#include <mitkProperties.h>
#include <mitkDataNode.h>
#include <mitkPointSet.h>
#include <mitkPlaneGeometry.h>
#include <mitkPlaneOrientationProperty.h>
#include <mitkLine.h>
#include <mitkAbstractTransformGeometry.h>
#include <mitkResliceMethodProperty.h>
#include <mitkSlicedGeometry3D.h>

//vtk includes
#include <mitkIPropertyAliases.h>
#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkLine.h>
#include <vtkPoints.h>
#include <vtkTriangle.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>

mitk::PlaneGeometryDataMapper2D::AllInstancesContainer mitk::PlaneGeometryDataMapper2D::s_AllInstances;

// input for this mapper ( = PlaneGeometryData)
const mitk::PlaneGeometryData* mitk::PlaneGeometryDataMapper2D::GetInput() const
{
  return static_cast< PlaneGeometryData * >(GetDataNode()->GetData());
}

mitk::PlaneGeometryDataMapper2D::PlaneGeometryDataMapper2D()
  : m_RenderOrientationArrows( false ),
    m_ArrowOrientationPositive( true ),
    m_DepthValue(1.0f)
{
  s_AllInstances.insert(this);
}

mitk::PlaneGeometryDataMapper2D::~PlaneGeometryDataMapper2D()
{
  s_AllInstances.erase(this);
}

vtkProp* mitk::PlaneGeometryDataMapper2D::GetVtkProp(mitk::BaseRenderer * renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_CrosshairAssembly;
}

void mitk::PlaneGeometryDataMapper2D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  // The PlaneGeometryDataMapper2D mapper is special in that the rendering of
  // OTHER PlaneGeometryDatas affects how we render THIS PlaneGeometryData
  // (for the gap at the point where they intersect). A change in any of the
  // other PlaneGeometryData nodes could mean that we render ourself
  // differently, so we check for that here.
  bool generateDataRequired = false;
  for (AllInstancesContainer::iterator it = s_AllInstances.begin();
       it != s_AllInstances.end();
       ++it)
  {
    generateDataRequired = ls->IsGenerateDataRequired(renderer, this, (*it)->GetDataNode());
    if (generateDataRequired) break;
  }

  ls->UpdateGenerateDataTime();

  // Collect all other PlaneGeometryDatas that are being mapped by this mapper
  m_OtherPlaneGeometries.clear();

  for (AllInstancesContainer::iterator it = s_AllInstances.begin(); it != s_AllInstances.end(); ++it)
  {
    Self *otherInstance = *it;

    // Skip ourself
    if (otherInstance == this) continue;

    mitk::DataNode *otherNode = otherInstance->GetDataNode();
    if (!otherNode) continue;

    // Skip other PlaneGeometryData nodes that are not visible on this renderer
    if (!otherNode->IsVisible(renderer)) continue;

    PlaneGeometryData* otherData = dynamic_cast<PlaneGeometryData*>(otherNode->GetData());
    if (!otherData) continue;

    PlaneGeometry* otherGeometry = dynamic_cast<PlaneGeometry*>(otherData->GetPlaneGeometry());
    if ( otherGeometry && !dynamic_cast<AbstractTransformGeometry*>(otherData->GetPlaneGeometry()) )
    {
      m_OtherPlaneGeometries.push_back(otherNode);
    }
  }

  CreateVtkCrosshair(renderer);

  ApplyAllProperties(renderer);
}

void mitk::PlaneGeometryDataMapper2D::CreateVtkCrosshair(mitk::BaseRenderer *renderer)
{
  bool visible = true;
  LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
  ls->m_CrosshairActor->SetVisibility(0);
  ls->m_ArrowActor->SetVisibility(0);
  ls->m_CrosshairHelperLineActor->SetVisibility(0);

  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if(!visible)
  {
    return;
  }

  PlaneGeometryData::Pointer input = const_cast< PlaneGeometryData * >(this->GetInput());
  mitk::DataNode* geometryDataNode = renderer->GetCurrentWorldPlaneGeometryNode();
  const PlaneGeometryData* rendererWorldPlaneGeometryData = dynamic_cast< PlaneGeometryData * >(geometryDataNode->GetData());

  // intersecting with ourself?
  if ( input.IsNull() || input.GetPointer() == rendererWorldPlaneGeometryData)
  {
    return; //nothing to do in this case
  }

  const PlaneGeometry *inputPlaneGeometry = dynamic_cast< const PlaneGeometry * >( input->GetPlaneGeometry() );

  const PlaneGeometry* worldPlaneGeometry = dynamic_cast< const PlaneGeometry* >(
        rendererWorldPlaneGeometryData->GetPlaneGeometry() );

  if ( worldPlaneGeometry && dynamic_cast<const AbstractTransformGeometry*>(worldPlaneGeometry)==NULL
       && inputPlaneGeometry && dynamic_cast<const AbstractTransformGeometry*>(input->GetPlaneGeometry() )==NULL
       && inputPlaneGeometry->GetReferenceGeometry() )
  {
    const BaseGeometry *referenceGeometry = inputPlaneGeometry->GetReferenceGeometry();

    // calculate intersection of the plane data with the border of the
    // world geometry rectangle
    Point3D point1, point2;

    Line3D crossLine;

    // Calculate the intersection line of the input plane with the world plane
    if ( worldPlaneGeometry->IntersectionLine( inputPlaneGeometry, crossLine ) )
    {
      Point3D boundingBoxMin, boundingBoxMax;
      boundingBoxMin = referenceGeometry->GetCornerPoint(0);
      boundingBoxMax = referenceGeometry->GetCornerPoint(7);

      Point3D indexLinePoint;
      Vector3D indexLineDirection;

      referenceGeometry->WorldToIndex(crossLine.GetPoint(),indexLinePoint);
      referenceGeometry->WorldToIndex(crossLine.GetDirection(),indexLineDirection);

      referenceGeometry->WorldToIndex(boundingBoxMin,boundingBoxMin);
      referenceGeometry->WorldToIndex(boundingBoxMax,boundingBoxMax);

      // Then, clip this line with the (transformed) bounding box of the
      // reference geometry.
      Line3D::BoxLineIntersection(
            boundingBoxMin[0], boundingBoxMin[1], boundingBoxMin[2],
          boundingBoxMax[0], boundingBoxMax[1], boundingBoxMax[2],
          indexLinePoint, indexLineDirection,
          point1, point2 );

      referenceGeometry->IndexToWorld(point1,point1);
      referenceGeometry->IndexToWorld(point2,point2);
      crossLine.SetPoints(point1,point2);

      vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
      vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
      vtkSmartPointer<vtkPolyData> linesPolyData = vtkSmartPointer<vtkPolyData>::New();


      // Now iterate through all other lines displayed in this window and
      // calculate the positions of intersection with the line to be
      // rendered; these positions will be stored in lineParams to form a
      // gap afterwards.
      NodesVectorType::iterator otherPlanesIt = m_OtherPlaneGeometries.begin();
      NodesVectorType::iterator otherPlanesEnd = m_OtherPlaneGeometries.end();

      std::vector<Point3D> intersections;

      intersections.push_back(point1);

      otherPlanesIt = m_OtherPlaneGeometries.begin();
      int gapsize = 32;
      this->GetDataNode()->GetPropertyValue( "Crosshair.Gap Size",gapsize, NULL );


      ScalarType lineLength = point1.EuclideanDistanceTo(point2);
      DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();

      ScalarType gapinmm = gapsize * displayGeometry->GetScaleFactorMMPerDisplayUnit();

      float gapSizeParam = gapinmm / lineLength;

      while ( otherPlanesIt != otherPlanesEnd )
      {
        PlaneGeometry *otherPlane = static_cast< PlaneGeometry * >(
              static_cast< PlaneGeometryData * >((*otherPlanesIt)->GetData() )->GetPlaneGeometry() );

        if (otherPlane != inputPlaneGeometry && otherPlane != worldPlaneGeometry)
        {
          Point3D planeIntersection;
          otherPlane->IntersectionPoint(crossLine,planeIntersection);
          ScalarType sectionLength = point1.EuclideanDistanceTo(planeIntersection);
          ScalarType lineValue = sectionLength/lineLength;
          if(lineValue-gapSizeParam > 0.0)
            intersections.push_back(crossLine.GetPoint(lineValue-gapSizeParam));
          else intersections.pop_back();
          if(lineValue+gapSizeParam < 1.0)
            intersections.push_back(crossLine.GetPoint(lineValue+gapSizeParam));
        }
        ++otherPlanesIt;
      }
      if(intersections.size()%2 == 1)
        intersections.push_back(point2);

      if(intersections.empty())
      {
        this->DrawLine(point1,point2,lines,points);
      }
      else
        for(unsigned int i = 0 ; i< intersections.size()-1 ; i+=2)
        {
          this->DrawLine(intersections[i],intersections[i+1],lines,points);
        }

      // Add the points to the dataset
      linesPolyData->SetPoints(points);
      // Add the lines to the dataset
      linesPolyData->SetLines(lines);

      Vector3D orthogonalVector;
      orthogonalVector = inputPlaneGeometry->GetNormal();
      worldPlaneGeometry->Project(orthogonalVector,orthogonalVector);
      orthogonalVector.Normalize();

      // Visualize
      ls->m_Mapper->SetInputData(linesPolyData);
      ls->m_CrosshairActor->SetMapper(ls->m_Mapper);

      // Determine if we should draw the area covered by the thick slicing, default is false.
      // This will also show the area of slices that do not have thick slice mode enabled
      bool showAreaOfThickSlicing = false;
      GetDataNode()->GetBoolProperty( "reslice.thickslices.showarea", showAreaOfThickSlicing );

      // determine the pixelSpacing in that direction
      double thickSliceDistance = SlicedGeometry3D::CalculateSpacing( referenceGeometry->GetSpacing(), orthogonalVector );

      IntProperty *intProperty=0;
      if( GetDataNode()->GetProperty( intProperty, "reslice.thickslices.num" ) && intProperty )
        thickSliceDistance *= intProperty->GetValue()+0.5;
      else
        showAreaOfThickSlicing = false;

      // not the nicest place to do it, but we have the width of the visible bloc in MM here
      // so we store it in this fancy property
      GetDataNode()->SetFloatProperty( "reslice.thickslices.sizeinmm", thickSliceDistance*2 );

      ls->m_CrosshairActor->SetVisibility(1);

      vtkSmartPointer<vtkPolyData> arrowPolyData = vtkSmartPointer<vtkPolyData>::New();
      ls->m_Arrowmapper->SetInputData(arrowPolyData);
      if(this->m_RenderOrientationArrows)
      {
        ScalarType triangleSizeMM = 7.0 * displayGeometry->GetScaleFactorMMPerDisplayUnit();

        vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkPoints> triPoints = vtkSmartPointer<vtkPoints>::New();

        DrawOrientationArrow(triangles,triPoints,triangleSizeMM,orthogonalVector,point1,point2);
        DrawOrientationArrow(triangles,triPoints,triangleSizeMM,orthogonalVector,point2,point1);
        arrowPolyData->SetPoints(triPoints);
        arrowPolyData->SetPolys(triangles);
        ls->m_ArrowActor->SetVisibility(1);
      }

      // Visualize
      vtkSmartPointer<vtkPolyData> helperlinesPolyData = vtkSmartPointer<vtkPolyData>::New();
      ls->m_HelperLinesmapper->SetInputData(helperlinesPolyData);
      if ( showAreaOfThickSlicing )
      {
        vtkSmartPointer<vtkCellArray> helperlines = vtkSmartPointer<vtkCellArray>::New();
        // vectorToHelperLine defines how to reach the helperLine from the mainLine
        // got the right direction, so we multiply the width
        Vector3D vecToHelperLine = orthogonalVector * thickSliceDistance;

        this->DrawLine(point1 - vecToHelperLine, point2 - vecToHelperLine,helperlines,points);
        this->DrawLine(point1 + vecToHelperLine, point2 + vecToHelperLine,helperlines,points);

        // Add the points to the dataset
        helperlinesPolyData->SetPoints(points);

        // Add the lines to the dataset
        helperlinesPolyData->SetLines(helperlines);

        ls->m_CrosshairActor->GetProperty()->SetLineStipplePattern(0xf0f0);
        ls->m_CrosshairActor->GetProperty()->SetLineStippleRepeatFactor(1);
        ls->m_CrosshairHelperLineActor->SetVisibility(1);
      }
    }
  }
}

void mitk::PlaneGeometryDataMapper2D::DrawLine( mitk::Point3D p0,mitk::Point3D p1,
                                                vtkCellArray* lines,
                                                vtkPoints* points
                                                )
{
  vtkIdType pidStart = points->InsertNextPoint(p0[0],p0[1], p0[2]);
  vtkIdType pidEnd = points->InsertNextPoint(p1[0],p1[1], p1[2]);

  vtkSmartPointer<vtkLine> lineVtk =  vtkSmartPointer<vtkLine>::New();
  lineVtk->GetPointIds()->SetId(0,pidStart);
  lineVtk->GetPointIds()->SetId(1,pidEnd);

  lines->InsertNextCell(lineVtk);

}

void mitk::PlaneGeometryDataMapper2D::DrawOrientationArrow(vtkSmartPointer<vtkCellArray> triangles,
                                                           vtkSmartPointer<vtkPoints> triPoints,
                                                           double triangleSizeMM,
                                                           Vector3D& orthogonalVector,
                                                           Point3D& point1, Point3D& point2)
{
  // Draw arrows to indicate plane orientation
  // Vector along line
  Vector3D v1 = point2 - point1;
  v1.Normalize();
  v1 *= triangleSizeMM;

  // Orthogonal vector
  Vector3D v2 = orthogonalVector;
  v2 *= triangleSizeMM;
  if(!this->m_ArrowOrientationPositive) v2*=-1.0;

  // Initialize remaining triangle coordinates accordingly
  Point3D p1 = point1 + v1 * 2.0;
  Point3D p2 = point1 + v1 + v2;

  vtkIdType t0 = triPoints->InsertNextPoint(point1[0],point1[1], point1[2]); // start of the line
  vtkIdType t1 = triPoints->InsertNextPoint(p1[0],p1[1], p1[2]); // point on line
  vtkIdType t2 = triPoints->InsertNextPoint(p2[0],p2[1], p2[2]); // direction point

  vtkSmartPointer<vtkTriangle> triangle =
      vtkSmartPointer<vtkTriangle>::New();
  triangle->GetPointIds()->SetId ( 0, t0 );
  triangle->GetPointIds()->SetId ( 1, t1 );
  triangle->GetPointIds()->SetId ( 2, t2 );

  triangles->InsertNextCell(triangle);
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

void mitk::PlaneGeometryDataMapper2D::ApplyAllProperties( BaseRenderer *renderer )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ApplyColorAndOpacityProperties2D(renderer, ls->m_CrosshairActor);
  ApplyColorAndOpacityProperties2D(renderer, ls->m_CrosshairHelperLineActor);
  ApplyColorAndOpacityProperties2D(renderer, ls->m_ArrowActor);

  float thickness;
  this->GetDataNode()->GetFloatProperty("Line width",thickness,renderer);
  ls->m_CrosshairActor->GetProperty()->SetLineWidth(thickness);
  ls->m_CrosshairHelperLineActor->GetProperty()->SetLineWidth(thickness);

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

void mitk::PlaneGeometryDataMapper2D::ApplyColorAndOpacityProperties2D(BaseRenderer* renderer, vtkActor2D* actor)
{
  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  DataNode * node = GetDataNode();

  // check for color prop and use it for rendering if it exists
  node->GetColor(rgba, renderer, "color");
  // check for opacity prop and use it for rendering if it exists
  node->GetOpacity(rgba[3], renderer, "opacity");

  double drgba[4]={rgba[0],rgba[1],rgba[2],rgba[3]};
  actor->GetProperty()->SetColor(drgba);
  actor->GetProperty()->SetOpacity(drgba[3]);
}

void mitk::PlaneGeometryDataMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  mitk::IPropertyAliases* aliases = mitk::CoreServices::GetPropertyAliases();
  node->AddProperty( "Line width", mitk::FloatProperty::New(1), renderer, overwrite );
  aliases->AddAlias( "line width", "Crosshair.Line Width", "");
  node->AddProperty( "Crosshair.Gap Size", mitk::IntProperty::New(32), renderer, overwrite );
  node->AddProperty( "decoration", mitk::PlaneOrientationProperty
                     ::New(PlaneOrientationProperty::PLANE_DECORATION_NONE), renderer, overwrite );
  aliases->AddAlias( "decoration", "Crosshair.Orientation Decoration", "");

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

void mitk::PlaneGeometryDataMapper2D::UpdateVtkTransform(mitk::BaseRenderer* /*renderer*/)
{
}

mitk::PlaneGeometryDataMapper2D::LocalStorage::LocalStorage()
{
  m_CrosshairAssembly = vtkSmartPointer <vtkPropAssembly>::New();

  m_CrosshairActor = vtkSmartPointer <vtkActor2D>::New();
  m_ArrowActor = vtkSmartPointer <vtkActor2D>::New();
  m_CrosshairHelperLineActor = vtkSmartPointer <vtkActor2D>::New();

  m_HelperLinesmapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_Arrowmapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();

  m_CrosshairActor->SetMapper(m_Mapper);
  m_ArrowActor->SetMapper(m_Arrowmapper);
  m_CrosshairHelperLineActor->SetMapper(m_HelperLinesmapper);

  m_CrosshairActor->SetVisibility(0);
  m_ArrowActor->SetVisibility(0);
  m_CrosshairHelperLineActor->SetVisibility(0);

  m_CrosshairAssembly->AddPart(m_CrosshairActor);
  m_CrosshairAssembly->AddPart(m_ArrowActor);
  m_CrosshairAssembly->AddPart(m_CrosshairHelperLineActor);

  vtkCoordinate *tcoord = vtkCoordinate::New();
  tcoord->SetCoordinateSystemToWorld();
  m_HelperLinesmapper->SetTransformCoordinate(tcoord);
  m_Mapper->SetTransformCoordinate(tcoord);
//  tcoord->SetCoordinateSystemToNormalizedDisplay();
  m_Arrowmapper->SetTransformCoordinate(tcoord);
  tcoord->Delete();
}

mitk::PlaneGeometryDataMapper2D::LocalStorage::~LocalStorage()
{
}
