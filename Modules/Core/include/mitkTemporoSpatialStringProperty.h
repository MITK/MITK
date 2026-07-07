/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTemporoSpatialStringProperty_h
#define mitkTemporoSpatialStringProperty_h

#include <itkConfigure.h>

#include <mitkBaseProperty.h>
#include <MitkCoreExports.h>

#include <mitkTimeGeometry.h>

#include <string>

namespace mitk
{
  /**
   * \brief Property for storing string values resolved by time step and slice index.
   *
   * This property can hold different string values for each combination of time step
   * and z-slice index, making it suitable for DICOM metadata that varies across slices
   * or time points (e.g., per-slice acquisition parameters). When constructed with a
   * single string, the value is stored at time step 0, slice 0.
   *
   * \sa BaseProperty
   * \sa StringProperty
   */
  class MITKCORE_EXPORT TemporoSpatialStringProperty : public BaseProperty
  {
  public:
    /** \brief Index type for slice indices. */
    typedef ::itk::IndexValueType IndexValueType;

    /** \brief The type of the string value stored per time/slice entry. */
    typedef std::string ValueType;

    mitkClassMacro(TemporoSpatialStringProperty, BaseProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);
    mitkNewMacro1Param(TemporoSpatialStringProperty, const char*);
    mitkNewMacro1Param(TemporoSpatialStringProperty, const std::string &);

    /**
     * \brief Get the value at the first time step and first slice.
     *
     * \return The string value, or an empty string if no values exist.
     */
    ValueType GetValue() const;

    /**
     * \brief Get the value for a specific time step and slice.
     *
     * \param[in] timeStep The time step to query.
     * \param[in] zSlice The z-slice index to query.
     * \param[in] allowCloseTime If \c true, the closest earlier time step is used
     *            when the exact time step is not found.
     * \param[in] allowCloseSlice If \c true, the closest earlier slice is used
     *            when the exact slice is not found.
     * \return The string value, or an empty string if nothing was found.
     */
    ValueType GetValue(const TimeStepType &timeStep,
                       const IndexValueType &zSlice,
                       bool allowCloseTime = false,
                       bool allowCloseSlice = false) const;

    /**
     * \brief Get the value for a given slice (time step defaults to 0).
     *
     * \param[in] zSlice The z-slice index to query.
     * \param[in] allowClose If \c true, allows finding the closest slice.
     * \return The string value, or an empty string if not found.
     */
    ValueType GetValueBySlice(const IndexValueType &zSlice, bool allowClose = false) const;

    /**
     * \brief Get the value for a given time step (slice defaults to 0).
     *
     * \param[in] timeStep The time step to query.
     * \param[in] allowClose If \c true, allows finding the closest time step.
     * \return The string value, or an empty string if not found.
     */
    ValueType GetValueByTimeStep(const TimeStepType &timeStep, bool allowClose = false) const;

    /**
     * \brief Check whether any values are stored.
     * \return \c true if at least one value exists, \c false otherwise.
     */
    bool HasValue() const;

    /**
     * \brief Check whether a value exists for the given time step and slice.
     *
     * \param[in] timeStep The time step to check.
     * \param[in] zSlice The z-slice index to check.
     * \param[in] allowCloseTime If \c true, allows finding the closest time step.
     * \param[in] allowCloseSlice If \c true, allows finding the closest slice.
     * \return \c true if a matching value exists, \c false otherwise.
     */
    bool HasValue(const TimeStepType &timeStep,
                  const IndexValueType &zSlice,
                  bool allowCloseTime = false,
                  bool allowCloseSlice = false) const;

    /**
     * \brief Check whether a value exists for the given slice.
     *
     * \param[in] zSlice The z-slice index to check.
     * \param[in] allowClose If \c true, allows finding the closest slice.
     * \return \c true if a matching value exists, \c false otherwise.
     */
    bool HasValueBySlice(const IndexValueType &zSlice, bool allowClose = false) const;

    /**
     * \brief Check whether a value exists for the given time step.
     *
     * \param[in] timeStep The time step to check.
     * \param[in] allowClose If \c true, allows finding the closest time step.
     * \return \c true if a matching value exists, \c false otherwise.
     */
    bool HasValueByTimeStep(const TimeStepType &timeStep, bool allowClose = false) const;

