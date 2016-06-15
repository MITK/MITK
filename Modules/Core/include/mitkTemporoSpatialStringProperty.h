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


#ifndef MITKTEMPOROSPATIALSTRINGPROPERTY_H_HEADER
#define MITKTEMPOROSPATIALSTRINGPROPERTY_H_HEADER

#include <itkConfigure.h>

#include <MitkCoreExports.h>
#include "mitkBaseProperty.h"

#include "mitkTimeGeometry.h"

#include <string>

namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
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

      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)
      mitkNewMacro1Param(TemporoSpatialStringProperty, const char*);
      mitkNewMacro1Param(TemporoSpatialStringProperty, const std::string&);

      /**Returns the value of the first time point in the first slice.
       * If now value is set it returns an empty string.*/
      ValueType GetValue() const;
      /**Returns the value of the passed time step and slice. If it does not exist and allowedClosed is true
       * it will look for the closest value. If nothing could be found an empty string will be returned.*/
      ValueType GetValue(const TimeStepType& timeStep, const IndexValueType& zSlice, bool allowCloseTime = false, bool allowCloseSlice = false) const;
      ValueType GetValueBySlice(const IndexValueType& zSlice, bool allowClose = false) const;
      ValueType GetValueByTimeStep(const TimeStepType& timeStep, bool allowClose = false) const;

      bool HasValue() const;
      bool HasValue(const TimeStepType& timeStep, const IndexValueType& zSlice, bool allowCloseTime = false, bool allowCloseSlice = false) const;
      bool HasValueBySlice(const IndexValueType& zSlice, bool allowClose = false) const;
      bool HasValueByTimeStep(const TimeStepType& timeStep, bool allowClose = false) const;

      std::vector<IndexValueType> GetAvailableSlices(const TimeStepType& timeStep) const;
      std::vector<TimeStepType> GetAvailableTimeSteps() const;

      void SetValue(const TimeStepType& timeStep, const IndexValueType& zSlice, const ValueType& value);

      void SetValue(const ValueType& value);

      virtual std::string GetValueAsString() const override;

      using BaseProperty::operator=;

  protected:
    typedef std::map<IndexValueType, std::string> SliceMapType;
    typedef std::map<TimeStepType, SliceMapType> TimeMapType;

    TimeMapType m_Values;

    TemporoSpatialStringProperty(const char* string = nullptr);
    TemporoSpatialStringProperty(const std::string&  s);

    TemporoSpatialStringProperty(const TemporoSpatialStringProperty&);

    std::pair<bool, ValueType> CheckValue(const TimeStepType& timeStep, const IndexValueType& zSlice, bool allowCloseTime = false, bool allowCloseSlice = false) const;

    private:
      // purposely not implemented
      TemporoSpatialStringProperty& operator=(const TemporoSpatialStringProperty&);

      itk::LightObject::Pointer InternalClone() const override;

      virtual bool IsEqual(const BaseProperty& property ) const override;
      virtual bool Assign(const BaseProperty& property ) override;
  };

  namespace PropertyPersistenceSerialization
  {
    /** Serialization of a TemporoSpatialStringProperty into a JSON string.*/
    MITKCORE_EXPORT::std::string serializeTemporoSpatialStringPropertyToJSON(const mitk::BaseProperty* prop);
  }

  namespace PropertyPersistenceDeserialization
  {
    /**Deserialize a passed JSON string into a TemporoSpatialStringProperty.*/
    MITKCORE_EXPORT mitk::BaseProperty::Pointer deserializeJSONToTemporoSpatialStringProperty(const std::string& value);
  }

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk

#endif

