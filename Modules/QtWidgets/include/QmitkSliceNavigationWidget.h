/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSliceNavigationWidget_h
#define QmitkSliceNavigationWidget_h

#include "MitkQtWidgetsExports.h"
#include "ui_QmitkSliceNavigationWidget.h"

#include <mitkStepper.h>

#include <QString>
#include <QWidget>

class MITKQTWIDGETS_EXPORT QmitkSliceNavigationWidget : public QWidget, public Ui::QmitkSliceNavigationWidget
{
  Q_OBJECT

public:

  QmitkSliceNavigationWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);

  /**
   * \brief Convert the passed value to a QString representation.
   *
   * If the value exceeds a certain maximum, "INF" (for "infinity") is displayed
   * instead.
   */
  QString ClippedValueToString(float value);

  QString GetLabelUnit();

  QString GetMinValueLabel();

  QString GetMaxValueLabel();

  int GetPos();

  bool GetInverseDirection() const;

  bool GetInvertedControls() const;

public slots:

  /**
   * \brief Update the slider with the recent changes applied to the navigation widget.
   *
   * Intended to be called via event mechanism, e.g. if the connected
   * mitk::Stepper is modified.
   */
  void Refetch();

  /**
   * \brief Set the stepper that should be represented and modified.
   *
   */
  void SetStepper(mitk::Stepper* stepper);

  /**
   * \brief Enable / disable displaying of the minimum and maximum labels
   */
  void ShowLabels(bool show);

  /**
   * \brief Enable / disable displaying of the unit label (range will be displayed
   * without unit if enabled).
   */
  void ShowLabelUnit(bool show);

  void SetPos(int val);

  void SetInverseDirection(bool inverseDirection);

  void SetInvertedControls(bool invertedControls);

protected slots:

  /**
   * \brief React on changes of the slider.
   *
   *  The position of the stepper (class member) is set according to the
   *  current slider value.
   *  This will also update the value labels.
   */
  void SliderChanged(double);

  /**
   * \brief React on changes of the spinbox.
   *
   *  The position of the stepper (class member) is set according to the
   *  current spinbox value.
   *  This will also update the value labels.
   */
  void SpinBoxChanged(double);

  /**
   * \brief Set label values for the range minimum and maximum.
   *
   *  Displayed as labels to the left and the right of the slider, if enabled.
   */
  void SetLabelValues(float min, float max);

  /**
   * \brief Enable / disable labels for the range minimum or maximum.
   *
   *  Displayed as labels to the left and the right of the slider, if enabled.
   */
  void SetLabelValuesValid(bool minValid, bool maxValid);

  /**
   * \brief Set the range unit (e.g. mm or ms).
   *
   *  Displayed below the range labels, if enabled.
   */
  void SetLabelUnit(const char* unit);

  /**
   * \brief Configure slider with labels according to range and unit settings.
   */
  void SetLabels();

protected:

  mitk::Stepper::Pointer m_Stepper;
  bool m_InRefetch;

  QString m_LabelUnit;

  bool m_HasLabelUnit;
  bool m_MaxValueValid;
  bool m_MinValueValid;

  bool m_HasLabels;
  float m_MinValue;
  float m_MaxValue;

  bool m_InverseDirection;
  bool m_InvertedControls;

};

#endif
