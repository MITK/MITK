/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkImageStatisticsContainer.h>

#include <algorithm>

namespace mitk
{
  ImageStatisticsContainer::ImageStatisticsContainer() { this->Reset(); }

  // The order is derived from the old (<2018) image statistics plugin.
  const ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector
    ImageStatisticsContainer::ImageStatisticsObject::m_DefaultNames = {ImageStatisticsConstants::MEAN(),
                                                                  ImageStatisticsConstants::MEDIAN(),
                                                                  ImageStatisticsConstants::STANDARDDEVIATION(),
                                                                  ImageStatisticsConstants::RMS(),
                                                                  ImageStatisticsConstants::MAXIMUM(),
                                                                  ImageStatisticsConstants::MAXIMUMPOSITION(),
                                                                  ImageStatisticsConstants::MINIMUM(),
                                                                  ImageStatisticsConstants::MINIMUMPOSITION(),
                                                                  ImageStatisticsConstants::NUMBEROFVOXELS(),
                                                                  ImageStatisticsConstants::VOLUME(),
                                                                  ImageStatisticsConstants::SKEWNESS(),
                                                                  ImageStatisticsConstants::KURTOSIS(),
                                                                  ImageStatisticsConstants::UNIFORMITY(),
                                                                  ImageStatisticsConstants::ENTROPY(),
                                                                  ImageStatisticsConstants::MPP(),
                                                                  ImageStatisticsConstants::UPP()};

  ImageStatisticsContainer::ImageStatisticsObject::ImageStatisticsObject() { Reset(); }

  void ImageStatisticsContainer::ImageStatisticsObject::AddStatistic(const std::string_view key, StatisticsVariantType value)
  {
    m_Statistics.emplace(key, value);

    if (std::find(m_DefaultNames.cbegin(), m_DefaultNames.cend(), key) == m_DefaultNames.cend())
    {
      if (std::find(m_CustomNames.cbegin(), m_CustomNames.cend(), key) == m_CustomNames.cend())
      {
        m_CustomNames.emplace_back(key);
      }
    }
  }

  const ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector &
    ImageStatisticsContainer::ImageStatisticsObject::GetDefaultStatisticNames()
  {
    return m_DefaultNames;
  }

  const ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector &
    ImageStatisticsContainer::ImageStatisticsObject::GetCustomStatisticNames() const
  {
    return m_CustomNames;
  }

  ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector
    ImageStatisticsContainer::ImageStatisticsObject::GetAllStatisticNames() const
  {
    StatisticNameVector names = GetDefaultStatisticNames();

      names.insert(names.cend(), m_CustomNames.cbegin(), m_CustomNames.cend());

    return names;
  }

    ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector
    ImageStatisticsContainer::ImageStatisticsObject::GetExistingStatisticNames() const
  {
    StatisticNameVector names;

      std::transform(m_Statistics.begin(), m_Statistics.end(), std::back_inserter(names), [](const auto &pair) {
        return pair.first;
      });

    return names;
  }

  bool ImageStatisticsContainer::ImageStatisticsObject::HasStatistic(const std::string_view name) const
  {
    return m_Statistics.find(name) != m_Statistics.cend();
  }

  ImageStatisticsContainer::StatisticsVariantType ImageStatisticsContainer::ImageStatisticsObject::GetValueNonConverted(
    const std::string_view name) const
  {
    if (HasStatistic(name))
    {
      return m_Statistics.find(name)->second;
    }
    else
    {
      mitkThrow() << "invalid statistic key, could not find";
    }
  }

  void ImageStatisticsContainer::ImageStatisticsObject::Reset()
  {
    m_Statistics.clear();
    m_CustomNames.clear();
  }

  const static ImageStatisticsContainer::LabelValueType NO_MASK_LABEL_VALUE = 1;

  bool ImageStatisticsContainer::StatisticsExist(LabelValueType labelValue, TimeStepType timeStep) const
  {
    auto labelFinding = m_LabelTimeStep2StatisticsMap.find(labelValue);
    if (labelFinding == m_LabelTimeStep2StatisticsMap.end()) return false;

    auto timeFinding = labelFinding->second.find(timeStep);

    return timeFinding != labelFinding->second.end();
  }

  const ImageStatisticsContainer::HistogramType*
    ImageStatisticsContainer::GetHistogram(LabelValueType labelValue, TimeStepType timeStep) const
  {
    return this->GetStatistics(labelValue, timeStep).m_Histogram;
  }

  const ImageStatisticsContainer::ImageStatisticsObject &ImageStatisticsContainer::GetStatistics(LabelValueType labelValue,
    TimeStepType timeStep) const
  {
    auto labelFinding = m_LabelTimeStep2StatisticsMap.find(labelValue);
    if (labelFinding == m_LabelTimeStep2StatisticsMap.end()) mitkThrow() << "Cannot get statistics. Requested label value does not exist. Invalid label:" <<labelValue;

    auto timeFinding = labelFinding->second.find(timeStep);
    if (timeFinding == labelFinding->second.end()) mitkThrow() << "Cannot get statistics. Requested time step does not exist. Invalid time step:" << timeStep;

    return timeFinding->second;
  }

