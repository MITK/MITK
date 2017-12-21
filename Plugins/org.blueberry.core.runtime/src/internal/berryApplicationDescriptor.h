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

#ifndef BERRYAPPLICATIONDESCRIPTOR_H
#define BERRYAPPLICATIONDESCRIPTOR_H

#include <service/application/ctkApplicationDescriptor.h>

#include <ctkServiceRegistration.h>

#include <QObject>
#include <QSharedPointer>
#include <QMutex>
#include <QWaitCondition>

struct ctkApplicationHandle;
class ctkPlugin;

namespace berry {

class ApplicationContainer;
class ApplicationHandle;

class ApplicationDescriptor : public QObject, public ctkApplicationDescriptor
{
  Q_OBJECT
  Q_INTERFACES(ctkApplicationDescriptor)

public:

  static const QString APP_TYPE; // = "blueberry.application.type";
  static const QString APP_DEFAULT; // = "blueberry.application.default";
  static const QString APP_TYPE_MAIN_THREAD; // = "main.thread";
  static const QString APP_TYPE_ANY_THREAD; // = "any.thread";

  enum Flag {

    FLAG_VISIBLE = 0x01,
    FLAG_CARD_SINGLETON_GLOGAL = 0x02,
    FLAG_CARD_SINGLETON_SCOPED = 0x04,
    FLAG_CARD_UNLIMITED = 0x08,
    FLAG_CARD_LIMITED = 0x10,
    FLAG_TYPE_MAIN_THREAD = 0x20,
    FLAG_TYPE_ANY_THREAD = 0x40,
    FLAG_DEFAULT_APP = 0x80

  };

  Q_DECLARE_FLAGS(Flags, Flag)

  /**
   * Constructs the {@code ApplicationDescriptor}.
   *
   * @param applicationId
   *            The identifier of the application. Its value is also available
   *            as the {@code service.pid} service property of this
   *            {@code ApplicationDescriptor} service. This parameter must not
   *            be {@code null}.
   * @throws std::invalid_argument if the specified {@code applicationId} is empty.
   */
  ApplicationDescriptor(const QSharedPointer<ctkPlugin>& contributor, const QString& pid, const QString& name,
                        const QString& iconPath, const Flags& flags, int cardinality,
                        ApplicationContainer* appContainer);

  ~ApplicationDescriptor() override;

  QString getApplicationId() const override;

  QHash<QString, QVariant> getProperties(const QLocale& locale) const override;

  QHash<QString, QVariant> getProperties() const override;

  ctkApplicationHandle* launch(const QHash<QString, QVariant>& arguments) override;

  void Unregister();

private:

  friend class ApplicationHandle;
  friend class ApplicationContainer;

  void CheckArgs(const QHash<QString, QVariant> arguments);

  /*
   * Gets a snapshot of the current service properties.
   */
  ctkProperties GetServiceProperties() const;

  /*
   * Returns the appHandle.  If it does not exist then one is created.
   */
  ApplicationHandle* CreateAppHandle(const QHash<QString, QVariant>& arguments);

  QString GetInstanceID() const;

  QString GetLocation() const;

  bool GetLocked() const;

  int GetThreadType() const;

  QString GetThreadTypeString() const;

  ApplicationContainer* GetContainerManager() const;

  void RefreshProperties();

  void SetServiceRegistration(const ctkServiceRegistration& sr);

  ctkServiceRegistration GetServiceRegistration() const;


  QString pid;
  QString name;
  QSharedPointer<ctkPlugin> contributor;
  ApplicationContainer* appContainer;
  Flags flags;
  int cardinality;
  QString iconPath;
  mutable int long instanceId;
  bool locked;

  ctkServiceRegistration sr;

  mutable QMutex mutex;

  mutable QMutex registrationMutex;
  mutable QWaitCondition registrationMutexCondition;
  bool registrationValidOrWaiting;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(berry::ApplicationDescriptor::Flags)

#endif // BERRYAPPLICATIONDESCRIPTOR_H
