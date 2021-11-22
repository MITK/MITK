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
#include "mitkTwoCompartmentExchangeModel.h"
#include "mitkAIFBasedModelBase.h"

class mitkTwoCompartmentExchangeModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkTwoCompartmentExchangeModelTestSuite);
  MITK_TEST(GetModelDisplayNameTest);
  MITK_TEST(GetModelTypeTest);
  MITK_TEST(GetParameterNamesTest);
  MITK_TEST(GetNumberOfParametersTest);
  MITK_TEST(GetParameterUnitsTest);
  MITK_TEST(ComputeModelfunctionTest);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::TwoCompartmentExchangeModel::Pointer m_testmodel;
  mitk::ModelBase::ModelResultType m_output;

public:
  void setUp() override
  {
    mitk::ModelBase::TimeGridType m_grid(22);
    mitk::ModelBase::ParametersType m_testparameters(4);
    mitk::AIFBasedModelBase::AterialInputFunctionType m_arterialInputFunction (22);

    m_testparameters[mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_F] = 35.0;
    m_testparameters(mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_PS) = 5.0;
    m_testparameters(mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_ve) = 0.5;
    m_testparameters(mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_vp) = 0.05;


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
      if (i < 5)
        m_arterialInputFunction[i] = 0;
      else
        m_arterialInputFunction[i] = D * (a1 * exp(-m1 * m_grid[i]) + a2 * exp(-m2 * m_grid[i]));
    }

    m_testmodel = mitk::TwoCompartmentExchangeModel::New();
    m_testmodel->SetTimeGrid(m_grid);
    m_testmodel->SetAterialInputFunctionValues(m_arterialInputFunction);
    m_testmodel->SetAterialInputFunctionTimeGrid(m_grid);

    //ComputeModelfunction is called within GetSignal(), therefore no explicit testing of ComputeModelFunction()
    m_output = m_testmodel->GetSignal(m_testparameters);
  }
  void tearDown() override
  {
    m_testmodel = nullptr;
    m_output.clear();
  }

  void GetModelDisplayNameTest()
  {
    m_testmodel->GetModelDisplayName();
    CPPUNIT_ASSERT_MESSAGE("Checking model display name.", m_testmodel->GetModelDisplayName() == "Two Compartment Exchange Model");
  }


  void GetModelTypeTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking model type.", m_testmodel->GetModelType() == "Perfusion.MR");
  }


  void GetParameterNamesTest()
  {
    mitk::TwoCompartmentExchangeModel::ParameterNamesType parameterNames;
    parameterNames.push_back("F");
    parameterNames.push_back("PS");
    parameterNames.push_back("ve");
    parameterNames.push_back("vp");
    CPPUNIT_ASSERT_MESSAGE("Checking parameter names.", m_testmodel->GetParameterNames() == parameterNames);
  }


  void GetNumberOfParametersTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking number of parameters in model.", m_testmodel->GetNumberOfParameters() == 4);
  }


  void GetParameterUnitsTest()
  {
    mitk::TwoCompartmentExchangeModel::ParamterUnitMapType parameterUnits;
    parameterUnits.insert(std::make_pair("F", "ml/min/100ml"));
    parameterUnits.insert(std::make_pair("PS", "ml/min/100ml"));
    parameterUnits.insert(std::make_pair("ve", "ml/ml"));
    parameterUnits.insert(std::make_pair("vp", "ml/ml"));

    CPPUNIT_ASSERT_MESSAGE("Checking parameter units.", m_testmodel->GetParameterUnits() == parameterUnits);
  }

  void ComputeModelfunctionTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 0.", mitk::Equal(0.0, m_output[0], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 5.", mitk::Equal(0.057246, m_output[5], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 10.", mitk::Equal(0.127806, m_output[10], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 15.", mitk::Equal(0.131465, m_output[15], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 20.", mitk::Equal(0.126101, m_output[20], 1e-6, true) == true);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkTwoCompartmentExchangeModel)
