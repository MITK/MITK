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

#ifndef BERRYIINTROMANAGER_H_
#define BERRYIINTROMANAGER_H_

#include "berryIIntroPart.h"

#include <berryIWorkbenchWindow.h>

namespace berry
{

/**
 * Manages the intro part that introduces the product to new users.
 * The intro part is typically shown the first time a product is started up.
 * <p>
 * The initial behavior of the intro part is controlled by the application
 * from via the {@link org.eclipse.ui.application.WorkbenchWindowAdvisor#openIntro()}
 * method.
 * </p>
 * <p>
 * See {@link org.eclipse.ui.intro.IIntroPart} for details on where intro parts
 * come from.
 * </p>
 * <p>
 * This interface is not intended to be extended or implemented by clients.
 * </p>
 *
 * @see org.eclipse.ui.IWorkbench#getIntroManager()
 * @since 3.0
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI IIntroManager
{

  virtual ~IIntroManager() {}
  /**
   * Closes the given intro part.
   *
   * @param part the intro part
   * @return <code>true</code> if the intro part was closed, and
   * <code>false</code> otherwise.  <code>false</code> is returned
   * if part is <code>null</code> or it is not the intro part returned
   * by {@link #getIntro()}.
   */
  virtual bool CloseIntro(IIntroPart::Pointer part) = 0;

  /**
   * Returns the intro part. Returns <code>null</code> if there is no intro
   * part, if it has been previously closed via {@link #closeIntro(IIntroPart)}
   * or if there is an intro part but {@link #showIntro(IWorkbenchWindow, boolean)}
   * has not yet been called to create it.
   *
   * @return the intro part, or <code>null</code> if none is available
   */
  virtual IIntroPart::Pointer GetIntro() const = 0;

  /**
   * Return whether an intro is available. Note that this checks whether
   * there is an applicable intro part that could be instantiated and shown
   * to the user.
   * Use {@link #getIntro()} to discover whether an intro part has already
   * been created.
   *
   * @return <code>true</code> if there is an intro that could be shown, and
   * <code>false</code> if there is no intro
   */
  virtual bool HasIntro() const = 0;

  /**
   * Return the standby state of the given intro part.
   *
   * @param part the intro part
   * @return <code>true</code> if the part in its partially
   * visible standy mode, and <code>false</code> if in its fully visible state.
   * <code>false</code> is returned if part is <code>null</code> or it is not
   * the intro part returned by {@link #getIntro()}.
   */
  virtual bool IsIntroStandby(IIntroPart::Pointer part) const = 0;

  /**
   * Sets the standby state of the given intro part. Intro part usually should
   * render themselves differently in the full and standby modes. In standby
   * mode, the part should be partially visible to the user but otherwise
   * allow them to work. In full mode, the part should be fully visible and
   * be the center of the user's attention.
   * <p>
   * This method does nothing if the part is <code>null</code> or is not
   * the intro part returned by {@link #getIntro()}.
   * </p>
   *
   * @param part the intro part, or <code>null</code>
   * @param standby <code>true</code> to put the part in its partially
   * visible standy mode, and <code>false</code> to make it fully visible.
   */
  virtual void SetIntroStandby(IIntroPart::Pointer part, bool standby) = 0;

  /**
   * Shows the intro part in the given workbench window. If the intro part has
   * not been created yet, one will be created. If the intro part is currently
   * being shown in some workbench window, that other window is made active.
   *
   * @param preferredWindow the preferred workbench window, or
   * <code>null</code> to indicate the currently active workbench window
   * @param standby <code>true</code> to put the intro part in its partially
   * visible standy mode, and <code>false</code> to make it fully visible
   * @return the newly-created or existing intro part, or <code>null</code>
   * if no intro part is available or if <code>preferredWindow</code> is
   * <code>null</code> and there is no currently active workbench window
   */
  virtual IIntroPart::Pointer ShowIntro(
      IWorkbenchWindow::Pointer preferredWindow, bool standby) = 0;

  /**
   * Returns <code>true</code> if there is an intro content detector and it
   * reports that new intro content is available.
   *
   * @return <code>true</code> if new intro content is available
   *
   * @since 3.3
   */
  virtual bool IsNewContentAvailable() = 0;
};

}

#endif /* BERRYIINTROMANAGER_H_ */
