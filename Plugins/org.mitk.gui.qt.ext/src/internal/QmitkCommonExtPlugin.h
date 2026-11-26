/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkCommonExtPlugin_h
#define QmitkCommonExtPlugin_h

#include <ctkPluginActivator.h>

#ifdef MITK_HAS_CRASHHANDLING
#include <mitkICrashReportService.h>
#include <usServiceRegistration.h>

#include <memory>
#endif

class QmitkCommonExtPlugin : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_ext")
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

  static ctkPluginContext* getContext();

private:

  void loadDataFromDisk(const QStringList& args, bool atStartup);
  void startNewInstance(const QStringList& args, const QStringList &files);

private Q_SLOTS:

  void handleIPCMessage(const QByteArray &msg);

private:

  static ctkPluginContext* _context;

#ifdef MITK_HAS_CRASHHANDLING
  std::unique_ptr<mitk::ICrashReportService> m_CrashReportService;
  us::ServiceRegistration<mitk::ICrashReportService> m_CrashReportRegistration;
#endif

};

#endif
