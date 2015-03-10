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

#ifndef BERRYIVIEWREGISTRY_H_
#define BERRYIVIEWREGISTRY_H_

#include <org_blueberry_ui_qt_Export.h>

#include "berryIViewDescriptor.h"
#include "berryIViewCategory.h"
#include "berryIStickyViewDescriptor.h"


namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * The view registry maintains a list of views explicitly registered
 * against the view extension point.
 * <p>
 * The description of a given view is kept in a <code>IViewDescriptor</code>.
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see IViewDescriptor
 * @see IStickyViewDescriptor
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IViewRegistry
{
  /**
   * Return a view descriptor with the given extension id.  If no view exists
   * with the id return <code>null</code>.
   * Will also return <code>null</code> if the view descriptor exists, but
   * is filtered by an expression-based activity.
   *
   * @param id the id to search for
   * @return the descriptor or <code>null</code>
   */
  virtual IViewDescriptor::Pointer Find(const QString& id) const = 0;

  /**
   * Returns an array of view categories.
   *
   * @return the categories.
   */
  virtual QList<IViewCategory::Pointer> GetCategories() = 0;

  /**
   * Return a list of views defined in the registry.
   *
   * @return the views.
   */
  virtual QList<IViewDescriptor::Pointer> GetViews() const = 0;

  /**
   * Return a list of sticky views defined in the registry.
   *
   * @return the sticky views.  Never <code>null</code>.
   */
  virtual QList<IStickyViewDescriptor::Pointer> GetStickyViews() const = 0;

  virtual ~IViewRegistry();
};

}

#endif /*BERRYIVIEWREGISTRY_H_*/
