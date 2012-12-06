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

#include "mitkConnectomicsNetworkMapper3D.h"

#include <vtkMutableUndirectedGraph.h>
#include "vtkGraphLayout.h"
#include <vtkPoints.h>
#include "vtkGraphToPolyData.h"
#include <vtkPassThroughLayoutStrategy.h>
#include "vtkGlyph3D.h"
#include "vtkGlyphSource2D.h"

#include "mitkConnectomicsRenderingProperties.h"

mitk::ConnectomicsNetworkMapper3D::ConnectomicsNetworkMapper3D()
{
  //TODO: implement
  m_NetworkAssembly = vtkPropAssembly::New();

}

mitk::ConnectomicsNetworkMapper3D:: ~ConnectomicsNetworkMapper3D()
{
  //TODO: implement
  m_NetworkAssembly->Delete();

}

void mitk::ConnectomicsNetworkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
  //TODO: implement

  if( this->GetInput() == NULL )
  {
    return;
  }
  if( this->GetInput()->GetIsModified( ) )
  {
    GenerateData();
  }
}

void mitk::ConnectomicsNetworkMapper3D::GenerateData()
{
  m_NetworkAssembly->Delete();
  m_NetworkAssembly = vtkPropAssembly::New();

  // Here is the part where a graph is given and converted to points and connections between points...
  std::vector< mitk::ConnectomicsNetwork::NetworkNode > vectorOfNodes = this->GetInput()->GetVectorOfAllNodes();
  std::vector< std::pair<
    std::pair< mitk::ConnectomicsNetwork::NetworkNode, mitk::ConnectomicsNetwork::NetworkNode >
    , mitk::ConnectomicsNetwork::NetworkEdge > >  vectorOfEdges = this->GetInput()->GetVectorOfAllEdges();

  // Decide on the style of rendering due to property
  if( false )
  {
    mitk::Point3D tempWorldPoint, tempCNFGeometryPoint;

    //////////////////////Create Spheres/////////////////////////
    for(unsigned int i = 0; i < vectorOfNodes.size(); i++)
    {
      vtkSmartPointer<vtkSphereSource> sphereSource =
        vtkSmartPointer<vtkSphereSource>::New();

      for(unsigned int dimension = 0; dimension < 3; dimension++)
      {
        tempCNFGeometryPoint.SetElement( dimension , vectorOfNodes[i].coordinates[dimension] );
      }

      this->GetData()->GetGeometry()->IndexToWorld( tempCNFGeometryPoint, tempWorldPoint );

      sphereSource->SetCenter( tempWorldPoint[0] , tempWorldPoint[1], tempWorldPoint[2] );
      sphereSource->SetRadius(1.0);

      vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInput(sphereSource->GetOutput());

      vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper);

      m_NetworkAssembly->AddPart(actor);
    }

    //////////////////////Create Tubes/////////////////////////

    for(unsigned int i = 0; i < vectorOfEdges.size(); i++)
    {

      vtkSmartPointer<vtkLineSource> lineSource =
        vtkSmartPointer<vtkLineSource>::New();

      for(unsigned int dimension = 0; dimension < 3; dimension++)
      {
        tempCNFGeometryPoint[ dimension ] = vectorOfEdges[i].first.first.coordinates[dimension];
      }

      this->GetData()->GetGeometry()->IndexToWorld( tempCNFGeometryPoint, tempWorldPoint );

      lineSource->SetPoint1(tempWorldPoint[0], tempWorldPoint[1],tempWorldPoint[2]  );

      for(unsigned int dimension = 0; dimension < 3; dimension++)
      {
        tempCNFGeometryPoint[ dimension ] = vectorOfEdges[i].first.second.coordinates[dimension];
      }

      this->GetData()->GetGeometry()->IndexToWorld( tempCNFGeometryPoint, tempWorldPoint );

      lineSource->SetPoint2(tempWorldPoint[0], tempWorldPoint[1], tempWorldPoint[2] );

      vtkSmartPointer<vtkTubeFilter> tubes = vtkSmartPointer<vtkTubeFilter>::New();
      tubes->SetInput( lineSource->GetOutput() );
      tubes->SetNumberOfSides( 12 );
      double radiusFactor = 1.0 + ((double) vectorOfEdges[i].second.weight) / 10.0 ;
      tubes->SetRadius( std::log10( radiusFactor ) );

      vtkSmartPointer<vtkPolyDataMapper> mapper2 =
        vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper2->SetInput( tubes->GetOutput() );

      vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper2);

      double maxWeight = (double) this->GetInput()->GetMaximumWeight();
      double colourFactor = vectorOfEdges[i].second.weight / maxWeight;
      actor->GetProperty()->SetColor( colourFactor, colourFactor, colourFactor);


      m_NetworkAssembly->AddPart(actor);

    }
  }
  else
  {
    vtkSmartPointer<vtkMutableUndirectedGraph> graph =
      vtkSmartPointer<vtkMutableUndirectedGraph>::New();

    std::vector< vtkIdType > networkToVTKvector;
    networkToVTKvector.resize(vectorOfNodes.size());

    for(unsigned int i = 0; i < vectorOfNodes.size(); i++)
    {
      networkToVTKvector[vectorOfNodes[i].id] = graph->AddVertex();
    }

    for(unsigned int i = 0; i < vectorOfEdges.size(); i++)
    {
      graph->AddEdge(networkToVTKvector[vectorOfEdges[i].first.first.id], networkToVTKvector[vectorOfEdges[i].first.second.id]);
    }

    vtkSmartPointer<vtkPoints> points =
      vtkSmartPointer<vtkPoints>::New();
    for(unsigned int i = 0; i < vectorOfNodes.size(); i++)
    {
      double x = vectorOfNodes[i].coordinates[0];
      double y = vectorOfNodes[i].coordinates[1];
      double z = vectorOfNodes[i].coordinates[2];
      points->InsertNextPoint( x, y, z);
    }

    graph->SetPoints(points);

    vtkGraphLayout* layout = vtkGraphLayout::New();
    layout->SetInput(graph);
    layout->SetLayoutStrategy(vtkPassThroughLayoutStrategy::New());

    vtkGraphToPolyData* graphToPoly = vtkGraphToPolyData::New();
    graphToPoly->SetInputConnection(layout->GetOutputPort());

    // Create the standard VTK polydata mapper and actor
    // for the connections (edges) in the tree.
    vtkPolyDataMapper* edgeMapper = vtkPolyDataMapper::New();
    edgeMapper->SetInputConnection(graphToPoly->GetOutputPort());
    vtkActor* edgeActor = vtkActor::New();
    edgeActor->SetMapper(edgeMapper);
    edgeActor->GetProperty()->SetColor(0.0, 0.5, 1.0);

    // Glyph the points of the tree polydata to create
    // VTK_VERTEX cells at each vertex in the tree.
    vtkGlyph3D* vertGlyph = vtkGlyph3D::New();
    vertGlyph->SetInputConnection(0, graphToPoly->GetOutputPort());
    vtkGlyphSource2D* glyphSource = vtkGlyphSource2D::New();
    glyphSource->SetGlyphTypeToVertex();
    vertGlyph->SetInputConnection(1, glyphSource->GetOutputPort());

    // Create a mapper for the vertices, and tell the mapper
    // to use the specified color array.
    vtkPolyDataMapper* vertMapper = vtkPolyDataMapper::New();
    vertMapper->SetInputConnection(vertGlyph->GetOutputPort());
    /*if (colorArray)
    {
    vertMapper->SetScalarModeToUsePointFieldData();
    vertMapper->SelectColorArray(colorArray);
    vertMapper->SetScalarRange(colorRange);
    }*/

    // Create an actor for the vertices.  Move the actor forward
    // in the z direction so it is drawn on top of the edge actor.
    vtkActor* vertActor = vtkActor::New();
    vertActor->SetMapper(vertMapper);
    vertActor->GetProperty()->SetPointSize(5);
    vertActor->SetPosition(0, 0, 0.001);

    m_NetworkAssembly->AddPart(edgeActor);
    m_NetworkAssembly->AddPart(vertActor);
  }

  (static_cast<mitk::ConnectomicsNetwork * > ( GetData() ) )->SetIsModified( false );
}

