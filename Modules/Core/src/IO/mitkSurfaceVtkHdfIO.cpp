/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceVtkHdfIO.h"
#include "mitkSurfaceIOHelper.h"
#include "mitkVtkFileIOProgressObserver.h"

#include <mitkIOMimeTypes.h>
#include <mitkSurface.h>

#include <vtkCallbackCommand.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkErrorCode.h>
#include <vtkFieldData.h>
#include <vtkHDFReader.h>
#include <vtkHDFWriter.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <itksys/SystemTools.hxx>

#include <algorithm>
#include <functional>
#include <map>

namespace
{
  const char *const TimeBoundsArrayName = "MITKTimeBounds";

  /**
   * \brief Hands one poly data per time step to the pipeline.
   *
   * vtkHDFWriter pulls the time steps of its input through the pipeline, one
   * request per time value the input announces. Without time values, the
   * single poly data is not tied to any time.
   */
  class TimeStepSource : public vtkPolyDataAlgorithm
  {
  public:
    static TimeStepSource *New();
    vtkTypeMacro(TimeStepSource, vtkPolyDataAlgorithm);

    void SetTimeSteps(const std::vector<vtkSmartPointer<vtkPolyData>> &polyDatas, const std::vector<double> &timeValues)
    {
      m_PolyDatas = polyDatas;
      m_TimeValues = timeValues;
      this->Modified();
    }

    /**
     * \brief Called with the share of the time steps written so far.
     *
     * vtkHDFWriter writes all time steps in one execution of the pipeline and
     * reports no progress in between, but it requests the time steps in order.
     */
    void SetProgressCallback(std::function<void(float)> callback)
    {
      m_ProgressCallback = std::move(callback);
    }

  protected:
    TimeStepSource()
    {
      this->SetNumberOfInputPorts(0);
    }

