/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef BERRYQTSHOWVIEWDIALOG_H_
#define BERRYQTSHOWVIEWDIALOG_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QDialog>

#include <berryIViewRegistry.h>
#include <berryIShowViewDialog.h>

#include "ui_berryQtShowViewDialog.h"

namespace berry {

class QtShowViewDialog : public QDialog, public IShowViewDialog
{
public:
  berryObjectMacro(QtShowViewDialog);

  QtShowViewDialog(IViewRegistry* registry, QWidget* parent = 0, Qt::WindowFlags f = 0);

  std::vector<IViewDescriptor::Pointer> GetSelection();
  int Open();

protected:

  Ui::QtShowViewDialog_ m_UserInterface;
};

}

#endif /*BERRYQTSHOWVIEWDIALOG_H_*/
