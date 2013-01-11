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
#include "mitkConnectomicsRenderingSchemeProperty.h"
#include "mitkConnectomicsRenderingEdgeFilteringProperty.h"
#include "mitkConnectomicsRenderingNodeFilteringProperty.h"
#include "mitkConnectomicsRenderingNodeColorParameterProperty.h"
#include "mitkConnectomicsRenderingNodeRadiusParameterProperty.h"
#include "mitkConnectomicsRenderingEdgeColorParameterProperty.h"
#include "mitkConnectomicsRenderingEdgeRadiusParameterProperty.h"
#include "mitkConnectomicsRenderingNodeThresholdParameterProperty.h"
#include "mitkConnectomicsRenderingEdgeThresholdParameterProperty.h"

#include <algorithm>

mitk::ConnectomicsNetworkMapper3D::ConnectomicsNetworkMapper3D()
{
  m_NetworkAssembly = vtkPropAssembly::New();

}

mitk::ConnectomicsNetworkMapper3D:: ~ConnectomicsNetworkMapper3D()
{
  m_NetworkAssembly->Delete();

}

void mitk::ConnectomicsNetworkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
  if( this->GetInput() == NULL )
  {
    return;
  }

  bool propertiesHaveChanged = this->PropertiesChanged();

  if( this->GetInput()->GetIsModified( ) || propertiesHaveChanged )
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

  if( m_ChosenRenderingScheme == connectomicsRenderingMITKScheme )
  {
    mitk::Point3D tempWorldPoint, tempCNFGeometryPoint;

    //////////////////////Prepare coloring and radius////////////

    std::vector< double > vectorOfNodeRadiusParameterValues;
    vectorOfNodeRadiusParameterValues.resize( vectorOfNodes.size() );
    double maxNodeRadiusParameterValue( FillNodeParameterVector( &vectorOfNodeRadiusParameterValues, m_NodeRadiusParameter ) );

    std::vector< double > vectorOfNodeColorParameterValues;
    vectorOfNodeColorParameterValues.resize( vectorOfNodes.size() );
    double maxNodeColorParameterValue( FillNodeParameterVector( &vectorOfNodeColorParameterValues, m_NodeColorParameter ) );

    std::vector< double > vectorOfEdgeRadiusParameterValues;
    vectorOfEdgeRadiusParameterValues.resize( vectorOfEdges.size() );
    double maxEdgeRadiusParameterValue( FillEdgeParameterVector( &vectorOfEdgeRadiusParameterValues, m_EdgeRadiusParameter ) );

    std::vector< double > vectorOfEdgeColorParameterValues;
    vectorOfEdgeColorParameterValues.resize( vectorOfEdges.size() );
    double maxEdgeColorParameterValue( FillEdgeParameterVector( &vectorOfEdgeColorParameterValues, m_EdgeColorParameter ) );

    //////////////////////Prepare Filtering//////////////////////
    // true will be rendered
    std::vector< bool > vectorOfNodeFilterBools( vectorOfNodes.size(), true );
    if( m_ChosenNodeFilter == connectomicsRenderingNodeThresholdingFilter )
    {
      FillNodeFilterBoolVector( &vectorOfNodeFilterBools, m_NodeThresholdParameter );
    }

    std::vector< bool > vectorOfEdgeFilterBools( vectorOfEdges.size(), true );
    if( m_ChosenEdgeFilter == connectomicsRenderingEdgeThresholdFilter )
    {
      FillEdgeFilterBoolVector( &vectorOfEdgeFilterBools, m_EdgeThresholdParameter );
    }

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

      // determine radius
      double radiusFactor = vectorOfNodeRadiusParameterValues[i] / maxNodeRadiusParameterValue;

      double radius = m_NodeRadiusStart + ( m_NodeRadiusEnd - m_NodeRadiusStart) * radiusFactor;
      sphereSource->SetRadius( radius );

      vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInput(sphereSource->GetOutput());

      vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper);

      // determine color
      double colorFactor = vectorOfNodeColorParameterValues[i] / maxNodeColorParameterValue;

      double redStart = m_NodeColorStart.GetElement( 0 );
      double greenStart = m_NodeColorStart.GetElement( 1 );
      double blueStart = m_NodeColorStart.GetElement( 2 );
      double redEnd = m_NodeColorEnd.GetElement( 0 );
      double greenEnd = m_NodeColorEnd.GetElement( 1 );
      double blueEnd = m_NodeColorEnd.GetElement( 2 );

      double red = redStart + ( redEnd - redStart ) * colorFactor;
      double green = greenStart + ( greenEnd - greenStart ) * colorFactor;
      double blue = blueStart + ( blueEnd - blueStart ) * colorFactor;

      actor->GetProperty()->SetColor( red, green, blue);

      if( vectorOfNodeFilterBools[i] )
      {
        m_NetworkAssembly->AddPart(actor);
      }
    }

    //////////////////////Create Tubes/////////////////////////

    double maxWeight = (double) this->GetInput()->GetMaximumWeight();

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

      // determine radius
      double radiusFactor = vectorOfEdgeRadiusParameterValues[i] / maxEdgeRadiusParameterValue;

      double radius = m_EdgeRadiusStart + ( m_EdgeRadiusEnd - m_EdgeRadiusStart) * radiusFactor;
      tubes->SetRadius( radius );

      // originally we used a logarithmic scaling,
      // double radiusFactor = 1.0 + ((double) vectorOfEdges[i].second.weight) / 10.0 ;
      // tubes->SetRadius( std::log10( radiusFactor ) );

      vtkSmartPointer<vtkPolyDataMapper> mapper2 =
        vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper2->SetInput( tubes->GetOutput() );

      vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper2);

      // determine color
      double colorFactor = vectorOfEdgeColorParameterValues[i] / maxEdgeColorParameterValue;

      double redStart = m_EdgeColorStart.GetElement( 0 );
      double greenStart = m_EdgeColorStart.GetElement( 1 );
      double blueStart = m_EdgeColorStart.GetElement( 2 );
      double redEnd = m_EdgeColorEnd.GetElement( 0 );
      double greenEnd = m_EdgeColorEnd.GetElement( 1 );
      double blueEnd = m_EdgeColorEnd.GetElement( 2 );

      double red = redStart + ( redEnd - redStart ) * colorFactor;
      double green = greenStart + ( greenEnd - greenStart ) * colorFactor;
      double blue = blueStart + ( blueEnd - blueStart ) * colorFactor;

      actor->GetProperty()->SetColor( red, green, blue);

      if( vectorOfEdgeFilterBools[i] )
      {
        m_NetworkAssembly->AddPart(actor);
      }
    }
  }
  else if( m_ChosenRenderingScheme == connectomicsRenderingVTKScheme )
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

  mitk::ConnectomicsRenderingSchemeProperty::Pointer connectomicsRenderingScheme =
    mitk::ConnectomicsRenderingSchemeProperty::New();
  mitk::ConnectomicsRenderingEdgeFilteringProperty::Pointer connectomicsRenderingEdgeFiltering =
    mitk::ConnectomicsRenderingEdgeFilteringProperty::New();
  mitk::ConnectomicsRenderingNodeFilteringProperty::Pointer connectomicsRenderingNodeFiltering =
     mitk::ConnectomicsRenderingNodeFilteringProperty::New();

   mitk::ConnectomicsRenderingNodeColorParameterProperty::Pointer  connectomicsRenderingNodeGradientColorParameter =
    mitk::ConnectomicsRenderingNodeColorParameterProperty::New();
  mitk::ConnectomicsRenderingNodeRadiusParameterProperty::Pointer connectomicsRenderingNodeRadiusParameter =
    mitk::ConnectomicsRenderingNodeRadiusParameterProperty::New();
  mitk::ConnectomicsRenderingEdgeColorParameterProperty::Pointer connectomicsRenderingEdgeGradientColorParameter =
    mitk::ConnectomicsRenderingEdgeColorParameterProperty::New();
  mitk::ConnectomicsRenderingEdgeRadiusParameterProperty::Pointer connectomicsRenderingEdgeRadiusParameter =
    mitk::ConnectomicsRenderingEdgeRadiusParameterProperty::New();

  mitk::ConnectomicsRenderingNodeThresholdParameterProperty::Pointer connectomicsRenderingNodeThresholdParameter =
    mitk::ConnectomicsRenderingNodeThresholdParameterProperty::New();
  mitk::ConnectomicsRenderingEdgeThresholdParameterProperty::Pointer connectomicsRenderingEdgeThresholdParameter =
    mitk::ConnectomicsRenderingEdgeThresholdParameterProperty::New();

  // set the properties
  node->AddProperty( connectomicsRenderingSchemePropertyName.c_str(),
    connectomicsRenderingScheme, renderer, overwrite );

  node->AddProperty( connectomicsRenderingEdgeFilteringPropertyName.c_str(),
    connectomicsRenderingEdgeFiltering, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeThresholdFilterParameterName.c_str(),
    connectomicsRenderingEdgeThresholdParameter, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeThresholdFilterThresholdName.c_str(),
    connectomicsRenderingEdgeThresholdFilterThresholdDefault, renderer, overwrite );

  node->AddProperty( connectomicsRenderingNodeFilteringPropertyName.c_str(),
    connectomicsRenderingNodeFiltering, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeThresholdFilterParameterName.c_str(),
    connectomicsRenderingNodeThresholdParameter, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeThresholdFilterThresholdName.c_str(),
    connectomicsRenderingNodeThresholdFilterThresholdDefault, renderer, overwrite );

  node->AddProperty( connectomicsRenderingNodeGradientStartColorName.c_str(),
    connectomicsRenderingNodeGradientStartColorDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeGradientEndColorName.c_str(),
    connectomicsRenderingNodeGradientEndColorDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeGradientColorParameterName.c_str(),
    connectomicsRenderingNodeGradientColorParameter, renderer, overwrite );

  node->AddProperty( connectomicsRenderingNodeRadiusStartName.c_str(),
    connectomicsRenderingNodeRadiusStartDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeRadiusEndName.c_str(),
    connectomicsRenderingNodeRadiusEndDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingNodeRadiusParameterName.c_str(),
    connectomicsRenderingNodeRadiusParameter, renderer, overwrite );

  node->AddProperty( connectomicsRenderingNodeChosenNodeName.c_str(),
    connectomicsRenderingNodeChosenNodeDefault, renderer, overwrite );

  node->AddProperty( connectomicsRenderingEdgeGradientStartColorName.c_str(),
    connectomicsRenderingEdgeGradientStartColorDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeGradientEndColorName.c_str(),
    connectomicsRenderingEdgeGradientEndColorDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeGradientColorParameterName.c_str(),
    connectomicsRenderingEdgeGradientColorParameter, renderer, overwrite );

  node->AddProperty( connectomicsRenderingEdgeRadiusStartName.c_str(),
    connectomicsRenderingEdgeRadiusStartDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeRadiusEndName.c_str(),
    connectomicsRenderingEdgeRadiusEndDefault, renderer, overwrite );
  node->AddProperty( connectomicsRenderingEdgeRadiusParameterName.c_str(),
    connectomicsRenderingEdgeRadiusParameter, renderer, overwrite );

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

