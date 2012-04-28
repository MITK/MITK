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


#ifndef MITKIZOMBIEVIEWPART_H
#define MITKIZOMBIEVIEWPART_H

#include "mitkILifecycleAwarePart.h"

namespace mitk {

/**
 * \ingroup org_mitk_gui_common
 *
 * \brief Interface for MITK Workbench Views with special \e Zombie state handling.
 *
 * Some MITK Views need to change the application state in a way which should not be affected
 * by this View's activated/deactivated state. Examples are setting special interactors or removing
 * the crosshair from a QmitkRenderWindow. Such MITK Views are called \emph{Zombie Views} because
 * they partially outlive their own deactivation. Zombie Views are usually mutually exclusive.
 *
 * To ease the handling of Zombie Views, this interface acts as a marker to declare the implementing
 * class being a Zombie View, meaning that it will change some application state when it is activated
 * but will not reset it when deactivated.
 *
 * If a Zombie View is activated, the previously active Zombie View is notified by calling its
 * ActivatedZombieView() method. It should then reset the application state.
 *
 * \see ILifecycleAwarePart
 */
struct IZombieViewPart : public virtual ILifecycleAwarePart
{

  /**
   * Called when another Zombie View was activated. This usually means that this part
   * should clean up its Zombie state.
   *
   * \param zombieView The newly activate Zombie View.
   */
  virtual void ActivatedZombieView(berry::IWorkbenchPartReference::Pointer zombieView) = 0;

};

}

#endif // MITKIZOMBIEVIEWPART_H
