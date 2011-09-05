/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef MITKMODULEACTIVATOR_H_
#define MITKMODULEACTIVATOR_H_

namespace mitk {

class ModuleContext;

/**
 * \ingroup MicroServices
 *
 * Customizes the starting and stopping of a MITK module.
 * <p>
 * <code>%ModuleActivator</code> is an interface that can be implemented by
 * MITK modules. The MITK core can create instances of a
 * module's <code>%ModuleActivator</code> as required. If an instance's
 * <code>moduleActivator::Load</code> method executes successfully, it is
 * guaranteed that the same instance's <code>%ModuleActivator::Unload</code>
 * method will be called when the module is to be unloaded. The MITK core does
 * not concurrently call a <code>%ModuleActivator</code> object.
 *
 * <p>
 * <code>%ModuleActivator</code> is an abstract class interface whose implementations
 * must be exported via a special macro. Implementations are usually declared
 * and defined directly in .cpp files.
 *
 * <p>
 * \snippet uServices-activator/main.cpp 0
 *
 * <p>
 * The class implementing the <code>%ModuleActivator</code> interface must have a public
 * default constructor so that a <code>%ModuleActivator</code>
 * object can be created by the MITK core.
 *
 */
struct ModuleActivator
{
public:

  virtual ~ModuleActivator() {}

  /**
   * Called when this module is loaded. This method
   * can be used to register services or to allocate any resources that this
   * module may need globally (during the whole module lifetime).
   *
   * <p>
   * This method must complete and return to its caller in a timely manner.
   *
   * @param context The execution context of the module being loaded.
   * @throws std::exception If this method throws an exception, this
   *         module is marked as stopped and the framework will remove this
   *         module's listeners, unregister all services registered by this
   *         module, and release all services used by this module.
   */
  virtual void Load(ModuleContext* context) = 0;

  /**
   * Called when this module is unloaded. In general, this
   * method should undo the work that the <code>ModuleActivator::Load</code>
   * method started. There should be no active threads that were started by
   * this module when this method returns.
   *
   * <p>
   * This method must complete and return to its caller in a timely manner.
   *
   * @param context The execution context of the module being unloaded.
   * @throws std::exception If this method throws an exception, the
   *         module is still marked as unloaded, and the framework will remove
   *         the module's listeners, unregister all services registered by the
   *         module, and release all services used by the module.
   */
  virtual void Unload(ModuleContext* context) = 0;

};

} // end namespace mitk

#define MITK_EXPORT_MODULE_ACTIVATOR(moduleName, type)                                  \
  extern "C" MITK_EXPORT mitk::ModuleActivator* _mitk_module_activator_instance_ ## moduleName () \
  {                                                                                     \
    struct ScopedPointer                                                                \
    {                                                                                   \
      ScopedPointer(mitk::ModuleActivator* activator = 0) : m_Activator(activator) {}   \
      ~ScopedPointer() { delete m_Activator; }                                          \
      mitk::ModuleActivator* m_Activator;                                               \
    };                                                                                  \
                                                                                        \
    static ScopedPointer activatorPtr;                                                  \
    if (activatorPtr.m_Activator == 0) activatorPtr.m_Activator = new type;             \
    return activatorPtr.m_Activator;                                                    \
  }

#endif /* MITKMODULEACTIVATOR_H_ */
