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

#include "mitkTestingMacros.h"
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>

#include <mitkGeometryTransformHolder.h>
#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include "mitkOperationActor.h"

#include "mitkVector.h"
#include <vtkMatrix4x4.h>

#include <itkAffineGeometryFrame.h>
#include "itkScalableAffineTransform.h"
#include <itkIndex.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>

#include "mitkInteractionConst.h"
#include <mitkMatrixConvert.h>

class vtkMatrix4x4;
class vtkMatrixToLinearTransform;
class vtkLinearTransform;

class mitkGeometryTransformHolderTestSuite : public mitk::TestFixture
{
  // List of Tests
  CPPUNIT_TEST_SUITE(mitkGeometryTransformHolderTestSuite);

  //Constructor
  MITK_TEST(TestConstructors);
  MITK_TEST(TestInitialize);

  //Set
  MITK_TEST(TestSetOrigin);
  MITK_TEST(TestSetIndexToWorldTransform);
  MITK_TEST(TestSetSpacing);
  MITK_TEST(TestTransferItkToVtkTransform);
  MITK_TEST(TestSetIndexToWorldTransformByVtkMatrix);
  MITK_TEST(TestSetIdentity);

  //other Functions
  MITK_TEST(TestComposeTransform);
  MITK_TEST(TestComposeVtkMatrix);

  CPPUNIT_TEST_SUITE_END();

  // Used Variables
private:
  mitk::Point3D  aPoint;
  float aFloatSpacing[3];
  mitk::Vector3D aSpacing;
  mitk::AffineTransform3D::Pointer aTransform;
  mitk::AffineTransform3D::MatrixType aMatrix;

  mitk::Point3D  anotherPoint;
  mitk::Vector3D anotherSpacing;
  mitk::AffineTransform3D::Pointer anotherTransform;
  mitk::AffineTransform3D::Pointer aThirdTransform;
  mitk::AffineTransform3D::MatrixType anotherMatrix;
  mitk::AffineTransform3D::MatrixType aThirdMatrix;

  mitk::GeometryTransformHolder* dummyGeoHolder;
  mitk::GeometryTransformHolder* dummyGeoHolder_Unchanged;

public:

  // Set up for variables
  void setUp()
  {
    mitk::FillVector3D(aFloatSpacing, 1,1,1);
    mitk::FillVector3D(aSpacing, 1,1,1);
    mitk::FillVector3D(aPoint, 0,0,0);

    //Transform
    aTransform = mitk::AffineTransform3D::New();
    aTransform->SetIdentity();

    aMatrix.SetIdentity();

    anotherTransform = mitk::AffineTransform3D::New();

    anotherMatrix.SetIdentity();
    anotherMatrix(1,1) = 2;
    anotherTransform->SetMatrix( anotherMatrix );

    aThirdTransform = mitk::AffineTransform3D::New();

    aThirdMatrix.SetIdentity();
    aThirdMatrix(1,1) = 7;
    aThirdTransform->SetMatrix( aThirdMatrix );

    mitk::FillVector3D(anotherPoint, 2,3,4);
    mitk::FillVector3D(anotherSpacing, 5,6.5,7);

    dummyGeoHolder = new mitk::GeometryTransformHolder();
    dummyGeoHolder_Unchanged = new mitk::GeometryTransformHolder();
  }

  void tearDown()
  {
    delete dummyGeoHolder;
  }

  // Test functions

