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

#include "mitkNavigationDataObjectVisualizationFilter.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>

#include <vtkTransform.h>
#include <iostream>

#include "mitkSurface.h"

/**Documentation
*  test for the class "NavigationDataObjectVisualizationFilter".
*/
class mitkNavigationDataObjectVisualizationFilterTestSuite : public mitk::TestFixture
{
  // List of Tests
  CPPUNIT_TEST_SUITE(mitkNavigationDataObjectVisualizationFilterTestSuite);

  //Constructor
  MITK_TEST(TestInitialize);
  MITK_TEST(TestInput);
  MITK_TEST(TestOutput);
  MITK_TEST(TestRepresentationObjects);
  MITK_TEST(TestTransforms);
  MITK_TEST(TestMessWithRepresentationObjects);
  MITK_TEST(TestThirdInput);
  MITK_TEST(TestSetTransformPosition);
  MITK_TEST(TestSetTransformOrientation);
  MITK_TEST(TestConvenienceSetTransformOrientation);
  MITK_TEST(TestUpdateOrientation);

  CPPUNIT_TEST_SUITE_END();

  // Used Variables
private:
  mitk::NavigationDataObjectVisualizationFilter::Pointer myFilter;
  mitk::NavigationData::PositionType initialPos1, initialPos2;
  mitk::NavigationData::OrientationType initialOri1;
  mitk::NavigationData::OrientationType initialOri2;
  mitk::ScalarType initialError1;
  mitk::ScalarType initialError2;
  bool initialValid1;
  bool initialValid2;
  mitk::NavigationData::Pointer nd1;
  mitk::NavigationData::Pointer nd2;

  // Test setting BaseData
  mitk::Surface::Pointer mitkToolData1 ;

  mitk::Surface::Pointer mitkToolData2 ;

  //dummy for test; will not be set but used to test find
  mitk::Surface::Pointer mitkToolDataDummy ;
  //and the Dummy NavigationData for this
  mitk::NavigationData::OrientationType initialOriDummy;
  mitk::NavigationData::PositionType initialPosDummy;
  mitk::ScalarType initialErrorDummy;
  bool initialValidDummy;
  mitk::NavigationData::Pointer ndDummy ;

  mitk::NavigationData* output;

  mitk::AffineTransform3D::Pointer affineTransform1;
  mitk::AffineTransform3D::OutputVectorType offset1;

  mitk::AffineTransform3D::Pointer affineTransform2;
  mitk::AffineTransform3D::OutputVectorType offset2;
  mitk::AffineTransform3D::MatrixType::InternalMatrixType m1;
  mitk::AffineTransform3D::MatrixType::InternalMatrixType m2;

public:

  // Set up for variables
  void setUp()
  {
    // -------------- Setup for Filter ---------------------------

    /* create helper objects: navigation data with position as origin, zero quaternion, zero error and data valid */
    myFilter = mitk::NavigationDataObjectVisualizationFilter::New();
    mitk::FillVector3D(initialPos1, 1.1, 2.2, 3.3);
    mitk::FillVector3D(initialPos2, 5.0, 6.0, 7.0);
    mitk::FillVector4D(initialOri1, 0.1, 0.2, 0.3, 0.4);
    mitk::FillVector4D(initialOri2,0.5, 0.6, 0.7, 0.8);
    initialError1=0.0;
    initialError2=5.0;
    initialValid1 = true;
    initialValid2 = true;

    //Set Navigation Data
    nd1 = mitk::NavigationData::New();
    nd1->SetPosition(initialPos1);
    nd1->SetOrientation(initialOri1);
    nd1->SetPositionAccuracy(initialError1);
    nd1->SetDataValid(initialValid1);

    nd2 = mitk::NavigationData::New();
    nd2->SetPosition(initialPos2);
    nd2->SetOrientation(initialOri2);
    nd2->SetPositionAccuracy(initialError2);
    nd2->SetDataValid(initialValid2);

    //Set Input
    myFilter->SetInput(nd1);
    myFilter->SetInput(1, nd2);
    output = myFilter->GetOutput();

    // -------------------------- Setup for TestData ----------------------

    // Test setting BaseData
    mitkToolData1 = mitk::Surface::New();
    mitkToolData2 = mitk::Surface::New();

    //dummy for test; will not be set but used to test find
    mitkToolDataDummy = mitk::Surface::New();
    //and the Dummy NavigationData for this
    mitk::FillVector3D(initialPosDummy, 8.8, 9.9, 10.10);
    mitk::FillVector4D(initialOriDummy,1.1, 2.2, 3.3, 4.4);
    initialErrorDummy=10.0;
    initialValidDummy=true;
    ndDummy = mitk::NavigationData::New();
    ndDummy->SetPosition(initialPosDummy);
    ndDummy->SetOrientation(initialOriDummy);
    ndDummy->SetPositionAccuracy(initialErrorDummy);
    ndDummy->SetDataValid(initialValidDummy);
    //now we have ndDummy and mitkToolDataDummy to test with
  }

