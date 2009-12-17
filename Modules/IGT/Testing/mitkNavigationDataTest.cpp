/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-18 21:46:13 +0200 (So, 18. Okt 2009) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTestingMacros.h"
#include "mitkNavigationData.h"
//#include "mitkTrackingTypes.h"
//
//#include "mitkCommon.h"
//
//#include <itkObject.h>
//#include <itkObjectFactory.h>

/**
* This function is testing the Class mitk::NavigationData. For most tests we would need the MicronTracker hardware, so only a few
* simple tests, which can run without the hardware are implemented yet (2009, January, 23rd). As soon as there is a working
* concept to test the tracking classes which are very close to the hardware on all systems more tests are needed here.
*/
int mitkNavigationDataTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationData");

  // Test instantiation of NavigationData
  mitk::NavigationData::Pointer nd = mitk::NavigationData::New();
  MITK_TEST_CONDITION(nd.IsNotNull(),"Test instatiation");

  mitk::NavigationData::PositionType p;
  mitk::FillVector3D(p, 44.4, 55.5, 66.66);
  nd->SetPosition(p);
  MITK_TEST_CONDITION(nd->GetPosition() == p, "Set-/GetPosition()");

  mitk::NavigationData::OrientationType o(1.0, 2.0, 3.0, 4.0);
  nd->SetOrientation(o);
  MITK_TEST_CONDITION(nd->GetOrientation() == o, "Set-/GetOrientation()");

  nd->SetDataValid(true);
  MITK_TEST_CONDITION(nd->IsDataValid() == true, "Set-/IsDataValid()");

  nd->SetTimeStamp(100.111);
  MITK_TEST_CONDITION(mitk::Equal(nd->GetTimeStamp(), 100.111), "Set-/GetTimeStamp()");

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


  mitk::NavigationData::Pointer graftedCopy = mitk::NavigationData::New();
  graftedCopy->Graft(nd);

  bool graftIsEqual = (nd->GetPosition() == graftedCopy->GetPosition())
                   && (nd->GetOrientation() == graftedCopy->GetOrientation())
                   && (nd->IsDataValid() == graftedCopy->IsDataValid())
                   && mitk::Equal(nd->GetTimeStamp(), graftedCopy->GetTimeStamp())
                   && (nd->GetHasPosition() == graftedCopy->GetHasPosition())
                   && (nd->GetHasOrientation() == graftedCopy->GetHasOrientation())
                   && (nd->GetCovErrorMatrix() == graftedCopy->GetCovErrorMatrix())
                   && (std::string(nd->GetName()) == graftedCopy->GetName());
  

  MITK_TEST_CONDITION(graftIsEqual, "Graft() produces equal NavigationData object");


  MITK_TEST_END();
}
