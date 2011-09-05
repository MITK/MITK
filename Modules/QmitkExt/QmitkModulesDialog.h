/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef QMITKMODULESDIALOG_H
#define QMITKMODULESDIALOG_H

#include <QDialog>

#include <QmitkExtExports.h>

class QmitkExt_EXPORT QmitkModulesDialog : public QDialog
{

public:

  explicit QmitkModulesDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

};

#endif // QMITKMODULESDIALOG_H
