/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkNumberPropertyView_h
#define QmitkNumberPropertyView_h

#include <MitkQtWidgetsExtExports.h>
#include <QLabel>
#include <mitkProperties.h>
#include <mitkPropertyObserver.h>

/**
 * \brief Read-only QLabel that displays a numeric MITK property value.
 * \ingroup Widgets
 *
 * Observes an mitk::IntProperty, mitk::FloatProperty, or mitk::DoubleProperty
 * and displays its current value as text. Supports configurable decimal places,
 * a text suffix, and percentage display mode. If the property is removed,
 * "n/a" is shown.
 *
 * \sa QmitkNumberPropertyEditor, QmitkNumberPropertySlider, QmitkPropertyViewFactory
 */
class MITKQTWIDGETSEXT_EXPORT QmitkNumberPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT
  Q_PROPERTY(short decimalPlaces READ decimalPlaces WRITE setDecimalPlaces)
  Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
  Q_PROPERTY(bool showPercent READ showPercent WRITE setShowPercent)

public:
  /**
   * \brief Construct a view for an integer property.
   * \param[in] property The int property to observe.
   * \param[in] parent The parent widget.
   */
  QmitkNumberPropertyView(const mitk::IntProperty *property, QWidget *parent);

  /**
   * \brief Construct a view for a float property.
   * \param[in] property The float property to observe.
   * \param[in] parent The parent widget.
   */
  QmitkNumberPropertyView(const mitk::FloatProperty *property, QWidget *parent);

  /**
   * \brief Construct a view for a double property.
   * \param[in] property The double property to observe.
   * \param[in] parent The parent widget.
   */
  QmitkNumberPropertyView(const mitk::DoubleProperty *property, QWidget *parent);

  /** \brief Destructor. */
  ~QmitkNumberPropertyView() override;

  /**
   * \brief Get the number of decimal places displayed.
   * \return The decimal places count.
   */
  short decimalPlaces() const;

  /**
   * \brief Set the number of decimal places to display.
   * \param[in] places The number of decimal places.
   */
  void setDecimalPlaces(short places);

  /**
   * \brief Get the current text suffix.
   * \return The suffix string.
   */
  QString suffix() const;

  /**
   * \brief Set a text suffix appended to the displayed value.
   * \param[in] suffix The suffix string (e.g. " mm").
   */
  void setSuffix(const QString &suffix);

  /**
   * \brief Check whether values are displayed as percentages.
   * \return True if percentage mode is active.
   */
  bool showPercent() const;

  /**
   * \brief Enable or disable percentage display (value multiplied by 100, "%" suffix).
   * \param[in] show True to enable percentage mode.
   */
  void setShowPercent(bool show);

protected:
  void initialize();

  void PropertyChanged() override;
  void PropertyRemoved() override;

  void DisplayNumber();

  union {
    const mitk::GenericProperty<int> *m_IntProperty;
    const mitk::GenericProperty<float> *m_FloatProperty;
    const mitk::GenericProperty<double> *m_DoubleProperty;
  };

  const int m_DataType;

  short m_DecimalPlaces; /// -1 indicates "no limit to decimal places"
  QString m_Suffix;
  double m_DisplayFactor;
};

#endif
