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


#include "mitkPointSetVtkMapper3D.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
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
#include <vtkCylinderSource.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkCellArray.h>
#include <vtkVectorText.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkPolyDataAlgorithm.h>
#include <math.h> 
#include <stdlib.h>
#include <vtkMatrix4x4.h>

#include <vtkgl.h>

#include <mitkPropertyObserver.h>

const mitk::PointSet* mitk::PointSetVtkMapper3D::GetInput()
{
  return static_cast<const mitk::PointSet * > ( GetDataNode()->GetData() );
}

mitk::PointSetVtkMapper3D::PointSetVtkMapper3D()
: m_vtkSelectedPointList(NULL),
 m_vtkUnselectedPointList(NULL),
 m_VtkSelectedPolyDataMapper(NULL),
 m_VtkUnselectedPolyDataMapper(NULL),
 m_vtkTextList(NULL),
 m_NumberOfSelectedAdded(0),
 m_NumberOfUnselectedAdded(0),
 m_PointSize(1.0),
 m_ContourRadius(0.5),
 m_VertexRendering(false)
{
  //propassembly
  m_PointsAssembly = vtkSmartPointer<vtkPropAssembly>::New();

  //creating actors to be able to set transform
  m_SelectedActor = vtkSmartPointer<vtkActor>::New();
  m_UnselectedActor = vtkSmartPointer<vtkActor>::New();
  m_ContourActor = vtkSmartPointer<vtkActor>::New();
}

mitk::PointSetVtkMapper3D::~PointSetVtkMapper3D()
{
}

void mitk::PointSetVtkMapper3D::ReleaseGraphicsResources(vtkWindow *renWin)
{
  m_PointsAssembly->ReleaseGraphicsResources(renWin);

  m_SelectedActor->ReleaseGraphicsResources(renWin);
  m_UnselectedActor->ReleaseGraphicsResources(renWin);
  m_ContourActor->ReleaseGraphicsResources(renWin);
}

void mitk::PointSetVtkMapper3D::ReleaseGraphicsResources(mitk::BaseRenderer* renderer)
{
  m_PointsAssembly->ReleaseGraphicsResources(renderer->GetRenderWindow());

  m_SelectedActor->ReleaseGraphicsResources(renderer->GetRenderWindow());
  m_UnselectedActor->ReleaseGraphicsResources(renderer->GetRenderWindow());
  m_ContourActor->ReleaseGraphicsResources(renderer->GetRenderWindow());

}

void mitk::PointSetVtkMapper3D::CreateEdgeObjectsBetweenPoints(itk::Point<float> point1, itk::Point<float> point2) {

  float h = point1.EuclideanDistanceTo(point2);
  double px = point2[0] - point1[0];
  double py = point2[1] - point1[1];
  double pz = point2[2] - point1[2];

  double hXY = sqrt(px*px + py*py);
  double thetta = asin(px/hXY)*180/3.14;
  if (py > 0) {
    thetta = -thetta;
  }

  double phi = 90 - (acos(pz/h))*180/3.14;
  if (py < 0) {
    phi = -phi;
  }

  vtkSmartPointer<vtkTransform> aTransform = vtkSmartPointer<vtkTransform>::New();
  aTransform->Identity();

  aTransform->Translate(point1[0] + (point2[0] - point1[0])/2, point1[1] + (point2[1] - point1[1])/2, point1[2] + (point2[2] - point1[2])/2);
  aTransform->RotateZ(thetta);
  aTransform->RotateX(phi);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetTransform(aTransform);

  vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
  cylinder->SetRadius(m_ContourRadius);
  cylinder->SetCenter(0, 0, 0);
          
  cylinder->SetHeight(h);
  cylinder->SetResolution(20);
  transformFilter->SetInputConnection(cylinder->GetOutputPort());

  m_vtkUnselectedPointList->AddInputConnection(transformFilter->GetOutputPort());
  ++m_NumberOfSelectedAdded;
}

