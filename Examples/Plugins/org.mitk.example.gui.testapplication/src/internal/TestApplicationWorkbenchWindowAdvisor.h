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

#ifndef TESTAPPLICATIONWORKBENCHWINDOWADVISOR_H_
#define TESTAPPLICATIONWORKBENCHWINDOWADVISOR_H_

#include <berryWorkbenchWindowAdvisor.h>
#include <QObject.h>

class TestApplicationWorkbenchWindowAdvisor : public QObject, public berry::WorkbenchWindowAdvisor
{
  Q_OBJECT

public:

    TestApplicationWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer configurer);

    berry::ActionBarAdvisor::Pointer CreateActionBarAdvisor(
        berry::IActionBarConfigurer::Pointer configurer);

    void CreateWindowContents(berry::Shell::Pointer shell);

    void PostWindowCreate();

    void PreWindowOpen();

private:

private slots:

  void UpdateStyle();


//    berry::IDropTargetListener::Pointer dropTargetListener;
};

#endif /*TESTAPPLICATIONWORKBENCHWINDOWADVISOR_H_*/