bool mitk::ConnectomicsNetworkMapper3D::PropertiesChanged()
{
  mitk::ConnectomicsRenderingSchemeProperty * renderingScheme =
    static_cast< mitk::ConnectomicsRenderingSchemeProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingSchemePropertyName.c_str() ) );
  mitk::ConnectomicsRenderingEdgeFilteringProperty * edgeFilter =
    static_cast< mitk::ConnectomicsRenderingEdgeFilteringProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingEdgeFilteringPropertyName.c_str() ) );
  mitk::FloatProperty * edgeThreshold = static_cast< mitk::FloatProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingEdgeThresholdFilterThresholdName.c_str() ) );
  mitk::ConnectomicsRenderingNodeFilteringProperty * nodeFilter =
    static_cast< mitk::ConnectomicsRenderingNodeFilteringProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingNodeFilteringPropertyName.c_str() ) );

  mitk::ConnectomicsRenderingNodeThresholdParameterProperty * nodeThresholdParameter =
    static_cast< mitk::ConnectomicsRenderingNodeThresholdParameterProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingNodeThresholdFilterParameterName.c_str() ) );
  mitk::ConnectomicsRenderingEdgeThresholdParameterProperty * edgeThresholdParameter =
    static_cast< mitk::ConnectomicsRenderingEdgeThresholdParameterProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingEdgeThresholdFilterParameterName.c_str() ) );

  mitk::FloatProperty * nodeThreshold = static_cast< mitk::FloatProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingNodeThresholdFilterThresholdName.c_str() ) );
  mitk::ColorProperty * nodeColorStart = static_cast< mitk::ColorProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingNodeGradientStartColorName.c_str() ) );
  mitk::ColorProperty * nodeColorEnd = static_cast< mitk::ColorProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingNodeGradientEndColorName.c_str() ) );
  mitk::FloatProperty * nodeRadiusStart = static_cast< mitk::FloatProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingNodeRadiusStartName.c_str() ) );
  mitk::FloatProperty * nodeRadiusEnd = static_cast< mitk::FloatProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingNodeRadiusEndName.c_str() ) );
  mitk::StringProperty * chosenNode = static_cast< mitk::StringProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingNodeChosenNodeName.c_str() ) );
  mitk::ColorProperty * edgeColorStart = static_cast< mitk::ColorProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingEdgeGradientStartColorName.c_str() ) );
  mitk::ColorProperty * edgeColorEnd  = static_cast< mitk::ColorProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingEdgeGradientEndColorName.c_str() ) );
  mitk::FloatProperty * edgeRadiusStart = static_cast< mitk::FloatProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingEdgeRadiusStartName.c_str() ) );
  mitk::FloatProperty * edgeRadiusEnd = static_cast< mitk::FloatProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingEdgeRadiusEndName.c_str() ) );
  mitk::ConnectomicsRenderingNodeColorParameterProperty *  nodeColorParameter =
    static_cast< mitk::ConnectomicsRenderingNodeColorParameterProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingNodeGradientColorParameterName.c_str() ) );
  mitk::ConnectomicsRenderingNodeRadiusParameterProperty * nodeRadiusParameter =
    static_cast< mitk::ConnectomicsRenderingNodeRadiusParameterProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingNodeRadiusParameterName.c_str() ) );
  mitk::ConnectomicsRenderingEdgeColorParameterProperty * edgeColorParameter =
    static_cast< mitk::ConnectomicsRenderingEdgeColorParameterProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingEdgeGradientColorParameterName.c_str() ) );
  mitk::ConnectomicsRenderingEdgeRadiusParameterProperty * edgeRadiusParameter =
    static_cast< mitk::ConnectomicsRenderingEdgeRadiusParameterProperty * > (
    this->GetDataNode()->GetProperty( connectomicsRenderingEdgeRadiusParameterName.c_str() ) );

  if(
    m_ChosenRenderingScheme != renderingScheme->GetValueAsString() ||
    m_ChosenEdgeFilter != edgeFilter->GetValueAsString() ||
    m_EdgeThreshold != edgeThreshold->GetValue() ||
    m_EdgeThresholdParameter != edgeThresholdParameter->GetValueAsString() ||
    m_ChosenNodeFilter != nodeFilter->GetValueAsString() ||
    m_NodeThreshold != nodeThreshold->GetValue() ||
    m_NodeThresholdParameter != nodeThresholdParameter->GetValueAsString() ||
    m_NodeColorStart != nodeColorStart->GetValue() ||
    m_NodeColorEnd != nodeColorEnd->GetValue() ||
    m_NodeRadiusStart != nodeRadiusStart->GetValue() ||
    m_NodeRadiusEnd != nodeRadiusEnd->GetValue() ||
    m_ChosenNodeLabel != chosenNode->GetValueAsString() ||
    m_EdgeColorStart != edgeColorStart->GetValue() ||
    m_EdgeColorEnd != edgeColorEnd->GetValue() ||
    m_EdgeRadiusStart != edgeRadiusStart->GetValue() ||
    m_EdgeRadiusEnd != edgeRadiusEnd->GetValue() ||
    m_NodeColorParameter != nodeColorParameter->GetValueAsString() ||
    m_NodeRadiusParameter != nodeRadiusParameter->GetValueAsString() ||
    m_EdgeColorParameter != edgeColorParameter->GetValueAsString() ||
    m_EdgeRadiusParameter != edgeRadiusParameter->GetValueAsString()
    )
  {
    m_ChosenRenderingScheme = renderingScheme->GetValueAsString();
    m_ChosenEdgeFilter = edgeFilter->GetValueAsString();
    m_EdgeThreshold = edgeThreshold->GetValue();
    m_EdgeThresholdParameter = edgeThresholdParameter->GetValueAsString();
    m_ChosenNodeFilter = nodeFilter->GetValueAsString();
    m_NodeThreshold = nodeThreshold->GetValue();
    m_NodeThresholdParameter = nodeThresholdParameter->GetValueAsString();
    m_NodeColorStart = nodeColorStart->GetValue();
    m_NodeColorEnd = nodeColorEnd->GetValue();
    m_NodeRadiusStart = nodeRadiusStart->GetValue();
    m_NodeRadiusEnd = nodeRadiusEnd->GetValue();
    m_ChosenNodeLabel = chosenNode->GetValueAsString();
    m_EdgeColorStart = edgeColorStart->GetValue();
    m_EdgeColorEnd = edgeColorEnd->GetValue();
    m_EdgeRadiusStart = edgeRadiusStart->GetValue();
    m_EdgeRadiusEnd = edgeRadiusEnd->GetValue();
    m_NodeColorParameter = nodeColorParameter->GetValueAsString();
    m_NodeRadiusParameter = nodeRadiusParameter->GetValueAsString();
    m_EdgeColorParameter = edgeColorParameter->GetValueAsString();
    m_EdgeRadiusParameter = edgeRadiusParameter->GetValueAsString();

    return true;
  }

  return false;

}

