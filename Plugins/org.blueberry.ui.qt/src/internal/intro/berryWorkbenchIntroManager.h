/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHINTROMANAGER_H_
#define BERRYWORKBENCHINTROMANAGER_H_

#include <intro/berryIIntroManager.h>

#include "berryIWorkbenchPart.h"

namespace berry
{

struct IWorkbenchWindow;
class Workbench;

/**
 * Workbench implementation of the IIntroManager interface.
 *
 * @since 3.0
 */
class WorkbenchIntroManager: public IIntroManager
{

private:

  Workbench* workbench;

  /**
   * The currently active introPart in this workspace, <code>null</code> if none.
   */
  IIntroPart::Pointer introPart;

  /**
   * Create a new Intro area (a view, currently) in the provided window.  If there is no intro
   * descriptor for this workbench then no work is done.
   *
   * @param preferredWindow the window to create the intro in.
   */
  void CreateIntro(SmartPointer<IWorkbenchWindow> preferredWindow);

  bool IntroIsView() const;

public:

  /**
   * Create a new instance of the receiver.
   *
   * @param workbench the workbench instance
   */
  WorkbenchIntroManager(Workbench* workbench);

  /* (non-Javadoc)
   * @see org.eclipse.ui.IWorkbench#closeIntro(org.eclipse.ui.intro.IIntroPart)
   */
  bool CloseIntro(IIntroPart::Pointer part) override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.IWorkbench#showIntro(org.eclipse.ui.IWorkbenchWindow)
   */
  IIntroPart::Pointer ShowIntro(SmartPointer<IWorkbenchWindow> preferredWindow,
      bool standby) override;

  /**
   * @param testWindow the window to test
   * @return whether the intro exists in the given window
   */
  bool IsIntroInWindow(SmartPointer<IWorkbenchWindow> testWindow) const;

  /* (non-Javadoc)
   * @see org.eclipse.ui.IWorkbench#setIntroStandby(org.eclipse.ui.intro.IIntroPart, boolean)
   */
  void SetIntroStandby(IIntroPart::Pointer part, bool standby) override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.IWorkbench#isIntroStandby(org.eclipse.ui.intro.IIntroPart)
   */
  bool IsIntroStandby(IIntroPart::Pointer part) const override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.IWorkbench#findIntro()
   */
  IIntroPart::Pointer GetIntro() const override;

  /**
   * @return the <code>ViewIntroAdapterPart</code> for this workbench, <code>null</code> if it
   * cannot be found.
   */
  IWorkbenchPart::Pointer GetIntroAdapterPart() const;

  /**
   * @return a new IIntroPart.  This has the side effect of setting the introPart field to the new
   * value.
   */
  IIntroPart::Pointer CreateNewIntroPart();

  /* (non-Javadoc)
   * @see org.eclipse.ui.IWorkbench#hasIntro()
   */
  bool HasIntro() const override;

  bool IsNewContentAvailable() override;

};

}

#endif /* BERRYWORKBENCHINTROMANAGER_H_ */
