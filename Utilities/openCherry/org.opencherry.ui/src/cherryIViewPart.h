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

#ifndef CHERRYIVIEWPART_H_
#define CHERRYIVIEWPART_H_

#include "cherryUiDll.h"
#include "cherryMacros.h"

#include "cherryIMemento.h"
#include "cherryIWorkbenchPart.h"
#include "cherryIViewSite.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 * 
 * A view is a visual component within a workbench page. It is typically used to
 * navigate a hierarchy of information (like the workspace), open an editor, or
 * display properties for the active editor. Modifications made in a view are
 * saved immediately (in contrast to an editor part, which conforms to a more
 * elaborate open-save-close lifecycle).
 * <p>
 * Only one instance of a particular view type may exist within a workbench
 * page. This policy is designed to simplify part management for a user.
 * </p>
 * <p>
 * This interface may be implemented directly. For convenience, a base
 * implementation is defined in <code>ViewPart</code>.
 * </p>
 * <p>
 * A view is added to the workbench in two steps:
 * <ol>
 * <li>A view extension is contributed to the workbench registry. This
 * extension defines the extension id and extension class.</li>
 * <li>The view is included in the default layout for a perspective.
 * Alternatively, the user may open the view from the Perspective menu.</li>
 * </ol>
 * </p>
 * <p>
 * Views implement the <code>IAdaptable</code> interface; extensions are
 * managed by the platform's adapter manager.
 * </p>
 * <p>
 * As of 3.4, views may optionally adapt to {@link ISizeProvider} if they have
 * a preferred size. The default presentation will make a best effort to
 * allocate the preferred size to a view if it is the only part in a stack. If
 * there is more than one part in the stack, the constraints will be disabled
 * for that stack. The size constraints are adjusted for the size of the tab and
 * border trim. Note that this is considered to be a hint to the presentation,
 * and not all presentations may honor size constraints.
 * </p>
 * 
 * @see IWorkbenchPage#showView
 * @see org.eclipse.ui.part.ViewPart
 * @see ISizeProvider
 */
struct CHERRY_UI IViewPart : public virtual IWorkbenchPart {
  
  cherryClassMacro(IViewPart)
  
  virtual ~IViewPart() {}
  
    /**
     * Returns the site for this view. 
     * This method is equivalent to <code>(IViewSite) getSite()</code>.
     * <p>  
     * The site can be <code>null</code> while the view is being initialized. 
     * After the initialization is complete, this value must be non-<code>null</code>
     * for the remainder of the view's life cycle.
     * </p>
     * 
     * @return the view site; this value may be <code>null</code> if the view
     *         has not yet been initialized
     */
    virtual IViewSite::Pointer GetViewSite() = 0;

   
    /**
     * Initializes this view with the given view site.  A memento is passed to
     * the view which contains a snapshot of the views state from a previous
     * session.  Where possible, the view should try to recreate that state
     * within the part controls.
     * <p>
     * This method is automatically called by the workbench shortly after the part 
     * is instantiated.  It marks the start of the views's lifecycle. Clients must 
     * not call this method.
     * </p>
     *
     * @param site the view site
     * @param memento the IViewPart state or null if there is no previous saved state
     * @exception PartInitException if this view was not initialized successfully
     */
    virtual void Init(IViewSite::Pointer site, IMemento::Pointer memento = 0) = 0;

    /**
     * Saves the object state within a memento.
     *
     * @param memento a memento to receive the object state
     */
    virtual void SaveState(IMemento::Pointer memento) = 0;
};

}

#endif /*CHERRYIVIEWPART_H_*/
