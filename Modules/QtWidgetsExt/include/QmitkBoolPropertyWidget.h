/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkBoolPropertyWidget_h
#define QmitkBoolPropertyWidget_h

#include "MitkQtWidgetsExtExports.h"
#include <QCheckBox>
#include <mitkProperties.h>

class _BoolPropertyWidgetImpl;

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkBoolPropertyWidget : public QCheckBox
{
  Q_OBJECT

public:
  QmitkBoolPropertyWidget(QWidget *parent = nullptr);
  QmitkBoolPropertyWidget(const QString &text, QWidget *parent = nullptr);
  ~QmitkBoolPropertyWidget() override;

  void SetProperty(mitk::BoolProperty *property);

protected slots:

  void onToggle(bool on);

protected:
  _BoolPropertyWidgetImpl *m_PropEditorImpl;
};

#endif
