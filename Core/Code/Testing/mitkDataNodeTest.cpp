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
#include <mitkRenderWindowFrame.h>
#include <mitkGeometryData.h>
#include <mitkPlaneGeometryData.h>
#include <mitkGradientBackground.h>
#include <mitkManufacturerLogo.h>
#include <mitkPointSet.h>
#include <mitkImage.h>
#include <mitkSurface.h>

//Mapper Test
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkImageVtkMapper2D.h>
#include <mitkSurfaceGLMapper2D.h>

#include <mitkPlaneGeometryDataVtkMapper3D.h>
#include <mitkPointSetVtkMapper3D.h>
#include <mitkPointSetVtkMapper2D.h>
#include <mitkSurfaceVtkMapper3D.h>
#include <mitkVolumeDataVtkMapper3D.h>

//Interactors
#include <mitkPointSetDataInteractor.h>

//Propertylist Test



/**
 *  Simple example for a test for the (non-existent) class "DataNode".
 *
 *  argc and argv are the command line parameters which were passed to
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
class mitkDataNodeTestClass { public:

static void TestDataSetting(mitk::DataNode::Pointer dataNode)
{

  mitk::BaseData::Pointer baseData;

  //NULL pointer Test
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a NULL pointer was set correctly" )

  baseData = mitk::RenderWindowFrame::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a RenderWindowFrame object was set correctly" )
//  MITK_TEST_CONDITION( baseData->GetGeometry(0)->GetVtkTransform() == dataNode->GetVtkTransform(0), "Testing if a NULL pointer was set correctly" )

  baseData = mitk::GeometryData::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a GeometryData object was set correctly" )

  baseData = mitk::PlaneGeometryData::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a PlaneGeometryData object was set correctly" )

  baseData = mitk::GradientBackground::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a GradientBackground object was set correctly" )

  baseData = mitk::ManufacturerLogo::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a ManufacturerLogo object was set correctly" )

  baseData = mitk::PointSet::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a PointSet object was set correctly" )

  baseData = mitk::Image::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a Image object was set correctly" )

  baseData = mitk::Surface::New();
  dataNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataNode->GetData(), "Testing if a Surface object was set correctly" )
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

  mapper = mitk::PlaneGeometryDataMapper2D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a PlaneGeometryDataMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::ImageVtkMapper2D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a ImageVtkMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::PointSetVtkMapper2D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a PointSetVtkMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::SurfaceGLMapper2D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a SurfaceGLMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::PlaneGeometryDataVtkMapper3D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a PlaneGeometryDataVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::PointSetVtkMapper3D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a PointSetVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::SurfaceVtkMapper3D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a SurfaceVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::VolumeDataVtkMapper3D::New();
  dataNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a VolumeDataVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  //linker error
  //mapper = mitk::LineVtkMapper3D::New();
  //dataNode->SetMapper(1,mapper);
  //MITK_TEST_CONDITION( mapper == dataNode->GetMapper(1), "Testing if a LineVtkMapper3D was set correctly" )
  //MITK_TEST_CONDITION( dataNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )
}
static void TestInteractorSetting(mitk::DataNode::Pointer dataNode)
{

  //this method tests the SetInteractor() and GetInteractor methods
  //the DataInteractor base class calls the DataNode->SetInteractor method

  mitk::DataInteractor::Pointer interactor;

  MITK_TEST_CONDITION( interactor == dataNode->GetDataInteractor(), "Testing if a NULL pointer was set correctly (DataInteractor)" )

  interactor = mitk::PointSetDataInteractor::New();
  interactor->SetEventConfig("PointSetConfig.xml");
  interactor->SetDataNode(dataNode);
  MITK_TEST_CONDITION( interactor == dataNode->GetDataInteractor(), "Testing if a PointSetDataInteractor was set correctly" )

  interactor = mitk::PointSetDataInteractor::New();
  dataNode->SetDataInteractor(interactor);
  MITK_TEST_CONDITION( interactor == dataNode->GetDataInteractor(), "Testing if a PointSetDataInteractor was set correctly" )
}
static void TestPropertyList(mitk::DataNode::Pointer dataNode)
{

  mitk::PropertyList::Pointer propertyList =  dataNode->GetPropertyList();


  MITK_TEST_CONDITION(dataNode->GetPropertyList() != NULL, "Testing if the constructor set the propertylist" )

  dataNode->SetIntProperty("int", -31337);
  int x;
  dataNode->GetIntProperty("int", x);
  MITK_TEST_CONDITION(x == -31337, "Testing Set/GetIntProperty");

  dataNode->SetBoolProperty("bool", true);
  bool b;
  dataNode->GetBoolProperty("bool", b);
  MITK_TEST_CONDITION(b == true, "Testing Set/GetBoolProperty");
  dataNode->SetFloatProperty("float", -31.337);
  float y;
  dataNode->GetFloatProperty("float", y);
  MITK_TEST_CONDITION(y - -31.337 < 0.01, "Testing Set/GetFloatProperty");
  double yd = 0;
  dataNode->GetDoubleProperty("float", yd);
  MITK_TEST_CONDITION(mitk::Equal(yd, static_cast<double>(y)), "Testing GetDoubleProperty");
  dataNode->SetStringProperty("string", "MITK");
  std::string s = "GANZVIELPLATZ";
  dataNode->GetStringProperty("string", s);
  MITK_TEST_CONDITION(s == "MITK", "Testing Set/GetStringProperty");

  std::string name = "MyTestName";
  dataNode->SetName(name.c_str());
  MITK_TEST_CONDITION(dataNode->GetName() == name, "Testing Set/GetName");
  name = "MySecondTestName";
  dataNode->SetName(name);
  MITK_TEST_CONDITION(dataNode->GetName() == name, "Testing Set/GetName(std::string)");

  MITK_TEST_CONDITION(propertyList == dataNode->GetPropertyList(), "Testing if the propertylist has changed during the last tests" )
}

static void TestSelected(mitk::DataNode::Pointer dataNode)
{
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();

  mitk::VtkPropRenderer::Pointer base = mitk::VtkPropRenderer::New( "the first renderer", renderWindow, mitk::RenderingManager::GetInstance() );

  //with BaseRenderer==Null
  MITK_TEST_CONDITION(!dataNode->IsSelected(), "Testing if this node is not set as selected" )

  dataNode->SetSelected(true);
  MITK_TEST_CONDITION(dataNode->IsSelected(), "Testing if this node is set as selected" )
  dataNode->SetSelected(false);

  dataNode->SetSelected(true,base);

  MITK_TEST_CONDITION(dataNode->IsSelected(base), "Testing if this node with right base renderer is set as selected" )

  //Delete RenderWindow correctly
  renderWindow->Delete();

}
static void TestGetMTime(mitk::DataNode::Pointer dataNode)
{
  unsigned long time;
  time = dataNode->GetMTime();
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  dataNode->SetData(pointSet);
  MITK_TEST_CONDITION( time != dataNode->GetMTime(), "Testing if the node timestamp is updated after adding data to the node" )

  mitk::Point3D point;
  point.Fill(3.0);
  pointSet->SetPoint(0,point);

  //less or equal because dataNode timestamp is little later then the basedata timestamp
  MITK_TEST_CONDITION( pointSet->GetMTime() <= dataNode->GetMTime(), "Testing if the node timestamp is updated after base data was modified" )

  // testing if changing anything in the property list also sets the node in a modified state
  unsigned long lastModified = dataNode->GetMTime();
  dataNode->SetIntProperty("testIntProp", 2344);
  MITK_TEST_CONDITION( lastModified <= dataNode->GetMTime(), "Testing if the node timestamp is updated after property list was modified" )

}
}; //mitkDataNodeTestClass
int mitkDataNodeTest(int /* argc */, char* /*argv*/[])
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
  mitkDataNodeTestClass::TestDataSetting(myDataNode);
  mitkDataNodeTestClass::TestMapperSetting(myDataNode);
  //
  //note, that no data is set to the dataNode
  mitkDataNodeTestClass::TestInteractorSetting(myDataNode);
  mitkDataNodeTestClass::TestPropertyList(myDataNode);
  mitkDataNodeTestClass::TestSelected(myDataNode);
  mitkDataNodeTestClass::TestGetMTime(myDataNode);

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!

  // always end with this!
  MITK_TEST_END()
}
