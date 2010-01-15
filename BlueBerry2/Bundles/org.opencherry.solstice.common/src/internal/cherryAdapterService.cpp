/*=========================================================================
 
Program:   openCherry Platform
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

#include "cherryAdapterService.h"

namespace cherry {

AdapterManager* AdapterService::GetDefault()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  static AdapterService instance;
  return &instance;
}

AdapterService::AdapterService() :
  factories(5), lazyFactoryProviders(1)
{

}

bool
AdapterService::IsA(const std::type_info& type) const
{
  std::string name(GetType().name());
  return name == type.name() || Service::IsA(type);
}
  
const std::type_info& 
AdapterService::GetType() const
{
  return typeid(IAdapterManager);
}

void AdapterService::AddFactoriesFor(const std::string& typeName,
    std::map<std::string, IAdapterFactory*>& table)
{
  List factoryList = (List) getFactories().get(typeName);
  if (factoryList == null)
    return;
  for (int i = 0, imax = factoryList.size(); i < imax; i++)
  {
    IAdapterFactory factory = (IAdapterFactory) factoryList.get(i);
    if (factory instanceof IAdapterFactoryExt)
    {
      String[] adapters = ((IAdapterFactoryExt) factory).getAdapterNames();
      for (int j = 0; j < adapters.length; j++)
      {
        if (table.get(adapters[j]) == null)
          table.put(adapters[j], factory);
      }
    }
    else
    {
      Class[] adapters = factory.getAdapterList();
      for (int j = 0; j < adapters.length; j++)
      {
        String adapterName = adapters[j].getName();
        if (table.get(adapterName) == null)
          table.put(adapterName, factory);
      }
    }
  }
}

void AdapterService::CacheClassLookup(IAdapterFactory* factory,
    const std::type_info& clazz)
{
  synchronized(classLookupLock)
  {
    //cache reference to lookup to protect against concurrent flush
    Map lookup = classLookup;
    if (lookup == null)
      classLookup = lookup = new HashMap(4);
    HashMap classes = (HashMap) lookup.get(factory);
    if (classes == null)
    {
      classes = new HashMap(4);
      lookup.put(factory, classes);
    }
    classes.put(clazz.getName(), clazz);
  }
}

void* /*Class*/
AdapterService::CachedClassForName(IAdapterFactory* factory,
    const std::string& typeName)
{
  synchronized(classLookupLock)
  {
    Class clazz = null;
    //cache reference to lookup to protect against concurrent flush
    Map lookup = classLookup;
    if (lookup != null)
    {
      HashMap classes = (HashMap) lookup.get(factory);
      if (classes != null)
      {
        clazz = (Class) classes.get(typeName);
      }
    }
    return clazz;
  }
}

void* /*Class*/
AdapterService::ClassForName(IAdapterFactory* factory,
    const std::string& typeName)
{
  Class clazz = cachedClassForName(factory, typeName);
  if (clazz == null)
  {
    if (factory instanceof IAdapterFactoryExt)
      factory = ((IAdapterFactoryExt) factory).loadFactory(false);
    if (factory != null)
    {
      try
      {
        clazz = factory.getClass().getClassLoader().loadClass(typeName);
      }
      catch (ClassNotFoundException e)
      {
        // it is possible that the default bundle classloader is unaware of this class
        // but the adaptor factory can load it in some other way. See bug 200068.
        if (typeName == null)
        return null;
        Class[] adapterList = factory.getAdapterList();
        clazz = null;
        for (int i = 0; i < adapterList.length; i++)
        {
          if (typeName.equals(adapterList[i].getName()))
          {
            clazz = adapterList[i];
            break;
          }
        }
        if (clazz == null)
        return null; // class not yet loaded
      }
      cacheClassLookup(factory, clazz);
    }
  }
  return clazz;
}

void AdapterService::ComputeAdapterTypes(std::vector<std::string>& types,
    const std::type_info& adaptable)
{
  Set types = getFactories(adaptable).keySet();
  return (String[]) types.toArray(new String[types.size()]);
}

void AdapterService::GetFactories(
    Poco::HashMap<std::string, IAdapterFactory*> table,
    const std::type_info& adaptable)
{
  //cache reference to lookup to protect against concurrent flush
  Map lookup = adapterLookup;
  if (lookup == null)
    adapterLookup = lookup = Collections.synchronizedMap(new HashMap(30));
  Map table = (Map) lookup.get(adaptable.getName());
  if (table == null)
  {
    // calculate adapters for the class
    table = new HashMap(4);
    Class[] classes = computeClassOrder(adaptable);
    for (int i = 0; i < classes.length; i++)
      addFactoriesFor(classes[i].getName(), table);
    // cache the table
    lookup.put(adaptable.getName(), table);
  }
  return table;
}

//public: Class[] computeClassOrder(Class adaptable) {
//    Class[] classes = null;
//    //cache reference to lookup to protect against concurrent flush
//    Map lookup = classSearchOrderLookup;
//    if (lookup == null)
//      classSearchOrderLookup = lookup = Collections.synchronizedMap(new HashMap());
//    else
//      classes = (Class[]) lookup.get(adaptable);
//    // compute class order only if it hasn't been cached before
//    if (classes == null) {
//      ArrayList classList = new ArrayList();
//      computeClassOrder(adaptable, classList);
//      classes = (Class[]) classList.toArray(new Class[classList.size()]);
//      lookup.put(adaptable, classes);
//    }
//    return classes;
//  }


