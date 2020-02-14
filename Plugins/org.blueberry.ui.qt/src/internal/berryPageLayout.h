/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPAGELAYOUT_H_
#define BERRYPAGELAYOUT_H_

#include "berryIPageLayout.h"

#include "berryViewLayoutRec.h"
#include "berryContainerPlaceholder.h"
#include "berryViewSashContainer.h"
#include "berryPartStack.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui_internal
 *
 * This factory is used to define the initial layout of a part sash container.
 * <p>
 * Design notes: The design of <code>IPageLayout</code> is a reflection of
 * three requirements:
 * <ol>
 *   <li>A mechanism is required to define the initial layout for a page. </li>
 *   <li>The views and editors within a page will be persisted between
 *     sessions.</li>
 *   <li>The view and editor lifecycle for (1) and (2) should be identical.</li>
 * </ol>
 * </p>
 * <p>
 * In reflection of these requirements, the following strategy has been
 * implemented for layout definition.
 * <ol>
 *   <li>A view extension is added to the workbench registry for the view.
 *     This extension defines the extension id and extension class.  </li>
 *   <li>A view is added to a page by invoking one of the add methods
 *     in <code>IPageLayout</code>. The type of view is passed as an
 *     extension id, rather than a handle. The page layout will map
 *     the extension id to a view class, create an instance of the class,
 *     and then add the view to the page.</li>
 * </ol>
 * </p>
 */
class PageLayout : public IPageLayout
{

public:
  berryObjectMacro(PageLayout);

private:

  //ArrayList actionSets = new ArrayList(3);

  IPerspectiveDescriptor::Pointer descriptor;

  LayoutPart::Pointer editorFolder;

  bool editorVisible;

  bool fixed;

  typedef QHash<QString, ILayoutContainer::Pointer> IDToFolderMap;
  IDToFolderMap mapIDtoFolder;

  typedef QHash<QString, LayoutPart::Pointer> IDToPartMap;
  IDToPartMap mapIDtoPart;

  typedef QHash<QString, ViewLayoutRec::Pointer> IDToViewLayoutRecMap;
  IDToViewLayoutRecMap mapIDtoViewLayoutRec;

  typedef QHash<ILayoutContainer::Pointer, IPlaceholderFolderLayout::Pointer> FolderToFolderLayoutMap;
  FolderToFolderLayoutMap mapFolderToFolderLayout;

  QList<QString> perspectiveShortcuts;

  ViewSashContainer::Pointer rootLayoutContainer;

  QList<QString> showInPartIds;
  QList<QString> showViewShortcuts;

  ViewFactory* viewFactory;

  /**
   * Constructs a new PageLayout for other purposes.
   */
public:
  PageLayout();

  /**
   * Constructs a new PageLayout for the normal case of creating a new
   * perspective.
   */
public:
  PageLayout(ViewSashContainer::Pointer container, ViewFactory* viewFactory,
      LayoutPart::Pointer editorFolder,
      IPerspectiveDescriptor::Pointer descriptor);

  /**
   * Adds the editor to a layout.
   */
private:
  void AddEditorArea();

