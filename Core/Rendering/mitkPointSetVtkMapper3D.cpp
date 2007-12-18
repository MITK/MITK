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
#include "mitkDataTreeNode.h"
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

#if (VTK_MAJOR_VERSION >= 5)
#include <vtkPolyDataAlgorithm.h>
#else
#include <vtkPolyData.h>
#endif

#include <stdlib.h>



const mitk::PointSet* mitk::PointSetVtkMapper3D::GetInput()
{
  return static_cast<const mitk::PointSet * > ( GetData() );
}

mitk::PointSetVtkMapper3D::PointSetVtkMapper3D() 
: m_vtkSelectedPointList(NULL),
  m_vtkUnselectedPointList(NULL), 
  //m_vtkContourPolyData(NULL),
  m_VtkSelectedPolyDataMapper(NULL), 
  m_VtkUnselectedPolyDataMapper(NULL),
  //m_vtkContourPolyDataMapper(NULL),
  m_vtkTextList(NULL), 
  //m_Contour(NULL), 
  //m_TubeFilter(NULL),
  m_NumberOfSelectedAdded(0), 
  m_NumberOfUnselectedAdded(0)
{
  //propassembly
  m_PointsAssembly = vtkPropAssembly::New();

  //creating actors to be able to set transform
  m_SelectedActor = vtkActor::New();
  m_UnselectedActor = vtkActor::New();
  m_ContourActor = vtkActor::New();
}

mitk::PointSetVtkMapper3D::~PointSetVtkMapper3D()
{
  m_PointsAssembly->Delete();

  //m_SelectedActor->Delete(); //leads to an exeption
  m_UnselectedActor->Delete();
  m_ContourActor->Delete();
}


void mitk::PointSetVtkMapper3D::GenerateData()
{
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
  if( this->GetDataTreeNode()->GetBoolProperty("inputdevice",isInputDevice) && isInputDevice )
  {
    if( this->GetDataTreeNode()->GetIntProperty("BaseRendererMapperID",mapperID) && mapperID == 2)
      return; //The event for the PositionTracker came from the 3d widget and  not needs to be displayed
  }

  // get and update the PointSet
  mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());
  input->Update();

  mitk::PointSet::DataType::Pointer itkPointSet = input->GetPointSet( m_TimeStep );

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
  this->GetDataTreeNode()->GetBoolProperty("contour", makeContour);
  if (makeContour)
  {
    this->CreateContour(NULL);
  }

  //now fill selected and unselected pointList
  //get size of Points in Property
  ScalarType pointSize = 2;
  mitk::FloatProperty::Pointer pointSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList()->GetProperty("pointsize"));
  if ( pointSizeProp.IsNotNull() )
    pointSize = pointSizeProp->GetValue();

  //get the property for creating a lable onto every point only once
  bool showLabel = true;
  this->GetDataTreeNode()->GetBoolProperty("show label", showLabel);
  const char * pointLabel=NULL;
  if(showLabel)
  {
    if(dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetPropertyList()->GetProperty("label")) != NULL)
      pointLabel =dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetPropertyList()->GetProperty("label"))->GetValue();
    else
      showLabel = false;
  }

  m_vtkSelectedPointList = vtkAppendPolyData::New();
  m_vtkUnselectedPointList = vtkAppendPolyData::New();
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

#if (VTK_MAJOR_VERSION >= 5)
    vtkPolyDataAlgorithm *source;
#else
    vtkPolyDataSource *source;
