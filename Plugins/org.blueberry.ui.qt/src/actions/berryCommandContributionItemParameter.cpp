/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryCommandContributionItemParameter.h"

namespace berry {

CommandContributionItemParameter::CommandContributionItemParameter(IServiceLocator* serviceLocator,
                                                                   const QString& id, const QString& commandId,
                                                                   CommandContributionItem::Style style)
  : serviceLocator(serviceLocator), id(id), commandId(commandId), style(style), visibleEnabled(false)
{
}

CommandContributionItemParameter::CommandContributionItemParameter(IServiceLocator* serviceLocator,
                                 const QString& id, const QString& commandId,
                                 const QHash<QString,Object::Pointer>& parameters,
                                 const QIcon& icon, const QString label,
                                 const QChar& mnemonic, const QString& tooltip,
                                 CommandContributionItem::Style style, const QString& helpContextId,
                                 bool visibleEnabled)
  : serviceLocator(serviceLocator)
  , id(id)
  , commandId(commandId)
  , parameters(parameters)
  , icon(icon)
  , label(label)
  , mnemonic(mnemonic)
  , tooltip(tooltip)
  , style(style)
  , helpContextId(helpContextId)
  , visibleEnabled(visibleEnabled)
{
}

}