void mitk::PointSetVtkMapper3D::CreateVTKRenderObjects()
{
  m_vtkSelectedPointList = vtkSmartPointer<vtkAppendPolyData>::New();
  m_vtkUnselectedPointList = vtkSmartPointer<vtkAppendPolyData>::New();

  m_PointsAssembly->VisibilityOn();

  if(m_PointsAssembly->GetParts()->IsItemPresent(m_SelectedActor))
    m_PointsAssembly->RemovePart(m_SelectedActor);
  if(m_PointsAssembly->GetParts()->IsItemPresent(m_UnselectedActor))
    m_PointsAssembly->RemovePart(m_UnselectedActor);
  if(m_PointsAssembly->GetParts()->IsItemPresent(m_ContourActor))
    m_PointsAssembly->RemovePart(m_ContourActor);

  // exceptional displaying for PositionTracker -> MouseOrientationTool
  int mapperID;
  bool isInputDevice=false;
  if( this->GetDataNode()->GetBoolProperty("inputdevice",isInputDevice) && isInputDevice )
  {
    if (this->GetDataNode()->GetIntProperty("BaseRendererMapperID", mapperID) && mapperID == BaseRenderer::Standard3D)
      return; //The event for the PositionTracker came from the 3d widget and  not needs to be displayed
  }

  // get and update the PointSet
  mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());

  /* only update the input data, if the property tells us to */
  bool update = true;
  this->GetDataNode()->GetBoolProperty("updateDataOnRender", update);
  if (update == true)
    input->Update();

  int timestep = this->GetTimestep();

  mitk::PointSet::DataType::Pointer itkPointSet = input->GetPointSet( timestep );

  if ( itkPointSet.GetPointer() == NULL)
  {
    m_PointsAssembly->VisibilityOff();
    return;
  }

  //now fill selected and unselected pointList
  //get size of Points in Property
  m_PointSize = 2;
  mitk::FloatProperty::Pointer pointSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("pointsize"));
  if ( pointSizeProp.IsNotNull() )
    m_PointSize = pointSizeProp->GetValue();

  m_ContourRadius = 2;
  mitk::FloatProperty::Pointer contourSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("contoursize"));
  if (contourSizeProp.IsNotNull())
    m_ContourRadius = contourSizeProp->GetValue();

  //get the property for creating a label onto every point only once
  bool showLabel = true;
  this->GetDataNode()->GetBoolProperty("show label", showLabel);
  const char * pointLabel=NULL;
  if(showLabel)
  {
    if(dynamic_cast<mitk::StringProperty *>(this->GetDataNode()->GetPropertyList()->GetProperty("label")) != NULL)
      pointLabel =dynamic_cast<mitk::StringProperty *>(this->GetDataNode()->GetPropertyList()->GetProperty("label"))->GetValue();
    else
      showLabel = false;
  }

  // whether or not to creat a "contour" - connecting lines between all the points
  int nbPoints = itkPointSet->GetPointData()->Size();
  bool makeContour = false;
  this->GetDataNode()->GetBoolProperty("show contour", makeContour);

  bool closeContour = false;
  this->GetDataNode()->GetBoolProperty("close contour", closeContour);
  int contourPointLimit = 0; // NO contour
  if ( makeContour )
  {
    if ( closeContour )
      contourPointLimit = nbPoints;
    else
      contourPointLimit = nbPoints - 1;
  }

  // build list of all positions for later transform in one go
  mitk::PointSet::PointsContainer::Iterator pointsIter;
  int ptIdx;

  m_NumberOfSelectedAdded = 0;
  m_NumberOfUnselectedAdded = 0;
  vtkSmartPointer<vtkPoints> localPoints = vtkSmartPointer<vtkPoints>::New();
  m_WorldPositions = vtkSmartPointer<vtkPoints>::New();
  m_PointConnections = vtkSmartPointer<vtkCellArray>::New(); // m_PointConnections between points
  for ( ptIdx = 0, pointsIter = itkPointSet->GetPoints()->Begin();
  pointsIter != itkPointSet->GetPoints()->End();
  pointsIter++, ptIdx++ )
  {
    itk::Point<float> currentPoint = pointsIter->Value();
    localPoints->InsertPoint(ptIdx, currentPoint[0], currentPoint[1], currentPoint[2]);

    if ( makeContour && ptIdx < contourPointLimit )
    {
      vtkIdType cell[2] = { (ptIdx + 1) % nbPoints, ptIdx };
      m_PointConnections->InsertNextCell(2, cell);
    }
  }

  vtkSmartPointer<vtkLinearTransform> vtktransform = this->GetDataNode()->GetVtkTransform(this->GetTimestep());
  vtktransform->TransformPoints(localPoints, m_WorldPositions);

  // create contour
  if (makeContour)
  {
    this->CreateContour(m_WorldPositions, m_PointConnections);
  }

  //check if the list for the PointDataContainer is the same size as the PointsContainer. Is not, then the points were inserted manually and can not be visualized according to the PointData (selected/unselected)
  bool pointDataBroken = (itkPointSet->GetPointData()->Size() != itkPointSet->GetPoints()->Size());

  itk::Point<float> lastPoint;
  
  //now add an object for each point in data
  mitk::PointSet::PointDataContainer::Iterator pointDataIter = itkPointSet->GetPointData()->Begin();
  for (ptIdx=0; ptIdx < nbPoints; ++ptIdx) // pointDataIter moved at end of loop
  {
    double currentPoint[3];
    m_WorldPositions->GetPoint(ptIdx, currentPoint);
    vtkSmartPointer<vtkPolyDataAlgorithm> source;

    //check for the pointtype in data and decide which geom-object to take and then add to the selected or unselected list
    int pointType;
    if ( itkPointSet->GetPointData()->size() == 0 || pointDataBroken )
      pointType = mitk::PTUNDEFINED;
    else
      pointType = pointDataIter.Value().pointSpec;

    bool showEdge = false;
    this->GetDataNode()->GetBoolProperty("show edge", showEdge);

    switch (pointType)
    {
    case mitk::PTUNDEFINED:
    {
      vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
      sphere->SetRadius(m_PointSize/2.0f);
      
      if (j > 0) {
        itk::Point<float> currentPoint = pointsIter->Value();
        if (showEdge) {
          CreateEdgeObjectsBetweenPoints(currentPoint, lastPoint);
        }
      }
      
      itk::Point<float> point1 = pointsIter->Value();
      lastPoint = point1;
      
      sphere->SetCenter(currentPoint);
      //sphere->SetCenter(pointsIter.Value()[0],pointsIter.Value()[1],pointsIter.Value()[2]);

      //MouseOrientation Tool (PositionTracker)
      if(isInputDevice)
      {         
        sphere->SetThetaResolution(10);
        sphere->SetPhiResolution(10);
      }
      else
      {
        sphere->SetThetaResolution(20);
        sphere->SetPhiResolution(20);
      }
      source = sphere;
    }
      break;
    case mitk::PTSTART:
    {
      vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New();
      cube->SetXLength(m_PointSize/2);
      cube->SetYLength(m_PointSize/2);
      cube->SetZLength(m_PointSize/2);
      cube->SetCenter(currentPoint);
      source = cube;
    }
      break;
    case mitk::PTCORNER:
      {
        vtkSmartPointer<vtkConeSource> cone = vtkSmartPointer<vtkConeSource>::New();
        cone->SetRadius(m_PointSize/2.0f);
        cone->SetCenter(currentPoint);
        cone->SetResolution(20);
        source = cone;
      }
      break;
    case mitk::PTEDGE:
      {
        vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
        cylinder->SetRadius(m_PointSize/2.0f);
        cylinder->SetCenter(currentPoint);
        cylinder->SetResolution(20);
        source = cylinder;
      }
      break;
    case mitk::PTEND:
      {
        vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetRadius(m_PointSize/2.0f);
        // no SetCenter?? this functionality should be explained!
        // otherwise: join with default block!
        sphere->SetThetaResolution(20);
        sphere->SetPhiResolution(20);
        source = sphere;
      }
      break;
    default:
      {
        vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetRadius(m_PointSize/2.0f);
        sphere->SetCenter(currentPoint);
        sphere->SetThetaResolution(20);
        sphere->SetPhiResolution(20);
        source = sphere;
      }
      break;
    }

    if (pointDataIter.Value().selected && !pointDataBroken)
    {
      m_vtkSelectedPointList->AddInputConnection(source->GetOutputPort());
      ++m_NumberOfSelectedAdded;
    }
    else
    {
      m_vtkUnselectedPointList->AddInputConnection(source->GetOutputPort());
      ++m_NumberOfUnselectedAdded;
    }
    if (showLabel)
    {
      char buffer[20];
      std::string l = pointLabel;
      if ( input->GetSize()>1 )
      {
        sprintf(buffer,"%d",ptIdx+1);
        l.append(buffer);
      }
      // Define the text for the label
      vtkSmartPointer<vtkVectorText> label = vtkSmartPointer<vtkVectorText>::New();
      label->SetText(l.c_str());

      //# Set up a transform to move the label to a new position.
      vtkSmartPointer<vtkTransform> aLabelTransform = vtkSmartPointer<vtkTransform>::New();
      aLabelTransform->Identity();
      aLabelTransform->Translate(currentPoint[0]+2, currentPoint[1]+2, currentPoint[2]);
      aLabelTransform->Scale(5.7,5.7,5.7);

      //# Move the label to a new position.
      vtkSmartPointer<vtkTransformPolyDataFilter> labelTransform = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      labelTransform->SetTransform(aLabelTransform);
      labelTransform->SetInputConnection(label->GetOutputPort());

      //add it to the wright PointList
      if (pointType)
      {
        m_vtkSelectedPointList->AddInputConnection(labelTransform->GetOutputPort());
        ++m_NumberOfSelectedAdded;
      }
      else
      {
        m_vtkUnselectedPointList->AddInputConnection(labelTransform->GetOutputPort());
        ++m_NumberOfUnselectedAdded;
      }
    }

    if(pointDataIter != itkPointSet->GetPointData()->End())
      pointDataIter++;
  } // end FOR

  //now according to number of elements added to selected or unselected, build up the rendering pipeline
  if (m_NumberOfSelectedAdded > 0)
  {
    m_VtkSelectedPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_VtkSelectedPolyDataMapper->SetInputConnection(m_vtkSelectedPointList->GetOutputPort());

    //create a new instance of the actor
    m_SelectedActor = vtkSmartPointer<vtkActor>::New();

    m_SelectedActor->SetMapper(m_VtkSelectedPolyDataMapper);
    m_PointsAssembly->AddPart(m_SelectedActor);
  }

  if (m_NumberOfUnselectedAdded > 0)
  {
    m_VtkUnselectedPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_VtkUnselectedPolyDataMapper->SetInputConnection(m_vtkUnselectedPointList->GetOutputPort());

    //create a new instance of the actor
    m_UnselectedActor = vtkSmartPointer<vtkActor>::New();

    m_UnselectedActor->SetMapper(m_VtkUnselectedPolyDataMapper);
    m_PointsAssembly->AddPart(m_UnselectedActor);
  }
}