#endif

    switch (pointType)
    {
    case mitk::PTUNDEFINED:
      {
        vtkSphereSource *sphere = vtkSphereSource::New();
        sphere->SetRadius(pointSize);
        mitk::Point3D point1;
        point1 = input->GetPoint(pointsIter->Index(), m_TimeStep);
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
        vtkCubeSource *cube = vtkCubeSource::New();
        cube->SetXLength(pointSize/2);
        cube->SetYLength(pointSize/2);
        cube->SetZLength(pointSize/2);
        mitk::Point3D point1;
        point1 = input->GetPoint(pointsIter->Index(), m_TimeStep);
        cube->SetCenter(point1[0],point1[1],point1[2]);
        //cube->SetCenter(pointsIter.Value()[0],pointsIter.Value()[1],pointsIter.Value()[2]);
        source = cube;
      }
      break;
    case mitk::PTCORNER:
      {
        vtkConeSource *cone = vtkConeSource::New();
        cone->SetRadius(pointSize);
        mitk::Point3D point1;
        point1 = input->GetPoint(pointsIter->Index(), m_TimeStep);
        cone->SetCenter(point1[0],point1[1],point1[2]);
        //cone->SetCenter(pointsIter.Value()[0],pointsIter.Value()[1],pointsIter.Value()[2]);
        cone->SetResolution(20);
        source = cone;
      }
      break;
    case mitk::PTEDGE:
      {
        vtkCylinderSource *cylinder = vtkCylinderSource::New();
        cylinder->SetRadius(pointSize);
        mitk::Point3D point1;
        point1 = input->GetPoint(pointsIter->Index(), m_TimeStep);
        cylinder->SetCenter(point1[0],point1[1],point1[2]);
        //cylinder->SetCenter(pointsIter.Value()[0],pointsIter.Value()[1],pointsIter.Value()[2]);
        cylinder->SetResolution(20);
        source = cylinder;
      }
      break;
    case mitk::PTEND:
      {
        vtkSphereSource *sphere = vtkSphereSource::New();
        sphere->SetRadius(pointSize);
        mitk::Point3D point1;
        point1 = input->GetPoint(pointsIter->Index(), m_TimeStep);
        sphere->SetCenter(point1[0],point1[1],point1[2]);
        //sphere->SetCenter(pointsIter.Value()[0],pointsIter.Value()[1],pointsIter.Value()[2]);
        sphere->SetThetaResolution(20);
        sphere->SetPhiResolution(20);
        source = sphere;
      }
      break;
    default:
      {
        vtkSphereSource *sphere = vtkSphereSource::New();
        sphere->SetRadius(pointSize);
        mitk::Point3D point1;
        point1 = input->GetPoint(pointsIter->Index(), m_TimeStep);
        sphere->SetCenter(point1[0],point1[1],point1[2]);
        //sphere->SetCenter(pointsIter.Value()[0],pointsIter.Value()[1],pointsIter.Value()[2]);
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

    source->Delete();

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
      vtkVectorText *label = vtkVectorText::New();
      label->SetText(l.c_str());

      //# Set up a transform to move the label to a new position.
      vtkTransform *aLabelTransform =vtkTransform::New();
      aLabelTransform->Identity();
      mitk::Point3D point1;
      point1 = input->GetPoint(pointsIter->Index(), m_TimeStep);
      aLabelTransform->Translate(point1[0]+2,point1[1]+2,point1[2]);
      //aLabelTransform->Translate(pointsIter.Value()[0]+2,pointsIter.Value()[1]+2,pointsIter.Value()[2]);
      aLabelTransform->Scale(5.7,5.7,5.7);

      //# Move the label to a new position.
      vtkTransformPolyDataFilter *labelTransform = vtkTransformPolyDataFilter::New();
      labelTransform->SetTransform(aLabelTransform);
      aLabelTransform->Delete();
      labelTransform->SetInput(label->GetOutput());
      label->Delete();

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
      labelTransform->Delete();
    }

    if(pointDataIter != itkPointSet->GetPointData()->End())
      pointDataIter++;
  } // end FOR

  //now according to number of elements added to selected or unselected, build up the rendering pipeline
  if (m_NumberOfSelectedAdded > 0)
  {
    m_VtkSelectedPolyDataMapper = vtkPolyDataMapper::New();
    m_VtkSelectedPolyDataMapper->SetInput(m_vtkSelectedPointList->GetOutput());
    m_vtkSelectedPointList->Delete();

    //create a new instance of the actor
    m_SelectedActor->Delete();
    m_SelectedActor = vtkActor::New();

    m_SelectedActor->SetMapper(m_VtkSelectedPolyDataMapper);
    m_VtkSelectedPolyDataMapper->Delete();
    m_PointsAssembly->AddPart(m_SelectedActor);
  }

  if (m_NumberOfUnselectedAdded > 0)
  {
    m_VtkUnselectedPolyDataMapper = vtkPolyDataMapper::New();
    m_VtkUnselectedPolyDataMapper->SetInput(m_vtkUnselectedPointList->GetOutput());
    m_vtkUnselectedPointList->Delete();

    //create a new instance of the actor
    m_UnselectedActor->Delete();
    m_UnselectedActor = vtkActor::New();

    m_UnselectedActor->SetMapper(m_VtkUnselectedPolyDataMapper);
    m_VtkUnselectedPolyDataMapper->Delete();
    m_PointsAssembly->AddPart(m_UnselectedActor);
  }

    //apply props
  this->ApplyProperties(NULL);

}