  /**
   * Adds an action set to the page.
   *
   * @param actionSetID Identifies the action set extension to use. It must
   *            exist within the workbench registry.
   */
  //    public: void addActionSet(String actionSetID) {
  //        if (!actionSets.contains(actionSetID)) {
  //            actionSets.add(actionSetID);
  //        }
  //    }

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#addFastView(java.lang.String)
   */
  //    public: void addFastView(String id) {
  //        addFastView(id, INVALID_RATIO);
  //    }

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#addFastView(java.lang.String, float)
   */
  //    public: void addFastView(String id, float ratio) {
  //        if (checkPartInLayout(id)) {
  //      return;
  //    }
  //        if (id != null) {
  //            try {
  //        IViewDescriptor viewDescriptor = viewFactory.getViewRegistry()
  //            .find(ViewFactory.extractPrimaryId(id));
  //        if (!WorkbenchActivityHelper.filterItem(viewDescriptor)) {
  //          IViewReference ref = viewFactory.createView(ViewFactory
  //              .extractPrimaryId(id), ViewFactory
  //              .extractSecondaryId(id));
  //          fastViews.add(ref);
  //
  //          // force creation of the view layout rec
  //          ViewLayoutRec rec = getViewLayoutRec(id, true);
  //
  //          // remember the ratio, if valid
  //          if (ratio >= IPageLayout.RATIO_MIN
  //              && ratio <= IPageLayout.RATIO_MAX) {
  //            rec.fastViewWidthRatio = ratio;
  //          }
  //        }
  //      } catch (PartInitException e) {
  //        WorkbenchPlugin.log(getClass(), "addFastView", e); //$NON-NLS-1$
  //      }
  //        }
  //    }

  /**
   * Check to see if the partId represents a fast view's id.
   *
   * @param partId
   *            The part's id.
   * @return true if the partId is a fast view id.
   */
  //    private: boolean isFastViewId(String partId) {
  //        for (int i = 0; i < fastViews.size(); i++) {
  //      IViewReference ref = (IViewReference) fastViews.get(i);
  //      String secondaryId = ref.getSecondaryId();
  //      String refId = (secondaryId == null ? ref.getId() : ref.getId()
  //          + ":" + secondaryId); //$NON-NLS-1$
  //      if (refId.equals(partId)) {
  //        return true;
  //      }
  //        }
  //        return false;
  //    }

  /**
   * Returns the view layout record for the given view id, or null if not
   * found. If create is true, the record is created if it doesn't already
   * exist.
   *
   * @since 3.0
   */
public:
  ViewLayoutRec::Pointer GetViewLayoutRec(const QString& id, bool create);

  /**
   * Adds a creation wizard to the File New menu.
   * The id must name a new wizard extension contributed to the
   * workbench's extension point (named <code>"org.blueberry.ui.newWizards"</code>).
   *
   * @param id the wizard id
   */
  //    public: void addNewWizardShortcut(String id) {
  //        if (!newWizardShortcuts.contains(id)) {
  //            newWizardShortcuts.add(id);
  //        }
  //    }

  /**
   * Add the layout part to the page's layout
   */
private:
  void AddPart(LayoutPart::Pointer newPart, const QString& partId,
               int relationship, float ratio, const QString& refId);

  /**
   * Adds a perspective shortcut to the Perspective menu.
   * The id must name a perspective extension contributed to the
   * workbench's extension point (named <code>"org.blueberry.ui.perspectives"</code>).
   *
   * @param id the perspective id
   */
public:
  void AddPerspectiveShortcut(const QString& id) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#addPlaceholder(java.lang.String, int, float, java.lang.String)
   */
public:
  void AddPlaceholder(const QString& viewId, int relationship, float ratio,
      const QString& refId) override;

  /**
   * Checks whether the given id is a valid placeholder id.
   * A placeholder id may be simple or compound, and can optionally contain a wildcard.
   *
   * @param id the placeholder id
   * @return <code>true</code> if the given id is a valid placeholder id, <code>false</code> otherwise
   */
  bool CheckValidPlaceholderId(const QString& id);

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#addShowInPart(java.lang.String)
   */
public:
  void AddShowInPart(const QString& id) override;

  /**
   * Adds a view to the Show View menu. The id must name a view extension
   * contributed to the workbench's extension point (named <code>"org.blueberry.ui.views"</code>).
   *
   * @param id the view id
   */
public:
  void AddShowViewShortcut(const QString& id) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#addView(java.lang.String, int, float, java.lang.String)
   */
public:
  void AddView(const QString& viewId, int relationship, float ratio,
      const QString& refId) override;