void mitk::PointSetVtkMapper3D::VertexRendering()
{
  // get and update the PointSet
  mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());

  /* only update the input data, if the property tells us to */
  bool update = true;
  this->GetDataNode()->GetBoolProperty("updateDataOnRender", update);
  if (update == true)
    input->Update();
  int timestep = this->GetTimestep();

  mitk::PointSet::DataType::Pointer itkPointSet = input->GetPointSet( timestep );

  // turn off standard actors
  m_UnselectedActor->VisibilityOff();
  m_SelectedActor->VisibilityOff();

  // point size
  m_PointSize = 2.0;
  mitk::FloatProperty::Pointer pointSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("pointsize"));
  if ( pointSizeProp.IsNotNull() )
    m_PointSize = pointSizeProp->GetValue();

  double* color = m_UnselectedActor->GetProperty()->GetColor();
  double opacity = m_UnselectedActor->GetProperty()->GetOpacity();

  glClearColor(0.0,0.0,0.0,0.0);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_LIGHTING);
  glEnable(GL_POINT_SMOOTH);

  glPointSize(m_PointSize);
  glBegin(GL_POINTS);

  glColor4d(color[0],color[1],color[2],opacity);

  for ( auto pointsIter=itkPointSet->GetPoints()->Begin();
        pointsIter!=itkPointSet->GetPoints()->End();
        pointsIter++ )
  {
    const itk::Point<mitk::ScalarType>& point = pointsIter->Value();
    glVertex3d(point[0],point[1],point[2]);
  }

  glEnd();

  // reset context
  glPointSize(1.0);
  glDisable(GL_POINT_SMOOTH);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHTING);
}


