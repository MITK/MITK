/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYINESTABLE_H_
#define BERRYINESTABLE_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryObject.h>
#include <berryMacros.h>

namespace berry {

/**
 * </p>
 * A service which can appear on a component which is wholly contained with
 * another component. The component on which it appears can be active or
 * inactive -- depending on the state of the application. For example, a
 * workbench part is a component which appears within a workbench window. This
 * workbench part can either be active or inactive, depending on what the user
 * is doing.
 * </p>
 * <p>
 * Services implement this interface, and are then notified by the component
 * when the activation changes. It is the responsibility of the component to
 * notify such services when the activation changes.
 * </p>
 * <p>
 * This class is not intended for use outside of the
 * <code>org.blueberry.ui.workbench</code> plug-in.
 * </p>
 * <p>
 * <strong>PROVISIONAL</strong>. This class or interface has been added as part
 * of a work in progress. There is a guarantee neither that this API will work
 * nor that it will remain the same. Please do not use this API without
 * consulting with the Platform/UI team.
 * </p>
 * <p>
 * This class should eventually move to <code>org.blueberry.ui.services</code>.
 * </p>
 */
struct BERRY_UI_QT INestable : public virtual Object
{

  berryObjectMacro(berry::INestable);

  ~INestable() override;

  /**
   * Notifies this service that the component within which it exists has
   * become active. The service should modify its state as appropriate.
   *
   */
  virtual void Activate() = 0;

  /**
   * Notifies this service that the component within which it exists has
   * become inactive. The service should modify its state as appropriate.
   */
  virtual void Deactivate() = 0;

};

}

#endif /* BERRYINESTABLE_H_ */
