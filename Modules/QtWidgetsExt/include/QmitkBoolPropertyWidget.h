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
#ifndef QMITK_BOOLPROPERTYVIEW_H_INCLUDED
#define QMITK_BOOLPROPERTYVIEW_H_INCLUDED

#include "MitkQtWidgetsExtExports.h"
#include <mitkProperties.h>
#include <QCheckBox>

class _BoolPropertyWidgetImpl;

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkBoolPropertyWidget : public QCheckBox
{
  Q_OBJECT

  public:

    QmitkBoolPropertyWidget(QWidget* parent = nullptr );
    QmitkBoolPropertyWidget(const QString& text, QWidget* parent = nullptr );
    virtual ~QmitkBoolPropertyWidget();

    void SetProperty(mitk::BoolProperty* property);

  protected slots:

    void onToggle(bool on);

  protected:

    _BoolPropertyWidgetImpl* m_PropEditorImpl;

};

#endif

