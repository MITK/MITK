/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYABSTRACTSOURCEPROVIDER_H_
#define BERRYABSTRACTSOURCEPROVIDER_H_

#include "berryISourceProvider.h"

namespace berry {

using namespace osgi::framework;

struct IServiceLocator;

/**
 * <p>
 * An implementation of <code>ISourceProvider</code> that provides listener
 * support. Subclasses need only call <code>fireSourceChanged</code> whenever
 * appropriate.
 * </p>
 *
 * @since 3.1
 */
class AbstractSourceProvider : public ISourceProvider {

protected:

  /**
   * Whether source providers should print out debugging information to the
   * console when events arrive.
   *
   * @since 3.2
   */
  static bool DEBUG; // = Policy.DEBUG_SOURCES;

  /**
   * Notifies all listeners that a single source has changed.
   *
   * @param sourcePriority
   *            The source priority that has changed.
   * @param sourceName
   *            The name of the source that has changed; must not be
   *            <code>null</code>.
   * @param sourceValue
   *            The new value for the source; may be <code>null</code>.
   */
  void FireSourceChanged(int sourcePriority,
      const std::string& sourceName, Object::Pointer sourceValue);

  /**
   * Notifies all listeners that multiple sources have changed.
   *
   * @param sourcePriority
   *            The source priority that has changed.
   * @param sourceValuesByName
   *            The map of source names (<code>String</code>) to source
   *            values (<code>Object</code>) that have changed; must not
   *            be <code>null</code>. The names must not be
   *            <code>null</code>, but the values may be <code>null</code>.
   */
  void FireSourceChanged(int sourcePriority,
      const std::map<std::string, Object::Pointer>& sourceValuesByName);

  /**
   * Logs a debugging message in an appropriate manner. If the message is
   * <code>null</code> or the <code>DEBUG</code> is <code>false</code>,
   * then this method does nothing.
   *
   * @param message
   *            The debugging message to log; if <code>null</code>, then
   *            nothing is logged.
   * @since 3.2
   */
  void LogDebuggingInfo(const std::string& message);


private:


  ISourceProviderListener::Events sourceEvents;


public:

  void AddSourceProviderListener(ISourceProviderListener::Pointer listener);

  void RemoveSourceProviderListener(
      ISourceProviderListener::Pointer listener);

  /**
   * This method is called when the source provider is instantiated by
   * <code>org.blueberry.ui.services</code>. Clients may override this method
   * to perform initialization.
   *
   * @param locator
   *            The global service locator. It can be used to retrieve
   *            services like the IContextService
   * @since 3.4
   */
  void Initialize(SmartPointer<const IServiceLocator> locator);
};

}

#endif /* BERRYABSTRACTSOURCEPROVIDER_H_ */
