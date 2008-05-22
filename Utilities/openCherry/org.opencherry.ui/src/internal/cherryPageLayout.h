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

#ifndef CHERRYPAGELAYOUT_H_
#define CHERRYPAGELAYOUT_H_

#include "../cherryIPageLayout.h"

#include "cherryViewLayoutRec.h"
#include "cherryContainerPlaceholder.h"

namespace cherry
{

/**
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

private:

  //ArrayList actionSets = new ArrayList(3);

  IPerspectiveDescriptor::Pointer descriptor;

  LayoutPart::Pointer editorFolder;

  bool editorVisible;

  bool fixed;

  std::map<std::string, LayoutPart::Pointer> mapIDtoFolder;

  std::map<std::string, LayoutPart::Pointer> mapIDtoPart;

  std::map<std::string, ViewLayoutRec> mapIDtoViewLayoutRec;

  std::map<Object, Object> mapFolderToFolderLayout;

  std::vector<std::string> perspectiveShortcuts;

  ViewSashContainer::Pointer rootLayoutContainer;

  std::vector<std::string> showInPartIds;
  std::vector<std::string> showViewShortcuts;

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
   * @see org.eclipse.ui.IPageLayout#addFastView(java.lang.String)
   */
  //    public: void addFastView(String id) {
  //        addFastView(id, INVALID_RATIO);
  //    }

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#addFastView(java.lang.String, float)
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
  ViewLayoutRec GetViewLayoutRec(const std::string& id, bool create);

  /**
   * Adds a creation wizard to the File New menu.
   * The id must name a new wizard extension contributed to the 
   * workbench's extension point (named <code>"org.eclipse.ui.newWizards"</code>).
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
  void AddPart(LayoutPart::Pointer newPart, const std::string& partId,
      int relationship, float ratio, const std::string& refId);

  /**
   * Adds a perspective shortcut to the Perspective menu.
   * The id must name a perspective extension contributed to the 
   * workbench's extension point (named <code>"org.eclipse.ui.perspectives"</code>).
   *
   * @param id the perspective id
   */
public:
  void AddPerspectiveShortcut(const std::string& id);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#addPlaceholder(java.lang.String, int, float, java.lang.String)
   */
public:
  void AddPlaceholder(const std::string& viewId, int relationship, float ratio,
      const std::string& refId);

  /**
   * Checks whether the given id is a valid placeholder id.
   * A placeholder id may be simple or compound, and can optionally contain a wildcard.
   * 
   * @param id the placeholder id
   * @return <code>true</code> if the given id is a valid placeholder id, <code>false</code> otherwise
   */
  bool CheckValidPlaceholderId(const std::string& id);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#addShowInPart(java.lang.String)
   */
public:
  void AddShowInPart(const std::string& id);

  /**
   * Adds a view to the Show View menu. The id must name a view extension
   * contributed to the workbench's extension point (named <code>"org.eclipse.ui.views"</code>).
   * 
   * @param id the view id
   */
public:
  void AddShowViewShortcut(const std::string& id);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#addView(java.lang.String, int, float, java.lang.String)
   */
public:
  void AddView(const std::string& viewId, int relationship, float ratio,
      const std::string& refId);

  /**
   * Convenience method to allow setting the initial minimized
   * state if a new stack is created. Used by the 'perspectiveExtension'
   * reader.
   * 
   *  @since 3.3
   */
public:
  void AddView(const std::string& viewId, int relationship, float ratio,
      const std::string& refId, bool minimized);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#addView(java.lang.String, int, float, java.lang.String)
   */
private:
  void
      AddView(const std::string& viewId, int relationship, float ratio,
          const std::string& refId, bool minimized, bool standalone,
          bool showTitle);

  //    public: List getMinimizedStacks() {
  //      return minimizedStacks;
  //    }

  /**
   * Verify that the part is already present in the layout
   * and cannot be added again. Log a warning message.
   */
public:
  bool CheckPartInLayout(const std::string& partId);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#createFolder(java.lang.String, int, float, java.lang.String)
   */
public:
  IFolderLayout::Pointer CreateFolder(const std::string& folderId,
      int relationship, float ratio, const std::string& refId);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#createPlaceholderFolder(java.lang.String, int, float, java.lang.String)
   */
public:
  IPlaceholderFolderLayout::Pointer CreatePlaceholderFolder(
      const std::string& folderId, int relationship, float ratio,
      const std::string& refId);

  /**
   * Create a new <code>LayoutPart</code>.
   * 
   * @param partID the id of the part to create.
   * @return the <code>LayoutPart</code>, or <code>null</code> if it should not be
   * created because of activity filtering.
   * @throws PartInitException thrown if there is a problem creating the part.
   */
private:
  LayoutPart::Pointer CreateView(const std::string& partID);

  /**
   * @return the action set list for the page. This is <code>List</code> of 
   * <code>String</code>s.
   */
  //    public: ArrayList getActionSets() {
  //        return actionSets;
  //    }

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#getDescriptor()
   */
public:
  IPerspectiveDescriptor::Pointer GetDescriptor();

  /**
   * @return an identifier for the editor area. The editor area is
   * automatically added to each layout before any other part. It should be
   * used as a reference part for other views.
   */
public:
  std::string GetEditorArea();

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#getEditorReuseThreshold()
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
  PartStack::Pointer GetFolderPart(const std::string& viewId);

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
  std::vector<std::string> GetPerspectiveShortcuts();

