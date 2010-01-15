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


#ifndef CHERRYCOMMANDCONTRIBUTIONITEMPARAMETER_H_
#define CHERRYCOMMANDCONTRIBUTIONITEMPARAMETER_H_

#include "../cherryUiDll.h"

#include <string>
#include <map>

#include "cherryUIElement.h"

namespace cherry {

struct IServiceLocator;
struct ImageDescriptor;

/**
 * A help class for the various parameters that can be used with command
 * contributions. Mandatory parameters are in the constructor, and public fields
 * can be set to fill in other parameters.
 *
 */
struct CHERRY_UI CommandContributionItemParameter {

  /**
   * a service locator that is most appropriate for this contribution.
   * Typically the local {@link IWorkbenchWindow} or
   * {@link IWorkbenchPartSite} will be sufficient. Must not be
   * <code>null</code>.
   */
  SmartPointer<IServiceLocator> serviceLocator;

  /**
   * The id for this item. May be <code>null</code>. Items without an id
   * cannot be referenced later.
   */
  std::string id;

  /**
   * A command id for a defined command. Must not be <code>null</code>.
   */
  std::string commandId;

  /**
   * A map of strings to strings which represent parameter names to values.
   * The parameter names must match those in the command definition. May be
   * empty.
   */
  std::map<std::string, Object::Pointer> parameters;

  /**
   * An icon for this item. May be <code>null</code>.
   */
  SmartPointer<ImageDescriptor> icon;

  /**
   * A disabled icon for this item. May be <code>null</code>.
   */
  SmartPointer<ImageDescriptor> disabledIcon;

  /**
   * A hover icon for this item. May be <code>null</code>.
   */
  SmartPointer<ImageDescriptor> hoverIcon;

  /**
   * A label for this item. May be <code>null</code>.
   */
  std::string label;

  /**
   * A mnemonic for this item to be applied to the label. May be
   * <code>null</code>.
   */
  std::string mnemonic;

  /**
   * A tooltip for this item. May be <code>null</code>. Tooltips are
   * currently only valid for toolbar contributions.
   */
  std::string tooltip;

  /**
   * The style of this menu contribution. See the CommandContributionItem
   * STYLE_* contants.
   */
  UIElement::Styles style;

  /**
   * The help context id to be applied to this contribution. May be
   * <code>null</code>
   */
  std::string helpContextId;

  /**
   * The icon style to use. May be <code>null</code> for default style.
   *
   * @see org.eclipse.ui.commands.ICommandImageService
   */
  std::string iconStyle;

  /**
   * The visibility tracking for a menu contribution.
   */
  bool visibleEnabled;

  /**
   * Any number of mode bits, like
   * {@link CommandContributionItem#MODE_FORCE_TEXT}.
   */
  int mode;

  /**
   * Create the parameter object. Nullable attributes can be set directly.
   *
   * @param serviceLocator
   *            a service locator that is most appropriate for this
   *            contribution. Typically the local {@link IWorkbenchWindow} or
   *            {@link IWorkbenchPartSite} will be sufficient. Must not be
   *            <code>null</code>.
   * @param id
   *            The id for this item. May be <code>null</code>. Items
   *            without an id cannot be referenced later.
   * @param commandId
   *            A command id for a defined command. Must not be
   *            <code>null</code>.
   * @param style
   *            The style of this menu contribution. See the STYLE_* contants.
   */
  CommandContributionItemParameter(SmartPointer<IServiceLocator> serviceLocator,
      const std::string& id, const std::string& commandId, UIElement::Styles style);

  /**
   * Build the parameter object.
   * <p>
   * <b>Note:</b> This method should not be called outside the framework.
   * </p>
   *
   * @param serviceLocator
   *            a service locator that is most appropriate for this
   *            contribution. Typically the local {@link IWorkbenchWindow} or
   *            {@link IWorkbenchPartSite} will be sufficient. Must not be
   *            <code>null</code>.
   * @param id
   *            The id for this item. May be <code>null</code>. Items
   *            without an id cannot be referenced later.
   * @param commandId
   *            A command id for a defined command. Must not be
   *            <code>null</code>.
   * @param parameters
   *            A map of strings to strings which represent parameter names to
   *            values. The parameter names must match those in the command
   *            definition. May be <code>null</code>
   * @param icon
   *            An icon for this item. May be <code>null</code>.
   * @param disabledIcon
   *            A disabled icon for this item. May be <code>null</code>.
   * @param hoverIcon
   *            A hover icon for this item. May be <code>null</code>.
   * @param label
   *            A label for this item. May be <code>null</code>.
   * @param mnemonic
   *            A mnemonic for this item to be applied to the label. May be
   *            <code>null</code>.
   * @param tooltip
   *            A tooltip for this item. May be <code>null</code>. Tooltips
   *            are currently only valid for toolbar contributions.
   * @param style
   *            The style of this menu contribution. See the STYLE_* contants.
   * @param helpContextId
   *            the help context id to be applied to this contribution. May be
   *            <code>null</code>
   * @param visibleEnabled
   *            Visibility tracking for the menu contribution.
   */
  CommandContributionItemParameter(
      SmartPointer<IServiceLocator> serviceLocator,
      const std::string& id,
      const std::string& commandId,
      const std::map<std::string, Object::Pointer>& parameters,
      SmartPointer<ImageDescriptor> icon,
      SmartPointer<ImageDescriptor> disabledIcon,
      SmartPointer<ImageDescriptor> hoverIcon,
      const std::string& label,
      const std::string& mnemonic,
      const std::string& tooltip,
      UIElement::Styles style,
      const std::string& helpContextId,
      bool visibleEnabled);

  CommandContributionItemParameter(const CommandContributionItemParameter& p);

  CommandContributionItemParameter& operator=(const CommandContributionItemParameter& p);
};

}

#endif /* CHERRYCOMMANDCONTRIBUTIONITEMPARAMETER_H_ */
