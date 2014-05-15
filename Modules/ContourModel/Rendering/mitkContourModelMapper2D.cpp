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
#include <mitkContourModelMapper2D.h>

#include <mitkContourModelSubDivisionFilter.h>

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkAppendPolyData.h>
#include <vtkProperty.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkStripper.h>
#include <vtkTubeFilter.h>
#include <vtkSphereSource.h>

#include <mitkPlaneGeometry.h>

mitk::ContourModelMapper2D::ContourModelMapper2D()
{
}


mitk::ContourModelMapper2D::~ContourModelMapper2D()
{
}


const mitk::ContourModel* mitk::ContourModelMapper2D::GetInput( void )
{
  //convient way to get the data from the dataNode
  return static_cast< const mitk::ContourModel * >( GetDataNode()->GetData() );
}


vtkProp* mitk::ContourModelMapper2D::GetVtkProp(mitk::BaseRenderer* renderer)
{
  //return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Actor;
}


void mitk::ContourModelMapper2D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  /*++ convert the contour to vtkPolyData and set it as input for our mapper ++*/

  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  mitk::ContourModel* inputContour  = static_cast< mitk::ContourModel* >( GetDataNode()->GetData() );

  unsigned int timestep = renderer->GetTimeStep();

  //if there's something to be rendered
  if( inputContour->GetNumberOfVertices(timestep) > 0)
  {
    localStorage->m_OutlinePolyData = this->CreateVtkPolyDataFromContour(inputContour, renderer);
  }

  this->ApplyContourProperties(renderer);

  localStorage->m_Mapper->SetInputData(localStorage->m_OutlinePolyData);

}



