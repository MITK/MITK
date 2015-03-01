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

#include "mitkTubeGraphVtkMapper3D.h"

#include <mitkColorProperty.h>

#include <vtkCellArray.h>
#include <vtkClipPolyData.h>
#include <vtkContourFilter.h>
#include <vtkCylinder.h>
#include <vtkFloatArray.h>
#include <vtkGeneralTransform.h>
#include <vtkImplicitBoolean.h>
#include <vtkImplicitModeller.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSampleFunction.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>
#include <vtkUnsignedIntArray.h>

mitk::TubeGraphVtkMapper3D::TubeGraphVtkMapper3D()
{
}

mitk::TubeGraphVtkMapper3D::~TubeGraphVtkMapper3D()
{
}

const mitk::TubeGraph* mitk::TubeGraphVtkMapper3D::GetInput()
{
  return dynamic_cast <const TubeGraph*> (GetData());
}

vtkProp* mitk::TubeGraphVtkMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return m_LSH.GetLocalStorage(renderer)->m_vtkTubeGraphAssembly;
}

void mitk::TubeGraphVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
  bool renderTubeGraph(false);
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);


  TubeGraph::Pointer tubeGraph = const_cast<mitk::TubeGraph*>(this->GetInput());
  TubeGraphProperty::Pointer tubeGraphProperty = dynamic_cast<TubeGraphProperty*>(tubeGraph->GetProperty("Tube Graph.Visualization Information").GetPointer());


  if(tubeGraph.IsNull() || tubeGraphProperty.IsNull())
  {
    itkWarningMacro(<< "Input of tube graph mapper is NULL!");
    return;
  }
  //Check if the tube graph has changed; if the data has changed, generate the spheres and tubes new;
  if(tubeGraph->GetMTime() > ls->m_lastGenerateDataTime)
  {
    this->GenerateTubeGraphData(renderer);
    renderTubeGraph = true;
  }
  else
  {
    //Check if the tube graph property has changed; if the property has changed, render the visualization information new;
    if (tubeGraphProperty->GetMTime() > ls->m_lastRenderDataTime)
    {
      this->RenderTubeGraphPropertyInformation(renderer);
      renderTubeGraph = true;
    }
  }

  if (renderTubeGraph)
  {
    std::vector<TubeGraph::VertexDescriptorType> alreadyRenderedVertexList;
    //don't render the sphere which is the root of the graph; so add it to the list before;
    //TODO check both spheres
    TubeGraph::VertexDescriptorType root = tubeGraph->GetRootVertex();
    alreadyRenderedVertexList.push_back(root);


    for (std::map<TubeGraph::TubeDescriptorType, vtkSmartPointer<vtkActor> >::iterator itTubes = ls->m_vtkTubesActorMap.begin(); itTubes != ls->m_vtkTubesActorMap.end(); itTubes++ )
    {
      if(tubeGraphProperty->IsTubeVisible(itTubes->first))
      {
        //add tube actor to assembly
        ls->m_vtkTubeGraphAssembly->AddPart(itTubes->second);

        //render the clipped spheres as end-cups of a tube and connections between tubes
        if(std::find(alreadyRenderedVertexList.begin(), alreadyRenderedVertexList.end(), itTubes->first.first)== alreadyRenderedVertexList.end())
        {
          std::map<TubeGraph::VertexDescriptorType, vtkSmartPointer<vtkActor> >::iterator itSourceSphere = ls->m_vtkSpheresActorMap.find(itTubes->first.first);
          if(itSourceSphere != ls->m_vtkSpheresActorMap.end())
            ls->m_vtkTubeGraphAssembly->AddPart(itSourceSphere->second);
          alreadyRenderedVertexList.push_back(itSourceSphere->first);
        }
        if(std::find(alreadyRenderedVertexList.begin(), alreadyRenderedVertexList.end(), itTubes->first.second)== alreadyRenderedVertexList.end())
        {
          std::map<TubeGraph::VertexDescriptorType, vtkSmartPointer<vtkActor> >::iterator itTargetSphere = ls->m_vtkSpheresActorMap.find(itTubes->first.second);
          if(itTargetSphere != ls->m_vtkSpheresActorMap.end())
            ls->m_vtkTubeGraphAssembly->AddPart(itTargetSphere->second);
          alreadyRenderedVertexList.push_back(itTargetSphere->first);
        }
      }
    }
  }

  //// Opacity TODO
  //{
  //  float opacity = 1.0f;
  //  if( this->GetDataNode()->GetOpacity(opacity,renderer) )
  //    ls->m_vtkTubesActor->GetProperty()->SetOpacity( opacity );
  //}
}