    int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *outputVector) override
    {
      if (m_TimeValues.empty())
        return 1;

      auto *outInfo = outputVector->GetInformationObject(0);

      outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), m_TimeValues.data(), static_cast<int>(m_TimeValues.size()));

      const double timeRange[] = { m_TimeValues.front(), m_TimeValues.back() };
      outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);

      return 1;
    }

    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *outputVector) override
    {
      auto *outInfo = outputVector->GetInformationObject(0);
      std::size_t timeStep = 0;

      if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
      {
        const auto timeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
        const auto next = std::upper_bound(m_TimeValues.begin(), m_TimeValues.end(), timeValue);

        if (next != m_TimeValues.begin())
          timeStep = static_cast<std::size_t>(std::distance(m_TimeValues.begin(), next) - 1);
      }

      if (m_ProgressCallback)
        m_ProgressCallback(static_cast<float>(timeStep) / static_cast<float>(m_PolyDatas.size()));

      vtkPolyData::GetData(outInfo)->ShallowCopy(m_PolyDatas[timeStep]);
      return 1;
    }

  private:
    std::vector<vtkSmartPointer<vtkPolyData>> m_PolyDatas;
    std::vector<double> m_TimeValues;
    std::function<void(float)> m_ProgressCallback;
  };

  vtkStandardNewMacro(TimeStepSource);

  /**
   * \brief Give every time step the same arrays of the given attribute type.
   *
   * vtkHDFWriter tracks the offsets of an array only across the time steps
   * that have it, so a time step without an array corrupts that array in all
   * later time steps. Time steps without elements, like those of labels that
   * are absent at some time steps, get empty arrays.
   */
  void UnifyArrays(const std::vector<vtkSmartPointer<vtkPolyData>> &polyDatas, int attributeType)
  {
    struct Reference
    {
      vtkAbstractArray *Array;
      vtkFieldData *Owner;
    };

    std::map<std::string, Reference> references;

    for (const auto &polyData : polyDatas)
    {
      auto *arrays = polyData->GetAttributesAsFieldData(attributeType);

      for (int i = 0; i < arrays->GetNumberOfArrays(); ++i)
      {
        auto *array = arrays->GetAbstractArray(i);

        if (nullptr == array->GetName())
          continue;

        const auto [reference, isNew] = references.emplace(array->GetName(), Reference{ array, arrays });

        if (!isNew && (reference->second.Array->GetDataType() != array->GetDataType() ||
                       reference->second.Array->GetNumberOfComponents() != array->GetNumberOfComponents()))
        {
          mitkThrow() << "The array \"" << array->GetName() << "\" differs in type between time steps.";
        }
      }
    }

    for (std::size_t timeStep = 0; timeStep < polyDatas.size(); ++timeStep)
    {
      auto *polyData = polyDatas[timeStep].GetPointer();
      auto *arrays = polyData->GetAttributesAsFieldData(attributeType);

      for (const auto &[name, reference] : references)
      {
        if (arrays->HasArray(name.c_str()))
          continue;

        if (attributeType != vtkDataObject::FIELD && polyData->GetNumberOfElements(attributeType) != 0)
        {
          mitkThrow() << "Time step " << timeStep << " lacks the array \"" << name
                      << "\" of other time steps, but VTKHDF needs the same arrays in every time step.";
        }

        auto emptyArray = vtkSmartPointer<vtkAbstractArray>::Take(reference.Array->NewInstance());
        emptyArray->SetName(name.c_str());
        emptyArray->SetNumberOfComponents(reference.Array->GetNumberOfComponents());
        arrays->AddArray(emptyArray);

        auto *attributes = vtkDataSetAttributes::SafeDownCast(arrays);
        auto *referenceAttributes = vtkDataSetAttributes::SafeDownCast(reference.Owner);

        if (nullptr == attributes || nullptr == referenceAttributes)
          continue;

        for (int attribute = 0; attribute < vtkDataSetAttributes::NUM_ATTRIBUTES; ++attribute)
        {
          if (referenceAttributes->GetAbstractAttribute(attribute) == reference.Array)
            attributes->SetActiveAttribute(name.c_str(), attribute);
        }
      }
    }
  }

  std::vector<mitk::TimeBounds> GetTimeBoundsFromTimeValues(const std::vector<double> &timeValues)
  {
    std::vector<mitk::TimeBounds> timeBounds(timeValues.size());

    for (std::size_t timeStep = 0; timeStep < timeValues.size(); ++timeStep)
    {
      timeBounds[timeStep][0] = timeValues[timeStep];
      timeBounds[timeStep][1] = timeStep + 1 < timeValues.size()
        ? timeValues[timeStep + 1]
        : 2.0 * timeValues[timeStep] - timeValues[timeStep - 1];
    }

    return timeBounds;
  }
}

mitk::SurfaceVtkHdfIO::SurfaceVtkHdfIO()
  : AbstractFileIO(Surface::GetStaticNameOfClass(), IOMimeTypes::VTKHDF_MIMETYPE(), "VTKHDF PolyData")
{
  this->RegisterService();
}

