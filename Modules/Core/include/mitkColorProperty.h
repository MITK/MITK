/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkColorProperty_h
#define mitkColorProperty_h

#include <mitkBaseProperty.h>
#include <MitkCoreExports.h>

#include <itkRGBPixel.h>

#include <nlohmann/json.hpp>

namespace mitk
{
  /**
   * \brief Standard RGB color typedef using float components.
   *
   * Each color component (red, green, blue) ranges from 0.0f to 1.0f.
   *
   * \ingroup Property
   */
  typedef itk::RGBPixel<float> Color;

  /**
   * \brief Convenience function to create a Color from individual RGB components.
   *
   * \param[in] r Red component (0.0f to 1.0f).
   * \param[in] g Green component (0.0f to 1.0f).
   * \param[in] b Blue component (0.0f to 1.0f).
   * \return A Color initialized with the given RGB values.
   */
  inline Color MakeColor(float r, float g, float b)
  {
    Color color;
    color.Set(r, g, b);
    return color;
  }

  /**
   * \brief Property for storing RGB color values.
   *
   * Stores an RGB color with float precision (each component 0.0f to 1.0f).
   * Used to associate a rendering color with a DataNode.
   *
   * \ingroup DataManagement
   *
   * \note If you want to apply the mitk::ColorProperty to an mitk::Image,
   * make sure to set the mitk::RenderingModeProperty to a mode which
   * supports color (e.g. LOOKUPTABLE_LEVELWINDOW_COLOR). For an example, see
   * mitkImageVtkMapper2DColorTest.cpp in Core/Code/Rendering.
   *
   * \sa BaseProperty
   * \sa RenderingModeProperty
   */
  class MITKCORE_EXPORT ColorProperty : public BaseProperty
  {
  protected:
    mitk::Color m_Color;

    ColorProperty();

    ColorProperty(const ColorProperty &other);

    ColorProperty(const float red, const float green, const float blue);

    ColorProperty(const float color[3]);

    ColorProperty(const mitk::Color &color);

    mitkCloneMacro(ColorProperty);

  public:
    mitkClassMacro(ColorProperty, BaseProperty);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self) mitkNewMacro1Param(ColorProperty, const float *);
    mitkNewMacro1Param(ColorProperty, const mitk::Color &);
    mitkNewMacro3Param(ColorProperty, const float, const float, const float);

    /** \brief The type of the value stored by this property. */
    typedef mitk::Color ValueType;

    /**
     * \brief Get the color value.
     * \return A const reference to the stored color.
     */
    const mitk::Color &GetColor() const;

    /**
     * \brief Get the color value (alias for GetColor()).
     * \return A const reference to the stored color.
     */
    const mitk::Color &GetValue() const;

    /**
     * \brief Return the color as a human-readable string.
     * \return A string representation of the RGB values.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Set the color value.
     *
     * Marks the property as modified if the new color differs from the current one.
     *
     * \param[in] color The new color to set.
     */
    void SetColor(const mitk::Color &color);

    /**
     * \brief Set the color value (alias for SetColor()).
     * \param[in] color The new color to set.
     */
    void SetValue(const mitk::Color &color);

    /**
     * \brief Set the color from individual RGB components.
     *
     * \param[in] red Red component (0.0f to 1.0f).
     * \param[in] green Green component (0.0f to 1.0f).
     * \param[in] blue Blue component (0.0f to 1.0f).
     */
    void SetColor(float red, float green, float blue);

    /**
     * \brief Serialize the color to JSON as a three-element array [r, g, b].
     *
     * \param[out] j The JSON object to write the value into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json &j) const override;

    /**
     * \brief Deserialize the color from a JSON three-element array [r, g, b].
     *
     * \param[in] j The JSON array containing the RGB values.
     * \return Always \c true.
     */
    bool FromJSON(const nlohmann::json &j) override;

    using BaseProperty::operator=;

  private:
    // purposely not implemented
    ColorProperty &operator=(const ColorProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };


} // namespace mitk

namespace itk
{
  template <typename TComponent>
  void to_json(nlohmann::json& j, const RGBPixel<TComponent>& c)
  {
    j = nlohmann::json::array();

    for (size_t i = 0; i < 3; ++i)
      j.push_back(c[i]);
  }

  template <typename TComponent>
  void from_json(const nlohmann::json& j, RGBPixel<TComponent>& c)
  {
    for (size_t i = 0; i < 3; ++i)
      j.at(i).get_to(c[i]);
  }
} // namespace itk

#endif
