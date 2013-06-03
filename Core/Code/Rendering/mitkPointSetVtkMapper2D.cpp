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


#include "mitkPointSetVtkMapper2D.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkEnumerationProperty.h"
#include "mitkVtkPropRenderer.h"
#include "mitkPointSet.h"

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkPropAssembly.h>
#include <vtkTubeFilter.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkConeSource.h>
#include <vtkDiskSource.h>
#include <vtkCylinderSource.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkVectorText.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkTransform.h>
#include <vtkGlyph3D.h>
#include <vtkTransformFilter.h>
#include <vtkLinearTransform.h>
#include <vtkLine.h>
#include <vtkSource.h>
#include <vtkGlyphSource2D.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyLine.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <stdlib.h>


mitk::PointSetVtkMapper2D::LocalStorage::LocalStorage()
{

  // points
  m_UnselectedPoints = vtkSmartPointer<vtkPoints>::New();
  m_SelectedPoints = vtkSmartPointer<vtkPoints>::New();
  m_ContourPoints = vtkSmartPointer<vtkPoints>::New();

  // scales
  m_UnselectedScales = vtkSmartPointer<vtkFloatArray>::New();
  m_SelectedScales = vtkSmartPointer<vtkFloatArray>::New();

  // distances
  m_DistancesBetweenPoints = vtkSmartPointer<vtkFloatArray>::New();

  // lines
  m_ContourLines = vtkSmartPointer<vtkCellArray>::New();

  // glyph source (provides the different shapes)
  m_UnselectedGlyphSource2D = vtkSmartPointer<vtkGlyphSource2D>::New();
  m_SelectedGlyphSource2D = vtkSmartPointer<vtkGlyphSource2D>::New();

  // glyphs
  m_UnselectedGlyph3D = vtkSmartPointer<vtkGlyph3D>::New();
  m_SelectedGlyph3D = vtkSmartPointer<vtkGlyph3D>::New();

   // polydata
  m_VtkUnselectedPointListPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_VtkSelectedPointListPolyData = vtkSmartPointer <vtkPolyData>::New();
  m_VtkContourPolyData = vtkSmartPointer<vtkPolyData>::New();

  // actors
  m_UnselectedActor = vtkSmartPointer <vtkActor>::New();
  m_SelectedActor = vtkSmartPointer <vtkActor>::New();
  m_ContourActor = vtkSmartPointer <vtkActor>::New();

  // mappers
  m_VtkUnselectedPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_VtkSelectedPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_VtkContourPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

  // propassembly
  m_PropAssembly = vtkSmartPointer <vtkPropAssembly>::New();

}

mitk::PointSetVtkMapper2D::LocalStorage::~LocalStorage()
{
}

// input for this mapper ( = point set)
const mitk::PointSet* mitk::PointSetVtkMapper2D::GetInput()
{
  return static_cast<const mitk::PointSet * > ( GetDataNode()->GetData() );
}

// constructor
mitk::PointSetVtkMapper2D::PointSetVtkMapper2D()
: m_Polygon(false),
m_PolygonClosed(false),
m_ShowPoints(true),
m_ShowDistances(false),
m_DistancesDecimalDigits(1),
m_ShowAngles(false),
m_ShowDistantLines(false),
m_LineWidth(1),
m_PointLineWidth(1),
m_Point2DSize(6),
m_IdGlyph(3), // default: cross
m_FillGlyphs(false)
{
}

// destructor
mitk::PointSetVtkMapper2D::~PointSetVtkMapper2D()
{
}

// toggles visiblity of the prop assembly
void mitk::PointSetVtkMapper2D::ResetMapper( BaseRenderer* renderer )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ls->m_PropAssembly->VisibilityOff();
}

// returns propassembly
vtkProp* mitk::PointSetVtkMapper2D::GetVtkProp(mitk::BaseRenderer * renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_PropAssembly;
}