  void tearDown()
  {
  }

  // Test functions

  void TestInitialize(){
    // first test: did this work?
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation", myFilter.IsNotNull());
  }

  void TestInput(){
    //testing the input
    CPPUNIT_ASSERT_MESSAGE("Testing Set-/GetInput() input 1 without index",myFilter->GetInput() == nd1);
    CPPUNIT_ASSERT_MESSAGE("Testing Set-/GetInput() input 1", myFilter->GetInput(0) == nd1);
    CPPUNIT_ASSERT_MESSAGE( "Testing Set-/GetInput() input 2", myFilter->GetInput(1) == nd2);
    CPPUNIT_ASSERT_MESSAGE("Testing GetNumberOfToolRepresentations()", myFilter->GetNumberOfToolRepresentations() == 0);
  }

  void TestOutput(){
    //testing getting the output
    CPPUNIT_ASSERT_MESSAGE("Testing GetOutput()", output != NULL);
    CPPUNIT_ASSERT_MESSAGE("Testing GetOutput() == GetOutput()", output == myFilter->GetOutput());
    CPPUNIT_ASSERT_MESSAGE("Testing GetOutput() != GetOutput(1)", output != myFilter->GetOutput(1));
  }

  void TestRepresentationObjects(){
    //setting nodes
    myFilter->SetRepresentationObject(0, mitkToolData1);
    CPPUNIT_ASSERT_MESSAGE( "Testing SetRepresentationObject()/GetRepresentationObject() node 1", myFilter->GetRepresentationObject(0) == mitkToolData1);
    CPPUNIT_ASSERT_MESSAGE("Testing GetNumberOfToolRepresentations() after adding first tool", myFilter->GetNumberOfToolRepresentations() == 1);

    myFilter->SetRepresentationObject(1, mitkToolData2);
    CPPUNIT_ASSERT_MESSAGE( "Testing SetRepresentationObject() node 2", myFilter->GetRepresentationObject(1) == mitkToolData2);
    CPPUNIT_ASSERT_MESSAGE( "Testing GetNumberOfToolRepresentations() after adding second tool", myFilter->GetNumberOfToolRepresentations() == 2);
    //getting nodes
    CPPUNIT_ASSERT_MESSAGE("Testing GetRepresentationObject() node 1", myFilter->GetRepresentationObject(0) == mitkToolData1);
    CPPUNIT_ASSERT_MESSAGE( "Testing GetRepresentationObject() != Dummy node", myFilter->GetRepresentationObject(0) != mitkToolDataDummy);
    CPPUNIT_ASSERT_MESSAGE( "Testing GetRepresentationObject() node 2", myFilter->GetRepresentationObject(1) == mitkToolData2);
    CPPUNIT_ASSERT_MESSAGE( "Testing GetRepresentationObject() != Dummy node", myFilter->GetRepresentationObject(1) != mitkToolDataDummy);
    CPPUNIT_ASSERT_MESSAGE("Testing GetRepresentationObject() with out of range parameter", myFilter->GetRepresentationObject(111) == NULL);
  }

