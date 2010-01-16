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


#ifndef BERRYIHANDLERACTIVATION_H_
#define BERRYIHANDLERACTIVATION_H_

#include "../internal/berryIEvaluationResultCache.h"

#include "../berryUiDll.h"

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
 * @since 3.1
 * @see org.eclipse.ui.ISources
 * @see org.eclipse.ui.ISourceProvider
 */
struct BERRY_UI IHandlerActivation : public IEvaluationResultCache {

  berryInterfaceMacro(IHandlerActivation, berry)

  /**
   * The depth at which the root exists.
   *
   * @since 3.2
   */
  static const int ROOT_DEPTH; // = 1;

  /**
   * Clears the cached computation of the <code>isActive</code> method, if
   * any. This method is only intended for internal use. It provides a
   * mechanism by which <code>ISourceProvider</code> events can invalidate
   * state on a <code>IHandlerActivation</code> instance.
   *
   * @deprecated Use {@link IEvaluationResultCache#clearResult()} instead.
   */
  virtual void ClearActive() = 0;

  /**
   * Returns the identifier of the command whose handler is being activated.
   *
   * @return The command identifier; never <code>null</code>.
   */
  virtual std::string GetCommandId() const = 0;

  /**
   * Returns the depth at which this activation was created within the
   * services hierarchy. The root of the hierarchy is at a depth of
   * <code>1</code>. This is used as the final tie-breaker in the event
   * that no other method can be used to determine a winner.
   *
   * @return The depth at which the handler was inserted into the services
   *         hierarchy; should be a positive integer.
   * @since 3.2
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
  virtual SmartPointer<IHandlerService> GetHandlerService() const = 0;

  /**
   * Returns whether this handler activation is currently active -- given the
   * current state of the workbench. This method should cache its computation.
   * The cache will be cleared by a call to <code>clearActive</code>.
   *
   * @param context
   *            The context in which this state should be evaluated; must not
   *            be <code>null</code>.
   * @return <code>true</code> if the activation is currently active;
   *         <code>false</code> otherwise.
   * @deprecated Use
   *             {@link IEvaluationResultCache#evaluate(IEvaluationContext)}
   *             instead.
   */
  virtual bool IsActive(SmartPointer<IEvaluationContext> context) const = 0;
};

}

#endif /* BERRYIHANDLERACTIVATION_H_ */
