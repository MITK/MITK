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

#include "mitkNavigationDataVisualizationByBaseDataTransformFilter.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"

#include <vtkTransform.h>
#include <iostream>

#include <vtkConeSource.h>
#include <mitkCone.h>


/**Documentation
 *  test for the class "NavigationDataVisualizationByBaseDataTransformFilter".
 */
int mitkNavigationDataVisualizationByBaseDataTransformFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataVisualizationByBaseDataTransformFilter")

  // let's create an object of our class  
  mitk::NavigationDataVisualizationByBaseDataTransformFilter::Pointer myFilter = mitk::NavigationDataVisualizationByBaseDataTransformFilter::New();
  
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
  mitk::NavigationData::ErrorType initialError1(0.0);
  mitk::NavigationData::ErrorType initialError2(5.0);
  bool initialValid1(true);
  bool initialValid2(false);
  
  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->SetPosition(initialPos1);
  nd1->SetOrientation(initialOri1);
  nd1->SetError(initialError1);
  nd1->SetDataValid(initialValid1);

  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
  nd2->SetPosition(initialPos2);
  nd2->SetOrientation(initialOri2);
  nd2->SetError(initialError2);
  nd2->SetDataValid(initialValid2);

  myFilter->SetInput(nd1);
  myFilter->SetInput(1, nd2);
  MITK_TEST_CONDITION(myFilter->GetInput() == nd1, "Testing Set-/GetInput() input 1");
  MITK_TEST_CONDITION(myFilter->GetInput(1) == nd2, "Testing Set-/GetInput() input 2");

  // Test setting dataTreeNode
  //new nodes
  mitk::DataTreeNode::Pointer node1 = mitk::DataTreeNode::New();
  vtkConeSource* vtkData1 = vtkConeSource::New();
  mitk::Cone::Pointer mitkToolData1 = mitk::Cone::New();
  mitkToolData1->SetVtkPolyData(vtkData1->GetOutput());

  vtkConeSource* vtkData2 = vtkConeSource::New();
  mitk::Cone::Pointer mitkToolData2 = mitk::Cone::New();
  mitkToolData2->SetVtkPolyData(vtkData2->GetOutput());

  //setting nodes
  MITK_TEST_CONDITION(myFilter->SetBaseData(nd1, mitkToolData1), "Testing SetBaseData() node 1");
  MITK_TEST_CONDITION(myFilter->SetBaseData(nd2, mitkToolData2), "Testing SetBaseData() node 2");
  //getting nodes
  MITK_TEST_CONDITION(myFilter->GetBaseData(nd1) == mitkToolData1, "Testing GetBaseData() node 1");
  MITK_TEST_CONDITION(myFilter->GetBaseData(nd2) == mitkToolData2, "Testing GetBaseData() node 2");

  
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
  

  // always end with this!
  MITK_TEST_END();

}


