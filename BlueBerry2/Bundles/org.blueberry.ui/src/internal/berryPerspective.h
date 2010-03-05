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

#ifndef BERRYPERSPECTIVE_H_
#define BERRYPERSPECTIVE_H_

#include <osgi/framework/Macros.h>

#include "berryPerspectiveDescriptor.h"
#include "berryPartPlaceholder.h"
#include "berryViewLayoutRec.h"
#include "berryWorkbenchPage.h"
#include "berryLayoutPart.h"
#include "berryPageLayout.h"

#include "berryPartPane.h"
#include "../berryIWorkbenchPartReference.h"
#include "../berryIViewReference.h"
#include "../berryIViewPart.h"

#include <map>
#include <vector>
#include <string>

namespace berry {

class ViewFactory;
class PerspectiveHelper;

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
class Perspective : public Object {

public:

  osgiObjectMacro(Perspective);

  friend class WorkbenchPage;

private:

  ViewFactory* viewFactory;
  std::map<std::string, ViewLayoutRec::Pointer> mapIDtoViewLayoutRec;

  static const std::string VERSION_STRING; // = "0.016";//$NON-NLS-1$

  /**
   * Reference to the part that was previously active
   * when this perspective was deactivated.
   */
  IWorkbenchPartReference::Pointer oldPartRef;

protected:

  PerspectiveDescriptor::Pointer descriptor;

  WorkbenchPage* page;

    // Editor Area management
    LayoutPart::Pointer editorArea;
    ContainerPlaceholder::Pointer editorHolder;
    bool editorHidden;
    int editorAreaState;

    //ArrayList alwaysOnActionSets;

    //ArrayList alwaysOffActionSets;

    std::vector<std::string> showViewShortcuts;

    std::vector<std::string> perspectiveShortcuts;

    bool fixed;

    std::vector<std::string> showInPartIds;

    //HashMap showInTimes;

    IMemento::Pointer memento;

    PerspectiveHelper* presentation;

    bool shouldHideEditorsOnActivate;

    PageLayout::Pointer layout;


    /**
     * ViewManager constructor comment.
     */
public: Perspective(PerspectiveDescriptor::Pointer desc, WorkbenchPage::Pointer page);

    /**
     * ViewManager constructor comment.
     */
    protected: Perspective(WorkbenchPage::Pointer page);

    protected: void Init(WorkbenchPage::Pointer page);


    /**
     * Moves a part forward in the Z order of a perspective so it is visible.
     *
     * @param part the part to bring to move forward
     * @return true if the part was brought to top, false if not.
     */
    public: bool BringToTop(IViewReference::Pointer ref);

    /**
     * Returns whether a view exists within the perspective.
     */
    public: bool ContainsView(IViewPart::Pointer view);

    /**
     * Create the initial list of action sets.
     */
//    protected: void CreateInitialActionSets(List outputList, List stringList) {
//        ActionSetRegistry reg = WorkbenchPlugin.getDefault()
//                .getActionSetRegistry();
//        Iterator iter = stringList.iterator();
//        while (iter.hasNext()) {
//            String id = (String) iter.next();
//            IActionSetDescriptor desc = reg.findActionSet(id);
//            if (desc != null) {
//        outputList.add(desc);
//      } else {
//        WorkbenchPlugin.log("Unable to find Action Set: " + id);//$NON-NLS-1$
//      }
//        }
//    }

    /**
     * Create a presentation for a perspective.
     */
    private: void CreatePresentation(PerspectiveDescriptor::Pointer persp);

    /**
     * Dispose the perspective and all views contained within.
     */
    public: ~Perspective();

    private: void DisposeViewRefs();

    /**
     * Finds the view with the given ID that is open in this page, or <code>null</code>
     * if not found.
     *
     * @param viewId the view ID
     */
    public: IViewReference::Pointer FindView(const std::string& viewId);

    /**
     * Finds the view with the given id and secondary id that is open in this page,
     * or <code>null</code> if not found.
     *
     * @param viewId the view ID
     * @param secondaryId the secondary ID
     */
    public: IViewReference::Pointer FindView(const std::string& id, const std::string& secondaryId);

    /**
     * Returns the window's client composite widget
     * which views and editor area will be parented.
     */
    public: void* GetClientComposite();

    /**
     * Returns the perspective.
     */
    public: IPerspectiveDescriptor::Pointer GetDesc();