  void TestTransforms(){
    myFilter->SetRepresentationObject(0, mitkToolData1);
    myFilter->SetRepresentationObject(1, mitkToolData2);
    //Process
    myFilter->Update();

    affineTransform1 = mitkToolData1->GetGeometry()->GetIndexToWorldTransform();
    offset1 = affineTransform1->GetOffset();
    affineTransform2 = mitkToolData2->GetGeometry()->GetIndexToWorldTransform();
    offset2 = affineTransform2->GetOffset();

    m1 = affineTransform1->GetMatrix().GetVnlMatrix();
    m2 = affineTransform2->GetMatrix().GetVnlMatrix();

    //now check it there are data connected to the nodes with the according orientation and offsets
    CPPUNIT_ASSERT_MESSAGE("Testing Offset position 1", offset1.GetVnlVector()==initialPos1.GetVnlVector());
    CPPUNIT_ASSERT_MESSAGE("Testing Offset position 2", offset2.GetVnlVector()==initialPos2.GetVnlVector());

    MITK_TEST_OUTPUT( << "\n initOrient1="<<initialOri1<<" affineTransform1->GetVnlMatrix():\n "<< m1);
    MITK_TEST_OUTPUT( << "\n initOrient2=" << initialOri2 << " affineTransform2->GetVnlMatrix():\n " << m2);
  }

  void TestMessWithRepresentationObjects(){
    myFilter->SetRepresentationObject(0, mitkToolData1);
    myFilter->SetRepresentationObject(1, mitkToolData2);
    //Process
    myFilter->Update();

    affineTransform1 = mitkToolData1->GetGeometry()->GetIndexToWorldTransform();
    offset1 = affineTransform1->GetOffset();
    affineTransform2 = mitkToolData2->GetGeometry()->GetIndexToWorldTransform();
    offset2 = affineTransform2->GetOffset();

    m1 = affineTransform1->GetMatrix().GetVnlMatrix();
    m2 = affineTransform2->GetMatrix().GetVnlMatrix();

    //messing with SetRepresentationObject
    //setting nodes
    myFilter->SetRepresentationObject(0, mitkToolData2);
    CPPUNIT_ASSERT_MESSAGE("Twisting mitkToolData by using SetRepresentationObject() NavigationData 1 with ToolData 2", myFilter->GetRepresentationObject(0) == mitkToolData2);
    CPPUNIT_ASSERT_MESSAGE( "Testing GetNumberOfToolRepresentations() == 1", myFilter->GetNumberOfToolRepresentations() == 2);

    myFilter->SetRepresentationObject(1, mitkToolData1);
    CPPUNIT_ASSERT_MESSAGE("Twisting mitkToolData by using SetRepresentationObject() NavigationData 2 with ToolData 1", myFilter->GetRepresentationObject(1) == mitkToolData1);
    CPPUNIT_ASSERT_MESSAGE( "Testing GetNumberOfToolRepresentations() == 2", myFilter->GetNumberOfToolRepresentations() == 2);

    //getting nodes
    CPPUNIT_ASSERT_MESSAGE("Testing switched BaseData of NavigationData 1 ", myFilter->GetRepresentationObject(0) == mitkToolData2);
    CPPUNIT_ASSERT_MESSAGE("Testing GetRepresentationObject() != Dummy node", myFilter->GetRepresentationObject(0) != mitkToolDataDummy);
    CPPUNIT_ASSERT_MESSAGE("Testing switched BaseData NavigationData 2", myFilter->GetRepresentationObject(1) == mitkToolData1);
    CPPUNIT_ASSERT_MESSAGE("Testing GetRepresentationObject() != Dummy node", myFilter->GetRepresentationObject(1) != mitkToolDataDummy);

    //processing update through pipeline
    myFilter->Update();

    //now check it there are data connected to the nodes with the according orientation and offsets
    mitk::AffineTransform3D::Pointer affineTransform1Second = mitkToolData1->GetGeometry()->GetIndexToWorldTransform();
    CPPUNIT_ASSERT_MESSAGE( "Testing affineTransform1 after second update", affineTransform1 == affineTransform1Second);
    mitk::AffineTransform3D::OutputVectorType offset1Second = affineTransform1->GetOffset();
    CPPUNIT_ASSERT_MESSAGE( "Testing offset1 after second update", offset1 == offset1Second);
    CPPUNIT_ASSERT_MESSAGE("Testing offset1 equals first update", offset1Second.GetVnlVector()==offset1.GetVnlVector());

    mitk::AffineTransform3D::Pointer affineTransform2Second = mitkToolData2->GetGeometry()->GetIndexToWorldTransform();
    CPPUNIT_ASSERT_MESSAGE("Testing affineTransform2 after second update", affineTransform2 == affineTransform2Second);
    mitk::AffineTransform3D::OutputVectorType offset2Second = affineTransform2->GetOffset();
    CPPUNIT_ASSERT_MESSAGE("Testing offset2 after second update", offset2 == offset2Second);
    CPPUNIT_ASSERT_MESSAGE("Testing offset2 equals first update", offset2Second.GetVnlVector()==offset2.GetVnlVector());

    mitk::AffineTransform3D::MatrixType::InternalMatrixType m1Second= affineTransform1Second->GetMatrix().GetVnlMatrix();
    MITK_TEST_OUTPUT( <<"\n after second update initOrient1="<<initialOri1<<" affineTransform1->GetVnlMatrix():\n "<< m1Second);

    mitk::AffineTransform3D::MatrixType::InternalMatrixType m2Second= affineTransform2Second->GetMatrix().GetVnlMatrix();
    MITK_TEST_OUTPUT( << "\n after second update initOrient2="<<initialOri2<<" affineTransform2->GetVnlMatrix():\n "<< m2Second);
  }

