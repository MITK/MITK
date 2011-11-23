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

#include <stdlib.h>



const mitk::PointSet* mitk::PointSetVtkMapper3D::GetInput()
{
  return static_cast<const mitk::PointSet * > ( GetData() );
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
 m_ContourRadius(0.5)
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
    if( this->GetDataNode()->GetIntProperty("BaseRendererMapperID",mapperID) && mapperID == 2)
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

  mitk::PointSet::PointsContainer::Iterator pointsIter;
  mitk::PointSet::PointDataContainer::Iterator pointDataIter;
  int j;

  m_NumberOfSelectedAdded = 0;
  m_NumberOfUnselectedAdded = 0;

  //create contour
  bool makeContour = false;
  this->GetDataNode()->GetBoolProperty("show contour", makeContour);
  if (makeContour)
  {
    this->CreateContour(NULL);
  }

  //now fill selected and unselected pointList
  //get size of Points in Property
  m_PointSize = 2;
  mitk::FloatProperty::Pointer pointSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("pointsize"));
  if ( pointSizeProp.IsNotNull() )
    m_PointSize = pointSizeProp->GetValue();

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


  //check if the list for the PointDataContainer is the same size as the PointsContainer. Is not, then the points were inserted manually and can not be visualized according to the PointData (selected/unselected)
  bool pointDataBroken = (itkPointSet->GetPointData()->Size() != itkPointSet->GetPoints()->Size());
  //now add an object for each point in data
  pointDataIter = itkPointSet->GetPointData()->Begin();
  for (j=0, pointsIter=itkPointSet->GetPoints()->Begin(); 
    pointsIter!=itkPointSet->GetPoints()->End();
    pointsIter++, j++)
  {
    //check for the pointtype in data and decide which geom-object to take and then add to the selected or unselected list
    int pointType;

    if(itkPointSet->GetPointData()->size() == 0 || pointDataBroken)
      pointType = mitk::PTUNDEFINED;
    else
      pointType = pointDataIter.Value().pointSpec;

    vtkSmartPointer<vtkPolyDataAlgorithm> source;

    switch (pointType)
    {
    case mitk::PTUNDEFINED:
      {
        vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetRadius(m_PointSize);
        itk::Point<float> point1 = pointsIter->Value();
        sphere->SetCenter(point1[0],point1[1],point1[2]);
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
        itk::Point<float> point1 = pointsIter->Value();
        cube->SetCenter(point1[0],point1[1],point1[2]);
        source = cube;
      }
      break;
    case mitk::PTCORNER:
      {
        vtkSmartPointer<vtkConeSource> cone = vtkSmartPointer<vtkConeSource>::New();
        cone->SetRadius(m_PointSize);
        itk::Point<float> point1 = pointsIter->Value();
        cone->SetCenter(point1[0],point1[1],point1[2]);
        cone->SetResolution(20);
        source = cone;
      }
      break;
    case mitk::PTEDGE:
      {
        vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
        cylinder->SetRadius(m_PointSize);
        itk::Point<float> point1 = pointsIter->Value();
        cylinder->SetCenter(point1[0],point1[1],point1[2]);
        cylinder->SetResolution(20);
        source = cylinder;
      }
      break;
    case mitk::PTEND:
      {
        vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetRadius(m_PointSize);
        itk::Point<float> point1 = pointsIter->Value(); 
        sphere->SetThetaResolution(20);
        sphere->SetPhiResolution(20);
        source = sphere;
      }
      break;
    default:
      {
        vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetRadius(m_PointSize);
        itk::Point<float> point1 = pointsIter->Value(); 
        sphere->SetCenter(point1[0],point1[1],point1[2]);
        sphere->SetThetaResolution(20);
        sphere->SetPhiResolution(20);
        source = sphere;
      }
      break;
    }
    if (!pointDataBroken)
    {
      if (pointDataIter.Value().selected)
      {
        m_vtkSelectedPointList->AddInput(source->GetOutput());
        ++m_NumberOfSelectedAdded;
      }
      else 
      {
        m_vtkUnselectedPointList->AddInput(source->GetOutput());
        ++m_NumberOfUnselectedAdded;
      }
    }
    else
    {
      m_vtkUnselectedPointList->AddInput(source->GetOutput());
      ++m_NumberOfUnselectedAdded;
    }

    if (showLabel)
    {
      char buffer[20];
      std::string l = pointLabel;
      if ( input->GetSize()>1 )
      {
        sprintf(buffer,"%d",j+1);
        l.append(buffer);
      }
      // Define the text for the label
      vtkSmartPointer<vtkVectorText> label = vtkSmartPointer<vtkVectorText>::New();
      label->SetText(l.c_str());

      //# Set up a transform to move the label to a new position.
      vtkSmartPointer<vtkTransform> aLabelTransform = vtkSmartPointer<vtkTransform>::New();
      aLabelTransform->Identity();
      itk::Point<float> point1 = pointsIter->Value();
      aLabelTransform->Translate(point1[0]+2,point1[1]+2,point1[2]);
      aLabelTransform->Scale(5.7,5.7,5.7);

      //# Move the label to a new position.
      vtkSmartPointer<vtkTransformPolyDataFilter> labelTransform = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      labelTransform->SetTransform(aLabelTransform);
      labelTransform->SetInput(label->GetOutput());

      //add it to the wright PointList
      if (pointType)
      {
        m_vtkSelectedPointList->AddInput(labelTransform->GetOutput());
        ++m_NumberOfSelectedAdded;
      }
      else 
      {
        m_vtkUnselectedPointList->AddInput(labelTransform->GetOutput());
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
    m_VtkSelectedPolyDataMapper->SetInput(m_vtkSelectedPointList->GetOutput());

    //create a new instance of the actor
    m_SelectedActor = vtkSmartPointer<vtkActor>::New();

    m_SelectedActor->SetMapper(m_VtkSelectedPolyDataMapper);
    m_PointsAssembly->AddPart(m_SelectedActor);
  }

  if (m_NumberOfUnselectedAdded > 0)
  {
    m_VtkUnselectedPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_VtkUnselectedPolyDataMapper->SetInput(m_vtkUnselectedPointList->GetOutput());

    //create a new instance of the actor
    m_UnselectedActor = vtkSmartPointer<vtkActor>::New();

    m_UnselectedActor->SetMapper(m_VtkUnselectedPolyDataMapper);
    m_PointsAssembly->AddPart(m_UnselectedActor);
  }
}


void mitk::PointSetVtkMapper3D::GenerateData()
{
  //create new vtk render objects (e.g. sphere for a point)
  this->CreateVTKRenderObjects();

  //apply props
  this->ApplyProperties(m_ContourActor,NULL);

}


void mitk::PointSetVtkMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  SetVtkMapperImmediateModeRendering(m_VtkSelectedPolyDataMapper);
  SetVtkMapperImmediateModeRendering(m_VtkUnselectedPolyDataMapper);

  mitk::FloatProperty::Pointer pointSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("pointsize"));
  mitk::FloatProperty::Pointer contourSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("contoursize"));
  // only create new vtk render objects if property values were changed
  if ( pointSizeProp.IsNotNull() &&  contourSizeProp.IsNotNull() )
  {
    if (m_PointSize!=pointSizeProp->GetValue() || m_ContourRadius!= contourSizeProp->GetValue())
    {
      this->CreateVTKRenderObjects();
    }
  }

  this->ApplyProperties(m_ContourActor,renderer);

  if(IsVisible(renderer)==false)
  {
    m_UnselectedActor->VisibilityOff();
    m_SelectedActor->VisibilityOff();
    m_ContourActor->VisibilityOff();
    return;
  }

  bool showPoints = true;
  this->GetDataNode()->GetBoolProperty("show points", showPoints);

  if(showPoints)
  {
    m_UnselectedActor->VisibilityOn();
    m_SelectedActor->VisibilityOn();
  }
  else
  {
    m_UnselectedActor->VisibilityOff();
    m_SelectedActor->VisibilityOff();
  }

  if(dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("opacity")) != NULL)
  {  
    mitk::FloatProperty::Pointer pointOpacity =dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("opacity"));
    float opacity = pointOpacity->GetValue();
    m_ContourActor->GetProperty()->SetOpacity(opacity);
    m_UnselectedActor->GetProperty()->SetOpacity(opacity);
    m_SelectedActor->GetProperty()->SetOpacity(opacity);
  }

  bool makeContour = false;
  this->GetDataNode()->GetBoolProperty("show contour", makeContour);
  if (makeContour)
  {
    m_ContourActor->VisibilityOn();
  }
  else
  {
    m_ContourActor->VisibilityOff();
  }
}