    /**
     * Returns the pane for a view reference.
     */
    protected: PartPane::Pointer GetPane(IViewReference::Pointer ref);

    /**
     * Returns the perspective shortcuts associated with this perspective.
     *
     * @return an array of perspective identifiers
     */
    public: std::vector<std::string> GetPerspectiveShortcuts();

    /**
     * Returns the presentation.
     */
    public: PerspectiveHelper* GetPresentation() const;

    /**
     * Returns the show view shortcuts associated with this perspective.
     *
     * @return an array of view identifiers
     */
    public: std::vector<std::string> GetShowViewShortcuts();

    /**
     * Returns the view factory.
     */
    public: ViewFactory* GetViewFactory();

    /**
     * See IWorkbenchPage.
     */
    public: std::vector<IViewReference::Pointer> GetViewReferences();


    /**
     * Hide the editor area if visible
     */
    protected: void HideEditorArea();

    /**
     * Hide the editor area if visible
     */
    protected: void HideEditorAreaLocal();


    public: bool HideView(IViewReference::Pointer ref);

    /*
     * Return whether the editor area is visible or not.
     */
    protected: bool IsEditorAreaVisible();


    /**
     * Returns the view layout rec for the given view reference,
     * or null if not found.  If create is true, it creates the record
     * if not already created.
     */
    public: ViewLayoutRec::Pointer GetViewLayoutRec(IViewReference::Pointer ref, bool create);

    /**
     * Returns the view layout record for the given view id
     * or null if not found.  If create is true, it creates the record
     * if not already created.
     */
    private: ViewLayoutRec::Pointer GetViewLayoutRec(const std::string& viewId, bool create);

    /**
     * Returns true if a layout or perspective is fixed.
     */
    public: bool IsFixedLayout();

    /**
     * Returns true if a view is standalone.
     *
     * @since 3.0
     */
    public: bool IsStandaloneView(IViewReference::Pointer ref);

    /**
     * Returns whether the title for a view should
     * be shown.  This applies only to standalone views.
     *
     * @since 3.0
     */
    public: bool GetShowTitleView(IViewReference::Pointer ref);

    /**
     * Creates a new presentation from a persistence file.
     * Note: This method should not modify the current state of the perspective.
     */
    private: void LoadCustomPersp(PerspectiveDescriptor::Pointer persp);

    private: void UnableToOpenPerspective(PerspectiveDescriptor::Pointer persp,
            const std::string& status);

    /**
     * Create a presentation for a perspective.
     * Note: This method should not modify the current state of the perspective.
     */
    protected: void LoadPredefinedPersp(PerspectiveDescriptor::Pointer persp);

//    private: void RemoveAlwaysOn(IActionSetDescriptor::Pointer descriptor) {
//        if (descriptor == null) {
//            return;
//        }
//        if (!alwaysOnActionSets.contains(descriptor)) {
//            return;
//        }
//
//        alwaysOnActionSets.remove(descriptor);
//        if (page != null) {
//            page.perspectiveActionSetChanged(this, descriptor, ActionSetManager.CHANGE_HIDE);
//        }
//    }

//    protected: void AddAlwaysOff(IActionSetDescriptor descriptor) {
//        if (descriptor == null) {
//            return;
//        }
//        if (alwaysOffActionSets.contains(descriptor)) {
//            return;
//        }
//        alwaysOffActionSets.add(descriptor);
//        if (page != null) {
//            page.perspectiveActionSetChanged(this, descriptor, ActionSetManager.CHANGE_MASK);
//        }
//        removeAlwaysOn(descriptor);
//    }

//    protected: void AddAlwaysOn(IActionSetDescriptor descriptor) {
//        if (descriptor == null) {
//            return;
//        }
//        if (alwaysOnActionSets.contains(descriptor)) {
//            return;
//        }
//        alwaysOnActionSets.add(descriptor);
//        if (page != null) {
//            page.perspectiveActionSetChanged(this, descriptor, ActionSetManager.CHANGE_SHOW);
//        }
//        removeAlwaysOff(descriptor);
//    }

//    private: void RemoveAlwaysOff(IActionSetDescriptor descriptor) {
//        if (descriptor == null) {
//            return;
//        }
//        if (!alwaysOffActionSets.contains(descriptor)) {
//            return;
//        }
//        alwaysOffActionSets.remove(descriptor);
//        if (page != null) {
//            page.perspectiveActionSetChanged(this, descriptor, ActionSetManager.CHANGE_UNMASK);
//        }
//    }

