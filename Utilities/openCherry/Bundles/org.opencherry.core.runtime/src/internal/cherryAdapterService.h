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

#ifndef CHERRYADAPTERMANAGER_H_
#define CHERRYADAPTERMANAGER_H_

#include <service/cherryService.h>

#include "../cherryIAdapterManager.h"
#include "../cherryIAdapterFactory.h"

#include <Poco/HashMap.h>

#include <string>
#include <list>
#include <map>
#include <typeinfo>

namespace cherry {

/**
 * This class is the standard implementation of <code>IAdapterManager</code>. It provides
 * fast lookup of property values with the following semantics:
 * <ul>
 * <li>At most one factory will be invoked per property lookup
 * <li>If multiple installed factories provide the same adapter, only the first found in
 * the search order will be invoked.
 * <li>The search order from a class with the definition <br>
 * <code>class X extends Y implements A, B</code><br> is as follows: <il>
 * <li>the target's class: X
 * <li>X's superclasses in order to <code>Object</code>
 * <li>a breadth-first traversal of the target class's interfaces in the order returned by
 * <code>getInterfaces</code> (in the example, A and its superinterfaces then B and its
 * superinterfaces) </il>
 * </ul>
 * 
 * @see IAdapterFactory
 * @see IAdapterManager
 */
class AdapterService : public Service, public IAdapterManager {
  
  
  /** 
   * Cache of adapters for a given adaptable class. Maps String  -> Map
   * (adaptable class name -> (adapter class name -> factory instance))
   * Thread safety note: The outer map is synchronized using a synchronized
   * map wrapper class.  The inner map is not synchronized, but it is immutable
   * so synchronization is not necessary.
   */
private:
  std::map<std::string, Poco::HashMap<std::string, IAdapterFactory*> > adapterLookup;

  /**
   * Cache of classes for a given type name. Avoids too many loadClass calls.
   * (factory -> (type name -> Class)).
   * Thread safety note: Since this structure is a nested hash map, and both
   * the inner and outer maps are mutable, access to this entire structure is
   * controlled by the classLookupLock field.  Note the field can still be
   * nulled concurrently without holding the lock.
   */
  std::map<IAdapterFactory*, Poco::HashMap<std::string, const std::type_info&> > classLookup;

  /**
   * The lock object controlling access to the classLookup data structure.
   */
  //const Object classLookupLock = new Object();

  /**
   * Cache of class lookup order (Class -> Class[]). This avoids having to compute often, and
   * provides clients with quick lookup for instanceOf checks based on type name.
   * Thread safety note: The map is synchronized using a synchronized
   * map wrapper class.  The arrays within the map are immutable.
   */
  //std::map<> classSearchOrderLookup;

  /**
   * Map of factories, keyed by <code>String</code>, fully qualified class name of
   * the adaptable class that the factory provides adapters for. Value is a <code>List</code>
   * of <code>IAdapterFactory</code>.
   */
  Poco::HashMap<std::string, IAdapterFactory*> factories;

  std::list<IAdapterManagerProvider*> lazyFactoryProviders;

public: bool IsA(const std::type_info& type) const;
public: const std::type_info& GetType() const;
  
public: static AdapterManager GetDefault();

  /**
   * Private constructor to block instance creation.
   */
private: AdapterManager();

  /**
   * Given a type name, add all of the factories that respond to those types into
   * the given table. Each entry will be keyed by the adapter class name (supplied in
   * IAdapterFactory.getAdapterList).
   */
private: void AddFactoriesFor(const std::string& typeName, std::map<std::string, IAdapterFactory*>& table);

private: void CacheClassLookup(IAdapterFactory* factory, const std::type_info& clazz);

private: void* /*Class*/ CachedClassForName(IAdapterFactory* factory, const std::string& typeName);

  /**
   * Returns the class with the given fully qualified name, or null
   * if that class does not exist or belongs to a plug-in that has not
   * yet been loaded.
   */
private: void* /*Class*/ ClassForName(IAdapterFactory* factory, const std::string& typeName);