  /**
   * Convenience method to allow setting the initial minimized
   * state if a new stack is created. Used by the 'perspectiveExtension'
   * reader.
   *
   *  @since 3.3
   */
public:
  void AddView(const QString& viewId, int relationship, float ratio,
      const QString& refId, bool minimized);

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#addView(java.lang.String, int, float, java.lang.String)
   */
private:
  void
      AddView(const QString& viewId, int relationship, float ratio,
          const QString& refId, bool minimized, bool standalone,
          bool showTitle);

  //    public: List getMinimizedStacks() {
  //      return minimizedStacks;
  //    }

  /**
   * Verify that the part is already present in the layout
   * and cannot be added again. Log a warning message.
   */
public:
  bool CheckPartInLayout(const QString& partId);

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#createFolder(java.lang.String, int, float, java.lang.String)
   */
public:
  IFolderLayout::Pointer CreateFolder(const QString& folderId,
      int relationship, float ratio, const QString& refId) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#createPlaceholderFolder(java.lang.String, int, float, java.lang.String)
   */
public:
  IPlaceholderFolderLayout::Pointer CreatePlaceholderFolder(
      const QString& folderId, int relationship, float ratio,
      const QString& refId) override;

  /**
   * Create a new <code>LayoutPart</code>.
   *
   * @param partID the id of the part to create.
   * @return the <code>LayoutPart</code>, or <code>null</code> if it should not be
   * created because of activity filtering.
   * @throws PartInitException thrown if there is a problem creating the part.
   */
private:
  LayoutPart::Pointer CreateView(const QString& partID);

  /**
   * @return the action set list for the page. This is <code>List</code> of
   * <code>String</code>s.
   */
  //    public: ArrayList getActionSets() {
  //        return actionSets;
  //    }

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#getDescriptor()
   */
public:
  IPerspectiveDescriptor::Pointer GetDescriptor() override;

  /**
   * @return an identifier for the editor area. The editor area is
   * automatically added to each layout before any other part. It should be
   * used as a reference part for other views.
   */
public:
  QString GetEditorArea() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#getEditorReuseThreshold()
   */
  //    public: int getEditorReuseThreshold() {
  //        return -1;
  //    }

  /**
   * @return <code>ArrayList</code>
   */
  //    public: ArrayList getFastViews() {
  //        return fastViews;
  //    }

  /**
   * @return the folder part containing the given view ID or <code>null</code>
   * if none (i.e. part of the page layout instead of a folder layout).
   */
private:
  PartStack::Pointer GetFolderPart(const QString& viewId);

  /**
   * @return the new wizard shortcuts associated with the page. This is a <code>List</code> of
   * <code>String</code>s.
   */
  //    public: ArrayList getNewWizardShortcuts() {
  //        return newWizardShortcuts;
  //    }

  /**
   * @return the part sash container const for a layout value.
   */
private:
  int GetPartSashConst(int nRelationship);

  /**
   * @return the perspective shortcuts associated with the page. This is a <code>List</code> of
   * <code>String</code>s.
   */
public:
  QList<QString> GetPerspectiveShortcuts();

  /**
   * @return the part for a given ID.
   */
  /*package*/
  LayoutPart::Pointer GetRefPart(const QString& partID);

  /**
   * @return the top level layout container.
   */
public:
  PartSashContainer::Pointer GetRootLayoutContainer();

  /**
   * @return the ids of the parts to list in the Show In... prompter. This is
   * a <code>List</code> of <code>String</code>s.
   */
public:
  QList<QString> GetShowInPartIds();

  /**
   * @return the show view shortcuts associated with the page. This is a <code>List</code> of
   * <code>String</code>s.
   */
public:
  QList<QString> GetShowViewShortcuts();

  /**
   * @return the <code>ViewFactory</code> for this <code>PageLayout</code>.
   * @since 3.0
   */
  /* package */
  ViewFactory* GetViewFactory();

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#isEditorAreaVisible()
   */
public:
  bool IsEditorAreaVisible() override;

