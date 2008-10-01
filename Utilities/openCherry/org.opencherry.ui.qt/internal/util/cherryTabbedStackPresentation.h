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


#ifndef CHERRYTABBEDSTACKPRESENTATION_H_
#define CHERRYTABBEDSTACKPRESENTATION_H_

#include <presentations/cherryStackPresentation.h>

#include "cherryPresentablePartFolder.h"
#include "cherryTabOrder.h"

namespace cherry {

class TabbedStackPresentation : public StackPresentation {

private:

  typedef MessageDelegate1<TabbedStackPresentation, TabFolderEvent::Pointer> TabFolderEventDelegate;

  PresentablePartFolder* folder;
  //ISystemMenu systemMenu;
  //ISystemMenu partList;

//  PreferenceStoreAdapter apiPreferences = new PreferenceStoreAdapter(PrefUtil
//            .getAPIPreferenceStore());
//  ThemeManagerAdapter themePreferences = new ThemeManagerAdapter(
//            PlatformUI.getWorkbench().getThemeManager());

  TabOrder* tabs;

  //TODO Drag Drop
  //TabDragHandler dragBehavior;

    bool initializing;
    int ignoreSelectionChanges;

    int dragStart;
    //Map prefs = new HashMap();

    /**
     * Returns the minimum size for this stack, taking into account
     * the available perpendicular space.
     * @param width indicates whether a width (=true) or a height (=false) is being computed
     * @param availablePerpendicular available space perpendicular to the direction being measured
     * or INFINITE if unbounded (pixels).
     * @return returns the preferred minimum size (pixels).
     * This is a width if width == true or a height if width == false.
     */
    int ComputePreferredMinimumSize(bool width, int availablePerpendicular);


public:

  void HandleTabFolderEvent(TabFolderEvent::Pointer e);

/**
 * Creates a TabbedStackPresentation. The created object will take ownership of the AbstractTabFolder pointer.
 */
  TabbedStackPresentation(IStackPresentationSite::Pointer site, AbstractTabFolder* widget/*, ISystemMenu systemMenu*/);

  /**
   * Creates a TabbedStackPresentation. The created object will take ownership of the PresentablePartFolder pointer.
   */
    TabbedStackPresentation(IStackPresentationSite::Pointer site, PresentablePartFolder* folder/*, ISystemMenu systemMenu*/);

    /**
   * Creates a TabbedStackPresentation. The created object will take ownership of the PresentablePartFolder and the TabOrder pointer.
   */
    TabbedStackPresentation(IStackPresentationSite::Pointer site,
            PresentablePartFolder* newFolder, TabOrder* tabs /*, TabDragHandler dragBehavior, ISystemMenu systemMenu*/);

    void Init(IStackPresentationSite::Pointer site,
            PresentablePartFolder* newFolder, TabOrder* tabs /*, TabDragHandler dragBehavior, ISystemMenu systemMenu*/);
    /**
     * Restores a presentation from a previously stored state
     *
     * @param serializer (not null)
     * @param savedState (not null)
     */
    void RestoreState(IPresentationSerializer* serializer,
            IMemento::Pointer savedState);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#saveState(org.opencherry.ui.presentations.IPresentationSerializer, org.opencherry.ui.IMemento)
     */
    void SaveState(IPresentationSerializer* context, IMemento::Pointer memento);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#setBounds(org.opencherry.swt.graphics.Rectangle)
     */
    void SetBounds(const Rectangle& bounds);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#computeMinimumSize()
     */
    Point ComputeMinimumSize();

    /* (non-Javadoc)
     * @see org.opencherry.ui.ISizeProvider#computePreferredSize(boolean, int, int, int)
     */
    int ComputePreferredSize(bool width, int availableParallel,
            int availablePerpendicular, int preferredResult);


    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#getSizeFlags(boolean)
     */
    int GetSizeFlags(bool width);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#showPartList()
     */
    void ShowPartList();

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#dispose()
     */
    ~TabbedStackPresentation();

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#setActive(int)
     */
    void SetActive(int newState);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#setVisible(boolean)
     */
    void SetVisible(bool isVisible);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#setState(int)
     */
    void SetState(int state);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#getControl()
     */
    void* GetControl();