static bool makePerpendicularVector2D(const mitk::Vector2D& in, mitk::Vector2D& out)
{
  if((fabs(in[0])>0) && ( (fabs(in[0])>fabs(in[1])) || (in[1] == 0) ) )
  {
    out[0]=-in[1]/in[0];
    out[1]=1;
    out.Normalize();
    return true;
  }
  else
    if(fabs(in[1])>0)
    {
      out[0]=1;
      out[1]=-in[0]/in[1];
      out.Normalize();
      return true;
    }
    else
      return false;
}


void mitk::PointSetVtkMapper2D::CreateVTKRenderObjects(mitk::BaseRenderer* renderer)
{

  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  if(ls->m_PropAssembly->GetParts()->IsItemPresent(ls->m_UnselectedActor))
    ls->m_PropAssembly->RemovePart(ls->m_UnselectedActor);
  if(ls->m_PropAssembly->GetParts()->IsItemPresent(ls->m_SelectedActor))
    ls->m_PropAssembly->RemovePart(ls->m_SelectedActor);
  if(ls->m_PropAssembly->GetParts()->IsItemPresent(ls->m_ContourActor))
    ls->m_PropAssembly->RemovePart(ls->m_ContourActor);
  if(ls->m_PropAssembly->GetParts()->IsItemPresent(ls->m_VtkTextActor))
    ls->m_PropAssembly->RemovePart(ls->m_VtkTextActor);

  unsigned i = 0;
  for(i=0; i< ls->m_VtkTextLabelActors.size(); i++)
  {
    if(ls->m_PropAssembly->GetParts()->IsItemPresent(ls->m_VtkTextLabelActors.at(i)))
      ls->m_PropAssembly->RemovePart(ls->m_VtkTextLabelActors.at(i));
  }

  for(i=0; i< ls->m_VtkTextDistanceActors.size(); i++)
  {
    if(ls->m_PropAssembly->GetParts()->IsItemPresent(ls->m_VtkTextDistanceActors.at(i)))
      ls->m_PropAssembly->RemovePart(ls->m_VtkTextDistanceActors.at(i));
  }

  for(i=0; i< ls->m_VtkTextAngleActors.size(); i++)
  {
    if(ls->m_PropAssembly->GetParts()->IsItemPresent(ls->m_VtkTextAngleActors.at(i)))
      ls->m_PropAssembly->RemovePart(ls->m_VtkTextAngleActors.at(i));
  }

   // polydata
  ls->m_VtkUnselectedPointListPolyData = vtkSmartPointer<vtkPolyData>::New();
  ls->m_VtkSelectedPointListPolyData = vtkSmartPointer <vtkPolyData>::New();
  ls->m_VtkContourPolyData = vtkSmartPointer<vtkPolyData>::New();


  // exceptional displaying for PositionTracker -> MouseOrientationTool
  int mapperID;
  bool isInputDevice=false;
  if( this->GetDataNode()->GetBoolProperty("inputdevice",isInputDevice) && isInputDevice )
  {
    if( this->GetDataNode()->GetIntProperty("BaseRendererMapperID",mapperID) && mapperID == 2)
      return; //The event for the PositionTracker came from the 3d widget and  not needs to be displayed
  }

  // get input point set and update the PointSet
  mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());

  /* only update the input data, if the property tells us to */
  bool update = true;
  this->GetDataNode()->GetBoolProperty("updateDataOnRender", update);
  if (update == true)
    input->Update();

  //TODO: insert last changes on timestamps
  int timestep = this->GetTimestep();

  mitk::PointSet::DataType::Pointer itkPointSet = input->GetPointSet( timestep );

  if ( itkPointSet.GetPointer() == NULL)
  {
    ls->m_PropAssembly->VisibilityOff();
    return;
  }

  //iterator for point set
  mitk::PointSet::PointsContainer::Iterator pointsIter;
  mitk::PointSet::PointsContainer::Iterator pointsIterPredecessor;
  pointsIterPredecessor = itkPointSet->GetPoints()->Begin();


  // PointDataContainer has additional information to each point, e.g. whether
  // it is selected or not
  mitk::PointSet::PointDataContainer::Iterator pointDataIter;
  pointDataIter = itkPointSet->GetPointData()->Begin();


  //check if the list for the PointDataContainer is the same size as the PointsContainer.
  //If not, then the points were inserted manually and can not be visualized according to the PointData (selected/unselected)
  bool pointDataBroken = (itkPointSet->GetPointData()->Size() != itkPointSet->GetPoints()->Size());

  if(itkPointSet->GetPointData()->size() == 0 || pointDataBroken)
  {
    return;
  }

  // empty point sets, cellarrays, scalars
  ls->m_UnselectedPoints->Reset();
  ls->m_SelectedPoints->Reset();

  ls->m_ContourPoints->Reset();
  ls->m_ContourLines->Reset();

  ls->m_UnselectedScales->Reset();
  ls->m_SelectedScales->Reset();

  ls->m_DistancesBetweenPoints->Reset();

  ls->m_VtkTextLabelActors.clear();
  ls->m_VtkTextDistanceActors.clear();
  ls->m_VtkTextAngleActors.clear();

  ls->m_UnselectedScales->SetNumberOfComponents(3);
  ls->m_SelectedScales->SetNumberOfComponents(3);

  int NumberContourPoints = 0;
  bool pointsOnSameSideOfPlane = false;

  const int text2dDistance = 10;

  // current point in point set
  itk::Point<float> point;

  Point3D p;                      // currently visited point
  Point3D lastP;                  // last visited point
  Vector3D vec;                   // p - lastP
  Vector3D lastVec;               // lastP - point before lastP
  vec.Fill(0);

  mitk::Point3D projected_p;      // p projected on viewplane

  Point2D pt2d;       // projected_p in display coordinates
  Point2D lastPt2d;   // last projected_p in display coordinates
  Point2D preLastPt2d;// projected_p in display coordinates before lastPt2

  // get display geometry
  mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();

  // get plane geometry
  mitk::PlaneGeometry::ConstPointer planeGeometry = renderer->GetSliceNavigationController()->GetCurrentPlaneGeometry();

  int count = 0;

  for (pointsIter=itkPointSet->GetPoints()->Begin();
    pointsIter!=itkPointSet->GetPoints()->End();
    pointsIter++)
  {

    lastP = p; // valid for number of points count > 0
    preLastPt2d = lastPt2d; // valid only for count > 1
    lastPt2d = pt2d;  // valid for number of points count > 0

    lastVec = vec;    // valid only for counter > 1

    // get current point in point set
    point = pointsIter->Value();

    p[0] = point[0];
    p[1] = point[1];
    p[2] = point[2];

    displayGeometry->Project(p, projected_p);
    displayGeometry->Map(projected_p, pt2d);
    displayGeometry->WorldToDisplay(pt2d, pt2d);

    vec = p-lastP;    // valid only for counter > 0

    // compute distance to current plane
    float diff = planeGeometry->DistanceFromPlane(point);
    diff = diff * diff;

    //MouseOrientation
    bool isInputDevice=false;
    this->GetDataNode()->GetBoolProperty("inputdevice",isInputDevice);

    // if point is close to current plane ( distance < 4) it will be displayed
    if(!isInputDevice && (diff < 4.0))
    {

      // is point selected or not?
      if (pointDataIter->Value().selected)
      {
        ls->m_SelectedPoints->InsertNextPoint(point[0],point[1],point[2]);
        // point is scaled according to its distance to the plane
        ls->m_SelectedScales->InsertNextTuple3(m_Point2DSize - (2*diff),0,0);
      }
      else
      {
        ls->m_UnselectedPoints->InsertNextPoint(point[0],point[1],point[2]);
        // point is scaled according to its distance to the plane
        ls->m_UnselectedScales->InsertNextTuple3(m_Point2DSize - (2*diff),0,0);
      }


      //---- LABEL -----//

      //  paint label for each point if available
      if (dynamic_cast<mitk::StringProperty *>(this->GetDataNode()->GetProperty("label")) != NULL)
      {
        const char * pointLabel = dynamic_cast<mitk::StringProperty *>(
          this->GetDataNode()->GetProperty("label"))->GetValue();
        std::string l = pointLabel;
        if (input->GetSize()>1)
        {
          char buffer[20];
          sprintf(buffer,"%u",pointsIter->Index());
          std::stringstream ss;
          ss << pointsIter->Index();
          l.append(ss.str());
        }

        ls->m_VtkTextActor = vtkSmartPointer<vtkTextActor>::New();

        ls->m_VtkTextActor->SetPosition(pt2d[0] + text2dDistance, pt2d[1] + text2dDistance);
        ls->m_VtkTextActor->SetInput(l.c_str());
        ls->m_VtkTextActor->GetTextProperty()->SetOpacity( 100 );

        float unselectedColor[4];

        //check if there is a color property
        GetDataNode()->GetColor(unselectedColor);

        if (unselectedColor != NULL)
          ls->m_VtkTextActor->GetTextProperty()->SetColor(unselectedColor[0], unselectedColor[1], unselectedColor[2]);
        else
          ls->m_VtkTextActor->GetTextProperty()->SetColor(0.0f, 1.0f, 0.0f);

        ls->m_VtkTextLabelActors.push_back(ls->m_VtkTextActor);

      }
    }

    // draw contour, distance text and angle text in render window

    // lines between points, which intersect the current plane, are drawn
    if( m_Polygon && count > 0 )
    {
      ScalarType distance =    displayGeometry->GetWorldGeometry()->SignedDistance(point);
      ScalarType lastDistance =    displayGeometry->GetWorldGeometry()->SignedDistance(lastP);

      pointsOnSameSideOfPlane = (distance * lastDistance) > 0.5;

      // Points must be on different side of plane in order to draw a contour.
      // If "show distant lines" is enabled this condition is disregarded.
      if ( !pointsOnSameSideOfPlane || m_ShowDistantLines)
      {
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();

        ls->m_ContourPoints->InsertNextPoint(lastP[0],lastP[1],lastP[2]);
        line->GetPointIds()->SetId(0, NumberContourPoints);
        NumberContourPoints++;

        ls->m_ContourPoints->InsertNextPoint(point[0], point[1], point[2]);
        line->GetPointIds()->SetId(1, NumberContourPoints);
        NumberContourPoints++;

        ls->m_ContourLines->InsertNextCell(line);


        if(m_ShowDistances) // calculate and print distance between adjacent points
        {
          float distancePoints = point.EuclideanDistanceTo(lastP);

          std::stringstream buffer;
          buffer<<std::fixed <<std::setprecision(m_DistancesDecimalDigits)<<distancePoints<<" mm";

          // compute desired display position of text
          Vector2D vec2d = pt2d-lastPt2d;
          makePerpendicularVector2D(vec2d, vec2d); // text is rendered within text2dDistance perpendicular to current line
          Vector2D pos2d = (lastPt2d.GetVectorFromOrigin() + pt2d ) * 0.5 + vec2d * text2dDistance;

          ls->m_VtkTextActor = vtkSmartPointer<vtkTextActor>::New();

          ls->m_VtkTextActor->SetPosition(pos2d[0],pos2d[1]);
          ls->m_VtkTextActor->SetInput(buffer.str().c_str());
          ls->m_VtkTextActor->GetTextProperty()->SetColor(0.0, 1.0, 0.0);

          ls->m_VtkTextDistanceActors.push_back(ls->m_VtkTextActor);

        }

        if(m_ShowAngles && count > 1) // calculate and print angle between connected lines
        {
          std::stringstream buffer;
          //(char) 176 is the degree sign
          buffer << angle(vec.GetVnlVector(), -lastVec.GetVnlVector())*180/vnl_math::pi << (char)176;

          //compute desired display position of text
          Vector2D vec2d = pt2d-lastPt2d;             // first arm enclosing the angle
          vec2d.Normalize();
          Vector2D lastVec2d = lastPt2d-preLastPt2d;  // second arm enclosing the angle
          lastVec2d.Normalize();
          vec2d=vec2d-lastVec2d;                      // vector connecting both arms
          vec2d.Normalize();

          // middle between two vectors that enclose the angle
          Vector2D pos2d = lastPt2d.GetVectorFromOrigin() + vec2d * text2dDistance * text2dDistance;

          ls->m_VtkTextActor = vtkSmartPointer<vtkTextActor>::New();

          ls->m_VtkTextActor->SetPosition(pos2d[0],pos2d[1]);
          ls->m_VtkTextActor->SetInput(buffer.str().c_str());
          ls->m_VtkTextActor->GetTextProperty()->SetColor(0.0, 1.0, 0.0);

          ls->m_VtkTextAngleActors.push_back(ls->m_VtkTextActor);
        }
      }
    }

    if(pointDataIter != itkPointSet->GetPointData()->End())
    {
      pointDataIter++;
      count++;
    }
  }

  // add each single text actor to the assembly
  for(i=0; i< ls->m_VtkTextLabelActors.size(); i++)
  {
    ls->m_PropAssembly->AddPart(ls->m_VtkTextLabelActors.at(i));
  }

  for(i=0; i< ls->m_VtkTextDistanceActors.size(); i++)
  {
    ls->m_PropAssembly->AddPart(ls->m_VtkTextDistanceActors.at(i));
  }

  for(i=0; i< ls->m_VtkTextAngleActors.size(); i++)
  {
    ls->m_PropAssembly->AddPart(ls->m_VtkTextAngleActors.at(i));
  }

  //---- CONTOUR -----//


  //create lines between the points which intersect the plane
  if (m_Polygon)
  {
    // draw line between first and last point which is rendered
    if(m_PolygonClosed && NumberContourPoints > 1){

      vtkSmartPointer<vtkLine> closingLine = vtkSmartPointer<vtkLine>::New();
      closingLine->GetPointIds()->SetId(0, 0); // index of first point
      closingLine->GetPointIds()->SetId(1, NumberContourPoints-1); // index of last point
      ls->m_ContourLines->InsertNextCell(closingLine);
    }

    ls->m_VtkContourPolyData->SetPoints(ls->m_ContourPoints);
    ls->m_VtkContourPolyData->SetLines(ls->m_ContourLines);

    ls->m_VtkContourPolyDataMapper->SetInput(ls->m_VtkContourPolyData);
    ls->m_ContourActor->SetMapper(ls->m_VtkContourPolyDataMapper);
    ls->m_ContourActor->GetProperty()->SetLineWidth(m_LineWidth);

    ls->m_PropAssembly->AddPart(ls->m_ContourActor);

  }

  // the point set must be transformed in order to obtain the appropriate glyph orientation
  // according to the current view
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkMatrix4x4> a,b = vtkSmartPointer<vtkMatrix4x4>::New();

  a = planeGeometry->GetVtkTransform()->GetMatrix();
  b->DeepCopy( a );

  // delete transformation from matrix, only take orientation
  b->SetElement(3,3,1);
  b->SetElement(2,3,0);
  b->SetElement(1,3,0);
  b->SetElement(0,3,0);
  b->SetElement(3,2,0);
  b->SetElement(3,1,0);
  b->SetElement(3,0,0);

  transform->SetMatrix(  b );

  //---- UNSELECTED POINTS  -----//

  // apply properties to glyph
  ls->m_UnselectedGlyphSource2D->SetGlyphType(m_IdGlyph);

  if(m_FillGlyphs)
    ls->m_UnselectedGlyphSource2D->FilledOn();
  else
    ls->m_UnselectedGlyphSource2D->FilledOff();

  // apply transform
  vtkSmartPointer<vtkTransformFilter> transformFilterU = vtkSmartPointer<vtkTransformFilter>::New();
  transformFilterU->SetInputConnection(ls->m_UnselectedGlyphSource2D->GetOutputPort());
  transformFilterU->SetTransform(transform);

  ls->m_VtkUnselectedPointListPolyData->SetPoints(ls->m_UnselectedPoints);
  ls->m_VtkUnselectedPointListPolyData->GetPointData()->SetVectors(ls->m_UnselectedScales);

  // apply transform of current plane to glyphs
  ls->m_UnselectedGlyph3D->SetSourceConnection(transformFilterU->GetOutputPort());
  ls->m_UnselectedGlyph3D->SetInput(ls->m_VtkUnselectedPointListPolyData);
  ls->m_UnselectedGlyph3D->SetScaleModeToScaleByVector();
  ls->m_UnselectedGlyph3D->SetVectorModeToUseVector();

  ls->m_VtkUnselectedPolyDataMapper->SetInput(ls->m_UnselectedGlyph3D->GetOutput());
  ls->m_UnselectedActor->SetMapper(ls->m_VtkUnselectedPolyDataMapper);
  ls->m_UnselectedActor->GetProperty()->SetLineWidth(m_PointLineWidth);

  ls->m_PropAssembly->AddPart(ls->m_UnselectedActor);


  //---- SELECTED POINTS  -----//

  ls->m_SelectedGlyphSource2D->SetGlyphTypeToDiamond();
  ls->m_SelectedGlyphSource2D->CrossOn();
  ls->m_SelectedGlyphSource2D->FilledOff();

  // apply transform
  vtkSmartPointer<vtkTransformFilter> transformFilterS = vtkSmartPointer<vtkTransformFilter>::New();
  transformFilterS->SetInputConnection(ls->m_SelectedGlyphSource2D->GetOutputPort());
  transformFilterS->SetTransform(transform);

  ls->m_VtkSelectedPointListPolyData->SetPoints(ls->m_SelectedPoints);
  ls->m_VtkSelectedPointListPolyData->GetPointData()->SetVectors(ls->m_SelectedScales);


  // apply transform of current plane to glyphs
  ls->m_SelectedGlyph3D->SetSourceConnection(transformFilterS->GetOutputPort());
  ls->m_SelectedGlyph3D->SetInput(ls->m_VtkSelectedPointListPolyData);
  ls->m_SelectedGlyph3D->SetScaleModeToScaleByVector();
  ls->m_SelectedGlyph3D->SetVectorModeToUseVector();

  ls->m_VtkSelectedPolyDataMapper->SetInput(ls->m_SelectedGlyph3D->GetOutput());
  ls->m_SelectedActor->SetMapper(ls->m_VtkSelectedPolyDataMapper);
  ls->m_SelectedActor->GetProperty()->SetLineWidth(m_PointLineWidth);

  ls->m_PropAssembly->AddPart(ls->m_SelectedActor);

}


