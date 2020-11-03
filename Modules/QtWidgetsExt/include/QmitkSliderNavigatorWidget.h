/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSLIDERNAVIGATORWIDGET_H_
#define QMITKSLIDERNAVIGATORWIDGET_H_

#include "MitkQtWidgetsExtExports.h"
#include "ui_QmitkSliderNavigator.h"

#include <mitkStepper.h>

#include <QString>
#include <QWidget>

class MITKQTWIDGETSEXT_EXPORT QmitkSliderNavigatorWidget : public QWidget, public Ui::QmitkSliderNavigator
{
  Q_OBJECT

public:
  QmitkSliderNavigatorWidget(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);

  QString GetLabelUnit();

  /**
   * \brief Converts the passed value to a QString representation.
   *
   * If the value exceeds a certain maximum, "INF" (for "infinity") is displayed
   * instead.
   */
  QString ClippedValueToString(float value);

  /**
   * \brief Returns range-minimum (displayed as label left of slider if enabled)
   */
  QString GetMinValueLabel();

  QString GetMaxValueLabel();

  int GetPos();

  bool GetInverseDirection() const;

  bool GetInvertedControls() const;

public slots:

  /**
   * \brief Updates the slider with the recent changes applied to the navigator.
   *
   * Intended to be called via event mechanism, e.g. if the connected
   * mitk::Stepper is modified.
   */
  void Refetch();

  void SetStepper(mitk::Stepper *stepper);

  void ShowLabels(bool show);

  /**
   * \brief En-/disables displaying of the unit label (range will be displayed
   * without unit if enabled).
   */
  void ShowLabelUnit(bool show);

  void SetPos(int val);

  void SetInverseDirection(bool inverseDirection);

  void SetInvertedControls(bool invertedControls);

protected slots:

  void slider_valueChanged(double);

  /**
   * \brief Set range minimum and maximum (displayed as labels left and right
   * of slider if enabled)
   */
  void SetLabelValues(float min, float max);

  void SetLabelValuesValid(bool minValid, bool maxValid);

  /**
   * \brief Set range unit (e.g. mm or ms) which will be displayed below range
   * labels if enabled.
   */
  void SetLabelUnit(const char *unit);

  /**
   * \brief Configure slider with labels according to range and unit settings
   */
  void SetLabels();

  void spinBox_valueChanged(double);


protected:
  bool m_HasLabelUnit;
  bool m_MaxValueValid;
  bool m_MinValueValid;
  QString m_LabelUnit;
  mitk::Stepper::Pointer m_Stepper;
  bool m_InRefetch;
  bool m_HasLabels;
  float m_MinValue;
  float m_MaxValue;

  bool m_InverseDirection;
  bool m_InvertedControls;

};

#endif