const mitk::ConnectomicsNetwork* mitk::ConnectomicsNetworkMapper3D::GetInput()
{

  return static_cast<const mitk::ConnectomicsNetwork * > ( GetData() );
}


void mitk::ConnectomicsNetworkMapper3D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer , bool overwrite)
{
  // Initialize enumeration properties



  // set the properties
  node->AddProperty( connectomicsRenderingSchemePropertyName.c_str(),
    connectomicsRenderingSchemeProperty, renderer, overwrite );

  node->AddProperty( connectomicsRenderingEdgeFilteringPropertyName.c_str(),
    connectomicsRenderingEdgeFilteringProperty, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeThresholdFilterParameterName.c_str(),
    connectomicsRenderingEdgeThresholdFilterParameterDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeThresholdFilterThresholdName.c_str(),
    connectomicsRenderingEdgeThresholdFilterThresholdDefault, renderer, overwrite );

  node->AddProperty( connectomicsRenderingNodeFilteringPropertyName.c_str(),
    connectomicsRenderingNodeFilteringProperty, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeThresholdFilterParameterName.c_str(),
    connectomicsRenderingNodeThresholdFilterParameterDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeThresholdFilterThresholdName.c_str(),
    connectomicsRenderingNodeThresholdFilterThresholdDefault, renderer, overwrite );

  node->AddProperty( connectomicsRenderingNodeColoringSchemeName.c_str(),
    connectomicsRenderingNodeColoringSchemeProperty, renderer, overwrite );

  node->AddProperty( connectomicsRenderingNodeGradientStartColorName.c_str(),
    connectomicsRenderingNodeGradientStartColorDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeGradientEndColorName.c_str(),
    connectomicsRenderingNodeGradientEndColorDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeGradientColorParameterName.c_str(),
    connectomicsRenderingNodeGradientColorParameterDefault, renderer, overwrite );

  node->AddProperty( connectomicsRenderingNodeRadiusStartName.c_str(),
    connectomicsRenderingNodeRadiusStartDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeRadiusEndName.c_str(),
    connectomicsRenderingNodeRadiusEndDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeRadiusParameterName.c_str(),
    connectomicsRenderingNodeRadiusParameterDefault, renderer, overwrite );

  node->AddProperty( connectomicsRenderingNodeChosenNodeName.c_str(),
    connectomicsRenderingNodeChosenNodeDefault, renderer, overwrite );

  node->AddProperty( connectomicsRenderingEdgeGradientStartColorName.c_str(),
    connectomicsRenderingEdgeGradientStartColorDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeGradientEndColorName.c_str(),
    connectomicsRenderingEdgeGradientEndColorDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeGradientColorParameterName.c_str(),
    connectomicsRenderingEdgeGradientColorParameterDefault, renderer, overwrite );

  node->AddProperty( connectomicsRenderingEdgeRadiusStartName.c_str(),
    connectomicsRenderingEdgeRadiusStartDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeRadiusEndName.c_str(),
    connectomicsRenderingEdgeRadiusEndDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeRadiusParameterName.c_str(),
    connectomicsRenderingEdgeRadiusParameterDefault, renderer, overwrite );

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

void mitk::ConnectomicsNetworkMapper3D::ApplyProperties(mitk::BaseRenderer* renderer)
{
  //TODO: implement

}

void mitk::ConnectomicsNetworkMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *mapper)
{
  //TODO: implement

}

void mitk::ConnectomicsNetworkMapper3D::UpdateVtkObjects()
{
  //TODO: implement
}

vtkProp* mitk::ConnectomicsNetworkMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{

  return m_NetworkAssembly;

}