  /**
   * Trim the ratio so that direct manipulation of parts is easy.
   *
   * @param in the initial ratio.
   * @return the normalized ratio.
   */
private:
  float NormalizeRatio(float in);

  /**
   * Prefill the layout with required parts.
   */
private:
  void Prefill();

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#setEditorAreaVisible(boolean)
   */
public:
  void SetEditorAreaVisible(bool showEditorArea) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#setEditorReuseThreshold(int)
   */
  //    public: void setEditorReuseThreshold(int openEditors) {
  //        //no-op
  //    }

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#setFixed(boolean)
   */
public:
  void SetFixed(bool fixed) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#getFixed()
   */
public:
  bool IsFixed() override;

  /**
   * Map the folder part containing the given view ID.
   *
   * @param viewId the part ID.
   * @param container the <code>ContainerPlaceholder</code>.
   */
  /*package*/
  void SetFolderPart(const QString& viewId,
      ContainerPlaceholder::Pointer container);

  /**
   * Map the folder part containing the given view ID.
   *
   * @param viewId the part ID.
   * @param folder the <code>ViewStack</code>.
   */
  /*package*/
  void SetFolderPart(const QString& viewId, PartStack::Pointer folder);

  void SetFolderPart(const QString& viewId, ILayoutContainer::Pointer folder);

  /**
   * Map an ID to a part.
   *
   * @param partId the part ID.
   * @param part the <code>LayoutPart</code>.
   */
  /*package*/
  void SetRefPart(const QString& partID, LayoutPart::Pointer part);

  /**
   * Stack a part on top of another.
   *
   * @param newPart the new part.
   * @param viewId the view ID.
   * @param refId the reference ID.
   */
private:
  void StackPart(LayoutPart::Pointer newPart, const QString& viewId,
                 const QString& refId);

  /**
   * Stack a placeholder on top of another.
   *
   * @param viewId the view ID.
   * @param refId the reference ID.
   */
public:
  void StackPlaceholder(const QString& viewId, const QString& refId);

  // stackView(String, String) modified by dan_rubel@instantiations.com
  /**
   * Stack one view on top of another.
   *
   * @param viewId the view ID.
   * @param refId the reference ID.
   */
public:
  void StackView(const QString& viewId, const QString& refId);

  /**
   * Converts common position constants into layout position constants.
   *
   * @param one of Constants::TOP, Constants::BOTTOM, Constants::LEFT, or Constants::RIGHT
   * @return one of IPageLayout::TOP, IPageLayout::BOTTOM, IPageLayout::LEFT, IPageLayout::RIGHT, or -1 indicating an
   * invalid input
   *
   * @since 3.0
   */
  public: static int ConstantToLayoutPosition(int constant);

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#addStandaloneView(java.lang.String, boolean, int, float, java.lang.String)
   * @since 3.0
   */
public:
  void AddStandaloneView(const QString& viewId, bool showTitle,
      int relationship, float ratio, const QString& refId) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#addStandaloneViewPlaceholder(java.lang.String, int, float, java.lang.String, boolean)
   */
public:
  void AddStandaloneViewPlaceholder(const QString& viewId,
      int relationship, float ratio, const QString& refId, bool showTitle) override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IPageLayout#getViewLayout(java.lang.String)
   * @since 3.0
   */
public:
  IViewLayout::Pointer GetViewLayout(const QString& viewId) override;

  /**
   * @since 3.0
   */
public:
  QHash<QString, ViewLayoutRec::Pointer> GetIDtoViewLayoutRecMap();

  /**
   * Removes any existing placeholder with the given id.
   *
   * @param id the id for the placeholder
   * @since 3.1
   */
public:
  void RemovePlaceholder(const QString& id);

  /* (non-Javadoc)
   * @see org.blueberry.ui.IPageLayout#getFolderForView(java.lang.String)
   */
public:
  IPlaceholderFolderLayout::Pointer GetFolderForView(const QString& viewId) override;
};

}

#endif /*BERRYPAGELAYOUT_H_*/
