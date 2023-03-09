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

#include "mitkBaseProperty.h"
#include <MitkCoreExports.h>

#include "mitkTimeGeometry.h"

#include <string>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * @brief Property for time and space resolved string values
   * @ingroup DataManagement
   */
  class MITKCORE_EXPORT TemporoSpatialStringProperty : public BaseProperty
  {
  public:
    typedef ::itk::IndexValueType IndexValueType;
    typedef std::string ValueType;

    mitkClassMacro(TemporoSpatialStringProperty, BaseProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);
    mitkNewMacro1Param(TemporoSpatialStringProperty, const char*);
    mitkNewMacro1Param(TemporoSpatialStringProperty, const std::string &);

    /**Returns the value of the first time point in the first slice.
     * If now value is set it returns an empty string.*/
    ValueType GetValue() const;
    /**Returns the value of the passed time step and slice. If it does not exist and allowedClosed is true
     * it will look for the closest value. If nothing could be found an empty string will be returned.*/
    ValueType GetValue(const TimeStepType &timeStep,
                       const IndexValueType &zSlice,
                       bool allowCloseTime = false,
                       bool allowCloseSlice = false) const;
    ValueType GetValueBySlice(const IndexValueType &zSlice, bool allowClose = false) const;
    ValueType GetValueByTimeStep(const TimeStepType &timeStep, bool allowClose = false) const;

    bool HasValue() const;
    bool HasValue(const TimeStepType &timeStep,
                  const IndexValueType &zSlice,
                  bool allowCloseTime = false,
                  bool allowCloseSlice = false) const;
    bool HasValueBySlice(const IndexValueType &zSlice, bool allowClose = false) const;
    bool HasValueByTimeStep(const TimeStepType &timeStep, bool allowClose = false) const;

    /** return all slices stored for the specified timestep.*/
    std::vector<IndexValueType> GetAvailableSlices(const TimeStepType& timeStep) const;
    /** return all time steps stored for the specified slice.*/
    std::vector<TimeStepType> GetAvailableTimeSteps(const IndexValueType& slice) const;
    /** return all time steps stored in the property.*/
    std::vector<TimeStepType> GetAvailableTimeSteps() const;
    /** return all slices stored in the property. @remark not all time steps may contain all slices.*/
    std::vector<IndexValueType> GetAvailableSlices() const;

    void SetValue(const TimeStepType &timeStep, const IndexValueType &zSlice, const ValueType &value);

    void SetValue(const ValueType &value);

    std::string GetValueAsString() const override;

    /** Indicates of all values (all time steps, all slices) are the same, or if at least one value stored
    in the property is different. If IsUniform==true one can i.a. use GetValueAsString() without the loss of
    information to retrieve the stored value.*/
    bool IsUniform() const;

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

  private:
    // purposely not implemented
    TemporoSpatialStringProperty &operator=(const TemporoSpatialStringProperty &);

    itk::LightObject::Pointer InternalClone() const override;

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };

  namespace PropertyPersistenceSerialization
  {
    /** Serialization of a TemporoSpatialStringProperty into a JSON string.*/
    MITKCORE_EXPORT::std::string serializeTemporoSpatialStringPropertyToJSON(const mitk::BaseProperty *prop);
  }

  namespace PropertyPersistenceDeserialization
  {
    /**Deserialize a passed JSON string into a TemporoSpatialStringProperty.*/
    MITKCORE_EXPORT mitk::BaseProperty::Pointer deserializeJSONToTemporoSpatialStringProperty(const std::string &value);
  }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace mitk

#endif
