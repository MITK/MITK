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

#include <mitkBaseGeometry.h>
#include <MitkExports.h>
#include <mitkCommon.h>
#include "mitkoperationactor.h"

#include <itkBoundingBox.h>
#include "mitkvector.h"
#include <itkAffineGeometryFrame.h>
#include <itkQuaternionRigidTransform.h>
#include "itkScalableAffineTransform.h"
#include <itkIndex.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>

class vtkMatrix4x4;
class vtkMatrixToLinearTransform;
class vtkLinearTransform;

typedef itk::BoundingBox<unsigned long, 3, mitk::ScalarType> BoundingBox;
typedef itk::BoundingBox<unsigned long, 3, mitk::ScalarType> BoundingBoxType;
typedef BoundingBoxType::BoundsArrayType               BoundsArrayType;
typedef BoundingBoxType::Pointer                       BoundingBoxPointer;

// Dummy instance of abstract base class
class DummyTestClass : public mitk::BaseGeometry
{
public:
  DummyTestClass(){};
  DummyTestClass(const DummyTestClass& other) : BaseGeometry(other){};
  ~DummyTestClass(){};

  mitkClassMacro(DummyTestClass, mitk::BaseGeometry);
  itkNewMacro(Self);

  itkGetConstMacro(IndexToWorldTransformLastModified, unsigned long);
  itkGetConstMacro(VtkMatrix, vtkMatrix4x4*);

  void DummyTestClass::ExecuteOperation(mitk::Operation* operation){};
};

class mitkBaseGeometryTestSuite : public mitk::TestFixture
{
  // List of Tests
  CPPUNIT_TEST_SUITE(mitkBaseGeometryTestSuite);
  MITK_TEST(TestConstructors);
  MITK_TEST(TestSetOrigin);
  MITK_TEST(TestSetBounds);
  MITK_TEST(TestSetFloatBounds);
  MITK_TEST(TestSetFloatBoundsDouble);
  MITK_TEST(TestSetFrameOfReferenceID);
  MITK_TEST(TestSetIndexToWorldTransform);
  MITK_TEST(TestSetSpacing);
  MITK_TEST(TestTransferItkToVtkTransform);
  CPPUNIT_TEST_SUITE_END();

  // Used Variables
private:
  mitk::Point3D  aPoint;
  float aFloatSpacing[3];
  mitk::Vector3D aSpacing;
  mitk::AffineTransform3D::Pointer aTransform;
  BoundingBoxPointer aBoundingBox;
  mitk::AffineTransform3D::MatrixType aMatrix;

  mitk::Point3D  anotherPoint;
  mitk::Vector3D anotherSpacing;
  BoundingBoxPointer anotherBoundingBox;
  mitk::AffineTransform3D::Pointer anotherTransform;
  mitk::AffineTransform3D::MatrixType anotherMatrix;

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

    anotherTransform = mitk::AffineTransform3D::New();

    aMatrix.SetIdentity();

    anotherMatrix.SetIdentity();
    anotherMatrix(1,1) = 2;
    anotherTransform->SetMatrix( anotherMatrix );

    //Bounding Box
    float bounds[6] = {0,1,0,1,0,1};
    mitk::BoundingBox::BoundsArrayType b;
    const float *input = bounds;
    int j=0;
    for(mitk::BoundingBox::BoundsArrayType::Iterator it = b.Begin(); j < 6 ;++j) *it++ = (mitk::ScalarType)*input++;

    aBoundingBox = BoundingBoxType::New();

    BoundingBoxType::PointsContainer::Pointer pointscontainer = BoundingBoxType::PointsContainer::New();
    BoundingBoxType::PointType p;
    BoundingBoxType::PointIdentifier pointid;
    for(pointid=0; pointid<2;++pointid)
    {
      unsigned int i;
      for(i=0; i<3; ++i)
      {
        p[i] = bounds[2*i+pointid];
      }
      pointscontainer->InsertElement(pointid, p);
    }

    aBoundingBox->SetPoints(pointscontainer);
    aBoundingBox->ComputeBoundingBox();

    anotherBoundingBox = BoundingBoxType::New();
    p[0]=11;
    p[1]=12;
    p[2]=13;
    pointscontainer->InsertElement(1, p);
    anotherBoundingBox->SetPoints(pointscontainer);
    anotherBoundingBox->ComputeBoundingBox();

