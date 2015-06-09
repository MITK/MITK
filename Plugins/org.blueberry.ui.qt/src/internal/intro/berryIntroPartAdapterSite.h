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

    Object* GetService(const QString& key) override;

    bool HasService(const QString& key) const override;

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchPartSite#getId()
     */
    QString GetId() const override;

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchSite#getPage()
     */
    SmartPointer<IWorkbenchPage> GetPage() override;

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchPartSite#getPluginId()
     */
    QString GetPluginId() const override;

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchSite#getSelectionProvider()
     */
    SmartPointer<ISelectionProvider> GetSelectionProvider() override;

    SmartPointer<Shell> GetShell() override;

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchSite#getWorkbenchWindow()
     */
    SmartPointer<IWorkbenchWindow> GetWorkbenchWindow() override;

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchSite#setSelectionProvider(org.eclipse.jface.viewers.ISelectionProvider)
     */
    void SetSelectionProvider(SmartPointer<ISelectionProvider> provider) override;

};

}

#endif /* BERRYVIEWINTROADAPTERSITE_H_ */
