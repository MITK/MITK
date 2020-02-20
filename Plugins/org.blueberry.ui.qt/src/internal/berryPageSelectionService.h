/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYPAGESELECTIONSERVICE_H_
#define BERRYPAGESELECTIONSERVICE_H_

#include "berryAbstractSelectionService.h"

namespace berry {

struct IWorkbenchPage;

/**
 * The selection service for a page.
 */
class PageSelectionService : public AbstractSelectionService
{

private:

  IWorkbenchPage* page;

  /**
   * Sets the page.
   */
  void SetPage(IWorkbenchPage* page);

public:

    /**
     * Creates a new selection service for a specific workbench page.
     */
    PageSelectionService(IWorkbenchPage* page);

protected:

    /**
     * Returns the page.
     */
    IWorkbenchPage* GetPage() const;

    /*
     * @see AbstractSelectionService#CreatePartTracker(const QString&)
     */
    AbstractPartSelectionTracker::Pointer CreatePartTracker(const QString& partId) const override;
};

}

#endif /* BERRYPAGESELECTIONSERVICE_H_ */
