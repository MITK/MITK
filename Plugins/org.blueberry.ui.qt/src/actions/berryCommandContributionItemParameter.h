/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYCOMMANDCONTRIBUTIONITEMPARAMETER_H
#define BERRYCOMMANDCONTRIBUTIONITEMPARAMETER_H

#include <QHash>
#include <QIcon>
#include <QKeySequence>

#include "berryCommandContributionItem.h"

namespace berry {

struct IServiceLocator;

/**
 * A help class for the various parameters that can be used with command
 * contributions. Mandatory parameters are in the constructor, and public fields
 * can be set to fill in other parameters.
 */
class BERRY_UI_QT CommandContributionItemParameter : public virtual Object
{

public:

  berryObjectMacro(CommandContributionItemParameter);

  /**
   * a service locator that is most appropriate for this contribution.
   * Typically the local {@link IWorkbenchWindow} or
   * {@link IWorkbenchPartSite} will be sufficient. Must not be
   * <code>null</code>.
   */
  IServiceLocator* serviceLocator;

  /**
   * The id for this item. May be <code>null</code>. Items without an id
   * cannot be referenced later.
   */
  QString id;

  /**
   * A command id for a defined command. Must not be <code>null</code>.
   */
  QString commandId;

  /**
   * A map of strings to strings which represent parameter names to values.
   * The parameter names must match those in the command definition. May be
   * <code>null</code>
   */
  QHash<QString, Object::Pointer> parameters;

  /**
   * An icon for this item. May be <code>null</code>.
   */
  QIcon icon;

  /**
   * A label for this item. May be <code>null</code>.
   */
  QString label;

  /**
   * A mnemonic for this item to be applied to the label. May be
   * <code>null</code>.
   */
  QChar mnemonic;

  /**
   * A shortcut key sequence. This is a workaround and will be
   * removed when key binding support is fully implemented
   */
  QKeySequence shortcut;

  /**
   * A tooltip for this item. May be <code>null</code>. Tooltips are
   * currently only valid for toolbar contributions.
   */
  QString tooltip;

  /**
   * The style of this menu contribution. See the CommandContributionItem
   * STYLE_* contants.
   */
  CommandContributionItem::Style style;

  /**
   * The help context id to be applied to this contribution. May be
   * <code>null</code>
   */
  QString helpContextId;

  /**
   * The icon style to use.
   */
  QString iconStyle;

  /**
   * The visibility tracking for a menu contribution.
   */
  bool visibleEnabled;

  /**
   * Any number of mode bits, like
   * {@link CommandContributionItem#MODE_FORCE_TEXT}.
   */
  CommandContributionItem::Modes mode;

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
  CommandContributionItemParameter(IServiceLocator* serviceLocator,
                                   const QString& id, const QString& commandId,
                                   CommandContributionItem::Style style);

  /**
   * Build the parameter object.
   * <p>
   * <b>Note:</b> This constructor should not be called outside the framework.
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
   * @noreference This constructor is not intended to be referenced by clients.
   */
  CommandContributionItemParameter(IServiceLocator* serviceLocator,
                                   const QString& id, const QString& commandId,
                                   const QHash<QString, Object::Pointer> &parameters,
                                   const QIcon& icon, const QString label,
                                   const QChar &mnemonic, const QString& tooltip,
                                   CommandContributionItem::Style style, const QString& helpContextId,
                                   bool visibleEnabled);
};

}

#endif // BERRYCOMMANDCONTRIBUTIONITEM_H
