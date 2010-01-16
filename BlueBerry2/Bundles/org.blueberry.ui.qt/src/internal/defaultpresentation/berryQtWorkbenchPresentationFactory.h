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

#ifndef BERRYQTWORKBENCHPRESENTATIONFACTORY_H_
#define BERRYQTWORKBENCHPRESENTATIONFACTORY_H_

#include <berryIPresentationFactory.h>

namespace berry
{

/**
 * The default presentation factory for the Workbench.
 *
 */
class QtWorkbenchPresentationFactory: public IPresentationFactory
{

  //  // don't reset these dynamically, so just keep the information static.
  //  // see bug:
  //  // 75422 [Presentations] Switching presentation to R21 switches immediately,
  //  // but only partially
  //  private static int editorTabPosition = PlatformUI.getPreferenceStore()
  //      .getInt(IWorkbenchPreferenceConstants.EDITOR_TAB_POSITION);
  //  private static int viewTabPosition = PlatformUI.getPreferenceStore()
  //      .getInt(IWorkbenchPreferenceConstants.VIEW_TAB_POSITION);

public:

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.presentations.AbstractPresentationFactory#createEditorPresentation(org.blueberry.swt.widgets.Composite,
   *      org.blueberry.ui.presentations.IStackPresentationSite)
   */
  StackPresentation::Pointer CreateEditorPresentation(void* parent,
      IStackPresentationSite::Pointer site);

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.presentations.AbstractPresentationFactory#createViewPresentation(org.blueberry.swt.widgets.Composite,
   *      org.blueberry.ui.presentations.IStackPresentationSite)
   */
  StackPresentation::Pointer CreateViewPresentation(void* parent,
      IStackPresentationSite::Pointer site);

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.presentations.AbstractPresentationFactory#createStandaloneViewPresentation(org.blueberry.swt.widgets.Composite,
   *      org.blueberry.ui.presentations.IStackPresentationSite, boolean)
   */
  StackPresentation::Pointer CreateStandaloneViewPresentation(void* parent,
      IStackPresentationSite::Pointer site, bool showTitle);

  std::string GetId();

  void* CreateSash(void* parent, int style);

  int GetSashSize(int style);

  void UpdateTheme();

};

}

#endif /* BERRYQTWORKBENCHPRESENTATIONFACTORY_H_ */