    /**
     * activate.
     */
  protected: void OnActivate();

  /**
     * deactivate.
     */
  protected: void OnDeactivate();

    /**
     * Notifies that a part has been activated.
     */
    public: void PartActivated(IWorkbenchPart::Pointer activePart);

    /**
     * The user successfully performed a Show In... action on the specified part.
     * Update the history.
     */
    public: void PerformedShowIn(const std::string& partId);


    /**
     * Fills a presentation with layout data.
     * Note: This method should not modify the current state of the perspective.
     */
    public: bool RestoreState(IMemento::Pointer memento);

    bool CreateReferences(const std::vector<IMemento::Pointer>& views);

    /**
     * Fills a presentation with layout data.
     * Note: This method should not modify the current state of the perspective.
     */
    public: bool RestoreState();


    /**
     * Returns the ActionSets read from perspectiveExtensions in the registry.
     */
//    protected: ArrayList GetPerspectiveExtensionActionSets() {
//        PerspectiveExtensionReader reader = new PerspectiveExtensionReader();
//        reader
//                .setIncludeOnlyTags(new String[] { IWorkbenchRegistryConstants.TAG_ACTION_SET });
//        PageLayout layout = new PageLayout();
//        reader.extendLayout(null, descriptor.getOriginalId(), layout);
//        return layout.getActionSets();
//    }

    /**
     * Returns the Show In... part ids read from the registry.
     */
    protected: std::vector<std::string> GetShowInIdsFromRegistry();

    /**
     * Save the layout.
     */
    public: void SaveDesc();

    /**
     * Save the layout.
     */
    public: void SaveDescAs(IPerspectiveDescriptor::Pointer desc);

    /**
     * Save the layout.
     */
    public: bool SaveState(IMemento::Pointer memento);

    /**
     * Save the layout.
     */
    private: bool SaveState(IMemento::Pointer memento, PerspectiveDescriptor::Pointer p,
            bool saveInnerViewState);

//    public: void turnOnActionSets(IActionSetDescriptor[] newArray) {
//        for (int i = 0; i < newArray.length; i++) {
//            IActionSetDescriptor descriptor = newArray[i];
//
//            addAlwaysOn(descriptor);
//        }
//    }

//    public: void turnOffActionSets(IActionSetDescriptor[] toDisable) {
//        for (int i = 0; i < toDisable.length; i++) {
//            IActionSetDescriptor descriptor = toDisable[i];
//
//            turnOffActionSet(descriptor);
//        }
//    }

//    public: void turnOffActionSet(IActionSetDescriptor toDisable) {
//        addAlwaysOff(toDisable);
//    }



    /**
     * Sets the perspective actions for this page.
     * This is List of Strings.
     */
    public: void SetPerspectiveActionIds(const std::vector<std::string>& list);

    /**
     * Sets the ids of the parts to list in the Show In... prompter.
     * This is a List of Strings.
     */
    public: void SetShowInPartIds(const std::vector<std::string>& list);

    /**
     * Sets the ids of the views to list in the Show View shortcuts.
     * This is a List of Strings.
     */
    public: void SetShowViewActionIds(const std::vector<std::string>& list);


    /**
     * Show the editor area if not visible
     */
    protected: void ShowEditorArea();

    /**
     * Show the editor area if not visible
     */
    protected: void ShowEditorAreaLocal();

    public: void SetEditorAreaState(int newState);

    public: int GetEditorAreaState();

    /**
   *
   */
  public: void RefreshEditorAreaVisibility();


    /**
     * Resolves a view's id into its reference, creating the
     * view if necessary.
     *
     * @param viewId The primary id of the view (must not be
     * <code>null</code>
     * @param secondaryId The secondary id of a multiple-instance view
     * (may be <code>null</code>).
     *
     * @return The reference to the specified view. This may be null if the
     * view fails to create (i.e. thrown a PartInitException)
     */
    public: IViewReference::Pointer GetViewReference(const std::string& viewId, const std::string& secondaryId);

    /**
     * Shows the view with the given id and secondary id.
     */
    public: IViewPart::Pointer ShowView(const std::string& viewId, const std::string& secondaryId);


