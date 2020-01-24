/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYABSTRACTSOURCEPROVIDER_H_
#define BERRYABSTRACTSOURCEPROVIDER_H_

#include "berryISourceProvider.h"

#include <berryISourceProviderListener.h>

namespace berry {

struct IServiceLocator;

/**
 * <p>
 * An implementation of <code>ISourceProvider</code> that provides listener
 * support. Subclasses need only call <code>fireSourceChanged</code> whenever
 * appropriate.
 * </p>
 */
class AbstractSourceProvider : public ISourceProvider {

protected:

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
                         const QString& sourceName, Object::ConstPointer sourceValue);

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
                         const QHash<QString, Object::ConstPointer>& sourceValuesByName);

  /**
   * Logs a debugging message in an appropriate manner. If the message is
   * <code>null</code> or the <code>DEBUG</code> is <code>false</code>,
   * then this method does nothing.
   *
   * @param message
   *            The debugging message to log; if <code>null</code>, then
   *            nothing is logged.
   */
  void LogDebuggingInfo(const QString& message);


private:


  ISourceProviderListener::Events sourceEvents;


public:

  /**
   * Whether source providers should print out debugging information to the
   * console when events arrive.
   */
  static bool DEBUG; // = Policy.DEBUG_SOURCES;

  berryObjectMacro(berry::AbstractSourceProvider);

  void AddSourceProviderListener(ISourceProviderListener* listener) override;

  void RemoveSourceProviderListener(ISourceProviderListener *listener) override;

  /**
   * This method is called when the source provider is instantiated by
   * <code>org.blueberry.ui.services</code>. Clients may override this method
   * to perform initialization.
   *
   * @param locator
   *            The global service locator. It can be used to retrieve
   *            services like the IContextService
   */
  virtual void Initialize(IServiceLocator* locator);
};

}

#endif /* BERRYABSTRACTSOURCEPROVIDER_H_ */