  void TestThirdInput(){
    myFilter->SetRepresentationObject(0, mitkToolData1);
    myFilter->SetRepresentationObject(1, mitkToolData2);
    //Process
    myFilter->Update();

    //testing adding a third input
    myFilter->SetInput(2,ndDummy);
    CPPUNIT_ASSERT_MESSAGE("Adding new input and testing GetNumberOfInputs == 3", myFilter->GetNumberOfInputs() == 3);
    CPPUNIT_ASSERT_MESSAGE("testing GetNumberOfOutputs == 3", myFilter->GetNumberOfOutputs() == 3);
    CPPUNIT_ASSERT_MESSAGE("Testing Input == newly added input", myFilter->GetInput(2) == ndDummy);
    CPPUNIT_ASSERT_MESSAGE("Testing GetOutput(2) != NULL", myFilter->GetOutput(2) != NULL);
    CPPUNIT_ASSERT_MESSAGE( "Testing GetOutput(2) != GetOutput(1)", myFilter->GetOutput(2) != myFilter->GetOutput(1));
    myFilter->SetRepresentationObject(2, mitkToolDataDummy);
    CPPUNIT_ASSERT_MESSAGE("Testing GetNumberOfToolRepresentations() after adding latest tool", myFilter->GetNumberOfToolRepresentations() == 3);
    CPPUNIT_ASSERT_MESSAGE("Testing Set-/GetRepresentationObject() equals was set", myFilter->GetRepresentationObject(2) == mitkToolDataDummy);

    //last time processing update through pipeline
    myFilter->Update();

    //now check for the new values
    mitk::AffineTransform3D::Pointer affineTransformDummy = mitkToolDataDummy->GetGeometry()->GetIndexToWorldTransform();
    mitk::AffineTransform3D::OutputVectorType offsetDummy = affineTransformDummy->GetOffset();
    CPPUNIT_ASSERT_MESSAGE("Testing Offset latest added tool", offsetDummy.GetVnlVector()==initialPosDummy.GetVnlVector());

    mitk::AffineTransform3D::MatrixType::InternalMatrixType m1Latest= affineTransformDummy->GetMatrix().GetVnlMatrix();
    MITK_TEST_OUTPUT( << "\n latest initOrient="<<initialOriDummy<<" latest affineTransform->GetVnlMatrix():\n "<< m1Latest);

    mitk::Surface::Pointer anotherSurface = mitk::Surface::New();
    myFilter->SetRepresentationObject(0, anotherSurface);
    CPPUNIT_ASSERT_MESSAGE("Overwriting BaseData index 0", myFilter->GetRepresentationObject(0) == anotherSurface);
  }

  void TestSetTransformPosition(){
    // test Set/GetTransformPosition()
    myFilter->SetTransformPosition(0,true);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformPosition(0,true)", myFilter->GetTransformPosition(0)==true);
    myFilter->SetTransformPosition(1,true);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformPosition(1,true)", myFilter->GetTransformPosition(1)==true);
    myFilter->SetTransformPosition(2,true);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformPosition(2,true)", myFilter->GetTransformPosition(2)==true);
    myFilter->SetTransformPosition(3,true);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformPosition(3,true)", myFilter->GetTransformPosition(3)==true);

    myFilter->SetTransformPosition(0,false);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformPosition(0,false)", myFilter->GetTransformPosition(0)==false);
    myFilter->SetTransformPosition(1,false);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformPosition(1,false)", myFilter->GetTransformPosition(1)==false);
    myFilter->SetTransformPosition(2,false);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformPosition(2,false)", myFilter->GetTransformPosition(2)==false);
    myFilter->SetTransformPosition(3,false);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformPosition(3,false)", myFilter->GetTransformPosition(3)==false);
  }

