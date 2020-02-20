/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPLACEHOLDERFOLDERLAYOUT_H_
#define BERRYPLACEHOLDERFOLDERLAYOUT_H_

#include "berryIPlaceholderFolderLayout.h"

#include "berryLayoutPart.h"
#include "berryPageLayout.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui_internal
 *
 * This layout is used to define the initial set of placeholders
 * in a placeholder.
 * <p>
 * Views are added to the placeholder by ID. This id is used to identify
 * a view descriptor in the view registry, and this descriptor is used to
 * instantiate the IViewPart.
 * </p>
 */
class PlaceholderFolderLayout : public IPlaceholderFolderLayout
{

public:
  berryObjectMacro(PlaceholderFolderLayout);

private:

  PageLayout* pageLayout;

  ContainerPlaceholder::Pointer placeholder;

public:

  PlaceholderFolderLayout(PageLayout* pageLayout,
      ContainerPlaceholder::Pointer folder);

  /**
   * @see IPlaceholderFolderLayout
   */
  void AddPlaceholder(const QString& viewId) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPlaceholderFolderLayout#getProperty(java.lang.String)
   */
  QString GetProperty(const QString& id) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPlaceholderFolderLayout#setProperty(java.lang.String, java.lang.String)
   */
  void SetProperty(const QString& id, const QString& value) override;

private:

  /**
   * Inform the page layout of the new part created
   * and the placeholder the part belongs to.
   */
  void LinkPartToPageLayout(const QString& viewId,
      LayoutPart::Pointer newPart);

};

}

#endif /*BERRYPLACEHOLDERFOLDERLAYOUT_H_*/