void mitk::PointSetVtkMapper3D::ResetMapper( BaseRenderer* /*renderer*/ )
{
  m_PointsAssembly->VisibilityOff();
}


vtkProp* mitk::PointSetVtkMapper3D::GetVtkProp(mitk::BaseRenderer * /*renderer*/)
{
  return m_PointsAssembly;
}

void mitk::PointSetVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer * /*renderer*/)
{
  vtkSmartPointer<vtkLinearTransform> vtktransform =
    this->GetDataNode()->GetVtkTransform(this->GetTimestep());

  m_SelectedActor->SetUserTransform(vtktransform);
  m_UnselectedActor->SetUserTransform(vtktransform);
  m_ContourActor->SetUserTransform(vtktransform);
}

void mitk::PointSetVtkMapper3D::ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer)
{
  Superclass::ApplyProperties(actor,renderer);
  //check for color props and use it for rendering of selected/unselected points and contour 
  //due to different params in VTK (double/float) we have to convert!

  //vars to convert to
  vtkFloatingPointType unselectedColor[4]={1.0f,1.0f,0.0f,1.0f};//yellow
  vtkFloatingPointType selectedColor[4]={1.0f,0.0f,0.0f,1.0f};//red
  vtkFloatingPointType contourColor[4]={1.0f,0.0f,0.0f,1.0f};//red

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
  bool makeContour = false;
  this->GetDataNode()->GetBoolProperty("show contour", makeContour, renderer);
  int visibleBefore = m_ContourActor->GetVisibility();
  if(makeContour && (m_ContourActor != NULL) )
  {
    if ( visibleBefore == 0)
    {//was not visible before, so create it.
      this->CreateContour(renderer);
    }
    m_ContourActor->GetProperty()->SetColor(contourColor);
    m_ContourActor->GetProperty()->SetOpacity(opacity);
  }

  m_SelectedActor->GetProperty()->SetColor(selectedColor);
  m_SelectedActor->GetProperty()->SetOpacity(opacity);

  m_UnselectedActor->GetProperty()->SetColor(unselectedColor);
  m_UnselectedActor->GetProperty()->SetOpacity(opacity);

}

