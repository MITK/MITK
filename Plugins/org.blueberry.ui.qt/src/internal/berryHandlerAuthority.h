/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHANDLERAUTHORITY_H
#define BERRYHANDLERAUTHORITY_H

#include "berryIPropertyChangeListener.h"

#include <QMap>
#include <QHash>

namespace berry {

template<class T> class SmartPointer;

struct ICommandService;
struct IEvaluationContext;
struct IEvaluationResultCache;
struct IEvaluationService;
struct IHandler;
struct IHandlerActivation;
struct IServiceLocator;

class HandlerActivation;
class MultiStatus;
class Shell;

/**
 * <p>
 * A central authority for resolving conflicts between handlers. This authority
 * listens to a variety of incoming sources, and updates the underlying commands
 * if changes in the active handlers occur.
 * </p>
 * <p>
 * This authority encapsulates all of the handler conflict resolution mechanisms
 * for the workbench. A conflict occurs if two or more handlers are assigned to
 * the same command identifier. To resolve this conflict, the authority
 * considers which source the handler came from.
 * </p>
 */
class HandlerAuthority : private IPropertyChangeListener
{

private:

  /**
   * The component name to print when displaying tracing information.
   */
  static const QString TRACING_COMPONENT; // = "HANDLERS";

  static const QList<QString> SELECTION_VARIABLES;

  /**
   * The command service that should be updated when the handlers are
   * changing. This value is never <code>null</code>.
   */
  ICommandService* const commandService;

  struct Empty {};
  typedef QHash<QString, QMap<SmartPointer<HandlerActivation>, Empty> > IdToHandlerActivationMap;
  /**
   * This is a map of handler activations (<code>SortedSet</code> of
   * <code>IHandlerActivation</code>) sorted by command identifier (<code>String</code>).
   * If there is only one handler activation for a command, then the
   * <code>SortedSet</code> is replaced by a <code>IHandlerActivation</code>.
   * If there is no activation, the entry should be removed entirely.
   */
  IdToHandlerActivationMap handlerActivationsByCommandId;

  QSet<QString> previousLogs;

  IServiceLocator* locator;

  QSet<QString> changedCommandIds;

  mutable IEvaluationService* evalService;

  IEvaluationService* GetEvaluationService() const;

  IPropertyChangeListener* GetServiceListener() const;

public:

  /**
   * Whether the workbench command support should kick into debugging mode.
   * This causes the unresolvable handler conflicts to be printed to the
   * console.
   */
  static bool DEBUG; // = Policy.DEBUG_HANDLERS;

  /**
   * Whether the performance information should be printed about the
   * performance of the handler authority.
   */
  static bool DEBUG_PERFORMANCE; // = Policy.DEBUG_HANDLERS_PERFORMANCE;

  /**
   * Whether the workbench command support should kick into verbose debugging
   * mode. This causes the resolvable handler conflicts to be printed to the
   * console.
   */
  static bool DEBUG_VERBOSE; // = Policy.DEBUG_HANDLERS && Policy.DEBUG_HANDLERS_VERBOSE;

  /**
   * The command identifier to which the verbose output should be restricted.
   */
  static QString DEBUG_VERBOSE_COMMAND_ID; // = Policy.DEBUG_HANDLERS_VERBOSE_COMMAND_ID;


  /**
   * Constructs a new instance of <code>HandlerAuthority</code>.
   *
   * @param commandService
   *            The command service from which commands can be retrieved (to
   *            update their handlers); must not be <code>null</code>.
   * @param locator
   *            the appropriate service locator
   */
  HandlerAuthority(ICommandService* commandService,
                   IServiceLocator* locator);

  ~HandlerAuthority() override;

  /**
   * Activates a handler on the workbench. This will add it to a master list.
   * If conflicts exist, they will be resolved based on the source priority.
   * If conflicts still exist, then no handler becomes active.
   *
   * @param activation
   *            The activation; must not be <code>null</code>.
   */
  void ActivateHandler(const SmartPointer<IHandlerActivation>& activation);

  /**
   * Removes an activation for a handler on the workbench. This will remove it
   * from the master list, and update the appropriate command, if necessary.
   *
   * @param activation
   *            The activation; must not be <code>null</code>.
   */
  void DeactivateHandler(const SmartPointer<IHandlerActivation>& activation);

private:

  /**
   * Resolves conflicts between multiple handlers for the same command
   * identifier. This tries to select the best activation based on the source
   * priority. For the sake of comparison, activations with the same handler
   * are considered equivalent (i.e., non-conflicting).
   *
   * @param commandId
   *            The identifier of the command for which the conflicts should
   *            be detected; must not be <code>null</code>. This is only
   *            used for debugging purposes.
   * @param activations
   *            All of the possible handler activations for the given command
   *            identifier; must not be <code>null</code>.
   * @return The best matching handler activation. If none can be found (e.g.,
   *         because of unresolvable conflicts), then this returns
   *         <code>null</code>.
   */
  SmartPointer<IHandlerActivation> ResolveConflicts(
      const QString& commandId,
      const QMap<SmartPointer<HandlerActivation>,Empty>& activations,
      SmartPointer<MultiStatus> conflicts);

  /**
   * Updates the command with the given handler activation.
   *
   * @param commandId
   *            The identifier of the command which should be updated; must
   *            not be <code>null</code>.
   * @param activation
   *            The activation to use; may be <code>null</code> if the
   *            command should have a <code>null</code> handler.
   */
  void UpdateCommand(const QString& commandId,
                     const SmartPointer<IHandlerActivation>& activation);

public:

  /**
   * Currently this is a an internal method to help locate a handler.
   * <p>
   * DO NOT CALL THIS METHOD.
   * </p>
   *
   * @param commandId
   *            the command id to check
   * @param context
   *            the context to use for activations
   */
  SmartPointer<IHandler> FindHandler(const QString& commandId,
                                     IEvaluationContext* context);

  SmartPointer<IEvaluationContext> CreateContextSnapshot(bool includeSelection);

private:

  using IPropertyChangeListener::PropertyChange;
  void PropertyChange(const SmartPointer<PropertyChangeEvent>& event) override;

  /**
   * Evaluate the expression for the handler and bypass the result cache.
   * <p>
   * DO NOT CALL THIS METHOD.
   * </p>
   *
   * @param context
   * @param activation
   * @return <code>true</code> if the handler expression can evaluate to
   *         true.
   * @throws CoreException
   */
  bool Eval(IEvaluationContext* context,
            const SmartPointer<IHandlerActivation>& activation);

  bool IsSelectionVariable(const QString& name);

  void CopyVariable(IEvaluationContext* context,
                    IEvaluationContext* tmpContext, const QString& var);

  class HandlerPropertyListener;

  void ProcessChangedCommands();

protected:

  bool Evaluate(const SmartPointer<IEvaluationResultCache>& expression);

public:

  SmartPointer<IEvaluationContext> GetCurrentState() const;

//  void UpdateShellKludge();

//  void UpdateShellKludge(const SmartPointer<Shell>& shell);
};

}

#endif // BERRYHANDLERAUTHORITY_H
