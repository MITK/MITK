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

#include "vtkWindow.h"
#include <mitkVtkPropRenderer.h>

#include "mitkTestingMacros.h"

#include <iostream>

//Basedata Test
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

//Mapper Test
//#include <mitkMapper.h>
//#include <mitkMapper2D.h>
//#include <mitkBaseVtkMapper2D.h>
#include <mitkContourMapper2D.h>
#include <mitkContourSetMapper2D.h>
#include <mitkGeometry2DDataMapper2D.h>
#include <mitkGeometry2DDataMapper2D.h>
#include <mitkImageMapper2D.h>
#include <mitkMeshMapper2D.h>
#include <mitkPointSetMapper2D.h>
#include <mitkPointSetSliceMapper2D.h>
#include <mitkPolyDataGLMapper2D.h>
#include <mitkSurfaceMapper2D.h>
#include <mitkLineMapper2D.h>
#include <mitkSplineMapper2D.h>

#include <mitkContourSetVtkMapper3D.h>
#include <mitkContourVtkMapper3D.h>
#include <mitkGeometry2DDataVtkMapper3D.h>
#include <mitkMeshVtkMapper3D.h>
#include <mitkPointDataVtkMapper3D.h>
#include <mitkPointSetVtkMapper3D.h>
#include <mitkSurfaceVtkMapper3D.h>
#include <mitkUnstructuredGridVtkMapper3D.h>
#include <mitkVolumeDataVtkMapper3D.h>
#include <mitkLineVtkMapper3D.h>
#include <mitkSplineVtkMapper3D.h>

//Interactors
#include <mitkAffineInteractor.h>
#include <mitkConnectPointsInteractor.h>
#include <mitkContourInteractor.h>
#include <mitkExtrudedContourInteractor.h>
#include <mitkHierarchicalInteractor.h>
#include <mitkPointInteractor.h>
#include <mitkPointSelectorInteractor.h>
#include <mitkPointSetInteractor.h>
#include <mitkSeedsInteractor.h>

#include <mitkLineInteractor.h>
#include <mitkPointSnapInteractor.h>
#include <mitkPolygonInteractor.h>
#include <mitkPrimStripInteractor.h>

#include <mitkDisplayPointSetInteractor.h>

//Propertylist Test
//#include <mitkAnnotationProperty.h>
//#include <mitkClippingProperty.h>
//#include <mitkColorProperty.h>
//#include <mitkEnumerationProperty.h>
//  #include <mitkGridRepresentationProperty.h>
//  #include <mitkGridVolumeMapperProperty.h>
//  #include <mitkOrganTypeProperty.h>
//  #include <mitkVtkInterpolationProperty.h>
//  #include <mitkVtkRepresentationProperty.h>
//  #include <mitkVtkResliceInterpolationProperty.h>
//  #include <mitkVtkScalarModeProperty.h>
//




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

  //NULL pointer Test
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a NULL pointer was set correctly" )

  baseData = mitk::ColoredRectangleRendering::New();
  dataTreeNode->SetData(baseData);
  MITK_TEST_CONDITION( baseData == dataTreeNode->GetData(), "Testing if a ColoredRectangleRendering object was set correctly" )
