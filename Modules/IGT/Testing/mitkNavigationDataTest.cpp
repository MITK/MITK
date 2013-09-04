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
#include "mitkNavigationData.h"
#include "mitkVector.h"
#include <vnl/vnl_math.h>
#include <itkIndent.h>

static mitk::NavigationData::Pointer GetTestData()
{
  mitk::NavigationData::Pointer nd = mitk::NavigationData::New();
  mitk::NavigationData::PositionType p;
  mitk::FillVector3D(p, 44.4, 55.5, 66.66);
  nd->SetPosition(p);
  mitk::NavigationData::OrientationType o(1.0, 2.0, 3.0, 4.0);
  nd->SetOrientation(o);
  nd->SetDataValid(true);
  nd->SetIGTTimeStamp(100.111);
  nd->SetHasPosition(false);
  nd->SetHasOrientation(false);
  mitk::NavigationData::CovarianceMatrixType m;
  m.Fill(17.17);
  m(2, 2) = 1000.01;
  nd->SetCovErrorMatrix(m);
  nd->SetName("my NavigationData");
  nd->SetPositionAccuracy(100.0);
  nd->SetOrientationAccuracy(10.0);
  return nd;
}

static void TestInstatiation()
{
  // Test instantiation of NavigationData
  mitk::NavigationData::Pointer nd = mitk::NavigationData::New();
  MITK_TEST_CONDITION(nd.IsNotNull(),"Test instatiation");
}

static void TestGetterSetter()
{
  mitk::NavigationData::Pointer nd = mitk::NavigationData::New();

  mitk::NavigationData::PositionType p;
  mitk::FillVector3D(p, 44.4, 55.5, 66.66);
  nd->SetPosition(p);
  MITK_TEST_CONDITION(nd->GetPosition() == p, "Set-/GetPosition()");

  mitk::NavigationData::OrientationType o(1.0, 2.0, 3.0, 4.0);
  nd->SetOrientation(o);
  MITK_TEST_CONDITION(nd->GetOrientation() == o, "Set-/GetOrientation()");

  nd->SetDataValid(true);
  MITK_TEST_CONDITION(nd->IsDataValid() == true, "Set-/IsDataValid()");

  nd->SetIGTTimeStamp(100.111);
  MITK_TEST_CONDITION(mitk::Equal(nd->GetIGTTimeStamp(), 100.111), "Set-/GetIGTTimeStamp()");

  nd->SetHasPosition(false);
  MITK_TEST_CONDITION(nd->GetHasPosition() == false, "Set-/GetHasPosition()");

  nd->SetHasOrientation(false);
  MITK_TEST_CONDITION(nd->GetHasOrientation() == false, "Set-/GetHasOrientation()");

  mitk::NavigationData::CovarianceMatrixType m;
  m.Fill(17.17);
  m(2, 2) = 1000.01;
  nd->SetCovErrorMatrix(m);
  MITK_TEST_CONDITION(nd->GetCovErrorMatrix() == m, "Set-/GetCovErrorMatrix()");

  nd->SetName("my NavigationData");
  MITK_TEST_CONDITION(std::string(nd->GetName()) == "my NavigationData", "Set-/GetName()");

  nd->SetPositionAccuracy(100.0);
  mitk::NavigationData::CovarianceMatrixType result = nd->GetCovErrorMatrix();
  MITK_TEST_CONDITION(mitk::Equal(result(0, 0), 10000.0)
    && mitk::Equal(result(1, 1), 10000.0)
    && mitk::Equal(result(2, 2), 10000.0), "SetPositionAccuracy()");

  nd->SetOrientationAccuracy(10.0);
  mitk::NavigationData::CovarianceMatrixType result2 = nd->GetCovErrorMatrix();
  MITK_TEST_CONDITION(mitk::Equal(result2(3, 3), 100.0)
    && mitk::Equal(result2(4, 4), 100.0)
    && mitk::Equal(result2(5, 5), 100.0), "SetOrientationAccuracy()");
}
static void TestGraft()
{
  //create test data
  mitk::NavigationData::Pointer nd = GetTestData();

  mitk::NavigationData::Pointer graftedCopy = mitk::NavigationData::New();
  graftedCopy->Graft(nd);

  bool graftIsEqual = (nd->GetPosition() == graftedCopy->GetPosition())
    && (nd->GetOrientation() == graftedCopy->GetOrientation())
    && (nd->IsDataValid() == graftedCopy->IsDataValid())
    && mitk::Equal(nd->GetIGTTimeStamp(), graftedCopy->GetIGTTimeStamp())
    && (nd->GetHasPosition() == graftedCopy->GetHasPosition())
    && (nd->GetHasOrientation() == graftedCopy->GetHasOrientation())
    && (nd->GetCovErrorMatrix() == graftedCopy->GetCovErrorMatrix())
    && (std::string(nd->GetName()) == graftedCopy->GetName());

  MITK_TEST_CONDITION(graftIsEqual, "Graft() produces equal NavigationData object");
}

