/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIHANDLERACTIVATION_H_
#define BERRYIHANDLERACTIVATION_H_

#include "internal/berryIEvaluationResultCache.h"

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct IHandler;
struct IHandlerService;

/**
 * <p>
 * A token representing the activation of a handler. This token can later be
 * used to cancel that activation. Without this token, then handler will only
 * become inactive if the component in which the handler was activated is
 * destroyed.
 * </p>
 * <p>
 * This interface is not intended to be implemented or extended by clients.
 * </p>
 *
 * @see org.eclipse.ui.ISources
 * @see org.eclipse.ui.ISourceProvider
 */
struct BERRY_UI_QT IHandlerActivation : public virtual IEvaluationResultCache
{

  berryObjectMacro(berry::IHandlerActivation);

  ~IHandlerActivation() override;

  /**
   * The depth at which the root exists.
   */
  static const int ROOT_DEPTH; // = 1;

  /**
   * Clears the cached computation of the <code>isActive</code> method, if
   * any. This method is only intended for internal use. It provides a
   * mechanism by which <code>ISourceProvider</code> events can invalidate
   * state on a <code>IHandlerActivation</code> instance.
   *
   * @deprecated Use berry::IEvaluationResultCache::clearResult() instead.
   */
  virtual void ClearActive() = 0;

  /**
   * Returns the identifier of the command whose handler is being activated.
   *
   * @return The command identifier; never <code>null</code>.
   */
  virtual QString GetCommandId() const = 0;

  /**
   * Returns the depth at which this activation was created within the
   * services hierarchy. The root of the hierarchy is at a depth of
   * <code>1</code>. This is used as the final tie-breaker in the event
   * that no other method can be used to determine a winner.
   *
   * @return The depth at which the handler was inserted into the services
   *         hierarchy; should be a positive integer.
   */
  virtual int GetDepth() const = 0;

  /**
   * Returns the handler that should be activated.
   *
   * @return The handler; may be <code>null</code>.
   */
  virtual SmartPointer<IHandler> GetHandler() const = 0;

  /**
   * Returns the handler service from which this activation was requested.
   * This is used to ensure that an activation can only be retracted from the
   * same service which issued it.
   *
   * @return The handler service; never <code>null</code>.
   */
  virtual IHandlerService* GetHandlerService() const = 0;

};

}

#endif /* BERRYIHANDLERACTIVATION_H_ */
