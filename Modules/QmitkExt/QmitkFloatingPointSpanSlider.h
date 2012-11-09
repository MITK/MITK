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


#ifndef QMITKFLOATINGPOINTSPANSLIDER_H
#define QMITKFLOATINGPOINTSPANSLIDER_H

#include <QmitkExtExports.h>

#include <qxtspanslider.h>
#include <QWidget>

class QmitkExt_EXPORT QmitkFloatingPointSpanSlider : public QxtSpanSlider
{
  Q_OBJECT
  Q_PROPERTY(double loweralue READ lowerValue WRITE setLowerValue
             NOTIFY lowerValueChanged)
  Q_PROPERTY(double upperValue READ upperValue WRITE setUpperValue
             NOTIFY upperValueChanged)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)

public:

  QmitkFloatingPointSpanSlider(QWidget *parent = 0);

  void setIntegerMode(bool intMode);

  double lowerValue() const;
  double upperValue() const;

  double maximum() const;
  double minimum() const;

  void setMaximum(double max);
  void setMinimum(double min);
  void setRange(double min, double max);

signals:

  void lowerValueChanged(double lower);
  void upperValueChanged(double upper);
  void spanChanged(double lower, double upper);

public slots:

  void setLowerValue(double lower);
  void setUpperValue(double upper);
  void setSpan(double lower, double upper);

private slots:

  void IntSpanChanged(int lower, int upper);

private:

  void scaleSliderToInt();
  inline int scaleValue(double val);
  inline double unscaleValue(int val);

  double m_LowerValue;
  double m_UpperValue;

  double m_Minimum;
  double m_Maximum;

  double offset;
  double factor;

  bool m_IntMode;
};

#endif // QMITKFLOATINGPOINTSPANSLIDER_H
