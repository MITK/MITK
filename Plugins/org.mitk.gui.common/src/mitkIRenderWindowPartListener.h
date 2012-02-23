/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKIRENDERWINDOWPARTLISTENER_H
#define MITKIRENDERWINDOWPARTLISTENER_H

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
struct IRenderWindowPartListener
{
  virtual ~IRenderWindowPartListener() {}

  /**
   * Called when a IRenderWindowPart is activated or if it becomes visible and no
   * other IRenderWindowPart was activated before.
   *
   * \param renderWindowPart The newly activated IRenderWindowPart.
   */
  virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) = 0;
  
  /**
   * Called when a IRenderWindowPart becomes invisible and if it was active before.
   *
   * \param renderWindowPart The deactivated IRenderWindowPart.
   */
  virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) = 0;
};

}

#endif // MITKIRENDERWINDOWPARTLISTENER_H
