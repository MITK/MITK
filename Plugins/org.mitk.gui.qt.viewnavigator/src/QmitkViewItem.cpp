/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkViewItem.h"

#include <berryPlatform.h>
#include <berryIExtensionRegistry.h>
#include <berryIViewDescriptor.h>

namespace
{
  QStringList GetKeywordsFromView(const berry::IViewDescriptor* view)
  {
    QStringList result;

    const auto* extensionRegistry = berry::Platform::GetExtensionRegistry();
    const auto configElems = extensionRegistry->GetConfigurationElementsFor("org.blueberry.ui.keywords");

    const auto keywordRefs = view->GetKeywordReferences();

    for (const auto& keywordRef : keywordRefs)
    {
      for (const auto& configElem : configElems)
      {
        auto id = configElem->GetAttribute("id");

        if (id != keywordRef)
          continue;

        auto keyword = configElem->GetAttribute("label");

        if (!keyword.isEmpty())
          result.append(keyword);
      }
    }

    return result;
  }
}

QmitkViewItem::QmitkViewItem(const berry::IViewDescriptor* view)
  : QStandardItem(view->GetImageDescriptor(), view->GetLabel())
{
  this->setData(view->GetId());
  this->setToolTip(view->GetDescription());
  this->SetKeywords(GetKeywordsFromView(view));
}

QmitkViewItem::~QmitkViewItem()
{
}

void QmitkViewItem::SetBoldFont(bool enable)
{
  auto font = this->font();
  font.setBold(enable);
  this->setFont(font);
}

QStringList QmitkViewItem::GetKeywords() const
{
  return this->data(KeywordsRole).toStringList();
}

void QmitkViewItem::SetKeywords(const QStringList& keywords)
{
  this->setData(keywords, KeywordsRole);
}

bool QmitkViewItem::Match(const QRegularExpression& re) const
{
  if (this->text().contains(re))
    return true;

  if (this->toolTip().contains(re))
    return true;

  for (const auto& keyword : this->GetKeywords())
  {
    if (keyword.contains(re))
      return true;
  }

  return false;
}
