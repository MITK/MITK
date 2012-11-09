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

#ifndef BERRYQTWORKBENCHPRESENTATIONFACTORY_H_
#define BERRYQTWORKBENCHPRESENTATIONFACTORY_H_

#include <berryIPresentationFactory.h>

namespace berry
{

/**
 * The default presentation factory for the Workbench.
 *
 */
class QtWorkbenchPresentationFactory: public QObject, public IPresentationFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPresentationFactory)

  //  // don't reset these dynamically, so just keep the information static.
  //  // see bug:
  //  // 75422 [Presentations] Switching presentation to R21 switches immediately,
  //  // but only partially
  //  private static int editorTabPosition = PlatformUI.getPreferenceStore()
  //      .getInt(IWorkbenchPreferenceConstants.EDITOR_TAB_POSITION);
  //  private static int viewTabPosition = PlatformUI.getPreferenceStore()
  //      .getInt(IWorkbenchPreferenceConstants.VIEW_TAB_POSITION);

public:

  QtWorkbenchPresentationFactory();
  QtWorkbenchPresentationFactory(const QtWorkbenchPresentationFactory& other);

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