  void TestSetOrigin()
  {
    dummyGeoHolder->SetOrigin(anotherPoint);
    CPPUNIT_ASSERT(mitk::Equal(anotherPoint,dummyGeoHolder->GetOrigin()));

    //undo changes, new and changed object need to be the same!
    dummyGeoHolder->SetOrigin(aPoint);
    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder,dummyGeoHolder_Unchanged,mitk::eps,true));
  }

  void TestSetIndexToWorldTransform()
  {
    dummyGeoHolder->SetIndexToWorldTransform(anotherTransform);
    CPPUNIT_ASSERT(mitk::Equal(anotherTransform,dummyGeoHolder->GetIndexToWorldTransform(),mitk::eps,true));

    //Test needs to fail now
    dummyGeoHolder->SetIndexToWorldTransform(aThirdTransform);
    CPPUNIT_ASSERT(mitk::Equal(anotherTransform,dummyGeoHolder->GetIndexToWorldTransform(),mitk::eps,false)==false);

    //undo changes, new and changed object need to be the same!
    dummyGeoHolder->SetIndexToWorldTransform(aTransform);
    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder,dummyGeoHolder_Unchanged,mitk::eps,true));
  }

  void TestSetIndexToWorldTransformByVtkMatrix()
  {
    vtkMatrix4x4* vtkmatrix;
    vtkmatrix = vtkMatrix4x4::New();
    vtkmatrix->Identity();
    vtkmatrix->SetElement(1,1,2);

    dummyGeoHolder->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    CPPUNIT_ASSERT(mitk::Equal(anotherTransform,dummyGeoHolder->GetIndexToWorldTransform(),mitk::eps,true));

    //test needs to fail now
    vtkmatrix->SetElement(1,1,7);
    dummyGeoHolder->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    CPPUNIT_ASSERT(mitk::Equal(anotherTransform,dummyGeoHolder->GetIndexToWorldTransform(),mitk::eps,false)==false);

    //undo changes, new and changed object need to be the same!
    vtkmatrix->SetElement(1,1,1);
    dummyGeoHolder->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder,dummyGeoHolder_Unchanged,mitk::eps,true));
  }

  void TestSetIdentity()
  {
    //Change IndextoWorldTransform and Origin
    dummyGeoHolder->SetIndexToWorldTransform(anotherTransform);
    dummyGeoHolder->SetOrigin(anotherPoint);

    //Set Identity should reset ITWT and Origin
    dummyGeoHolder->SetIdentity();

    CPPUNIT_ASSERT(mitk::Equal(aTransform,dummyGeoHolder->GetIndexToWorldTransform(),mitk::eps,true));
    CPPUNIT_ASSERT(mitk::Equal(aPoint,dummyGeoHolder->GetOrigin()));
    CPPUNIT_ASSERT(mitk::Equal(aSpacing,dummyGeoHolder->GetSpacing()));

    //new and changed object need to be the same!
    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder,dummyGeoHolder_Unchanged,mitk::eps,true));
  }

  void TestSetSpacing()
  {
    dummyGeoHolder->SetSpacing(anotherSpacing);
    CPPUNIT_ASSERT(mitk::Equal(anotherSpacing,dummyGeoHolder->GetSpacing()));

    //undo changes, new and changed object need to be the same!
    dummyGeoHolder->SetSpacing(aSpacing);

    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder,dummyGeoHolder_Unchanged,mitk::eps,true));
  }

  void TestTransferItkToVtkTransform()
  {
    dummyGeoHolder->SetIndexToWorldTransform(anotherTransform); //calls TransferItkToVtkTransform
    mitk::AffineTransform3D::Pointer dummyTransform = dummyGeoHolder->GetIndexToWorldTransform();
    CPPUNIT_ASSERT(mitk::MatrixEqualElementWise( anotherMatrix, dummyTransform->GetMatrix() ));
  }

  void TestConstructors()
  {
    //test standard constructor
    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder->GetSpacing(), aSpacing));
    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder->GetOrigin(), aPoint));

    CPPUNIT_ASSERT(mitk::Equal( dummyGeoHolder->GetIndexToWorldTransform(), aTransform, mitk::eps, true));

    dummyGeoHolder->SetOrigin(anotherPoint);
    dummyGeoHolder->SetIndexToWorldTransform(anotherTransform);
    dummyGeoHolder->SetSpacing(anotherSpacing);

    mitk::GeometryTransformHolder* dummy3 = new mitk::GeometryTransformHolder(*dummyGeoHolder);
    CPPUNIT_ASSERT(mitk::Equal(dummy3,dummyGeoHolder,mitk::eps,true));
  }

  //Equal Tests

  void TestComposeTransform(){
    //Create Transformations to set and compare
    mitk::AffineTransform3D::Pointer transform1;
    transform1 = mitk::AffineTransform3D::New();
    mitk::AffineTransform3D::MatrixType matrix1;
    matrix1.SetIdentity();
    matrix1(1,1) = 2;
    transform1->SetMatrix( matrix1 );  //Spacing = 2

    mitk::AffineTransform3D::Pointer transform2;
    transform2 = mitk::AffineTransform3D::New();
    mitk::AffineTransform3D::MatrixType matrix2;
    matrix2.SetIdentity();
    matrix2(1,1) = 2;
    transform2->SetMatrix( matrix2 );  //Spacing = 2

    mitk::AffineTransform3D::Pointer transform3;
    transform3 = mitk::AffineTransform3D::New();
    mitk::AffineTransform3D::MatrixType matrix3;
    matrix3.SetIdentity();
    matrix3(1,1) = 4;
    transform3->SetMatrix( matrix3 );  //Spacing = 4

    mitk::AffineTransform3D::Pointer transform4;
    transform4 = mitk::AffineTransform3D::New();
    mitk::AffineTransform3D::MatrixType matrix4;
    matrix4.SetIdentity();
    matrix4(1,1) = 0.25;
    transform4->SetMatrix( matrix4 );  //Spacing = 0.25

    //Vector to compare spacing
    mitk::Vector3D expectedSpacing;
    expectedSpacing.Fill(1.0);
    expectedSpacing[1] = 4;

    dummyGeoHolder->SetIndexToWorldTransform(transform1);  //Spacing = 2
    //xxxdummyGeoHolder->Compose(transform2); //Spacing = 4
    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder->GetSpacing(), expectedSpacing));
    CPPUNIT_ASSERT(mitk::Equal(transform3,dummyGeoHolder->GetIndexToWorldTransform(),mitk::eps,true)); // 4=4

    //undo changes, new and changed object need to be the same!
    //xxxx dummyGeoHolder->Compose(transform4); //Spacing = 1
    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder,dummyGeoHolder_Unchanged,mitk::eps,true)); // 1=1
  }

  void TestComposeVtkMatrix(){
    //Create Transformations to set and compare
    mitk::AffineTransform3D::Pointer transform1;
    transform1 = mitk::AffineTransform3D::New();
    mitk::AffineTransform3D::MatrixType matrix1;
    matrix1.SetIdentity();
    matrix1(1,1) = 2;
    transform1->SetMatrix( matrix1 );  //Spacing = 2

    vtkMatrix4x4* vtkmatrix2;
    vtkmatrix2 = vtkMatrix4x4::New();
    vtkmatrix2->Identity();
    vtkmatrix2->SetElement(1,1,2); //Spacing = 2

    mitk::AffineTransform3D::Pointer transform3;
    transform3 = mitk::AffineTransform3D::New();
    mitk::AffineTransform3D::MatrixType matrix3;
    matrix3.SetIdentity();
    matrix3(1,1) = 4;
    transform3->SetMatrix( matrix3 );  //Spacing = 4

    vtkMatrix4x4* vtkmatrix4;
    vtkmatrix4 = vtkMatrix4x4::New();
    vtkmatrix4->Identity();
    vtkmatrix4->SetElement(1,1,0.25); //Spacing = 0.25

    //Vector to compare spacing
    mitk::Vector3D expectedSpacing;
    expectedSpacing.Fill(1.0);
    expectedSpacing[1] = 4;

    dummyGeoHolder->SetIndexToWorldTransform(transform1);  //Spacing = 2
    //XXXXXXXXXdummyGeoHolder->Compose(vtkmatrix2); //Spacing = 4

    CPPUNIT_ASSERT(mitk::Equal(transform3,dummyGeoHolder->GetIndexToWorldTransform(),mitk::eps,true)); // 4=4
    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder->GetSpacing(), expectedSpacing));

    //undo changes, new and changed object need to be the same!
    //XXXXXXXXXdummyGeoHolder->Compose(vtkmatrix4); //Spacing = 1
    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder,dummyGeoHolder_Unchanged,mitk::eps,true)); // 1=1
  }

  void TestInitialize()
  {
    dummyGeoHolder->SetOrigin(anotherPoint);
    dummyGeoHolder->SetIndexToWorldTransform(anotherTransform);
    dummyGeoHolder->SetSpacing(anotherSpacing);

    mitk::GeometryTransformHolder* dummy2 = new mitk::GeometryTransformHolder();
    dummy2->Initialize(dummyGeoHolder);
    CPPUNIT_ASSERT(mitk::Equal(dummy2,dummyGeoHolder,mitk::eps,true));

    dummyGeoHolder->Initialize();

    CPPUNIT_ASSERT(mitk::Equal(dummyGeoHolder,dummyGeoHolder_Unchanged,mitk::eps,true));
  }
};//end class mitkGeometryTransformHolderTestSuite

MITK_TEST_SUITE_REGISTRATION(mitkGeometryTransformHolder)
