/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef MITKILIFECYCLEAWAREPART_H
#define MITKILIFECYCLEAWAREPART_H

#include <org_mitk_gui_common_Export.h>


namespace mitk {

/**
 * \ingroup org_mitk_gui_common
 *
 * \brief Interface for a MITK Workbench part which wants to be notified about lifecycle changes.
 *
 * This interface is intended to be implemented by subclasses of berry::IWorkbenchPart. MITK Workbench parts
 * implementing this interface will be notified about their lifecycle changes. The same effect could
 * be achieved by registering a custom berry::IPartListener.
 *
 * \note There are no "Closed()" or "Opened()" methods. These correspond to the constructor and
 *       desctructor of the Workbench part class.
 *
 * \see berry::IPartListener
 * \see mitk::IZombieViewPart
 */
struct MITK_GUI_COMMON_PLUGIN ILifecycleAwarePart
{
  virtual ~ILifecycleAwarePart();

  /** \see berry::IPartListener::PartActivated */
  virtual void Activated() = 0;

  /** \see berry::IPartListener::PartDeactivated */
  virtual void Deactivated() = 0;

  /** \see berry::IPartListener::PartVisible */
  virtual void Visible() = 0;

  /** \see berry::IPartListener::PartHidden */
  virtual void Hidden() = 0;
};

}

#endif // MITKILIFECYCLEAWAREPART_H