  void TestSetTransformOrientation(){
    // test Set/GetTransformOrientation()
    myFilter->SetTransformOrientation(0,true);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformOrientation(0,true)", myFilter->GetTransformOrientation(0)==true);
    myFilter->SetTransformOrientation(1,true);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformOrientation(1,true)", myFilter->GetTransformOrientation(1)==true);
    myFilter->SetTransformOrientation(2,true);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformOrientation(2,true)", myFilter->GetTransformOrientation(2)==true);
    myFilter->SetTransformOrientation(3,true);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformOrientation(3,true)", myFilter->GetTransformOrientation(3)==true);

    myFilter->SetTransformOrientation(0,false);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformOrientation(0,false)", myFilter->GetTransformOrientation(0)==false);
    myFilter->SetTransformOrientation(1,false);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformOrientation(1,false)", myFilter->GetTransformOrientation(1)==false);
    myFilter->SetTransformOrientation(2,false);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformOrientation(2,false)", myFilter->GetTransformOrientation(2)==false);
    myFilter->SetTransformOrientation(3,false);
    CPPUNIT_ASSERT_MESSAGE("test Set/GetTransformOrientation(3,false)", myFilter->GetTransformOrientation(3)==false);
  }

  void TestConvenienceSetTransformOrientation(){
    // test the convenience methods to set/getTransformOrientation/Position
    myFilter->TransformOrientationOn(0);
    CPPUNIT_ASSERT_MESSAGE("test TransformOrientationOn()", myFilter->GetTransformOrientation(0)==true);
    myFilter->TransformOrientationOff(0);
    CPPUNIT_ASSERT_MESSAGE("test TransformOrientationOff()", myFilter->GetTransformOrientation(0)==false);
    myFilter->TransformOrientationOff(1);
    CPPUNIT_ASSERT_MESSAGE("test TransformOrientationOff()", myFilter->GetTransformOrientation(1)==false);
    myFilter->TransformOrientationOn(1);
    CPPUNIT_ASSERT_MESSAGE("test TransformOrientationOn()", myFilter->GetTransformOrientation(1)==true);
    myFilter->TransformOrientationOn(2);
    CPPUNIT_ASSERT_MESSAGE("test TransformOrientationOn()", myFilter->GetTransformOrientation(2)==true);
    myFilter->TransformOrientationOff(2);
    CPPUNIT_ASSERT_MESSAGE("test TransformOrientationOff()", myFilter->GetTransformOrientation(2)==false);
    myFilter->TransformOrientationOn(3);
    CPPUNIT_ASSERT_MESSAGE("test TransformOrientationOn()", myFilter->GetTransformOrientation(3)==true);
    myFilter->TransformOrientationOff(3);
    CPPUNIT_ASSERT_MESSAGE("test TransformOrientationOff()", myFilter->GetTransformOrientation(3)==false);

    myFilter->TransformPositionOn(0);
    CPPUNIT_ASSERT_MESSAGE("test TransformPositionOn()", myFilter->GetTransformPosition(0)==true);
    myFilter->TransformPositionOff(0);
    CPPUNIT_ASSERT_MESSAGE("test TransformPositionOff()", myFilter->GetTransformPosition(0)==false);
    myFilter->TransformPositionOff(1);
    CPPUNIT_ASSERT_MESSAGE("test TransformPositionOff()", myFilter->GetTransformPosition(1)==false);
    myFilter->TransformPositionOn(1);
    CPPUNIT_ASSERT_MESSAGE("test TransformPositionOn()", myFilter->GetTransformPosition(1)==true);
    myFilter->TransformPositionOn(2);
    CPPUNIT_ASSERT_MESSAGE("test TransformPositionOn()", myFilter->GetTransformPosition(2)==true);
    myFilter->TransformPositionOff(2);
    CPPUNIT_ASSERT_MESSAGE("test TransformPositionOff()", myFilter->GetTransformPosition(2)==false);
    myFilter->TransformPositionOn(3);
    CPPUNIT_ASSERT_MESSAGE("test TransformPositionOn()", myFilter->GetTransformPosition(3)==true);
    myFilter->TransformPositionOff(3);
    CPPUNIT_ASSERT_MESSAGE("test TransformPositionOff()", myFilter->GetTransformPosition(3)==false);
  }