double mitk::ConnectomicsNetworkMapper3D::FillNodeParameterVector( std::vector< double > * parameterVector, std::string parameterName )
{
  int end( parameterVector->size() );

  // constant parameter - uniform style
  if( parameterName == connectomicsRenderingNodeParameterConstant )
  {
    for(int index(0); index < end; index++)
    {
      parameterVector->at( index ) = 1.0;
    }
    return 1.0;
  }

  double maximum( 0.0 );

  // using the degree as parameter
  if( parameterName == connectomicsRenderingNodeParameterDegree )
  {
    std::vector< int > vectorOfDegree = this->GetInput()->GetDegreeOfNodes();
    for(int index(0); index < end; index++)
    {
      parameterVector->at( index ) = vectorOfDegree[ index ];
    }

    maximum = *std::max_element( parameterVector->begin(), parameterVector->end() );
  }

  // using betweenness centrality as parameter
  if( parameterName == connectomicsRenderingNodeParameterBetweenness )
  {
    std::vector< double > vectorOfBetweenness = this->GetInput()->GetNodeBetweennessVector();
    for(int index(0); index < end; index++)
    {
      parameterVector->at( index ) = vectorOfBetweenness[index];
    }
    maximum = *std::max_element( parameterVector->begin(), parameterVector->end() );
  }

  // using clustering coefficient as parameter
  if( parameterName == connectomicsRenderingNodeParameterClustering )
  {
    const std::vector< double > vectorOfClustering = this->GetInput()->GetLocalClusteringCoefficients();
    for(int index(0); index < end; index++)
    {
      parameterVector->at( index ) = vectorOfClustering[index];
    }
    maximum = *std::max_element( parameterVector->begin(), parameterVector->end() );
  }

  // using distance to a specific node as parameter
  if( parameterName == connectomicsRenderingNodeParameterColoringShortestPath )
  {
    bool labelFound( this->GetInput()->CheckForLabel( m_ChosenNodeLabel ) );
    // check whether the chosen node is valid
    if( !labelFound )
    {
      MITK_WARN << "Node chosen for rendering is not valid.";
      for(int index(0); index < end; index++)
      {
        parameterVector->at( index ) = 1.0;
      }
      return 1.0;
    }
    else
    {
      const std::vector< double > distanceVector = this->GetInput()->GetShortestDistanceVectorFromLabel( m_ChosenNodeLabel );
      for(int index(0); index < end; index++)
      {
        parameterVector->at( index ) = distanceVector[index];
      }
      maximum = *std::max_element( parameterVector->begin(), parameterVector->end() );
    }
  }

  // if the maximum is nearly zero
  if( std::abs( maximum ) < mitk::eps )
  {
    maximum = 1.0;
  }

  return maximum;
}

