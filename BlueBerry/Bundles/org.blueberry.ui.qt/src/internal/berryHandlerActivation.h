/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYHANDLERACTIVATION_H
#define BERRYHANDLERACTIVATION_H

#include "berryIHandlerActivation.h"
#include "berryEvaluationResultCache.h"
#include "berryIEvaluationReference.h"
#include "berryIPropertyChangeListener.h"

namespace berry {

struct IEvaluationReference;
struct IPropertyChangeListener;

/**
 * <p>
 * A token representing the activation of a handler. This token can later be
 * used to cancel that activation. Without this token, then handler will only
 * become inactive if the component in which the handler was activated is
 * destroyed.
 * </p>
 * <p>
 * This caches the command id and the handler, so that they can later be
 * identified.
 * </p>
 * <p>
 * <b>Note:</b> this class has a natural ordering that is inconsistent with
 * equals.
 * </p>
 */
class HandlerActivation : public EvaluationResultCache, public IHandlerActivation
{

public:

  berryObjectMacro(berry::HandlerActivation)

private:

  /**
   * The identifier for the command which the activated handler handles. This
   * value is never <code>null</code>.
   */
  const QString commandId;

  /**
   * The depth of services at which this token was created. This is used as a
   * final tie-breaker if all other things are equivalent.
   */
  const int depth;

  /**
   * The handler that has been activated. This value may be <code>null</code>.
   */
  SmartPointer<IHandler> handler;

  /**
   * The handler service from which this handler activation was request. This
   * value is never <code>null</code>.
   */
  IHandlerService* handlerService;

  SmartPointer<IEvaluationReference> reference;

  QScopedPointer<IPropertyChangeListener> listener;

public:

  /**
   * Constructs a new instance of <code>HandlerActivation</code>.
   *
   * @param commandId
   *            The identifier for the command which the activated handler
   *            handles. This value must not be <code>null</code>.
   * @param handler `
   *            The handler that has been activated. This value may be
   *            <code>null</code>.
   * @param expression
   *            The expression that must evaluate to <code>true</code>
   *            before this handler is active. This value may be
   *            <code>null</code> if it is always active.</code>.
   * @param depth
   *            The depth at which this activation was created within the
   *            services hierarchy. This is used as the final tie-breaker if
   *            all other conditions are equal. This should be a positive
   *            integer.
   * @param handlerService
   *            The handler service from which the handler activation was
   *            requested; must not be <code>null</code>.
   * @see ISources
   */
  HandlerActivation(const QString& commandId, const SmartPointer<IHandler>& handler,
                    const SmartPointer<Expression>& expression, int depth,
                    IHandlerService* handlerService);

  void ClearActive();

  /**
   * Implement {@link Object#operator<(const Object*)}.
   * <p>
   * <b>Note:</b> this class has a natural ordering that is inconsistent with
   * equals.
   * </p>
   */
  bool operator<(const Object* object) const;

  int CompareTo(const HandlerActivation* other) const;

  QString GetCommandId() const;

  int GetDepth() const;

  SmartPointer<IHandler> GetHandler() const;

  IHandlerService* GetHandlerService() const;

  QString ToString() const;

  /**
   * @return Returns the reference.
   */
  SmartPointer<IEvaluationReference> GetReference();

  /**
   * @param reference
   *            The reference to set.
   */
  void SetReference(const SmartPointer<IEvaluationReference>& reference);

  /**
   * @param listener The listener to set.
   */
  void SetListener(IPropertyChangeListener* listener);

  /**
   * @return Returns the listener.
   */
  IPropertyChangeListener* GetListener() const;
};

}

#endif // BERRYHANDLERACTIVATION_H
