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
#include <mitkContour.h>
#include <mitkContourSet.h>

//Mapper Test
#include <mitkMapper.h>
#include <mitkGLMapper.h>
#include <mitkVtkMapper.h>
#include <mitkContourMapper2D.h>
#include <mitkContourSetMapper2D.h>

#include <mitkContourSetVtkMapper3D.h>
#include <mitkContourVtkMapper3D.h>

//Interactors
#include <mitkContourInteractor.h>
#include <mitkExtrudedContourInteractor.h>

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
class mitkDataNodeSegmentationTestClass { public:

static void TestDataSetting(mitk::DataNode::Pointer dataNode)
{

  mitk::BaseData::Pointer baseData;

  //NULL pointer Test
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a NULL pointer was set correctly" )

  baseData = mitk::Contour::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a Contour object was set correctly" )

  baseData = mitk::ContourSet::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a ContourSet object was set correctly" )
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

  mapper = mitk::ContourMapper2D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a ContourMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::ContourSetMapper2D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a ContourSetMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  //3D Mappers
  mapper = mitk::ContourSetVtkMapper3D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a ContourSetVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::ContourVtkMapper3D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a ContourVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )
}
static void TestInteractorSetting(mitk::DataNode::Pointer dataNode)
{

  //this method tests the SetInteractor() and GetInteractor methods
  //the Interactor base class calls the DataNode->SetInteractor method

  mitk::Interactor::Pointer interactor;

  MITK_TEST_CONDITION( interactor == dataNode->GetInteractor(), "Testing if a NULL pointer was set correctly (Interactor)" )

  interactor = mitk::ContourInteractor::New("AffineInteractions click to select", dataNode);
  MITK_TEST_CONDITION( interactor == dataNode->GetInteractor(), "Testing if a ContourInteractor was set correctly" )

  interactor = mitk::ExtrudedContourInteractor::New("AffineInteractions click to select", dataNode);
  MITK_TEST_CONDITION( interactor == dataNode->GetInteractor(), "Testing if a ExtrudedContourInteractor was set correctly" )
}

};

int mitkDataNodeSegmentationTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("DataNodeSegmentation")

  // Global interaction must(!) be initialized
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  // let's create an object of our class
  mitk::DataNode::Pointer myDataNode = mitk::DataNode::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myDataNode.IsNotNull(),"Testing instantiation")

  //test setData() Method
  mitkDataNodeSegmentationTestClass::TestDataSetting(myDataNode);
  mitkDataNodeSegmentationTestClass::TestMapperSetting(myDataNode);

  //note, that no data is set to the dataNode
  mitkDataNodeSegmentationTestClass::TestInteractorSetting(myDataNode);

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!

  // always end with this!
  MITK_TEST_END()
}