    /**
     * Returns the old part reference.
     * Returns null if there was no previously active part.
     *
     * @return the old part reference or <code>null</code>
     */
    public: IWorkbenchPartReference::Pointer GetOldPartRef();

    /**
     * Sets the old part reference.
     *
     * @param oldPartRef The old part reference to set, or <code>null</code>
     */
    public: void SetOldPartRef(IWorkbenchPartReference::Pointer oldPartRef);

//    //for dynamic UI
//    protected: void AddActionSet(IActionSetDescriptor newDesc) {
//      IContextService service = (IContextService)page.getWorkbenchWindow().getService(IContextService.class);
//      try {
//      service.activateContext(ContextAuthority.DEFER_EVENTS);
//      for (int i = 0; i < alwaysOnActionSets.size(); i++) {
//        IActionSetDescriptor desc = (IActionSetDescriptor) alwaysOnActionSets
//            .get(i);
//        if (desc.getId().equals(newDesc.getId())) {
//          removeAlwaysOn(desc);
//          removeAlwaysOff(desc);
//          break;
//        }
//      }
//      addAlwaysOn(newDesc);
//    } finally {
//        service.activateContext(ContextAuthority.SEND_EVENTS);
//      }
//    }

//    // for dynamic UI
//    /* package */void removeActionSet(String id) {
//      IContextService service = (IContextService)page.getWorkbenchWindow().getService(IContextService.class);
//      try {
//      service.activateContext(ContextAuthority.DEFER_EVENTS);
//      for (int i = 0; i < alwaysOnActionSets.size(); i++) {
//        IActionSetDescriptor desc = (IActionSetDescriptor) alwaysOnActionSets
//            .get(i);
//        if (desc.getId().equals(id)) {
//          removeAlwaysOn(desc);
//          break;
//        }
//      }
//
//      for (int i = 0; i < alwaysOffActionSets.size(); i++) {
//        IActionSetDescriptor desc = (IActionSetDescriptor) alwaysOffActionSets
//            .get(i);
//        if (desc.getId().equals(id)) {
//          removeAlwaysOff(desc);
//          break;
//        }
//      }
//    } finally {
//        service.activateContext(ContextAuthority.SEND_EVENTS);
//      }
//    }

//    void removeActionSet(IActionSetDescriptor toRemove) {
//        removeAlwaysOn(toRemove);
//        removeAlwaysOff(toRemove);
//    }


    /**
     * Returns whether the given view is closeable in this perspective.
     *
     * @since 3.0
     */
    public: bool IsCloseable(IViewReference::Pointer reference);

    /**
     * Returns whether the given view is moveable in this perspective.
     *
     * @since 3.0
     */
    public: bool IsMoveable(IViewReference::Pointer reference);

    /**
     * Writes a description of the layout to the given string buffer.
     * This is used for drag-drop test suites to determine if two layouts are the
     * same. Like a hash code, the description should compare as equal iff the
     * layouts are the same. However, it should be user-readable in order to
     * help debug failed tests. Although these are english readable strings,
     * they should not be translated or equality tests will fail.
     * <p>
     * This is only intended for use by test suites.
     * </p>
     *
     * @param buf
     */
    public: void DescribeLayout(std::string& buf) const;

    /**
     * Sanity-checks the LayoutParts in this perspective. Throws an Assertation exception
     * if an object's internal state is invalid.
     */
    public: void TestInvariants();

//    public: IActionSetDescriptor[] getAlwaysOnActionSets() {
//        return (IActionSetDescriptor[]) alwaysOnActionSets.toArray(new IActionSetDescriptor[alwaysOnActionSets.size()]);
//    }

//    public: IActionSetDescriptor[] getAlwaysOffActionSets() {
//        return (IActionSetDescriptor[]) alwaysOffActionSets.toArray(new IActionSetDescriptor[alwaysOffActionSets.size()]);
//    }


  /**
   * Used to restrict the use of the new min/max behavior to envoronments
   * in which it has a chance of working...
   *
   * @param activePerspective We pass this in as an arg so others won't have
   * to check it for 'null' (which is one of the failure cases)
   *
   */
  public: static bool UseNewMinMax(Perspective::Pointer activePerspective);

};

}

#endif /*BERRYPERSPECTIVE_H_*/
