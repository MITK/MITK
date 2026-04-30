/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIsoDoseLevelSetProperty_h
#define mitkIsoDoseLevelSetProperty_h

#include <mitkBaseProperty.h>
#include <mitkIsoDoseLevelCollections.h>
#include <MitkRTExports.h>

namespace mitk {

/**
 * \brief Property class that wraps an IsoDoseLevelSet for storage in a PropertyList.
 *
 * This property allows attaching a complete set of iso dose level definitions
 * to a DataNode via the MITK property system. It is typically stored under the
 * key defined by RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.
 *
 * \sa IsoDoseLevelSet
 * \sa IsoDoseLevelVectorProperty
 * \sa mitk::BaseProperty
 * \ingroup MitkRTModule
 */
class MITKRT_EXPORT IsoDoseLevelSetProperty : public BaseProperty
{

protected:
    /** \brief The wrapped iso dose level set. */
    IsoDoseLevelSet::Pointer m_IsoLevelSet;

    /** \brief Default constructor. Creates an empty property. */
    IsoDoseLevelSetProperty();

    /** \brief Copy constructor. */
    explicit IsoDoseLevelSetProperty(const IsoDoseLevelSetProperty& other);

    /**
     * \brief Constructor initializing the property with a given level set.
     * \param[in] levelSet The iso dose level set to wrap. May be nullptr.
     */
    explicit IsoDoseLevelSetProperty(IsoDoseLevelSet* levelSet);

    mitkCloneMacro(IsoDoseLevelSetProperty);

public:
    mitkClassMacro(IsoDoseLevelSetProperty, BaseProperty);

    itkNewMacro(IsoDoseLevelSetProperty);
    mitkNewMacro1Param(IsoDoseLevelSetProperty, IsoDoseLevelSet*);

    /** \brief The value type stored by this property. */
    typedef IsoDoseLevelSet ValueType;

    /** \brief Destructor. */
    ~IsoDoseLevelSetProperty() override;

    /**
     * \brief Get the stored iso dose level set (const version).
     * \return Const pointer to the IsoDoseLevelSet, or nullptr if none is set.
     */
    const IsoDoseLevelSet * GetIsoDoseLevelSet() const;

    /**
     * \brief Get the stored value (const version). Equivalent to GetIsoDoseLevelSet().
     * \return Const pointer to the IsoDoseLevelSet, or nullptr if none is set.
     */
    const IsoDoseLevelSet * GetValue() const;

    /**
     * \brief Get the stored iso dose level set (mutable version).
     * \return Pointer to the IsoDoseLevelSet, or nullptr if none is set.
     */
    IsoDoseLevelSet * GetIsoDoseLevelSet();

    /**
     * \brief Get the stored value (mutable version). Equivalent to GetIsoDoseLevelSet().
     * \return Pointer to the IsoDoseLevelSet, or nullptr if none is set.
     */
    IsoDoseLevelSet * GetValue();

    /**
     * \brief Set the iso dose level set wrapped by this property.
     * \param[in] levelSet The new iso dose level set. Triggers Modified() if changed.
     */
    void SetIsoDoseLevelSet(IsoDoseLevelSet* levelSet);

    /**
     * \brief Set the value. Equivalent to SetIsoDoseLevelSet().
     * \param[in] levelSet The new iso dose level set.
     */
    void SetValue(IsoDoseLevelSet* levelSet);

    /**
     * \brief Returns a human-readable string representation of the iso dose levels.
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