double mitk::ConnectomicsNetworkMapper3D::FillEdgeParameterVector( std::vector< double > * parameterVector, std::string parameterName )
{
    int end( parameterVector->size() );

  // constant parameter - uniform style
  if( parameterName == connectomicsRenderingEdgeParameterConstant )
  {
    for(int index(0); index < end; index++)
    {
      parameterVector->at( index ) = 1.0;
    }
    return 1.0;
  }

  double maximum( 0.0 );

  // using the weight as parameter
  if( parameterName == connectomicsRenderingEdgeParameterWeight )
  {
    std::vector< std::pair<
      std::pair< mitk::ConnectomicsNetwork::NetworkNode, mitk::ConnectomicsNetwork::NetworkNode >
      , mitk::ConnectomicsNetwork::NetworkEdge > >  vectorOfEdges = this->GetInput()->GetVectorOfAllEdges();

    for(int index(0); index < end; index++)
    {
      parameterVector->at( index ) = vectorOfEdges[ index ].second.weight;
    }

    maximum = *std::max_element( parameterVector->begin(), parameterVector->end() );
  }

  // using the edge centrality as parameter
  if( parameterName == connectomicsRenderingEdgeParameterCentrality )
  {
    const std::vector< double > vectorOfCentrality = this->GetInput()->GetEdgeBetweennessVector();
    for(int index(0); index < end; index++)
    {
      parameterVector->at( index ) = vectorOfCentrality[index];
    }

    maximum = *std::max_element( parameterVector->begin(), parameterVector->end() );
  }

  // if the maximum is nearly zero
  if( std::abs( maximum ) < mitk::eps )
  {
    maximum = 1.0;
  }

  return maximum;
}