void mitk::TubeGraphVtkMapper3D::RenderTubeGraphPropertyInformation(mitk::BaseRenderer* renderer)
{
  MITK_INFO<< "Render tube graph property information!";
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  TubeGraph::Pointer tubeGraph = const_cast<mitk::TubeGraph*>(this->GetInput());
  TubeGraphProperty::Pointer tubeGraphProperty = dynamic_cast<TubeGraphProperty*>(tubeGraph->GetProperty("Tube Graph.Visualization Information").GetPointer());

  if (tubeGraphProperty.IsNull())
  {
    MITK_INFO << "No tube graph property!! So no special render information...";
    return;
  }

  std::vector<TubeGraphVertex> allVertices = tubeGraph->GetVectorOfAllVertices();
  for(std::vector<TubeGraphVertex>::iterator vertex = allVertices.begin(); vertex != allVertices.end(); ++vertex)
  {
    TubeGraph::VertexDescriptorType vertexDesc = tubeGraph->GetVertexDescriptor(*vertex);

    double sphereColorR = 0;
    double sphereColorG = 0;
    double sphereColorB = 0;

    int numberOfVisibleEdges = 0;
    std::vector<TubeGraphEdge> allEdgesOfVertex = tubeGraph->GetAllEdgesOfAVertex(vertexDesc);
    for(std::vector<TubeGraphEdge>::iterator edge = allEdgesOfVertex.begin(); edge != allEdgesOfVertex.end(); ++edge)
    {
      // get edge descriptor
      EdgeDescriptorType edgeDesc = tubeGraph->GetEdgeDescriptor(*edge);

      // get source and target vertex descriptor
      std::pair<TubeGraphVertex, TubeGraphVertex> soureTargetPair = tubeGraph->GetVerticesOfAnEdge(edgeDesc);
      TubeGraphVertex source = soureTargetPair.first;
      TubeGraphVertex target = soureTargetPair.second;

      // build tube descriptor [sourceId,targetId]
      TubeGraph::TubeDescriptorType tube;
      tube.first = tubeGraph->GetVertexDescriptor(source);
      tube.second = tubeGraph->GetVertexDescriptor(target);

      if(tubeGraphProperty->IsTubeVisible(tube))
      {
        mitk::Color tubeColor = tubeGraphProperty->GetColorOfTube(tube);

        vtkSmartPointer<vtkDataArray> scalars = ls->m_vtkTubesActorMap[tube]->GetMapper()->GetInput()->GetPointData()->GetScalars();
        double color[3];
        scalars->GetTuple(0, color);

        if (color[0] != tubeColor[0] || color[1] != tubeColor[1] || color[2] != tubeColor[2])
        {
          int numberOfPoints = scalars->GetSize();

          vtkSmartPointer<vtkUnsignedCharArray> colorScalars = vtkSmartPointer<vtkUnsignedCharArray>::New();
          colorScalars->SetName("colorScalars");
          colorScalars->SetNumberOfComponents(3);
          colorScalars->SetNumberOfTuples(numberOfPoints);
          for (int i = 0; i < numberOfPoints; i++)
          {
            scalars->InsertTuple3(i, tubeColor[0], tubeColor[1], tubeColor[2]);
          }
          ls->m_vtkTubesActorMap[tube]->GetMapper()->GetInput()->GetPointData()->SetActiveScalars( "colorScalars" );
        }

        sphereColorR += tubeColor[0];
        sphereColorG += tubeColor[1];
        sphereColorB += tubeColor[2];
        numberOfVisibleEdges++;
      }
    }
    if(numberOfVisibleEdges > 0)
    {
      sphereColorR /= 255 * numberOfVisibleEdges;
      sphereColorG /= 255 * numberOfVisibleEdges;
      sphereColorB /= 255 * numberOfVisibleEdges;
    }

    ls->m_vtkSpheresActorMap[vertexDesc]->GetProperty()->SetColor(sphereColorR,sphereColorG, sphereColorB);
  }
  ls->m_lastRenderDataTime.Modified();
}

