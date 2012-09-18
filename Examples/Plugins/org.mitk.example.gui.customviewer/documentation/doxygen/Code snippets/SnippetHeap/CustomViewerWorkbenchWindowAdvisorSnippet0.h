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

#ifndef CUSTOMVIEWERWORKBENCHWINDOWADVISOR_H_
#define CUSTOMVIEWERWORKBENCHWINDOWADVISOR_H_

#include <berryWorkbenchWindowAdvisor.h>
#include <QObject.h>
#include <QmitkFileOpenAction.h>
//! [CustomViewerWorkbenchWindowAdvisorClassDeclaration]
class CustomViewerWorkbenchWindowAdvisor : public QObject, public berry::WorkbenchWindowAdvisor
{
//! [CustomViewerWorkbenchWindowAdvisorClassDeclaration]
  Q_OBJECT

public:

    CustomViewerWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer configurer);

    ~CustomViewerWorkbenchWindowAdvisor();

    void CreateWindowContents(berry::Shell::Pointer shell);

    void PostWindowCreate();

    void PreWindowOpen();

private:

private slots:

  void UpdateStyle();
  void OpenFile();

private:

  QmitkFileOpenAction* FileOpenAction;
};

#endif /*CUSTOMVIEWERWORKBENCHWINDOWADVISOR_H_*/
