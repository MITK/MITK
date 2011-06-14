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

#ifndef BERRYLOGPLUGIN_H_
#define BERRYLOGPLUGIN_H_

#include <ctkPluginActivator.h>

#include "berryQtPlatformLogModel.h"


namespace berry {

class QtLogPlugin : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  QtLogPlugin();

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static QtLogPlugin* GetInstance();

  QtPlatformLogModel* GetLogModel();

private:
  static QtLogPlugin* instance;
  QtPlatformLogModel* m_LogModel;

};

}

#endif /*BERRYLOGPLUGIN_H_*/
