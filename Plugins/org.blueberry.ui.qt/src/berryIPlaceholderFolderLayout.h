/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIPLACEHOLDERFOLDERLAYOUT_H_
#define BERRYIPLACEHOLDERFOLDERLAYOUT_H_

#include <berryMacros.h>
#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * An <code>IPlaceholderFolderLayout</code> is used to define the initial
 * view placeholders within a folder.
 * The folder itself is contained within an <code>IPageLayout</code>.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see IPageLayout#createPlaceholderFolder
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IPlaceholderFolderLayout : public Object
{

  berryObjectMacro(berry::IPlaceholderFolderLayout);

  ~IPlaceholderFolderLayout() override;

    /**
     * Adds a view placeholder to this folder.
     * A view placeholder is used to define the position of a view before the view
     * appears.  Initially, it is invisible; however, if the user ever opens a view
     * whose compound id matches the placeholder, the view will appear at the same
     * location as the placeholder.
     * See the {@link IPageLayout} type documentation for more details about compound ids.
     * If the placeholder contains wildcards, it remains in the layout, otherwise
     * it is replaced by the view.
     * If the primary id of the placeholder has no wildcards, it must refer to a view
     * contributed to the workbench's view extension point
     * (named <code>"org.blueberry.ui.views"</code>).
     *
     * @param viewId the compound view id (wildcards allowed)
     */
    virtual void AddPlaceholder(const QString& viewId) = 0;

    /**
   * Returns the property with the given id or <code>null</code>. Folder
   * properties are an extensible mechanism for perspective authors to
   * customize the appearance of view stacks. The list of customizable
   * properties is determined by the presentation factory.
   *
   * @param id
   *            Must not be <code>null</code>.
   * @return property value, or <code>null</code> if the property is not
   *         set.
   * @since 3.3
   */
  virtual QString GetProperty(const QString& id) = 0;

    /**
   * Sets the given property to the given value. Folder properties are an
   * extensible mechanism for perspective authors to customize the appearance
   * of view stacks. The list of customizable properties is determined by the
   * presentation factory.
   * <p>
   * These folder properties are intended to be set during
   * <code>IPerspectiveFactory#createInitialLayout</code>. Any subsequent
   * changes to property values after this method completes will not fire
   * change notifications and will not be reflected in the presentation.
   * </p>
   *
   * @param id
   *            property id. Must not be <code>null</code>.
   * @param value
   *            property value. <code>null</code> will clear the property.
   * @since 3.3
   */
  virtual void SetProperty(const QString& id, const QString& value) = 0;
};

}

#endif /*BERRYIPLACEHOLDERFOLDERLAYOUT_H_*/
