/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkDataNode.h"

#include <vtkWindow.h>
#include "mitkVtkPropRenderer.h"

#include "mitkTestingMacros.h"
#include "mitkGlobalInteraction.h"

#include <iostream>

//Basedata Test
#include <mitkColoredRectangleRendering.h>
#include <mitkGeometryData.h>
#include <mitkGeometry2DData.h>
#include <mitkGradientBackground.h>
#include <mitkLogoRendering.h>
#include <mitkPointSet.h>
#include <mitkImage.h>
#include <mitkSurface.h>

//Mapper Test
#include <mitkGeometry2DDataMapper2D.h>
#include <mitkGeometry2DDataMapper2D.h>
#include <mitkImageMapper2D.h>
#include <mitkPointSetMapper2D.h>
#include <mitkPolyDataGLMapper2D.h>
#include <mitkSurfaceMapper2D.h>

#include <mitkGeometry2DDataVtkMapper3D.h>
#include <mitkPointSetVtkMapper3D.h>
#include <mitkSurfaceVtkMapper3D.h>
#include <mitkVolumeDataVtkMapper3D.h>

//Interactors
#include <mitkAffineInteractor.h>
#include <mitkPointSetInteractor.h>

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

static void TestDataSetting(mitk::DataNode::Pointer dataTreeNode)
{

  mitk::BaseData::Pointer baseData; 

  //NULL pointer Test
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a NULL pointer was set correctly" )

  baseData = mitk::ColoredRectangleRendering::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a ColoredRectangleRendering object was set correctly" )
//  MITK_TEST_CONDITION( baseData->GetGeometry(0)->GetVtkTransform() == dataTreeNode->GetVtkTransform(0), "Testing if a NULL pointer was set correctly" )    
  
  baseData = mitk::GeometryData::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a GeometryData object was set correctly" )

  baseData = mitk::Geometry2DData::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a Geometry2DData object was set correctly" )

  baseData = mitk::GradientBackground::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a GradientBackground object was set correctly" )

  baseData = mitk::LogoRendering::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a LogoRendering object was set correctly" )

  baseData = mitk::PointSet::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a PointSet object was set correctly" )

  baseData = mitk::Image::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a Image object was set correctly" )

  baseData = mitk::Surface::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a Surface object was set correctly" )
}
static void TestMapperSetting(mitk::DataNode::Pointer dataTreeNode)
{
  //tests the SetMapper() method
  //in dataTreeNode is a mapper vector which can be accessed by index
  //in this test method we use only slot 0 (filled with null) and slot 1
  //so we also test the destructor of the mapper classes
  mitk::Mapper::Pointer mapper;

  dataTreeNode->SetMapper(0,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(0), "Testing if a NULL pointer was set correctly" )

  mapper = mitk::Geometry2DDataMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a Geometry2DDataMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::ImageMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a ImageMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::PointSetMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a PointSetMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a PolyDataGLMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::SurfaceMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a SurfaceMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::Geometry2DDataVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a Geometry2DDataVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::PointSetVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a PointSetVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::SurfaceVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a SurfaceVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  mapper = mitk::VolumeDataVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a VolumeDataVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )

  //linker error
  //mapper = mitk::LineVtkMapper3D::New();
  //dataTreeNode->SetMapper(1,mapper);
  //MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a LineVtkMapper3D was set correctly" )
  //MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataNode(), "Testing if the mapper returns the right DataNode" )
}
static void TestInteractorSetting(mitk::DataNode::Pointer dataTreeNode)
{

  //this method tests the SetInteractor() and GetInteractor methods
  //the Interactor base class calls the DataNode->SetInteractor method

  mitk::Interactor::Pointer interactor;

  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a NULL pointer was set correctly (Interactor)" )

  interactor = mitk::AffineInteractor::New("AffineInteractions click to select", dataTreeNode);
  dataTreeNode->EnableInteractor();
  dataTreeNode->DisableInteractor();
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a AffineInteractor was set correctly" )

  interactor = mitk::PointSetInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a PointSetInteractor was set correctly" )
}
static void TestPropertyList(mitk::DataNode::Pointer dataTreeNode)
{

  mitk::PropertyList::Pointer propertyList =  dataTreeNode->GetPropertyList();
   

  MITK_TEST_CONDITION(dataTreeNode->GetPropertyList() != NULL, "Testing if the constructor set the propertylist" )

  dataTreeNode->SetIntProperty("int", -31337);
  int x;
  dataTreeNode->GetIntProperty("int", x);
  MITK_TEST_CONDITION(x == -31337, "Testing Set/GetIntProperty");

  dataTreeNode->SetBoolProperty("bool", true);
  bool b;
  dataTreeNode->GetBoolProperty("bool", b);
  MITK_TEST_CONDITION(b == true, "Testing Set/GetBoolProperty");
  dataTreeNode->SetFloatProperty("float", -31.337);
  float y;
  dataTreeNode->GetFloatProperty("float", y);
  MITK_TEST_CONDITION(y - -31.337 < 0.01, "Testing Set/GetFloatProperty");
  dataTreeNode->SetStringProperty("string", "MITK");
  std::string s = "GANZVIELPLATZ";
  dataTreeNode->GetStringProperty("string", s);
  MITK_TEST_CONDITION(s == "MITK", "Testing Set/GetStringProperty");

  std::string name = "MyTestName";
  dataTreeNode->SetName(name.c_str());
  MITK_TEST_CONDITION(dataTreeNode->GetName() == name, "Testing Set/GetName");
  name = "MySecondTestName";
  dataTreeNode->SetName(name);
  MITK_TEST_CONDITION(dataTreeNode->GetName() == name, "Testing Set/GetName(std::string)");

  MITK_TEST_CONDITION(propertyList == dataTreeNode->GetPropertyList(), "Testing if the propertylist has changed during the last tests" )
}

