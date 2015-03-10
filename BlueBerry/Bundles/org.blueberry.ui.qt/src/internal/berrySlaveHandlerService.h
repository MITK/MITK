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

#ifndef BERRYSLAVEHANDLERSERVICE_H
#define BERRYSLAVEHANDLERSERVICE_H

#include "berryIHandlerService.h"

namespace berry {

/**
 * A handler service which delegates almost all responsibility to the parent
 * service. It is only responsible for disposing of locally activated handlers
 * when it is disposed.
 * <p>
 * This class is not intended for use outside of the
 * <code>org.eclipse.ui.workbench</code> plug-in.
 * </p>
 */
class SlaveHandlerService : public IHandlerService
{

protected:

  /**
   * The default expression to use when
   * {@link #activateHandler(String, IHandler)} is called. Handlers
   * contributed using that method will only be active when this service is
   * active. However, this expression will be used for conflict resolution.
   */
  const SmartPointer<Expression> defaultExpression;

  /**
   * A map of the local activation to the parent activations. If this service
   * is inactive, then all parent activations are <code>null</code>.
   * Otherwise, they point to the corresponding activation in the parent
   * service.
   */
  QMap<SmartPointer<IHandlerActivation>, IHandlerActivation*> localActivationsToParentActivations;

  /**
   * The parent handler service to which all requests are ultimately routed.
   * This value is never <code>null</code>.
   */
  IHandlerService* const parent;

  /**
   * The activations registered with the parent handler service. This value is
   * never <code>null</code>.
   */
  QSet<SmartPointer<IHandlerActivation> > parentActivations;

private:

  /**
   * A collection of source providers. The listeners are not
   * activated/deactivated, but they will be removed when this service is
   * disposed.
   */
  QList<SmartPointer<ISourceProvider> > fSourceProviders;

public:

  /**
   * Constructs a new instance.
   *
   * @param parentHandlerService
   *            The parent handler service for this slave; must not be
   *            <code>null</code>.
   * @param defaultExpression
   *            The default expression to use if none is provided. This is
   *            primarily used for conflict resolution. This value may be
   *            <code>null</code>.
   */
  SlaveHandlerService(IHandlerService* parentHandlerService,
                      const SmartPointer<Expression>& defaultExpression);

  SmartPointer<IHandlerActivation> ActivateHandler(const SmartPointer<IHandlerActivation>& childActivation);

  SmartPointer<IHandlerActivation> ActivateHandler(const QString& commandId,
                                                   const SmartPointer<IHandler>& handler);

  SmartPointer<IHandlerActivation> ActivateHandler(const QString& commandId,
                                                   const SmartPointer<IHandler>& handler,
                                                   const SmartPointer<Expression>& expression);

  SmartPointer<IHandlerActivation> ActivateHandler(const QString& commandId,
                                                   const SmartPointer<IHandler>& handler,
                                                   const SmartPointer<Expression>& expression,
                                                   bool global);

  void AddSourceProvider(const SmartPointer<ISourceProvider>& provider);

  SmartPointer<const ExecutionEvent> CreateExecutionEvent(const SmartPointer<const Command>& command,
                                                          const SmartPointer<const UIElement>& event);

  SmartPointer<const ExecutionEvent> CreateExecutionEvent(
      const SmartPointer<const ParameterizedCommand>& command,
      const SmartPointer<const UIElement>& event);

  void DeactivateHandler(const SmartPointer<IHandlerActivation>& activation);

  void DeactivateHandlers(const QList<SmartPointer<IHandlerActivation> >& activations);

  void Dispose();

  Object::Pointer ExecuteCommand(const SmartPointer<ParameterizedCommand>& command,
                                 const SmartPointer<const UIElement>& event);

  Object::Pointer ExecuteCommand(const QString& commandId,
                                 const SmartPointer<const UIElement>& event);

  SmartPointer<IEvaluationContext> GetCurrentState() const;

  void ReadRegistry();

  void RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider);

  void SetHelpContextId(const SmartPointer<IHandler>& handler,
                        const QString& helpContextId);

  SmartPointer<Expression> GetDefaultExpression() const;

  SmartPointer<IEvaluationContext> CreateContextSnapshot(bool includeSelection);

  Object::Pointer ExecuteCommandInContext(const SmartPointer<ParameterizedCommand>& command,
                                          const SmartPointer<const UIElement>& event,
                                          const SmartPointer<IEvaluationContext>& context);

protected:

  virtual SmartPointer<IHandlerActivation> DoActivation(
      const SmartPointer<IHandlerActivation>& localActivation);
};

}

#endif // BERRYSLAVEHANDLERSERVICE_H