//private: void computeClassOrder(Class adaptable, Collection classes) {
//    Class clazz = adaptable;
//    Set seen = new HashSet(4);
//    while (clazz != null) {
//      classes.add(clazz);
//      computeInterfaceOrder(clazz.getInterfaces(), classes, seen);
//      clazz = clazz.getSuperclass();
//    }
//  }

//private: void computeInterfaceOrder(Class[] interfaces, Collection classes, Set seen) {
//    List newInterfaces = new ArrayList(interfaces.length);
//    for (int i = 0; i < interfaces.length; i++) {
//      Class interfac = interfaces[i];
//      if (seen.add(interfac)) {
//        //note we cannot recurse here without changing the resulting interface order
//        classes.add(interfac);
//        newInterfaces.add(interfac);
//      }
//    }
//    for (Iterator it = newInterfaces.iterator(); it.hasNext();)
//      computeInterfaceOrder(((Class) it.next()).getInterfaces(), classes, seen);
//  }

void AdapterService::FlushLookup()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  adapterLookup = null;
  classLookup = null;
  //classSearchOrderLookup = null;
}

Poco::Any AdapterService::GetAdapter(Poco::Any& adaptable,
    const std::type_info& adapterType)
{
  IAdapterFactory factory = (IAdapterFactory) getFactories(adaptable.getClass()).get(adapterType.getName());
  Object result = null;
  if (factory != null)
    result = factory.getAdapter(adaptable, adapterType);
  if (result == null && adapterType.isInstance(adaptable))
    return adaptable;
  return result;
}

Poco::Any AdapterService::GetAdapter(Poco::Any& adaptable, const std::string& adapterType)
{
  return getAdapter(adaptable, adapterType, false);
}

Poco::Any AdapterService::GetAdapter(Poco::Any& adaptable,
    const std::string& adapterType, bool force)
{
  IAdapterFactory factory = (IAdapterFactory) getFactories(adaptable.getClass()).get(adapterType);
  if (force && factory instanceof IAdapterFactoryExt)
factory = ((IAdapterFactoryExt) factory).loadFactory(true);
        Object result = null;
  if (factory != null)
  {
    Class clazz = classForName(factory, adapterType);
    if (clazz != null)
      result = factory.getAdapter(adaptable, clazz);
  }
  if (result == null && adaptable.getClass().getName().equals(adapterType))
    return adaptable;
  return result;
}

bool AdapterService::HasAdapter(Poco::Any& adaptable,
    const std::string& adapterTypeName)
{
  return getFactories(adaptable.getClass()).get(adapterTypeName) != null;
}

int AdapterService::QueryAdapter(Poco::Any& adaptable,
    const std::string& adapterTypeName)
{
  IAdapterFactory factory = (IAdapterFactory) getFactories(adaptable.getClass()).get(adapterTypeName);
  if (factory == null)
    return NONE;
  if (factory instanceof IAdapterFactoryExt)
  {
    factory = ((IAdapterFactoryExt) factory).loadFactory(false); // don't force loading
    if (factory == null)
      return NOT_LOADED;
  }
  return LOADED;
}

Poco::Any AdapterService::LoadAdapter(Poco::Any& adaptable,
    const std::string& adapterTypeName)
{
  return getAdapter(adaptable, adapterTypeName, true);
}

void AdapterService::RegisterAdapters(IAdapterFactory* factory,
    const std::type_info& adaptable)
{
  Poco::Mutex::ScopeLock lock(m_Mutex);
  this->RegisterFactory(factory, adaptable.getName());
  this->FlushLookup();
}

void AdapterService::RegisterFactory(IAdapterFactory* factory,
    const std::string& adaptableType)
{
  List list = (List) factories.get(adaptableType);
  if (list == null)
  {
    list = new ArrayList(5);
    factories.put(adaptableType, list);
  }
  list.add(factory);
}

void AdapterService::UnregisterAdapters(IAdapterFactory* factory)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  for (Iterator it = factories.values().iterator(); it.hasNext();)
    ((List) it.next()).remove(factory);
  flushLookup();
}

void AdapterService::UnregisterAdapters(IAdapterFactory* factory,
    const std::type_info& adaptable)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  List factoryList = (List) factories.get(adaptable.getName());
  if (factoryList == null)
    return;
  factoryList.remove(factory);
  flushLookup();
}

void AdapterService::UnregisterAllAdapters()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  factories.clear();
  flushLookup();
}

void AdapterService::RegisterLazyFactoryProvider(
    IAdapterManagerProvider* factoryProvider)
{
  synchronized(lazyFactoryProviders)
  {
    lazyFactoryProviders.add(factoryProvider);
  }
}

bool AdapterService::UnregisterLazyFactoryProvider(
    IAdapterManagerProvider* factoryProvider)
{
  synchronized(lazyFactoryProviders)
  {
    return lazyFactoryProviders.remove(factoryProvider);
  }
}

Poco::HashMap<>& AdapterService::GetFactories()
{
  // avoid the synchronize if we don't have to call it
  if (lazyFactoryProviders.size() == 0)
    return factories;
  synchronized(lazyFactoryProviders)
  {
    while (lazyFactoryProviders.size() > 0)
    {
      IAdapterServiceProvider provider =
          (IAdapterManagerProvider) lazyFactoryProviders.remove(0);
      if (provider.addFactories(this))
        flushLookup();
    }
  }
  return factories;
}

}  // namespace cherry
