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
#ifndef QMITK_STRINGPROPERTYEDITOR_H_INCLUDED
#define QMITK_STRINGPROPERTYEDITOR_H_INCLUDED

#include <mitkPropertyObserver.h>
#include "QmitkExtExports.h"
#include <mitkStringProperty.h>
#include <QLineEdit>

/// @ingroup Widgets
class QmitkExt_EXPORT QmitkStringPropertyEditor : public QLineEdit, public mitk::PropertyEditor
{
  Q_OBJECT

  public:

    QmitkStringPropertyEditor( mitk::StringProperty*, QWidget* parent );
    virtual ~QmitkStringPropertyEditor();

  protected:

    virtual void PropertyChanged();
    virtual void PropertyRemoved();

    mitk::StringProperty* m_StringProperty;

  protected slots:

    void onTextChanged(const QString&);

  private:

};

#endif

