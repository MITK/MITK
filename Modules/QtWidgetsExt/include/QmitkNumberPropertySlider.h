/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkNumberPropertySlider_h
#define QmitkNumberPropertySlider_h

#include <MitkQtWidgetsExtExports.h>

#include <QSlider>

#include <memory>

namespace mitk
{
  class IntProperty;
  class FloatProperty;
  class DoubleProperty;
}

/**
 * \brief A QSlider that edits numeric MITK properties (int, float, double).
 *
 * Provides a slider editor that bidirectionally synchronizes with an
 * mitk::IntProperty, mitk::FloatProperty, or mitk::DoubleProperty. Supports
 * configurable decimal places, percentage display mode, and min/max ranges.
 * The property is bound via SetProperty(). Pass nullptr to unbind.
 *
 * \sa QmitkNumberPropertyEditor, QmitkNumberPropertyView, QmitkPropertyViewFactory
 */
class MITKQTWIDGETSEXT_EXPORT QmitkNumberPropertySlider : public QSlider
{
  Q_OBJECT
  Q_PROPERTY(short decimalPlaces READ getDecimalPlaces WRITE setDecimalPlaces)
  Q_PROPERTY(bool showPercent READ getShowPercent WRITE setShowPercent)
  Q_PROPERTY(int minValue READ minValue WRITE setMinValue)
  Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue)

public:
  /**
   * \brief Construct the slider without an associated property.
   * \param[in] parent The parent widget.
   */
  QmitkNumberPropertySlider(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkNumberPropertySlider() override;

  /**
   * \brief Bind this slider to an integer property.
   * \param[in] property The int property to edit. Pass nullptr to unbind.
   */
  void SetProperty(mitk::IntProperty *property);

  /**
   * \brief Bind this slider to a float property.
   * \param[in] property The float property to edit. Pass nullptr to unbind.
   */
  void SetProperty(mitk::FloatProperty *property);

  /**
   * \brief Bind this slider to a double property.
   * \param[in] property The double property to edit. Pass nullptr to unbind.
   */
  void SetProperty(mitk::DoubleProperty *property);

  /**
   * \brief Get the number of decimal places used for conversion.
   * \return The decimal places count.
   */
  short getDecimalPlaces() const;

  /**
   * \brief Set the number of decimal places (only effective for float/double properties).
   * \param[in] places The number of decimal places.
   */
  void setDecimalPlaces(short places);

  /**
   * \brief Check whether values are interpreted as percentages.
   * \return True if percentage mode is active.
   */
  bool getShowPercent() const;

  /**
   * \brief Enable or disable percentage mode.
   * \param[in] showPercent True to enable percentage mode.
   */
  void setShowPercent(bool showPercent);

  /**
   * \brief Get the minimum allowed value.
   * \return The minimum value.
   */
  int minValue() const;

  /**
   * \brief Set the minimum allowed value.
   * \param[in] value The minimum value.
   */
  void setMinValue(int value);

  /**
   * \brief Get the maximum allowed value.
   * \return The maximum value.
   */
  int maxValue() const;

  /**
   * \brief Set the maximum allowed value.
   * \param[in] value The maximum value.
   */
  void setMaxValue(int value);

  /**
   * \brief Get the current value as a double.
   * \return The property value.
   */
  double doubleValue() const;

  /**
   * \brief Set the current value from a double.
   * \param[in] value The value to set.
   */
  void setDoubleValue(double value);

protected slots:

  void onValueChanged(int);

private:
  class Impl;
  std::unique_ptr<Impl> d;
};

#endif
