/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKMODELTESTFIXTURE_H
#define MITKMODELTESTFIXTURE_H


#include "mitkTestingMacros.h"
#include <mitkTestingConfig.h>
#include "mitkTestFixture.h"
#include <itksys/SystemTools.hxx>
#include <mitkModelBase.h>
#include "mitkModelFitException.h"

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include "mitkVector.h"

using json = nlohmann::json;

namespace mitk
{
  class mitkModelTestFixture : public mitk::TestFixture
  {
  public:
    static json ParseJSONFile(const std::string path)
    {
      std::string pathToFile = GetTestDataFilePath(path);
      std::ifstream file(pathToFile);
      json json_obj;
      if (!file.is_open())
      {
        MITK_ERROR << "Could not open \"" << pathToFile << "\"!";
      }
      try
      {
        json_obj = nlohmann::json::parse(file);
      }
      catch (const json::exception& e)
      {
        MITK_ERROR << "Could not parse JSON file!";
      }
      return json_obj;
    }

    static ModelBase::ParametersType ParseTestParameters(const json modelValues_json_obj)
    {
      ModelBase::ParametersType testparameters;
      testparameters.SetSize(modelValues_json_obj["modelParameterValues"].size());
      for (unsigned long i = 0; i < modelValues_json_obj["modelParameterValues"].size(); ++i)
      {
        testparameters[i] = modelValues_json_obj["modelParameterValues"][i];
      }
      return testparameters;
    }

    static ModelBase::StaticParameterMapType ParseStaticParameters(const json profile_json_obj, const json modelValues_json_obj)
    {
      ModelBase::StaticParameterMapType staticParameterMap;
      for (unsigned long i = 0; i < modelValues_json_obj["staticParameterValues"].size(); ++i)
      {
        ModelBase::StaticParameterValuesType staticParameterValues;
        ModelBase::ParameterNameType staticParameterName;
        staticParameterName = profile_json_obj["staticParameterNames"][i];
        for (unsigned int j = 0; j < modelValues_json_obj["staticParameterValues"][i].size(); ++j)
        {
          staticParameterValues.push_back(modelValues_json_obj["staticParameterValues"][i][j]);
        }
        staticParameterMap.insert(std::make_pair(staticParameterName, staticParameterValues));
      }
      return staticParameterMap;
    }

    static void SetStaticParametersForTest(mitk::ModelBase::Pointer testmodel, const json profile_json_obj, const json modelValues_json_obj)
    {
      mitk::ModelBase::StaticParameterMapType staticParameterMap;
      staticParameterMap = ParseStaticParameters(profile_json_obj, modelValues_json_obj);
      testmodel->SetStaticParameters(staticParameterMap);
    }

    static void CompareModelAndReferenceProfile(const mitk::ModelBase::Pointer testmodel, const json profile_json_obj)
    {
      CPPUNIT_ASSERT_MESSAGE("Checking number of parameters in model.", testmodel->GetNumberOfParameters() == profile_json_obj["numberOfParameters"]);
      for (unsigned long i = 0; i < profile_json_obj["numberOfParameters"]; i++)
      {
        CPPUNIT_ASSERT_MESSAGE("Checking parameter names.", testmodel->GetParameterNames()[i] == profile_json_obj["parameterNames"][i]);
        CPPUNIT_ASSERT_MESSAGE("Checking parameter scales.", testmodel->GetParameterScales()[testmodel->GetParameterNames()[i]] == profile_json_obj["parameterScales"][i]);
        CPPUNIT_ASSERT_MESSAGE("Checking parameter units.", testmodel->GetParameterUnits()[testmodel->GetParameterNames()[i]] == profile_json_obj["parameterUnits"][i]);
      }
      CPPUNIT_ASSERT_MESSAGE("Checking number of derived parameters in model.", testmodel->GetNumberOfDerivedParameters() == profile_json_obj["numberOfDerivedParameters"]);
      for (unsigned long i = 0; i < profile_json_obj["numberOfDerivedParameters"]; i++)
      {
        CPPUNIT_ASSERT_MESSAGE("Checking derived parameter names.", testmodel->GetDerivedParameterNames()[i] == profile_json_obj["derivedParameterNames"][i]);
        CPPUNIT_ASSERT_MESSAGE("Checking derived parameter scales.", testmodel->GetDerivedParameterScales()[testmodel->GetDerivedParameterNames()[i]] == profile_json_obj["derivedParameterScales"][i]);
        CPPUNIT_ASSERT_MESSAGE("Checking derived parameter units.", testmodel->GetDerivedParameterUnits()[testmodel->GetDerivedParameterNames()[i]] == profile_json_obj["derivedParameterUnits"][i]);
      }
      CPPUNIT_ASSERT_MESSAGE("Checking number of static parameters in model.", testmodel->GetNumberOfStaticParameters() == profile_json_obj["numberOfStaticParameters"]);
      for (unsigned long i = 0; i < profile_json_obj["numberOfStaticParameters"]; i++)
      {
        CPPUNIT_ASSERT_MESSAGE("Checking static parameter names.", testmodel->GetStaticParameterNames()[i] == profile_json_obj["staticParameterNames"][i]);
        CPPUNIT_ASSERT_MESSAGE("Checking static parameter units.", testmodel->GetStaticParameterUnits()[testmodel->GetStaticParameterNames()[i]] == profile_json_obj["staticParameterUnits"][i]);
      }
      CPPUNIT_ASSERT_MESSAGE("Checking function string.", testmodel->GetFunctionString() == profile_json_obj["functionString"]);
      CPPUNIT_ASSERT_MESSAGE("Checking class ID.", testmodel->GetClassID() == profile_json_obj["classID"]);
      CPPUNIT_ASSERT_MESSAGE("Checking model display name.", testmodel->GetModelDisplayName() == profile_json_obj["modelDisplayName"]);
      CPPUNIT_ASSERT_MESSAGE("Checking model type.", testmodel->GetModelType() == profile_json_obj["modelType"]);
      CPPUNIT_ASSERT_MESSAGE("Checking x name.", testmodel->GetXName() == profile_json_obj["xName"]);
      CPPUNIT_ASSERT_MESSAGE("Checking x axis name.", testmodel->GetXAxisName() == profile_json_obj["xAxisName"]);
      CPPUNIT_ASSERT_MESSAGE("Checking x axis unit.", testmodel->GetXAxisUnit() == profile_json_obj["xAxisUnit"]);
      CPPUNIT_ASSERT_MESSAGE("Checking y axis name.", testmodel->GetYAxisName() == profile_json_obj["yAxisName"]);
      CPPUNIT_ASSERT_MESSAGE("Checking y axis unit.", testmodel->GetYAxisUnit() == profile_json_obj["yAxisUnit"]);
    }