void mitk::TubeGraphVtkMapper3D::GenerateTubeGraphData(mitk::BaseRenderer* renderer)
{
  MITK_INFO<< "Render tube graph!";
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  ls->m_vtkTubesActorMap.clear();
  ls->m_vtkSpheresActorMap.clear();

  TubeGraph::Pointer tubeGraph = const_cast<mitk::TubeGraph*>(this->GetInput());
  TubeGraphProperty::Pointer tubeGraphProperty = dynamic_cast<TubeGraphProperty*>(tubeGraph->GetProperty("Tube Graph.Visualization Information").GetPointer());
  if (tubeGraphProperty.IsNull())
    MITK_INFO << "No tube graph property!! So no special render information...";

  //render all edges as tubular structures using the vtkTubeFilter
  std::vector<TubeGraphEdge> allEdges = tubeGraph->GetVectorOfAllEdges();
  for(std::vector<TubeGraphEdge>::iterator edge = allEdges.begin(); edge != allEdges.end(); ++edge)
  {
    this->GeneratePolyDataForTube(*edge, tubeGraph, tubeGraphProperty, renderer);
  }

  //Generate all vertices as spheres
  std::vector<TubeGraphVertex> allVertices = tubeGraph->GetVectorOfAllVertices();
  for(std::vector<TubeGraphVertex>::iterator vertex = allVertices.begin(); vertex != allVertices.end(); ++vertex)
  {
    this->GeneratePolyDataForFurcation(*vertex, tubeGraph, renderer);
    if (this->ClipStructures())
    {
      this->ClipPolyData(*vertex, tubeGraph, tubeGraphProperty, renderer);
    }

    ls->m_lastGenerateDataTime.Modified();
  }
}

void mitk::TubeGraphVtkMapper3D::GeneratePolyDataForFurcation(mitk::TubeGraphVertex& vertex, const mitk::TubeGraph::Pointer& graph, mitk::BaseRenderer* renderer)
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);

  mitk::Point3D coordinates;
  float diameter = 2;

  coordinates = (vertex.GetTubeElement())->GetCoordinates();
  if (dynamic_cast<const mitk::CircularProfileTubeElement* >(vertex.GetTubeElement()))
  {
    diameter = (dynamic_cast<const mitk::CircularProfileTubeElement* >(vertex.GetTubeElement()))->GetDiameter();
  }
  vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->SetCenter(coordinates[0], coordinates[1], coordinates[2]);
  sphereSource->SetRadius(diameter / 2.0f);
  sphereSource->SetThetaResolution(12);
  sphereSource->SetPhiResolution(12);
  sphereSource->Update();

  // generate a actor with a mapper for the sphere
  vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();

  sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
  sphereActor->SetMapper(sphereMapper);

  ls->m_vtkSpheresActorMap.insert(std::make_pair(graph->GetVertexDescriptor(vertex), sphereActor));
}