void mitk::PointSetVtkMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if(!visible)
  {
    m_UnselectedActor->VisibilityOff();
    m_SelectedActor->VisibilityOff();
    m_ContourActor->VisibilityOff();
    return;
  }

  // create new vtk render objects (e.g. sphere for a point)

  SetVtkMapperImmediateModeRendering(m_VtkSelectedPolyDataMapper);
  SetVtkMapperImmediateModeRendering(m_VtkUnselectedPolyDataMapper);

  BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  bool needGenerateData = ls->IsGenerateDataRequired( renderer, this, GetDataNode() );

  if(!needGenerateData)
  {
    mitk::FloatProperty * pointSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("pointsize"));
    mitk::FloatProperty * contourSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("contoursize"));

    bool useVertexRendering = false;
    this->GetDataNode()->GetBoolProperty("Vertex Rendering", useVertexRendering);

    // only create new vtk render objects if property values were changed
    if(pointSizeProp && m_PointSize!=pointSizeProp->GetValue() )
      needGenerateData = true;
    if(contourSizeProp && m_ContourRadius!=contourSizeProp->GetValue() )
      needGenerateData = true;

    // when vertex rendering is enabled the pointset is always
    // drawn with opengl, thus we leave needGenerateData always false
    if(useVertexRendering && m_VertexRendering != useVertexRendering )
    {
      needGenerateData = false;
      m_VertexRendering = true;
    } else if(!useVertexRendering && m_VertexRendering)
    {
      m_VertexRendering = false;
      needGenerateData = true;
    }
  }

  if(needGenerateData)
  {
    this->CreateVTKRenderObjects();
    ls->UpdateGenerateDataTime();
  }

  this->ApplyAllProperties(renderer, m_ContourActor);

  bool showPoints = true;
  this->GetDataNode()->GetBoolProperty("show points", showPoints);

  m_UnselectedActor->SetVisibility( showPoints && !m_VertexRendering);
  m_SelectedActor->SetVisibility( showPoints && !m_VertexRendering);

  if(false && dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("opacity")) != NULL)
  {
    mitk::FloatProperty::Pointer pointOpacity =dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("opacity"));
    float opacity = pointOpacity->GetValue();
    m_ContourActor->GetProperty()->SetOpacity(opacity);
    m_UnselectedActor->GetProperty()->SetOpacity(opacity);
    m_SelectedActor->GetProperty()->SetOpacity(opacity);
  }

  bool showContour = false;
  this->GetDataNode()->GetBoolProperty("show contour", showContour);
  m_ContourActor->SetVisibility( showContour );

  // use vertex rendering
  if(m_VertexRendering)
  {
    VertexRendering();
    ls->UpdateGenerateDataTime();
  }
}


