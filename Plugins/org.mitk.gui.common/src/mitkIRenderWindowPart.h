/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKIRENDERWINDOWPART_H
#define MITKIRENDERWINDOWPART_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <QtPlugin>

#include <mitkNumericTypes.h>
#include <mitkRenderingManager.h>

#include <org_mitk_gui_common_Export.h>

class QmitkRenderWindow;

namespace mitk {

struct IRenderingManager;
class SliceNavigationController;

/**
 * \ingroup org_mitk_gui_common
 *
 * \brief Interface for a MITK Workbench Part providing a render window.
 *
 * This interface allows generic access to Workbench parts which provide some
 * kind of render window. The interface is intended to be implemented by
 * subclasses of berry::IWorkbenchPart. Usually, the interface is implemented
 * by a Workbench editor.
 *
 * A IRenderWindowPart provides zero or more QmitkRenderWindow instances which can
 * be controlled via this interface. QmitkRenderWindow instances have an associated
 * \e id, which is implementation specific. However, implementations should consider
 * to use one of the following ids for certain QmitkRenderWindow instances to maximize
 * reusability (they are free to map multiple ids to one QmitkRenderWindow internally):
 * <ul>
 * <li>axial</li>
 * <li>sagittal</li>
 * <li>coronal</li>
 * <li>3d</li>
 * </ul>
 *
 * \see ILinkedRenderWindowPart
 * \see IRenderWindowPartListener
 * \see QmitkAbstractRenderEditor
 */
struct MITK_GUI_COMMON_PLUGIN IRenderWindowPart {

  static const QString DECORATION_BORDER; // = "border"
  static const QString DECORATION_LOGO; // = "logo"
  static const QString DECORATION_MENU; // = "menu"
  static const QString DECORATION_BACKGROUND; // = "background;

  virtual ~IRenderWindowPart();

  /**
   * Get the currently active (focused) render window.
   * Focus handling is implementation specific.
   *
   * \return The active QmitkRenderWindow instance; <code>NULL</code>
   *         if no render window is active.
   */
  virtual QmitkRenderWindow* GetActiveQmitkRenderWindow() const = 0;

  /**
   * Get all render windows with their ids.
   *
   * \return A hash map mapping the render window id to the QmitkRenderWindow instance.
   */
  virtual QHash<QString,QmitkRenderWindow*> GetQmitkRenderWindows() const  = 0;

  /**
   * Get a render window with a specific id.
   *
   * \param id The render window id.
   * \return The QmitkRenderWindow instance for <code>id</code>
   */
  virtual QmitkRenderWindow* GetQmitkRenderWindow(const QString& id) const = 0;

  /**
   * Get the rendering manager used by this render window part.
   *
   * \return The current IRenderingManager instance or <code>NULL</code>
   *         if no rendering manager is used.
   */
  virtual mitk::IRenderingManager* GetRenderingManager() const = 0;

  /**
   * Request an update of all render windows.
   *
   * \param requestType Specifies the type of render windows for which an update
   *        will be requested.
   */
  virtual void RequestUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL) = 0;

  /**
   * Force an immediate update of all render windows.
   *
   * \param requestType Specifies the type of render windows for which an immediate update
   *        will be requested.
   */
  virtual void ForceImmediateUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL) = 0;

  /**
   * Get the SliceNavigationController for controlling time positions.
   *
   * \return A SliceNavigationController if the render window supports this
   *         operation; otherwise returns <code>NULL</code>.
   */
  virtual mitk::SliceNavigationController* GetTimeNavigationController() const = 0;

  /**
   * Get the selected position in the render window with id <code>id</code>
   * or in the active render window if <code>id</code> is NULL.
   *
   * \param id The render window id.
   * \return The currently selected position in world coordinates.
   */
  virtual mitk::Point3D GetSelectedPosition(const QString& id = QString()) const = 0;

  /**
   * Set the selected position in the render window with id <code>id</code>
   * or in the active render window if <code>id</code> is NULL.
   *
   * \param pos The position in world coordinates which should be selected.
   * \param id The render window id in which the selection should take place.
   */
  virtual void SetSelectedPosition(const mitk::Point3D& pos, const QString& id = QString()) = 0;

  /**
   * Enable \e decorations like colored borders, menu widgets, logos, text annotations, etc.
   *
   * Decorations are implementation specific. A set of standardized decoration names is listed
   * in GetDecorations().
   *
   * \param enable If <code>true</code> enable the decorations specified in <code>decorations</code>,
   *        otherwise disable them.
   * \param decorations A list of decoration names. If empty, all supported decorations are affected.
   *
   * \see GetDecorations()
   */
  virtual void EnableDecorations(bool enable, const QStringList& decorations = QStringList()) = 0;

  /**
   * Return if a specific decoration is enabled.
   *
   * \return <code>true</code> if the decoration is enabled, <code>false</code> if it is disabled
   *         or unknown.
   *
   * \see GetDecorations()
   */
  virtual bool IsDecorationEnabled(const QString& decoration) const = 0;

  /**
   * Get a list of supported decorations.
   *
   * The following decoration names are standardized and should not be used for other decoration types:
   * <ul>
   * <li>\e DECORATION_BORDER Any border decorations like colored rectangles, etc.
   * <li>\e DECORATION_MENU Menus associated with render windows
   * <li>\e DECORATION_BACKGROUND All kinds of backgrounds (patterns, gradients, etc.) except for solid colored backgrounds
   * <li>\e DECORATION_LOGO Any kind of logo overlayed on the rendered scene
   * </ul>
   *
   * \return A list of supported decoration names.
   */
  virtual QStringList GetDecorations() const = 0;
};

}

Q_DECLARE_INTERFACE(mitk::IRenderWindowPart, "org.mitk.ui.IRenderWindowPart")

#endif // MITKIRENDERWINDOWPART_H
