/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYICOMMANDIMAGESERVICE_H_
#define BERRYICOMMANDIMAGESERVICE_H_

#include "../services/berryIDisposable.h"

#include <QIcon>

namespace berry {

/**
 * <p>
 * Provides a look-up facility for images associated with commands.
 * </p>
 * <p>
 * The <em>type</em> of an image indicates the state of the associated command
 * within the user interface. The supported types are: <code>TYPE_DEFAULT</code>
 * (to be used for an enabled command), <code>TYPE_DISABLED</code> (to be used
 * for a disabled command) and <code>TYPE_HOVER</code> (to be used for an
 * enabled command over which the mouse is hovering).
 * </p>
 * <p>
 * The <em>style</em> of an image is an arbitrary string used to distinguish
 * between sets of images associated with a command. For example, a command may
 * appear in the menus as the default style. However, in the toolbar, the
 * command is simply the default action for a toolbar drop down item. As such,
 * perhaps a different image style is appropriate. The classic case is the "Run
 * Last Launched" command, which appears in the menu and the toolbar, but with
 * different icons in each location.
 * </p>
 * <p>
 * We currently support a default image style (none) and an image style of
 * IMAGE_STYLE_TOOLBAR.
 * </p>
 *
 * @noimplement This interface is not intended to be implemented by clients.
 * @noextend This interface is not intended to be extended by clients.
 *
 */
struct BERRY_UI_QT ICommandImageService : public IDisposable
{

  berryObjectMacro(berry::ICommandImageService);

  /**
   * The default image style. This is provided when no style is requested or
   * when the requested style is unavailable. (Value is <b>null</b>)
   */
  static const QString IMAGE_STYLE_DEFAULT;

  /**
   * The image style used for commands in a toolbar. This is useful if you
   * want the icon for the command in the toolbar to be different than the one
   * that is displayed with menu items. (Value is <b>toolbar</b>)
   */
  static const QString IMAGE_STYLE_TOOLBAR; // = "toolbar";

  /**
   * Retrieves the image associated with the given command in the
   * default style.
   *
   * @param commandId
   *            The identifier to find; must not be <code>null</code>.
   * @return An image appropriate for the given command; may be
   *         <code>null</code>.
   */
  virtual QIcon GetImage(const QString& commandId) = 0;

  /**
   * Retrieves the image associated with the given command in the
   * given style.
   *
   * @param commandId
   *            The identifier to find; must not be <code>null</code>.
   * @param style
   *            The style of the image to retrieve; may be <code>null</code>.
   * @return An image appropriate for the given command; A null QIcon
   *         if the given image style cannot be found.
   */
  virtual QIcon GetImage(const QString& commandId, const QString& style) = 0;
};

}

Q_DECLARE_INTERFACE(berry::ICommandImageService, "org.blueberry.ui.ICommandImageService")

#endif /* BERRYICOMMANDIMAGESERVICE_H_ */
