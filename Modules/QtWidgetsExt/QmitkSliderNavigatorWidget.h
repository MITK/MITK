/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKSLIDERNAVIGATORWIDGET_H_
#define QMITKSLIDERNAVIGATORWIDGET_H_

#include "ui_QmitkSliderNavigator.h"
#include "MitkQtWidgetsExtExports.h"

#include <mitkStepper.h>

#include <QWidget>
#include <QString>

class MitkQtWidgetsExt_EXPORT QmitkSliderNavigatorWidget : public QWidget, public Ui::QmitkSliderNavigator
{
  Q_OBJECT

public:

  QmitkSliderNavigatorWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

  QString GetLabelUnit();

  /**
   * \brief Converts the passed value to a QString representation.
   *
   * If the value exceeds a certain maximum, "INF" (for "infinity") is displayed
   * instead.
   */
  QString ClippedValueToString( float value );

  /**
   * \brief Returns range-minimum (displayed as label left of slider if enabled)
   */
  QString GetMinValueLabel();

  QString GetMaxValueLabel();

  int GetPos();

public slots:

  /**
   * \brief Updates the slider with the recent changes applied to the navigator.
   *
   * Intended to be called via event mechanism, e.g. if the connected
   * mitk::Stepper is modified.
   */
  void Refetch();

  void SetStepper( mitk::Stepper * stepper);

  void ShowLabels( bool show );

  /**
   * \brief En-/disables displaying of the unit label (range will be displayed
   * without unit if enabled).
   */
  void ShowLabelUnit( bool show );

  void SetPos(int val);

  void SetInverseDirection (bool inverseDirection);

protected slots:

  void slider_valueChanged( int );

  /**
   * \brief Set range minimum and maximum (displayed as labels left and right
   * of slider if enabled)
   */
  void SetLabelValues( float min, float max );

  void SetLabelValuesValid( bool minValid, bool maxValid );

  /**
   * \brief Set range unit (e.g. mm or ms) which will be displayed below range
   * labels if enabled.
   */
  void SetLabelUnit( const char *unit );

  /**
   * \brief Configure slider with labels according to range and unit settings
   */
  void SetLabels();

  void spinBox_valueChanged( int );


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

};

#endif

