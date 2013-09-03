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
#include "vnl/vnl_math.h"

#include "itkIndent.h"

using namespace mitk;

class NavigationDataTestClass
  {
  public:
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
  };


  static Quaternion quaternion;
  static Vector3D   offsetVector;
  static Point3D    offsetPoint;
  static Matrix3D   rotation;

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
    quaternion = Quaternion(0, 0, 0.7071067811865475, 0.7071067811865476);

    // set offset to some value. Some tests need vectors, offers points.
    double offsetArray[3] = {1.0,2.0,3.123456};
    offsetVector          = offsetArray;
    offsetPoint           = offsetArray;
  }

  /**
   * Helper method, which creates a NavigationData object using the data created in SetupNaviDataTests()
   */
  static NavigationData::Pointer CreateNavidata(Quaternion quaternion, Point3D offset)
  {
    NavigationData::Pointer navigationData = NavigationData::New();
    navigationData->SetOrientation(quaternion);
    navigationData->SetPosition(offset);

    return navigationData;
  }

  static AffineTransform3D::Pointer CreateAffineTransform(Matrix3D rotationMatrix, Vector3D offset)
  {
    AffineTransform3D::Pointer affineTransform3D = AffineTransform3D::New();
    affineTransform3D->SetOffset(offset);
    affineTransform3D->SetMatrix(rotationMatrix);

    return affineTransform3D;
  }

  static void TestInverse()
  {
    SetupNaviDataTests();
    NavigationData::Pointer navigationData = CreateNavidata(quaternion, offsetPoint);



    MITK_TEST_CONDITION(false,"bla");
  }

  static void TestAffineConstructor()
  {
    SetupNaviDataTests();
    AffineTransform3D::Pointer affineTransform3D = CreateAffineTransform(rotation, offsetVector);

    NavigationData::Pointer navigationData = NavigationData::New(affineTransform3D);

    MITK_TEST_CONDITION(Equal(navigationData->GetPosition(), offsetPoint), "Testing affine constructor: offset");
    MITK_TEST_CONDITION(Equal(navigationData->GetOrientation(), quaternion), "Testing affine constructor: quaternion");
    MITK_TEST_CONDITION(true == navigationData->GetHasPosition(), "Testing affine constructor: hasposition == true");
    MITK_TEST_CONDITION(true == navigationData->GetHasOrientation(), "Testing affine constructor: hasorientation == true");
    MITK_TEST_CONDITION(true == navigationData->IsDataValid(), "Testing affine constructor: isdatavalid == true");
    MITK_TEST_CONDITION(Equal(navigationData->GetIGTTimeStamp(),0.0), "Testing affine constructor: IGTTimestamp is zero");

  }


  static void TestAffineConstructorErrorTransposedNotInverse()
  {
    SetupNaviDataTests();
    rotation.SetIdentity();
    rotation[1][0] = 2; // this matrix has determinant = 1 (triangular matrix with ones in diagonal) but transposed != inverse
    AffineTransform3D::Pointer affineTransform3D = CreateAffineTransform(rotation, offsetVector);

    MITK_TEST_FOR_EXCEPTION(mitk::Exception&, NavigationData::New(affineTransform3D));
  }

  static void TestAffineConstructorErrorDeterminantNonEqualOne()
  {
    SetupNaviDataTests();
    rotation.SetIdentity();
    rotation[0][0] = 2; // determinant for diagonal matrices is product of diagonal elements => det = 2
    AffineTransform3D::Pointer affineTransform3D = CreateAffineTransform(rotation, offsetVector);

    MITK_TEST_FOR_EXCEPTION(mitk::Exception&, NavigationData::New(affineTransform3D));
  }

  static void TestAffineConstructorErrorCheckingFalse()
  {
    SetupNaviDataTests();
    rotation.SetIdentity();
    rotation[0][0] = 2; // determinant for diagonal matrices is product of diagonal elements => det = 2
    AffineTransform3D::Pointer affineTransform3D = CreateAffineTransform(rotation, offsetVector);

    bool exceptionSuppressed = true;
    try
    {
      NavigationData::New(affineTransform3D, false);
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
    NavigationData::Pointer navigationData = CreateNavidata(quaternion, offsetPoint);

    AffineTransform3D::Pointer affineTransform = navigationData->GetAffineTransform3D();

    MITK_TEST_CONDITION(Equal(affineTransform->GetOffset(), offsetVector), "Testing AffineTransform3D getter: offset");
    MITK_TEST_CONDITION(MatrixEqualElementWise(affineTransform->GetMatrix(), rotation), "Testing AffineTransform3D getter: rotation");
  }

  /**
   * This test tests the complete chain from affineTransform -> NavigationData -> affineTransform
   */
  static void TestAffineToNaviDataToAffine()
  {
    SetupNaviDataTests();
    AffineTransform3D::Pointer affineTransform3D = CreateAffineTransform(rotation, offsetVector);

    // there and back again: affineTransform -> NavigationData -> affineTransform
    NavigationData::Pointer navigationData = NavigationData::New(affineTransform3D);
    AffineTransform3D::Pointer affineTransform3D_2;
    affineTransform3D_2 = navigationData->GetAffineTransform3D();

    MITK_TEST_CONDITION(Equal(affineTransform3D->GetOffset(), affineTransform3D_2->GetOffset()), "Testing affine -> navidata -> affine chain: offset");
    MITK_TEST_CONDITION(MatrixEqualElementWise(affineTransform3D->GetMatrix(), affineTransform3D_2->GetMatrix()), "Testing affine -> navidata -> affine chain: rotation");
  }

/**
* This function is testing the Class mitk::NavigationData. For most tests we would need the MicronTracker hardware, so only a few
* simple tests, which can run without the hardware are implemented yet (2009, January, 23rd). As soon as there is a working
* concept to test the tracking classes which are very close to the hardware on all systems more tests are needed here.
*/
int mitkNavigationDataTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationData");

  NavigationDataTestClass::TestInstatiation();
  NavigationDataTestClass::TestGetterSetter();
  NavigationDataTestClass::TestGraft();
  NavigationDataTestClass::TestPrintSelf();
  NavigationDataTestClass::TestWrongInputs();

  TestAffineConstructor();
  TestAffineConstructorErrorDeterminantNonEqualOne();
  TestAffineConstructorErrorTransposedNotInverse();
  TestAffineConstructorErrorCheckingFalse();

  TestAffineGetter();
  TestAffineToNaviDataToAffine();

  //TestInverse();


  MITK_TEST_END();
}
