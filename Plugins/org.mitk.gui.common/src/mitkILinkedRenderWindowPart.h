/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKILINKEDRENDERWINDOWPART_H
#define MITKILINKEDRENDERWINDOWPART_H

#include "mitkIRenderWindowPart.h"

namespace mitk {

class SlicesRotator;
class SlicesSwiveller;

/**
 * \ingroup org_mitk_gui_common
 *
 * \brief Extends the IRenderWindowPart interface with methods for controlling linked
 *        render windows.
 *
 * This inteface should be implemented by subclasses of berry::IWorkbenchPart if they
 * provided several linked QmitkRenderWindow instances.
 */
struct MITK_GUI_COMMON_PLUGIN ILinkedRenderWindowPart : public virtual IRenderWindowPart {

  ~ILinkedRenderWindowPart();

  /**
   * Get the mitk::SlicesRotator.
   *
   * \return A mitk::SlicesRotator instance if rotating slices is supported;
   *         <code>NULL</code> otherwise.
   */
  virtual mitk::SlicesRotator* GetSlicesRotator() const = 0;

  /**
   * Get the mitk::SlicesSwiveller.
   *
   * \return A mitk::SlicesSwiveller instance if swivelling slices is supported;
   *         <code>NULL</code> otherwise.
   */
  virtual mitk::SlicesSwiveller* GetSlicesSwiveller() const = 0;

  /**
   * Enable or disable the slicing planes linking the QmitkRenderWindow instances.
   *
   * \param enable <code>true</code> if the slicing planes should be enabled;
   *        <code>false</code> otherwise.
   */
  virtual void EnableSlicingPlanes(bool enable) = 0;

  /**
   * Get the enabled status of the slicing planes.
   *
   * \return <code>true</code> if the slicing planes are enabled; <code>false</code>
   *         otherwise.
   */
  virtual bool IsSlicingPlanesEnabled() const = 0;

  /**
   * Enable or disalbe linked navigation.
   *
   * \param enable If <code>true</code>, setting the selected position in one QmitkRenderWindow
   *        affects the positions in the linked windows. If <code>false</code>, linked navigation
   *        is disabled.
   */
  virtual void EnableLinkedNavigation(bool enable) = 0;

  /**
   * Get the enabled status of linked navigation.
   *
   * \return <code>true</code> if linked navigation is enabled; <code>false</code>
   *         otherwise.
   */
  virtual bool IsLinkedNavigationEnabled() const = 0;
};

}

#endif // MITKILINKEDRENDERWINDOWPART_H
