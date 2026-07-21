/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageStatisticsContainer_h
#define mitkImageStatisticsContainer_h

#include <MitkImageStatisticsExports.h>
#include <mitkBaseData.h>
#include <itkHistogram.h>
#include <mitkLabelSetImage.h>
#include <mitkImageStatisticsConstants.h>

#include <variant>

namespace mitk
{

  /**
   \brief Container class for storing a StatisticsObject for each time step.

   Stored statistics are:
   - for the defined statistics, see GetAllStatisticNames
   - Histogram of Pixel Values
  */
  class MITKIMAGESTATISTICS_EXPORT ImageStatisticsContainer : public mitk::BaseData
  {
  public:
    mitkClassMacro(ImageStatisticsContainer, mitk::BaseData);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    using HistogramType = itk::Statistics::Histogram<double>;
    using LabelValueType = MultiLabelSegmentation::LabelValueType;

    /** \brief Required by BaseData. Sets requested region to largest possible. No-op. */
    void SetRequestedRegionToLargestPossibleRegion() override {}

    /** \brief Required by BaseData. Always returns false (data is always buffered). */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return false; }

    /** \brief Required by BaseData. Always returns true. */
    bool VerifyRequestedRegion() override { return true; }

    /** \brief Required by BaseData. No-op for this container. */
    void SetRequestedRegion(const itk::DataObject*) override {}

    /**
    \brief Container class for storing the computed image statistics.
    @details The statistics are stored in a map <name,value> with value as std::variant<RealType, VoxelCountType, IndexType >.
    The type used to create the std::variant is important as only this type can be recovered later on.
    */
    class MITKIMAGESTATISTICS_EXPORT ImageStatisticsObject {
    public:
      ImageStatisticsObject();

      using RealType = double;
      using IndexType = vnl_vector<int>;
      using VoxelCountType = unsigned long;

      using StatisticsVariantType = std::variant<RealType, VoxelCountType, IndexType >;

      /**
      \brief Adds a statistic to the statistics object
      @details if already a statistic with that name is included, it is overwritten
      */
      void AddStatistic(const std::string_view key, StatisticsVariantType value);

      using StatisticNameVector = std::vector<std::string>;

      /**
      \brief Returns the names of the default statistics
      @details The order is derived from the image statistics plugin.
      */
      static const StatisticNameVector& GetDefaultStatisticNames();

      /**
      \brief Returns the names of all custom statistics (defined at runtime and no default names).
      */
      const StatisticNameVector& GetCustomStatisticNames() const;

      /**
      \brief Returns the names of all statistics (default and custom defined)
      Additional custom keys are added at the end in a sorted order.
      */
      StatisticNameVector GetAllStatisticNames() const;

      /**
       * \brief Returns the names of statistics that actually have stored values.
       * \return A vector of statistic names for which values exist.
       */
      StatisticNameVector GetExistingStatisticNames() const;

      /**
       * \brief Check whether a statistic with the given name exists.
       * \param[in] name The name of the statistic to query.
       * \return True if a statistic with the given name has been added.
       */
      bool HasStatistic(const std::string_view name) const;

      /**
      \brief Converts the requested value to the defined type
      \param name defined string on creation (AddStatistic)
      @exception if no statistics with key name was found.
      */
      template <typename TType>
      TType GetValueConverted(const std::string_view name) const
      {
        auto value = GetValueNonConverted(name);
        return std::get<TType>(value);
      }

      /**
      \brief Returns the requested value
      @exception if no statistics with key name was found.
      */
      StatisticsVariantType GetValueNonConverted(const std::string_view name) const;

      /** \brief Clear all stored statistics and custom names. */
      void Reset();

      /** \brief The histogram of pixel values for this statistics object. Can be nullptr. */
      HistogramType::ConstPointer m_Histogram=nullptr;
    private:

      using StatisticsMapType = std::map < std::string, StatisticsVariantType, std::less<>>;

      StatisticsMapType m_Statistics;
      StatisticNameVector m_CustomNames;
      static const StatisticNameVector m_DefaultNames;
    };

    using StatisticsVariantType = ImageStatisticsObject::StatisticsVariantType;
    using RealType = ImageStatisticsObject::RealType;
    using IndexType = ImageStatisticsObject::IndexType;
    using VoxelCountType = ImageStatisticsObject::VoxelCountType;

    /** \brief Vector of time step indices. */
    using TimeStepVectorType = std::vector<TimeStepType>;

    /**
     * \brief Get all time steps for which statistics exist for the given label.
     * \param[in] labelValue The label value to query.
     * \return Vector of time step indices that have stored statistics.
     */
    TimeStepVectorType GetExistingTimeSteps(LabelValueType labelValue) const;

    /** \brief Label value used to query statistics when no mask was provided. */
    static constexpr LabelValueType NO_MASK_LABEL_VALUE = Label::UNLABELED_VALUE;
    using LabelValueVectorType = MultiLabelSegmentation::LabelValueVectorType;

    /**
     * \brief Get all label values for which statistics have been stored.
     * \return Vector of label values.
     */
    LabelValueVectorType GetExistingLabelValues() const;

    /**
     * \brief Delete all stored statistics for all labels and time steps.
     */
    void Reset();

    /**
     * \brief Retrieve the statistics object for a given label and time step.
     * \param[in] labelValue The label value to query.
     * \param[in] timeStep The time step to query.
     * \return Const reference to the ImageStatisticsObject.
     * \throw if no statistics exist for the given label and time step.
     */
    const ImageStatisticsObject& GetStatistics(LabelValueType labelValue, TimeStepType timeStep) const;

    /**
    \brief Sets the statisticObject for the given Timestep
    \pre timeStep must be valid
    */
    void SetStatistics(LabelValueType labelValue, TimeStepType timeStep, const ImageStatisticsObject& statistics);

    /**
    \brief Checks if the Time step exists
    \pre timeStep must be valid
    */
    bool StatisticsExist(LabelValueType labelValue, TimeStepType timeStep) const;

    /**
    /brief Returns the histogram of the passed time step.
    \pre timeStep must be valid*/
    const HistogramType* GetHistogram(LabelValueType labelValue, TimeStepType timeStep) const;

    /**
     * \brief Check whether this container was computed with zero-voxel ignoring enabled.
     * \return True if zero-valued voxels were excluded during computation.
     */
    bool IgnoresZeroVoxel() const;

    /**
     * \brief Check whether this container is a work-in-progress placeholder.
     * \return True if the statistics are still being computed.
     */
    bool IsWIP() const;

  protected:
    ImageStatisticsContainer();
    ImageStatisticsContainer(const ImageStatisticsContainer &other);
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    mitkCloneMacro(Self);

  private:

    using TimeStepMapType = std::map<TimeStepType, ImageStatisticsObject>;
    using LabelMapType = std::map<LabelValueType, TimeStepMapType>;

    LabelMapType m_LabelTimeStep2StatisticsMap;
  };

  /**
   * \brief Collect all statistic names (default and custom) from a single container.
   * \param[in] container Pointer to the ImageStatisticsContainer to query.
   * \return Vector of all statistic names present across all labels and time steps.
   */
  MITKIMAGESTATISTICS_EXPORT ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector GetAllStatisticNames(const ImageStatisticsContainer* container);

  /**
   * \brief Collect all statistic names from a vector of containers.
   * \param[in] containers Vector of const pointers to ImageStatisticsContainer objects.
   * \return Union of all statistic names present across all containers.
   */
  MITKIMAGESTATISTICS_EXPORT ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector GetAllStatisticNames(std::vector<ImageStatisticsContainer::ConstPointer> containers);
}
#endif