//  MITK_TEST_CONDITION( baseData->GetGeometry(0)->GetVtkTransform() == dataTreeNode->GetVtkTransform(0), "Testing if a NULL pointer was set correctly" )    
  
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
static void TestMapperSetting(mitk::DataTreeNode::Pointer dataTreeNode)
{
  //tests the SetMapper() method
  //in dataTreeNode is a mapper vector which can be accessed by index
  //in this test method we use only slot 0 (filled with null) and slot 1
  //so we also test the destructor of the mapper classes
  mitk::Mapper::Pointer mapper;

  dataTreeNode->SetMapper(0,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(0), "Testing if a NULL pointer was set correctly" )

  mapper = mitk::ContourMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a ContourMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::ContourSetMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a ContourSetMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::Geometry2DDataMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a Geometry2DDataMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::ImageMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a ImageMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::MeshMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a MeshMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::PointSetMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a PointSetMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::PointSetSliceMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a PointSetSliceMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::PolyDataGLMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a PolyDataGLMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::SurfaceMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a SurfaceMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::LineMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a LineMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::SplineMapper2D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a SplineMapper2D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  //3D Mappers
  mapper = mitk::ContourSetVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a ContourSetVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::ContourVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a ContourVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::Geometry2DDataVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a Geometry2DDataVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::MeshVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a MeshVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::PointDataVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a PointDataVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::PointSetVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a PointSetVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::SurfaceVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a SurfaceVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::UnstructuredGridVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a UnstructuredGridVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::VolumeDataVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a VolumeDataVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  //linker error
  //mapper = mitk::LineVtkMapper3D::New();
  //dataTreeNode->SetMapper(1,mapper);
  //MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a LineVtkMapper3D was set correctly" )
  //MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

  mapper = mitk::SplineVtkMapper3D::New();
  dataTreeNode->SetMapper(1,mapper);
  MITK_TEST_CONDITION( mapper == dataTreeNode->GetMapper(1), "Testing if a SplineVtkMapper3D was set correctly" )
  MITK_TEST_CONDITION( dataTreeNode == mapper->GetDataTreeNode(), "Testing if the mapper returns the right DataTreeNode" )

}
static void TestInteractorSetting(mitk::DataTreeNode::Pointer dataTreeNode)
{

  //this method tests the SetInteractor() and GetInteractor methods
  //the Interactor base class calls the DataTreeNode->SetInteractor method

  mitk::Interactor::Pointer interactor;

  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a NULL pointer was set correctly (Interactor)" )

  interactor = mitk::AffineInteractor::New("AffineInteractions click to select", dataTreeNode);
  dataTreeNode->EnableInteractor();
  dataTreeNode->DisableInteractor();
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a AffineInteractor was set correctly" )

  interactor = mitk::ConnectPointsInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a ConnectPointsInteractor was set correctly" )

  interactor = mitk::ContourInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a ContourInteractor was set correctly" )

  interactor = mitk::ExtrudedContourInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a ExtrudedContourInteractor was set correctly" )

  interactor = mitk::HierarchicalInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a HierarchicalInteractor was set correctly" )

  interactor = mitk::PointInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a PointInteractor was set correctly" )

  interactor = mitk::PointSelectorInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a PointSelectorInteractor was set correctly" )

  interactor = mitk::PointSetInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a PointSetInteractor was set correctly" )

  interactor = mitk::SeedsInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a SeedsInteractor was set correctly" )

  interactor = mitk::LineInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a LineInteractor was set correctly" )
  
  //Bug#: 	1501
  //interactor = mitk::PointSnapInteractor::New("AffineInteractions click to select", dataTreeNode);
  //MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a PointSnapInteractor was set correctly" )

  //Bug#: 	1502
  //interactor = mitk::PolygonInteractor::New("AffineInteractions click to select", dataTreeNode);
  //MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a PolygonInteractor was set correctly" )

  //see other bugs
  //interactor = mitk::PrimStripInteractor::New("AffineInteractions click to select", dataTreeNode);
  //MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a PrimStripInteractor was set correctly" )

  interactor = mitk::DisplayPointSetInteractor::New("AffineInteractions click to select", dataTreeNode);
  MITK_TEST_CONDITION( interactor == dataTreeNode->GetInteractor(), "Testing if a DisplayPointSetInteractor was set correctly" )

}
static void TestPropertyList(mitk::DataTreeNode::Pointer dataTreeNode)
{

  mitk::PropertyList::Pointer propertyList =  dataTreeNode->GetPropertyList();
   

  MITK_TEST_CONDITION(dataTreeNode->GetPropertyList() != NULL, "Testing if the constructor set the propertylist" )
    
//  MITK_TEST_CONDITION(dataTreeNode->AddProperty() != NULL, "Testing if t" )

  MITK_TEST_CONDITION(propertyList == dataTreeNode->GetPropertyList(), "Testing if the propertylist has changed during the last tests" )


}
static void TestSelected(mitk::DataTreeNode::Pointer dataTreeNode)
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
static void TestGetMTime(mitk::DataTreeNode::Pointer dataTreeNode)
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
  mitkDataTreeNodeTestClass::TestMapperSetting(myDataTreeNode);
  
  //note, that no data is set to the dataTreeNode
  mitkDataTreeNodeTestClass::TestInteractorSetting(myDataTreeNode);
  mitkDataTreeNodeTestClass::TestPropertyList(myDataTreeNode);
  mitkDataTreeNodeTestClass::TestSelected(myDataTreeNode);
  mitkDataTreeNodeTestClass::TestGetMTime(myDataTreeNode);

 


  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!
  
  // always end with this!
  MITK_TEST_END()
}
