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
#include <mitkImageStatisticsContainer.h>

#include <algorithm>

namespace mitk
{
    ImageStatisticsContainer::ImageStatisticsContainer()
    {
      this->Reset();
    }

    //The order is derived from the old (<2018) image statistics plugin.
    const ImageStatisticsContainer::StatisticsObject::StatisticNameVector ImageStatisticsContainer::StatisticsObject::m_DefaultNames = {
      ImageStatisticsConstants::MEAN(),
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
      ImageStatisticsConstants::UPP() };

    ImageStatisticsContainer::StatisticsObject::StatisticsObject() {
      Reset();
    }

    void ImageStatisticsContainer::StatisticsObject::AddStatistic(const std::string& key, StatisticsVariantType value)
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

    const ImageStatisticsContainer::StatisticsObject::StatisticNameVector& ImageStatisticsContainer::StatisticsObject::GetDefaultStatisticNames()
    {
      return m_DefaultNames;
    }

    const ImageStatisticsContainer::StatisticsObject::StatisticNameVector& ImageStatisticsContainer::StatisticsObject::GetCustomStatisticNames() const
    {
      return m_CustomNames;
    }

    ImageStatisticsContainer::StatisticsObject::StatisticNameVector ImageStatisticsContainer::StatisticsObject::GetAllStatisticNames() const
    {
      StatisticNameVector names = GetDefaultStatisticNames();
      names.insert(names.cend(), m_CustomNames.cbegin(), m_CustomNames.cend());

      return names;
    }

    bool ImageStatisticsContainer::StatisticsObject::HasStatistic(const std::string& name) const
    {
      return m_Statistics.find(name) != m_Statistics.cend();
    }

    ImageStatisticsContainer::StatisticsVariantType ImageStatisticsContainer::StatisticsObject::GetValueNonConverted(const std::string& name) const
    {
      if (HasStatistic(name)) {
        return m_Statistics.find(name)->second;
      }
      else {
        mitkThrow() << "invalid statistic key, could not find";
      }
    }

    void ImageStatisticsContainer::StatisticsObject::Reset()
    {
      m_Statistics.clear();
      m_CustomNames.clear();
    }

    bool ImageStatisticsContainer::TimeStepExists(TimeStepType timeStep) const {
      return m_TimeStepMap.find(timeStep) != m_TimeStepMap.cend();
    }

    const ImageStatisticsContainer::StatisticsObject& ImageStatisticsContainer::GetStatisticsForTimeStep(TimeStepType timeStep) const {
      auto it = m_TimeStepMap.find(timeStep);
      if (it != m_TimeStepMap.cend()) {
        return it->second;
      }
      mitkThrow() << "StatisticsObject for timeStep " << timeStep << " not found!";
    }

    void ImageStatisticsContainer::SetStatisticsForTimeStep(TimeStepType timeStep, StatisticsObject statistics) {
      if (timeStep < this->GetTimeSteps()) {
        m_TimeStepMap.emplace(timeStep, statistics);
        this->Modified();
      }
      else {
        mitkThrow() << "Given timeStep " << timeStep << " out of timeStep geometry bounds. TimeSteps in geometry: " << this->GetTimeSteps();
      }
    }

    void ImageStatisticsContainer::PrintSelf(std::ostream &os, itk::Indent indent) const
    {
      Superclass::PrintSelf(os, indent);
      for (unsigned int i = 0; i < this->GetTimeSteps(); i++) {
        auto statisticsValues = GetStatisticsForTimeStep(i);
        os << std::endl << indent << "Statistics instance for timeStep " << i << ":";
        auto statisticKeys = statisticsValues.GetAllStatisticNames();
        os << std::endl << indent << "Number of entries: " << statisticKeys.size();
        for (const auto& aKey : statisticKeys) {
          os << std::endl << indent.GetNextIndent() << aKey << ": " << statisticsValues.GetValueNonConverted(aKey);
        }
      }
    }

    unsigned int ImageStatisticsContainer::GetNumberOfTimeSteps()const {
      return this->GetTimeSteps();
    }

    void ImageStatisticsContainer::Reset()
    {
      for (auto iter = m_TimeStepMap.begin(); iter != m_TimeStepMap.end(); iter++) {
        iter->second.Reset();
      }
    }

    itk::LightObject::Pointer ImageStatisticsContainer::InternalClone() const
    {
      itk::LightObject::Pointer ioPtr = Superclass::InternalClone();
      Self::Pointer rval = dynamic_cast<Self*>(ioPtr.GetPointer());
      if (rval.IsNull())
      {
        itkExceptionMacro(<< "downcast to type "
          << "StatisticsContainer"
          << " failed.");
      }

      rval->SetTimeStepMap(m_TimeStepMap);
      rval->SetTimeGeometry(this->GetTimeGeometry()->Clone());

      return ioPtr;
    }

    void ImageStatisticsContainer::SetTimeStepMap(TimeStepMapType map) {
      m_TimeStepMap = map;
    }

    ImageStatisticsContainer::StatisticsObject::StatisticNameVector
      GetAllStatisticNames(const ImageStatisticsContainer* container)
    {
      ImageStatisticsContainer::StatisticsObject::StatisticNameVector names = ImageStatisticsContainer::StatisticsObject::GetDefaultStatisticNames();

      if (container)
      {
        std::set<std::string> customKeys;

        for (unsigned int i = 0; i < container->GetTimeSteps(); i++) {
          auto statisticKeys = container->GetStatisticsForTimeStep(i).GetCustomStatisticNames();
          customKeys.insert(statisticKeys.cbegin(), statisticKeys.cend());
        }

        names.insert(names.cend(), customKeys.cbegin(), customKeys.cend());
      }

      return names;
    }

    ImageStatisticsContainer::StatisticsObject::StatisticNameVector
      GetAllStatisticNames(std::vector<ImageStatisticsContainer::ConstPointer> containers)
    {
      ImageStatisticsContainer::StatisticsObject::StatisticNameVector names = ImageStatisticsContainer::StatisticsObject::GetDefaultStatisticNames();

      std::set<std::string> customKeys;

      for (auto container : containers)
      {
        for (unsigned int i = 0; i < container->GetTimeSteps(); i++) {
          auto statisticKeys = container->GetStatisticsForTimeStep(i).GetCustomStatisticNames();
          customKeys.insert(statisticKeys.cbegin(), statisticKeys.cend());
        }
      }

      names.insert(names.cend(), customKeys.cbegin(), customKeys.cend());

      return names;
    }

}
