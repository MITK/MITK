/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkNumberPropertyView_h
#define QmitkNumberPropertyView_h

#include "MitkQtWidgetsExtExports.h"
#include <QLabel>
#include <mitkProperties.h>
#include <mitkPropertyObserver.h>

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkNumberPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT
  Q_PROPERTY(short decimalPlaces READ decimalPlaces WRITE setDecimalPlaces)
  Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
  Q_PROPERTY(bool showPercent READ showPercent WRITE setShowPercent)

public:
  QmitkNumberPropertyView(const mitk::IntProperty *, QWidget *parent);
  QmitkNumberPropertyView(const mitk::FloatProperty *, QWidget *parent);
  QmitkNumberPropertyView(const mitk::DoubleProperty *, QWidget *parent);

  ~QmitkNumberPropertyView() override;

  short decimalPlaces() const;
  void setDecimalPlaces(short);

  QString suffix() const;
  void setSuffix(const QString &);

  bool showPercent() const;
  void setShowPercent(bool);

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