  void ImageStatisticsContainer::SetStatistics(LabelValueType labelValue, TimeStepType timeStep, const ImageStatisticsObject& statistics)
  {
    
    if (!this->GetTimeGeometry()->IsValidTimeStep(timeStep)) mitkThrow() << "Given timeStep " << timeStep
      << " out of TimeGeometry bounds of the object. TimeSteps in geometry: " << this->GetTimeSteps();

    m_LabelTimeStep2StatisticsMap[labelValue][timeStep] = statistics;
    this->Modified();
  }

  void ImageStatisticsContainer::PrintSelf(std::ostream &os, itk::Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
    for (const auto& [labelValue, timeMap] : m_LabelTimeStep2StatisticsMap)
    {
      for (const auto& [timeStep, container] : timeMap)
      {
        os << std::endl << indent << "Statistics instance (Label "<< labelValue << ", TimeStep "<< timeStep << "):";
        auto statisticsValues = GetStatistics(labelValue,timeStep);

        auto statisticKeys = statisticsValues.GetExistingStatisticNames();
        os << std::endl << indent << "Number of entries: " << statisticKeys.size();
        for (const auto& aKey : statisticKeys)
        {
          os << std::endl << indent.GetNextIndent() << aKey << ": " << statisticsValues.GetValueNonConverted(aKey);
        }
      }
    }
  }


  ImageStatisticsContainer::LabelValueVectorType ImageStatisticsContainer::GetExistingLabelValues(bool ignoreUnlabeled) const
  {
    LabelValueVectorType result;
    for (const auto& [labelValue, timeMap] : m_LabelTimeStep2StatisticsMap)
    {
      if (!ignoreUnlabeled || labelValue != NO_MASK_LABEL_VALUE)
      {
        result.push_back(labelValue);
      }
    }
    return result;
  }

  ImageStatisticsContainer::TimeStepVectorType ImageStatisticsContainer::GetExistingTimeSteps(LabelValueType labelValue) const
  {
    TimeStepVectorType result;

    auto labelFinding = m_LabelTimeStep2StatisticsMap.find(labelValue);
    if (labelFinding == m_LabelTimeStep2StatisticsMap.end()) mitkThrow() << "Cannot get existing time steps. Requested label value does not exist. Invalid label:" << labelValue;

    for (const auto& [timestep, stats] : labelFinding->second)
    {
      result.push_back(timestep);
    }
    return result;
  }

  void ImageStatisticsContainer::Reset()
  {
    m_LabelTimeStep2StatisticsMap.clear();
    this->Modified();
  }

  itk::LightObject::Pointer ImageStatisticsContainer::InternalClone() const
  {
    itk::LightObject::Pointer ioPtr = Superclass::InternalClone();
    Self::Pointer rval = dynamic_cast<Self *>(ioPtr.GetPointer());
    if (rval.IsNull())
    {
      itkExceptionMacro(<< "downcast to type "
                        << "StatisticsContainer"
                        << " failed.");
    }

    rval->m_LabelTimeStep2StatisticsMap = m_LabelTimeStep2StatisticsMap;
    rval->SetTimeGeometry(this->GetTimeGeometry()->Clone());

    return ioPtr;
  }

  ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector GetAllStatisticNames(
    const ImageStatisticsContainer *container)
  {
    ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector names =
      ImageStatisticsContainer::ImageStatisticsObject::GetDefaultStatisticNames();

    if (container)
    {
      std::set<std::string> customKeys;

      auto labelValues = container->GetExistingLabelValues(false);

      for (const auto labelValue : labelValues)
      {
        auto timeSteps = container->GetExistingTimeSteps(labelValue);

        for (const auto timeStep : timeSteps)
        {
          auto statisticKeys = container->GetStatistics(labelValue, timeStep).GetCustomStatisticNames();
          customKeys.insert(statisticKeys.cbegin(), statisticKeys.cend());
        }
      }

      names.insert(names.cend(), customKeys.cbegin(), customKeys.cend());
    }

    return names;
  }

  ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector GetAllStatisticNames(
    std::vector<ImageStatisticsContainer::ConstPointer> containers)
  {
    ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector names =
      ImageStatisticsContainer::ImageStatisticsObject::GetDefaultStatisticNames();

    std::set<std::string> customKeys;

    for (const auto &container : containers)
    {
      std::set<std::string> customKeys;

      auto labelValues = container->GetExistingLabelValues(false);

      for (const auto labelValue : labelValues)
      {
        auto timeSteps = container->GetExistingTimeSteps(labelValue);

        for (const auto timeStep : timeSteps)
        {
          auto statisticKeys = container->GetStatistics(labelValue, timeStep).GetCustomStatisticNames();
          customKeys.insert(statisticKeys.cbegin(), statisticKeys.cend());
        }
      }

      names.insert(names.cend(), customKeys.cbegin(), customKeys.cend());
    }

    names.insert(names.end(), customKeys.begin(), customKeys.end());

    return names;
  };

} // namespace mitk
