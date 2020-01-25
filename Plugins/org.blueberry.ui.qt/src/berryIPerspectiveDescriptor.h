/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIPERSPECTIVEDESCRIPTOR_H_
#define BERRYIPERSPECTIVEDESCRIPTOR_H_

#include <berryMacros.h>
#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * A perspective descriptor describes a perspective in an
 * <code>IPerspectiveRegistry</code>.
 * <p>
 * A perspective is a template for view visibility, layout, and action visibility
 * within a workbench page. There are two types of perspective: a predefined
 * perspective and a custom perspective.
 * <ul>
 *   <li>A predefined perspective is defined by an extension to the workbench's
 *     perspective extension point (<code>"org.blueberry.ui.perspectives"</code>).
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
struct BERRY_UI_QT IPerspectiveDescriptor : public virtual Object
{

  berryObjectMacro(berry::IPerspectiveDescriptor);

  ~IPerspectiveDescriptor() override;

    /**
     * Returns the description of this perspective.
     * This is the value of its <code>"description"</code> attribute.
     *
     * @return the description
     * @since 3.0
     */
    virtual QString GetDescription() const = 0;

    /**
     * Returns this perspective's id. For perspectives declared via an extension,
     * this is the value of its <code>"id"</code> attribute.
     *
     * @return the perspective id
     */
    virtual QString GetId() const = 0;

    /**
     * Returns the descriptor of the image to show for this perspective.
     * If the extension for this perspective specifies an image, the descriptor
     * for it is returned.  Otherwise a default image is returned.
     *
     * @return the descriptor of the image to show for this perspective
     */
    virtual QIcon GetImageDescriptor() const = 0;

    /**
     * Returns this perspective's label. For perspectives declared via an extension,
     * this is the value of its <code>"label"</code> attribute.
     *
     * @return the label
     */
    virtual QString GetLabel() const = 0;

    /**
     * Returns <code>true</code> if this perspective is predefined by an
     * extension.
     *
     * @return boolean whether this perspective is predefined by an extension
     */
    //virtual bool IsPredefined() const = 0;

    /**
     * Return the category path of this descriptor
     *
     * @return the category path of this descriptor
     */
    virtual QStringList GetCategoryPath() const = 0;

  /**
   * Returns a list of ids belonging to keyword reference extensions.
   *
   * The keywords listed in each referenced id can be used to filter
   * this perspective.
   *
   * @return A list of ids for keyword reference extensions.
   */
  virtual QStringList GetKeywordReferences() const = 0;
};

}

#endif /*BERRYIPERSPECTIVEDESCRIPTOR_H_*/
