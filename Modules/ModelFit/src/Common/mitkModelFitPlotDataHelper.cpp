/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkModelFitPlotDataHelper.h"

#include "mitkExceptionMacro.h"
#include "mitkImage.h"
#include "mitkModelFitParameterValueExtraction.h"
#include "mitkModelGenerator.h"

#include "mitkFormulaParser.h"

const std::string mitk::MODEL_FIT_PLOT_SAMPLE_NAME()
{
  return "Sample";
};

const std::string mitk::MODEL_FIT_PLOT_SIGNAL_NAME()
{
  return "Signal";
}

const std::string mitk::MODEL_FIT_PLOT_INTERPOLATED_SIGNAL_NAME()
{
  return "INTERP_Signal";
};

void
mitk::PlotDataCurve::
SetValues(const PlotDataValues& _arg)
{
  if (this->m_Values != _arg)
  {
    this->m_Values = _arg;
    this->Modified();
  }
}

void
mitk::PlotDataCurve::
SetValues(PlotDataValues&& _arg)
{
  if (this->m_Values != _arg)
  {
    this->m_Values = std::move(_arg);
    this->Modified();
  }
}

mitk::PlotDataCurve & mitk::PlotDataCurve::operator=(const PlotDataCurve& rhs)
{
  this->m_Values = rhs.m_Values;
  this->SetTimeStamp(rhs.GetTimeStamp());
  return *this;
};

mitk::PlotDataCurve& mitk::PlotDataCurve::operator=(PlotDataCurve&& rhs) noexcept
{
  this->m_Values = std::move(rhs.m_Values);
  this->SetTimeStamp(rhs.GetTimeStamp());

  return *this;
};

void mitk::PlotDataCurve::Reset()
{
  this->m_Values.clear();
  this->Modified();
}

mitk::PlotDataCurve::PlotDataCurve()
{
}

const mitk::PlotDataCurve* GetPlotCurve(const mitk::PlotDataCurveCollection* collection, const std::string& key)
{
  if (collection)
  {
    auto iter = collection->find(key);
    if (iter != collection->end())
    {
      return iter->second.GetPointer();
    }
  }
  return nullptr;
};

const mitk::PlotDataCurve* mitk::ModelFitPlotData::GetSamplePlot(const PlotDataCurveCollection* coll)
{
  if (coll)
  {
    return GetPlotCurve(coll, MODEL_FIT_PLOT_SAMPLE_NAME());
  }
  return nullptr;
};

const mitk::PlotDataCurve* mitk::ModelFitPlotData::GetSignalPlot(const PlotDataCurveCollection* coll)
{
  if (coll)
  {
    return GetPlotCurve(coll, MODEL_FIT_PLOT_SIGNAL_NAME());
  }
  return nullptr;
};

const mitk::PlotDataCurve* mitk::ModelFitPlotData::GetInterpolatedSignalPlot(const PlotDataCurveCollection* coll)
{
  if (coll)
  {
    return GetPlotCurve(coll, MODEL_FIT_PLOT_INTERPOLATED_SIGNAL_NAME());
  }
  return nullptr;
};

std::string mitk::ModelFitPlotData::GetPositionalCollectionName(const PositionalCollectionMap::value_type& mapValue)
{
  std::ostringstream nameStrm;
  nameStrm.imbue(std::locale("C"));
  nameStrm << "Pos " << mapValue.first << std::endl << std::setprecision(3) << "(" << mapValue.second.first[0] << "|" << mapValue.second.first[1] << "|" << mapValue.second.first[2] << ")";
  return nameStrm.str();
};


const mitk::PlotDataCurveCollection* mitk::ModelFitPlotData::GetPositionalPlot(const mitk::Point3D& point) const
{
  auto predicate = [point](const PositionalCollectionMap::value_type& value) {return value.second.first == point; };

  auto iter = std::find_if(std::begin(this->positionalPlots), std::end(this->positionalPlots), predicate);
  if (iter != positionalPlots.end())
  {
    return iter->second.second.GetPointer();
  }
  return nullptr;
};

const mitk::PlotDataCurveCollection* mitk::ModelFitPlotData::GetPositionalPlot(mitk::PointSet::PointIdentifier id) const
{
  auto iter = this->positionalPlots.find(id);
  if (iter != positionalPlots.end())
  {
    return iter->second.second.GetPointer();
  }
  return nullptr;
};


