/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYICOMMANDIMAGESERVICE_H_
#define BERRYICOMMANDIMAGESERVICE_H_

#include "../services/berryIDisposable.h"

namespace berry {

using namespace osgi::framework;

class ImageDescriptor;

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
 * @since 3.4
 */
struct BERRY_UI ICommandImageService : public IDisposable {

  osgiInterfaceMacro(berry::ICommandImageService)

  enum Type {
    TYPE_DEFAULT, /// The type of image to display in the default case.
    TYPE_DISABLED, /// The type of image to display if the corresponding command is disabled.
    TYPE_HOVER /** The type of image to display if the mouse is hovering over the command
                 * and the command is enabled. **/
  };

  /**
   * The default image style. This is provided when no style is requested or
   * when the requested style is unavailable. (Value is <b>null</b>)
   */
  static const std::string IMAGE_STYLE_DEFAULT;

  /**
   * The image style used for commands in a toolbar. This is useful if you
   * want the icon for the command in the toolbar to be different than the one
   * that is displayed with menu items. (Value is <b>toolbar</b>)
   */
  static const std::string IMAGE_STYLE_TOOLBAR; // = "toolbar";

  /**
   * Retrieves the default image associated with the given command in the
   * default style.
   *
   * @param commandId
   *            The identifier to find; must not be <code>null</code>.
   * @return An image appropriate for the given command; may be
   *         <code>null</code>.
   */
  virtual SmartPointer<ImageDescriptor> GetImageDescriptor(const std::string& commandId) = 0;

  /**
   * Retrieves the image of the given type associated with the given command
   * in the default style.
   *
   * @param commandId
   *            The identifier to find; must not be <code>null</code>.
   *
   * @param type
   *            The type of image to retrieve. This value must be one of the
   *            <code>TYPE</code> constants defined in this interface.
   * @return An image appropriate for the given command; <code>null</code>
   *         if the given image type cannot be found.
   */
  virtual SmartPointer<ImageDescriptor> GetImageDescriptor(const std::string& commandId, Type type) = 0;

  /**
   * Retrieves the image of the given type associated with the given command
   * in the given style.
   *
   * @param commandId
   *            The identifier to find; must not be <code>null</code>.
   * @param type
   *            The type of image to retrieve. This value must be one of the
   *            <code>TYPE</code> constants defined in this interface.
   * @param style
   *            The style of the image to retrieve; may be <code>null</code>.
   * @return An image appropriate for the given command; <code>null</code>
   *         if the given image style and type cannot be found.
   */
  virtual SmartPointer<ImageDescriptor> GetImageDescriptor(const std::string& commandId,
      Type type, const std::string& style) = 0;

  /**
   * Retrieves the default image associated with the given command in the
   * given style.
   *
   * @param commandId
   *            The identifier to find; must not be <code>null</code>.
   * @param style
   *            The style of the image to retrieve; may be <code>null</code>.
   * @return An image appropriate for the given command; <code>null</code>
   *         if the given image style cannot be found.
   */
  virtual SmartPointer<ImageDescriptor> GetImageDescriptor(const std::string& commandId, const std::string& style) = 0;
};

}

#endif /* BERRYICOMMANDIMAGESERVICE_H_ */