void mitk::PointSetVtkMapper3D::GenerateData( mitk::BaseRenderer *renderer )
{
  
  this->ApplyProperties(renderer);
  
  if(IsVisible(renderer)==false)
  {
    m_UnselectedActor->VisibilityOff();
    m_SelectedActor->VisibilityOff();
    m_ContourActor->VisibilityOff();
    return;
  }

  bool showPoints = true;
  this->GetDataTreeNode()->GetBoolProperty("show points", showPoints);
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
 
  if(dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("opacity")) != NULL)
  {  
    mitk::FloatProperty::Pointer pointOpacity =dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("opacity"));
    float opacity = pointOpacity->GetValue();
    m_ContourActor->GetProperty()->SetOpacity(opacity);
    m_UnselectedActor->GetProperty()->SetOpacity(opacity);
    m_SelectedActor->GetProperty()->SetOpacity(opacity);
  }

  bool makeContour = false;
  this->GetDataTreeNode()->GetBoolProperty("contour", makeContour);
  if (makeContour)
  {
    m_ContourActor->VisibilityOn();
  }
  else
  {
    m_ContourActor->VisibilityOff();
  }


  // Get the TimeSlicedGeometry of the input object
  mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());
  const TimeSlicedGeometry* inputTimeGeometry = input->GetTimeSlicedGeometry();
  if (( inputTimeGeometry == NULL ) || ( inputTimeGeometry->GetTimeSteps() == 0 ))
  {
    m_PointsAssembly->VisibilityOff();
    return;
  }

  if ( inputTimeGeometry->IsValidTime( m_TimeStep ) == false )
  {
    m_PointsAssembly->VisibilityOff();
    return;
  }
}

vtkProp* mitk::PointSetVtkMapper3D::GetProp()
{
  if(GetDataTreeNode()!=NULL && 
    m_PointsAssembly != NULL) 
  {
    m_SelectedActor->SetUserTransform(GetDataTreeNode()->GetVtkTransform());
    m_UnselectedActor->SetUserTransform(GetDataTreeNode()->GetVtkTransform());
    m_ContourActor->SetUserTransform(GetDataTreeNode()->GetVtkTransform());
  } 
  return m_PointsAssembly;
}

void mitk::PointSetVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer* renderer)
{
  vtkLinearTransform * vtktransform = 
    this->GetDataTreeNode()->GetVtkTransform(
      renderer->GetTimeStep(this->GetDataTreeNode()->GetData()));

  m_SelectedActor->SetUserTransform(vtktransform);
  m_UnselectedActor->SetUserTransform(vtktransform);
  m_ContourActor->SetUserTransform(vtktransform);
}