    /**
     * @return AbstractTabFolder the presentation's tab folder
     */
    AbstractTabFolder* GetTabFolder();

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#addPart(org.opencherry.ui.presentations.IPresentablePart, java.lang.Object)
     */
    void AddPart(IPresentablePart::Pointer newPart, Object::Pointer cookie);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#movePart(org.opencherry.ui.presentations.IPresentablePart, java.lang.Object)
     */
    void MovePart(IPresentablePart::Pointer toMove, Object::Pointer cookie);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#removePart(org.opencherry.ui.presentations.IPresentablePart)
     */
    void RemovePart(IPresentablePart::Pointer oldPart);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#selectPart(org.opencherry.ui.presentations.IPresentablePart)
     */
    void SelectPart(IPresentablePart::Pointer toSelect);

    //TODO DnD
//    /* (non-Javadoc)
//     * @see org.opencherry.ui.presentations.StackPresentation#dragOver(org.opencherry.swt.widgets.Control, org.opencherry.swt.graphics.Point)
//     */
//    StackDropResult dragOver(Control currentControl, Point location) {
//        return dragBehavior.dragOver(currentControl, location, dragStart);
//    }

//    void showSystemMenu() {
//        showSystemMenu(folder.getTabFolder().getSystemMenuLocation(), getSite().getSelectedPart());
//    }

//    void showSystemMenu(Point displayCoordinates, IPresentablePart context) {
//        if (context != getSite().getSelectedPart()) {
//            getSite().selectPart(context);
//        }
//        systemMenu.show(getControl(), displayCoordinates, context);
//    }

//    /* (non-Javadoc)
//     * @see org.opencherry.ui.presentations.StackPresentation#showPaneMenu()
//     */
//    void showPaneMenu() {
//        IPresentablePart part = getSite().getSelectedPart();
//
//        if (part != null) {
//            showPaneMenu(part, folder.getTabFolder().getPaneMenuLocation());
//        }
//    }

//    void showPaneMenu(IPresentablePart part, Point location) {
//        Assert.isTrue(!isDisposed());
//
//        IPartMenu menu = part.getMenu();
//
//        if (menu != null) {
//            menu.showMenu(location);
//        }
//    }

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#getTabList(org.opencherry.ui.presentations.IPresentablePart)
     */
    std::vector<void*> GetTabList(IPresentablePart::Pointer part);

//    void SetPartList(ISystemMenu menu) {
//        this.partList = menu;
//    }

//    IDynamicPropertyMap getTheme() {
//        return themePreferences;
//    }

//    IDynamicPropertyMap getApiPreferences() {
//        return apiPreferences;
//    }

//    IDynamicPropertyMap getPluginPreferences(Plugin toQuery) {
//        String id = toQuery.getBundle().getSymbolicName();
//        IDynamicPropertyMap result = (IDynamicPropertyMap)prefs.get(id);
//
//        if (result != null) {
//            return result;
//        }
//
//        result = new PreferencesAdapter(toQuery.getPluginPreferences());
//        prefs.put(id, result);
//        return result;
//    }

    /**
     * Move the tabs around.  This is for testing <b>ONLY</b>.
     * @param part the part to move
     * @param index the new index
     * @since 3.2
     */
    void MoveTab(IPresentablePart::Pointer part, int index);

    /**
     * Get the tab list. This is for testing <b>ONLY</b>.
     * @return the presentable parts in order.
     * @since 3.2
     */
    std::vector<IPresentablePart::Pointer> GetPartList();

//  /**
//   * Cause the folder to hide or show its
//   * Minimize and Maximize affordances.
//   *
//   * @param show
//   *            <code>true</code> - the min/max buttons are visible.
//   * @since 3.3
//   */
//  void ShowMinMax(bool show) {
//        folder.getTabFolder().showMinMax(show);
//  }
};

}

#endif /* CHERRYTABBEDSTACKPRESENTATION_H_ */
