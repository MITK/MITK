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

#ifndef CHERRYVIEWSITE_H_
#define CHERRYVIEWSITE_H_

#include "cherryPartSite.h"

#include "../cherryIViewSite.h"

namespace cherry {

class WorkbenchPage;
struct IViewReference;
struct IViewPart;
struct IViewDescriptor;

/**
 * \ingroup org_opencherry_ui_internal
 * 
 * A view container manages the services for a view.
 */
class ViewSite : public PartSite, public IViewSite {
    
public:
  
  cherryObjectMacro(ViewSite)
  
  ViewSite(SmartPointer<IViewReference> ref, SmartPointer<IViewPart> view, 
           SmartPointer<WorkbenchPage> page,
            const std::string& id, const std::string& pluginId, const std::string& registeredName);
    
    /**
     * Creates a new ViewSite.
     */
    ViewSite(SmartPointer<IViewReference> ref, SmartPointer<IViewPart> view, SmartPointer<WorkbenchPage> page,
        SmartPointer<IViewDescriptor> desc);

    /**
     * Returns the secondary id or <code>null</code>.
     */
    std::string GetSecondaryId();

    /**
     * Returns the view.
     */
    SmartPointer<IViewPart> GetViewPart();
};

}

#endif /*CHERRYVIEWSITE_H_*/
