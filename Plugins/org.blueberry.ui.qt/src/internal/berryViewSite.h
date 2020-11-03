/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
