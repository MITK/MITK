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
     * @see AbstractSelectionService#CreatePartTracker(const std::string&)
     */
    AbstractPartSelectionTracker::Pointer CreatePartTracker(const std::string& partId) const;
};

}

#endif /* BERRYPAGESELECTIONSERVICE_H_ */
