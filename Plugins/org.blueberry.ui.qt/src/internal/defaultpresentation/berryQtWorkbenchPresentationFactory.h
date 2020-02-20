/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  StackPresentation::Pointer CreateEditorPresentation(QWidget* parent,
      IStackPresentationSite::Pointer site) override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.presentations.AbstractPresentationFactory#createViewPresentation(org.blueberry.swt.widgets.Composite,
   *      org.blueberry.ui.presentations.IStackPresentationSite)
   */
  StackPresentation::Pointer CreateViewPresentation(QWidget* parent,
      IStackPresentationSite::Pointer site) override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.presentations.AbstractPresentationFactory#createStandaloneViewPresentation(org.blueberry.swt.widgets.Composite,
   *      org.blueberry.ui.presentations.IStackPresentationSite, boolean)
   */
  StackPresentation::Pointer CreateStandaloneViewPresentation(QWidget* parent,
      IStackPresentationSite::Pointer site, bool showTitle) override;

  QString GetId() override;

  QWidget* CreateSash(QWidget* parent, int style) override;

  int GetSashSize(int style) override;

  void UpdateTheme() override;

};

}

#endif /* BERRYQTWORKBENCHPRESENTATIONFACTORY_H_ */
