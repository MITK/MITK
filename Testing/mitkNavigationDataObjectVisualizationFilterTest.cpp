/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-02-02 18:48:48 +0100 (Mo, 02 Feb 2009) $
Version:   $Revision: 16010 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataObjectVisualizationFilter.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"

#include <vtkTransform.h>
#include <iostream>

#include <vtkConeSource.h>
#include <mitkCone.h>


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
  mitk::Cone::Pointer mitkToolData1 = mitk::Cone::New();

  mitk::Cone::Pointer mitkToolData2 = mitk::Cone::New();

  //dummy for test; will not be set but used to test find
  mitk::Cone::Pointer mitkToolDataDummy = mitk::Cone::New();
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
  MITK_TEST_CONDITION(myFilter->SetBaseData(0, mitkToolData1), "Testing SetBaseData() node 1");
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 1, "Testing GetNumberOfToolRepresentations() after adding first tool");
  MITK_TEST_CONDITION(myFilter->SetBaseData(1, mitkToolData2), "Testing SetBaseData() node 2");
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 2, "Testing GetNumberOfToolRepresentations() after adding second tool");
  //getting nodes
  MITK_TEST_CONDITION(myFilter->GetBaseData(0) == mitkToolData1, "Testing GetBaseData() node 1");
  MITK_TEST_CONDITION(myFilter->GetBaseData(0) != mitkToolDataDummy, "Testing GetBaseData() != Dummy node");
  MITK_TEST_CONDITION(myFilter->GetBaseData(1) == mitkToolData2, "Testing GetBaseData() node 2");
  MITK_TEST_CONDITION(myFilter->GetBaseData(1) != mitkToolDataDummy, "Testing GetBaseData() != Dummy node");
  MITK_TEST_CONDITION(myFilter->GetBaseData(111) == NULL, "Testing GetBaseData() with out of range parameter");
  
  //Process
  myFilter->Update();

  //now check it there are data connected to the nodes with the according orientation and offsets
  mitk::AffineTransform3D::Pointer affineTransform1 = mitkToolData1->GetGeometry()->GetIndexToWorldTransform();
  mitk::AffineTransform3D::OutputVectorType offset1 = affineTransform1->GetOffset();
  MITK_TEST_CONDITION(offset1.Get_vnl_vector()==initialPos1.Get_vnl_vector(), "Testing Offset position 1");
  
  mitk::AffineTransform3D::Pointer affineTransform2 = mitkToolData2->GetGeometry()->GetIndexToWorldTransform();
  mitk::AffineTransform3D::OutputVectorType offset2 = affineTransform2->GetOffset();
  MITK_TEST_CONDITION(offset2.Get_vnl_vector()==initialPos2.Get_vnl_vector(), "Testing Offset position 2");
  
  mitk::AffineTransform3D::MatrixType::InternalMatrixType m1= affineTransform1->GetMatrix().GetVnlMatrix();
  std::cout<<"\n initOrient1="<<initialOri1<<" affineTransform1->GetVnlMatrix():\n "<< m1 <<"\n";
  
  mitk::AffineTransform3D::MatrixType::InternalMatrixType m2= affineTransform2->GetMatrix().GetVnlMatrix();
  std::cout<<"\n initOrient2="<<initialOri2<<" affineTransform2->GetVnlMatrix():\n "<< m2 <<"\n";
  

//messing with SetBaseData
//setting nodes
  myFilter->Clear();
  std::cout<<"Clearing BaseData\n";

  MITK_TEST_CONDITION(myFilter->SetBaseData(0, mitkToolData2), "Twisting mitkToolData by using SetBaseData() NavigationData 1 with ToolData 2");
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 1, "Testing GetNumberOfToolRepresentations() == 1");
  MITK_TEST_CONDITION(myFilter->SetBaseData(1, mitkToolData1), "Twisting mitkToolData by using SetBaseData() NavigationData 2 with ToolData 1");
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 2, "Testing GetNumberOfToolRepresentations() == 2");
  //getting nodes
  MITK_TEST_CONDITION(myFilter->GetBaseData(0) == mitkToolData2, "Testing switched BaseData of NavigationData 1 ");
  MITK_TEST_CONDITION(myFilter->GetBaseData(0) != mitkToolDataDummy, "Testing GetBaseData() != Dummy node");
  MITK_TEST_CONDITION(myFilter->GetBaseData(1) == mitkToolData1, "Testing switched BaseData NavigationData 2");
  MITK_TEST_CONDITION(myFilter->GetBaseData(1) != mitkToolDataDummy, "Testing GetBaseData() != Dummy node");

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
  std::cout<<"\n after second update initOrient1="<<initialOri1<<" affineTransform1->GetVnlMatrix():\n "<< m1Second <<"\n";
  
  mitk::AffineTransform3D::MatrixType::InternalMatrixType m2Second= affineTransform2Second->GetMatrix().GetVnlMatrix();
  std::cout<<"\n after second update initOrient2="<<initialOri2<<" affineTransform2->GetVnlMatrix():\n "<< m2Second <<"\n";

  //testing adding a third input
  myFilter->SetInput(2,ndDummy);
  MITK_TEST_CONDITION(myFilter->GetNumberOfInputs() == 3, "Adding new input and testing GetNumberOfInputs == 3");
  MITK_TEST_CONDITION(myFilter->GetNumberOfOutputs() == 3, "testing GetNumberOfOutputs == 3");
  MITK_TEST_CONDITION(myFilter->GetInput(2) == ndDummy, "Testing Input == newly added input");
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetOutput(2) != NULL, "Testing GetOutput(2) != NULL");
  MITK_TEST_CONDITION_REQUIRED(myFilter->GetOutput(2) != myFilter->GetOutput(1), "Testing GetOutput(2) != GetOutput(1)");
  MITK_TEST_CONDITION(myFilter->SetBaseData(2, mitkToolDataDummy), "Testing SetBaseData() node 3");
  MITK_TEST_CONDITION(myFilter->GetNumberOfToolRepresentations() == 3, "Testing GetNumberOfToolRepresentations() after adding latest tool");
  MITK_TEST_CONDITION(myFilter->GetBaseData(2) == mitkToolDataDummy, "Testing GetBaseData() equals wat was set");
  
  //last time processing update through pipeline
  myFilter->Update();
  
  //now check for the new values
  mitk::AffineTransform3D::Pointer affineTransformDummy = mitkToolDataDummy->GetGeometry()->GetIndexToWorldTransform();
  mitk::AffineTransform3D::OutputVectorType offsetDummy = affineTransformDummy->GetOffset();
  MITK_TEST_CONDITION(offsetDummy.Get_vnl_vector()==initialPosDummy.Get_vnl_vector(), "Testing Offset latest added tool");
  
  mitk::AffineTransform3D::MatrixType::InternalMatrixType m1Latest= affineTransformDummy->GetMatrix().GetVnlMatrix();
  std::cout<<"\n latest initOrient="<<initialOriDummy<<" latest affineTransform->GetVnlMatrix():\n "<< m1Latest <<"\n";

  // always end with this!
  MITK_TEST_END();
}
