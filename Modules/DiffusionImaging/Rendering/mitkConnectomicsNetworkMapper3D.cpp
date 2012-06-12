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

  (static_cast<mitk::ConnectomicsNetwork * > ( GetData() ) )->SetIsModified( false );

  //this->UpdateVtkObjects();

  
}

const mitk::ConnectomicsNetwork* mitk::ConnectomicsNetworkMapper3D::GetInput()
{

  return static_cast<const mitk::ConnectomicsNetwork * > ( GetData() );
}


void mitk::ConnectomicsNetworkMapper3D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer , bool overwrite)
{
  //TODO: implement


  // hand it to the superclass for base default properties
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
