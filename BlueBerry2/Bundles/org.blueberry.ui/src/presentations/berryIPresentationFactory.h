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


#ifndef BERRYIPRESENTATIONFACTORY_H_
#define BERRYIPRESENTATIONFACTORY_H_

#include <osgi/framework/Macros.h>

#include "berryStackPresentation.h"

namespace berry
{

/**
 * This is a factory for presentation objects that control the appearance of
 * editors, views and other components in the workbench.
 *
 * @since 3.0
 */
class BERRY_UI IPresentationFactory {

public:

  osgiManifestMacro(IPresentationFactory);

  virtual ~IPresentationFactory() {}

  /**
   * Bit value for the createSash method's 'style' parameter.
   * @since 3.4
   */
  static int SASHTYPE_NORMAL; // = 0;
  /**
   * Bit value for the createSash method's 'style' parameter.
   * @since 3.4
   */
  static int SASHTYPE_FLOATING; // = 1<<1;
  /**
   * Bit value for the createSash method's 'style' parameter.
   * @since 3.4
   */
  static int SASHORIENTATION_HORIZONTAL; // = SWT.HORIZONTAL; // 1<<8
  /**
   * Bit value for the createSash method's 'style' parameter.
   * @since 3.4
   */
  static int SASHORIENTATION_VERTICAL; // = SWT.VERTICAL; // 1<<9

    /**
     * Creates an editor presentation for presenting editors.
     * <p>
     * The presentation creates its controls under the given parent composite.
     * </p>
     *
     * @param parent
     *            the parent composite to use for the presentation's controls
     * @param site
     *            the site used for communication between the presentation and
     *            the workbench
     * @return a newly created part presentation
     */
    virtual StackPresentation::Pointer CreateEditorPresentation(
            void* parent, IStackPresentationSite::Pointer site) = 0;

    /**
     * Creates a stack presentation for presenting regular docked views.
     * <p>
     * The presentation creates its controls under the given parent composite.
     * </p>
     *
     * @param parent
     *            the parent composite to use for the presentation's controls
     * @param site
     *            the site used for communication between the presentation and
     *            the workbench
     * @return a newly created part presentation
     */
    virtual StackPresentation::Pointer CreateViewPresentation(void* parent,
            IStackPresentationSite::Pointer site) = 0;

    /**
     * Creates a standalone stack presentation for presenting a standalone view.
     * A standalone view cannot be docked together with other views. The title
     * of a standalone view may be hidden.
     * <p>
     * The presentation creates its controls under the given parent composite.
     * </p>
     *
     * @param parent
     *            the parent composite to use for the presentation's controls
     * @param site
     *            the site used for communication between the presentation and
     *            the workbench
     * @param showTitle
     *            <code>true</code> to show the title for the view,
     *            <code>false</code> to hide it
     * @return a newly created part presentation
     */
    virtual StackPresentation::Pointer CreateStandaloneViewPresentation(
            void* parent, IStackPresentationSite::Pointer site, bool showTitle) = 0;

    /**
     * Creates the status line manager for the window.
     * Subclasses may override.
     *
     * @return the window's status line manager
     */
//    public IStatusLineManager createStatusLineManager() {
//        return new StatusLineManager();
//    }

    /**
     * Creates the control for the window's status line.
     * Subclasses may override.
     *
     * @param statusLine the window's status line manager
     * @param parent the parent composite
     * @return the window's status line control
     */
//    public Control createStatusLineControl(IStatusLineManager statusLine,
//            Composite parent) {
//        return ((StatusLineManager) statusLine).createControl(parent, SWT.NONE);
//    }

    /**
     * Returns a globally unique identifier for this type of presentation factory. This is used
     * to ensure that one presentation is not restored from mementos saved by a different
     * presentation.
     *
     * @return a globally unique identifier for this type of presentation factory.
     */
    virtual std::string GetId() = 0;

    /**
     * Creates the Sash control that is used to separate view and editor parts.
     *
     * @param parent the parent composite
     * @param style A bit set giving both the 'type' of the desired sash and
     * its orientation (i.e. one 'SASHTYPE' value and one "SASHORIENTATION" value).
     * @return the sash control
     * @since 3.4
     */
    virtual void* CreateSash(void* parent, int style) = 0;
//    {
//      int orientation = style & (SASHORIENTATION_HORIZONTAL
//          | SASHORIENTATION_VERTICAL);
//      Sash sash = new Sash(parent, orientation | SWT.SMOOTH);
//      return sash;
//    }

    /**
     * Returns the size of the Sash control that is used to separate view and editor parts.
     *
     * @param style A bit set giving both the 'type' of the desired sash and
     * its orientation.
     * @return the sash size
     * @since 3.4
     */
    virtual int GetSashSize(int style) = 0;

    /**
     * Applies changes of the current theme to the user interface.
     */
    virtual void UpdateTheme() = 0;
};

}

#endif /* BERRYABSTRACTPRESENTATIONFACTORY_H_ */