    /**
     * \brief Get all slice indices stored for the specified time step.
     *
     * \param[in] timeStep The time step to query.
     * \return A vector of slice indices. Empty if the time step does not exist.
     */
    std::vector<IndexValueType> GetAvailableSlices(const TimeStepType& timeStep) const;

    /**
     * \brief Get all time steps that contain a value for the specified slice.
     *
     * \param[in] slice The z-slice index to query.
     * \return A vector of time steps.
     */
    std::vector<TimeStepType> GetAvailableTimeSteps(const IndexValueType& slice) const;

    /**
     * \brief Get all time steps stored in the property.
     * \return A vector of all time steps.
     */
    std::vector<TimeStepType> GetAvailableTimeSteps() const;

    /**
     * \brief Get all unique slice indices across all time steps.
     *
     * \note Not all time steps may contain all slices.
     *
     * \return A sorted vector of unique slice indices.
     */
    std::vector<IndexValueType> GetAvailableSlices() const;

    /**
     * \brief Set a value for a specific time step and slice.
     *
     * If the time step or slice does not exist yet, it is created.
     *
     * \param[in] timeStep The time step to set.
     * \param[in] zSlice The z-slice index to set.
     * \param[in] value The string value to store.
     */
    void SetValue(const TimeStepType &timeStep, const IndexValueType &zSlice, const ValueType &value);

    /**
     * \brief Set a uniform value (stored at time step 0, slice 0).
     *
     * Clears all existing values and stores the given value at
     * time step 0, slice 0.
     *
     * \param[in] value The string value to store.
     */
    void SetValue(const ValueType &value);

    /**
     * \brief Return the value at the first time step and slice as a string.
     * \return The string value (same as GetValue()).
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Check whether all stored values are identical.
     *
     * If \c true, GetValueAsString() returns the complete information
     * without loss.
     *
     * \return \c true if all values across all time steps and slices are equal,
     *         \c false if at least one value differs.
     */
    bool IsUniform() const;

    /**
     * \brief Serialize the property to JSON.
     *
     * Serializes the values as a condensed JSON representation, grouping
     * consecutive time steps and slices with identical values.
     *
     * \param[out] j The JSON object to write the value into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize the property from JSON.
     *
     * Reads the condensed JSON representation and expands time step/slice
     * ranges into individual value entries.
     *
     * \param[in] j The JSON object containing the serialized data.
     * \return Always \c true.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;

  protected:
    typedef std::map<IndexValueType, std::string> SliceMapType;
    typedef std::map<TimeStepType, SliceMapType> TimeMapType;

    TimeMapType m_Values;

    TemporoSpatialStringProperty(const char *string = nullptr);
    TemporoSpatialStringProperty(const std::string &s);

    TemporoSpatialStringProperty(const TemporoSpatialStringProperty &);

    std::pair<bool, ValueType> CheckValue(const TimeStepType &timeStep,
                                          const IndexValueType &zSlice,
                                          bool allowCloseTime = false,
                                          bool allowCloseSlice = false) const;

    mitkCloneMacro(TemporoSpatialStringProperty);

  private:
    // purposely not implemented
    TemporoSpatialStringProperty &operator=(const TemporoSpatialStringProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };

  namespace PropertyPersistenceSerialization
  {
    /** Serialization of a TemporoSpatialStringProperty into a JSON string.*/
    MITKCORE_EXPORT std::string serializeTemporoSpatialStringPropertyToJSON(const mitk::BaseProperty *prop);
  }

  namespace PropertyPersistenceDeserialization
  {
    /**Deserialize a passed JSON string into a TemporoSpatialStringProperty.*/
    MITKCORE_EXPORT mitk::BaseProperty::Pointer deserializeJSONToTemporoSpatialStringProperty(const std::string &value);
  }

  /** Helper function that extracts the information of a time step out of a TemporoSpatialStringProperty
  * and returns a TemporoSpatialStringProperty that only contains that time step.
  * @param tsProperty The source property from which the values should be extracted.
  * @param ts The time point that should be extracted.
  * @pre tsProperty must point to a valid instance.
  * @pre ts must indicate a time step that exists in tsProperty.
  * @result Returns a TemporoSpatialStringProperty instance that only contains the values of the indicated time step. In the result the time step is always time step 0.*/
  TemporoSpatialStringProperty::Pointer MITKCORE_EXPORT ExtractTimeStepFromTemporoSpatialStringProperty(const TemporoSpatialStringProperty* tsProperty, TimeStepType ts);


} // namespace mitk

#endif