static void TestPrintSelf()
{
  mitk::NavigationData::Pointer nd = GetTestData();
  itk::Indent myIndent = itk::Indent();

  MITK_TEST_OUTPUT(<<"Testing method PrintSelf(), method output will follow:");
  bool success = true;
  try
  {
    nd->PrintSelf(std::cout,myIndent);
  }
  catch(...)
  {
    success = false;
  }
  MITK_TEST_CONDITION(success, "Testing method PrintSelf().");
}

static void TestWrongInputs()
{
  mitk::NavigationData::Pointer nd = GetTestData();

  // Test CopyInformation
  bool success = false;
  try
  {
    nd->CopyInformation(NULL);
  }
  catch(itk::ExceptionObject e)
  {
    success = true;
  }
  MITK_TEST_CONDITION(success, "Testing wrong input for method CopyInformation.");


  // Test Graft
  success = false;
  try
  {
    nd->Graft(NULL);
  }
  catch(itk::ExceptionObject e)
  {
    success = true;
  }
  MITK_TEST_CONDITION(success, "Testing wrong input for method Graft.");
}

static mitk::Quaternion quaternion;
static mitk::Vector3D   offsetVector;
static mitk::Point3D    offsetPoint;
static mitk::Matrix3D   rotation;

static mitk::Quaternion quaternion2;
static mitk::Vector3D   offsetVector2;
static mitk::Point3D    offsetPoint2;
static mitk::Matrix3D   rotation2;

static mitk::Point3D  point;

static void SetupNaviDataTests()
{
  // set rotation matrix to
  /*
  * 0 -1  0
  * 1  0  0
  * 0  0  1
  */
  rotation.Fill(0);
  rotation[0][1] = -1;
  rotation[1][0] =  1;
  rotation[2][2] =  1;

  // set quaternion to quaternion equivalent
  // values calculated with javascript at
  // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
  quaternion = mitk::Quaternion(0, 0, 0.7071067811865475, 0.7071067811865476);

  // set offset to some value. Some tests need vectors, offers points.
  double offsetArray[3] = {1.0,2.0,3.123456};
  offsetVector          = offsetArray;
  offsetPoint           = offsetArray;

  /***** Second set of data for compose tests ****/

  // set rotation2 matrix to
  /*
  * 1  0  0
  * 0  0 -1
  * 0  1  0
  */
  rotation2.Fill(0);
  rotation2[0][0] =  1;
  rotation2[1][2] = -1;
  rotation2[2][1] =  1;

  quaternion2                = mitk::Quaternion(0.7071067811865475, 0, 0, 0.7071067811865476);
  mitk::ScalarType offsetArray2[3] = {1, 0, 0};
  offsetVector2              = offsetArray2;
  offsetPoint2               = offsetArray2;

  /***** Create a point to be transformed *****/
  mitk::ScalarType pointArray[] = {1.0, 3.0, 5.0};
  point        = pointArray;
}

/**
* Helper method, which creates a NavigationData object using the data created in SetupNaviDataTests()
*/
static mitk::NavigationData::Pointer CreateNavidata(mitk::Quaternion quaternion, mitk::Point3D offset)
{
  mitk::NavigationData::Pointer navigationData = mitk::NavigationData::New();
  navigationData->SetOrientation(quaternion);
  navigationData->SetPosition(offset);

  return navigationData;
}

static mitk::AffineTransform3D::Pointer CreateAffineTransform(mitk::Matrix3D rotationMatrix, mitk::Vector3D offset)
{
  mitk::AffineTransform3D::Pointer affineTransform3D = mitk::AffineTransform3D::New();
  affineTransform3D->SetOffset(offset);
  affineTransform3D->SetMatrix(rotationMatrix);

  return affineTransform3D;
}