void mitk::PointSetVtkMapper3D::ApplyProperties(mitk::BaseRenderer* renderer)
{
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
  if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("unselectedcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("unselectedcolor"))->GetValue();
    unselectedColor[0] = tmpColor[0];
    unselectedColor[1] = tmpColor[1];
    unselectedColor[2] = tmpColor[2];
    unselectedColor[3] = 1.0f; //!!define a new ColorProp to be able to pass alpha value
  }
  else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("unselectedcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("unselectedcolor"))->GetValue();
    unselectedColor[0] = tmpColor[0];
    unselectedColor[1] = tmpColor[1];
    unselectedColor[2] = tmpColor[2];
    unselectedColor[3] = 1.0f; //!!define a new ColorProp to be able to pass alpha value
  }
  else
  {
    //check if the node has a color
    float unselectedColorTMP[4]={1.0f,1.0f,0.0f,1.0f};//yellow
    m_DataTreeNode->GetColor(unselectedColorTMP, NULL);
    unselectedColor[0] = unselectedColorTMP[0];
    unselectedColor[1] = unselectedColorTMP[1];
    unselectedColor[2] = unselectedColorTMP[2];
    //unselectedColor[3] stays 1.0f
  }

  //get selected property
  if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("selectedcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("selectedcolor"))->GetValue();
    selectedColor[0] = tmpColor[0];
    selectedColor[1] = tmpColor[1];
    selectedColor[2] = tmpColor[2];
    selectedColor[3] = 1.0f;
  }
  else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("selectedcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("selectedcolor"))->GetValue();
    selectedColor[0] = tmpColor[0];
    selectedColor[1] = tmpColor[1];
    selectedColor[2] = tmpColor[2];
    selectedColor[3] = 1.0f;
  }

  //get contour property
  if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("contourcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("contourcolor"))->GetValue();
    contourColor[0] = tmpColor[0];
    contourColor[1] = tmpColor[1];
    contourColor[2] = tmpColor[2];
    contourColor[3] = 1.0f;
  }
  else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("contourcolor")) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("contourcolor"))->GetValue();
    contourColor[0] = tmpColor[0];
    contourColor[1] = tmpColor[1];
    contourColor[2] = tmpColor[2];
    contourColor[3] = 1.0f;
  }

  if(dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("opacity")) != NULL)
  {
    mitk::FloatProperty::Pointer pointOpacity =dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("opacity"));
    opacity = pointOpacity->GetValue();
  }
  else if(dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("opacity")) != NULL)
  {
    mitk::FloatProperty::Pointer pointOpacity =dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("opacity"));
    opacity = pointOpacity->GetValue();
  }
  //finished color / opacity fishing!

  //check if a contour shall be drawn
  bool makeContour = false;
  this->GetDataTreeNode()->GetBoolProperty("contour", makeContour, renderer);
  int visibleBefore = m_ContourActor->GetVisibility();
  if(makeContour && (m_ContourActor != NULL) )
  {
    if ( visibleBefore == 0)//was not visible before, so create it.
      this->CreateContour(renderer);
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
    vtkAppendPolyData* vtkContourPolyData = vtkAppendPolyData::New();
    vtkPolyDataMapper* vtkContourPolyDataMapper = vtkPolyDataMapper::New(); 

    vtkPoints *points = vtkPoints::New();
    vtkCellArray *polys = vtkCellArray::New();
    
    mitk::PointSet::PointsContainer::Iterator pointsIter;
    mitk::PointSet::PointDataContainer::Iterator pointDataIter;
    int j;

    // get and update the PointSet
    mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());
    input->Update();

    mitk::PointSet::DataType::Pointer itkPointSet = input->GetPointSet( m_TimeStep );
    if ( itkPointSet.GetPointer() == NULL) 
    {
      return;
    }

    for (j=0, pointsIter=itkPointSet->GetPoints()->Begin(); pointsIter!=itkPointSet->GetPoints()->End() ; pointsIter++,j++)
    {
      vtkIdType cell[2] = {j-1,j};
      mitk::Point3D point1;
      point1 = input->GetPoint(pointsIter->Index(), m_TimeStep);
      points->InsertPoint(j,point1[0],point1[1],point1[2]);
      if (j>0)
        polys->InsertNextCell(2,cell);
    }

    bool close;
    if (dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetPropertyList()->GetProperty("close"), renderer) == NULL)
      close = false;
    else
      close = dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetPropertyList()->GetProperty("close"), renderer)->GetValue();
    if (close) 
    {
      vtkIdType cell[2] = {j-1,0};
      polys->InsertNextCell(2,cell);
    }

    vtkPolyData* contour = vtkPolyData::New();
    contour->SetPoints(points);
    points->Delete();
    contour->SetLines(polys);
    polys->Delete();
    contour->Update();

    vtkTubeFilter* tubeFilter = vtkTubeFilter::New();
    tubeFilter->SetNumberOfSides( 12 );
    tubeFilter->SetInput(contour);
    contour->Delete();

    //check for property contoursize. If not present, then take pointsize
    ScalarType radius = 0.5;
    mitk::FloatProperty::Pointer contourSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList()->GetProperty("contoursize"), renderer);

    //if no property could be found, then use pointsize
    if (contourSizeProp.IsNull())
      contourSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList()->GetProperty("pointsize"), renderer);

    //take whatever was usefull
    if (contourSizeProp.IsNotNull())
      radius = contourSizeProp->GetValue();

    tubeFilter->SetRadius( radius );
    tubeFilter->Update();

    //add to pipeline
    vtkContourPolyData->AddInput(tubeFilter->GetOutput());
    tubeFilter->Delete();
    vtkContourPolyDataMapper->SetInput(vtkContourPolyData->GetOutput());
    vtkContourPolyData->Delete();

    //create a new instance of the actor
    m_ContourActor->Delete();
    m_ContourActor = vtkActor::New();

    m_ContourActor->SetMapper(vtkContourPolyDataMapper);
    vtkContourPolyDataMapper->Delete();
    
    m_PointsAssembly->AddPart(m_ContourActor);
}

void mitk::PointSetVtkMapper3D::SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "line width", new mitk::IntProperty(2), renderer, overwrite );
  node->AddProperty( "pointsize", new mitk::FloatProperty(1.0), renderer, overwrite);
  node->AddProperty( "selectedcolor", new mitk::ColorProperty(1.0f, 0.0f, 0.0f), renderer, overwrite);  //red
  node->AddProperty( "color", new mitk::ColorProperty(1.0f, 1.0f, 0.0f), renderer, overwrite);  //yellow
  node->AddProperty( "contour", new mitk::BoolProperty(false), renderer, overwrite );
  node->AddProperty( "contourcolor", new mitk::ColorProperty(1.0f, 0.0f, 0.0f), renderer, overwrite);
  node->AddProperty( "contoursize", new mitk::FloatProperty(0.5), renderer, overwrite );
  node->AddProperty( "close", new mitk::BoolProperty(false), renderer, overwrite );
  node->AddProperty( "show points", new mitk::BoolProperty(true), renderer, overwrite );
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
