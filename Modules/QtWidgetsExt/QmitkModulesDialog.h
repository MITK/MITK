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


#ifndef QMITKMODULESDIALOG_H
#define QMITKMODULESDIALOG_H

#include <QDialog>

#include <MitkQtWidgetsExtExports.h>

class MitkQtWidgetsExt_EXPORT QmitkModulesDialog : public QDialog
{

public:

  explicit QmitkModulesDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

};

#endif

