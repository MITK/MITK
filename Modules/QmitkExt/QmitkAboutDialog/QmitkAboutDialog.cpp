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

#include "QmitkAboutDialog.h"

#include <ui_QmitkAboutDialogGUI.h>


#include <itkConfigure.h>
#include <vtkConfigure.h>
#include <mitkVersion.h>

QmitkAboutDialog::QmitkAboutDialog(QWidget* parent, Qt::WindowFlags f)
:QDialog(parent, f)
{
  Ui::QmitkAboutDialog gui;
  gui.setupUi(this);

  QString mitkRevision(MITK_SVN_REVISION);
  mitkRevision.replace( QRegExp("[^0-9]+(\\d+).*"), "\\1");
  QString itkVersion = "%1.%2.%3";
  itkVersion = itkVersion.arg(ITK_VERSION_MAJOR).arg(ITK_VERSION_MINOR).arg(ITK_VERSION_PATCH);
  QString vtkVersion = "%1.%2.%3";
  vtkVersion = vtkVersion.arg(VTK_MAJOR_VERSION).arg(VTK_MINOR_VERSION).arg(VTK_BUILD_VERSION);

  gui.m_PropsLabel->setText(gui.m_PropsLabel->text().arg(itkVersion, QT_VERSION_STR, vtkVersion, mitkRevision));
      
  //general
  //connect( m_GUI->btnQuitApplication,SIGNAL(clicked()), qApp, SLOT(closeAllWindows()) );

}

QmitkAboutDialog::~QmitkAboutDialog()
{

}









