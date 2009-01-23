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


#ifndef CHERRYISOURCEPROVIDER_H_
#define CHERRYISOURCEPROVIDER_H_

#include "cherryUiDll.h"
#include "cherryISourceProviderListener.h"

#include <map>
#include <vector>
#include <string>

namespace cherry {

/**
 * <p>
 * A provider of notifications for when a change has occurred to a particular
 * type of source. These providers can be given to the appropriate service, and
 * this service will then re-evaluate the appropriate pieces of its internal
 * state in response to these changes.
 * </p>
 * <p>
 * It is recommended that clients subclass <code>AbstractSourceProvider</code>
 * instead, as this provides some common support for listeners.
 * </p>
 *
 * @since 3.1
 * @see org.opencherry.ui.handlers.IHandlerService
 * @see org.opencherry.ui.ISources
 */
struct CHERRY_UI ISourceProvider {

  cherrySimpleInterfaceMacro(ISourceProvider, cherry);

  typedef std::map<std::string, Object::Pointer> StateMapType;

  /**
   * Allows the source provider an opportunity to clean up resources (e.g.,
   * listeners) before being released. This method should be called by the
   * creator after the source provider has been removed from all the services
   * with which it was registered.
   */
  virtual ~ISourceProvider() {}

  /**
   * Adds a listener to this source provider. This listener will be notified
   * whenever the corresponding source changes.
   *
   * @param listener
   *            The listener to add; must not be <code>null</code>.
   */
  virtual void AddSourceProviderListener(ISourceProviderListener::Pointer listener) = 0;


  /**
   * Returns the current state of the sources tracked by this provider. This
   * is used to provide a view of the world if the event loop is busy and
   * things are some state has already changed.
   * <p>
   * For use with core expressions, this map should contain
   * IEvaluationContext#UNDEFINED_VARIABLE for properties which
   * are only sometimes available.
   * </p>
   *
   * @return A map of variable names (<code>String</code>) to variable
   *         values (<code>Object</code>). This may be empty, and may be
   *         <code>null</code>.
   */
  virtual StateMapType GetCurrentState() = 0;

  /**
   * Returns the names of those sources provided by this class. This is used
   * by clients of source providers to determine which source providers they
   * actually need.
   *
   * @return An array of source names. This value should never be
   *         <code>null</code> or empty.
   */
  virtual std::vector<std::string> GetProvidedSourceNames() = 0;

  /**
   * Removes a listener from this source provider. This listener will be
   * notified whenever the corresponding source changes.
   *
   * @param listener
   *            The listener to remove; must not be <code>null</code>.
   */
  virtual void RemoveSourceProviderListener(ISourceProviderListener::Pointer listener) = 0;
};

}

#endif /* CHERRYISOURCEPROVIDER_H_ */
