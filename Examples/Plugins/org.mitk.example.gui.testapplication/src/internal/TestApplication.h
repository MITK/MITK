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

#ifndef TESTAPPLICATION_H_
#define TESTAPPLICATION_H_

#include <berryIApplication.h>

#include <QObject>
#include <QScopedPointer>

class TestWorkbenchAdvisor;

class TestApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)
  
public:
  
  TestApplication();
  ~TestApplication();
  
  int Start();
  void Stop();

private:

  QScopedPointer<TestWorkbenchAdvisor> wbAdvisor;
};

#endif /*TESTAPPLICATION_H_*/
