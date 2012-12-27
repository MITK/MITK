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

#include <vtkTransform.h>
#include <iostream>

#include "mitkSurface.h"


/**Documentation
 *  test for the class "NavigationDataObjectVisualizationFilter".
 */
int mitkNavigationDataObjectVisualizationFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataObjectVisualizationFilter")

  // let's create an object of our class
  mitk::NavigationDataObjectVisualizationFilter::Pointer myFilter = mitk::NavigationDataObjectVisualizationFilter::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myFilter.IsNotNull(),"Testing instantiation");

  /* create helper objects: navigation data with position as origin, zero quaternion, zero error and data valid */
  mitk::NavigationData::PositionType initialPos1, initialPos2;
  mitk::FillVector3D(initialPos1, 1.1, 2.2, 3.3);
  mitk::FillVector3D(initialPos2, 5.0, 6.0, 7.0);
  mitk::NavigationData::OrientationType initialOri1(0.1, 0.2, 0.3, 0.4);
  mitk::NavigationData::OrientationType initialOri2(0.5, 0.6, 0.7, 0.8);
  mitk::ScalarType initialError1(0.0);
  mitk::ScalarType initialError2(5.0);
  bool initialValid1(true);
  bool initialValid2(true);

  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->SetPosition(initialPos1);
  nd1->SetOrientation(initialOri1);
  nd1->SetPositionAccuracy(initialError1);
  nd1->SetDataValid(initialValid1);

  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
  nd2->SetPosition(initialPos2);
  nd2->SetOrientation(initialOri2);
  nd2->SetPositionAccuracy(initialError2);
  nd2->SetDataValid(initialValid2);

  myFilter->SetInput(nd1);
  myFilter->SetInput(1, nd2);

  //testing the input
  MITK_TEST_CONDITION(myFilter->GetInput() == nd1, "Testing Set-/GetInput() input 1 without index");
  MITK_TEST_CONDITION(myFilter->GetInput(0) == nd1, "Testing Set-/GetInput() input 1");
  MITK_TEST_CONDITION(myFilter->GetInput(1) == nd2, "Testing Set-/GetInput() input 2");
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 0, "Testing GetNumberOfToolRepresentations()");

  //testing getting the output
  mitk::NavigationData* output = myFilter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(output != NULL, "Testing GetOutput()");
  MITK_TEST_CONDITION_REQUIRED(output == myFilter->GetOutput(0), "Testing GetOutput() == GetOutput(0)");
  MITK_TEST_CONDITION_REQUIRED(output != myFilter->GetOutput(1), "Testing GetOutput() != GetOutput(1)");

  // Test setting BaseData
  mitk::Surface::Pointer mitkToolData1 = mitk::Surface::New();

  mitk::Surface::Pointer mitkToolData2 = mitk::Surface::New();

  //dummy for test; will not be set but used to test find
  mitk::Surface::Pointer mitkToolDataDummy = mitk::Surface::New();
  //and the Dummy NavigationData for this
  mitk::NavigationData::PositionType initialPosDummy;
  mitk::FillVector3D(initialPosDummy, 8.8, 9.9, 10.10);
  mitk::NavigationData::OrientationType initialOriDummy(1.1, 2.2, 3.3, 4.4);
  mitk::ScalarType initialErrorDummy(10.0);
  bool initialValidDummy(true);
  mitk::NavigationData::Pointer ndDummy = mitk::NavigationData::New();
  ndDummy->SetPosition(initialPosDummy);
  ndDummy->SetOrientation(initialOriDummy);
  ndDummy->SetPositionAccuracy(initialErrorDummy);
  ndDummy->SetDataValid(initialValidDummy);
  //now we have ndDummy and mitkToolDataDummy to test with

  //setting nodes
  myFilter->SetRepresentationObject(0, mitkToolData1);
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(0) == mitkToolData1, "Testing SetRepresentationObject()/GetRepresentationObject() node 1");
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 1, "Testing GetNumberOfToolRepresentations() after adding first tool");
  myFilter->SetRepresentationObject(1, mitkToolData2);
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(1) == mitkToolData2, "Testing SetRepresentationObject() node 2");
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 2, "Testing GetNumberOfToolRepresentations() after adding second tool");
  //getting nodes
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(0) == mitkToolData1, "Testing GetRepresentationObject() node 1");
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(0) != mitkToolDataDummy, "Testing GetRepresentationObject() != Dummy node");
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(1) == mitkToolData2, "Testing GetRepresentationObject() node 2");
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(1) != mitkToolDataDummy, "Testing GetRepresentationObject() != Dummy node");
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(111) == NULL, "Testing GetRepresentationObject() with out of range parameter");

  //Process
  myFilter->Update();

  //now check it there are data connected to the nodes with the according orientation and offsets
  mitk::AffineTransform3D::Pointer affineTransform1 = mitkToolData1->GetGeometry()->GetIndexToWorldTransform();
  mitk::AffineTransform3D::OutputVectorType offset1 = affineTransform1->GetOffset();
  MITK_TEST_CONDITION(offset1.Get_vnl_vector()==initialPos1.Get_vnl_vector(), "Testing Offset position 1");

  mitk::AffineTransform3D::Pointer affineTransform2 = mitkToolData2->GetGeometry()->GetIndexToWorldTransform();
  mitk::AffineTransform3D::OutputVectorType offset2 = affineTransform2->GetOffset();
  MITK_TEST_CONDITION(offset2.Get_vnl_vector()==initialPos2.Get_vnl_vector(), "Testing Offset position 2");

  mitk::AffineTransform3D::MatrixType::InternalMatrixType m1 = affineTransform1->GetMatrix().GetVnlMatrix();
  MITK_TEST_OUTPUT( << "\n initOrient1="<<initialOri1<<" affineTransform1->GetVnlMatrix():\n "<< m1);

  mitk::AffineTransform3D::MatrixType::InternalMatrixType m2 = affineTransform2->GetMatrix().GetVnlMatrix();
  MITK_TEST_OUTPUT( << "\n initOrient2=" << initialOri2 << " affineTransform2->GetVnlMatrix():\n " << m2);