  /* (non-Javadoc)
   * @see org.opencherry.core.runtime.IAdapterManager#getAdapterTypes(java.lang.Class)
   */
public: void ComputeAdapterTypes(std::vector<std::string>& types, const std::type_info& adaptable);

  /**
   * Computes the adapters that the provided class can adapt to, along
   * with the factory object that can perform that transformation. Returns 
   * a table of adapter class name to factory object.
   * @param adaptable
   */
private: void GetFactories(std::map<std::string, IAdapterFactory*> table, const std::type_info& adaptable);

//public: Class[] computeClassOrder(Class adaptable);

  /**
   * Builds and returns a table of adapters for the given adaptable type.
   * The table is keyed by adapter class name. The
   * value is the <b>sole<b> factory that defines that adapter. Note that
   * if multiple adapters technically define the same property, only the
   * first found in the search order is considered.
   * 
   * Note that it is important to maintain a consistent class and interface
   * lookup order. See the class comment for more details.
   */
//private: void computeClassOrder(Class adaptable, Collection classes);

//private: void computeInterfaceOrder(Class[] interfaces, Collection classes, Set seen);

  /**
   * Flushes the cache of adapter search paths. This is generally required whenever an
   * adapter is added or removed.
   * <p>
   * It is likely easier to just toss the whole cache rather than trying to be smart
   * and remove only those entries affected.
   * </p>
   */
public:  void FlushLookup();

  /* (non-Javadoc)
   * @see org.opencherry.core.runtime.IAdapterManager#getAdapter(java.lang.Object, java.lang.Class)
   */
public: ExpressionVariable::Ptr GetAdapter(ExpressionVariable::Ptr adaptable, const std::type_info& adapterType);

  /* (non-Javadoc)
   * @see org.opencherry.core.runtime.IAdapterManager#getAdapter(java.lang.Object, java.lang.Class)
   */
public: ExpressionVariable::Ptr GetAdapter(ExpressionVariable::Ptr adaptable, const std::string& adapterType);

  /**
   * Returns an adapter of the given type for the provided adapter.
   * @param adaptable the object to adapt
   * @param adapterType the type to adapt the object to
   * @param force <code>true</code> if the plug-in providing the
   * factory should be activated if necessary. <code>false</code>
   * if no plugin activations are desired.
   */
private: ExpressionVariable::Ptr GetAdapter(ExpressionVariable::Ptr adaptable, const std::string& adapterType, bool force);

public: bool HasAdapter(ExpressionVariable::Ptr adaptable, const std::string& adapterTypeName);

  /* (non-Javadoc)
   * @see org.opencherry.core.runtime.IAdapterManager#queryAdapter(java.lang.Object, java.lang.String)
   */
public: int QueryAdapter(ExpressionVariable::Ptr adaptable, const std::string& adapterTypeName);

  /* (non-Javadoc)
   * @see org.opencherry.core.runtime.IAdapterManager#loadAdapter(java.lang.Object, java.lang.String)
   */
public: ExpressionVariable::Ptr LoadAdapter(ExpressionVariable::Ptr adaptable, const std::string& adapterTypeName);

  /*
   * @see IAdapterManager#registerAdapters
   */
public: void RegisterAdapters(IAdapterFactory* factory, const std::type_info& adaptable);

  /*
   * @see IAdapterManager#registerAdapters
   */
public: void RegisterFactory(IAdapterFactory* factory, const std::string& adaptableType);

  /*
   * @see IAdapterManager#unregisterAdapters
   */
public: void UnregisterAdapters(IAdapterFactory* factory);

  /*
   * @see IAdapterManager#unregisterAdapters
   */
public: void UnregisterAdapters(IAdapterFactory* factory, const std::type_info& adaptable);

  /*
   * Shuts down the adapter manager by removing all factories
   * and removing the registry change listener. Should only be
   * invoked during platform shutdown.
   */
public: void UnregisterAllAdapters();

public: void RegisterLazyFactoryProvider(IAdapterManagerProvider* factoryProvider);

public: bool UnregisterLazyFactoryProvider(IAdapterManagerProvider* factoryProvider);

public: Poco::HashMap<>& GetFactories();
};

} // namespace cherry

#endif /*CHERRYADAPTERMANAGER_H_*/