void mitk::ConnectomicsNetworkMapper3D::FillNodeFilterBoolVector( std::vector< bool > * boolVector, std::string parameterName )
{
  std::vector< double > parameterVector;
  parameterVector.resize( boolVector->size() );
  int end( parameterVector.size() );

  // using the degree as parameter
  if( parameterName == connectomicsRenderingNodeParameterDegree )
  {
    std::vector< int > vectorOfDegree = this->GetInput()->GetDegreeOfNodes();
    for(int index(0); index < end; index++)
    {
      parameterVector.at( index ) = vectorOfDegree[ index ];
    }
  }

  // using betweenness centrality as parameter
  if( parameterName == connectomicsRenderingNodeParameterBetweenness )
  {
    std::vector< double > vectorOfBetweenness = this->GetInput()->GetNodeBetweennessVector();
    for(int index(0); index < end; index++)
    {
      parameterVector.at( index ) = vectorOfBetweenness[index];
    }
  }

  // using clustering coefficient as parameter
  if( parameterName == connectomicsRenderingNodeParameterClustering )
  {
    const std::vector< double > vectorOfClustering = this->GetInput()->GetLocalClusteringCoefficients();
    for(int index(0); index < end; index++)
    {
      parameterVector.at( index ) = vectorOfClustering[index];
    }
  }

  for( int index( 0 ), end( boolVector->size() ); index < end; index++ )
  {
    if( parameterVector.at( index ) >= m_NodeThreshold )
    {
      boolVector->at( index ) = true;
    }
    else
    {
      boolVector->at( index ) = false;
    }
  }

  return;

}