mitk::PlotDataValues::value_type mitk::ModelFitPlotData::GetXMinMax() const
{
  double max = itk::NumericTraits<double>::NonpositiveMin();
  double min = itk::NumericTraits<double>::max();

  //currently we assume that within a model fit, plot data does not exceed
  //the sample/signale on the x axis.
  auto sample = this->GetSamplePlot(this->currentPositionPlots);
  if (sample)
  {
    CheckXMinMaxFromPlotDataValues(sample->GetValues(), min, max);
  }
  for (const auto& posCollection : this->positionalPlots)
  {
    auto sample = this->GetSamplePlot(posCollection.second.second);
    if (sample)
    {
      CheckXMinMaxFromPlotDataValues(sample->GetValues(), min, max);
    }
  }

  return std::make_pair(min, max);
};

mitk::PlotDataValues::value_type mitk::ModelFitPlotData::GetYMinMax() const
{
  double max = itk::NumericTraits<double>::NonpositiveMin();
  double min = itk::NumericTraits<double>::max();

  for (const auto& plot : *(this->currentPositionPlots.GetPointer()))
  {
    CheckYMinMaxFromPlotDataValues(plot.second->GetValues(), min, max);
  }

  for (const auto& posCollection : this->positionalPlots)
  {
    for (const auto& plot : *(posCollection.second.second))
    {
      CheckYMinMaxFromPlotDataValues(plot.second->GetValues(), min, max);
    }
  }

  for (const auto& plot : *(this->staticPlots))
  {
    CheckYMinMaxFromPlotDataValues(plot.second->GetValues(), min, max);
  }

  return std::make_pair(min, max);
};

mitk::ModelFitPlotData::ModelFitPlotData()
{
  this->currentPositionPlots = PlotDataCurveCollection::New();
  this->staticPlots = PlotDataCurveCollection::New();
};

void mitk::CheckYMinMaxFromPlotDataValues(const PlotDataValues& data, double& min, double& max)
{
  for (const auto & pos : data)
  {
    if (max < pos.second)
    {
      max = pos.second;
    }

    if (min > pos.second)
    {
      min = pos.second;
    }
  }
}

void mitk::CheckXMinMaxFromPlotDataValues(const PlotDataValues& data, double& min, double& max)
{
  for (const auto & pos : data)
  {
    if (max < pos.first)
    {
      max = pos.first;
    }

    if (min > pos.first)
    {
      min = pos.first;
    }
  }
}

/** Helper function that generates the curve based on a stored and on the fly parsed function string.*/
mitk::PlotDataCurve::Pointer
CalcSignalFromFunction(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelBase::TimeGridType& timeGrid)
{
  if (!fitInfo)
  {
    mitkThrow() << "Cannot calc model curve from function for given fit. Passed ModelFitInfo instance is nullptr.";
  }

  mitk::Image::Pointer inputImage = fitInfo->inputImage;
  assert(inputImage.IsNotNull());

  mitk::PlotDataCurve::Pointer result = mitk::PlotDataCurve::New();
  mitk::PlotDataCurve::ValuesType values;
  values.reserve(timeGrid.size());

  // Calculate index
  ::itk::Index<3> index;
  mitk::BaseGeometry::Pointer geometry = inputImage->GetTimeGeometry()->GetGeometryForTimeStep(0);

  geometry->WorldToIndex(position, index);

  mitk::ParameterValueMapType parameterMap = mitk::ExtractParameterValueMapFromModelFit(fitInfo, position);

  mitk::FormulaParser parser(&parameterMap);

  for (unsigned int t = 0; t < timeGrid.size(); ++t)
  {
    // Set up static parameters
    for (const auto& var : fitInfo->staticParamMap)
    {
      const auto& list = var.second;

      if (list.size() == 1)
      {
        parameterMap[var.first] = list.front();
      }
      else
      {
        parameterMap[var.first] = list.at(t);
      }
    }

    // Calculate curve data
    double x = timeGrid[t];
    parameterMap[fitInfo->x] = x;

    double y = parser.parse(fitInfo->function);
    values.emplace_back(std::make_pair(x, y));
  }

  result->SetValues(std::move(values));
  return result;
}

