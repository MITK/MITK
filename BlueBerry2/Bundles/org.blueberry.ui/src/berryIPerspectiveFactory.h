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

#ifndef BERRYIPERSPECTIVEFACTORY_H_
#define BERRYIPERSPECTIVEFACTORY_H_

#include <osgi/framework/Macros.h>

#include "berryUiDll.h"
#include "berryIPageLayout.h"

namespace berry {

/**
 * A perspective factory generates the initial page layout and visible
 * action set for a page.
 * <p>
 * When a new page is created in the workbench a perspective is used to define
 * the initial page layout.  If this is a predefined perspective (based on an extension to
 * the workbench's perspective extension point) an <code>IPerspectiveFactory</code>
 * is used to define the initial page layout.
 * </p><p>
 * The factory for the perspective is created and passed an <code>IPageLayout</code>
 * where views can be added.  The default layout consists of the editor area with no
 * additional views.  Additional views are added to the layout using
 * the editor area as the initial point of reference.  The factory is used only briefly
 * while a new page is created; then discarded.
 * </p><p>
 * To define a perspective clients should implement this interface and
 * include the name of their class in an extension to the workbench's perspective
 * extension point (named <code>"org.blueberry.ui.perspectives"</code>).  For example,
 * the plug-in's XML markup might contain:
 * <pre>
 * &lt;extension point="org.blueberry.ui.perspectives"&gt;
 *   &lt;perspective
 *       id="com.example.plugin.perspective"
 *       name="My Perspective"
 *       class="namespaze::MyPerspective"&gt;
 *   &lt;/perspective&gt;
 * &lt;/extension&gt;
 * </pre>
 * </p><p>
 * Example of populating a page with standard workbench views:
 * <pre>
 * public: void CreateInitialLayout(IPageLayout layout) {
 *    // Get the editor area.
 *    std::string editorArea = layout->GetEditorArea();
 *
 *    // Top left: Resource Navigator view and Bookmarks view placeholder
 *    IFolderLayout::Pointer topLeft = layout->CreateFolder("topLeft", IPageLayout::LEFT, 0.25f,
 *      editorArea);
 *    topLeft->AddView(IPageLayout::ID_RES_NAV);
 *    topLeft->AddPlaceholder(IPageLayout::ID_BOOKMARKS);
 *
 *    // Bottom left: Outline view and Property Sheet view
 *    IFolderLayout::Pointer bottomLeft = layout->CreateFolder("bottomLeft", IPageLayout::BOTTOM, 0.50f,
 *      "topLeft");
 *    bottomLeft->AddView(IPageLayout::ID_OUTLINE);
 *    bottomLeft->AddView(IPageLayout::ID_PROP_SHEET);
 *
 *    // Bottom right: Task List view
 *    layout->AddView(IPageLayout::ID_TASK_LIST, IPageLayout::BOTTOM, 0.66f, editorArea);
 *  }
 * </pre>
 * </p><p>
 * Within the workbench a user may override the visible views, layout and
 * action sets of a predefined perspective to create a custom perspective.  In such cases
 * the layout is persisted by the workbench and the factory is not used.
 * </p>
 */
struct BERRY_UI IPerspectiveFactory : public Object {

  osgiInterfaceMacro(berry::IPerspectiveFactory);

  /**
   * Creates the initial layout for a page.
   * <p>
   * Implementors of this method may add additional views to a
   * perspective.  The perspective already contains an editor folder
   * identified by the result of <code>IPageLayout.getEditorArea()</code>.
   * Additional views should be added to the layout using this value as
   * the initial point of reference.
   * </p>
   *
   * @param layout the page layout
   */
  virtual void CreateInitialLayout(IPageLayout::Pointer layout) = 0;
};

}


#endif /* BERRYIPERSPECTIVEFACTORY_H_ */
