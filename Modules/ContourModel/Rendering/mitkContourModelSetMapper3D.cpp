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
#include <mitkContourModelSetMapper3D.h>

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkProperty.h>
#include "mitkSurface.h"

mitk::ContourModelSetMapper3D::ContourModelSetMapper3D()
{
}


mitk::ContourModelSetMapper3D::~ContourModelSetMapper3D()
{
}


const mitk::ContourModelSet* mitk::ContourModelSetMapper3D::GetInput( void )
{
  //convient way to get the data from the dataNode
  return static_cast< const mitk::ContourModelSet * >( GetDataNode()->GetData() );
}


vtkProp* mitk::ContourModelSetMapper3D::GetVtkProp(mitk::BaseRenderer* renderer)
{
  //return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Assembly;
}


void mitk::ContourModelSetMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  /* First convert the contourModel to vtkPolyData, then tube filter it and
   * set it input for our mapper
   */

  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  mitk::ContourModelSet* contourSet = static_cast< mitk::ContourModelSet* >( GetDataNode()->GetData() );

  mitk::ContourModelSet::ContourModelSetIterator it = contourSet->Begin();
  mitk::ContourModelSet::ContourModelSetIterator end = contourSet->End();

  while(it!=end)
  {
    mitk::ContourModel* inputContour  = it->GetPointer();
    vtkSmartPointer<vtkPolyData> polyData = this->CreateVtkPolyDataFromContour(inputContour, renderer);

    vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
    tubeFilter->SetInputData(polyData);

    float lineWidth(1.0);
    if (this->GetDataNode()->GetFloatProperty( "contour.3D.width", lineWidth, renderer ))
    {
      tubeFilter->SetRadius(lineWidth);
    }else
    {
      tubeFilter->SetRadius(0.5);
    }
    tubeFilter->CappingOn();
    tubeFilter->SetNumberOfSides(10);
    tubeFilter->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    mapper->SetInputConnection(tubeFilter->GetOutputPort());
    //mapper->SetInput(polyData);

    localStorage->m_Assembly->AddPart(actor);

    ++it;
  }


  this->ApplyContourProperties(renderer);

}



void mitk::ContourModelSetMapper3D::Update(mitk::BaseRenderer* renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");


  mitk::ContourModel* data  = static_cast< mitk::ContourModel*>( GetDataNode()->GetData() );
  if ( data == NULL )
  {
    return;
  }

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep( renderer );

  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  if ( this->GetTimestep() == -1 )
  {
    return;
  }

  const DataNode *node = this->GetDataNode();
  data->UpdateOutputInformation();

  //check if something important has changed and we need to rerender
  if ( (localStorage->m_LastUpdateTime < node->GetMTime()) //was the node modified?
    || (localStorage->m_LastUpdateTime < data->GetPipelineMTime()) //Was the data modified?
    || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) //was the geometry modified?
    || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime())
    || (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
    || (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()) )
  {
    this->GenerateDataForRenderer( renderer );
  }

  // since we have checked that nothing important has changed, we can set
  // m_LastUpdateTime to the current time
  localStorage->m_LastUpdateTime.Modified();
}



vtkSmartPointer<vtkPolyData> mitk::ContourModelSetMapper3D::CreateVtkPolyDataFromContour(mitk::ContourModel* inputContour, mitk::BaseRenderer* renderer)
{
  unsigned int timestep = this->GetTimestep();

  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  localStorage->m_contourToPolyData->SetInput(inputContour);
  localStorage->m_contourToPolyData->Update();

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData = localStorage->m_contourToPolyData->GetOutput()->GetVtkPolyData(timestep);

  return polyData;
}



void mitk::ContourModelSetMapper3D::ApplyContourProperties(mitk::BaseRenderer* renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);


  mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty
        ("contour.color", renderer));
  if(colorprop)
  {
    //set the color of the contour
    double red = colorprop->GetColor().GetRed();
    double green = colorprop->GetColor().GetGreen();
    double blue = colorprop->GetColor().GetBlue();


    vtkSmartPointer<vtkPropCollection> collection = vtkSmartPointer<vtkPropCollection>::New();
    localStorage->m_Assembly->GetActors(collection);
    collection->InitTraversal();
    for(vtkIdType i = 0; i < collection->GetNumberOfItems(); i++)
    {
      vtkActor::SafeDownCast(collection->GetNextProp())->GetProperty()->SetColor(red, green, blue);
    }
  }
}


/*+++++++++++++++++++ LocalStorage part +++++++++++++++++++++++++*/

mitk::ContourModelSetMapper3D::LocalStorage* mitk::ContourModelSetMapper3D::GetLocalStorage(mitk::BaseRenderer* renderer)
{
  return m_LSH.GetLocalStorage(renderer);
}


mitk::ContourModelSetMapper3D::LocalStorage::LocalStorage()
{
  m_Assembly = vtkSmartPointer<vtkAssembly>::New();
  m_contourToPolyData = mitk::ContourModelToSurfaceFilter::New();
}


void mitk::ContourModelSetMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "color", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
  node->AddProperty( "contour.3D.width", mitk::FloatProperty::New( 0.5 ), renderer, overwrite );

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
