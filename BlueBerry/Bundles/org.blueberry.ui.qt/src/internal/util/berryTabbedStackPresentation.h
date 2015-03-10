/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYTABBEDSTACKPRESENTATION_H_
#define BERRYTABBEDSTACKPRESENTATION_H_

#include <berryStackPresentation.h>

#include "berryPresentablePartFolder.h"
#include "berryTabOrder.h"
#include "berryTabDragHandler.h"

namespace berry {

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

  TabDragHandler* dragBehavior;

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
            PresentablePartFolder* newFolder, TabOrder* tabs, TabDragHandler* dragBehavior /*, ISystemMenu systemMenu*/);

    void Init(IStackPresentationSite::Pointer,
            PresentablePartFolder* newFolder, TabOrder* tabs, TabDragHandler* dragBehavior /*, ISystemMenu systemMenu*/);
    /**
     * Restores a presentation from a previously stored state
     *
     * @param serializer (not null)
     * @param savedState (not null)
     */
    void RestoreState(IPresentationSerializer* serializer,
            IMemento::Pointer savedState);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#saveState(org.blueberry.ui.presentations.IPresentationSerializer, org.blueberry.ui.IMemento)
     */
    void SaveState(IPresentationSerializer* context, IMemento::Pointer memento);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#setBounds(org.blueberry.swt.graphics.QRect)
     */
    void SetBounds(const QRect& bounds);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#computeMinimumSize()
     */
    QSize ComputeMinimumSize();

    /* (non-Javadoc)
     * @see org.blueberry.ui.ISizeProvider#computePreferredSize(boolean, int, int, int)
     */
    int ComputePreferredSize(bool width, int availableParallel,
            int availablePerpendicular, int preferredResult);


    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#getSizeFlags(boolean)
     */
    int GetSizeFlags(bool width);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#showPartList()
     */
    void ShowPartList();

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#dispose()
     */
    ~TabbedStackPresentation();

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#setActive(int)
     */
    void SetActive(int newState);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#setVisible(boolean)
     */
    void SetVisible(bool isVisible);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#setState(int)
     */
    void SetState(int state);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#getControl()
     */
    QWidget* GetControl();

    /**
     * @return AbstractTabFolder the presentation's tab folder
     */
    AbstractTabFolder* GetTabFolder();

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#addPart(org.blueberry.ui.presentations.IPresentablePart, java.lang.Object)
     */
    void AddPart(IPresentablePart::Pointer newPart, Object::Pointer cookie);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#movePart(org.blueberry.ui.presentations.IPresentablePart, java.lang.Object)
     */
    void MovePart(IPresentablePart::Pointer toMove, Object::Pointer cookie);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#removePart(org.blueberry.ui.presentations.IPresentablePart)
     */
    void RemovePart(IPresentablePart::Pointer oldPart);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#selectPart(org.blueberry.ui.presentations.IPresentablePart)
     */
    void SelectPart(IPresentablePart::Pointer toSelect);

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#DragOver(QWidget*, const Point&)
     */
    StackDropResult::Pointer DragOver(QWidget* currentControl, const QPoint& location);

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
//     * @see org.blueberry.ui.presentations.StackPresentation#showPaneMenu()
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
     * @see org.blueberry.ui.presentations.StackPresentation#getTabList(org.blueberry.ui.presentations.IPresentablePart)
     */
    QList<QWidget*> GetTabList(IPresentablePart::Pointer part);

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
    QList<IPresentablePart::Pointer> GetPartList();

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

#endif /* BERRYTABBEDSTACKPRESENTATION_H_ */