  /**
   * @return the part for a given ID.
   */
  /*package*/
  LayoutPart::Pointer GetRefPart(const std::string& partID);

  /**
   * @return the top level layout container.
   */
public:
  ViewSashContainer::Pointer GetRootLayoutContainer();

  /**
   * @return the ids of the parts to list in the Show In... prompter. This is
   * a <code>List</code> of <code>String</code>s.
   */
public:
  std::vector<std::string> GetShowInPartIds();

  /**
   * @return the show view shortcuts associated with the page. This is a <code>List</code> of 
   * <code>String</code>s.
   */
public:
  std::vector<std::string> GetShowViewShortcuts();

  /**
   * @return the <code>ViewFactory</code> for this <code>PageLayout</code>.
   * @since 3.0
   */
  /* package */
  ViewFactory* GetViewFactory();

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#isEditorAreaVisible()
   */
public:
  bool IsEditorAreaVisible();

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
   * @see org.eclipse.ui.IPageLayout#setEditorAreaVisible(boolean)
   */
public:
  void SetEditorAreaVisible(bool showEditorArea);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#setEditorReuseThreshold(int)
   */
  //    public: void setEditorReuseThreshold(int openEditors) {
  //        //no-op
  //    }

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#setFixed(boolean)
   */
public:
  void SetFixed(bool fixed);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#getFixed()
   */
public:
  bool IsFixed();

  /**
   * Map the folder part containing the given view ID.
   * 
   * @param viewId the part ID.
   * @param container the <code>ContainerPlaceholder</code>.
   */
  /*package*/
  void SetFolderPart(const std::string& viewId,
      ContainerPlaceholder::Pointer container);

  /**
   * Map the folder part containing the given view ID.
   * 
   * @param viewId the part ID.
   * @param folder the <code>ViewStack</code>.
   */
  /*package*/
  void SetFolderPart(const std::string& viewId, PartStack::Pointer folder);

  /**
   * Map an ID to a part.
   * 
   * @param partId the part ID.
   * @param part the <code>LayoutPart</code>.
   */
  /*package*/
  void SetRefPart(const std::string& partID, LayoutPart::Pointer part);

  // stackPart(Layoutpart, String, String) added by dan_rubel@instantiations.com
  /**
   * Stack a part on top of another.
   * 
   * @param newPart the new part.
   * @param viewId the view ID.
   * @param refId the reference ID.
   */
private:
  void StackPart(LayoutPart::Pointer newPart, const std::string& viewId,
      const std::string& refId);

  // stackPlaceholder(String, String) added by dan_rubel@instantiations.com
  /**
   * Stack a placeholder on top of another.
   * 
   * @param viewId the view ID.
   * @param refId the reference ID.
   */
public:
  void StackPlaceholder(const std::string& viewId, const std::string& refId);

  // stackView(String, String) modified by dan_rubel@instantiations.com
  /**
   * Stack one view on top of another.
   * 
   * @param viewId the view ID.
   * @param refId the reference ID.
   */
public:
  void StackView(const std::string& viewId, const std::string& refId);

  /**
   * Converts SWT position constants into layout position constants.
   * 
   * @param swtConstant one of SWT.TOP, SWT.BOTTOM, SWT.LEFT, or SWT.RIGHT
   * @return one of IPageLayout.TOP, IPageLayout.BOTTOM, IPageLayout.LEFT, IPageLayout.RIGHT, or -1 indicating an
   * invalid input
   * 
   * @since 3.0
   */
  //    public: static int swtConstantToLayoutPosition(int swtConstant) {
  //        switch (swtConstant) {
  //        case SWT.TOP:
  //            return IPageLayout.TOP;
  //        case SWT.BOTTOM:
  //            return IPageLayout.BOTTOM;
  //        case SWT.RIGHT:
  //            return IPageLayout.RIGHT;
  //        case SWT.LEFT:
  //            return IPageLayout.LEFT;
  //        }
  //
  //        return -1;
  //    }

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#addStandaloneView(java.lang.String, boolean, int, float, java.lang.String)
   * @since 3.0
   */
public:
  void AddStandaloneView(const std::string& viewId, bool showTitle,
      int relationship, float ratio, const std::string& refId);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#addStandaloneViewPlaceholder(java.lang.String, int, float, java.lang.String, boolean)
   */
public:
  void AddStandaloneViewPlaceholder(const std::string& viewId,
      int relationship, float ratio, const std::string& refId, bool showTitle);

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IPageLayout#getViewLayout(java.lang.String)
   * @since 3.0
   */
public:
  IViewLayout::Pointer GetViewLayout(const std::string& viewId);

  /**
   * @since 3.0
   */
public:
  std::map<std::string, ViewLayoutRec> GetIDtoViewLayoutRecMap();

  /**
   * Removes any existing placeholder with the given id.
   * 
   * @param id the id for the placeholder
   * @since 3.1
   */
public:
  void RemovePlaceholder(const std::string& id);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IPageLayout#getFolderForView(java.lang.String)
   */
public:
  IPlaceholderFolderLayout::Pointer GetFolderForView(const std::string& viewId);
};

}

#endif /*CHERRYPAGELAYOUT_H_*/