void mitk::PointSetVtkMapper2D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{

  const mitk::DataNode* node = GetDataNode();
  if( node == NULL )
    return;

  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  bool needGenerateData = ls->IsGenerateDataRequired( renderer, this, GetDataNode() );

  // toggle visibility
  bool visible = true;
  node->GetVisibility(visible, renderer, "visible");
  if(!visible)
  {
    ls->m_UnselectedActor->VisibilityOff();
    ls->m_SelectedActor->VisibilityOff();
    ls->m_ContourActor->VisibilityOff();
    ls->m_PropAssembly->VisibilityOff();
    return;
  }else{
    ls->m_PropAssembly->VisibilityOn();
  }

  node->GetBoolProperty("show contour",       m_Polygon, renderer);
  node->GetBoolProperty("close contour",      m_PolygonClosed, renderer);
  node->GetBoolProperty("show points",        m_ShowPoints, renderer);
  node->GetBoolProperty("show distances",     m_ShowDistances, renderer);
  node->GetIntProperty("distance decimal digits",     m_DistancesDecimalDigits, renderer);
  node->GetBoolProperty("show angles",        m_ShowAngles, renderer);
  node->GetBoolProperty("show distant lines", m_ShowDistantLines, renderer);
  node->GetIntProperty("line width",          m_LineWidth, renderer);
  node->GetIntProperty("point line width",    m_PointLineWidth, renderer);
  node->GetIntProperty("point 2D size",       m_Point2DSize, renderer);
  mitk::EnumerationProperty* eP = dynamic_cast<mitk::EnumerationProperty*> (node->GetProperty("Pointset.2D.shape", renderer));
  m_IdGlyph = eP->GetValueAsId();
  node->GetBoolProperty("Pointset.2D.fill shape", m_FillGlyphs, renderer);


  //check for color props and use it for rendering of selected/unselected points and contour
  //due to different params in VTK (double/float) we have to convert


  float unselectedColor[4];
  vtkFloatingPointType selectedColor[4]={1.0f,0.0f,0.0f,1.0f};    //red
  vtkFloatingPointType contourColor[4]={1.0f,0.0f,0.0f,1.0f};     //red

  //different types for color
  mitk::Color tmpColor;
  float opacity = 1.0;

  GetDataNode()->GetOpacity(opacity, renderer);

  // apply color and opacity
  if(m_ShowPoints)
  {
    ls->m_UnselectedActor->VisibilityOn();
    ls->m_SelectedActor->VisibilityOn();

    //check if there is a color property
    GetDataNode()->GetColor(unselectedColor);

    //get selected color property
    if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("selectedcolor")) != NULL)
    {
      tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("selectedcolor"))->GetValue();

    }
    else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("selectedcolor")) != NULL)
    {
      tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("selectedcolor"))->GetValue();
    }

    selectedColor[0] = tmpColor[0];
    selectedColor[1] = tmpColor[1];
    selectedColor[2] = tmpColor[2];
    selectedColor[3] = 1.0f; // alpha value

    ls->m_SelectedActor->GetProperty()->SetColor(selectedColor);
    ls->m_SelectedActor->GetProperty()->SetOpacity(opacity);

    ls->m_UnselectedActor->GetProperty()->SetColor(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
    ls->m_UnselectedActor->GetProperty()->SetOpacity(opacity);

  }
  else
  {
    ls->m_UnselectedActor->VisibilityOff();
    ls-> m_SelectedActor->VisibilityOff();
  }


  if (m_Polygon)
  {
    ls->m_ContourActor->VisibilityOn();

    //get contour color property
    if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("contourcolor")) != NULL)
    {
      tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("contourcolor"))->GetValue();
    }
    else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("contourcolor")) != NULL)
    {
      tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("contourcolor"))->GetValue();
    }

    contourColor[0] = tmpColor[0];
    contourColor[1] = tmpColor[1];
    contourColor[2] = tmpColor[2];
    contourColor[3] = 1.0f;

    ls->m_ContourActor->GetProperty()->SetColor(contourColor);
    ls->m_ContourActor->GetProperty()->SetOpacity(opacity);
  }
  else
  {
    ls->m_ContourActor->VisibilityOff();
  }

  if(needGenerateData)
  {
  // create new vtk render objects (e.g. a circle for a point)
  this->CreateVTKRenderObjects(renderer);
  }

}



