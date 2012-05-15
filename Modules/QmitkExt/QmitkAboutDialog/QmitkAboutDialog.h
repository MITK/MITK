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

#include <QDialog>
#include "QmitkExtExports.h"
#include <QWidget>
#include <mitkCommon.h>


class QmitkExt_EXPORT QmitkAboutDialog : public QDialog
{
  Q_OBJECT

  public:

    QmitkAboutDialog(QWidget* parent = 0, Qt::WindowFlags f = Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    virtual ~QmitkAboutDialog();

protected slots:

    void ShowModules();

};
