/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIVIEWSITE_H_
#define BERRYIVIEWSITE_H_

#include "berryIWorkbenchPartSite.h"

#include <string>

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * The primary interface between a view part and the workbench.
 * <p>
 * The workbench exposes its implemention of view part sites via this interface,
 * which is not intended to be implemented or extended by clients.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IViewSite : public virtual IWorkbenchPartSite
{

  berryObjectMacro(berry::IViewSite);

  ~IViewSite() override;

    /**
     * Returns the action bars for this part site.
     * Views have exclusive use of their site's action bars.
     *
     * @return the action bars
     */
    //IActionBars getActionBars();

    /**
     * Returns the secondary id for this part site's part,
     * or <code>null</code> if it has none.
     *
     * @see IWorkbenchPage#showView(String, String, int)
     */
    virtual QString GetSecondaryId() = 0;
};

}

#endif /*BERRYIVIEWSITE_H_*/
