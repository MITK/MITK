/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkVector.h"

#include "mitkDescriptivePharmacokineticBrixModel.h"



int mitkDescriptivePharmacokineticBrixModelTest(int  /*argc*/ , char*[] /*argv[]*/){

    MITK_TEST_BEGIN("DescriptivePharmacokineticBrixModel")

    mitk::ModelBase::TimeGridType grid(22);
    mitk::ModelBase::ModelResultType output;
    mitk::ModelBase::ParametersType testparameters(4);

    testparameters[mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_A] = 1.25;
    testparameters(mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_kep) = 3.89;
    testparameters(mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_kel) = 0.12;
    testparameters(mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_tlag) = 1.14;



    for (int i = 0; i<22; ++i)
    {
   // time grid in seconds, 14s between frames
      grid[i]=(double) 14*i;
    }
    //injection time in minutes --> 60s
    double injectiontime = 0.5;
    mitk::DescriptivePharmacokineticBrixModel::Pointer testmodel = mitk::DescriptivePharmacokineticBrixModel::New();
    testmodel->SetTimeGrid(grid);
    testmodel->SetTau(injectiontime);

    CPPUNIT_ASSERT_MESSAGE("Check number of parameters in Model.", 4 == testmodel->GetNumberOfParameters());
    output = testmodel->GetSignal(testparameters);
    //precision of output ?
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(1,output[0], 1e-6, true)==true,"Check Output signal values - 0");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(1,output[4], 1e-6, true)==true,"Check Output signal values - 4");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(1.003338,output[5], 1e-6, true)==true,"Check Output signal values - 5");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(1.238392,output[6], 1e-6, true)==true,"Check Output signal values - 6");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(1.669835,output[7], 1e-6, true)==true,"Check Output signal values - 7");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(1.982948,output[8], 1e-6, true)==true,"Check Output signal values - 8");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(2.089685,output[9], 1e-6, true)==true,"Check Output signal values - 9");
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(2.113611,output[10], 1e-6, true)==true,"Check Output signal values - 10");

    MITK_TEST_END()

}
