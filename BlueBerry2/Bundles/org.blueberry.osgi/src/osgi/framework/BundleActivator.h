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


#ifndef BUNDLEACTIVATOR_H_
#define BUNDLEACTIVATOR_H_

#include "Object.h"
#include "Macros.h"

#include "BundleContext.h"

namespace osgi {
namespace framework {

/**
 * Customizes the starting and stopping of a bundle.
 * <p>
 * <code>BundleActivator</code> is an interface that may be implemented when a
 * bundle is started or stopped. The Framework can create instances of a
 * bundle's <code>BundleActivator</code> as required. If an instance's
 * <code>BundleActivator.start</code> method executes successfully, it is
 * guaranteed that the same instance's <code>BundleActivator.stop</code>
 * method will be called when the bundle is to be stopped. The Framework must
 * not concurrently call a <code>BundleActivator</code> object.
 *
 * <p>
 * <code>BundleActivator</code> is specified through the
 * <code>Bundle-Activator</code> Manifest header. A bundle can only specify a
 * single <code>BundleActivator</code> in the Manifest file. Fragment bundles
 * must not have a <code>BundleActivator</code>. The form of the Manifest
 * header is:
 *
 * <p>
 * <code>Bundle-Activator: <i>class-name</i></code>
 *
 * <p>
 * where <code><i>class-name</i></code> is a fully qualified Java classname.
 * <p>
 * The specified <code>BundleActivator</code> class must have a public
 * constructor that takes no parameters so that a <code>BundleActivator</code>
 * object can be created by <code>Class.newInstance()</code>.
 *
 * @NotThreadSafe
 * @version $Revision$
 */

struct BundleActivator : public Object {

  osgiInterfaceMacro(osgi::framework::BundleActivator)

  /**
   * Called when this bundle is started so the Framework can perform the
   * bundle-specific activities necessary to start this bundle. This method
   * can be used to register services or to allocate any resources that this
   * bundle needs.
   *
   * <p>
   * This method must complete and return to its caller in a timely manner.
   *
   * @param context The execution context of the bundle being started.
   * @throws Exception If this method throws an exception, this
   *         bundle is marked as stopped and the Framework will remove this
   *         bundle's listeners, unregister all services registered by this
   *         bundle, and release all services used by this bundle.
   */
  virtual void Start(BundleContext::Pointer context) throw(std::exception) = 0;

  /**
   * Called when this bundle is stopped so the Framework can perform the
   * bundle-specific activities necessary to stop the bundle. In general, this
   * method should undo the work that the <code>BundleActivator.start</code>
   * method started. There should be no active threads that were started by
   * this bundle when this bundle returns. A stopped bundle must not call any
   * Framework objects.
   *
   * <p>
   * This method must complete and return to its caller in a timely manner.
   *
   * @param context The execution context of the bundle being stopped.
   * @throws Exception If this method throws an exception, the
   *         bundle is still marked as stopped, and the Framework will remove
   *         the bundle's listeners, unregister all services registered by the
   *         bundle, and release all services used by the bundle.
   */
  virtual void Stop(BundleContext::Pointer context) throw(std::exception) = 0;
};

}
}

#endif /* BUNDLEACTIVATOR_H_ */
