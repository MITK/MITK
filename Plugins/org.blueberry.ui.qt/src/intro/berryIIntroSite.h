/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIINTROSITE_H_
#define BERRYIINTROSITE_H_

#include <berryIWorkbenchSite.h>

namespace berry {

/**
 * The primary interface between an intro part and the workbench.
 * <p>
 * The workbench exposes its implemention of intro part sites via this
 * interface, which is not intended to be implemented or extended by clients.
 * </p>
 *
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IIntroSite : public IWorkbenchSite
{

  berryObjectMacro(berry::IIntroSite);

  ~IIntroSite() override;

  /**
   * Returns the part registry extension id for this intro site's part.
   * <p>
   * The name comes from the <code>id</code> attribute in the configuration
   * element.
   * </p>
   *
   * @return the registry extension id
   */
  virtual QString GetId() const = 0;

  /**
   * Returns the unique identifier of the plug-in that defines this intro
   * site's part.
   *
   * @return the unique identifier of the declaring plug-in
   * @see org.eclipse.core.runtime.IPluginDescriptor#getUniqueIdentifier()
   */
  virtual QString GetPluginId() const = 0;

  /**
   * Returns the action bars for this part site.
   * The intro part has exclusive use of its site's action bars.
   *
   * @return the action bars
   */
  //virtual IActionBars GetActionBars() const = 0;
};

}

#endif /* BERRYIINTROSITE_H_ */
