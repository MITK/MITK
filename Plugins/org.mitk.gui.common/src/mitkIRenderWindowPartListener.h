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

#ifndef MITKIRENDERWINDOWPARTLISTENER_H
#define MITKIRENDERWINDOWPARTLISTENER_H

#include <org_mitk_gui_common_Export.h>

namespace mitk {

  struct IRenderWindowPart;

  /**
   * \ingroup org_mitk_gui_common
   *
   * \brief Interface for berry::IViewPart implementations to be notified about mitk::IRenderWindowPart lifecycle changes.
   *
   * This interface is intended to be implemented by subclasses of berry::IWorkbenchPart. If implemented,
   * the interface methods are called automatically if a Workbench part which implementes mitk::IRenderWindowPart
   * is activated or deactivated.
   *
   * The notion of activated and deactivated is slightly different from the usual Workbench part lifecycle.
   */
  struct MITK_GUI_COMMON_PLUGIN IRenderWindowPartListener
  {
    virtual ~IRenderWindowPartListener();

    /**
     * Called when an IRenderWindowPart is activated or if it becomes visible and no
     * other IRenderWindowPart was activated before.
     *
     * \param renderWindowPart The newly activated IRenderWindowPart.
     */
    virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) = 0;

    /**
     * Called when an IRenderWindowPart becomes invisible and if it was active before.
     *
     * \param renderWindowPart The deactivated IRenderWindowPart.
     */
    virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) = 0;

    /**
    * Called when an IRenderWindowPart changes and if it was active before.
    *
    * \param renderWindowPart The modified IRenderWindowPart.
    */
    virtual void RenderWindowPartInputChanged(mitk::IRenderWindowPart* /*renderWindowPart*/) {};
  };
}

#endif // MITKIRENDERWINDOWPARTLISTENER_H
