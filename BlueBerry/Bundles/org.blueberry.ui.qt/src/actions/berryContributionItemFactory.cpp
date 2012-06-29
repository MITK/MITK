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


#include "berryContributionItemFactory.h"

#include <berryIContributionItem.h>
#include <berryIWorkbenchWindow.h>

#include "internal/berryShowViewMenu.h"

namespace berry {

ContributionItemFactory::ContributionItemFactory(const QString& contributionItemId)
  : contributionItemId(contributionItemId)
{}

QString ContributionItemFactory::GetId() const
{
  return contributionItemId;
}

class ViewsShortlistFactory : public ContributionItemFactory
{
public:

  ViewsShortlistFactory()
    : ContributionItemFactory("viewsShortlist")
  {}

  IContributionItem::Pointer Create(IWorkbenchWindow* window)
  {
    if (window == 0)
    {
      throw std::invalid_argument("window must not be null");
    }
    IContributionItem::Pointer item(new ShowViewMenu(window, GetId()));
    return item;
  }
};

ContributionItemFactory* const ContributionItemFactory::VIEWS_SHORTLIST = new ViewsShortlistFactory();

}
