/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIVIEWDESCRIPTOR_H_
#define BERRYIVIEWDESCRIPTOR_H_

#include <org_blueberry_ui_qt_Export.h>

#include "berryIWorkbenchPartDescriptor.h"
#include "berryIViewPart.h"

#include <berryIConfigurationElement.h>
#include <berryIAdaptable.h>

namespace berry
{

/**
 * \ingroup org_blueberry_ui_qt
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
struct BERRY_UI_QT IViewDescriptor : public IWorkbenchPartDescriptor, public IAdaptable
{

  berryObjectMacro(berry::IViewDescriptor);

  ~IViewDescriptor() override;

  /**
   * Creates an instance of the view defined in the descriptor.
   *
   * @return the view part
   * @throws CoreException thrown if there is a problem creating the part
   */
  virtual IViewPart::Pointer CreateView() = 0;

  /**
   * Returns a list of ids belonging to keyword reference extensions.
   *
   * The keywords listed in each referenced id can be used to filter
   * this view.
   *
   * @return A list of ids for keyword reference extensions.
   */
  virtual QStringList GetKeywordReferences() const = 0;

  /**
   * Returns an array of strings that represent
   * view's category path. This array will be used
   * for hierarchical presentation of the
   * view in places like submenus.
   * @return array of category tokens or null if not specified.
   */
  virtual QStringList GetCategoryPath() const = 0;

  /**
   * Returns the description of this view.
   *
   * @return the description
   */
  virtual QString GetDescription() const = 0;


  /**
   * Returns the descriptor for the icon to show for this view.
   */
  QIcon GetImageDescriptor() const override = 0;


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

  bool operator==(const Object*) const override = 0;
};

}

#endif /*BERRYIVIEWDESCRIPTOR_H_*/