void mitk::ContourModelMapper2D::Update(mitk::BaseRenderer* renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if ( !visible ) return;


  //check if there is something to be rendered
  mitk::ContourModel* data  = static_cast< mitk::ContourModel*>( GetDataNode()->GetData() );
  if ( data == NULL )
  {
    return;
  }

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep( renderer );

    LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  // Check if time step is valid
    const TimeGeometry *dataTimeGeometry = data->GetTimeGeometry();
  if ( ( dataTimeGeometry == NULL )
    || ( dataTimeGeometry->CountTimeSteps() == 0 )
    || ( !dataTimeGeometry->IsValidTimeStep( renderer->GetTimeStep() ) ) )
  {
    //clear the rendered polydata
    localStorage->m_Mapper->RemoveAllInputs();//SetInput(vtkSmartPointer<vtkPolyData>::New());
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



vtkSmartPointer<vtkPolyData> mitk::ContourModelMapper2D::CreateVtkPolyDataFromContour(mitk::ContourModel* inputContour, mitk::BaseRenderer* renderer)
{
  unsigned int timestep = this->GetTimestep();
  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> resultingPolyData = vtkSmartPointer<vtkPolyData>::New();


  //check for the worldgeometry from the current render window
  mitk::PlaneGeometry* currentWorldGeometry = dynamic_cast<mitk::PlaneGeometry*>( const_cast<mitk::PlaneGeometry*>(renderer->GetCurrentWorldPlaneGeometry()));

  if(currentWorldGeometry)
  {
    //origin and normal of vtkPlane
    mitk::Point3D origin = currentWorldGeometry->GetOrigin();
    mitk::Vector3D normal = currentWorldGeometry->GetNormal();
    //the implicit function to slice through the polyData
    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(origin[0], origin[1], origin[2]);
    plane->SetNormal(normal[0], normal[1], normal[2]);


    /* First of all convert the control points of the contourModel to vtk points
    * and add lines in between them
    */
    //the points to draw
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    //the lines to connect the points
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    // Create a polydata to store everything in
    vtkSmartPointer<vtkPolyData> polyDataIn3D = vtkSmartPointer<vtkPolyData>::New();


    vtkSmartPointer<vtkAppendPolyData> appendPoly = vtkSmartPointer<vtkAppendPolyData>::New();

    mitk::ContourModel::Pointer renderingContour = mitk::ContourModel::New();
    renderingContour = inputContour;



    bool subdivision = false;
    this->GetDataNode()->GetBoolProperty( "subdivision curve", subdivision, renderer );
    if (subdivision)
    {

      mitk::ContourModel::Pointer subdivContour = mitk::ContourModel::New();

      mitk::ContourModelSubDivisionFilter::Pointer subdivFilter = mitk::ContourModelSubDivisionFilter::New();

      subdivFilter->SetInput(inputContour);
      subdivFilter->Update();

      subdivContour = subdivFilter->GetOutput();

      if(subdivContour->GetNumberOfVertices() == 0 )
      {
        subdivContour = inputContour;
      }

      renderingContour = subdivContour;
    }


    //iterate over all control points
    mitk::ContourModel::VertexIterator current = renderingContour->IteratorBegin(timestep);
    mitk::ContourModel::VertexIterator next = renderingContour->IteratorBegin(timestep);
    if(next != renderingContour->IteratorEnd(timestep))
    {
      next++;

      mitk::ContourModel::VertexIterator end = renderingContour->IteratorEnd(timestep);

      while(next != end)
      {
        mitk::ContourModel::VertexType* currentControlPoint = *current;
        mitk::ContourModel::VertexType* nextControlPoint = *next;

        vtkIdType p1 = points->InsertNextPoint(currentControlPoint->Coordinates[0], currentControlPoint->Coordinates[1], currentControlPoint->Coordinates[2]);
        vtkIdType p2 = points->InsertNextPoint(nextControlPoint->Coordinates[0], nextControlPoint->Coordinates[1], nextControlPoint->Coordinates[2]);
        //add the line between both contorlPoints
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);

        if ( currentControlPoint->IsControlPoint )
        {
          double coordinates[3];
          coordinates[0] = currentControlPoint->Coordinates[0];
          coordinates[1] = currentControlPoint->Coordinates[1];
          coordinates[2] = currentControlPoint->Coordinates[2];

          double distance = plane->DistanceToPlane(coordinates);
          if(distance < 0.1)
          {
            vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();

            sphere->SetRadius(1.2);
            sphere->SetCenter(coordinates[0], coordinates[1], coordinates[2]);
            sphere->Update();
            appendPoly->AddInputConnection(sphere->GetOutputPort());
          }
        }


        current++;
        next++;
      }//end while (it!=end)

      //check if last control point is enabled to draw it
      if ( (*current)->IsControlPoint )
      {
        double coordinates[3];
        coordinates[0] = (*current)->Coordinates[0];
        coordinates[1] = (*current)->Coordinates[1];
        coordinates[2] = (*current)->Coordinates[2];

        double distance = plane->DistanceToPlane(coordinates);
        if(distance < 0.1)
        {
          vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();

          sphere->SetRadius(1.2);
          sphere->SetCenter(coordinates[0], coordinates[1], coordinates[2]);
          sphere->Update();
          appendPoly->AddInputConnection(sphere->GetOutputPort());
        }
      }


      /* If the contour is closed an additional line has to be created between the very first point
      * and the last point
      */
      if(renderingContour->IsClosed(timestep))
      {
        //add a line from the last to the first control point
        mitk::ContourModel::VertexType* firstControlPoint = *(renderingContour->IteratorBegin(timestep));
        mitk::ContourModel::VertexType* lastControlPoint = *(--(renderingContour->IteratorEnd(timestep)));
        vtkIdType p2 = points->InsertNextPoint(lastControlPoint->Coordinates[0], lastControlPoint->Coordinates[1], lastControlPoint->Coordinates[2]);
        vtkIdType p1 = points->InsertNextPoint(firstControlPoint->Coordinates[0], firstControlPoint->Coordinates[1], firstControlPoint->Coordinates[2]);

        //add the line between both contorlPoints
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }//end if(isClosed)


      // Add the points to the dataset
      polyDataIn3D->SetPoints(points);
      // Add the lines to the dataset
      polyDataIn3D->SetLines(lines);




      //cut through polyData
      bool useCuttingPlane = false;
      this->GetDataNode()->GetBoolProperty( "use cutting plane", useCuttingPlane, renderer );
      if (useCuttingPlane)
      {
        //slice through the data to get a 2D representation of the (possible) 3D contour

        //needed because currently there is no outher solution if the contour is within the plane
        vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
        tubeFilter->SetInputData(polyDataIn3D);
        tubeFilter->SetRadius(0.05);


        //cuts through vtkPolyData with a given implicit function. In our case a plane
        vtkSmartPointer<vtkCutter> cutter = vtkSmartPointer<vtkCutter>::New();

        cutter->SetCutFunction(plane);

        cutter->SetInputConnection(tubeFilter->GetOutputPort());

        //we want the scalars of the input - so turn off generating the scalars within vtkCutter
        cutter->GenerateCutScalarsOff();
        cutter->Update();


        //set to 2D representation of the contour
        resultingPolyData= cutter->GetOutput();

      }//end if(project contour)
      else
      {
        //set to 3D polyData
        resultingPolyData = polyDataIn3D;
      }

    }//end if (it != end)

    appendPoly->AddInputData(resultingPolyData);
    appendPoly->Update();

    //return contour with control points
    return appendPoly->GetOutput();
  }else
  {
    //return empty polyData
    return resultingPolyData;
  }
}



void mitk::ContourModelMapper2D::ApplyContourProperties(mitk::BaseRenderer* renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  float lineWidth(1.0);
  if (this->GetDataNode()->GetFloatProperty( "width", lineWidth, renderer ))
  {
    localStorage->m_Actor->GetProperty()->SetLineWidth(lineWidth);
  }

  mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty("color", renderer));
  if(colorprop)
  {
    //set the color of the contour
    double red = colorprop->GetColor().GetRed();
    double green = colorprop->GetColor().GetGreen();
    double blue = colorprop->GetColor().GetBlue();
    localStorage->m_Actor->GetProperty()->SetColor(red, green, blue);
  }


  //make sure that directional lighting isn't used for our contour
  localStorage->m_Actor->GetProperty()->SetAmbient(1.0);
  localStorage->m_Actor->GetProperty()->SetDiffuse(0.0);
  localStorage->m_Actor->GetProperty()->SetSpecular(0.0);
}


/*+++++++++++++++++++ LocalStorage part +++++++++++++++++++++++++*/

mitk::ContourModelMapper2D::LocalStorage* mitk::ContourModelMapper2D::GetLocalStorage(mitk::BaseRenderer* renderer)
{
  return m_LSH.GetLocalStorage(renderer);
}


mitk::ContourModelMapper2D::LocalStorage::LocalStorage()
{
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_OutlinePolyData = vtkSmartPointer<vtkPolyData>::New();

  //set the mapper for the actor
  m_Actor->SetMapper(m_Mapper);
}


void mitk::ContourModelMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "color", ColorProperty::New(0.9, 1.0, 0.1), renderer, overwrite );
  node->AddProperty( "width", mitk::FloatProperty::New( 1.0 ), renderer, overwrite );
  node->AddProperty( "use cutting plane", mitk::BoolProperty::New( true ), renderer, overwrite );
  node->AddProperty( "subdivision curve", mitk::BoolProperty::New( false ), renderer, overwrite );

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
