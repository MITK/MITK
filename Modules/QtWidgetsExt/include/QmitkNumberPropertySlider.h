/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkNumberPropertySlider_h
#define QmitkNumberPropertySlider_h

#include "MitkQtWidgetsExtExports.h"

#include <QSlider>

#include <memory>

namespace mitk
{
  class IntProperty;
  class FloatProperty;
  class DoubleProperty;
}

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkNumberPropertySlider : public QSlider
{
  Q_OBJECT
  Q_PROPERTY(short decimalPlaces READ getDecimalPlaces WRITE setDecimalPlaces)
  Q_PROPERTY(bool showPercent READ getShowPercent WRITE setShowPercent)
  Q_PROPERTY(int minValue READ minValue WRITE setMinValue)
  Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue)

public:
  QmitkNumberPropertySlider(QWidget *parent = nullptr);
  ~QmitkNumberPropertySlider() override;

  void SetProperty(mitk::IntProperty *property);
  void SetProperty(mitk::FloatProperty *property);
  void SetProperty(mitk::DoubleProperty *property);

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

protected slots:

  void onValueChanged(int);

private:
  class Impl;
  std::unique_ptr<Impl> d;
};

#endif
