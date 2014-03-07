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
  mitkNewMacro1Param(Self,Self);

  itkGetConstMacro(IndexToWorldTransformLastModified, unsigned long);
  itkGetConstMacro(VtkMatrix, vtkMatrix4x4*);

  void DummyTestClass::ExecuteOperation(mitk::Operation* operation){};
  itk::LightObject::Pointer DummyTestClass::InternalClone() const
  {
    Self::Pointer newGeometry = new Self(*this);
    newGeometry->UnRegister();
    return newGeometry.GetPointer();
  }
};

class mitkBaseGeometryTestSuite : public mitk::TestFixture
{
  // List of Tests
  CPPUNIT_TEST_SUITE(mitkBaseGeometryTestSuite);

  //Constructor
  MITK_TEST(TestConstructors);

  //Set
  MITK_TEST(TestSetOrigin);
  MITK_TEST(TestSetBounds);
  MITK_TEST(TestSetFloatBounds);
  MITK_TEST(TestSetFloatBoundsDouble);
  MITK_TEST(TestSetFrameOfReferenceID);
  MITK_TEST(TestSetIndexToWorldTransform);
  MITK_TEST(TestSetSpacing);
  MITK_TEST(TestTransferItkToVtkTransform);
  MITK_TEST(TestSetIndexToWorldTransformByVtkMatrix);
  MITK_TEST(TestSetIdentity);
  //Equal
  MITK_TEST(Equal_CloneAndOriginal_ReturnsTrue);
  MITK_TEST(Equal_DifferentOrigin_ReturnsFalse);
  MITK_TEST(Equal_DifferentIndexToWorldTransform_ReturnsFalse);
  MITK_TEST(Equal_DifferentSpacing_ReturnsFalse);
  MITK_TEST(Equal_InputIsNull_ReturnsFalse);
  MITK_TEST(Equal_DifferentBoundingBox_ReturnsFalse);
  MITK_TEST(TestComposeTransform);
  MITK_TEST(TestComposeVtkMatrix);

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
  BoundingBoxPointer aThirdBoundingBox;
  mitk::AffineTransform3D::Pointer anotherTransform;
  mitk::AffineTransform3D::Pointer aThirdTransform;
  mitk::AffineTransform3D::MatrixType anotherMatrix;
  mitk::AffineTransform3D::MatrixType aThirdMatrix;

  DummyTestClass::Pointer aDummyGeometry;
  DummyTestClass::Pointer anotherDummyGeometry;

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

    aThirdBoundingBox = BoundingBoxType::New();
    p[0]=22;
    p[1]=23;
    p[2]=24;
    pointscontainer->InsertElement(1, p);
    aThirdBoundingBox->SetPoints(pointscontainer);
    aThirdBoundingBox->ComputeBoundingBox();

    mitk::FillVector3D(anotherPoint, 2,3,4);
    mitk::FillVector3D(anotherSpacing, 5,6,7);

