/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITK_ENUMERATIONPROPERTYWIDGET_H_INCLUDED
#define QMITK_ENUMERATIONPROPERTYWIDGET_H_INCLUDED

#include "MitkQtWidgetsExtExports.h"

#include <QComboBox>
#include <QHash>

namespace mitk
{
  class EnumerationProperty;
}

class _EnumPropEditorImpl;

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkEnumerationPropertyWidget : public QComboBox
{
  Q_OBJECT

public:
  QmitkEnumerationPropertyWidget(QWidget *parent = nullptr);
  ~QmitkEnumerationPropertyWidget() override;

  void SetProperty(mitk::EnumerationProperty *property);

protected slots:

  void OnIndexChanged(int index);

protected:
  _EnumPropEditorImpl *propView;
};

#endif
