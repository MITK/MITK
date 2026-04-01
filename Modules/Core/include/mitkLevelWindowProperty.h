/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkLevelWindowProperty_h
#define mitkLevelWindowProperty_h

#include <mitkBaseProperty.h>
#include <mitkLevelWindow.h>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * \brief Property for storing a mitk::LevelWindow value.
   *
   * Holds the level/window settings used for intensity windowing during
   * image rendering. The level defines the center of the window, and the
   * window defines its width.
   *
   * \ingroup DataManagement
   *
   * \note If you want to apply the mitk::LevelWindowProperty to an mitk::Image,
   * make sure to set the mitk::RenderingModeProperty to a mode which supports
   * level window (e.g. LEVELWINDOW_COLOR). Check the documentation of
   * mitk::RenderingModeProperty. For a code example see
   * mitkImageVtkMapper2DLevelWindowTest.cpp in Core/Code/Testing.
   *
   * \sa BaseProperty
   * \sa LevelWindow
   * \sa RenderingModeProperty
   */
  class MITKCORE_EXPORT LevelWindowProperty : public BaseProperty
  {
  protected:
    LevelWindow m_LevWin;

    LevelWindowProperty();

    LevelWindowProperty(const LevelWindowProperty &other);

    LevelWindowProperty(const mitk::LevelWindow &levWin);

    mitkCloneMacro(LevelWindowProperty);

  public:
    mitkClassMacro(LevelWindowProperty, BaseProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self) mitkNewMacro1Param(LevelWindowProperty, const mitk::LevelWindow &);

    /** \brief The type of the value stored by this property. */
    typedef LevelWindow ValueType;

    ~LevelWindowProperty() override;

    /**
     * \brief Get the level window value.
     * \return A const reference to the stored LevelWindow.
     */
    const mitk::LevelWindow &GetLevelWindow() const;

    /**
     * \brief Get the level window value (alias for GetLevelWindow()).
     * \return A const reference to the stored LevelWindow.
     */
    const mitk::LevelWindow &GetValue() const;

    /**
     * \brief Set the level window value.
     *
     * Marks the property as modified if the new value differs from the current one.
     *
     * \param[in] levWin The new LevelWindow to set.
     */
    void SetLevelWindow(const LevelWindow &levWin);

    /**
     * \brief Set the level window value (alias for SetLevelWindow()).
     *
     * \param[in] levWin The new LevelWindow to set.
     */
    void SetValue(const ValueType &levWin);

    /**
     * \brief Return the level window as a human-readable string.
     *
     * Format: "L:<level> W:<window>".
     *
     * \return A string representation of the level and window values.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Serialize the level window value to JSON.
     *
     * \param[out] j The JSON object to write the value into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize the level window value from JSON.
     *
     * \param[in] j The JSON object containing the LevelWindow data.
     * \return Always \c true.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;

  private:
    // purposely not implemented
    LevelWindowProperty &operator=(const LevelWindowProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace mitk

#endif