void mitk::PointSetVtkMapper3D::ResetMapper( BaseRenderer* /*renderer*/ )
{
  m_PointsAssembly->VisibilityOff();
}


vtkProp* mitk::PointSetVtkMapper3D::GetVtkProp(mitk::BaseRenderer* /*renderer*/)
{
  return m_PointsAssembly;
}

void mitk::PointSetVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer* /*renderer*/)
{
}

void mitk::PointSetVtkMapper3D::ApplyAllProperties(mitk::BaseRenderer* renderer, vtkActor* actor)
{
  Superclass::ApplyColorAndOpacityProperties(renderer, actor);
  //check for color props and use it for rendering of selected/unselected points and contour
  //due to different params in VTK (double/float) we have to convert!

  //vars to convert to
  double unselectedColor[4]={1.0f,1.0f,0.0f,1.0f};//yellow
  double selectedColor[4]={1.0f,0.0f,0.0f,1.0f};//red
  double contourColor[4]={1.0f,0.0f,0.0f,1.0f};//red

  //different types for color!!!
  mitk::Color tmpColor;
  double opacity = 1.0;

  //check if there is an unselected property
  if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("unselectedcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("unselectedcolor"))->GetValue();
    unselectedColor[0] = tmpColor[0];
    unselectedColor[1] = tmpColor[1];
    unselectedColor[2] = tmpColor[2];
    unselectedColor[3] = 1.0f; //!!define a new ColorProp to be able to pass alpha value
  }
  else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("unselectedcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("unselectedcolor"))->GetValue();
    unselectedColor[0] = tmpColor[0];
    unselectedColor[1] = tmpColor[1];
    unselectedColor[2] = tmpColor[2];
    unselectedColor[3] = 1.0f; //!!define a new ColorProp to be able to pass alpha value
  }
  else
  {
    //check if the node has a color
    float unselectedColorTMP[4]={1.0f,1.0f,0.0f,1.0f};//yellow
    m_DataNode->GetColor(unselectedColorTMP, NULL);
    unselectedColor[0] = unselectedColorTMP[0];
    unselectedColor[1] = unselectedColorTMP[1];
    unselectedColor[2] = unselectedColorTMP[2];
    //unselectedColor[3] stays 1.0f
  }

  //get selected property
  if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("selectedcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("selectedcolor"))->GetValue();
    selectedColor[0] = tmpColor[0];
    selectedColor[1] = tmpColor[1];
    selectedColor[2] = tmpColor[2];
    selectedColor[3] = 1.0f;
  }
  else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("selectedcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("selectedcolor"))->GetValue();
    selectedColor[0] = tmpColor[0];
    selectedColor[1] = tmpColor[1];
    selectedColor[2] = tmpColor[2];
    selectedColor[3] = 1.0f;
  }

  //get contour property
  if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("contourcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("contourcolor"))->GetValue();
    contourColor[0] = tmpColor[0];
    contourColor[1] = tmpColor[1];
    contourColor[2] = tmpColor[2];
    contourColor[3] = 1.0f;
  }
  else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("contourcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("contourcolor"))->GetValue();
    contourColor[0] = tmpColor[0];
    contourColor[1] = tmpColor[1];
    contourColor[2] = tmpColor[2];
    contourColor[3] = 1.0f;
  }

  if(dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("opacity")) != NULL)
  {
    mitk::FloatProperty::Pointer pointOpacity =dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetPropertyList(renderer)->GetProperty("opacity"));
    opacity = pointOpacity->GetValue();
  }
  else if(dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("opacity")) != NULL)
  {
    mitk::FloatProperty::Pointer pointOpacity =dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetPropertyList(NULL)->GetProperty("opacity"));
    opacity = pointOpacity->GetValue();
  }
  //finished color / opacity fishing!

  //check if a contour shall be drawn
  bool showContour = false;
  this->GetDataNode()->GetBoolProperty("show contour", showContour, renderer);
  if(showContour && (m_ContourActor != NULL) )
  {
    this->CreateContour(m_WorldPositions, m_PointConnections);
    m_ContourActor->GetProperty()->SetColor(contourColor);
    m_ContourActor->GetProperty()->SetOpacity(opacity);
  }

  m_SelectedActor->GetProperty()->SetColor(selectedColor);
  m_SelectedActor->GetProperty()->SetOpacity(opacity);

  m_UnselectedActor->GetProperty()->SetColor(unselectedColor);
  m_UnselectedActor->GetProperty()->SetOpacity(opacity);
}

