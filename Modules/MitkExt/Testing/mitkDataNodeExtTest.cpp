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

#include "mitkDataNode.h"

#include <vtkWindow.h>
#include "mitkVtkPropRenderer.h"

#include "mitkTestingMacros.h"
#include "mitkGlobalInteraction.h"

#include <iostream>

//Basedata Test
#include <mitkItkBaseDataAdapter.h>
#include <mitkMesh.h>
#include <mitkSeedsImage.h>
#include <mitkBoundingObject.h>
#include <mitkUnstructuredGrid.h>

//Mapper Test
#include <mitkMapper.h>
#include <mitkMapper2D.h>
#include <mitkVtkMapper2D.h>
#include <mitkMeshMapper2D.h>
#include <mitkUnstructuredGridMapper2D.h>
#include <mitkLineMapper2D.h>
#include <mitkSplineMapper2D.h>

#include <mitkMeshVtkMapper3D.h>
#include <mitkUnstructuredGridVtkMapper3D.h>
#include <mitkLineVtkMapper3D.h>
#include <mitkSplineVtkMapper3D.h>

//Interactors
#include <mitkConnectPointsInteractor.h>
#include <mitkPointInteractor.h>
#include <mitkPointSelectorInteractor.h>
#include <mitkSeedsInteractor.h>
#include <mitkDisplayPointSetInteractor.h>

//Propertylist Test
#include <mitkAnnotationProperty.h>
#include <mitkClippingProperty.h>
#include <mitkColorProperty.h>
#include <mitkEnumerationProperty.h>
#include <mitkGridRepresentationProperty.h>
#include <mitkGridVolumeMapperProperty.h>
#include <mitkOrganTypeProperty.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkVtkScalarModeProperty.h>


/**
 *  Extended test for mitk::DataNode. A number of tests from the core test
 *  mitkDataNodeTest are assumed to pass!
 */
class mitkDataNodeExtTestClass { public:

static void TestDataSetting(mitk::DataNode::Pointer dataNode)
{

  mitk::BaseData::Pointer baseData;

  //NULL pointer Test
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a NULL pointer was set correctly" )

  baseData = mitk::ItkBaseDataAdapter::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a ItkBaseDataAdapter object was set correctly" )

  baseData = mitk::Mesh::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a Mesh object was set correctly" )

  baseData = mitk::SeedsImage::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a SeedsImage object was set correctly" )

  baseData = mitk::BoundingObject::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a BoundingObject object was set correctly" )

  baseData = mitk::UnstructuredGrid::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a UnstructuredGrid object was set correctly" )
}

static void TestMapperSetting(mitk::DataNode::Pointer dataNode)
{
  //tests the SetMapper() method
  //in dataNode is a mapper vector which can be accessed by index
  //in this test method we use only slot 0 (filled with null) and slot 1
  //so we also test the destructor of the mapper classes
  mitk::Mapper::Pointer mapper;

  dataNode->SetMapper(0,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(0), "Testing if a NULL pointer was set correctly" )

  mapper = mitk::MeshMapper2D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a MeshMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::UnstructuredGridMapper2D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a UnstructuredGridMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::LineMapper2D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a LineMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::SplineMapper2D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a SplineMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::MeshVtkMapper3D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a MeshVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::UnstructuredGridVtkMapper3D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a UnstructuredGridVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  //linker error
  //mapper = mitk::LineVtkMapper3D::New();
  //dataNode->SetMapper(1,mapper);
  //MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a LineVtkMapper3D was set correctly" )
  //MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::SplineVtkMapper3D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a SplineVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )
}
static void TestInteractorSetting(mitk::DataNode::Pointer dataNode)
{

  //this method tests the SetInteractor() and GetInteractor methods
  //the Interactor base class calls the DataNode->SetInteractor method

  mitk::Interactor::Pointer interactor;

  MITK_TEST_CONDITION( interactor == dataNode->GetInteractor(), "Testing if a NULL pointer was set correctly (Interactor)" )

  interactor = mitk::ConnectPointsInteractor::New("AffineInteractions click to select", dataNode);
  MITK_TEST_CONDITION( interactor == dataNode->GetInteractor(), "Testing if a ConnectPointsInteractor was set correctly" )

  interactor = mitk::PointInteractor::New("AffineInteractions click to select", dataNode);
  MITK_TEST_CONDITION( interactor == dataNode->GetInteractor(), "Testing if a PointInteractor was set correctly" )

  interactor = mitk::PointSelectorInteractor::New("AffineInteractions click to select", dataNode);
  MITK_TEST_CONDITION( interactor == dataNode->GetInteractor(), "Testing if a PointSelectorInteractor was set correctly" )

  interactor = mitk::SeedsInteractor::New("AffineInteractions click to select", dataNode);
  MITK_TEST_CONDITION( interactor == dataNode->GetInteractor(), "Testing if a SeedsInteractor was set correctly" )

  interactor = mitk::DisplayPointSetInteractor::New("AffineInteractions click to select", dataNode);
  MITK_TEST_CONDITION( interactor == dataNode->GetInteractor(), "Testing if a DisplayPointSetInteractor was set correctly" )
}

};

int mitkDataNodeExtTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("DataNode")

  // Global interaction must(!) be initialized
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  // let's create an object of our class
  mitk::DataNode::Pointer myDataNode = mitk::DataNode::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myDataNode.IsNotNull(),"Testing instantiation")

  //test setData() Method
  mitkDataNodeExtTestClass::TestDataSetting(myDataNode);
  mitkDataNodeExtTestClass::TestMapperSetting(myDataNode);

  //note, that no data is set to the dataNode
  mitkDataNodeExtTestClass::TestInteractorSetting(myDataNode);

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!

  // always end with this!
  MITK_TEST_END()
}
