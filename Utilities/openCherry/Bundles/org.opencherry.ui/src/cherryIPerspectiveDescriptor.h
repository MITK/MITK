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

#ifndef CHERRYIPERSPECTIVEDESCRIPTOR_H_
#define CHERRYIPERSPECTIVEDESCRIPTOR_H_

#include <cherryMacros.h>
#include <cherryObject.h>

#include "cherryUiDll.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 * A perspective descriptor describes a perspective in an
 * <code>IPerspectiveRegistry</code>.
 * <p>
 * A perspective is a template for view visibility, layout, and action visibility
 * within a workbench page. There are two types of perspective: a predefined
 * perspective and a custom perspective.
 * <ul>
 *   <li>A predefined perspective is defined by an extension to the workbench's
 *     perspective extension point (<code>"org.opencherry.ui.perspectives"</code>).
 *     The extension defines a id, label, and <code>IPerspectiveFactory</code>.
 *     A perspective factory is used to define the initial layout for a page.
 *     </li>
 *   <li>A custom perspective is defined by the user.  In this case a predefined
 *     perspective is modified to suit a particular task and saved as a new
 *     perspective.  The attributes for the perspective are stored in a separate file
 *     in the workbench's metadata directory.
 *     </li>
 * </ul>
 * </p>
 * <p>
 * Within a page the user can open any of the perspectives known
 * to the workbench's perspective registry, typically by selecting one from the
 * workbench's <code>Open Perspective</code> menu. When selected, the views
 * and actions within the active page rearrange to reflect the perspective.
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @see IPerspectiveRegistry
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct CHERRY_UI IPerspectiveDescriptor : public Object {

  cherryInterfaceMacro(IPerspectiveDescriptor, cherry)

  virtual ~IPerspectiveDescriptor() {}

    /**
     * Returns the description of this perspective.
     * This is the value of its <code>"description"</code> attribute.
     *
     * @return the description
     * @since 3.0
     */
    virtual std::string GetDescription() = 0;

    /**
     * Returns this perspective's id. For perspectives declared via an extension,
     * this is the value of its <code>"id"</code> attribute.
     *
     * @return the perspective id
     */
    virtual std::string GetId() = 0;

    /**
     * Returns the descriptor of the image to show for this perspective.
     * If the extension for this perspective specifies an image, the descriptor
     * for it is returned.  Otherwise a default image is returned.
     *
     * @return the descriptor of the image to show for this perspective
     */
    //virtual ImageDescriptor GetImageDescriptor() = 0;

    /**
     * Returns this perspective's label. For perspectives declared via an extension,
     * this is the value of its <code>"label"</code> attribute.
     *
     * @return the label
     */
    virtual std::string GetLabel() = 0;
};

}

#endif /*CHERRYIPERSPECTIVEDESCRIPTOR_H_*/