//messing with SetRepresentationObject
//setting nodes
  myFilter->SetRepresentationObject(0, mitkToolData2);
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(0) == mitkToolData2, "Twisting mitkToolData by using SetRepresentationObject() NavigationData 1 with ToolData 2");
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 2, "Testing GetNumberOfToolRepresentations() == 1");
  myFilter->SetRepresentationObject(1, mitkToolData1);
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(1) == mitkToolData1, "Twisting mitkToolData by using SetRepresentationObject() NavigationData 2 with ToolData 1");
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 2, "Testing GetNumberOfToolRepresentations() == 2");
  //getting nodes
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(0) == mitkToolData2, "Testing switched BaseData of NavigationData 1 ");
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(0) != mitkToolDataDummy, "Testing GetRepresentationObject() != Dummy node");
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(1) == mitkToolData1, "Testing switched BaseData NavigationData 2");
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(1) != mitkToolDataDummy, "Testing GetRepresentationObject() != Dummy node");

  //processing update through pipeline
  myFilter->Update();

  //now check it there are data connected to the nodes with the according orientation and offsets
  mitk::AffineTransform3D::Pointer affineTransform1Second = mitkToolData1->GetGeometry()->GetIndexToWorldTransform();
  MITK_TEST_CONDITION(affineTransform1 == affineTransform1Second, "Testing affineTransform1 after second update");
  mitk::AffineTransform3D::OutputVectorType offset1Second = affineTransform1->GetOffset();
  MITK_TEST_CONDITION(offset1 == offset1Second, "Testing offset1 after second update");
  MITK_TEST_CONDITION(offset1Second.Get_vnl_vector()==offset1.Get_vnl_vector(), "Testing offset1 equals first update");

  mitk::AffineTransform3D::Pointer affineTransform2Second = mitkToolData2->GetGeometry()->GetIndexToWorldTransform();
  MITK_TEST_CONDITION(affineTransform2 == affineTransform2Second, "Testing affineTransform2 after second update");
  mitk::AffineTransform3D::OutputVectorType offset2Second = affineTransform2->GetOffset();
  MITK_TEST_CONDITION(offset2 == offset2Second, "Testing offset2 after second update");
  MITK_TEST_CONDITION(offset2Second.Get_vnl_vector()==offset2.Get_vnl_vector(), "Testing offset2 equals first update");

  mitk::AffineTransform3D::MatrixType::InternalMatrixType m1Second= affineTransform1Second->GetMatrix().GetVnlMatrix();
  MITK_TEST_OUTPUT( <<"\n after second update initOrient1="<<initialOri1<<" affineTransform1->GetVnlMatrix():\n "<< m1Second);

  mitk::AffineTransform3D::MatrixType::InternalMatrixType m2Second= affineTransform2Second->GetMatrix().GetVnlMatrix();
  MITK_TEST_OUTPUT( << "\n after second update initOrient2="<<initialOri2<<" affineTransform2->GetVnlMatrix():\n "<< m2Second);

  //testing adding a third input
  myFilter->SetInput(2,ndDummy);
  MITK_TEST_CONDITION(myFilter->GetNumberOfInputs() == 3, "Adding new input and testing GetNumberOfInputs == 3");
  MITK_TEST_CONDITION(myFilter->GetNumberOfOutputs() == 3, "testing GetNumberOfOutputs == 3");
  MITK_TEST_CONDITION(myFilter->GetInput(2) == ndDummy, "Testing Input == newly added input");
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetOutput(2) != NULL, "Testing GetOutput(2) != NULL");
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetOutput(2) != myFilter->GetOutput(1), "Testing GetOutput(2) != GetOutput(1)");
  myFilter->SetRepresentationObject(2, mitkToolDataDummy);
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 3, "Testing GetNumberOfToolRepresentations() after adding latest tool");
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(2) == mitkToolDataDummy, "Testing Set-/GetRepresentationObject() equals was set");

  //last time processing update through pipeline
  myFilter->Update();

  //now check for the new values
  mitk::AffineTransform3D::Pointer affineTransformDummy = mitkToolDataDummy->GetGeometry()->GetIndexToWorldTransform();
  mitk::AffineTransform3D::OutputVectorType offsetDummy = affineTransformDummy->GetOffset();
  MITK_TEST_CONDITION(offsetDummy.Get_vnl_vector()==initialPosDummy.Get_vnl_vector(), "Testing Offset latest added tool");

  mitk::AffineTransform3D::MatrixType::InternalMatrixType m1Latest= affineTransformDummy->GetMatrix().GetVnlMatrix();
  MITK_TEST_OUTPUT( << "\n latest initOrient="<<initialOriDummy<<" latest affineTransform->GetVnlMatrix():\n "<< m1Latest);

  mitk::Surface::Pointer anotherSurface = mitk::Surface::New();
  myFilter->SetRepresentationObject(0, anotherSurface);
  MITK_TEST_CONDITION(myFilter->GetRepresentationObject(0) == anotherSurface, "Overwriting BaseData index 0");

  // test Set/GetTransformPosition()
  myFilter->SetTransformPosition(0,true);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(0)==true,"test Set/GetTransformPosition(0,true)");
  myFilter->SetTransformPosition(1,true);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(1)==true,"test Set/GetTransformPosition(1,true)");
  myFilter->SetTransformPosition(2,true);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(2)==true,"test Set/GetTransformPosition(2,true)");
  myFilter->SetTransformPosition(3,true);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(3)==true,"test Set/GetTransformPosition(3,true)");

  myFilter->SetTransformPosition(0,false);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(0)==false,"test Set/GetTransformPosition(0,false)");
  myFilter->SetTransformPosition(1,false);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(1)==false,"test Set/GetTransformPosition(1,false)");
  myFilter->SetTransformPosition(2,false);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(2)==false,"test Set/GetTransformPosition(2,false)");
  myFilter->SetTransformPosition(3,false);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(3)==false,"test Set/GetTransformPosition(3,false)");

  // test Set/GetTransformOrientation()
  myFilter->SetTransformOrientation(0,true);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(0)==true,"test Set/GetTransformOrientation(0,true)");
  myFilter->SetTransformOrientation(1,true);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(1)==true,"test Set/GetTransformOrientation(1,true)");
  myFilter->SetTransformOrientation(2,true);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(2)==true,"test Set/GetTransformOrientation(2,true)");
  myFilter->SetTransformOrientation(3,true);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(3)==true,"test Set/GetTransformOrientation(3,true)");

  myFilter->SetTransformOrientation(0,false);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(0)==false,"test Set/GetTransformOrientation(0,false)");
  myFilter->SetTransformOrientation(1,false);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(1)==false,"test Set/GetTransformOrientation(1,false)");
  myFilter->SetTransformOrientation(2,false);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(2)==false,"test Set/GetTransformOrientation(2,false)");
  myFilter->SetTransformOrientation(3,false);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(3)==false,"test Set/GetTransformOrientation(3,false)");

  // test the convenience methods to set/getTransformOrientation/Position
  myFilter->TransformOrientationOn(0);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(0)==true,"test TransformOrientationOn()");
  myFilter->TransformOrientationOff(0);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(0)==false,"test TransformOrientationOff()");
  myFilter->TransformOrientationOff(1);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(1)==false,"test TransformOrientationOff()");
  myFilter->TransformOrientationOn(1);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(1)==true,"test TransformOrientationOn()");
  myFilter->TransformOrientationOn(2);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(2)==true,"test TransformOrientationOn()");
  myFilter->TransformOrientationOff(2);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(2)==false,"test TransformOrientationOff()");
  myFilter->TransformOrientationOn(3);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(3)==true,"test TransformOrientationOn()");
  myFilter->TransformOrientationOff(3);
  MITK_TEST_CONDITION(myFilter->GetTransformOrientation(3)==false,"test TransformOrientationOff()");

  myFilter->TransformPositionOn(0);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(0)==true,"test TransformPositionOn()");
  myFilter->TransformPositionOff(0);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(0)==false,"test TransformPositionOff()");
  myFilter->TransformPositionOff(1);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(1)==false,"test TransformPositionOff()");
  myFilter->TransformPositionOn(1);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(1)==true,"test TransformPositionOn()");
  myFilter->TransformPositionOn(2);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(2)==true,"test TransformPositionOn()");
  myFilter->TransformPositionOff(2);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(2)==false,"test TransformPositionOff()");
  myFilter->TransformPositionOn(3);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(3)==true,"test TransformPositionOn()");
  myFilter->TransformPositionOff(3);
  MITK_TEST_CONDITION(myFilter->GetTransformPosition(3)==false,"test TransformPositionOff()");
  // update position and orientation
  mitk::NavigationData::PositionType updatedPos1, updatedPos2, zero;
  mitk::FillVector3D(updatedPos1, 3.2, 1.5, 2.8);
  mitk::FillVector3D(updatedPos2, 4.3, 5.2, 6.0);
  mitk::FillVector3D(zero, 0.0, 0.0, 0.0);
  mitk::NavigationData::OrientationType updatedOri1(0.7, 0.5, 0.1, 0.4);
  mitk::NavigationData::OrientationType updatedOri2(0.2, 0.7, 0.6, 0.1);
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
  MITK_TEST_CONDITION(mitk::Equal(updatedOffset1.Get_vnl_vector(),updatedPos1.Get_vnl_vector()), "Testing updated position 1");
  mitk::AffineTransform3D::Pointer updatedAffineTransform2 = mitkToolData2->GetGeometry()->GetIndexToWorldTransform();
  mitk::AffineTransform3D::OutputVectorType updatedOffset2 = updatedAffineTransform2->GetOffset();
  MITK_TEST_CONDITION(mitk::Equal(updatedOffset2.Get_vnl_vector(),zero.Get_vnl_vector()), "Testing updated position 2");

  mitk::AffineTransform3D::Pointer identityTransform = mitk::AffineTransform3D::New();
  identityTransform->SetIdentity();
  mitk::AffineTransform3D::MatrixType identityMatrix = identityTransform->GetMatrix();
  mitk::AffineTransform3D::MatrixType uM1 = updatedAffineTransform1->GetMatrix();
  MITK_TEST_CONDITION(mitk::MatrixEqualElementWise(uM1,identityMatrix), "Testing updated orientation 1");
  mitk::AffineTransform3D::MatrixType::InternalMatrixType uM2 = updatedAffineTransform2->GetMatrix().GetVnlMatrix();
  MITK_TEST_CONDITION(mitk::MatrixEqualElementWise(uM2,updatedOri2.rotation_matrix_transpose().transpose()), "Testing updated orientation 2");

  // always end with this!
  MITK_TEST_END();
}
