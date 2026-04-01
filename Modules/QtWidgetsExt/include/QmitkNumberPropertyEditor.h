/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkNumberPropertyEditor_h
#define QmitkNumberPropertyEditor_h

#include <MitkQtWidgetsExtExports.h>
#include <QSpinBox>
#include <mitkProperties.h>
#include <mitkPropertyObserver.h>

/**
 * \brief A QSpinBox that edits numeric MITK properties (int, float, double).
 * \ingroup Widgets
 *
 * Provides a spinbox editor that bidirectionally synchronizes with an
 * mitk::IntProperty, mitk::FloatProperty, or mitk::DoubleProperty. Supports
 * configurable decimal places, percentage display mode, and min/max ranges.
 * Internally converts between the property's floating-point value and the
 * spinbox's integer-based value using scale factors.
 *
 * \sa QmitkNumberPropertySlider, QmitkNumberPropertyView, QmitkPropertyViewFactory
 */
class MITKQTWIDGETSEXT_EXPORT QmitkNumberPropertyEditor : public QSpinBox, public mitk::PropertyEditor
{
  Q_OBJECT
  Q_PROPERTY(short decimalPlaces READ getDecimalPlaces WRITE setDecimalPlaces)
  Q_PROPERTY(bool showPercent READ getShowPercent WRITE setShowPercent)
  Q_PROPERTY(int minValue READ minValue WRITE setMinValue)
  Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue)

public:
  /**
   * \brief Construct an editor for an integer property.
   * \param[in] property The int property to edit.
   * \param[in] parent The parent widget.
   */
  QmitkNumberPropertyEditor(mitk::IntProperty *property, QWidget *parent);

  /**
   * \brief Construct an editor for a float property.
   * \param[in] property The float property to edit.
   * \param[in] parent The parent widget.
   */
  QmitkNumberPropertyEditor(mitk::FloatProperty *property, QWidget *parent);

  /**
   * \brief Construct an editor for a double property.
   * \param[in] property The double property to edit.
   * \param[in] parent The parent widget.
   */
  QmitkNumberPropertyEditor(mitk::DoubleProperty *property, QWidget *parent);

  /** \brief Destructor. */
  ~QmitkNumberPropertyEditor() override;

  /**
   * \brief Get the number of decimal places shown.
   * \return The decimal places count.
   */
  short getDecimalPlaces() const;

  /**
   * \brief Set the number of decimal places (only effective for float/double properties).
   * \param[in] places The number of decimal places.
   */
  void setDecimalPlaces(short places);

  /**
   * \brief Check whether values are displayed as percentages.
   * \return True if percentage mode is active.
   */
  bool getShowPercent() const;

  /**
   * \brief Enable or disable percentage display (e.g. 0.5 displayed as 50%).
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

protected:
  void initialize();

  QString textFromValue(int) const override;
  int valueFromText(const QString &) const override;

  void PropertyChanged() override;
  void PropertyRemoved() override;

  void DisplayNumber();

  union {
    mitk::GenericProperty<int> *m_IntProperty;
    mitk::GenericProperty<float> *m_FloatProperty;
    mitk::GenericProperty<double> *m_DoubleProperty;
  };

  const int m_DataType;

  short m_DecimalPlaces;            // how many decimal places are shown
  double m_FactorPropertyToSpinbox; // internal conversion factor. necessary because spinbox ranges work only with ints
  double m_FactorSpinboxToDisplay;  // internal conversion factor. necessary because spinbox ranges work only with ints
  bool m_ShowPercents;              // whether values are given in percent (0.5 -> 50%)

protected slots:

  void onValueChanged(int);

private:
  void adjustFactors(short, bool);

  bool m_SelfChangeLock;
};

#endif