static void TestInverse()
{
  SetupNaviDataTests();
  mitk::NavigationData::Pointer nd = CreateNavidata(quaternion, offsetPoint);

  mitk::NavigationData::Pointer ndInverse = nd->GetInverse();

  // calculate expected inverted position vector: b2 = -A2b1

  // for -A2b1 we need vnl_vectors.
  vnl_vector_fixed<mitk::ScalarType, 3> b1;
  for (int i = 0; i < 3; ++i) {
    b1[i] = nd->GetPosition()[i];
  }
  vnl_vector_fixed<mitk::ScalarType, 3> b2;
  b2 = -(ndInverse->GetOrientation().rotate(b1));

  // now copy result back into our mitk::Point3D
  mitk::Point3D invertedPosition;
  for (int i = 0; i < 3; ++i) {
    invertedPosition[i] = b2[i];
  }

  MITK_TEST_CONDITION(mitk::Equal(nd->GetOrientation().inverse(), ndInverse->GetOrientation()),"Testing GetInverse: orientation inverted");
  MITK_TEST_CONDITION(mitk::Equal(invertedPosition, ndInverse->GetPosition()), "Testing GetInverse: position inverted");

  bool otherFlagsOk = (nd->IsDataValid() == ndInverse->IsDataValid())
    && mitk::Equal(nd->GetIGTTimeStamp(), ndInverse->GetIGTTimeStamp())
    && (false == ndInverse->GetHasPosition()) // covariance update mechanism not yet implemented
    && (false == ndInverse->GetHasOrientation())
    && (nd->GetCovErrorMatrix() == ndInverse->GetCovErrorMatrix())
    && (std::string(nd->GetName()) == ndInverse->GetName());

  MITK_TEST_CONDITION(otherFlagsOk, "Testing GetInverse: other flags are same");
}

static void TestDoubleInverse()
{
  SetupNaviDataTests();
  mitk::NavigationData::Pointer nd = CreateNavidata(quaternion, offsetPoint);

  mitk::NavigationData::Pointer ndDoubleInverse = nd->GetInverse()->GetInverse();

  MITK_TEST_CONDITION(mitk::Equal(nd->GetOrientation(), ndDoubleInverse->GetOrientation()),"Testing GetInverse double application: orientation preserved");
  MITK_TEST_CONDITION(mitk::Equal(nd->GetPosition(), ndDoubleInverse->GetPosition()), "Testing GetInverse double application: position preserved");
}

static void TestInverseError()
{
  // initialize empty NavigationData (quaternion is zeroed)
  mitk::NavigationData::Pointer nd = mitk::NavigationData::New();
  mitk::Quaternion quaternion;
  quaternion.fill(0);
  nd->SetOrientation(quaternion);

  MITK_TEST_FOR_EXCEPTION(mitk::Exception&, nd->GetInverse());
}

static void TestTransform()
{
  SetupNaviDataTests();
  mitk::NavigationData::Pointer navigationData = CreateNavidata(quaternion, offsetPoint);

  point = navigationData->TransformPoint(point);

  mitk::ScalarType resultingPointArray[] = {-2, 3, 8.123456};
  mitk::Point3D    resultingPoint        = resultingPointArray;
  MITK_TEST_CONDITION(mitk::Equal(resultingPoint, point), "Testing point transformation");
}

static void TestAffineConstructor()
{
  SetupNaviDataTests();
  mitk::AffineTransform3D::Pointer affineTransform3D = CreateAffineTransform(rotation, offsetVector);

  mitk::NavigationData::Pointer navigationData = mitk::NavigationData::New(affineTransform3D);

  MITK_TEST_CONDITION(mitk::Equal(navigationData->GetPosition(), offsetPoint), "Testing affine constructor: offset");
  MITK_TEST_CONDITION(mitk::Equal(navigationData->GetOrientation(), quaternion), "Testing affine constructor: quaternion");
  MITK_TEST_CONDITION(true == navigationData->GetHasPosition(), "Testing affine constructor: hasposition == true");
  MITK_TEST_CONDITION(true == navigationData->GetHasOrientation(), "Testing affine constructor: hasorientation == true");
  MITK_TEST_CONDITION(true == navigationData->IsDataValid(), "Testing affine constructor: isdatavalid == true");
  MITK_TEST_CONDITION(mitk::Equal(navigationData->GetIGTTimeStamp(),0.0), "Testing affine constructor: IGTTimestamp is zero");
}


static void TestAffineConstructorErrorTransposedNotInverse()
{
  SetupNaviDataTests();
  rotation.SetIdentity();
  rotation[1][0] = 2; // this matrix has determinant = 1 (triangular matrix with ones in diagonal) but transposed != inverse
  mitk::AffineTransform3D::Pointer affineTransform3D = CreateAffineTransform(rotation, offsetVector);

  MITK_TEST_FOR_EXCEPTION(mitk::Exception&, mitk::NavigationData::New(affineTransform3D));
}

static void TestAffineConstructorErrorDeterminantNonEqualOne()
{
  SetupNaviDataTests();
  rotation.SetIdentity();
  rotation[0][0] = 2; // determinant for diagonal matrices is product of diagonal elements => det = 2
  mitk::AffineTransform3D::Pointer affineTransform3D = CreateAffineTransform(rotation, offsetVector);

  MITK_TEST_FOR_EXCEPTION(mitk::Exception&, mitk::NavigationData::New(affineTransform3D));
}