mitk::IFileIO::ConfidenceLevel mitk::SurfaceVtkHdfIO::GetReaderConfidenceLevel() const
{
  if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
    return Unsupported;

  // Not vtkHDFReader::CanReadFile(), which reads the whole file into memory.
  auto reader = vtkSmartPointer<vtkHDFReader>::New();
  reader->SetFileName(this->GetLocalFileName().c_str());
  reader->UpdateDataObject();

  return vtkPolyData::SafeDownCast(reader->GetOutputDataObject(0)) != nullptr
    ? Supported
    : Unsupported;
}

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::SurfaceVtkHdfIO::DoRead()
{
  auto reader = vtkSmartPointer<vtkHDFReader>::New();
  reader->SetFileName(this->GetLocalFileName().c_str());

  if (!reader->UpdateInformation())
    mitkThrow() << "Could not read the VTKHDF file \"" << this->GetInputLocation() << "\".";

  // The reader reports no time values for a single time step, which leaves
  // the stored time bounds, if any, as its only time.
  const auto isTemporal = reader->GetOutputInformation(0)->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  const auto timeSteps = static_cast<std::size_t>(std::max<vtkIdType>(1, reader->GetNumberOfSteps()));

  std::vector<vtkSmartPointer<vtkPolyData>> polyDatas;
  std::vector<double> timeValues;
  std::vector<TimeBounds> storedTimeBounds;
  std::size_t timeStep = 0;

  VtkFileIOProgressObserver progress(reader, [this, &timeStep, timeSteps](float p)
    {
      this->AbstractFileReader::ReportProgress((timeStep + p) / timeSteps);
    });

  for (; timeStep < timeSteps; ++timeStep)
  {
    reader->SetStep(static_cast<vtkIdType>(timeStep));

    // On a failure, the output holds whatever was read before, partly of
    // earlier time steps.
    auto *output = reader->Update()
      ? vtkPolyData::SafeDownCast(reader->GetOutputDataObject(0))
      : nullptr;

    if (nullptr == output)
    {
      mitkThrow() << "Could not read time step " << timeStep << " of the VTKHDF file \"" << this->GetInputLocation()
                  << "\".";
    }

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->DeepCopy(output);

    timeValues.push_back(reader->GetTimeValue());

    auto *fieldData = polyData->GetFieldData();

    if (auto *timeBoundsArray = fieldData->GetArray(TimeBoundsArrayName);
        timeBoundsArray != nullptr && timeBoundsArray->GetNumberOfComponents() == 2 && timeBoundsArray->GetNumberOfTuples() == 1)
    {
      TimeBounds timeBounds;
      timeBounds[0] = timeBoundsArray->GetComponent(0, 0);
      timeBounds[1] = timeBoundsArray->GetComponent(0, 1);
      storedTimeBounds.push_back(timeBounds);
    }

    fieldData->RemoveArray(TimeBoundsArrayName);
    polyDatas.push_back(polyData);
  }

  std::vector<TimeBounds> timeBounds;

  if (isTemporal)
  {
    // Another application may have changed the time values without knowing
    // about the stored time bounds. The time values are only stored in single
    // precision.
    bool useStoredTimeBounds = storedTimeBounds.size() == timeSteps && SurfaceIOHelper::AreValidTimeBounds(storedTimeBounds);

    for (std::size_t i = 0; i < storedTimeBounds.size() && useStoredTimeBounds; ++i)
      useStoredTimeBounds = static_cast<float>(storedTimeBounds[i][0]) == static_cast<float>(timeValues[i]);

    if (useStoredTimeBounds)
    {
      timeBounds = storedTimeBounds;
    }
    else
    {
      timeBounds = GetTimeBoundsFromTimeValues(timeValues);

      if (!SurfaceIOHelper::AreValidTimeBounds(timeBounds))
      {
        MITK_WARN << "The time values of the VTKHDF file \"" << this->GetInputLocation()
                  << "\" do not increase. The time steps are numbered instead.";

        timeBounds.clear();
      }
    }
  }
  else if (storedTimeBounds.size() == timeSteps && SurfaceIOHelper::AreValidTimeBounds(storedTimeBounds))
  {
    timeBounds = storedTimeBounds;
  }

  std::vector<itk::SmartPointer<BaseData>> result;
  result.push_back(SurfaceIOHelper::CreateSurface(polyDatas, timeBounds).GetPointer());
  return result;
}

