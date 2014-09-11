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
class MitkQtWidgetsExt_EXPORT QmitkEnumerationPropertyWidget : public QComboBox
{
  Q_OBJECT

  public:

    QmitkEnumerationPropertyWidget(QWidget* parent = 0);
    ~QmitkEnumerationPropertyWidget();

    void SetProperty(mitk::EnumerationProperty* property);

  protected slots:

    void OnIndexChanged(int index);

  protected:

    _EnumPropEditorImpl* propView;

};

#endif

