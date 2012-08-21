///*===================================================================
//
//The Medical Imaging Interaction Toolkit (MITK)
//
//Copyright (c) German Cancer Research Center,
//Division of Medical and Biological Informatics.
//All rights reserved.
//
//This software is distributed WITHOUT ANY WARRANTY; without
//even the implied warranty of MERCHANTABILITY or FITNESS FOR
//A PARTICULAR PURPOSE.
//
//See LICENSE.txt or http://www.mitk.org for details.
//
//===================================================================*/

#ifndef EXTENSIONPOINTCONTRIBUTION_H_
#define EXTENSIONPOINTCONTRIBUTION_H_

/// Berry
#include <berryIApplication.h>

/// Qt
#include <QObject>
#include <QScopedPointer>

class MinimalWorkbenchAdvisor;

class ExtensionPointContribution : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)
  
public:
  
  ExtensionPointContribution();
  ~ExtensionPointContribution();
  
  int Start();
  void Stop();

private:

  QScopedPointer<MinimalWorkbenchAdvisor> wbAdvisor;
};

#endif /*EXTENSIONPOINTCONTRIBUTION_H_*/
