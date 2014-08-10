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

#ifndef BERRYIVIEWDESCRIPTOR_H_
#define BERRYIVIEWDESCRIPTOR_H_

#include <org_blueberry_ui_Export.h>

#include "berryIWorkbenchPartDescriptor.h"
#include "berryIViewPart.h"

#include <berryIConfigurationElement.h>
#include <berryIAdaptable.h>
#include "berryImageDescriptor.h"

#include <string>
#include <vector>

namespace berry
{
/**
 * \ingroup org_blueberry_ui
 *
 * This is a view descriptor. It provides a "description" of a given
 * given view so that the view can later be constructed.
 * <p>
 * The view registry provides facilities to map from an extension
 * to a IViewDescriptor.
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see org.blueberry.ui.IViewRegistry
 */
struct BERRY_UI IViewDescriptor : public IWorkbenchPartDescriptor, public IAdaptable
{
  berryInterfaceMacro(IViewDescriptor, berry);

  ~IViewDescriptor();

  /**
   * Creates an instance of the view defined in the descriptor.
   *
   * @return the view part
   * @throws CoreException thrown if there is a problem creating the part
   */
  virtual IViewPart::Pointer CreateView() = 0;


  virtual std::vector< std::string> GetKeywordReferences() const = 0;

  /**
   * Returns an array of strings that represent
   * view's category path. This array will be used
   * for hierarchical presentation of the
   * view in places like submenus.
   * @return array of category tokens or null if not specified.
   */
  virtual const std::vector<std::string>& GetCategoryPath() const = 0;

  /**
   * Returns the description of this view.
   *
   * @return the description
   */
  virtual std::string GetDescription() const = 0;


  /**
   * Returns the descriptor for the icon to show for this view.
   */
  virtual SmartPointer<ImageDescriptor> GetImageDescriptor() const = 0;


  /**
   * Returns whether this view allows multiple instances.
   *
   * @return whether this view allows multiple instances
   */
  virtual bool GetAllowMultiple() const = 0;

  /**
   * Returns whether this view can be restored upon workbench restart.
   *
   * @return whether whether this view can be restored upon workbench restart
   */
  virtual bool IsRestorable() const = 0;

  virtual bool operator==(const Object*) const = 0;
};
}

#endif /*BERRYIVIEWDESCRIPTOR_H_*/