    aDummyGeometry = DummyTestClass::New();
    aDummyGeometry->Initialize();
    anotherDummyGeometry = aDummyGeometry->Clone();
  }

  void tearDown()
  {
    aDummyGeometry = NULL;
    anotherDummyGeometry = NULL;
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

    //Wrong bounds, test needs to fail
    bounds[1]=7;
    dummy->SetFloatBounds(bounds);
    CPPUNIT_ASSERT((mitk::Equal( dummy->GetBoundingBox(), anotherBoundingBox, mitk::eps, false))==false);

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

    //Test needs to fail now
    dummy->SetBounds(aThirdBoundingBox->GetBounds());
    CPPUNIT_ASSERT(mitk::Equal( dummy->GetBoundingBox(), anotherBoundingBox, mitk::eps, false)==false);

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

    //Test needs to fail now
    bounds[3]=7;
    dummy->SetFloatBounds(bounds);
    CPPUNIT_ASSERT(mitk::Equal( dummy->GetBoundingBox(), anotherBoundingBox, mitk::eps, false)==false);

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

    //Test needs to fail now
    dummy->SetIndexToWorldTransform(aThirdTransform);
    CPPUNIT_ASSERT(mitk::Equal(anotherTransform,dummy->GetIndexToWorldTransform(),mitk::eps,false)==false);

    //undo changes, new and changed object need to be the same!
    dummy->SetIndexToWorldTransform(aTransform);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  }

  void TestSetIndexToWorldTransformByVtkMatrix()
  {
    vtkMatrix4x4* vtkmatrix;
    vtkmatrix = vtkMatrix4x4::New();
    vtkmatrix->Identity();
    vtkmatrix->SetElement(1,1,2);
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    CPPUNIT_ASSERT(mitk::Equal(anotherTransform,dummy->GetIndexToWorldTransform(),mitk::eps,true));

    //test needs to fail now
    vtkmatrix->SetElement(1,1,7);
    dummy->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    CPPUNIT_ASSERT(mitk::Equal(anotherTransform,dummy->GetIndexToWorldTransform(),mitk::eps,false)==false);

    //undo changes, new and changed object need to be the same!
    vtkmatrix->SetElement(1,1,1);
    dummy->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  }

  void TestSetIdentity()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    //Change IndextoWorldTransform and Origin
    dummy->SetIndexToWorldTransform(anotherTransform);
    dummy->SetOrigin(anotherPoint);

    //Set Identity should reset ITWT and Origin
    dummy->SetIdentity();

    CPPUNIT_ASSERT(mitk::Equal(aTransform,dummy->GetIndexToWorldTransform(),mitk::eps,true));
    CPPUNIT_ASSERT(aPoint==dummy->GetOrigin());
    CPPUNIT_ASSERT(aSpacing==dummy->GetSpacing());

    //new and changed object need to be the same!
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
    dummy3 = DummyTestClass::New(*dummy2);
    CPPUNIT_ASSERT(mitk::Equal(dummy3,dummy2,mitk::eps,true));
  }

  //Equal Tests

  void Equal_CloneAndOriginal_ReturnsTrue()
  {
    CPPUNIT_ASSERT( mitk::Equal(aDummyGeometry, anotherDummyGeometry, mitk::eps,true));
  }

  void Equal_DifferentOrigin_ReturnsFalse()
  {
    anotherDummyGeometry->SetOrigin(anotherPoint);

    CPPUNIT_ASSERT( mitk::Equal(aDummyGeometry, anotherDummyGeometry, mitk::eps,false)==false);
  }

  void Equal_DifferentIndexToWorldTransform_ReturnsFalse()
  {
    anotherDummyGeometry->SetIndexToWorldTransform(anotherTransform);

    CPPUNIT_ASSERT( mitk::Equal(aDummyGeometry, anotherDummyGeometry, mitk::eps,false)==false);
  }

  void Equal_DifferentSpacing_ReturnsFalse()
  {
    anotherDummyGeometry->SetSpacing(anotherSpacing);

    CPPUNIT_ASSERT( mitk::Equal(aDummyGeometry, anotherDummyGeometry, mitk::eps,false)==false);
  }

  void Equal_InputIsNull_ReturnsFalse()
  {
    DummyTestClass::Pointer geometryNull = NULL;
    CPPUNIT_ASSERT( mitk::Equal(geometryNull, anotherDummyGeometry, mitk::eps,false)==false);
  }

  void Equal_DifferentBoundingBox_ReturnsFalse()
  {
    //create different bounds to make the comparison false
    mitk::ScalarType bounds[ ] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    anotherDummyGeometry->SetBounds(bounds);

    CPPUNIT_ASSERT( mitk::Equal(aDummyGeometry, anotherDummyGeometry, mitk::eps,false)==false);
  }

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

    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(transform1);  //Spacing = 2
    dummy->Compose(transform2); //Spacing = 4

    CPPUNIT_ASSERT(mitk::Equal(transform3,dummy->GetIndexToWorldTransform(),mitk::eps,true)); // 4=4

    //undo changes, new and changed object need to be the same!
    dummy->Compose(transform4); //Spacing = 1
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true)); // 1=1
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

    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(transform1);  //Spacing = 2
    dummy->Compose(vtkmatrix2); //Spacing = 4

    CPPUNIT_ASSERT(mitk::Equal(transform3,dummy->GetIndexToWorldTransform(),mitk::eps,true)); // 4=4

    //undo changes, new and changed object need to be the same!
    dummy->Compose(vtkmatrix4); //Spacing = 1
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true)); // 1=1
  }
};//end class mitkBaseGeometryTestSuite

MITK_TEST_SUITE_REGISTRATION(mitkBaseGeometry)
