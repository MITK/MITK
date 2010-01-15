/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYIEVALUATIONSERVICE_H_
#define CHERRYIEVALUATIONSERVICE_H_

#include "cherryIServiceWithSources.h"

namespace cherry {

class Expression;
struct IEvaluationContext;
struct IEvaluationReference;
struct IPropertyChangeListener;

/**
 * Evaluate a core expression against the workbench application context and
 * report updates using a Boolean property. Clients supply an
 * <code>IPropertyChangeListener</code> that will be notified as changes
 * occur.
 * <p>
 * This can be used to implement core expressions in client extension points
 * similar to the &lt;enabledWhen&gt; of
 * <code>org.eclipse.ui.handlers/handler</code> elements.
 * </p>
 * <p>
 * The service will fire <code>Boolean.TRUE</code> and
 * <code>Boolean.FALSE</code> for the oldValue and newValue in the property
 * change events.
 * </p>
 * <p>
 * Adding the evaluation listener will fire one change with oldValue=<code>null</code>
 * and newValue=&quot;evaluated expression&quot;. Remove the
 * <code>IEvaluationReference</code> will fire one change with
 * oldValue=&quot;last evaluated value&quot; and newValue=<code>null</code>.
 * </p>
 * <p>
 * Adding a service listener will fire the {@link #PROP_NOTIFYING} property
 * change event with newValue=<code>Boolean.TRUE</code> when a source change
 * causes expression evaluations to update and another {@link #PROP_NOTIFYING}
 * property change event with newValue=<code>Boolean.FALSE</code> when the
 * changes that started with a specific source change have finished. The
 * {@link #PROP_NOTIFYING} change events will not be fired for source changes
 * caused by the outer most recalculations.
 * </p>
 * <p>
 * Variable sources can be provided to this service using the <code>org.eclipse.ui.services</code>
 * Extension Point.  This makes the available to &lt;with/&gt; expressions.
 * </p>
 * <p>
 * This service can be acquired from your service locator:
 * <pre>
 *  IEvaluationService service = (IEvaluationService) getSite().getService(IEvaluationService.class);
 * </pre>
 * <ul>
 * <li>This service is available globally.</li>
 * </ul>
 * </p>
 *
 * @noextend This interface is not intended to be extended by clients.
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct IEvaluationService : public IServiceWithSources {

  /**
   * A general property that can be used.
   */
  static const std::string RESULT; // = "org.eclipse.ui.services.result"; //$NON-NLS-1$

  /**
   * The property used to notify any service listeners.
   */
  static const std::string PROP_NOTIFYING; // = "org.eclipse.ui.services.notifying"; //$NON-NLS-1$

  /**
   * When a source change starts recalculating expressions the
   * {@link #PROP_NOTIFYING} property change is fired with the newValue=<code>Boolean.TRUE</code>.
   * This property is not fired for any source changes caused by the outer
   * recalculations.
   * <p>
   * <b>Note:</b> listeners should be removed when no longer necessary. If
   * not, they will be removed when the IServiceLocator used to acquire this
   * service is disposed.
   * </p>
   *
   * @param listener
   *            The listener to be notified. Must not be <code>null</code>.
   *            Has no effect if the listener has already been added.
   */
  virtual void AddServiceListener(SmartPointer<IPropertyChangeListener> listener) = 0;

  /**
   * Remove the listener for {@link #PROP_NOTIFYING} property changes.
   *
   * @param listener
   *            The listener to remove. Must not be <code>null</code>. Has
   *            no effect if the listener is not currently registered.
   */
  virtual void RemoveServiceListener(SmartPointer<IPropertyChangeListener> listener) = 0;

  /**
   * Add a listener that can be notified when the workbench application
   * context causes the expression evaluation value to change.
   * <p>
   * <b>Note:</b> listeners should be removed when no longer necessary. If
   * not, they will be removed when the IServiceLocator used to acquire this
   * service is disposed.
   * </p>
   *
   * @param expression
   *            the core expression to evaluate.
   * @param listener
   *            the listener to be notified.
   * @param property
   *            the property contained in the notification
   * @return a token that can be used to remove this listener.
   * {@link #removeEvaluationListener(IEvaluationReference)}
   */
  virtual SmartPointer<IEvaluationReference> AddEvaluationListener(SmartPointer<Expression> expression,
      SmartPointer<IPropertyChangeListener> listener, const std::string& property) = 0;

  /**
   * Re-add a property change listener that has already been removed by
   * {@link #removeEvaluationListener(IEvaluationReference)}.
   * <p>
   * It will only accept IEvaluationReferences returned from a previous call
   * to
   * {@link #addEvaluationListener(Expression, IPropertyChangeListener, String)}
   * on this service.
   * </p>
   * <p>
   * <b>Note:</b> references should be removed when no longer necessary. If
   * not, they will be removed when the IServiceLocator used to acquire this
   * service is disposed.
   * </p>
   *
   * @param ref
   *            The listener to re-add.
   * @see #removeEvaluationListener(IEvaluationReference)
   */
  virtual void AddEvaluationReference(SmartPointer<IEvaluationReference> ref) = 0;

  /**
   * Remove the listener represented by the evaluation reference.
   *
   * @param ref
   *            the reference to be removed.
   */
  virtual void RemoveEvaluationListener(SmartPointer<IEvaluationReference> ref) = 0;

  /**
   * Get an IEvaluationContext that contains the current state of the
   * workbench application context. This context changes with the application
   * state, but becomes invalid when the global current selection changes.
   * <p>
   * Note: This context should not be modified.
   * </p>
   *
   * @return the latest context.
   * @see ISources#ACTIVE_CURRENT_SELECTION_NAME
   */
  virtual SmartPointer<const IEvaluationContext> GetCurrentState() = 0;

  /**
   * Request that this service re-evaluate all registered core expressions
   * that contain a property tester for the given property name. This will
   * fire a {@link #PROP_NOTIFYING} property change event to service
   * listeners.
   * <p>
   * Notes:
   * <ul>
   * <li>the property must be able to return the new value before this
   * re-evaluation is requested</li>
   * <li>limit calls to this method to avoid unnecessary churn</li>
   * <li>A re-evaluation that does not change the value of an expression will
   * not fire a property change event</li>
   * </ul>
   * </p>
   *
   * @param propertyName
   *            The fully qualified property name, like
   *            <code>org.eclipse.core.resources.name</code>. Must not be
   *            <code>null</code>.
   * @since 3.4
   */
  virtual void RequestEvaluation(const std::string& propertyName) = 0;
};


}


#endif /* CHERRYIEVALUATIONSERVICE_H_ */