void mitk::SurfaceVtkHdfIO::Write()
{
  this->ValidateOutputLocation();

  const auto *surface = dynamic_cast<const Surface *>(this->GetInput());
  const auto *timeGeometry = surface->GetTimeGeometry();
  const auto timeSteps = timeGeometry->CountTimeSteps();

  // Like the poly data of other applications, a surface that is valid at
  // every time point is written without time steps.
  const auto isTemporal = !timeGeometry->IsValidAtEveryTimePoint();

  // The time values of VTKHDF have to increase. Time steps of images may
  // start at the same time point, for example those of DICOM series acquired
  // faster than the resolution of their acquisition times.
  bool startsIncrease = true;

  for (TimeStepType timeStep = 1; timeStep < timeSteps && startsIncrease; ++timeStep)
    startsIncrease = timeGeometry->GetTimeBounds(timeStep - 1)[0] < timeGeometry->GetTimeBounds(timeStep)[0];

  if (isTemporal && !startsIncrease)
  {
    MITK_WARN << "The time steps of the surface do not start at increasing time points, so they are written to \""
              << this->GetOutputLocation() << "\" as numbered time steps.";
  }

  std::vector<vtkSmartPointer<vtkPolyData>> polyDatas;
  std::vector<double> timeValues;

  for (TimeStepType timeStep = 0; timeStep < timeSteps; ++timeStep)
  {
    auto polyData = SurfaceIOHelper::GetWorldPolyData(surface, timeStep);
    polyDatas.push_back(polyData);

    if (!isTemporal)
      continue;

    if (!startsIncrease)
    {
      timeValues.push_back(static_cast<double>(timeStep));
      continue;
    }

    const auto timeBounds = timeGeometry->GetTimeBounds(timeStep);

    auto timeBoundsArray = vtkSmartPointer<vtkDoubleArray>::New();
    timeBoundsArray->SetName(TimeBoundsArrayName);
    timeBoundsArray->SetNumberOfComponents(2);
    timeBoundsArray->InsertNextTuple2(timeBounds[0], timeBounds[1]);

    polyData->GetFieldData()->AddArray(timeBoundsArray);
    timeValues.push_back(timeBounds[0]);
  }

  for (const auto attributeType : { vtkDataObject::POINT, vtkDataObject::CELL, vtkDataObject::FIELD })
    UnifyArrays(polyDatas, attributeType);

  auto source = vtkSmartPointer<TimeStepSource>::New();
  source->SetTimeSteps(polyDatas, timeValues);
  source->SetProgressCallback([this](float p)
    {
      this->AbstractFileWriter::ReportProgress(p);
    });

  LocalFile localFile(this);

  auto writer = vtkSmartPointer<vtkHDFWriter>::New();
  writer->SetInputConnection(source->GetOutputPort());
  writer->SetFileName(localFile.GetFileName().c_str());
  writer->SetCompressionLevel(4);

  VtkFileIOProgressObserver progress(writer, [this](float p)
    {
      this->AbstractFileWriter::ReportProgress(p);
    });

  // vtkHDFWriter tells why it failed only in its error messages.
  std::string errorMessage;

  auto errorObserver = vtkSmartPointer<vtkCallbackCommand>::New();
  errorObserver->SetClientData(&errorMessage);
  errorObserver->SetCallback([](vtkObject *, unsigned long, void *clientData, void *callData)
    {
      auto *firstMessage = static_cast<std::string *>(clientData);

      if (firstMessage->empty() && nullptr != callData)
        *firstMessage = static_cast<const char *>(callData);
    });

  writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);

  const auto fileName = localFile.GetFileName();
  const bool fileExisted = itksys::SystemTools::FileExists(fileName);

  if (writer->Write() == 0 || writer->GetErrorCode() != 0 || !errorMessage.empty())
  {
    // A file this write created holds at most part of the time steps. An
    // existing file is left alone, as the writer may not have opened it.
    if (!fileExisted)
      itksys::SystemTools::RemoveFile(fileName);

    if (errorMessage.empty() && writer->GetErrorCode() != 0)
      errorMessage = vtkErrorCode::GetStringFromErrorCode(writer->GetErrorCode());

    mitkThrow() << "Error during surface writing" << (errorMessage.empty() ? "" : ": " + errorMessage);
  }
}

mitk::SurfaceVtkHdfIO *mitk::SurfaceVtkHdfIO::IOClone() const
{
  return new SurfaceVtkHdfIO(*this);
}
