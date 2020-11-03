/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYAPPLICATIONHANDLE_H
#define BERRYAPPLICATIONHANDLE_H

#include <berryIApplicationContext.h>

#include <berrySmartPointer.h>

#include "berryApplicationDescriptor.h"

#include <service/application/ctkApplicationHandle.h>
#include <ctkApplicationRunnable.h>
#include <ctkServiceRegistration.h>

#include <QObject>
#include <QMutex>
#include <QWaitCondition>

namespace berry {

class ApplicationDescriptor;

struct IApplication;
struct IConfigurationElement;

class ApplicationHandle : public QObject, public ctkApplicationRunnable, public ctkApplicationHandle, public IApplicationContext
{
  Q_OBJECT
  Q_INTERFACES(ctkApplicationHandle berry::IApplicationContext)

public:

  enum StatusCode
  {
    // Indicates the application is starting
    FLAG_STARTING = 0x01,
    // Indicates the application is active
    FLAG_ACTIVE = 0x02,
    // Indicates the application is stopping
    FLAG_STOPPING = 0x04,
    // Indicates the application is stopped
    FLAG_STOPPED = 0x08
  };
  Q_DECLARE_FLAGS(Status, StatusCode)

  ApplicationHandle(const QString& instanceId, const QHash<QString, QVariant>& arguments, ApplicationDescriptor* descriptor);

  ApplicationDescriptor* getApplicationDescriptor() const override;

  QString getState() const override;

  QVariant getExitValue(long timeout) const override;

  QString getInstanceId() const override;

  void destroy() override;

  QHash<QString, QVariant> GetArguments() const override;

  void ApplicationRunning() override;

  QVariant run(const QVariant& context) override;
  void stop() override;

  ctkServiceRegistration GetServiceRegistration() const;

  ctkDictionary GetServiceProperties() const;

  IApplication* GetApplication() const;

  bool IsDefault() const;

  SmartPointer<IConfigurationElement> GetConfiguration() const;


private:

  friend class ApplicationDescriptor;

  void SetAppStatus(Status status);
  void SetAppStatus_unlocked(Status status);

  void SetServiceRegistration(const ctkServiceRegistration& sr);

  QVariant SetInternalResult(const QVariant& result, IApplication* tokenApp);

  QString getState_unlocked() const;

  ctkDictionary GetServiceProperties_unlocked() const;

  QString instanceId;
  ApplicationDescriptor* descriptor;

  bool defaultAppInstance;
  QHash<QString, QVariant> arguments;
  Status status;
  ctkServiceRegistration handleRegistration;
  QVariant result;
  bool setResult;
  IApplication* application;

  mutable QMutex mutex;
  mutable QWaitCondition waitCondition;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(berry::ApplicationHandle::Status)

#endif // BERRYAPPLICATIONHANDLE_H