void mitk::PointSetVtkMapper3D::CreateContour(vtkPoints* points, vtkCellArray* m_PointConnections)
{
  vtkSmartPointer<vtkAppendPolyData> vtkContourPolyData = vtkSmartPointer<vtkAppendPolyData>::New();
  vtkSmartPointer<vtkPolyDataMapper> vtkContourPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

  vtkSmartPointer<vtkPolyData> contour = vtkSmartPointer<vtkPolyData>::New();
  contour->SetPoints(points);
  contour->SetLines(m_PointConnections);

  vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  tubeFilter->SetNumberOfSides( 12 );
  tubeFilter->SetInputData(contour);

  //check for property contoursize.
  m_ContourRadius = 0.5;
  mitk::FloatProperty::Pointer contourSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("contoursize") );

  if (contourSizeProp.IsNotNull())
    m_ContourRadius = contourSizeProp->GetValue();

  tubeFilter->SetRadius( m_ContourRadius );
  tubeFilter->Update();

  //add to pipeline
  vtkContourPolyData->AddInputConnection(tubeFilter->GetOutputPort());
  vtkContourPolyDataMapper->SetInputConnection(vtkContourPolyData->GetOutputPort());

  m_ContourActor->SetMapper(vtkContourPolyDataMapper);
  m_PointsAssembly->AddPart(m_ContourActor);
}

void mitk::PointSetVtkMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "line width", mitk::IntProperty::New(2), renderer, overwrite );
  node->AddProperty( "pointsize", mitk::FloatProperty::New(1.0), renderer, overwrite);
  node->AddProperty( "selectedcolor", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite);  //red
  node->AddProperty( "color", mitk::ColorProperty::New(1.0f, 1.0f, 0.0f), renderer, overwrite);  //yellow
  node->AddProperty( "opacity", mitk::FloatProperty::New(1.0f), renderer, overwrite );
  node->AddProperty( "show contour", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "close contour", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "contourcolor", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite);
  node->AddProperty( "contoursize", mitk::FloatProperty::New(0.5), renderer, overwrite );
  node->AddProperty( "show points", mitk::BoolProperty::New(true), renderer, overwrite );
  node->AddProperty( "updateDataOnRender", mitk::BoolProperty::New(true), renderer, overwrite );
  node->AddProperty( "Vertex Rendering",mitk::BoolProperty::New(false),renderer,overwrite );
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

