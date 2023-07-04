/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkModelTestFixture.h"

//MITK includes
#include "mitkT2DecayModel.h"


  class mitkT2DecayModelTestSuite : public mitk::mitkModelTestFixture
  {
    CPPUNIT_TEST_SUITE(mitkT2DecayModelTestSuite);
    MITK_TEST(GetModelInfoTest);
    MITK_TEST(ComputeModelfunctionTest);
    MITK_TEST(ComputeDerivedParametersTest);
    CPPUNIT_TEST_SUITE_END();

  private:
    json m_profile_json_obj;
    json m_modelValues_json_obj;
    mitk::ModelBase::Pointer m_testmodel;

  public:
    void setUp() override
    {
      // Parse JSON files
      m_profile_json_obj = ParseJSONFile("ModelFit/mitkT2DecayModelTest_profile.json");
      m_modelValues_json_obj = ParseJSONFile("ModelFit/mitkT2DecayModelTest_modelValues.json");

      // Generate test model
      m_testmodel = mitk::T2DecayModel::New();
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

MITK_TEST_SUITE_REGISTRATION(mitkT2DecayModel)
