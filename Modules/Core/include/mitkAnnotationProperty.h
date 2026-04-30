/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAnnotationProperty_h
#define mitkAnnotationProperty_h

#include <mitkBaseProperty.h>
#include <mitkNumericTypes.h>
#include <MitkCoreExports.h>

#include <itkConfigure.h>

#include <string>

namespace mitk
{
  /**
   * \brief Property for storing a text annotation with a 3D position.
   *
   * Combines a text label string with a 3D point position, allowing annotations
   * to be placed at specific locations in world coordinates.
   *
   * \ingroup DataManagement
   *
   * \sa BaseProperty
   */
  class MITKCORE_EXPORT AnnotationProperty : public BaseProperty
  {
  public:
    mitkClassMacro(AnnotationProperty, BaseProperty);

    /** \brief The type of the label value stored by this property. */
    typedef std::string ValueType;

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self)
      mitkNewMacro2Param(AnnotationProperty, const char *, const Point3D &);
    mitkNewMacro2Param(AnnotationProperty, const std::string &, const Point3D &);
    mitkNewMacro4Param(AnnotationProperty, const char *, ScalarType, ScalarType, ScalarType);
    mitkNewMacro4Param(AnnotationProperty, const std::string &, ScalarType, ScalarType, ScalarType);

    /**
     * \brief Get the annotation label text.
     * \return The label string.
     */
    itkGetStringMacro(Label);

    /**
     * \brief Set the annotation label text.
     * \param[in] _arg The new label string.
     */
    itkSetStringMacro(Label);

    /**
     * \brief Get the 3D position of the annotation.
     * \return A const reference to the position point.
     */
    const Point3D &GetPosition() const;

    /**
     * \brief Set the 3D position of the annotation.
     *
     * Marks the property as modified if the new position differs from the current one.
     *
     * \param[in] position The new 3D position.
     */
    void SetPosition(const Point3D &position);

    /**
     * \brief Return the annotation as a human-readable string.
     *
     * Returns the label text followed by the 3D position coordinates.
     *
     * \return A string representation of the annotation.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Serialize the annotation to JSON.
     *
     * Serializes both the label and position as a JSON object with
     * "Label" and "Position" fields.
     *
     * \param[out] j The JSON object to write the value into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize the annotation from JSON.
     *
     * Reads "Label" and "Position" fields from the JSON object.
     *
     * \param[in] j The JSON object containing annotation data.
     * \return Always \c true.
     */
    bool FromJSON(const nlohmann::json& j) override;

    virtual BaseProperty &operator=(const BaseProperty &other) { return Superclass::operator=(other); }
    using BaseProperty::operator=;

  protected:
    std::string m_Label;
    Point3D m_Position;

    AnnotationProperty();
    AnnotationProperty(const char *label, const Point3D &position);
    AnnotationProperty(const std::string &label, const Point3D &position);
    AnnotationProperty(const char *label, ScalarType x, ScalarType y, ScalarType z);
    AnnotationProperty(const std::string &label, ScalarType x, ScalarType y, ScalarType z);

    AnnotationProperty(const AnnotationProperty &other);

    mitkCloneMacro(AnnotationProperty);

  private:
    // purposely not implemented
    AnnotationProperty &operator=(const AnnotationProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };

} // namespace mitk

#endif
