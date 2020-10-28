/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

//MITK includes
#include "mitkVector.h"
#include "mitkStandardToftsModel.h"

class mitkStandardToftsModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkStandardToftsModelTestSuite);
  MITK_TEST(GetModelDisplayNameTest);
  MITK_TEST(GetModelTypeTest);
  MITK_TEST(GetParameterNamesTest);
  MITK_TEST(GetNumberOfParametersTest);
  MITK_TEST(GetParameterUnitsTest);
  MITK_TEST(GetDerivedParameterNamesTest);
  MITK_TEST(GetNumberOfDerivedParametersTest);
  MITK_TEST(GetDerivedParameterUnitsTest);
  MITK_TEST(ComputeModelfunctionTest);
  MITK_TEST(ComputeDerivedParametersTest);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::StandardToftsModel::Pointer m_testmodel;
  mitk::ModelBase::ModelResultType m_output;
  mitk::ModelBase::DerivedParameterMapType m_derivedParameters;

public:
  void setUp() override
  {
    mitk::ModelBase::TimeGridType m_grid(22);
    mitk::ModelBase::ParametersType m_testparameters(4);

    m_testparameters[mitk::StandardToftsModel::POSITION_PARAMETER_Ktrans] = 100.0;
    m_testparameters(mitk::StandardToftsModel::POSITION_PARAMETER_ve) = 0.3;



    for (int i = 0; i < 22; ++i)
    {
      // time grid in seconds, 14s between frames
      m_grid[i] = (double)14 * i;
    }
    //injection time in minutes --> 60s
    double m_injectiontime = 0.5;
    m_testmodel = mitk::StandardToftsModel::New();
    m_testmodel->SetTimeGrid(m_grid);
    //m_testmodel->SetAterialInputFunctionValues();
    //m_testmodel->SetAterialInputFunctionTimeGrid();

    //ComputeModelfunction is called within GetSignal(), therefore no explicit testing of ComputeModelFunction()
    m_output = m_testmodel->GetSignal(m_testparameters);
    m_derivedParameters = m_testmodel->GetDerivedParameters(m_testparameters);
  }

  void tearDown() override
  {
    m_testmodel = nullptr;
  }

  void GetModelDisplayNameTest()
  {
    //m_testmodel->GetModelDisplayName();
    //CPPUNIT_ASSERT_MESSAGE("Checking model display name.", m_testmodel->GetModelDisplayName() == "Standard Tofts Model");
  }


  void GetModelTypeTest()
  {
    //CPPUNIT_ASSERT_MESSAGE("Checking model type.", m_testmodel->GetModelType() == "Generic");
  }


  void GetParameterNamesTest()
  {
    //mitk::TwoStepLinearModel::ParameterNamesType parameterNames;
    //parameterNames.push_back("BaseValue");
    //parameterNames.push_back("Change_Point");
    //parameterNames.push_back("Slope_1");
    //parameterNames.push_back("Slope_2");
    //CPPUNIT_ASSERT_MESSAGE("Checking parameter names.", m_testmodel->GetParameterNames() == parameterNames);
  }

  void GetNumberOfParametersTest()
  {
    //CPPUNIT_ASSERT_MESSAGE("Checking number of parameters in model.", m_testmodel->GetNumberOfParameters() == 4);
  }


  void GetParameterUnitsTest()
  {

  }

  void GetDerivedParameterNamesTest()
  {
    //mitk::TwoStepLinearModel::ParameterNamesType derivedParameterNames;
    //derivedParameterNames.push_back("AUC");
    //derivedParameterNames.push_back("FinalUptake");
    //derivedParameterNames.push_back("Smax");
    //derivedParameterNames.push_back("y-intercept2");
    //CPPUNIT_ASSERT_MESSAGE("Checking derived parameter names.", m_testmodel->GetDerivedParameterNames() == derivedParameterNames);
  }

  void GetNumberOfDerivedParametersTest()
  {
    //CPPUNIT_ASSERT_MESSAGE("Checking number of parameters in model.", m_testmodel->GetNumberOfDerivedParameters() == 4);
  }

  void GetDerivedParameterUnitsTest()
  {
    //CPPUNIT_ASSERT_MESSAGE("Checking number of parameters in model.", m_testmodel->GetNumberOfDerivedParameters() == 4);
  }

  void ComputeModelfunctionTest()
  {
    //CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 0.", mitk::Equal(0, m_output[0], 1e-6, true) == true);
    //CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 4.", mitk::Equal(280.0, m_output[4], 1e-6, true) == true);
    //CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 9.", mitk::Equal(552.0, m_output[9], 1e-6, true) == true);
    //CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 14.", mitk::Equal(692.0, m_output[14], 1e-6, true) == true);
    //CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 19.", mitk::Equal(832.0, m_output[19], 1e-6, true) == true);
  }

  void ComputeDerivedParametersTest()
  {

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkStandardToftsModel)
