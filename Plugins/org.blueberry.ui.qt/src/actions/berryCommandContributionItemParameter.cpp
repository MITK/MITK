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
