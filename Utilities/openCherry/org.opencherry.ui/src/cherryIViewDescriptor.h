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

#ifndef CHERRYIVIEWDESCRIPTOR_H_
#define CHERRYIVIEWDESCRIPTOR_H_

#include "cherryUiDll.h"

#include "cherryIWorkbenchPartDescriptor.h"
#include "cherryIViewPart.h"

#include <service/cherryIConfigurationElement.h>
#include <cherryIAdaptable.h>

#include <string>
#include <vector>

namespace cherry
{

/**
 * \ingroup org_opencherry_ui
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
 * @see org.opencherry.ui.IViewRegistry
 */
struct CHERRY_UI IViewDescriptor : public IWorkbenchPartDescriptor, public IAdaptable
{

  cherryClassMacro(IViewDescriptor);
  
  /**
   * Creates an instance of the view defined in the descriptor.
   * 
   * @return the view part
   * @throws CoreException thrown if there is a problem creating the part
   */
  virtual IViewPart::Pointer CreateView() = 0;

  /**
   * Returns an array of strings that represent
   * view's category path. This array will be used
   * for hierarchical presentation of the
   * view in places like submenus.
   * @return array of category tokens or null if not specified.
   */
  virtual const std::vector<std::string>& GetCategoryPath() = 0;

  /**
   * Returns the description of this view.
   *
   * @return the description
   */
  virtual std::string GetDescription() = 0;


  /**
   * Returns the descriptor for the icon to show for this view.
   */
  //ImageDescriptor getImageDescriptor();


  /**
   * Returns whether this view allows multiple instances.
   * 
   * @return whether this view allows multiple instances
   */
  virtual bool GetAllowMultiple() = 0;

  virtual bool operator==(const IViewDescriptor*) const = 0;
};

}

#endif /*CHERRYIVIEWDESCRIPTOR_H_*/
