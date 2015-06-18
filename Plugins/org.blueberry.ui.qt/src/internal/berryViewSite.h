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

#ifndef BERRYVIEWSITE_H_
#define BERRYVIEWSITE_H_

#include "berryPartSite.h"

#include "berryIViewSite.h"

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

  berryObjectMacro(ViewSite);

  ViewSite(SmartPointer<IViewReference> ref, SmartPointer<IViewPart> view,
           WorkbenchPage* page, const QString& id, const QString& pluginId,
           const QString& registeredName);

    /**
     * Creates a new ViewSite.
     */
    ViewSite(SmartPointer<IViewReference> ref, SmartPointer<IViewPart> view, WorkbenchPage* page,
        SmartPointer<IViewDescriptor> desc);

    /**
     * Returns the secondary id or <code>null</code>.
     */
    QString GetSecondaryId() override;

    /**
     * Returns the view.
     */
    SmartPointer<IViewPart> GetViewPart();
};

}

#endif /*BERRYVIEWSITE_H_*/