void mitk::PointSetVtkMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "line width", mitk::IntProperty::New(2), renderer, overwrite );
  node->AddProperty( "point line width", mitk::IntProperty::New(1), renderer, overwrite );
  node->AddProperty( "point 2D size", mitk::IntProperty::New(6), renderer, overwrite );
  node->AddProperty( "show contour", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "close contour", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "show points", mitk::BoolProperty::New(true), renderer, overwrite );
  node->AddProperty( "show distances", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "distance decimal digits", mitk::IntProperty::New(2), renderer, overwrite );
  node->AddProperty( "show angles", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "show distant lines", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "layer", mitk::IntProperty::New(1), renderer, overwrite );

  mitk::EnumerationProperty::Pointer glyphType = mitk::EnumerationProperty::New();
  glyphType->AddEnum("None", 0);
  glyphType->AddEnum("Vertex", 1);
  glyphType->AddEnum("Dash", 2);
  glyphType->AddEnum("Cross", 3);
  glyphType->AddEnum("ThickCross", 4);
  glyphType->AddEnum("Triangle", 5);
  glyphType->AddEnum("Square", 6);
  glyphType->AddEnum("Circle", 7);
  glyphType->AddEnum("Diamond", 8);
  glyphType->AddEnum("Arrow", 9);
  glyphType->AddEnum("ThickArrow", 10);
  glyphType->AddEnum("HookedArrow", 11);
  glyphType->SetValue("Cross");
  node->AddProperty( "Pointset.2D.shape", glyphType, renderer, overwrite);

  node->AddProperty("Pointset.2D.fill shape", mitk::BoolProperty::New(false), renderer, overwrite); // fill or do not fill the glyph shape

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
