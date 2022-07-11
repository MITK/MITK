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
#include "mitkDescriptivePharmacokineticBrixModel.h"

class mitkDescriptivePharmacokineticBrixModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDescriptivePharmacokineticBrixModelTestSuite);
  MITK_TEST(GetModelDisplayNameTest);
  MITK_TEST(GetModelTypeTest);
  MITK_TEST(GetXAxisNameTest);
  MITK_TEST(GetXAxisUnitTest);
  MITK_TEST(GetYAxisNameTest);
  MITK_TEST(GetYAxisUnitTest);
  MITK_TEST(GetParameterNamesTest);
  MITK_TEST(GetNumberOfParametersTest);
  MITK_TEST(GetParameterUnitsTest);
  MITK_TEST(GetStaticParameterNamesTest);
  MITK_TEST(GetNumberOfStaticParametersTest);
  MITK_TEST(GetStaticParameterUnitsTest);
  MITK_TEST(ComputeModelfunctionTest);
  CPPUNIT_TEST_SUITE_END();

  private:
    mitk::DescriptivePharmacokineticBrixModel::Pointer m_testmodel;
    std::string NAME_PARAMETER_A, POSITION_PARAMETER_kep, POSITION_PARAMETER_kel, POSITION_PARAMETER_tlag;
    mitk::ModelBase::ModelResultType m_output;

  public:
    void setUp() override
    {
      mitk::ModelBase::TimeGridType m_grid(22);
      mitk::ModelBase::ParametersType m_testparameters(4);

      m_testparameters[mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_A] = 1.25;
      m_testparameters(mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_kep) = 3.89;
      m_testparameters(mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_kel) = 0.12;
      m_testparameters(mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_tlag) = 1.14;

      for (int i = 0; i < 22; ++i)
      {
        // time grid in seconds, 14s between frames
        m_grid[i] = (double)14 * i;
      }
      //injection time in minutes --> 30s
      double m_injectiontime = 0.5;
      m_testmodel = mitk::DescriptivePharmacokineticBrixModel::New();
      m_testmodel->SetTimeGrid(m_grid);
      m_testmodel->SetTau(m_injectiontime);

      //ComputeModelfunction is called within GetSignal(), therefore no explicit testing of ComputeModelFunction()
      m_output = m_testmodel->GetSignal(m_testparameters);
    }

    void tearDown() override
    {
      m_testmodel = nullptr;
      NAME_PARAMETER_A = "";
      POSITION_PARAMETER_kep = "";
      POSITION_PARAMETER_kel = "";
      POSITION_PARAMETER_tlag = "";
      m_output.clear();
    }

    void GetModelDisplayNameTest()
    {
      m_testmodel->GetModelDisplayName();
      CPPUNIT_ASSERT_MESSAGE("Checking model display name.", m_testmodel->GetModelDisplayName() == "Descriptive Pharmacokinetic Brix Model");
    }

    void GetModelTypeTest()
    {
      CPPUNIT_ASSERT_MESSAGE("Checking model type.", m_testmodel->GetModelType() == "Perfusion.MR");
    }

    void GetXAxisNameTest()
    {
      CPPUNIT_ASSERT_MESSAGE("Checking x-axis name.", m_testmodel->GetXAxisName() == "Time");
    }

    void GetXAxisUnitTest()
    {
      CPPUNIT_ASSERT_MESSAGE("Checking x-axis unit", m_testmodel->GetXAxisUnit() == "s");
    }

    void GetYAxisNameTest()
    {
      CPPUNIT_ASSERT_MESSAGE("Checking y-axis name.", m_testmodel->GetYAxisName() == "");
    }

    void GetYAxisUnitTest()
    {
      CPPUNIT_ASSERT_MESSAGE("Checking y-axis unit", m_testmodel->GetYAxisUnit() == "");
    }

    void GetParameterNamesTest()
    {
      mitk::DescriptivePharmacokineticBrixModel::ParameterNamesType parameterNames;
      parameterNames.push_back("A");
      parameterNames.push_back("kep");
      parameterNames.push_back("kel");
      parameterNames.push_back("tlag");
      CPPUNIT_ASSERT_MESSAGE("Checking parameter names.", m_testmodel->GetParameterNames() == parameterNames);
    }

    void GetNumberOfParametersTest()
    {
      CPPUNIT_ASSERT_MESSAGE("Checking number of parameters in model.", m_testmodel->GetNumberOfParameters() == 4);
    }

    void GetParameterUnitsTest()
    {
      mitk::DescriptivePharmacokineticBrixModel::ParamterUnitMapType parameterUnitMap;
      parameterUnitMap.insert(std::make_pair("A", "a.u."));
      parameterUnitMap.insert(std::make_pair("kep", "1/min"));
      parameterUnitMap.insert(std::make_pair("kel", "1/min"));
      parameterUnitMap.insert(std::make_pair("tlag", "min"));
      CPPUNIT_ASSERT_MESSAGE("Checking parameter unit map.", m_testmodel->GetParameterUnits() == parameterUnitMap);
    }

    void GetStaticParameterNamesTest()
    {
      mitk::DescriptivePharmacokineticBrixModel::ParameterNamesType staticParameterNames;
      staticParameterNames.push_back("Tau");
      staticParameterNames.push_back("S0");
      CPPUNIT_ASSERT_MESSAGE("Checking static parameter names.", m_testmodel->GetStaticParameterNames() == staticParameterNames);
    }

    void GetNumberOfStaticParametersTest()
    {
      CPPUNIT_ASSERT_MESSAGE("Checking number of static parameters.", m_testmodel->GetNumberOfStaticParameters() == 2);
    }

    void GetStaticParameterUnitsTest()
    {
      mitk::DescriptivePharmacokineticBrixModel::ParamterUnitMapType staticParameterUnitMap;
      staticParameterUnitMap.insert(std::make_pair("Tau", "min"));
      staticParameterUnitMap.insert(std::make_pair("S0", "I"));
      CPPUNIT_ASSERT_MESSAGE("Checking static parameters units.", m_testmodel->GetStaticParameterUnits() == staticParameterUnitMap);
    }

    void ComputeModelfunctionTest()
    {
      CPPUNIT_ASSERT_MESSAGE("Checking signal of parameter set 1 at time frame 0.", mitk::Equal(1, m_output[0], 1e-6, true) == true);
      CPPUNIT_ASSERT_MESSAGE("Checking signal of parameter set 1 at time frame 10.",mitk::Equal(2.113611, m_output[10], 1e-6, true) == true);
      CPPUNIT_ASSERT_MESSAGE("Checking signal of parameter set 1 at time frame 20.", mitk::Equal(1.870596, m_output[20], 1e-6, true) == true);
    }
};

MITK_TEST_SUITE_REGISTRATION(mitkDescriptivePharmacokineticBrixModel)
