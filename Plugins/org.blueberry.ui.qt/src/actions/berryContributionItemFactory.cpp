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

#include "internal/berryChangeToPerspectiveMenu.h"
#include "internal/berryShowViewMenu.h"
#include "internal/berryReopenEditorMenu.h"
#include "internal/berrySwitchToWindowMenu.h"

namespace berry {

ContributionItemFactory::ContributionItemFactory(const QString& contributionItemId)
  : contributionItemId(contributionItemId)
{}

QString ContributionItemFactory::GetId() const
{
  return contributionItemId;
}

class OpenWindowsFactory : public ContributionItemFactory
{
public:

  OpenWindowsFactory()
    : ContributionItemFactory("openWindows")
  {}

  IContributionItem::Pointer Create(IWorkbenchWindow *window)
  {
    if (window == nullptr)
    {
      throw ctkInvalidArgumentException("window must not be null");
    }
    IContributionItem::Pointer item(new SwitchToWindowMenu(window, GetId(), true));
    return item;
  }
};

const QScopedPointer<ContributionItemFactory>
ContributionItemFactory::OPEN_WINDOWS(new OpenWindowsFactory());

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
      throw ctkInvalidArgumentException("window must not be null");
    }
    IContributionItem::Pointer item(new ShowViewMenu(window, GetId()));
    return item;
  }
};

const QScopedPointer<ContributionItemFactory>
ContributionItemFactory::VIEWS_SHORTLIST(new ViewsShortlistFactory());

class ReopenEditorsFactory : public ContributionItemFactory
{
public:

  ReopenEditorsFactory()
    : ContributionItemFactory("reopenEditors")
  {}

  IContributionItem::Pointer Create(IWorkbenchWindow* window)
  {
    if (window == nullptr)
    {
      throw ctkInvalidArgumentException("window must not be null");
    }
    IContributionItem::Pointer item(new ReopenEditorMenu(window, GetId(), true));
    return item;
  }
};

const QScopedPointer<ContributionItemFactory>
ContributionItemFactory::REOPEN_EDITORS(new ReopenEditorsFactory());

class PerspectivesShortlistFactory : public ContributionItemFactory
{
public:

  PerspectivesShortlistFactory()
    : ContributionItemFactory("perspectivesShortlist")
  {}

  IContributionItem::Pointer Create(IWorkbenchWindow* window)
  {
    if (window == nullptr)
    {
      throw ctkInvalidArgumentException("window must not be null");
    }
    IContributionItem::Pointer item(new ChangeToPerspectiveMenu(window, GetId()));
    return item;
  }
};

const QScopedPointer<ContributionItemFactory>
ContributionItemFactory::PERSPECTIVES_SHORTLIST(new PerspectivesShortlistFactory());

}
