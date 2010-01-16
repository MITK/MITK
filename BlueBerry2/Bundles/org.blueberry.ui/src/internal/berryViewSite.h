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

#ifndef BERRYVIEWSITE_H_
#define BERRYVIEWSITE_H_

#include "berryPartSite.h"

#include "../berryIViewSite.h"

namespace berry {

class WorkbenchPage;
struct IViewReference;
struct IViewPart;
struct IViewDescriptor;

/**
 * \ingroup org_blueberry_ui_internal
 * 
 * A view container manages the services for a view.
 */
class ViewSite : public PartSite, public IViewSite {
    
public:
  
  osgiObjectMacro(ViewSite)
  
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

#endif /*BERRYVIEWSITE_H_*/
