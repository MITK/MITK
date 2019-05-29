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

#ifndef MITKIMAGESTATISTICSCONTAINER
#define MITKIMAGESTATISTICSCONTAINER

#include <MitkImageStatisticsExports.h>
#include <mitkBaseData.h>
#include <itkHistogram.h>
#include <mitkImageStatisticsConstants.h>

#include <boost/variant.hpp>

namespace mitk
{
  /**
   @brief Container class for storing a StatisticsObject for each timestep.

   Stored statistics are:
   - for the defined statistics, see GetAllStatisticNames
   - Histogram of Pixel Values
  */
  class MITKIMAGESTATISTICS_EXPORT ImageStatisticsContainer : public mitk::BaseData
  {
  public:
    mitkClassMacro(ImageStatisticsContainer, mitk::BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    using HistogramType = itk::Statistics::Histogram<double>;
    using RealType = double;
    using LabelIndex = unsigned int;
    using IndexType = vnl_vector<int>;
    using VoxelCountType = unsigned long;
    using StatisticsVariantType = boost::variant<RealType, VoxelCountType, IndexType >;
    using StatisticsMapType = std::map < std::string, StatisticsVariantType>;
    using StatisticsKeyType = std::string;

    virtual void SetRequestedRegionToLargestPossibleRegion() override {}

    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return false; }

    virtual bool VerifyRequestedRegion() override { return true; }

    virtual void SetRequestedRegion(const itk::DataObject*) override {}

    /**
    @brief Container class for storing the computed image statistics.
    @details The statistics are stored in a map <name,value> with value as boost::variant<RealType, VoxelCountType, IndexType >.
    The type used to create the boost::variant is important as only this type can be recovered lateron.
    */
    class MITKIMAGESTATISTICS_EXPORT ImageStatisticsObject {
    public:
      ImageStatisticsObject();
      /**
      @brief Adds a statistic to the statistics object
      @details if already a statistic with that name is included, it is overwritten
      */
      void AddStatistic(const std::string& key, StatisticsVariantType value);

      using StatisticNameVector = std::vector<std::string>;

      /**
      @brief Returns the names of the default statistics
      @details The order is derived from the image statistics plugin.
      */
      static const StatisticNameVector& GetDefaultStatisticNames();

      /**
      @brief Returns the names of all custom statistics (defined at runtime and no default names).
      */
      const StatisticNameVector& GetCustomStatisticNames() const;

      /**
      @brief Returns the names of all statistics (default and custom defined)
      Additional custom keys are added at the end in a sorted order.
      */
      StatisticNameVector GetAllStatisticNames() const;

      StatisticNameVector GetExistingStatisticNames() const;

      bool HasStatistic(const std::string& name) const;

      /**
      @brief Converts the requested value to the defined type
      @param name defined string on creation (AddStatistic)
      @exception if no statistics with key name was found.
      */
      template <typename TType>
      TType GetValueConverted(const std::string& name) const
      {
        auto value = GetValueNonConverted(name);
        return boost::get<TType>(value);
      }

      /**
      @brief Returns the requested value
      @exception if no statistics with key name was found.
      */
      StatisticsVariantType GetValueNonConverted(const std::string& name) const;

      void Reset();

      HistogramType::ConstPointer m_Histogram=nullptr;
    private:
      StatisticsMapType m_Statistics;
      StatisticNameVector m_CustomNames;
      static const StatisticNameVector m_DefaultNames;
    };

    using TimeStepMapType = std::map<TimeStepType, ImageStatisticsObject>;

    unsigned int GetNumberOfTimeSteps() const;

    /**
    @brief Deletes all stored values*/
    void Reset();

    /**
    @brief Returns the statisticObject for the given Timestep
    @pre timeStep must be valid
    */
    const ImageStatisticsObject& GetStatisticsForTimeStep(TimeStepType timeStep) const;

    /**
    @brief Sets the statisticObject for the given Timestep
    @pre timeStep must be valid
    */
    void SetStatisticsForTimeStep(TimeStepType timeStep, ImageStatisticsObject statistics);

    /**
    @brief Checks if the Time step exists
    @pre timeStep must be valid
    */
    bool TimeStepExists(TimeStepType timeStep) const;

  protected:
    ImageStatisticsContainer();
    virtual void PrintSelf(std::ostream &os, itk::Indent indent) const override;

  private:
    itk::LightObject::Pointer InternalClone() const override;

    void SetTimeStepMap(TimeStepMapType map);

    TimeStepMapType m_TimeStepMap;
  };

  MITKIMAGESTATISTICS_EXPORT ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector GetAllStatisticNames(const ImageStatisticsContainer* container);
  MITKIMAGESTATISTICS_EXPORT ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector GetAllStatisticNames(std::vector<ImageStatisticsContainer::ConstPointer> containers);

}
#endif // MITKIMAGESTATISTICSCONTAINER
