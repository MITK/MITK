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

#ifndef CHERRYMOCKVIEWPART_H_
#define CHERRYMOCKVIEWPART_H_

#include "cherryMockWorkbenchPart.h"

#include <cherryIViewPart.h>
#include <cherryUIException.h>


namespace cherry
{

class MockViewPart: public MockWorkbenchPart, public IViewPart
{
public:

  cherryObjectMacro(MockViewPart)

  static const std::string ID; // = "org.opencherry.ui.tests.api.MockViewPart";
  static const std::string ID2; // = ID + "2";
  static const std::string ID3; // = ID + "3";
  static const std::string ID4; // = ID + "4";
  static const std::string IDMULT; // = ID + "Mult";
  static const std::string NAME; // = "Mock View 1";


  /**
   * @see IViewPart#getViewSite()
   */
  IViewSite::Pointer GetViewSite();

  /**
   * @see IViewPart#Init(IViewSite, IMemento)
   */
  void Init(IViewSite::Pointer site, IMemento::Pointer memento)
      throw (PartInitException);

  /* (non-Javadoc)
   * @see org.eclipse.ui.tests.api.MockWorkbenchPart#createPartControl(org.eclipse.swt.widgets.Composite)
   */
  void CreatePartControl(void* parent);

  /* (non-Javadoc)
   * @see org.eclipse.ui.tests.api.MockPart#dispose()
   */
  ~MockViewPart();

  void ToolbarContributionItemWidgetDisposed();

  void ToolbarContributionItemDisposed();

  /**
   * @see IViewPart#saveState(IMemento)
   */
  void SaveState(IMemento::Pointer memento);

  /**
   * @see IWorkbenchPart#GetSite()
   */
  IWorkbenchPartSite::Pointer GetSite() const;

  /**
   * @see IWorkbenchPart#GetTitle()
   */
  std::string GetPartName() const;

  std::string GetContentDescription() const;

  /**
   * @see IWorkbenchPart#GetTitleToolTip()
   */
  std::string GetTitleToolTip() const;

  std::string GetPartProperty(const std::string& key) const;

  void SetPartProperty(const std::string& key, const std::string& value);

  const std::map<std::string, std::string>& GetPartProperties() const;

private:

  //  ContributionItem toolbarItem = new ContributionItem("someId") {
  //
  //      private DisposeListener disposeListener = new DisposeListener() {
  //        public void widgetDisposed(DisposeEvent e) {
  //          toolbarContributionItemWidgetDisposed();
  //        }
  //
  //       public void fill(ToolBar parent, int index) {
  //        super.fill(parent, index);
  //
  //        ToolItem item = new ToolItem(parent, index);
  //
  //        item.addDisposeListener(disposeListener);
  //        item.setImage(WorkbenchImages.getImage(ISharedImages.IMG_DEF_VIEW));
  //      }
  //
  //      public void dispose() {
  //        toolbarContributionItemDisposed();
  //        super.dispose();
  //      }
  //    };
  //
  //  class DummyAction extends Action {
  //      public DummyAction() {
  //        setText("Monkey");
  //      setImageDescriptor(getViewSite().getWorkbenchWindow()
  //          .getWorkbench().getSharedImages()
  //          .getImageDescriptor(
  //              ISharedImages.IMG_TOOL_DELETE));
  //    }
  //    };
  //
  //
  //
  //  void AddToolbarContributionItem() {
  //      getViewSite().getActionBars().getToolBarManager().add(toolbarItem);
  //    }


protected:

  /* (non-Javadoc)
   * @see org.eclipse.ui.tests.api.MockWorkbenchPart#getActionBars()
   */
  //   IActionBars GetActionBars() {
  //        return getViewSite().getActionBars();
  //    }
};

}

#endif /* CHERRYMOCKVIEWPART_H_ */
