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
#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include <mitkOperationActor.h>

#include <itkBoundingBox.h>
#include <mitkVector.h>
#include <itkAffineGeometryFrame.h>
#include <itkQuaternionRigidTransform.h>
#include <itkScalableAffineTransform.h>
#include <itkIndex.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>

#include <mitkRotationOperation.h>
#include <mitkScaleOperation.h>
#include <mitkInteractionConst.h>
#include <mitkMatrixConvert.h>
#include <mitkImageCast.h>

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
  mitkNewMacro1Param(Self, const Self&);

  itk::LightObject::Pointer InternalClone() const override
  {
    Self::Pointer newGeometry = new Self(*this);
    newGeometry->UnRegister();
    return newGeometry.GetPointer();
  }

protected:
  virtual void PrintSelf(std::ostream& /*os*/, itk::Indent /*indent*/) const override{};
  //##Documentation
  //## @brief Pre- and Post-functions are empty in BaseGeometry
  //##
  //## These virtual functions allow for a different beahiour in subclasses.
  //## Do implement them in every subclass of BaseGeometry. If not needed, use {}.
  //## If this class is inherited from a subclass of BaseGeometry, call {Superclass::Pre...();};, example: SlicedGeometry3D class
  virtual void PreSetSpacing(const mitk::Vector3D& aSpacing) override{};
};

class mitkBaseGeometryTestSuite : public mitk::TestFixture
{
  // List of Tests
  CPPUNIT_TEST_SUITE(mitkBaseGeometryTestSuite);

  //Constructor
  MITK_TEST(TestConstructors);
  MITK_TEST(TestInitialize);

  //Set
  MITK_TEST(TestSetOrigin);
  MITK_TEST(TestSetBounds);
  MITK_TEST(TestSetFloatBounds);
  MITK_TEST(TestSetFloatBoundsDouble);
  MITK_TEST(TestSetFrameOfReferenceID);
  MITK_TEST(TestSetIndexToWorldTransform);
  MITK_TEST(TestSetIndexToWorldTransformWithoutChangingSpacing);
  MITK_TEST(TestSetIndexToWorldTransform_WithPointerToSameTransform);
  MITK_TEST(TestSetSpacing);
  MITK_TEST(TestTransferItkToVtkTransform);
  MITK_TEST(TestSetIndexToWorldTransformByVtkMatrix);
  MITK_TEST(TestSetIdentity);
  MITK_TEST(TestSetImageGeometry);
  //Equal
  MITK_TEST(Equal_CloneAndOriginal_ReturnsTrue);
  MITK_TEST(Equal_DifferentOrigin_ReturnsFalse);
  MITK_TEST(Equal_DifferentIndexToWorldTransform_ReturnsFalse);
  MITK_TEST(Equal_DifferentSpacing_ReturnsFalse);
  MITK_TEST(Equal_InputIsNull_ReturnsFalse);
  MITK_TEST(Equal_DifferentBoundingBox_ReturnsFalse);
  MITK_TEST(Equal_Transforms_MinorDifferences_And_Eps);
  //other Functions
  MITK_TEST(TestComposeTransform);
  MITK_TEST(TestComposeVtkMatrix);
  MITK_TEST(TestTranslate);
  MITK_TEST(TestIndexToWorld);
  MITK_TEST(TestExecuteOperation);
  MITK_TEST(TestCalculateBoundingBoxRelToTransform);
  //MITK_TEST(TestSetTimeBounds);
  MITK_TEST(TestIs2DConvertable);
  MITK_TEST(TestGetCornerPoint);
  MITK_TEST(TestExtentInMM);
  MITK_TEST(TestGetAxisVector);
  MITK_TEST(TestGetCenter);
  MITK_TEST(TestGetDiagonalLength);
  MITK_TEST(TestGetExtent);
  MITK_TEST(TestIsInside);
  MITK_TEST(TestGetMatrixColumn);

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
  void setUp() override
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
    mitk::FillVector3D(anotherSpacing, 5,6.5,7);

