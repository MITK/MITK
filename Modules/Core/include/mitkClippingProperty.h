/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkClippingProperty_h
#define mitkClippingProperty_h

#include <mitkBaseProperty.h>
#include <mitkNumericTypes.h>
#include <MitkCoreExports.h>

#include <itkConfigure.h>

#include <string>

namespace mitk
{
  /**
   * \brief Property for defining a clipping plane for datasets.
   *
   * Stores a clipping plane defined by an origin point and a normal vector,
   * along with an enabled/disabled flag. When enabled, the clipping plane
   * can be used by renderers to clip the associated data object.
   *
   * When constructed with an origin and normal, clipping is automatically
   * enabled. The default constructor creates a disabled clipping property
   * with origin and normal at (0, 0, 0).
   *
   * \sa BaseProperty
   */
  class MITKCORE_EXPORT ClippingProperty : public BaseProperty
  {
  public:
    mitkClassMacro(ClippingProperty, BaseProperty);

    /** \brief The type used for string-based value representation. */
    typedef std::string ValueType;

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self)
      mitkNewMacro2Param(ClippingProperty, const Point3D &, const Vector3D &);

    /**
     * \brief Check whether clipping is enabled.
     * \return \c true if clipping is enabled, \c false otherwise.
     */
    bool GetClippingEnabled() const;

    /**
     * \brief Enable or disable clipping.
     *
     * Marks the property as modified if the state changes.
     *
     * \param[in] enabled \c true to enable clipping, \c false to disable.
     */
    void SetClippingEnabled(bool enabled);

    /**
     * \brief Get the origin point of the clipping plane.
     * \return A const reference to the origin point.
     */
    const Point3D &GetOrigin() const;

    /**
     * \brief Set the origin point of the clipping plane.
     *
     * Marks the property as modified if the new origin differs from the current one.
     *
     * \param[in] origin The new origin point.
     */
    void SetOrigin(const Point3D &origin);

    /**
     * \brief Get the normal vector of the clipping plane.
     * \return A const reference to the normal vector.
     */
    const Vector3D &GetNormal() const;

    /**
     * \brief Set the normal vector of the clipping plane.
     *
     * Marks the property as modified if the new normal differs from the current one.
     *
     * \param[in] normal The new normal vector.
     */
    void SetNormal(const Vector3D &normal);

    /**
     * \brief Return the clipping state and plane parameters as a string.
     * \return A string showing the enabled state, origin, and normal.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Serialize the clipping property to JSON.
     *
     * Serializes "Enabled", "Origin", and "Normal" fields.
     *
     * \param[out] j The JSON object to write the value into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize the clipping property from JSON.
     *
     * Reads "Enabled", "Origin", and "Normal" fields.
     *
     * \param[in] j The JSON object containing clipping data.
     * \return Always \c true.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;

  protected:
    bool m_ClippingEnabled;

    Point3D m_Origin;
    Vector3D m_Normal;

    ClippingProperty();
    ClippingProperty(const ClippingProperty &other);
    ClippingProperty(const Point3D &origin, const Vector3D &normal);

    mitkCloneMacro(ClippingProperty);

  private:
    // purposely not implemented
    ClippingProperty &operator=(const ClippingProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };


} // namespace mitk

#endif
