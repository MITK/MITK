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
#include "math.h"

//MITK includes
#include "mitkVector.h"
#include "mitkExtendedToftsModel.h"
#include "mitkAIFBasedModelBase.h"

class mitkExtendedToftsModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkExtendedToftsModelTestSuite);
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
  mitk::ExtendedToftsModel::Pointer m_testmodel;
  mitk::ModelBase::ModelResultType m_output;
  mitk::ModelBase::DerivedParameterMapType m_derivedParameters;

public:
  void setUp() override
  {
    mitk::ModelBase::TimeGridType m_grid(60);
    mitk::ModelBase::ParametersType m_testparameters(3);
    mitk::AIFBasedModelBase::AterialInputFunctionType m_arterialInputFunction (60);

    m_testparameters[mitk::ExtendedToftsModel::POSITION_PARAMETER_Ktrans] = 35.0;
    m_testparameters(mitk::ExtendedToftsModel::POSITION_PARAMETER_ve) = 0.5;
    m_testparameters[mitk::ExtendedToftsModel::POSITION_PARAMETER_vp] = 0.05;

    for (int i = 0; i < 22; ++i)
    {
      // time grid in seconds, 14s between frames
      m_grid[i] = (double)14 * i;
    }

    // AIF from Weinmann, H. J., Laniado, M., and W.Mützel (1984). Pharmacokinetics of GD - DTPA / dimeglumine after intravenous injection into healthy volunteers. Phys Chem Phys Med NMR, 16(2) : 167–72.
    int D = 1;
    double a1 = 3.99;
    double m1 = 0.144;
    double a2 = 4.78;
    double m2 = 0.0111;
    for (int i = 0; i < 22; ++i)
    {
      m_arterialInputFunction[i] = D * (a1 * exp(-m1 * m_grid[i]) + a2 * exp(-m2 * m_grid[i]));
    }

    m_testmodel = mitk::ExtendedToftsModel::New();
    m_testmodel->SetTimeGrid(m_grid);
    m_testmodel->SetAterialInputFunctionValues(m_arterialInputFunction);
    m_testmodel->SetAterialInputFunctionTimeGrid(m_grid);

    //ComputeModelfunction is called within GetSignal(), therefore no explicit testing of ComputeModelFunction()
    m_output = m_testmodel->GetSignal(m_testparameters);
    m_derivedParameters = m_testmodel->GetDerivedParameters(m_testparameters);
  }

  void tearDown() override
  {
    m_testmodel = nullptr;
    m_output.clear();
    m_derivedParameters.clear();
  }

  void GetModelDisplayNameTest()
  {
    m_testmodel->GetModelDisplayName();
    CPPUNIT_ASSERT_MESSAGE("Checking model display name.", m_testmodel->GetModelDisplayName() == "Extended Tofts Model");
  }


  void GetModelTypeTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking model type.", m_testmodel->GetModelType() == "Perfusion.MR");
  }


  void GetParameterNamesTest()
  {
    mitk::ExtendedToftsModel::ParameterNamesType parameterNames;
    parameterNames.push_back("KTrans");
    parameterNames.push_back("ve");
    parameterNames.push_back("vp");
    CPPUNIT_ASSERT_MESSAGE("Checking parameter names.", m_testmodel->GetParameterNames() == parameterNames);
  }

  void GetNumberOfParametersTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking number of parameters in model.", m_testmodel->GetNumberOfParameters() == 3);
  }


  void GetParameterUnitsTest()
  {
    mitk::ExtendedToftsModel::ParamterUnitMapType parameterUnits;
    parameterUnits.insert(std::make_pair("KTrans", "ml/min/100ml"));
    parameterUnits.insert(std::make_pair("vp", "ml/ml"));
    parameterUnits.insert(std::make_pair("ve","ml/ml"));

    CPPUNIT_ASSERT_MESSAGE("Checking parameter units.", m_testmodel->GetParameterUnits() == parameterUnits);
  }

  void GetDerivedParameterNamesTest()
  {
    mitk::ExtendedToftsModel::ParameterNamesType derivedParameterNames;

    derivedParameterNames.push_back("kep");

    CPPUNIT_ASSERT_MESSAGE("Checking derived parameter names.", m_testmodel->GetDerivedParameterNames() == derivedParameterNames);
  }

  void GetNumberOfDerivedParametersTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking number of parameters in model.", m_testmodel->GetNumberOfDerivedParameters() == 1);
  }

  void GetDerivedParameterUnitsTest()
  {
    mitk::ExtendedToftsModel::ParamterUnitMapType derivedParameterUnits;
    derivedParameterUnits.insert(std::make_pair("kep", "1/min"));

    CPPUNIT_ASSERT_MESSAGE("Checking parameter units.", m_testmodel->GetDerivedParameterUnits() == derivedParameterUnits);
  }

  void ComputeModelfunctionTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 0.", mitk::Equal(0.438500, m_output[0], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 5.", mitk::Equal(1.094436, m_output[5], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 10", mitk::Equal(0.890956, m_output[10], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 15", mitk::Equal(0.580996, m_output[15], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 20", mitk::Equal(0.342851, m_output[20], 1e-6, true) == true);
  }

  void ComputeDerivedParametersTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking kep.", mitk::Equal(70.00, m_derivedParameters["kep"], 1e-6, true) == true);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkExtendedToftsModel)