static void TestAffineConstructorErrorCheckingFalse()
{
  SetupNaviDataTests();
  rotation.SetIdentity();
  rotation[0][0] = 2; // determinant for diagonal matrices is product of diagonal elements => det = 2
  mitk::AffineTransform3D::Pointer affineTransform3D = CreateAffineTransform(rotation, offsetVector);

  bool exceptionSuppressed = true;
  try
  {
    mitk::NavigationData::New(affineTransform3D, false);
  }
  catch (mitk::Exception&)
  {
    exceptionSuppressed = false;
  }

  MITK_TEST_CONDITION(exceptionSuppressed, "Test affine constructor: exception can be suppressed.")
}

static void TestAffineGetter()
{
  SetupNaviDataTests();
  mitk::NavigationData::Pointer navigationData = CreateNavidata(quaternion, offsetPoint);

  mitk::AffineTransform3D::Pointer affineTransform = navigationData->GetAffineTransform3D();

  MITK_TEST_CONDITION(mitk::Equal(affineTransform->GetOffset(), offsetVector), "Testing AffineTransform3D getter: offset");
  MITK_TEST_CONDITION(mitk::MatrixEqualElementWise(affineTransform->GetMatrix(), rotation), "Testing AffineTransform3D getter: rotation");
}

/**
* This test tests the complete chain from affineTransform -> NavigationData -> affineTransform
*/
static void TestAffineToNaviDataToAffine()
{
  SetupNaviDataTests();
  mitk::AffineTransform3D::Pointer affineTransform3D = CreateAffineTransform(rotation, offsetVector);

  // there and back again: affineTransform -> NavigationData -> affineTransform
  mitk::NavigationData::Pointer navigationData = mitk::NavigationData::New(affineTransform3D);
  mitk::AffineTransform3D::Pointer affineTransform3D_2;
  affineTransform3D_2 = navigationData->GetAffineTransform3D();

  MITK_TEST_CONDITION(mitk::Equal(affineTransform3D->GetOffset(), affineTransform3D_2->GetOffset()), "Testing affine -> navidata -> affine chain: offset");
  MITK_TEST_CONDITION(mitk::MatrixEqualElementWise(affineTransform3D->GetMatrix(), affineTransform3D_2->GetMatrix()), "Testing affine -> navidata -> affine chain: rotation");
}

static void TestCompose(bool pre = false)
{
  SetupNaviDataTests();
  mitk::NavigationData::Pointer    nd = CreateNavidata(quaternion, offsetPoint);
  mitk::AffineTransform3D::Pointer at = CreateAffineTransform(rotation, offsetVector);
  // second transform for composition
  mitk::NavigationData::Pointer    nd2 = CreateNavidata(quaternion2, offsetPoint2);
  mitk::AffineTransform3D::Pointer at2 = CreateAffineTransform(rotation2, offsetVector2);
  // save point for affinetransform
  mitk::Point3D point2 = point;


  nd->Compose(nd2, pre);
  point  = nd->TransformPoint(point);

  at->Compose(at2, pre);
  point2 = at->TransformPoint(point2);

  MITK_TEST_CONDITION(mitk::Equal(point, point2), "Compose pre = " << pre << ": composition works ");

  bool covarianceValidityReset = !nd->GetHasOrientation() && !nd->GetHasPosition();

  MITK_TEST_CONDITION(covarianceValidityReset, "Compose pre = " << pre << ": covariance validities reset because not implemented yet.");
}

static void TestReverseCompose()
{
  TestCompose(true);
}

/**
* This function is testing the Class mitk::NavigationData. For most tests we would need the MicronTracker hardware, so only a few
* simple tests, which can run without the hardware are implemented yet (2009, January, 23rd). As soon as there is a working
* concept to test the tracking classes which are very close to the hardware on all systems more tests are needed here.
*/
int mitkNavigationDataTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationData");

  //TestInstatiation();
  //TestGetterSetter();
  //TestGraft();
  //TestPrintSelf();
  //TestWrongInputs();

  //TestAffineConstructor();
  //TestAffineConstructorErrorDeterminantNonEqualOne();
  //TestAffineConstructorErrorTransposedNotInverse();
  //TestAffineConstructorErrorCheckingFalse();

  //TestAffineGetter();
  //TestAffineToNaviDataToAffine();

  //TestTransform();

  //TestInverse();
  //TestDoubleInverse();
  TestInverseError();

  //TestCompose();
  //TestReverseCompose();

  MITK_TEST_END();
}
