/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPrimitiveMovieNavigatorWidget_h
#define QmitkPrimitiveMovieNavigatorWidget_h

#include "MitkQtWidgetsExtExports.h"
#include <QTimer>
#include <QWidget>
#include <ui_QmitkPrimitiveMovieNavigatorWidget.h>

class MITKQTWIDGETSEXT_EXPORT QmitkPrimitiveMovieNavigatorWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkPrimitiveMovieNavigatorWidget(QWidget *parent = nullptr, Qt::WindowFlags fl = nullptr);
  ~QmitkPrimitiveMovieNavigatorWidget() override;

  virtual int getTimerInterval();

public slots:
  virtual void Refetch();
  virtual void SetStepper(mitk::Stepper *stepper);
  virtual void goButton_clicked();
  virtual void stopButton_clicked();
  virtual void spinBoxValueChanged(int value);
  virtual void setTimerInterval(int timerIntervalInMS);

protected:
  Ui::QmitkPrimitiveMovieNavigator m_Controls;
  mitk::Stepper::Pointer m_Stepper;
  bool m_InRefetch;
  QTimer *m_Timer;
  int m_TimerIntervalInMS;

private slots:
  virtual void next();
};

#endif
