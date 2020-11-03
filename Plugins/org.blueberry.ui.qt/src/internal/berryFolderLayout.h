/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYFOLDERLAYOUT_H_
#define BERRYFOLDERLAYOUT_H_

#include "berryIFolderLayout.h"

#include "berryPartStack.h"
#include "berryViewFactory.h"

namespace berry {

class PageLayout;


/**
 * \ingroup org_blueberry_ui_internal
 *
 * This layout is used to define the initial set of views and placeholders
 * in a folder.
 * <p>
 * Views are added to the folder by ID. This id is used to identify
 * a view descriptor in the view registry, and this descriptor is used to
 * instantiate the <code>IViewPart</code>.
 * </p>
 */
class FolderLayout : public IFolderLayout {

public: berryObjectMacro(FolderLayout);

private:

  PartStack::Pointer folder;

  SmartPointer<PageLayout> pageLayout;

  ViewFactory* viewFactory;


public:

    /**
     * Create an instance of a <code>FolderLayout</code> belonging to a
     * <code>PageLayout</code>.
     */
     FolderLayout(SmartPointer<PageLayout> pageLayout, PartStack::Pointer folder,
            ViewFactory* viewFactory);

    /* (non-Javadoc)
     * @see org.blueberry.ui.IPlaceholderFolderLayout#addPlaceholder(java.lang.String)
     */
    void AddPlaceholder(const QString& viewId) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.IFolderLayout#addView(java.lang.String)
     */
    void AddView(const QString& viewId) override;

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
        * and the folder the part belongs to.
        */
  void LinkPartToPageLayout(const QString& viewId, LayoutPart::Pointer newPart);

};

}

#endif /*BERRYFOLDERLAYOUT_H_*/
