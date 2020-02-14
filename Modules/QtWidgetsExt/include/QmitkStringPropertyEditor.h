/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMITK_STRINGPROPERTYEDITOR_H_INCLUDED
#define QMITK_STRINGPROPERTYEDITOR_H_INCLUDED

#include "MitkQtWidgetsExtExports.h"
#include <QLineEdit>
#include <mitkPropertyObserver.h>
#include <mitkStringProperty.h>

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkStringPropertyEditor : public QLineEdit, public mitk::PropertyEditor
{
  Q_OBJECT

public:
  QmitkStringPropertyEditor(mitk::StringProperty *, QWidget *parent);
  ~QmitkStringPropertyEditor() override;

protected:
  void PropertyChanged() override;
  void PropertyRemoved() override;

  mitk::StringProperty *m_StringProperty;

protected slots:

  void onTextChanged(const QString &);

private:
};

#endif
