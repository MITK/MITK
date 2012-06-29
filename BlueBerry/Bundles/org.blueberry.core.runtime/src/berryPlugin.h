/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYPLUGIN_H_
#define BERRYPLUGIN_H_

#include <org_blueberry_core_runtime_Export.h>

#include <ctkPluginActivator.h>

namespace berry {

struct ILog;

class org_blueberry_core_runtime_EXPORT Plugin : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  Plugin();

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  QSharedPointer<ctkPlugin> GetPlugin() const;

  /**
   * Returns the log for this plug-in.  If no such log exists, one is created.
   *
   * @return the log for this plug-in
   * XXX change this into a LogMgr service that would keep track of the map. See if it can be a service factory.
   */
  ILog* GetLog() const;

  QString GetStateLocation() const;

protected:

  ctkPluginContext* m_Context;

};

}

#endif /*BERRYPLUGIN_H_*/