  void TestUpdateOrientation(){
    // update position and orientation
    mitk::NavigationData::PositionType updatedPos1, updatedPos2, zero;
    mitk::FillVector3D(updatedPos1, 3.2, 1.5, 2.8);
    mitk::FillVector3D(updatedPos2, 4.3, 5.2, 6.0);
    mitk::FillVector3D(zero, 0.0, 0.0, 0.0);
    mitk::NavigationData::OrientationType updatedOri1(0.7, 0.5, 0.1, 0.4);
    mitk::NavigationData::OrientationType updatedOri2(0.2, 0.7, 0.6, 0.1);

    updatedOri1.normalize();
    updatedOri2.normalize();

    nd1->SetPosition(updatedPos1);
    nd1->SetOrientation(updatedOri1);

    nd2->SetPosition(updatedPos2);
    nd2->SetOrientation(updatedOri2);

    myFilter->SetRepresentationObject(0,mitkToolData1);
    myFilter->SetRepresentationObject(1,mitkToolData2);

    myFilter->TransformPositionOn(0);
    myFilter->TransformOrientationOff(0);
    myFilter->TransformPositionOff(1);
    myFilter->TransformOrientationOn(1);

    myFilter->Update();

    // test positions and orientations
    mitk::AffineTransform3D::Pointer updatedAffineTransform1 = mitkToolData1->GetGeometry()->GetIndexToWorldTransform();
    mitk::AffineTransform3D::OutputVectorType updatedOffset1 = updatedAffineTransform1->GetOffset();
    CPPUNIT_ASSERT_MESSAGE("Testing updated position 1", mitk::Equal(updatedOffset1.GetVnlVector(),updatedPos1.GetVnlVector()));

    mitk::AffineTransform3D::Pointer updatedAffineTransform2 = mitkToolData2->GetGeometry()->GetIndexToWorldTransform();
    mitk::AffineTransform3D::OutputVectorType updatedOffset2 = updatedAffineTransform2->GetOffset();
    CPPUNIT_ASSERT_MESSAGE( "Testing updated position 2", mitk::Equal(updatedOffset2.GetVnlVector(),zero.GetVnlVector()));

    mitk::AffineTransform3D::Pointer identityTransform = mitk::AffineTransform3D::New();
    identityTransform->SetIdentity();
    mitk::AffineTransform3D::MatrixType identityMatrix = identityTransform->GetMatrix();
    mitk::AffineTransform3D::MatrixType uM1 = updatedAffineTransform1->GetMatrix();

    CPPUNIT_ASSERT_MESSAGE( "Testing updated orientation 1", mitk::MatrixEqualElementWise(uM1, identityMatrix));

    mitk::AffineTransform3D::MatrixType::InternalMatrixType uM2 = updatedAffineTransform2->GetMatrix().GetVnlMatrix();
    mitk::AffineTransform3D::MatrixType::InternalMatrixType updatedOriTransform2 = updatedOri2.rotation_matrix_transpose().transpose();

    CPPUNIT_ASSERT_MESSAGE("Testing updated orientation 2", mitk::MatrixEqualElementWise(uM2,updatedOriTransform2));

    // Test that the second RepresentationObject is updated properly even when
    // the first RepresentationObject is invalid
    nd2->Modified();
    myFilter->SetRepresentationObject(0, NULL);
    mitkToolData2->GetGeometry()->SetIdentity();
    myFilter->Update();
    CPPUNIT_ASSERT_MESSAGE( "Test that the second repr object is updated correctly when the first repr object is invalid",
      mitk::MatrixEqualElementWise(mitkToolData2->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix(),
      updatedOri2.rotation_matrix_transpose().transpose()));
  }
};//end class mitkNavigationDataObjectVisualizationFilterTestSuite

MITK_TEST_SUITE_REGISTRATION(mitkNavigationDataObjectVisualizationFilter)
