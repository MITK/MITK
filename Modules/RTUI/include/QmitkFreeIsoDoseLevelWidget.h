/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkFreeIsoDoseLevelWidget_h
#define QmitkFreeIsoDoseLevelWidget_h

#include <MitkRTUIExports.h>

#include <QWidget>

#include <mitkIsoDoseLevel.h>
#include <memory>

namespace Ui
{
  class QmitkFreeIsoDoseLevelWidget;
}

/**
 * \class QmitkFreeIsoDoseLevelWidget
 * \brief Widget for displaying and editing a single free (user-defined) iso dose level.
 *
 * Provides UI controls for adjusting the dose value (absolute and relative spin boxes
 * and a slider), the iso line color, and the iso line visibility of an mitk::IsoDoseLevel
 * instance. Changes are communicated via signals.
 *
 * \sa mitk::IsoDoseLevel
 * \sa QmitkIsoDoseLevelSetModel
 * \ingroup MitkRTUIModule
 */
class MITKRTUI_EXPORT QmitkFreeIsoDoseLevelWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructor.
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkFreeIsoDoseLevelWidget(QWidget* parent=nullptr);

  /** \brief Destructor. */
  ~QmitkFreeIsoDoseLevelWidget() override;

  /**
   * \brief Returns the currently set reference dose.
   * \return The reference dose value in Gy.
   */
  mitk::DoseValueAbs getReferenceDose() const;

  /**
   * \brief Returns the iso dose level currently managed by this widget.
   * \return Pointer to the current IsoDoseLevel instance.
   */
  mitk::IsoDoseLevel* getIsoDoseLevel() const;

signals:
  /**
   * \brief Emitted when the dose value of the iso dose level changes.
   * \param[in] level Pointer to the modified IsoDoseLevel.
   * \param[in] oldValue The previous relative dose value before the change.
   */
  void ValueChanged(mitk::IsoDoseLevel* level, mitk::DoseValueRel oldValue);

  /**
   * \brief Emitted when the color of the iso dose level changes.
   * \param[in] level Pointer to the modified IsoDoseLevel.
   */
  void ColorChanged(mitk::IsoDoseLevel* level);

  /**
   * \brief Emitted when the iso line visibility of the iso dose level changes.
   * \param[in] level Pointer to the modified IsoDoseLevel.
   */
  void VisualizationStyleChanged(mitk::IsoDoseLevel* level);

  public Q_SLOTS:
    /**
     * \brief Sets the reference dose used for absolute/relative conversion.
     * \param[in] newReferenceDose The new reference dose in Gy.
     */
    void setReferenceDose(double newReferenceDose);

    /**
     * \brief Sets the iso dose level instance managed by this widget.
     * \param[in] level The IsoDoseLevel to display and edit. Must not be nullptr.
     * \throw mitk::Exception if level is nullptr.
     */
    void setIsoDoseLevel(mitk::IsoDoseLevel* level);

    /**
     * \brief Handles changes from the relative dose value spin box.
     * \param[in] newValue The new relative dose value in percent.
     */
    void OnRelValueChanged(double newValue);

    /**
     * \brief Handles changes from the absolute dose value spin box.
     * \param[in] newValue The new absolute dose value in Gy.
     */
    void OnAbsValueChanged(double newValue);

    /**
     * \brief Handles changes from the dose slider.
     * \param[in] newValue The new slider value (interpreted as percentage).
     */
    void OnSliderChanged(int newValue);

    /**
     * \brief Handles clicks on the visibility checkbox.
     * \param[in] checked Whether the iso line should be visible.
     */
    void OnVisibleClicked(bool checked);

    /**
     * \brief Handles color changes from the color button.
     * \param[in] color The newly selected QColor.
     */
    void OnColorChanged(QColor color);

protected:

  /**
  * \brief Updates the widget according to its current settings.
  */
  void update();
  void updateValue(mitk::DoseValueRel newDose);

  std::unique_ptr<Ui::QmitkFreeIsoDoseLevelWidget> m_Controls;
  mitk::DoseValueAbs m_ReferenceDose;
  mitk::IsoDoseLevel::Pointer m_IsoDoseLevel;
  bool m_InternalUpdate;

};

#endif
