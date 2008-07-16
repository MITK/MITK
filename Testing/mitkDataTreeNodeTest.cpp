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

#include "mitkDataTreeNode.h"

#include "mitkTestingMacros.h"

#include <iostream>

#include <mitkColoredRectangleRendering.h>
#include <mitkContour.h>
#include <mitkContourSet.h>
#include <mitkGeometryData.h>
#include <mitkGeometry2DData.h>
#include <mitkGradientBackground.h>
#include <mitkItkBaseDataAdapter.h>
#include <mitkLogoRendering.h>
#include <mitkPointData.h>
#include <mitkPointSet.h>
#include <mitkMesh.h>
//#include <mitkClosedSpline.h>
//#include <mitkRingSpline.h>
#include <mitkImage.h>
#include <mitkSeedsImage.h>
#include <mitkSurface.h>
#include <mitkBoundingObject.h>
#include <mitkUnstructuredGrid.h>

/**
 *  Simple example for a test for the (non-existent) class "DataTreeNode".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
class mitkDataTreeNodeTestClass { public:

static void TestDataSetting(mitk::DataTreeNode::Pointer dataTreeNode)
{
  mitk::BaseData::Pointer baseData;
  
  baseData = mitk::ColoredRectangleRendering::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a ColoredRectangleRendering object was set correctly" )
    
  baseData = mitk::Contour::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a Contour object was set correctly" )

  baseData = mitk::ContourSet::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a ContourSet object was set correctly" )

  baseData = mitk::GeometryData::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a GeometryData object was set correctly" )

  baseData = mitk::Geometry2DData::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a Geometry2DData object was set correctly" )

  baseData = mitk::GradientBackground::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a GradientBackground object was set correctly" )

  baseData = mitk::ItkBaseDataAdapter::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a ItkBaseDataAdapter object was set correctly" )

  baseData = mitk::LogoRendering::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a LogoRendering object was set correctly" )

  baseData = mitk::PointData::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a PointSet object was set correctly" )
  
  baseData = mitk::PointSet::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a PointSet object was set correctly" )

  baseData = mitk::Mesh::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a Mesh object was set correctly" )

  //baseData = mitk::ClosedSpline::New();
  //dataTreeNode->SetData(baseData);
  //MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a ClosedSpline object was set correctly" )

  //baseData = mitk::RingSpline::New();
  //dataTreeNode->SetData(baseData);
  //MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a RingSpline object was set correctly" )

  baseData = mitk::Image::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a Image object was set correctly" )
  
  baseData = mitk::SeedsImage::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a SeedsImage object was set correctly" )

  baseData = mitk::Surface::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a Surface object was set correctly" )

  baseData = mitk::BoundingObject::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a BoundingObject object was set correctly" )

  baseData = mitk::UnstructuredGrid::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a UnstructuredGrid object was set correctly" )

}
}; //mitkDataTreeNodeTestClass
int mitkDataTreeNodeTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("DataTreeNode")

  // let's create an object of our class  
  mitk::DataTreeNode::Pointer myDataTreeNode = mitk::DataTreeNode::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myDataTreeNode.IsNotNull(),"Testing instantiation") 

  //test setData() Method
  mitkDataTreeNodeTestClass::TestDataSetting(myDataTreeNode);

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!
  
  // always end with this!
  MITK_TEST_END()
}