void mitk::TubeGraphVtkMapper3D::GeneratePolyDataForTube(mitk::TubeGraphEdge& edge, const mitk::TubeGraph::Pointer& graph, const mitk::TubeGraphProperty::Pointer& graphProperty, mitk::BaseRenderer* renderer)
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);

  // get edge descriptor
  EdgeDescriptorType edgeDesc = graph->GetEdgeDescriptor(edge);

  // get source and target vertex descriptor
  std::pair<TubeGraphVertex, TubeGraphVertex> soureTargetPair = graph->GetVerticesOfAnEdge(edgeDesc);
  TubeGraphVertex source = soureTargetPair.first;
  TubeGraphVertex target = soureTargetPair.second;

  // build tube descriptor [sourceId,targetId]
  TubeGraph::TubeDescriptorType tube;
  tube.first = graph->GetVertexDescriptor(source);
  tube.second = graph->GetVertexDescriptor(target);


  Color color;
  if(graphProperty.IsNotNull())
  {
    color = graphProperty->GetColorOfTube(tube);
  }
  else
  {
    color[0] = 150;
    color[1] = 150;
    color[2] = 150;
  }

  // add 2 points for the source and target vertices.
  unsigned int numberOfPoints = edge.GetNumberOfElements() + 2;

  // Initialize the required data-structures for building
  // an appropriate input to the tube filter
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(numberOfPoints);

  vtkSmartPointer<vtkFloatArray> radii = vtkSmartPointer<vtkFloatArray>::New();
  radii->SetName("radii");
  radii->SetNumberOfComponents(1);

  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

  vtkSmartPointer<vtkUnsignedCharArray> colorScalars = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colorScalars->SetName("colorScalars");
  colorScalars->SetNumberOfComponents(3);

  // resize the data-arrays
  radii->SetNumberOfTuples(numberOfPoints);
  colorScalars->SetNumberOfTuples(numberOfPoints);
  lines->InsertNextCell(numberOfPoints);

  // Add the positions of the source node, the elements along the edge and
  // the target node as lines to a cell. This cell is used as input
  // for a Tube Filter
  mitk::Point3D coordinates;
  float diameter = 2;
  unsigned int id = 0;

  //Source Node
  coordinates = (source.GetTubeElement())->GetCoordinates();
  if (dynamic_cast<const mitk::CircularProfileTubeElement* >(source.GetTubeElement()))
  {
    diameter = (dynamic_cast<const mitk::CircularProfileTubeElement* >(source.GetTubeElement()))->GetDiameter();
  }
  points->InsertPoint(id, coordinates[0], coordinates[1], coordinates[2]);
  radii->InsertTuple1(id, diameter / 2.0f);

  colorScalars->InsertTuple3(id, color[0], color[1], color[2]);
  lines->InsertCellPoint(id);
  ++id;

  //Iterate along the edge
  std::vector<mitk::TubeElement*> allElements = edge.GetElementVector();
  for(unsigned int index =0; index < edge.GetNumberOfElements(); index++)
  {
    coordinates = allElements[index]->GetCoordinates();
    if (dynamic_cast<mitk::CircularProfileTubeElement* >(allElements[index]))
    {
      diameter = (dynamic_cast< mitk::CircularProfileTubeElement* >(allElements[index]))->GetDiameter();
    }
    points->InsertPoint(id, coordinates[0], coordinates[1], coordinates[2]);
    radii->InsertTuple1(id, diameter / 2.0f);
    colorScalars->InsertTuple3(id, color[0], color[1], color[2]);
    lines->InsertCellPoint(id);
    ++id;
  }

  //Target Node
  coordinates = (target.GetTubeElement())->GetCoordinates();
  if (dynamic_cast<const mitk::CircularProfileTubeElement* >(target.GetTubeElement()))
  {
    diameter = (dynamic_cast<const mitk::CircularProfileTubeElement* >(target.GetTubeElement()))->GetDiameter();
  }
  points->InsertPoint(id, coordinates[0], coordinates[1], coordinates[2]);
  radii->InsertTuple1(id, diameter / 2.0f);
  colorScalars->InsertTuple3(id, color[0], color[1], color[2]);
  lines->InsertCellPoint(id);
  ++id;

  // Initialize poly data from the point set and the cell array
  // (representing topology)
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New( );
  polyData->SetPoints( points );
  polyData->SetLines( lines );
  polyData->GetPointData()->AddArray( radii );
  polyData->GetPointData()->AddArray( colorScalars );
  polyData->GetPointData()->SetActiveScalars( radii->GetName() );

  // Generate a tube  for all lines in the polydata object
  double* range = radii->GetRange( );

  assert( range[0] != 0.0f && range[1] != 0.0f );

  vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  tubeFilter->SetInputData(polyData);
  tubeFilter->SetRadius(range[0]);
  tubeFilter->SetRadiusFactor(range[1] / range[0]);

  if (range[0] != range[1])
    tubeFilter->SetVaryRadiusToVaryRadiusByScalar( );

  tubeFilter->SetNumberOfSides( 9 );
  tubeFilter->SidesShareVerticesOn( );
  tubeFilter->CappingOff();
  tubeFilter->Update();

  tubeFilter->GetOutput()->GetPointData()->SetActiveScalars( "colorScalars" );

  // generate a actor with a mapper for the
  vtkSmartPointer<vtkPolyDataMapper> tubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  vtkSmartPointer<vtkActor> tubeActor = vtkSmartPointer<vtkActor>::New();

  tubeMapper->SetInputConnection(tubeFilter->GetOutputPort());
  tubeActor->SetMapper(tubeMapper);
  tubeActor->GetProperty()->SetColor(color[0], color[1], color[2]);

  ls->m_vtkTubesActorMap.insert(std::pair<TubeGraph::TubeDescriptorType, vtkSmartPointer<vtkActor> >(tube, tubeActor) );
}

