/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryCommandContributionItemParameter.h"

#include "../services/cherryIServiceLocator.h"
#include "../cherryImageDescriptor.h"

namespace cherry
{

CommandContributionItemParameter::CommandContributionItemParameter(
    IServiceLocator::Pointer serviceLocator, const std::string& id,
    const std::string& commandId, UIElement::Styles style) :
  serviceLocator(serviceLocator), id(id), commandId(commandId), style(style),
      visibleEnabled(true), mode(0)
{
}

CommandContributionItemParameter::CommandContributionItemParameter(
    IServiceLocator::Pointer serviceLocator, const std::string& id,
    const std::string& commandId, const std::map<std::string,
    Object::Pointer>& parameters, SmartPointer<ImageDescriptor> icon, SmartPointer<
        ImageDescriptor> disabledIcon, SmartPointer<ImageDescriptor> hoverIcon,
    const std::string& label, const std::string& mnemonic,
    const std::string& tooltip, UIElement::Styles style,
    const std::string& helpContextId, bool visibleEnabled) :
  serviceLocator(serviceLocator), id(id), commandId(commandId), parameters(
      parameters), icon(icon), disabledIcon(disabledIcon),
      hoverIcon(hoverIcon), label(label), mnemonic(mnemonic), tooltip(tooltip),
      style(style), helpContextId(helpContextId),
      visibleEnabled(visibleEnabled), mode(0)
{
}

CommandContributionItemParameter::CommandContributionItemParameter(
    const CommandContributionItemParameter& p) :
  serviceLocator(p.serviceLocator), id(p.id), commandId(p.commandId),
      parameters(p.parameters), icon(p.icon), disabledIcon(p.disabledIcon),
      hoverIcon(p.hoverIcon), label(p.label), mnemonic(p.mnemonic), tooltip(
          p.tooltip), style(p.style), helpContextId(p.helpContextId),
      iconStyle(p.iconStyle), visibleEnabled(p.visibleEnabled), mode(p.mode)
{

}

CommandContributionItemParameter&
CommandContributionItemParameter::operator=(const CommandContributionItemParameter& p)
{
  serviceLocator = p.serviceLocator;
  id = p.id;
  commandId = p.commandId;
  parameters = p.parameters;
  icon = p.icon;
  disabledIcon = p.disabledIcon;
  hoverIcon = p.hoverIcon;
  label = p.label;
  mnemonic = p.mnemonic;
  tooltip = p.tooltip;
  style = p.style;
  helpContextId = p.helpContextId;
  iconStyle = p.iconStyle;
  visibleEnabled = p.visibleEnabled;
  mode = p.mode;

  return *this;
}

}
