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
#include "mitkAIFBasedModelBase.h"

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
    mitk::ModelBase::TimeGridType m_grid(60);
    mitk::ModelBase::ParametersType m_testparameters(2);
    mitk::AIFBasedModelBase::AterialInputFunctionType m_arterialInputFunction (60);

    m_testparameters[mitk::StandardToftsModel::POSITION_PARAMETER_Ktrans] = 34.78;
    m_testparameters(mitk::StandardToftsModel::POSITION_PARAMETER_ve) = 0.5;

    m_arterialInputFunction.fill(0.0);
    m_arterialInputFunction[0] = 0.00000;
    m_arterialInputFunction[1] = -0.15248;
    m_arterialInputFunction[2] = -0.237782;
    m_arterialInputFunction[3] = -0.243189;
    m_arterialInputFunction[4] = -0.144531;
    m_arterialInputFunction[5] = -0.200168;
    m_arterialInputFunction[6] = 0.0922118;
    m_arterialInputFunction[7] = 3.51957;
    m_arterialInputFunction[8] = 9.62333;
    m_arterialInputFunction[9] = 9.07447;
    m_arterialInputFunction[10] = 5.97842;
    m_arterialInputFunction[11] = 4.96097;
    m_arterialInputFunction[12] = 5.76308;
    m_arterialInputFunction[13] = 5.17594;
    m_arterialInputFunction[14] = 4.41313;
    m_arterialInputFunction[15] = 4.38105;
    m_arterialInputFunction[16] = 4.87772;
    m_arterialInputFunction[17] = 4.5807;
    m_arterialInputFunction[18] = 4.53279;
    m_arterialInputFunction[19] = 4.35079;
    m_arterialInputFunction[20] = 4.29456;
    m_arterialInputFunction[21] = 3.99838;
    m_arterialInputFunction[22] = 4.42678;
    m_arterialInputFunction[23] = 4.0375;
    m_arterialInputFunction[24] = 4.03682;
    m_arterialInputFunction[25] = 3.86102;
    m_arterialInputFunction[26] = 3.60841;
    m_arterialInputFunction[27] = 3.79723;
    m_arterialInputFunction[28] = 3.82096;
    m_arterialInputFunction[29] = 3.81099;
    m_arterialInputFunction[30] = 3.43013;
    m_arterialInputFunction[31] = 3.48450;
    m_arterialInputFunction[32] = 4.10285;
    m_arterialInputFunction[33] = 3.57935;
    m_arterialInputFunction[34] = 3.59937;
    m_arterialInputFunction[35] = 3.79997;
    m_arterialInputFunction[36] = 3.55403;
    m_arterialInputFunction[37] = 2.90888;
    m_arterialInputFunction[38] = 3.60713;
    m_arterialInputFunction[39] = 3.73377;
    m_arterialInputFunction[40] = 3.31633;
    m_arterialInputFunction[41] = 3.62506;
    m_arterialInputFunction[42] = 3.12624;
    m_arterialInputFunction[43] = 3.41160;
    m_arterialInputFunction[44] = 3.20870;
    m_arterialInputFunction[45] = 2.94028;
    m_arterialInputFunction[46] = 3.43235;
    m_arterialInputFunction[47] = 3.23044;
    m_arterialInputFunction[48] = 3.03722;
    m_arterialInputFunction[49] = 3.11801;
    m_arterialInputFunction[50] = 3.25045;
    m_arterialInputFunction[51] = 3.23418;
    m_arterialInputFunction[52] = 3.0279;
    m_arterialInputFunction[53] = 2.95201;
    m_arterialInputFunction[54] = 3.03391;
    m_arterialInputFunction[55] = 3.19314;
    m_arterialInputFunction[56] = 3.24602;
    m_arterialInputFunction[57] = 2.9689;
    m_arterialInputFunction[58] = 2.88674;
    m_arterialInputFunction[59] = 3.39835;

    m_grid.fill(0.0);
    m_grid[0] = 0.00;
    m_grid[1] = 4.270;
    m_grid[2] = 8.541;
    m_grid[3] = 12.812;
    m_grid[4] = 17.082;
    m_grid[5] = 21.353;
    m_grid[6] = 25.624;
    m_grid[7] = 29.894;
    m_grid[8] = 34.165;
    m_grid[9] = 38.436;
    m_grid[10] = 42.706;
    m_grid[11] = 46.977;
    m_grid[12] = 51.248;
    m_grid[13] = 55.518;
    m_grid[14] = 59.789;
    m_grid[15] = 64.06;
    m_grid[16] = 68.331;
    m_grid[17] = 72.601;
    m_grid[18] = 76.872;
    m_grid[19] = 81.143;
    m_grid[20] = 85.413;
    m_grid[21] = 89.684;
    m_grid[22] = 93.955;
    m_grid[23] = 98.225;
    m_grid[24] = 102.496;
    m_grid[25] = 106.767;
    m_grid[26] = 111.037;
    m_grid[27] = 115.308;
    m_grid[28] = 119.579;
    m_grid[29] = 123.850;
    m_grid[30] = 128.120;
    m_grid[31] = 132.391;
    m_grid[32] = 136.662;
    m_grid[33] = 140.932;
    m_grid[34] = 145.203;
    m_grid[35] = 149.474;
    m_grid[36] = 153.744;
    m_grid[37] = 158.015;
    m_grid[38] = 162.286;
    m_grid[39] = 166.556;
    m_grid[40] = 170.827;
    m_grid[41] = 175.098;
    m_grid[42] = 179.369;
    m_grid[43] = 183.639;
    m_grid[44] = 187.910;
    m_grid[45] = 192.181;
    m_grid[46] = 196.451;
    m_grid[47] = 200.722;
    m_grid[48] = 204.993;
    m_grid[49] = 209.263;
    m_grid[50] = 213.534;
    m_grid[51] = 217.805;
    m_grid[52] = 222.075;
    m_grid[53] = 226.346;
    m_grid[54] = 230.617;
    m_grid[55] = 234.888;
    m_grid[56] = 239.158;
    m_grid[57] = 243.429;
    m_grid[58] = 247.700;
    m_grid[59] = 251.970;



    //injection time in minutes --> 60s
    //double m_injectiontime = 0.5;
    m_testmodel = mitk::StandardToftsModel::New();
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
    CPPUNIT_ASSERT_MESSAGE("Checking model display name.", m_testmodel->GetModelDisplayName() == "Standard Tofts Model");
  }


  void GetModelTypeTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking model type.", m_testmodel->GetModelType() == "Perfusion.MR");
  }


  void GetParameterNamesTest()
  {
    mitk::StandardToftsModel::ParameterNamesType parameterNames;
    parameterNames.push_back("KTrans");
    parameterNames.push_back("ve");
    CPPUNIT_ASSERT_MESSAGE("Checking parameter names.", m_testmodel->GetParameterNames() == parameterNames);
  }

  void GetNumberOfParametersTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking number of parameters in model.", m_testmodel->GetNumberOfParameters() == 2);
  }


  void GetParameterUnitsTest()
  {
    mitk::StandardToftsModel::ParamterUnitMapType parameterUnits;
    parameterUnits.insert(std::make_pair("KTrans", "ml/min/100ml"));
    parameterUnits.insert(std::make_pair("ve", "ml/ml"));

    CPPUNIT_ASSERT_MESSAGE("Checking parameter units.", m_testmodel->GetParameterUnits() == parameterUnits);
  }

  void GetDerivedParameterNamesTest()
  {
    mitk::StandardToftsModel::ParameterNamesType derivedParameterNames;

    derivedParameterNames.push_back("kep");

    CPPUNIT_ASSERT_MESSAGE("Checking derived parameter names.", m_testmodel->GetDerivedParameterNames() == derivedParameterNames);
  }

  void GetNumberOfDerivedParametersTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking number of parameters in model.", m_testmodel->GetNumberOfDerivedParameters() == 1);
  }

  void GetDerivedParameterUnitsTest()
  {
    mitk::StandardToftsModel::ParamterUnitMapType derivedParameterUnits;
    derivedParameterUnits.insert(std::make_pair("kep", "1/min"));

    CPPUNIT_ASSERT_MESSAGE("Checking parameter units.", m_testmodel->GetDerivedParameterUnits() == derivedParameterUnits);
  }

  void ComputeModelfunctionTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 0.", mitk::Equal(0, m_output[0], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 10.", mitk::Equal(0.562266, m_output[10], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 20.", mitk::Equal(1.273072, m_output[20], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 30.", mitk::Equal(1.537241, m_output[30], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 40.", mitk::Equal(1.632757, m_output[40], 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking signal at time frame 50.", mitk::Equal(1.625615, m_output[50], 1e-6, true) == true);
  }

  void ComputeDerivedParametersTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking kep.", mitk::Equal(69.56, m_derivedParameters["kep"], 1e-6, true) == true);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkStandardToftsModel)
