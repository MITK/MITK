/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

 //Testing
#include "mitkModelTestFixture.h"

//MITK includes
#include "mitkExtendedOneTissueCompartmentModel.h"


  class mitkExtendedOneTissueCompartmentModelTestSuite : public mitk::mitkModelTestFixture
{
  CPPUNIT_TEST_SUITE(mitkExtendedOneTissueCompartmentModelTestSuite);
  MITK_TEST(GetModelInfoTest);
  MITK_TEST(ComputeModelfunctionTest);
  MITK_TEST(ComputeDerivedParametersTest);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::AIFBasedModelBase::Pointer m_testmodel;

  json m_profile_json_obj;
  json m_modelValues_json_obj;

public:
  void setUp() override
  {
    // Parse JSON files
    m_profile_json_obj = ParseJSONFile("Pharmacokinetics/mitkExtendedOneTissueCompartmentModelTest_profile.json");
    m_modelValues_json_obj = ParseJSONFile("Pharmacokinetics/mitkExtendedOneTissueCompartmentModelTest_modelValues.json");

    // Generate test model
    m_testmodel = mitk::ExtendedOneTissueCompartmentModel::New();
 }

  void tearDown() override
  {
  }

  void GetModelInfoTest()
  {
    // comparison of reference and testmodel profile
    CompareModelAndReferenceProfile(m_testmodel, m_profile_json_obj);
  }

  void ComputeModelfunctionTest()
  {
      CompareModelAndReferenceSignal(m_testmodel, m_modelValues_json_obj, m_profile_json_obj);
  }

  void ComputeDerivedParametersTest()
  {
      CompareModelAndReferenceDerivedParameters(m_testmodel, m_modelValues_json_obj);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkExtendedOneTissueCompartmentModel)
