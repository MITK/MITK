/*=========================================================================
 
Program:   openCherry Platform
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

#ifndef CHERRYQTSHOWVIEWDIALOG_H_
#define CHERRYQTSHOWVIEWDIALOG_H_

#include <QDialog>

#include <org.opencherry.ui/src/cherryIViewRegistry.h>
#include <org.opencherry.ui/src/dialogs/cherryIShowViewDialog.h>

#include "ui_cherryQtShowViewDialog.h"

namespace cherry {

class QtShowViewDialog : public QDialog, public IShowViewDialog
{
public:
  cherryClassMacro(QtShowViewDialog);
  
  QtShowViewDialog(IViewRegistry* registry, QWidget* parent = 0, Qt::WindowFlags f = 0);
  
  std::vector<IViewDescriptor::Pointer> GetSelection();
  int Exec();
  
protected:
  
  Ui::QtShowViewDialog_ m_UserInterface;
};

}

#endif /*CHERRYQTSHOWVIEWDIALOG_H_*/
