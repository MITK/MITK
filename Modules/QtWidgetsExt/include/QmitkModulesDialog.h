/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkModulesDialog_h
#define QmitkModulesDialog_h

#include <QDialog>

#include <MitkQtWidgetsExtExports.h>

class MITKQTWIDGETSEXT_EXPORT QmitkModulesDialog : public QDialog
{
public:
  explicit QmitkModulesDialog(QWidget *parent = nullptr,
                              Qt::WindowFlags f = Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
};

#endif