/** Helper function that generates the curve based on the model specified by the fit info.*/
mitk::PlotDataCurve::Pointer
CalcSignalFromModel(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelParameterizerBase* parameterizer = nullptr)
{
  assert(fitInfo);

  if (!parameterizer)
  {
    parameterizer = mitk::ModelGenerator::GenerateModelParameterizer(*fitInfo);
  }

  mitk::Image::Pointer inputImage = fitInfo->inputImage;
  assert(inputImage.IsNotNull());

  // Calculate index
  ::itk::Index<3> index;
  mitk::BaseGeometry::Pointer geometry = inputImage->GetTimeGeometry()->GetGeometryForTimeStep(0);

  geometry->WorldToIndex(position, index);

  //model generation
  mitk::ModelBase::Pointer model = parameterizer->GenerateParameterizedModel(index);

  mitk::ParameterValueMapType parameterMap = mitk::ExtractParameterValueMapFromModelFit(fitInfo, position);

  mitk::ModelBase::ParametersType paramArray = mitk::ConvertParameterMapToParameterVector(parameterMap, model);

  mitk::ModelBase::ModelResultType curveDataY = model->GetSignal(paramArray);
  mitk::PlotDataCurve::Pointer result = mitk::PlotDataCurve::New();

  mitk::ModelBase::TimeGridType timeGrid = model->GetTimeGrid();
  mitk::PlotDataCurve::ValuesType values;
  values.reserve(timeGrid.size());

  for (unsigned int t = 0; t < timeGrid.size(); ++t)
  {
    double x = timeGrid[t];
    double y = curveDataY[t];
    values.emplace_back(std::make_pair(x, y));
  }

  result->SetValues(std::move(values));
  return result;
}

mitk::PlotDataCurve::Pointer
mitk::GenerateModelSignalPlotData(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelBase::TimeGridType& timeGrid, mitk::ModelParameterizerBase* parameterizer)
{
  if (!fitInfo)
  {
    mitkThrow() << "Cannot calc model curve from function for given fit. Passed ModelFitInfo instance is nullptr.";
  }

  mitk::PlotDataCurve::Pointer result;

  if (!parameterizer)
  {
    parameterizer = ModelGenerator::GenerateModelParameterizer(*fitInfo);
  }

  if (parameterizer)
  {
    // Use model instead of formula parser
    parameterizer->SetDefaultTimeGrid(timeGrid);
    result = CalcSignalFromModel(position, fitInfo, parameterizer);
  }
  else
  {
    // Use formula parser to parse function string
    try
    {
      result = CalcSignalFromFunction(position, fitInfo, timeGrid);
    }
    catch (const mitk::FormulaParserException& e)
    {
      MITK_ERROR << "Error while parsing modelfit function: " << e.what();
    }
  }

  return result;
}

mitk::PlotDataCurveCollection::Pointer
mitk::GenerateAdditionalModelFitPlotData(const mitk::Point3D& /*position*/, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelBase::TimeGridType& timeGrid)
{
  if (!fitInfo)
  {
    mitkThrow() << "Cannot calc model curve from function for given fit. Passed ModelFitInfo instance is nullptr.";
  }

  mitk::PlotDataCurveCollection::Pointer result = mitk::PlotDataCurveCollection::New();

  for (const auto& additionalInput : fitInfo->inputData.GetLookupTable())
  {
    if (additionalInput.second.size() != timeGrid.size())
    {
      MITK_ERROR <<
        "Error while refreshing input data for visualization. Size of data and input image time grid differ. Invalid data name: "
        << additionalInput.first;
    }
    else
    {
      mitk::PlotDataCurve::Pointer pointData = mitk::PlotDataCurve::New();;
      mitk::PlotDataCurve::ValuesType values;
      values.reserve(timeGrid.size());

      for (unsigned int t = 0; t < timeGrid.size(); ++t)
      {
        const double x = timeGrid[t];
        const double y = additionalInput.second[t];
        values.emplace_back(std::make_pair(x, y));
      }
      pointData->SetValues(std::move(values));
      result->CastToSTLContainer().emplace(additionalInput.first, std::move(pointData));
    }
  }

  return result;
}

mitk::PlotDataCurve::Pointer
mitk::GenerateImageSamplePlotData(const mitk::Point3D& position, const mitk::Image* image, const mitk::ModelBase::TimeGridType& timeGrid)
{
  if (!image)
  {
    mitkThrow() << "Cannot generate sample plot data. Passed image instance is nullptr.";
  }

  mitk::PlotDataCurve::Pointer result = mitk::PlotDataCurve::New();
  mitk::PlotDataCurve::ValuesType values;
  values.reserve(timeGrid.size());

  for (unsigned int t = 0; t < timeGrid.size(); ++t)
  {
    const double x = timeGrid[t];
    const double y = ReadVoxel(image, position, t);
    values.emplace_back(std::make_pair(x, y));
  }

  result->SetValues(std::move(values));
  return result;
}
