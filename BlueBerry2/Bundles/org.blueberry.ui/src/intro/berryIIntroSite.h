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
 * @since 3.0
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI IIntroSite : public IWorkbenchSite {

  berryInterfaceMacro(IIntroSite, berry)

  /**
   * Returns the part registry extension id for this intro site's part.
   * <p>
   * The name comes from the <code>id</code> attribute in the configuration
   * element.
   * </p>
   *
   * @return the registry extension id
   */
  virtual std::string GetId() const = 0;

  /**
   * Returns the unique identifier of the plug-in that defines this intro
   * site's part.
   *
   * @return the unique identifier of the declaring plug-in
   * @see org.eclipse.core.runtime.IPluginDescriptor#getUniqueIdentifier()
   */
  virtual std::string GetPluginId() const = 0;

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
