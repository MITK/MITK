/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

  QSharedPointer<ctkPlugin> GetPlugin() const;

  /**
   * Returns the log for this plug-in.  If no such log exists, one is created.
   *
   * @return the log for this plug-in
   * XXX change this into a LogMgr service that would keep track of the map. See if it can be a service factory.
   */
  ILog* GetLog() const;

  /**
   * Returns the location in the local file system of the
   * plug-in state area for this plug-in.
   * If the plug-in state area did not exist prior to this call,
   * it is created.
   * <p>
   * The plug-in state area is a file directory within the
   * platform's metadata area where a plug-in is free to create files.
   * The content and structure of this area is defined by the plug-in,
   * and the particular plug-in is solely responsible for any files
   * it puts there. It is recommended for plug-in preference settings and
   * other configuration parameters.
   * </p>
   * @throws ctkIllegalStateException, when the system is running with no data area (-data @none),
   * or when a data area has not been set yet.
   * @return a local file system path
   */
  QString GetStateLocation() const;

protected:

  ctkPluginContext* m_Context;

};

}

#endif /*BERRYPLUGIN_H_*/
