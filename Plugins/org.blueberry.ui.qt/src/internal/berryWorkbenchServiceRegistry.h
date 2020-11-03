/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHSERVICEREGISTRY_H_
#define BERRYWORKBENCHSERVICEREGISTRY_H_

#include "services/berryIServiceLocator.h"

#include <vector>
#include <map>

namespace berry
{

struct ISourceProvider;
struct IServiceFactory;
struct IExtensionPoint;
struct IConfigurationElement;
class ServiceLocator;
class AbstractSourceProvider;

/**
 * This class will create a service from the matching factory. If the factory
 * doesn't exist, it will try and load it from the registry.
 *
 * @since 3.4
 */
class WorkbenchServiceRegistry
{ // implements IExtensionChangeHandler {

private:

  /**
   *
   */
  static const QString WORKBENCH_LEVEL; // = "workbench"; //$NON-NLS-1$

  static const QString EXT_ID_SERVICES; // = "org.blueberry.ui.services"; //$NON-NLS-1$

  WorkbenchServiceRegistry();

  struct ServiceFactoryHandle : public Object
  {
    berryObjectMacro(ServiceFactoryHandle);

    QScopedPointer<const IServiceFactory> factory;
    // TODO used in removeExtension to react to bundles being unloaded
    //WeakHashMap serviceLocators = new WeakHashMap();
    QList<QString> serviceNames;

    ServiceFactoryHandle(const IServiceFactory* factory);
  };

  QHash<QString, ServiceFactoryHandle::Pointer> factories;

  ServiceFactoryHandle::Pointer LoadFromRegistry(const QString& key);

  SmartPointer<IExtensionPoint> GetExtensionPoint() const;

  static QStringList SupportedLevels();

  void ProcessVariables(
      const QList<SmartPointer<IConfigurationElement> >& children) const;

  struct GlobalParentLocator: public IServiceLocator
  {
    Object* GetService(const QString& api) override;
    bool HasService(const QString& api) const override;
  };

public:

  static WorkbenchServiceRegistry* GetRegistry();

  /**
   * Used as the global service locator's parent.
   */
  static const IServiceLocator::Pointer GLOBAL_PARENT; // = new GlobalParentLocator();

  Object::Pointer GetService(const QString& key,
                             IServiceLocator* parentLocator, ServiceLocator* locator);

  QList<SmartPointer<AbstractSourceProvider> > GetSourceProviders() const;

  void InitializeSourcePriorities();

  //  void addExtension(IExtensionTracker tracker, IExtension extension) {
  //    // we don't need to react to adds because we are not caching the extensions we find -
  //    // next time a service is requested, we will look at all extensions again in
  //    // loadFromRegistry
  //  }

  //  void removeExtension(IExtension extension, Object[] objects) {
  //    for (int i = 0; i < objects.length; i++) {
  //      Object object = objects[i];
  //      if (object instanceof ServiceFactoryHandle) {
  //        ServiceFactoryHandle handle = (ServiceFactoryHandle) object;
  //        Set locatorSet = handle.serviceLocators.keySet();
  //        ServiceLocator[] locators = (ServiceLocator[]) locatorSet.toArray(new ServiceLocator[locatorSet.size()]);
  //        Arrays.sort(locators, new Comparator(){
  //          public int compare(Object o1, Object o2) {
  //            ServiceLocator loc1 = (ServiceLocator) o1;
  //            ServiceLocator loc2 = (ServiceLocator) o2;
  //            int l1 = ((IWorkbenchLocationService) loc1
  //                .getService(IWorkbenchLocationService.class))
  //                .getServiceLevel();
  //            int l2 = ((IWorkbenchLocationService) loc2
  //                .getService(IWorkbenchLocationService.class))
  //                .getServiceLevel();
  //            return l1 < l2 ? -1 : (l1 > l2 ? 1 : 0);
  //          }
  //        });
  //        for (int j = 0; j < locators.length; j++) {
  //          ServiceLocator serviceLocator = locators[j];
  //          if (!serviceLocator.isDisposed()) {
  //            serviceLocator.unregisterServices(handle.serviceNames);
  //          }
  //        }
  //        handle.factory = null;
  //        for (int j = 0; j < handle.serviceNames.length; j++) {
  //          String serviceName = handle.serviceNames[j];
  //          if (factories.get(serviceName) == handle) {
  //            factories.remove(serviceName);
  //          }
  //        }
  //      }
  //    }
  //  }
};

}

#endif /* BERRYWORKBENCHSERVICEREGISTRY_H_ */
