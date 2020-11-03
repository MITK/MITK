/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMITK_NUMBERPROPERTYEDITOR_H_INCLUDED
#define QMITK_NUMBERPROPERTYEDITOR_H_INCLUDED

#include "MitkQtWidgetsExtExports.h"
#include <QSpinBox>
#include <mitkProperties.h>
#include <mitkPropertyObserver.h>

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkNumberPropertyEditor : public QSpinBox, public mitk::PropertyEditor
{
  Q_OBJECT
  Q_PROPERTY(short decimalPlaces READ getDecimalPlaces WRITE setDecimalPlaces)
  Q_PROPERTY(bool showPercent READ getShowPercent WRITE setShowPercent)
  Q_PROPERTY(int minValue READ minValue WRITE setMinValue)
  Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue)

public:
  QmitkNumberPropertyEditor(mitk::IntProperty *, QWidget *parent);
  QmitkNumberPropertyEditor(mitk::FloatProperty *, QWidget *parent);
  QmitkNumberPropertyEditor(mitk::DoubleProperty *, QWidget *parent);

  ~QmitkNumberPropertyEditor() override;

  short getDecimalPlaces() const;
  void setDecimalPlaces(short);

  bool getShowPercent() const;
  void setShowPercent(bool);

  int minValue() const;
  void setMinValue(int);
  int maxValue() const;
  void setMaxValue(int);
  double doubleValue() const;
  void setDoubleValue(double);

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
  double m_FactorPropertyToSpinbox; // internal conversion factor. neccessary because spinbox ranges work only with ints
  double m_FactorSpinboxToDisplay;  // internal conversion factor. neccessary because spinbox ranges work only with ints
  bool m_ShowPercents;              // whether values are given in percent (0.5 -> 50%)

protected slots:

  void onValueChanged(int);

private:
  void adjustFactors(short, bool);

  bool m_SelfChangeLock;
};

#endif
