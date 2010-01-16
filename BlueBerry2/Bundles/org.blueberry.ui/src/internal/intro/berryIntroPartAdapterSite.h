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


#ifndef BERRYVIEWINTROADAPTERSITE_H_
#define BERRYVIEWINTROADAPTERSITE_H_

#include <intro/berryIIntroSite.h>

#include "berryIntroDescriptor.h"

#include <berryIWorkbenchPartSite.h>

namespace berry {

/**
 * Simple <code>IIntroSite</code> that wraps a <code>IViewSite</code>.  For use in conjunction with
 * <code>ViewIntroAdapterPart</code>.
 *
 */
class IntroPartAdapterSite : public IIntroSite {

private:

  IntroDescriptor::Pointer descriptor;

  IWorkbenchPartSite::Pointer partSite;

public:

    IntroPartAdapterSite(IWorkbenchPartSite::Pointer viewSite, IntroDescriptor::Pointer descriptor);

    /* (non-Javadoc)
     * @see org.eclipse.ui.intro.IIntroSite#getActionBars()
     */
//    IActionBars getActionBars() {
//        return viewSite.getActionBars();
//    }

    /* (non-Javadoc)
     * @see org.eclipse.core.runtime.IAdaptable#getAdapter(java.lang.Class)
     */
//    Object getAdapter(Class adapter) {
//        return viewSite.getAdapter(adapter);
//    }

    Object::Pointer GetService(const std::string& key);

    bool HasService(const std::string& key) const;

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchPartSite#getId()
     */
    std::string GetId() const;

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchSite#getPage()
     */
    SmartPointer<IWorkbenchPage> GetPage();

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchPartSite#getPluginId()
     */
    std::string GetPluginId() const;

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchSite#getSelectionProvider()
     */
    SmartPointer<ISelectionProvider> GetSelectionProvider();

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchSite#getWorkbenchWindow()
     */
    SmartPointer<IWorkbenchWindow> GetWorkbenchWindow();

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchSite#setSelectionProvider(org.eclipse.jface.viewers.ISelectionProvider)
     */
    void SetSelectionProvider(SmartPointer<ISelectionProvider> provider);

};

}

#endif /* BERRYVIEWINTROADAPTERSITE_H_ */