    aDummyGeometry = DummyTestClass::New();
    aDummyGeometry->Initialize();
    anotherDummyGeometry = aDummyGeometry->Clone();
  }

  void tearDown() override
  {
    aDummyGeometry = nullptr;
    anotherDummyGeometry = nullptr;
  }

  // Test functions

  void TestSetOrigin()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetOrigin(anotherPoint);
    CPPUNIT_ASSERT(mitk::Equal(anotherPoint,dummy->GetOrigin()));

    //undo changes, new and changed object need to be the same!
    dummy->SetOrigin(aPoint);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy, newDummy, "TestSetOrigin");
  }

  void TestSetImageGeometry()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetImageGeometry(true);
    CPPUNIT_ASSERT(dummy->GetImageGeometry());

    //undo changes, new and changed object need to be the same!
    dummy->SetImageGeometry(false);
    CPPUNIT_ASSERT(dummy->GetImageGeometry()==false);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy, "TestSetImageGeometry");
  }

  void TestSetFloatBounds(){
    float bounds[6] = {0,11,0,12,0,13};
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetFloatBounds(bounds);
    MITK_ASSERT_EQUAL(BoundingBox::ConstPointer(dummy->GetBoundingBox()), anotherBoundingBox, "BoundingBox equality");

    //Wrong bounds, test needs to fail
    bounds[1]=7;
    dummy->SetFloatBounds(bounds);
    MITK_ASSERT_NOT_EQUAL(BoundingBox::ConstPointer(dummy->GetBoundingBox()), anotherBoundingBox, "BoundingBox not equal");

    //undo changes, new and changed object need to be the same!
    float originalBounds[6] = {0,1,0,1,0,1};
    dummy->SetFloatBounds(originalBounds);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy, "Undo and equal");
  }

  void TestSetBounds(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetBounds(anotherBoundingBox->GetBounds());
    MITK_ASSERT_EQUAL(BoundingBox::ConstPointer(dummy->GetBoundingBox()), anotherBoundingBox, "Setting bounds");

    //Test needs to fail now
    dummy->SetBounds(aThirdBoundingBox->GetBounds());
    MITK_ASSERT_NOT_EQUAL(BoundingBox::ConstPointer(dummy->GetBoundingBox()), anotherBoundingBox, "Setting unequal bounds");

    //undo changes, new and changed object need to be the same!
    dummy->SetBounds(aBoundingBox->GetBounds());
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy, "Undo set bounds");
  }

  void TestSetFloatBoundsDouble(){
    double bounds[6] = {0,11,0,12,0,13};
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetFloatBounds(bounds);
    MITK_ASSERT_EQUAL(BoundingBox::ConstPointer(dummy->GetBoundingBox()), anotherBoundingBox, "Float bounds");

    //Test needs to fail now
    bounds[3]=7;
    dummy->SetFloatBounds(bounds);
    MITK_ASSERT_NOT_EQUAL(BoundingBox::ConstPointer(dummy->GetBoundingBox()), anotherBoundingBox, "Float bounds unequal");

    //undo changes, new and changed object need to be the same!
    double originalBounds[6] = {0,1,0,1,0,1};
    dummy->SetFloatBounds(originalBounds);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy, "Undo set float bounds");
  }

  void TestSetFrameOfReferenceID()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetFrameOfReferenceID(5);
    CPPUNIT_ASSERT(dummy->GetFrameOfReferenceID()==5);

    //undo changes, new and changed object need to be the same!
    dummy->SetFrameOfReferenceID(0);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy, "Undo set frame of reference");
  }

  void TestSetIndexToWorldTransform()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(anotherTransform);
    MITK_ASSERT_EQUAL(anotherTransform,mitk::AffineTransform3D::Pointer(dummy->GetIndexToWorldTransform()),"Compare IndexToWorldTransform 1");

    //Test needs to fail now
    dummy->SetIndexToWorldTransform(aThirdTransform);
    MITK_ASSERT_NOT_EQUAL(anotherTransform,mitk::AffineTransform3D::Pointer(dummy->GetIndexToWorldTransform()),"Compare IndexToWorldTransform 2");

    //undo changes, new and changed object need to be the same!
    dummy->SetIndexToWorldTransform(aTransform);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy,"Compare IndexToWorldTransform 3");
  }

  void TestSetIndexToWorldTransformWithoutChangingSpacing()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransformWithoutChangingSpacing(anotherTransform);
    CPPUNIT_ASSERT(mitk::Equal(aSpacing,dummy->GetSpacing(),mitk::eps,true));

    //calculate a new version of anotherTransform, so that the spacing should be the same as the original spacing of aTransform.
    mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
    vnlmatrix = anotherTransform->GetMatrix().GetVnlMatrix();

    mitk::VnlVector col;
    col = vnlmatrix.get_column(0); col.normalize(); col*=aSpacing[0]; vnlmatrix.set_column(0, col);
    col = vnlmatrix.get_column(1); col.normalize(); col*=aSpacing[1]; vnlmatrix.set_column(1, col);
    col = vnlmatrix.get_column(2); col.normalize(); col*=aSpacing[2]; vnlmatrix.set_column(2, col);

    mitk::Matrix3D matrix;
    matrix = vnlmatrix;
    anotherTransform->SetMatrix(matrix);

    CPPUNIT_ASSERT(mitk::Equal(anotherTransform,dummy->GetIndexToWorldTransform(),mitk::eps,true));
  }

  void TestSetIndexToWorldTransform_WithPointerToSameTransform()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();

    dummy->SetOrigin(anotherPoint);
    dummy->SetIndexToWorldTransform(anotherTransform);
    dummy->SetSpacing(anotherSpacing);

    mitk::AffineTransform3D::Pointer testTransfrom = dummy->GetIndexToWorldTransform();

    mitk::Vector3D modifiedPoint = anotherPoint.GetVectorFromOrigin() *2.;

    testTransfrom->SetOffset(modifiedPoint);

    dummy->SetIndexToWorldTransform(testTransfrom);

    CPPUNIT_ASSERT(mitk::Equal(modifiedPoint, dummy->GetOrigin().GetVectorFromOrigin()));
  }

  void TestSetIndexToWorldTransformByVtkMatrix()
  {
    vtkMatrix4x4* vtkmatrix;
    vtkmatrix = vtkMatrix4x4::New();
    vtkmatrix->Identity();
    vtkmatrix->SetElement(1,1,2);
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    MITK_ASSERT_EQUAL(anotherTransform,mitk::AffineTransform3D::Pointer(dummy->GetIndexToWorldTransform()),"Compare IndexToWorldTransformByVtkMatrix 1");

    //test needs to fail now
    vtkmatrix->SetElement(1,1,7);
    dummy->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    MITK_ASSERT_NOT_EQUAL(anotherTransform,mitk::AffineTransform3D::Pointer(dummy->GetIndexToWorldTransform()),"Compare IndexToWorldTransformByVtkMatrix 2");

    //undo changes, new and changed object need to be the same!
    vtkmatrix->SetElement(1,1,1);
    dummy->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    vtkmatrix->Delete();
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy,"Compare IndexToWorldTransformByVtkMatrix 3");
  }

  void TestSetIdentity()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    //Change IndextoWorldTransform and Origin
    dummy->SetIndexToWorldTransform(anotherTransform);
    dummy->SetOrigin(anotherPoint);

    //Set Identity should reset ITWT and Origin
    dummy->SetIdentity();

    MITK_ASSERT_EQUAL(aTransform,mitk::AffineTransform3D::Pointer(dummy->GetIndexToWorldTransform()),"Test set identity 1");
    CPPUNIT_ASSERT(mitk::Equal(aPoint,dummy->GetOrigin()));
    CPPUNIT_ASSERT(mitk::Equal(aSpacing,dummy->GetSpacing()));

    //new and changed object need to be the same!
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy,"Test set identity 2");
  }

  void TestSetSpacing()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetSpacing(anotherSpacing);
    CPPUNIT_ASSERT(mitk::Equal(anotherSpacing,dummy->GetSpacing()));

    //undo changes, new and changed object need to be the same!
    dummy->SetSpacing(aSpacing);
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy,"Dummy spacing");
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

    CPPUNIT_ASSERT(mitk::Equal(dummy1->GetSpacing(), aSpacing));
    CPPUNIT_ASSERT(mitk::Equal(dummy1->GetOrigin(), aPoint));

    CPPUNIT_ASSERT(dummy1->GetImageGeometry()==false);

    MITK_ASSERT_EQUAL(mitk::AffineTransform3D::Pointer(dummy1->GetIndexToWorldTransform()), aTransform,"Contructor test 1");

    MITK_ASSERT_EQUAL(mitk::BaseGeometry::BoundingBoxType::ConstPointer(dummy1->GetBoundingBox()), aBoundingBox, "Constructor test 2");

    DummyTestClass::Pointer dummy2 = DummyTestClass::New();
    dummy2->SetOrigin(anotherPoint);
    float bounds[6] = {0,11,0,12,0,13};
    dummy2->SetFloatBounds(bounds);
    dummy2->SetIndexToWorldTransform(anotherTransform);
    dummy2->SetSpacing(anotherSpacing);

    DummyTestClass::Pointer dummy3 = DummyTestClass::New(*dummy2);
    MITK_ASSERT_EQUAL(dummy3,dummy2,"Dummy contructor");
  }

  //Equal Tests

  void Equal_CloneAndOriginal_ReturnsTrue()
  {
    MITK_ASSERT_EQUAL( aDummyGeometry, anotherDummyGeometry, "Clone test");
  }

  void Equal_DifferentOrigin_ReturnsFalse()
  {
    anotherDummyGeometry->SetOrigin(anotherPoint);

    MITK_ASSERT_NOT_EQUAL(aDummyGeometry, anotherDummyGeometry, "Different origin test");
  }

  void Equal_DifferentIndexToWorldTransform_ReturnsFalse()
  {
    anotherDummyGeometry->SetIndexToWorldTransform(anotherTransform);

    MITK_ASSERT_NOT_EQUAL( aDummyGeometry, anotherDummyGeometry, "Different index to world");
  }

  void Equal_DifferentSpacing_ReturnsFalse()
  {
    anotherDummyGeometry->SetSpacing(anotherSpacing);

    MITK_ASSERT_NOT_EQUAL( aDummyGeometry, anotherDummyGeometry, "Different spacing");
  }

  void Equal_InputIsNull_ReturnsFalse()
  {
    DummyTestClass::Pointer geometryNull = nullptr;
    CPPUNIT_ASSERT_THROW( MITK_ASSERT_EQUAL(geometryNull,anotherDummyGeometry,"Input is null") , mitk::Exception );
  }

  void Equal_DifferentBoundingBox_ReturnsFalse()
  {
    //create different bounds to make the comparison false
    mitk::ScalarType bounds[ ] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    anotherDummyGeometry->SetBounds(bounds);

    MITK_ASSERT_NOT_EQUAL( aDummyGeometry, anotherDummyGeometry , "Different bounding box");
  }

  void Equal_Transforms_MinorDifferences_And_Eps()
  {
    // Verifies that the eps parameter is evaluated properly
    // when comparing two mitk::BaseGeometry::TransformTypes
    aMatrix.SetIdentity();
    anotherMatrix.SetIdentity();

    aMatrix(0,1)       = 0.0002; aTransform->SetMatrix( aMatrix );
    anotherMatrix(0,1) = 0.0002; anotherTransform->SetMatrix( anotherMatrix );
    anotherTransform->SetMatrix( aMatrix );
    CPPUNIT_ASSERT_MESSAGE( "Exact same transforms are mitk::Equal() for eps=mitk::eps", mitk::Equal(aTransform, anotherTransform, mitk::eps, true) );
    CPPUNIT_ASSERT_MESSAGE( "Exact same transforms are mitk::Equal() for eps=vnl_math::eps", mitk::Equal(aTransform, anotherTransform, vnl_math::eps, true) );

    anotherMatrix(0,1) = 0.0002 + mitk::eps; anotherTransform->SetMatrix( anotherMatrix );
    CPPUNIT_ASSERT_MESSAGE( "Transforms of diff mitk::eps are !mitk::Equal() for eps=vnl_math::eps", !mitk::Equal(aTransform, anotherTransform, vnl_math::eps, true) );
    CPPUNIT_ASSERT_MESSAGE( "Transforms of diff mitk::eps are !mitk::Equal() for eps=mitk::eps-1%", !mitk::Equal(aTransform, anotherTransform, mitk::eps * 0.99, true) );
    CPPUNIT_ASSERT_MESSAGE( "Transforms of diff mitk::eps _are_ mitk::Equal() for eps=mitk::eps+1%", mitk::Equal(aTransform, anotherTransform, mitk::eps * 1.01, true) );
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

    //Vector to compare spacing
    mitk::Vector3D expectedSpacing;
    expectedSpacing.Fill(1.0);
    expectedSpacing[1] = 4;

    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(transform1);  //Spacing = 2
    dummy->Compose(transform2); //Spacing = 4
    CPPUNIT_ASSERT(mitk::Equal(dummy->GetSpacing(), expectedSpacing));
    MITK_ASSERT_EQUAL(transform3,mitk::AffineTransform3D::Pointer(dummy->GetIndexToWorldTransform()),"Compose transform 2"); // 4=4

    //undo changes, new and changed object need to be the same!
    dummy->Compose(transform4); //Spacing = 1
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy,"Compose transform 3"); // 1=1
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

    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(transform1);  //Spacing = 2
    dummy->Compose(vtkmatrix2); //Spacing = 4
    vtkmatrix2->Delete();

    MITK_ASSERT_EQUAL(transform3, mitk::AffineTransform3D::Pointer(dummy->GetIndexToWorldTransform()),"Compose vtk matrix"); // 4=4
    CPPUNIT_ASSERT(mitk::Equal(dummy->GetSpacing(), expectedSpacing));

    //undo changes, new and changed object need to be the same!
    dummy->Compose(vtkmatrix4); //Spacing = 1
    vtkmatrix4->Delete();
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy,"Compose vtk"); // 1=1
  }

  void TestTranslate(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetOrigin(anotherPoint);
    CPPUNIT_ASSERT(mitk::Equal(anotherPoint,dummy->GetOrigin()));

    //use some random values for translation
    mitk::Vector3D translationVector;
    translationVector.SetElement(0, 17.5f);
    translationVector.SetElement(1, -32.3f);
    translationVector.SetElement(2, 4.0f);

    //compute ground truth
    mitk::Point3D tmpResult = anotherPoint + translationVector;
    dummy->Translate(translationVector);
    CPPUNIT_ASSERT( mitk::Equal( dummy->GetOrigin(), tmpResult ));

    //undo changes
    translationVector*=-1;
    dummy->Translate(translationVector);
    CPPUNIT_ASSERT( mitk::Equal( dummy->GetOrigin(), anotherPoint ));

    //undo changes, new and changed object need to be the same!
    translationVector.SetElement(0, -1 * anotherPoint[0]);
    translationVector.SetElement(1, -1 * anotherPoint[1]);
    translationVector.SetElement(2, -1 * anotherPoint[2]);
    dummy->Translate(translationVector);

    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy,"Translate test");
  }

  // a part of the test requires axis-parallel coordinates
  int testIndexAndWorldConsistency(DummyTestClass::Pointer dummyGeometry)
  {
    //Testing consistency of index and world coordinate systems
    mitk::Point3D origin = dummyGeometry->GetOrigin();
    mitk::Point3D dummyPoint;

    //Testing index->world->index conversion consistency
    dummyGeometry->WorldToIndex(origin, dummyPoint);
    dummyGeometry->IndexToWorld(dummyPoint, dummyPoint);
    CPPUNIT_ASSERT(mitk::EqualArray(dummyPoint, origin, 3, mitk::eps, true));

    //Testing WorldToIndex(origin, mitk::Point3D)==(0,0,0)
    mitk::Point3D globalOrigin;
    mitk::FillVector3D(globalOrigin, 0,0,0);

    mitk::Point3D originContinuousIndex;
    dummyGeometry->WorldToIndex(origin, originContinuousIndex);
    CPPUNIT_ASSERT(mitk::EqualArray(originContinuousIndex, globalOrigin, 3, mitk::eps, true));

    //Testing WorldToIndex(origin, itk::Index)==(0,0,0)
    itk::Index<3> itkindex;
    dummyGeometry->WorldToIndex(origin, itkindex);
    itk::Index<3> globalOriginIndex;
    mitk::vtk2itk(globalOrigin, globalOriginIndex);
    CPPUNIT_ASSERT(mitk::EqualArray(itkindex, globalOriginIndex, 3, mitk::eps, true));

    //Testing WorldToIndex(origin-0.5*spacing, itk::Index)==(0,0,0)
    mitk::Vector3D halfSpacingStep = dummyGeometry->GetSpacing()*0.5;
    mitk::Matrix3D rotation;
    mitk::Point3D originOffCenter = origin-halfSpacingStep;
    dummyGeometry->WorldToIndex(originOffCenter, itkindex);
    CPPUNIT_ASSERT(mitk::EqualArray(itkindex, globalOriginIndex, 3, mitk::eps, true));

    //Testing WorldToIndex(origin+0.5*spacing-eps, itk::Index)==(0,0,0)
    originOffCenter = origin+halfSpacingStep;
    originOffCenter -= 0.0001;
    dummyGeometry->WorldToIndex( originOffCenter, itkindex);
    CPPUNIT_ASSERT(mitk::EqualArray(itkindex, globalOriginIndex, 3, mitk::eps, true));

    //Testing WorldToIndex(origin+0.5*spacing, itk::Index)==(1,1,1)");
    originOffCenter = origin+halfSpacingStep;
    itk::Index<3> global111;
    mitk::FillVector3D(global111, 1,1,1);
    dummyGeometry->WorldToIndex( originOffCenter, itkindex);
    CPPUNIT_ASSERT(mitk::EqualArray(itkindex, global111, 3, mitk::eps, true));

    //Testing WorldToIndex(GetCenter())==BoundingBox.GetCenter
    mitk::Point3D center = dummyGeometry->GetCenter();
    mitk::Point3D centerContIndex;
    dummyGeometry->WorldToIndex(center, centerContIndex);
    mitk::BoundingBox::ConstPointer boundingBox = dummyGeometry->GetBoundingBox();
    mitk::BoundingBox::PointType centerBounds = boundingBox->GetCenter();
    CPPUNIT_ASSERT(mitk::Equal(centerContIndex,centerBounds));

    //Testing GetCenter()==IndexToWorld(BoundingBox.GetCenter)
    center = dummyGeometry->GetCenter();
    mitk::Point3D centerBoundsInWorldCoords;
    dummyGeometry->IndexToWorld(centerBounds, centerBoundsInWorldCoords);
    CPPUNIT_ASSERT(mitk::Equal(center,centerBoundsInWorldCoords));

    //Test using random point,
    //Testing consistency of index and world coordinate systems
    mitk::Point3D point;
    mitk::FillVector3D(point,3.5,-2,4.6);

    //Testing index->world->index conversion consistency
    dummyGeometry->WorldToIndex(point, dummyPoint);
    dummyGeometry->IndexToWorld(dummyPoint, dummyPoint);
    CPPUNIT_ASSERT(mitk::EqualArray(dummyPoint, point, 3, mitk::eps, true));

    return EXIT_SUCCESS;
  }

  int testIndexAndWorldConsistencyForVectors(DummyTestClass::Pointer dummyGeometry)
  {
    //Testing consistency of index and world coordinate systems for vectors
    mitk::Vector3D xAxisMM = dummyGeometry->GetAxisVector(0);
    mitk::Vector3D xAxisContinuousIndex;

    mitk::Point3D p, pIndex, origin;
    origin = dummyGeometry->GetOrigin();
    p[0] = xAxisMM[0]+origin[0];
    p[1] = xAxisMM[1]+origin[1];
    p[2] = xAxisMM[2]+origin[2];

    dummyGeometry->WorldToIndex(p,pIndex);

    dummyGeometry->WorldToIndex(xAxisMM,xAxisContinuousIndex);
    CPPUNIT_ASSERT(mitk::Equal(xAxisContinuousIndex[0], pIndex[0]));
    CPPUNIT_ASSERT(mitk::Equal(xAxisContinuousIndex[1], pIndex[1]));
    CPPUNIT_ASSERT(mitk::Equal(xAxisContinuousIndex[2], pIndex[2]));

    dummyGeometry->IndexToWorld(xAxisContinuousIndex,xAxisContinuousIndex);

    dummyGeometry->IndexToWorld(pIndex,p);

    CPPUNIT_ASSERT(xAxisContinuousIndex == xAxisMM);
    CPPUNIT_ASSERT(mitk::Equal(xAxisContinuousIndex[0], p[0]-origin[0]));
    CPPUNIT_ASSERT(mitk::Equal(xAxisContinuousIndex[1], p[1]-origin[1]));
    CPPUNIT_ASSERT(mitk::Equal(xAxisContinuousIndex[2], p[2]-origin[2]));

    //Test consictency for random vector
    mitk::Vector3D vector;
    mitk::FillVector3D(vector, 2.5,-3.2,8.1);
    mitk::Vector3D vectorContinuousIndex;

    p[0] = vector[0]+origin[0];
    p[1] = vector[1]+origin[1];
    p[2] = vector[2]+origin[2];

    dummyGeometry->WorldToIndex(p,pIndex);

    dummyGeometry->WorldToIndex(vector,vectorContinuousIndex);
    CPPUNIT_ASSERT(mitk::Equal(vectorContinuousIndex[0], pIndex[0]));
    CPPUNIT_ASSERT(mitk::Equal(vectorContinuousIndex[1], pIndex[1]));
    CPPUNIT_ASSERT(mitk::Equal(vectorContinuousIndex[2], pIndex[2]));

    dummyGeometry->IndexToWorld(vectorContinuousIndex,vectorContinuousIndex);

    dummyGeometry->IndexToWorld(pIndex,p);

    CPPUNIT_ASSERT(vectorContinuousIndex == vector);
    CPPUNIT_ASSERT(mitk::Equal(vectorContinuousIndex[0], p[0]-origin[0]));
    CPPUNIT_ASSERT(mitk::Equal(vectorContinuousIndex[1], p[1]-origin[1]));
    CPPUNIT_ASSERT(mitk::Equal(vectorContinuousIndex[2], p[2]-origin[2]));

    return EXIT_SUCCESS;
  }

  int testIndexAndWorldConsistencyForIndex(DummyTestClass::Pointer dummyGeometry)
  {
    //Testing consistency of index and world coordinate systems

    // creating testing data
    itk::Index<4> itkIndex4, itkIndex4b;
    itk::Index<3> itkIndex3, itkIndex3b;
    itk::Index<2> itkIndex2, itkIndex2b;
    itk::Index<3> mitkIndex, mitkIndexb;

    itkIndex4[0] = itkIndex4[1] = itkIndex4[2] = itkIndex4[3] = 4;
    itkIndex3[0] = itkIndex3[1] = itkIndex3[2] = 6;
    itkIndex2[0] = itkIndex2[1] = 2;
    mitkIndex[0] = mitkIndex[1] = mitkIndex[2] = 13;

    // check for constistency
    mitk::Point3D point;
    dummyGeometry->IndexToWorld(itkIndex2,point);
    dummyGeometry->WorldToIndex(point,itkIndex2b);

    CPPUNIT_ASSERT(
      ((itkIndex2b[0] == itkIndex2[0]) &&
      (itkIndex2b[1] == itkIndex2[1])));
    //Testing itk::index<2> for IndexToWorld/WorldToIndex consistency

    dummyGeometry->IndexToWorld(itkIndex3,point);
    dummyGeometry->WorldToIndex(point,itkIndex3b);

    CPPUNIT_ASSERT(
      ((itkIndex3b[0] == itkIndex3[0]) &&
      (itkIndex3b[1] == itkIndex3[1]) &&
      (itkIndex3b[2] == itkIndex3[2])));
    //Testing itk::index<3> for IndexToWorld/WorldToIndex consistency

    dummyGeometry->IndexToWorld(itkIndex4,point);
    dummyGeometry->WorldToIndex(point,itkIndex4b);

    CPPUNIT_ASSERT(
      ((itkIndex4b[0] == itkIndex4[0]) &&
      (itkIndex4b[1] == itkIndex4[1]) &&
      (itkIndex4b[2] == itkIndex4[2]) &&
      (itkIndex4b[3] == 0)));
    //Testing itk::index<3> for IndexToWorld/WorldToIndex consistency

    dummyGeometry->IndexToWorld(mitkIndex,point);
    dummyGeometry->WorldToIndex(point,mitkIndexb);

    CPPUNIT_ASSERT(
      ((mitkIndexb[0] == mitkIndex[0]) &&
      (mitkIndexb[1] == mitkIndex[1]) &&
      (mitkIndexb[2] == mitkIndex[2])));
    //Testing mitk::Index for IndexToWorld/WorldToIndex consistency

    return EXIT_SUCCESS;
  }

  void TestIndexToWorld(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();

    testIndexAndWorldConsistency(dummy);
    testIndexAndWorldConsistencyForVectors(dummy);
    testIndexAndWorldConsistencyForIndex(dummy);

    //Geometry must not have changed
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy,"Dummy index to world");

    //Test with other geometries
    dummy->SetOrigin(anotherPoint);
    testIndexAndWorldConsistency(dummy);
    testIndexAndWorldConsistencyForVectors(dummy);
    testIndexAndWorldConsistencyForIndex(dummy);

    dummy->SetIndexToWorldTransform(anotherTransform);
    testIndexAndWorldConsistency(dummy);
    testIndexAndWorldConsistencyForVectors(dummy);
    testIndexAndWorldConsistencyForIndex(dummy);

    dummy->SetOrigin(anotherPoint);
    testIndexAndWorldConsistency(dummy);
    testIndexAndWorldConsistencyForVectors(dummy);
    testIndexAndWorldConsistencyForIndex(dummy);

    dummy->SetSpacing(anotherSpacing);
    testIndexAndWorldConsistency(dummy);
    testIndexAndWorldConsistencyForVectors(dummy);
    testIndexAndWorldConsistencyForIndex(dummy);
  }

  void TestExecuteOperation(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();

    //Do same Operations with new Dummy and compare
    DummyTestClass::Pointer newDummy = DummyTestClass::New();

    //Test operation Nothing
    auto  opN = new mitk::Operation(mitk::OpNOTHING);
    dummy->ExecuteOperation(opN);
    MITK_ASSERT_EQUAL(dummy,newDummy,"Dummy execute operation 1");

    //Test operation Move
    auto  opP = new mitk::PointOperation(mitk::OpMOVE,anotherPoint);
    dummy->ExecuteOperation(opP);
    CPPUNIT_ASSERT(mitk::Equal(anotherPoint,dummy->GetOrigin()));
    newDummy->SetOrigin(anotherPoint);
    MITK_ASSERT_EQUAL(dummy,newDummy,"Dummy execute operation 2");

    //Test operation Scale, Scale sets spacing to scale+1
    mitk::Point3D spacing;
    spacing[0]=anotherSpacing[0]-1.;
    spacing[1]=anotherSpacing[1]-1.;
    spacing[2]=anotherSpacing[2]-1.;

    auto  opS = new mitk::ScaleOperation(mitk::OpSCALE,spacing,anotherPoint);
    dummy->ExecuteOperation(opS);
    CPPUNIT_ASSERT(mitk::Equal(anotherSpacing,dummy->GetSpacing()));
    newDummy->SetSpacing(anotherSpacing);
    MITK_ASSERT_EQUAL(dummy,newDummy,"Dummy execute operation 3");

    //change Geometry to test more cases
    dummy->SetIndexToWorldTransform(anotherTransform);
    dummy->SetSpacing(anotherSpacing);

    //Testing a rotation of the geometry
    double angle = 35.0;
    mitk::Vector3D rotationVector; mitk::FillVector3D( rotationVector, 1, 0, 0 );
    mitk::Point3D center = dummy->GetCenter();
    auto  opR = new mitk::RotationOperation( mitk::OpROTATE, center, rotationVector, angle );
    dummy->ExecuteOperation(opR);

    mitk::Matrix3D rotation;
    mitk::GetRotation(dummy, rotation);
    mitk::Vector3D voxelStep=rotation*anotherSpacing;
    mitk::Vector3D voxelStepIndex;
    dummy->WorldToIndex(voxelStep, voxelStepIndex);
    mitk::Vector3D expectedVoxelStepIndex;
    expectedVoxelStepIndex.Fill(1);
    CPPUNIT_ASSERT(mitk::Equal(voxelStepIndex,expectedVoxelStepIndex));

    delete opR;
    delete opN;
    delete opS;
    delete opP;
  }

  void TestCalculateBoundingBoxRelToTransform(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetExtentInMM(0,15);
    dummy->SetExtentInMM(1,20);
    dummy->SetExtentInMM(2,8);

    mitk::BoundingBox::Pointer dummyBoundingBox = dummy->CalculateBoundingBoxRelativeToTransform(anotherTransform);

    mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();
    mitk::BoundingBox::PointIdentifier pointid=0;
    unsigned char i;
    mitk::AffineTransform3D::Pointer inverse = mitk::AffineTransform3D::New();
    anotherTransform->GetInverse(inverse);
    for(i=0; i<8; ++i)
      pointscontainer->InsertElement( pointid++, inverse->TransformPoint( dummy->GetCornerPoint(i) ));
    mitk::BoundingBox::Pointer result = mitk::BoundingBox::New();
    result->SetPoints(pointscontainer);
    result->ComputeBoundingBox();

    MITK_ASSERT_EQUAL(result,dummyBoundingBox,"BBox rel to transform");

    //dummy still needs to be unchanged, except for extend
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    newDummy->SetExtentInMM(0,15);
    newDummy->SetExtentInMM(1,20);
    newDummy->SetExtentInMM(2,8);
    MITK_ASSERT_EQUAL(dummy,newDummy,"Dummy BBox");
  }

  //void TestSetTimeBounds(){
  //  mitk::TimeBounds timeBounds;
  //  timeBounds[0] = 1;
  //  timeBounds[1] = 9;

  //  DummyTestClass::Pointer dummy = DummyTestClass::New();
  //  dummy->SetTimeBounds(timeBounds);
  //  mitk::TimeBounds timeBounds2 = dummy->GetTimeBounds();

  //  CPPUNIT_ASSERT(timeBounds[0]==timeBounds2[0]);
  //  CPPUNIT_ASSERT(timeBounds[1]==timeBounds2[1]);

  //  //undo changes, new and changed object need to be the same!
  //  timeBounds[0]=mitk::ScalarTypeNumericTraits::NonpositiveMin();
  //  timeBounds[1]=mitk::ScalarTypeNumericTraits::max();

  //  DummyTestClass::Pointer newDummy = DummyTestClass::New();
  //  CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  //}

  void TestIs2DConvertable(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();

    //new initialized geometry is 2D convertable
    CPPUNIT_ASSERT(dummy->Is2DConvertable());

    //Wrong Spacing needs to fail
    dummy->SetSpacing(anotherSpacing);
    CPPUNIT_ASSERT(dummy->Is2DConvertable()==false);
    //undo
    dummy->SetSpacing(aSpacing);
    CPPUNIT_ASSERT(dummy->Is2DConvertable());

    //Wrong Origin needs to fail
    dummy->SetOrigin(anotherPoint);
    CPPUNIT_ASSERT(dummy->Is2DConvertable()==false);
    //undo
    dummy->SetOrigin(aPoint);
    CPPUNIT_ASSERT(dummy->Is2DConvertable());

    //third dimension must not be transformed
    mitk::AffineTransform3D::Pointer dummyTransform = mitk::AffineTransform3D::New();
    mitk::AffineTransform3D::MatrixType dummyMatrix;
    dummyMatrix.SetIdentity();
    dummyTransform->SetMatrix( dummyMatrix );
    dummy->SetIndexToWorldTransform(dummyTransform);

    //identity matrix is 2DConvertable
    CPPUNIT_ASSERT(dummy->Is2DConvertable());

    dummyMatrix(0,2) = 3;
    dummyTransform->SetMatrix( dummyMatrix );
    CPPUNIT_ASSERT(dummy->Is2DConvertable()==false);

    dummyMatrix.SetIdentity();
    dummyMatrix(1,2) = 0.4;
    dummyTransform->SetMatrix( dummyMatrix );
    CPPUNIT_ASSERT(dummy->Is2DConvertable()==false);

    dummyMatrix.SetIdentity();
    dummyMatrix(2,2) = 3;
    dummyTransform->SetMatrix( dummyMatrix );
    CPPUNIT_ASSERT(dummy->Is2DConvertable()==false);

    dummyMatrix.SetIdentity();
    dummyMatrix(2,1) = 3;
    dummyTransform->SetMatrix( dummyMatrix );
    CPPUNIT_ASSERT(dummy->Is2DConvertable()==false);

    dummyMatrix.SetIdentity();
    dummyMatrix(2,0) = 3;
    dummyTransform->SetMatrix( dummyMatrix );
    CPPUNIT_ASSERT(dummy->Is2DConvertable()==false);

    //undo changes, new and changed object need to be the same!
    dummyMatrix.SetIdentity();
    dummyTransform->SetMatrix( dummyMatrix );
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy,newDummy,"Is 2D convertable");
  }

  void TestGetCornerPoint(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(anotherTransform);
    double bounds[6] = {0,11,0,12,0,13};
    dummy->SetFloatBounds(bounds);

    mitk::Point3D corner, refCorner;

    //Corner 0
    mitk::FillVector3D(refCorner,bounds[0],bounds[2],bounds[4]);
    refCorner = anotherTransform->TransformPoint(refCorner);
    corner=dummy->GetCornerPoint(0);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));
    corner=dummy->GetCornerPoint(true,true,true);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));

    //Corner 1
    mitk::FillVector3D(refCorner,bounds[0],bounds[2],bounds[5]);
    refCorner = anotherTransform->TransformPoint(refCorner);
    corner=dummy->GetCornerPoint(1);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));
    corner=dummy->GetCornerPoint(true,true,false);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));

    //Corner 2
    mitk::FillVector3D(refCorner,bounds[0],bounds[3],bounds[4]);
    refCorner = anotherTransform->TransformPoint(refCorner);
    corner=dummy->GetCornerPoint(2);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));
    corner=dummy->GetCornerPoint(true,false,true);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));

    //Corner 3
    mitk::FillVector3D(refCorner,bounds[0],bounds[3],bounds[5]);
    refCorner = anotherTransform->TransformPoint(refCorner);
    corner=dummy->GetCornerPoint(3);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));
    corner=dummy->GetCornerPoint(true,false,false);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));

    //Corner 4
    mitk::FillVector3D(refCorner,bounds[1],bounds[2],bounds[4]);
    refCorner = anotherTransform->TransformPoint(refCorner);
    corner=dummy->GetCornerPoint(4);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));
    corner=dummy->GetCornerPoint(false,true,true);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));

    //Corner 5
    mitk::FillVector3D(refCorner,bounds[1],bounds[2],bounds[5]);
    refCorner = anotherTransform->TransformPoint(refCorner);
    corner=dummy->GetCornerPoint(5);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));
    corner=dummy->GetCornerPoint(false,true,false);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));

    //Corner 6
    mitk::FillVector3D(refCorner,bounds[1],bounds[3],bounds[4]);
    refCorner = anotherTransform->TransformPoint(refCorner);
    corner=dummy->GetCornerPoint(6);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));
    corner=dummy->GetCornerPoint(false,false,true);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));

    //Corner 7
    mitk::FillVector3D(refCorner,bounds[1],bounds[3],bounds[5]);
    refCorner = anotherTransform->TransformPoint(refCorner);
    corner=dummy->GetCornerPoint(7);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));
    corner=dummy->GetCornerPoint(false,false,false);
    CPPUNIT_ASSERT(mitk::Equal(refCorner,corner));

    //Wrong Corner needs to fail
    CPPUNIT_ASSERT_THROW(dummy->GetCornerPoint(20),itk::ExceptionObject);

    //dummy geometry must not have changed!
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    newDummy->SetIndexToWorldTransform(anotherTransform);
    newDummy->SetFloatBounds(bounds);
    MITK_ASSERT_EQUAL(dummy,newDummy,"Corner point");
  }

  void TestExtentInMM()
  {
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetExtentInMM(0,50);
    CPPUNIT_ASSERT(mitk::Equal(50.,dummy->GetExtentInMM(0)));
    //Vnl Matrix has changed. The next line only works because the spacing is 1!
    CPPUNIT_ASSERT(mitk::Equal(50.,dummy->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0).magnitude()));

    //Smaller extent than original
    dummy->SetExtentInMM(0,5);
    CPPUNIT_ASSERT(mitk::Equal(5.,dummy->GetExtentInMM(0)));
    CPPUNIT_ASSERT(mitk::Equal(5.,dummy->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0).magnitude()));

    dummy->SetExtentInMM(1,4);
    CPPUNIT_ASSERT(mitk::Equal(4.,dummy->GetExtentInMM(1)));
    CPPUNIT_ASSERT(mitk::Equal(4.,dummy->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(1).magnitude()));

    dummy->SetExtentInMM(2,2.5);
    CPPUNIT_ASSERT(mitk::Equal(2.5,dummy->GetExtentInMM(2)));
    CPPUNIT_ASSERT(mitk::Equal(2.5,dummy->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2).magnitude()));
  }

  void TestGetAxisVector(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(anotherTransform);
    double bounds[6] = {0,11,0,12,0,13};
    dummy->SetFloatBounds(bounds);

    mitk::Vector3D vector;
    mitk::FillVector3D(vector,bounds[1],0,0);
    dummy->IndexToWorld(vector,vector);
    CPPUNIT_ASSERT(mitk::Equal(dummy->GetAxisVector(0),vector));

    mitk::FillVector3D(vector,0,bounds[3],0);
    dummy->IndexToWorld(vector,vector);
    CPPUNIT_ASSERT(mitk::Equal(dummy->GetAxisVector(1),vector));

    mitk::FillVector3D(vector,0,0,bounds[5]);
    dummy->IndexToWorld(vector,vector);
    CPPUNIT_ASSERT(mitk::Equal(dummy->GetAxisVector(2),vector));
  }

  void TestGetCenter(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(anotherTransform);
    double bounds[6] = {0,11,2,12,1,13};
    dummy->SetFloatBounds(bounds);

    mitk::Point3D refCenter;
    for( int i=0;i<3;i++)
      refCenter.SetElement(i,( bounds[2 * i] + bounds[2 * i + 1] ) / 2.0);

    dummy->IndexToWorld(refCenter,refCenter);

    CPPUNIT_ASSERT(mitk::Equal(dummy->GetCenter(),refCenter));
  }

  void TestGetDiagonalLength(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    double bounds[6] = {1,3,5,8,7.5,11.5};
    dummy->SetFloatBounds(bounds);
    //3-1=2, 8-5=3, 11.5-7.5=4; 2^2+3^2+4^2 = 29
    double expectedLength = sqrt(29.);

    CPPUNIT_ASSERT(mitk::Equal(expectedLength, dummy->GetDiagonalLength(), mitk::eps, true));
    CPPUNIT_ASSERT(mitk::Equal(29., dummy->GetDiagonalLength2(), mitk::eps, true));

    //dummy must not have changed
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    newDummy->SetFloatBounds(bounds);
    MITK_ASSERT_EQUAL(dummy,newDummy,"Diagonal length");
  }

  void TestGetExtent(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    double bounds[6] = {1,3,5,8,7.5,11.5};
    dummy->SetFloatBounds(bounds);

    CPPUNIT_ASSERT(mitk::Equal(2.,dummy->GetExtent(0)));
    CPPUNIT_ASSERT(mitk::Equal(3.,dummy->GetExtent(1)));
    CPPUNIT_ASSERT(mitk::Equal(4.,dummy->GetExtent(2)));

    //dummy must not have changed
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    newDummy->SetFloatBounds(bounds);
    MITK_ASSERT_EQUAL(dummy,newDummy,"Extend");
  }

  void TestIsInside(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    double bounds[6] = {1,3,5,8,7.5,11.5};
    dummy->SetFloatBounds(bounds);

    mitk::Point3D insidePoint;
    mitk::Point3D outsidePoint;

    mitk::FillVector3D(insidePoint,2,6,7.6);
    mitk::FillVector3D(outsidePoint,0,9,8.2);

    CPPUNIT_ASSERT(dummy->IsIndexInside(insidePoint));
    CPPUNIT_ASSERT(false==dummy->IsIndexInside(outsidePoint));

    dummy->IndexToWorld(insidePoint,insidePoint);
    dummy->IndexToWorld(outsidePoint,outsidePoint);

    CPPUNIT_ASSERT(dummy->IsInside(insidePoint));
    CPPUNIT_ASSERT(false==dummy->IsInside(outsidePoint));

    //dummy must not have changed
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    newDummy->SetFloatBounds(bounds);
    MITK_ASSERT_EQUAL(dummy,newDummy,"Is inside");
  }

  void TestInitialize()
  {
    //test standard constructor
    DummyTestClass::Pointer dummy1 = DummyTestClass::New();

    DummyTestClass::Pointer dummy2 = DummyTestClass::New();
    dummy2->SetOrigin(anotherPoint);
    dummy2->SetBounds(anotherBoundingBox->GetBounds());
    //mitk::TimeBounds timeBounds;
    //timeBounds[0] = 1;
    //timeBounds[1] = 9;
    //dummy2->SetTimeBounds(timeBounds);
    dummy2->SetIndexToWorldTransform(anotherTransform);
    dummy2->SetSpacing(anotherSpacing);

    dummy1->InitializeGeometry(dummy2);

    MITK_ASSERT_EQUAL(dummy1,dummy2,"Initialize 1");

    dummy1->Initialize();

    DummyTestClass::Pointer dummy3 = DummyTestClass::New();
    MITK_ASSERT_EQUAL(dummy3,dummy1,"Initialize 2");
  }

  void TestGetMatrixColumn(){
    DummyTestClass::Pointer dummy = DummyTestClass::New();
    dummy->SetIndexToWorldTransform(anotherTransform);
    mitk::Vector3D testVector,refVector;

    testVector.SetVnlVector(dummy->GetMatrixColumn(0));
    mitk::FillVector3D(refVector,1,0,0);
    CPPUNIT_ASSERT(testVector==refVector);

    testVector.SetVnlVector(dummy->GetMatrixColumn(1));
    mitk::FillVector3D(refVector,0,2,0);
    CPPUNIT_ASSERT(testVector==refVector);

    testVector.SetVnlVector(dummy->GetMatrixColumn(2));
    mitk::FillVector3D(refVector,0,0,1);
    CPPUNIT_ASSERT(testVector==refVector);

    //dummy must not have changed
    DummyTestClass::Pointer newDummy = DummyTestClass::New();
    newDummy->SetIndexToWorldTransform(anotherTransform);
    MITK_ASSERT_EQUAL(dummy,newDummy,"GetMatrixColumn");
  }

  /*

  void (){
  DummyTestClass::Pointer dummy = DummyTestClass::New();

  CPPUNIT_ASSERT();

  //undo changes, new and changed object need to be the same!

  DummyTestClass::Pointer newDummy = DummyTestClass::New();
  CPPUNIT_ASSERT(mitk::Equal(dummy,newDummy,mitk::eps,true));
  }

  */
};//end class mitkBaseGeometryTestSuite

MITK_TEST_SUITE_REGISTRATION(mitkBaseGeometry)