    static void CompareModelAndReferenceSignal(mitk::ModelBase::Pointer testmodel, const json modelValues_json_obj, const json profile_json_obj)
    {

      for (unsigned int j = 0; j < modelValues_json_obj["modelValues"].size(); j++)
      {
        json modelValues_json_obj_current = modelValues_json_obj["modelValues"][j];

        SetStaticParametersForTest(testmodel, profile_json_obj, modelValues_json_obj_current);

        // Set time grid
        mitk::ModelBase::TimeGridType timeGrid;
        timeGrid.SetSize(modelValues_json_obj_current["timeGrid"].size());
        for (unsigned long i = 0; i < modelValues_json_obj_current["timeGrid"].size(); ++i)
        {
          timeGrid[i] = modelValues_json_obj_current["timeGrid"][i];
        }
        testmodel->SetTimeGrid(timeGrid);

        // Parse test parameters
        mitk::ModelBase::ParametersType testparameters;
        testparameters = ParseTestParameters(modelValues_json_obj_current);

        //Generate signal
        mitk::ModelBase::ModelResultType signal;
        signal = testmodel->GetSignal(testparameters);


        //ComputeModelfunction is called within GetSignal(), therefore no explicit testing of ComputeModelFunction()
        // The reference signal has been calculated in a matlab script.
        std::stringstream ss;
        ss << "Checking signal for model parameter set " << j << ".";
        std::string message = ss.str();
        for (unsigned long i = 0; i < signal.size(); i++)
        {
          CPPUNIT_ASSERT_MESSAGE(message, mitk::Equal(signal[i], modelValues_json_obj_current["signal"][i], 1e-6, true) == true);
        }
      }
    }

    static void CompareModelAndReferenceDerivedParameters(const mitk::ModelBase::Pointer testmodel, json modelValues_json_obj)
    {
      for (unsigned int j = 0; j < modelValues_json_obj["modelValues"].size(); j++)
      {
        json modelValues_json_obj_current = modelValues_json_obj["modelValues"][j];

        // Set time grid
        mitk::ModelBase::TimeGridType timeGrid;
        timeGrid.SetSize(modelValues_json_obj_current["timeGrid"].size());
        for (unsigned long i = 0; i < modelValues_json_obj_current["timeGrid"].size(); ++i)
        {
          timeGrid[i] = modelValues_json_obj_current["timeGrid"][i];
        }
        testmodel->SetTimeGrid(timeGrid);
        mitk::ModelBase::ParametersType testparameters;
        testparameters = mitk::mitkModelTestFixture::ParseTestParameters(modelValues_json_obj_current);
        mitk::ModelBase::DerivedParameterMapType derivedParameterMap;
        derivedParameterMap = testmodel->GetDerivedParameters(testparameters);
        std::stringstream ss;
        ss << "Checking derived parameters for model parameter set " << j << ".";
        std::string message = ss.str();
        for (unsigned long i = 0; i < modelValues_json_obj_current["derivedParameterValues"].size(); i++)
        {
          CPPUNIT_ASSERT_MESSAGE(message, mitk::Equal(derivedParameterMap[testmodel->GetDerivedParameterNames()[i]], modelValues_json_obj_current["derivedParameterValues"][i], 1e-6, true) == true);
        }
      }
    }
  };
}
#endif // MITKMODELTESTFIXTURE_H
