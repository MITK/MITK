/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIsoDoseLevelVectorProperty_h
#define mitkIsoDoseLevelVectorProperty_h

#include <mitkBaseProperty.h>
#include <mitkIsoDoseLevelCollections.h>
#include <MitkRTExports.h>

namespace mitk {

/**
 * \brief Property class that wraps an IsoDoseLevelVector for storage in a PropertyList.
 *
 * This property allows attaching an ordered vector of iso dose levels (e.g. user-defined
 * free iso values) to a DataNode via the MITK property system. It is typically stored
 * under the key defined by RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.
 *
 * \sa IsoDoseLevelVector
 * \sa IsoDoseLevelSetProperty
 * \sa mitk::BaseProperty
 */
class MITKRT_EXPORT IsoDoseLevelVectorProperty : public BaseProperty
{

protected:
    /** \brief The wrapped iso dose level vector. */
    IsoDoseLevelVector::Pointer m_IsoLevelVector;

    /** \brief Default constructor. Creates an empty property. */
    IsoDoseLevelVectorProperty();

    /** \brief Copy constructor. */
    explicit IsoDoseLevelVectorProperty(const IsoDoseLevelVectorProperty& other);

    /**
     * \brief Constructor initializing the property with a given level vector.
     * \param[in] levelVector The iso dose level vector to wrap. May be nullptr.
     */
    explicit IsoDoseLevelVectorProperty(IsoDoseLevelVector* levelVector);

    mitkCloneMacro(IsoDoseLevelVectorProperty);

public:
    mitkClassMacro(IsoDoseLevelVectorProperty, BaseProperty);

    itkNewMacro(IsoDoseLevelVectorProperty);
    mitkNewMacro1Param(IsoDoseLevelVectorProperty, IsoDoseLevelVector*);

    /** \brief The value type stored by this property. */
    typedef IsoDoseLevelVector ValueType;

    /** \brief Destructor. */
    ~IsoDoseLevelVectorProperty() override;

    /**
     * \brief Get the stored iso dose level vector (const version).
     * \return Const pointer to the IsoDoseLevelVector, or nullptr if none is set.
     */
    const IsoDoseLevelVector * GetIsoDoseLevelVector() const;

    /**
     * \brief Get the stored value (const version). Equivalent to GetIsoDoseLevelVector().
     * \return Const pointer to the IsoDoseLevelVector, or nullptr if none is set.
     */
    const IsoDoseLevelVector * GetValue() const;

    /**
     * \brief Get the stored iso dose level vector (mutable version).
     * \return Pointer to the IsoDoseLevelVector, or nullptr if none is set.
     */
    IsoDoseLevelVector * GetIsoDoseLevelVector();

    /**
     * \brief Get the stored value (mutable version). Equivalent to GetIsoDoseLevelVector().
     * \return Pointer to the IsoDoseLevelVector, or nullptr if none is set.
     */
    IsoDoseLevelVector * GetValue();

    /**
     * \brief Set the iso dose level vector wrapped by this property.
     * \param[in] levelVector The new iso dose level vector. Triggers Modified() if changed.
     */
    void SetIsoDoseLevelVector(IsoDoseLevelVector* levelVector);

    /**
     * \brief Set the value. Equivalent to SetIsoDoseLevelVector().
     * \param[in] levelVector The new iso dose level vector.
     */
    void SetValue(IsoDoseLevelVector* levelVector);

    /**
     * \brief Returns a human-readable string representation of the iso dose level vector.
     * \return A string listing the number of levels and their dose values, colors, and visibility.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Serialize property to JSON. Currently not implemented.
     * \param[out] j The JSON object to write to.
     * \return Always returns false.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize property from JSON. Currently not implemented.
     * \param[in] j The JSON object to read from.
     * \return Always returns false.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;

private:
    bool IsEqual(const BaseProperty& property) const override;
    bool Assign(const BaseProperty& property) override;

};

} // namespace mitk



#endif