void mitk::PointSetVtkMapper3D::CreateContour(mitk::BaseRenderer* renderer)
{
  vtkSmartPointer<vtkAppendPolyData> vtkContourPolyData = vtkSmartPointer<vtkAppendPolyData>::New();
  vtkSmartPointer<vtkPolyDataMapper> vtkContourPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New(); 

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();

  mitk::PointSet::PointsContainer::Iterator pointsIter;
//  mitk::PointSet::PointDataContainer::Iterator pointDataIter;
  int j;

  // get and update the PointSet
  mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());

  int timestep = this->GetTimestep();
  mitk::PointSet::DataType::Pointer itkPointSet = input->GetPointSet( timestep );
  if ( itkPointSet.GetPointer() == NULL) 
  {
    return;
  }

  for (j=0, pointsIter=itkPointSet->GetPoints()->Begin(); pointsIter!=itkPointSet->GetPoints()->End() ; pointsIter++,j++)
  {
    vtkIdType cell[2] = {j-1,j};
    itk::Point<float> point1 = pointsIter->Value();
    points->InsertPoint(j,point1[0],point1[1],point1[2]);
    if (j>0)
      polys->InsertNextCell(2,cell);
  }

  bool close;
  if (dynamic_cast<mitk::BoolProperty *>(this->GetDataNode()->GetPropertyList()->GetProperty("close contour"), renderer) == NULL)
    close = false;
  else
    close = dynamic_cast<mitk::BoolProperty *>(this->GetDataNode()->GetPropertyList()->GetProperty("close contour"), renderer)->GetValue();
  if (close) 
  {
    vtkIdType cell[2] = {j-1,0};
    polys->InsertNextCell(2,cell);
  }

  vtkSmartPointer<vtkPolyData> contour = vtkSmartPointer<vtkPolyData>::New();
  contour->SetPoints(points);
  contour->SetLines(polys);
  contour->Update();

  vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  tubeFilter->SetNumberOfSides( 12 );
  tubeFilter->SetInput(contour);

  //check for property contoursize. 
  m_ContourRadius = 0.5;
  mitk::FloatProperty::Pointer contourSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("contoursize") );

  if (contourSizeProp.IsNotNull())
    m_ContourRadius = contourSizeProp->GetValue();

  tubeFilter->SetRadius( m_ContourRadius );
  tubeFilter->Update();

  //add to pipeline
  vtkContourPolyData->AddInput(tubeFilter->GetOutput());
  vtkContourPolyDataMapper->SetInput(vtkContourPolyData->GetOutput());

  m_ContourActor->SetMapper(vtkContourPolyDataMapper);
  m_PointsAssembly->AddPart(m_ContourActor);
}

void mitk::PointSetVtkMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "line width", mitk::IntProperty::New(2), renderer, overwrite );
  node->AddProperty( "pointsize", mitk::FloatProperty::New(1.0), renderer, overwrite);
  node->AddProperty( "selectedcolor", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite);  //red
  node->AddProperty( "color", mitk::ColorProperty::New(1.0f, 1.0f, 0.0f), renderer, overwrite);  //yellow
  node->AddProperty( "show contour", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "contourcolor", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite);
  node->AddProperty( "contoursize", mitk::FloatProperty::New(0.5), renderer, overwrite );
  node->AddProperty( "show points", mitk::BoolProperty::New(true), renderer, overwrite );
  node->AddProperty( "updateDataOnRender", mitk::BoolProperty::New(true), renderer, overwrite );  
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

