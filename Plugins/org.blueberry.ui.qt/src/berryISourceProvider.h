/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYISOURCEPROVIDER_H_
#define BERRYISOURCEPROVIDER_H_

#include <berryObject.h>

#include <QHash>
#include <QStringList>

#include <QObject>

namespace berry {

struct ISourceProviderListener;

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
 * @see IHandlerService
 * @see ISources
 */
struct ISourceProvider : public virtual Object
{

  berryObjectMacro(berry::ISourceProvider)

  typedef QHash<QString, Object::ConstPointer> StateMapType;

  ~ISourceProvider();


  /**
   * Adds a listener to this source provider. This listener will be notified
   * whenever the corresponding source changes.
   *
   * @param listener
   *            The listener to add; must not be <code>null</code>.
   */
  virtual void AddSourceProviderListener(ISourceProviderListener* listener) = 0;

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
  virtual StateMapType GetCurrentState() const = 0;

  /**
   * Returns the names of those sources provided by this class. This is used
   * by clients of source providers to determine which source providers they
   * actually need.
   *
   * @return An array of source names. This value should never be
   *         <code>null</code> or empty.
   */
  virtual QList<QString> GetProvidedSourceNames() const = 0;

  /**
   * Removes a listener from this source provider. This listener will be
   * notified whenever the corresponding source changes.
   *
   * @param listener
   *            The listener to remove; must not be <code>null</code>.
   */
  virtual void RemoveSourceProviderListener(ISourceProviderListener* listener) = 0;
};

}

Q_DECLARE_INTERFACE(berry::ISourceProvider, "org.blueberry.ui.ISourceProvider")

#endif /* BERRYISOURCEPROVIDER_H_ */
