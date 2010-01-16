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

#ifndef BERRYIVIEWREGISTRY_H_
#define BERRYIVIEWREGISTRY_H_

#include "berryUiDll.h"

#include "berryIViewDescriptor.h"
#include "berryIViewCategory.h"
#include "berryIStickyViewDescriptor.h"

#include <vector>

namespace berry {

/**
 * \ingroup org_blueberry_ui
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
struct BERRY_UI IViewRegistry
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
  virtual IViewDescriptor::Pointer Find(const std::string& id) const = 0;
  
  /**
   * Returns an array of view categories.
   *
   * @return the categories.
   */
  virtual std::vector<IViewCategory::Pointer> GetCategories() = 0;

  /**
   * Return a list of views defined in the registry.
   *
   * @return the views.
   */
  virtual const std::vector<IViewDescriptor::Pointer>& GetViews() const = 0;

  /**
   * Return a list of sticky views defined in the registry.
   *
   * @return the sticky views.  Never <code>null</code>.
   */
  virtual std::vector<IStickyViewDescriptor::Pointer> GetStickyViews() const = 0;

  virtual ~IViewRegistry() {}
};

}

#endif /*BERRYIVIEWREGISTRY_H_*/