void mitk::TubeGraphVtkMapper3D::ClipPolyData(mitk::TubeGraphVertex& vertex, const mitk::TubeGraph::Pointer& graph,const mitk::TubeGraphProperty::Pointer& graphProperty, mitk::BaseRenderer* renderer)
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);

  mitk::Point3D centerVertex = vertex.GetTubeElement()->GetCoordinates();
  float diameter = 2;
  if (dynamic_cast<const mitk::CircularProfileTubeElement* >(vertex.GetTubeElement()))
  {
    diameter = (dynamic_cast<const mitk::CircularProfileTubeElement* >(vertex.GetTubeElement()))->GetDiameter();
  }

  TubeGraph::VertexDescriptorType vertexDesc = graph->GetVertexDescriptor(vertex);

  std::map<TubeGraph::TubeDescriptorType , vtkSmartPointer<vtkImplicitBoolean> > cylinderForClipping;

  //generate for all edges/tubes cylinders. With this structure you can clip the sphere and the other tubes, so that no fragments are shown in the tube.
  std::vector<TubeGraphEdge> allEdgesOfVertex = graph->GetAllEdgesOfAVertex(vertexDesc);
  for(std::vector<TubeGraphEdge>::iterator edge = allEdgesOfVertex.begin(); edge != allEdgesOfVertex.end(); ++edge)
  {
    // get edge descriptor
    EdgeDescriptorType edgeDesc = graph->GetEdgeDescriptor(*edge);

    // get source and target vertex descriptor
    std::pair<TubeGraphVertex, TubeGraphVertex> soureTargetPair = graph->GetVerticesOfAnEdge(edgeDesc);
    TubeGraphVertex source = soureTargetPair.first;
    TubeGraphVertex target = soureTargetPair.second;

    // build tube descriptor [sourceId,targetId]
    TubeGraph::TubeDescriptorType tube;
    tube.first = graph->GetVertexDescriptor(source);
    tube.second = graph->GetVertexDescriptor(target);

    //get reference point in the tube for the direction
    mitk::Point3D edgeDirectionPoint;
    //get reference diameter
    double cylinderDiameter = diameter;
    float radius = diameter/2;
    //if the vertex is the source vertex of the edge get the first element of elementVector; otherwise get the last element.
    if(source == vertex)
    {
      //if the edge has no element get the other vertex
      if ((*edge).GetNumberOfElements() != 0)
      {
        double lastDistance = 0, distance = 0;

        unsigned int index = 0;
        //Get the first element behind the radius of the sphere
        for (; index < (*edge).GetNumberOfElements(); index ++)
        {
          mitk::Vector3D diffVec = (*edge).GetTubeElement(index)->GetCoordinates() - centerVertex;
          distance = std::sqrt(pow(diffVec[0],2) + pow(diffVec[1],2) + pow(diffVec[2],2));
          if (distance > radius)
            break;
          lastDistance = distance;
        }
        //if the last element is not inside the sphere
        if(index < (*edge).GetNumberOfElements())
        {
          double withinSphereDiameter = diameter, outsideSphereDiameter = diameter, interpolationValue = 0.5;

          interpolationValue = (radius - lastDistance) / (distance - lastDistance);
          //if first element is outside of the sphere use sphere diameter and the element diameter for interpolation
          if(index == 0)
          {
            if (dynamic_cast<mitk::CircularProfileTubeElement* >((*edge).GetTubeElement(0)))
              outsideSphereDiameter = (dynamic_cast< mitk::CircularProfileTubeElement* >((*edge).GetTubeElement(0)))->GetDiameter();
          }
          else
          {
            if (dynamic_cast<mitk::CircularProfileTubeElement* >((*edge).GetTubeElement(index-1)))
              withinSphereDiameter = (dynamic_cast< mitk::CircularProfileTubeElement* >((*edge).GetTubeElement(index-1)))->GetDiameter();

            if (dynamic_cast<mitk::CircularProfileTubeElement* >((*edge).GetTubeElement(index)))
              outsideSphereDiameter = (dynamic_cast< mitk::CircularProfileTubeElement* >((*edge).GetTubeElement(index)))->GetDiameter();
          }
          // interpolate the diameter for clipping
          cylinderDiameter = (1 - interpolationValue) * withinSphereDiameter + interpolationValue * outsideSphereDiameter;
        }
        //Get the reference point, so the direction of the tube can be calculated
        edgeDirectionPoint = (*edge).GetTubeElement(0)->GetCoordinates();
      }
      else
      {
        //Get the reference point, so the direction of the tube can be calculated
        edgeDirectionPoint = target.GetTubeElement()->GetCoordinates();
      }
    }

    //if vertex is target of the tube
    else
    {
      //if the edge has no element, get the other vertex
      if ((*edge).GetNumberOfElements() != 0)
      {
        double lastDistance = 0, distance = 0;
        //Get the first element behind the radius of the sphere; now backwards through the element list
        unsigned int index = (*edge).GetNumberOfElements()-1;
        for (; index >= 0; index --)
        {
          mitk::Vector3D diffVec = (*edge).GetTubeElement(index)->GetCoordinates() - centerVertex;
          distance = std::sqrt(pow(diffVec[0],2) + pow(diffVec[1],2) + pow(diffVec[2],2));
          if (distance > radius)
            break;
          lastDistance = distance;
        }

        if(index >= 0)
        {
          double withinSphereDiameter = diameter, outsideSphereDiameter = diameter, interpolationValue = 0.5;

          interpolationValue = (radius-lastDistance) / (distance-lastDistance);

          if(index == (*edge).GetNumberOfElements()-1)
          {
            if (dynamic_cast<mitk::CircularProfileTubeElement* >((*edge).GetTubeElement((*edge).GetNumberOfElements()-1)))
              outsideSphereDiameter = (dynamic_cast< mitk::CircularProfileTubeElement* >((*edge).GetTubeElement((*edge).GetNumberOfElements()-1)))->GetDiameter();
          }
          else
          {
            if (dynamic_cast<mitk::CircularProfileTubeElement* >((*edge).GetTubeElement(index+1)))
              withinSphereDiameter = (dynamic_cast< mitk::CircularProfileTubeElement* >((*edge).GetTubeElement(index+1)))->GetDiameter();

            if (dynamic_cast<mitk::CircularProfileTubeElement* >((*edge).GetTubeElement(index)))
              outsideSphereDiameter = (dynamic_cast< mitk::CircularProfileTubeElement* >((*edge).GetTubeElement(index)))->GetDiameter();
          }
          // interpolate the diameter for clipping
          cylinderDiameter = (1 - interpolationValue) * withinSphereDiameter + interpolationValue * outsideSphereDiameter;
        }

        //Get the reference point, so the direction of the tube can be calculated
        edgeDirectionPoint = (*edge).GetTubeElement((*edge).GetNumberOfElements()-1)->GetCoordinates();
      }
      else
      {
        //Get the reference point, so the direction of the tube can be calculated
        edgeDirectionPoint = source.GetTubeElement()->GetCoordinates();
      }
    }

    //////Calculate the matrix for rotation and translation////
    //get the normalized vector for the orientation (tube element direction)
    mitk::Vector3D vecOrientation;
    mitk::FillVector3D(vecOrientation,(edgeDirectionPoint[0]-centerVertex[0]),(edgeDirectionPoint[1]-centerVertex[1]),(edgeDirectionPoint[2]-centerVertex[2]));
    vecOrientation.Normalize();

    //generate a random vector
    mitk::Vector3D vecRandom;
    mitk::FillVector3D (vecRandom, (rand()%100-50) , (rand()%100-50), (rand()%100-50));
    //project the random vector on the plane-->orthogonal vector to plane normal; normalize it!
    mitk::Vector3D vecOrthoToOrientation;
    vecOrthoToOrientation = vecRandom - (vecRandom*vecOrientation)* vecOrientation;
    vecOrthoToOrientation.Normalize();

    //get the cross product of both orthogonale vectors to get a third one
    mitk::Vector3D vecCrossProduct;
    vecCrossProduct = itk::CrossProduct(vecOrientation, vecOrthoToOrientation);
    vecCrossProduct.Normalize();

    //Fill matrix
    vtkSmartPointer<vtkMatrix4x4> vtkTransformMatrix = vtkSmartPointer<vtkMatrix4x4> ::New();
    vtkTransformMatrix->Identity();
    //1. column
    vtkTransformMatrix->SetElement(0,0,vecOrthoToOrientation[0]);
    vtkTransformMatrix->SetElement(1,0,vecOrthoToOrientation[1]);
    vtkTransformMatrix->SetElement(2,0,vecOrthoToOrientation[2]);
    //2. column
    vtkTransformMatrix->SetElement(0,1,vecOrientation[0]);
    vtkTransformMatrix->SetElement(1,1,vecOrientation[1]);
    vtkTransformMatrix->SetElement(2,1,vecOrientation[2]);
    //3. column
    vtkTransformMatrix->SetElement(0,2,vecCrossProduct[0]);
    vtkTransformMatrix->SetElement(1,2,vecCrossProduct[1]);
    vtkTransformMatrix->SetElement(2,2,vecCrossProduct[2]);
    //4. column
    vtkTransformMatrix->SetElement(0,3,centerVertex[0]);
    vtkTransformMatrix->SetElement(1,3,centerVertex[1]);
    vtkTransformMatrix->SetElement(2,3,centerVertex[2]);

    vtkSmartPointer<vtkGeneralTransform> transform = vtkSmartPointer<vtkGeneralTransform>::New();
    transform->Concatenate(vtkTransformMatrix);
    //transform->Translate(centerVertex[0],centerVertex[1],centerVertex[2]);

    transform->Inverse();
    transform->Update();

    //Generate plane in center [0,0,0] with n= (0,1,0) as normal vector
    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(0, 0, 0);
    plane->SetNormal(0, 1, 0);

    //Generate a cylinder in center [0,0,0] and the axes of rotation is along the y-axis; radius is vertex diameter/2;
    vtkSmartPointer<vtkCylinder> cylinder = vtkSmartPointer<vtkCylinder>::New();
    cylinder->SetCenter(0,0,0);
    cylinder->SetRadius(cylinderDiameter/2);
    //cylinder->SetTransform(transform);

    // Truncate the infinite cylinder with the plane
    vtkSmartPointer<vtkImplicitBoolean> cutCylinder = vtkSmartPointer<vtkImplicitBoolean>::New();
    cutCylinder->SetOperationTypeToDifference();
    cutCylinder->SetTransform(transform);
    cutCylinder->AddFunction(cylinder);
    cutCylinder->AddFunction(plane);


    cylinderForClipping.insert(std::pair<TubeGraph::TubeDescriptorType , vtkSmartPointer<vtkImplicitBoolean> >(tube , cutCylinder));

    //// Sample the function
    //vtkSmartPointer<vtkSampleFunction> sample = vtkSmartPointer<vtkSampleFunction>::New();
    //sample->SetSampleDimensions(100,100,100);
    //sample->SetImplicitFunction(cutCylinder);
    ////double value = 200.0;
    //double xmin = centerVertex[0]-(2*diameter), xmax = centerVertex[0]+(2*diameter),
    //  ymin =  centerVertex[1]-(2*diameter), ymax = centerVertex[1]+(2*diameter),
    //  zmin =  centerVertex[2]-(2*diameter), zmax = centerVertex[2]+(2*diameter);
    //sample->SetModelBounds(xmin, xmax, ymin, ymax, zmin, zmax);

    //vtkSmartPointer<vtkContourFilter> contour =vtkSmartPointer<vtkContourFilter>::New();
    //contour->SetInputConnection(sample->GetOutputPort());
    //contour->SetValue( 0, 0.25);

    //vtkSmartPointer<vtkPolyDataMapper> impMapper =  vtkSmartPointer<vtkPolyDataMapper>::New();
    //impMapper->SetInputConnection (contour->GetOutputPort());
    //impMapper->ScalarVisibilityOff();
    //vtkSmartPointer<vtkActor> impActor = vtkSmartPointer<vtkActor>::New();
    //impActor->SetMapper(impMapper);

    //ls->m_vtkTubeGraphAssembly->AddPart(impActor);
  }

  double sphereColorR = 0;
  double sphereColorG = 0;
  double sphereColorB = 0;

  for (std::map<TubeGraph::TubeDescriptorType ,vtkSmartPointer<vtkImplicitBoolean> >::iterator itClipStructure = cylinderForClipping.begin(); itClipStructure != cylinderForClipping.end(); itClipStructure++)
  {
    vtkSmartPointer<vtkPolyDataMapper> sphereMapper = dynamic_cast<vtkPolyDataMapper*>(ls->m_vtkSpheresActorMap[vertexDesc]->GetMapper());

    if(sphereMapper != NULL)
    {
      //first clip the sphere with the cylinder
      vtkSmartPointer<vtkClipPolyData> clipperSphere = vtkSmartPointer<vtkClipPolyData>::New();
      clipperSphere->SetInputData(sphereMapper->GetInput());
      clipperSphere->SetClipFunction(itClipStructure->second);
      clipperSphere->GenerateClippedOutputOn();
      clipperSphere->Update();

      sphereMapper->SetInputConnection(clipperSphere->GetOutputPort()) ;
      sphereMapper->Update();
    }

    mitk::Color tubeColor = graphProperty->GetColorOfTube(itClipStructure->first);
    sphereColorR += tubeColor[0];
    sphereColorG += tubeColor[1];
    sphereColorB += tubeColor[2];

    //than clip with all other tubes
    for (std::map<TubeGraph::TubeDescriptorType , vtkSmartPointer<vtkImplicitBoolean> >::iterator itTobBeClipped = cylinderForClipping.begin(); itTobBeClipped != cylinderForClipping.end(); itTobBeClipped++)
    {
      TubeGraph::TubeDescriptorType toBeClippedTube = itTobBeClipped->first;

      if (itClipStructure->first != toBeClippedTube)
      {

        vtkSmartPointer<vtkPolyDataMapper> tubeMapper = dynamic_cast<vtkPolyDataMapper*>(ls->m_vtkTubesActorMap[toBeClippedTube]->GetMapper());

        if (tubeMapper != NULL)
        {
          //first clip the sphere with the cylinder
          vtkSmartPointer<vtkClipPolyData> clipperTube = vtkSmartPointer<vtkClipPolyData>::New();
          tubeMapper->Update();
          clipperTube->SetInputData(tubeMapper->GetInput());
          clipperTube->SetClipFunction(itClipStructure->second);
          clipperTube->GenerateClippedOutputOn();
          clipperTube->Update();

          tubeMapper->SetInputConnection(clipperTube->GetOutputPort()) ;
          tubeMapper->Update();
        }
      }
    }
  }
  if(cylinderForClipping.size() != 0)
  {
    sphereColorR /= 255 * cylinderForClipping.size();
    sphereColorG /= 255 * cylinderForClipping.size();
    sphereColorB /= 255 * cylinderForClipping.size();
  }

  ls->m_vtkSpheresActorMap[vertexDesc]->GetProperty()->SetColor(sphereColorR,sphereColorG, sphereColorB);
}

bool mitk::TubeGraphVtkMapper3D::ClipStructures()
{
  DataNode::Pointer node = this->GetDataNode();
  if ( node.IsNull() )
  {
    itkWarningMacro( << "associated node is NULL!" );
    return false;
  }

  bool clipStructures = false;
  node->GetBoolProperty( "Tube Graph.Clip Structures", clipStructures );

  return clipStructures;
}
