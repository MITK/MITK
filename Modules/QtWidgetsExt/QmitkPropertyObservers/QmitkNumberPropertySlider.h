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
#ifndef QmitkNumberPropertySliderhincludecd
#define QmitkNumberPropertySliderhincludecd

#include "MitkQtWidgetsExtExports.h"

#include <QSlider>

#include <memory>

namespace mitk {
class IntProperty;
class FloatProperty;
class DoubleProperty;
}

/// @ingroup Widgets
class MitkQtWidgetsExt_EXPORT QmitkNumberPropertySlider : public QSlider
{
  Q_OBJECT
  Q_PROPERTY( short decimalPlaces READ getDecimalPlaces WRITE setDecimalPlaces )
  Q_PROPERTY( bool showPercent READ getShowPercent WRITE setShowPercent )
  Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
  Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )

public:

  QmitkNumberPropertySlider(QWidget* parent = 0);
  virtual ~QmitkNumberPropertySlider();

  void SetProperty(mitk::IntProperty* property);
  void SetProperty(mitk::FloatProperty* property);
  void SetProperty(mitk::DoubleProperty* property);

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
  std::auto_ptr<Impl> d;
};

#endif