    mitk::FillVector3D(anotherPoint, 2,3,4);
    mitk::FillVector3D(anotherSpacing, 5,6,7);
  }

  // Test functions

  void TestSetOrigin()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetOrigin(anotherPoint);
    CPPUNIT_ASSERT(anotherPoint==dummy->GetOrigin());

    //undo changes, new and changed object need to be the same!
    dummy->SetOrigin(aPoint);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  }

  void TestSetFloatBounds(){
    float bounds[6] = {0,11,0,12,0,13};
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetFloatBounds(bounds);
    CPPUNIT_ASSERT(mitk::Equal( dummy->GetBoundingBox(), anotherBoundingBox, mitk::eps, true));

    //undo changes, new and changed object need to be the same!
    float originalBounds[6] = {0,1,0,1,0,1};
    dummy->SetFloatBounds(originalBounds);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  }

  void TestSetBounds(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetBounds(anotherBoundingBox->GetBounds());
    CPPUNIT_ASSERT(mitk::Equal( dummy->GetBoundingBox(), anotherBoundingBox, mitk::eps, true));

    //undo changes, new and changed object need to be the same!
    dummy->SetBounds(aBoundingBox->GetBounds());
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  }

  void TestSetFloatBoundsDouble(){
    double bounds[6] = {0,11,0,12,0,13};
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetFloatBounds(bounds);
    CPPUNIT_ASSERT(mitk::Equal( dummy->GetBoundingBox(), anotherBoundingBox, mitk::eps, true));

    //undo changes, new and changed object need to be the same!
    double originalBounds[6] = {0,1,0,1,0,1};
    dummy->SetFloatBounds(originalBounds);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  }

  void TestSetFrameOfReferenceID()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetFrameOfReferenceID(5);
    CPPUNIT_ASSERT(dummy->GetFrameOfReferenceID()==5);

    //undo changes, new and changed object need to be the same!
    dummy->SetFrameOfReferenceID(0);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  }

  void TestSetIndexToWorldTransform()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(anotherTransform);
    CPPUNIT_ASSERT(mitk::Equal(anotherTransform,dummy->GetIndexToWorldTransform(),mitk::eps,true));

    //undo changes, new and changed object need to be the same!
    dummy->SetIndexToWorldTransform(aTransform);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  }

  void TestSetSpacing()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetSpacing(anotherSpacing);
    CPPUNIT_ASSERT(anotherSpacing==dummy->GetSpacing());

    //undo changes, new and changed object need to be the same!
    dummy->SetSpacing(aSpacing);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  }

  void TestTransferItkToVtkTransform()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(anotherTransform); //calls TransferItkToVtkTransform
    mitk::AffineTransform3D::Pointer dummyTransform = dummy->GetIndexToWorldTransform();
    CPPUNIT_ASSERT(mitk::MatrixEqualElementWise( anotherMatrix, dummyTransform->GetMatrix() ));
  }

  void TestConstructors()
  {
    //test standard constructor
    DummyTestClass::Pointer dummy1 = DummyTestClass::New();
    bool test = dummy1->IsValid();
    CPPUNIT_ASSERT(test == true);
    CPPUNIT_ASSERT(dummy1->GetFrameOfReferenceID() == 0);
    CPPUNIT_ASSERT(dummy1->GetIndexToWorldTransformLastModified() == 0);

    const float *dummy1FloatSpacing = dummy1->GetFloatSpacing();
    CPPUNIT_ASSERT(dummy1FloatSpacing[0]==aFloatSpacing[0]);
    CPPUNIT_ASSERT(dummy1FloatSpacing[1]==aFloatSpacing[1]);
    CPPUNIT_ASSERT(dummy1FloatSpacing[2]==aFloatSpacing[2]);

    CPPUNIT_ASSERT(dummy1->GetSpacing() == aSpacing);
    CPPUNIT_ASSERT(dummy1->GetOrigin()==aPoint);

    CPPUNIT_ASSERT(mitk::Equal( dummy1->GetIndexToWorldTransform(), aTransform, mitk::eps, true));

    CPPUNIT_ASSERT(mitk::Equal( dummy1->GetBoundingBox(), aBoundingBox, mitk::eps, true));

    DummyTestClass::Pointer dummy2 = DummyTestClass::New();
    dummy2->SetOrigin(anotherPoint);
    float bounds[6] = {0,11,0,12,0,13};
    dummy2->SetFloatBounds(bounds);
    dummy2->SetIndexToWorldTransform(anotherTransform);
    dummy2->SetSpacing(anotherSpacing);

    DummyTestClass::Pointer dummy3;
    dummy3 = new DummyTestClass(*dummy2);
    CPPUNIT_ASSERT(mitk::Equal(dummy3,dummy2,mitk::eps,true));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkBaseGeometry)