static void TestSelected(mitk::DataNode::Pointer dataTreeNode)
{
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();
 
  mitk::VtkPropRenderer::Pointer base = mitk::VtkPropRenderer::New( "the first renderer", renderWindow );

  //with BaseRenderer==Null
  MITK_TEST_CONDITION(!dataTreeNode->IsSelected(), "Testing if this node is not set as selected" )

  dataTreeNode->SetSelected(true);
  MITK_TEST_CONDITION(dataTreeNode->IsSelected(), "Testing if this node is set as selected" )
  dataTreeNode->SetSelected(false);

  dataTreeNode->SetSelected(true,base);

  MITK_TEST_CONDITION(dataTreeNode->IsSelected(base), "Testing if this node with right base renderer is set as selected" )




}
static void TestGetMTime(mitk::DataNode::Pointer dataTreeNode)
{
  unsigned long time;
  time = dataTreeNode->GetMTime();
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  
  dataTreeNode->SetData(pointSet);
  MITK_TEST_CONDITION( time != dataTreeNode->GetMTime(), "Testing if the node timestamp is updated after adding data to the node" )
  
  mitk::Point3D point;
  point.Fill(3.0);
  pointSet->SetPoint(0,point);

  //less or equal because dataTreeNode timestamp is little later then the basedata timestamp
  MITK_TEST_CONDITION( pointSet->GetMTime() <= dataTreeNode->GetMTime(), "Testing if the node timestamp is updated after base data was modified" )

  // testing if changing anything in the property list also sets the node in a modified state
  unsigned long lastModified = dataTreeNode->GetMTime();
  dataTreeNode->SetIntProperty("testIntProp", 2344);
  MITK_TEST_CONDITION( lastModified <= dataTreeNode->GetMTime(), "Testing if the node timestamp is updated after property list was modified" )

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
  
  //note, that no data is set to the dataTreeNode
  mitkDataNodeTestClass::TestInteractorSetting(myDataNode);
  mitkDataNodeTestClass::TestPropertyList(myDataNode);
  mitkDataNodeTestClass::TestSelected(myDataNode);
  mitkDataNodeTestClass::TestGetMTime(myDataNode);

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!
  
  // always end with this!
  MITK_TEST_END()
}
