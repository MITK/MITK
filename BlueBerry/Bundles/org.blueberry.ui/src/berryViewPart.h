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

#ifndef BERRYVIEWPART_H_
#define BERRYVIEWPART_H_

#include "berryIViewPart.h"
#include "berryIViewSite.h"
#include "berryWorkbenchPart.h"

#include "berryIMemento.h"


namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * Abstract base implementation of all workbench views.
 * <p>
 * This class should be subclassed by clients wishing to define new views.
 * The name of the subclass should be given as the <code>"class"</code>
 * attribute in a <code>view</code> extension contributed to the workbench's
 * view extension point (named <code>"org.blueberry.ui.views"</code>).
 * For example, the plug-in's XML markup might contain:
 * <pre>
 * &LT;extension point="org.blueberry.ui.views"&GT;
 *      &LT;view id="com.example.myplugin.view"
 *         name="My View"
 *         class="com.example.myplugin.MyView"
 *         icon="images/eview.gif"
 *      /&GT;
 * &LT;/extension&GT;
 * </pre>
 * where <code>com.example.myplugin.MyView</code> is the name of the
 * <code>ViewPart</code> subclass.
 * </p>
 * <p>
 * Subclasses must implement the following methods:
 * <ul>
 *   <li><code>createPartControl</code> - to create the view's controls </li>
 *   <li><code>setFocus</code> - to accept focus</li>
 * </ul>
 * </p>
 * <p>
 * Subclasses may extend or reimplement the following methods as required:
 * <ul>
 *   <li><code>setInitializationData</code> - extend to provide additional
 *       initialization when view extension is instantiated</li>
 *   <li><code>init(IWorkbenchPartSite)</code> - extend to provide additional
 *       initialization when view is assigned its site</li>
 *   <li><code>dispose</code> - extend to provide additional cleanup</li>
 *   <li><code>getAdapter</code> - reimplement to make their view adaptable</li>
 * </ul>
 * </p>
 */
class BERRY_UI ViewPart : public WorkbenchPart, public IViewPart
{

protected:
  ViewPart();

  /**
   * Checks that the given site is valid for this type of part.
   * The site for a view must be an <code>IViewSite</code>.
   *
   * @param site the site to check
   * @since 3.1
   */
  void CheckSite(IWorkbenchPartSite::Pointer site);

public:

  berryObjectMacro(ViewPart);

  void Init(IViewSite::Pointer site, IMemento::Pointer memento = IMemento::Pointer(0));

  void SaveState(IMemento::Pointer memento);

  /*
   * Method declared on IViewPart.
   */
  IViewSite::Pointer GetViewSite();

};

}  // namespace berry

#endif /*BERRYVIEWPART_H_*/