void mitk::ConnectomicsNetworkMapper3D::FillEdgeFilterBoolVector( std::vector< bool > * boolVector, std::string parameterName )
{
  std::vector< double > parameterVector;
  parameterVector.resize( boolVector->size() );
  int end( parameterVector.size() );


  // using the weight as parameter
  if( parameterName == connectomicsRenderingEdgeParameterWeight )
  {
    std::vector< std::pair<
      std::pair< mitk::ConnectomicsNetwork::NetworkNode, mitk::ConnectomicsNetwork::NetworkNode >
      , mitk::ConnectomicsNetwork::NetworkEdge > >  vectorOfEdges = this->GetInput()->GetVectorOfAllEdges();

    for(int index(0); index < end; index++)
    {
      parameterVector.at( index ) = vectorOfEdges[ index ].second.weight;
    }
  }

  // using the edge centrality as parameter
  if( parameterName == connectomicsRenderingEdgeParameterCentrality )
  {
    const std::vector< double > vectorOfCentrality = this->GetInput()->GetEdgeBetweennessVector();
    for(int index(0); index < end; index++)
    {
      parameterVector.at( index ) = vectorOfCentrality[index];
    }
  }

  for( int index( 0 ), end( boolVector->size() ); index < end; index++ )
  {
    if( parameterVector.at( index ) >= m_EdgeThreshold )
    {
      boolVector->at( index ) = true;
    }
    else
    {
      boolVector->at( index ) = false;
    }
  }

  return;
}
