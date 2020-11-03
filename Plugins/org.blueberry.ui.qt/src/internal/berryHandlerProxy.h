/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHANDLERPROXY_H
#define BERRYHANDLERPROXY_H

#include "berryAbstractHandlerWithState.h"
#include "berryIElementUpdater.h"
#include "berryIHandlerListener.h"
#include "berryIPropertyChangeListener.h"

namespace berry {

struct IConfigurationElement;
struct IEvaluationReference;
struct IEvaluationService;

class Expression;

/**
 * <p>
 * A proxy for a handler that has been defined in XML. This delays the class
 * loading until the handler is really asked for information (besides the
 * priority or the command identifier). Asking a proxy for anything but the
 * attributes defined publicly in this class will cause the proxy to instantiate
 * the proxied handler.
 * </p>
 */
class HandlerProxy : public AbstractHandlerWithState, public IElementUpdater,
    private IHandlerListener, private IPropertyChangeListener
{

public:

  berryObjectMacro(berry::HandlerProxy);

private:

  static QHash<SmartPointer<IConfigurationElement>, HandlerProxy*> CEToProxyMap;

  /**
   *
   */
  static const QString PROP_ENABLED; // = "enabled";

  /**
   * The configuration element from which the handler can be created. This
   * value will exist until the element is converted into a real class -- at
   * which point this value will be set to <code>null</code>.
   */
  mutable SmartPointer<IConfigurationElement> configurationElement;

  /**
   * The <code>enabledWhen</code> expression for the handler. Only if this
   * expression evaluates to <code>true</code> (or the value is
   * <code>null</code>) should we consult the handler.
   */
  const SmartPointer<Expression> enabledWhenExpression;

  /**
   * The real handler. This value is <code>null</code> until the proxy is
   * forced to load the real handler. At this point, the configuration element
   * is converted, nulled out, and this handler gains a reference.
   */
  mutable SmartPointer<IHandler> handler;

  /**
   * The name of the configuration element attribute which contains the
   * information necessary to instantiate the real handler.
   */
  const QString handlerAttributeName;

  /**
   * The evaluation service to use when evaluating
   * <code>enabledWhenExpression</code>. This value may be
   * <code>null</code> only if the <code>enabledWhenExpression</code> is
   * <code>null</code>.
   */
  IEvaluationService* evaluationService;

  SmartPointer<IEvaluationReference> enablementRef;

  bool proxyEnabled;

  QString commandId;

  //
  // state to support checked or radio commands.
  SmartPointer<State> checkedState;

  SmartPointer<State> radioState;

  // Exception that occurs while loading the proxied handler class
  mutable ctkException* loadException;

public:

  /**
   * Constructs a new instance of <code>HandlerProxy</code> with all the
   * information it needs to try to avoid loading until it is needed.
   *
   * @param commandId the id for this handler
   * @param configurationElement
   *            The configuration element from which the real class can be
   *            loaded at run-time; must not be <code>null</code>.
   * @param handlerAttributeName
   *            The name of the attribute or element containing the handler
   *            executable extension; must not be <code>null</code>.
   * @param enabledWhenExpression
   *            The name of the element containing the enabledWhen expression.
   *            This should be a child of the
   *            <code>configurationElement</code>. If this value is
   *            <code>null</code>, then there is no enablement expression
   *            (i.e., enablement will be delegated to the handler when
   *            possible).
   * @param evaluationService
   *            The evaluation service to manage enabledWhen expressions
   *            trying to evaluate the <code>enabledWhenExpression</code>.
   *            This value may be <code>null</code> only if the
   *            <code>enabledWhenExpression</code> is <code>null</code>.
   */
  HandlerProxy(const QString commandId,
               const SmartPointer<IConfigurationElement>& configurationElement,
               const QString handlerAttributeName,
               const SmartPointer<Expression> enabledWhenExpression = SmartPointer<Expression>(),
               IEvaluationService* evaluationService = nullptr);

  static void UpdateStaleCEs(const QList<SmartPointer<IConfigurationElement> >& replacements);

  void SetEnabled(const Object::Pointer& evaluationContext) override;

  /**
   * Passes the dipose on to the proxied handler, if it has been loaded.
   */
  void Dispose() override;

  Object::Pointer Execute(const SmartPointer<const ExecutionEvent>& event) override;

  bool IsEnabled() const override;

  bool IsHandled() const override;

  QString ToString() const override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.commands.IElementUpdater#updateElement(org.eclipse.ui.menus.UIElement,
   *      java.util.Map)
   */
  void UpdateElement(UIElement* element, const QHash<QString, Object::Pointer>& parameters) override;

  /* (non-Javadoc)
   * @see org.eclipse.core.commands.IStateListener#handleStateChange(org.eclipse.core.commands.State, java.lang.Object)
   */
  void HandleStateChange(const SmartPointer<State>& state, const Object::Pointer& oldValue) override;

  /**
   * @return the config element for use with the PDE framework.
   */
  SmartPointer<IConfigurationElement> GetConfigurationElement() const;

  QString GetAttributeName() const;

  /**
   * @return Returns the handler.
   */
  SmartPointer<IHandler> GetHandler() const;

private:

  /**
   *
   */
  void RegisterEnablement();

  void SetProxyEnabled(bool enabled);

  bool GetProxyEnabled() const;

  /**
   * @return
   */
  IPropertyChangeListener* GetEnablementListener() const;

  using IPropertyChangeListener::PropertyChange;
  void PropertyChange(const SmartPointer<PropertyChangeEvent>& event) override;

  /**
   * Loads the handler, if possible. If the handler is loaded, then the member
   * variables are updated accordingly.
   *
   * @return <code>true</code> if the handler is now non-null;
   *         <code>false</code> otherwise.
   */
  bool LoadHandler() const;

  IHandlerListener* GetHandlerListener() const;

  void HandlerChanged(const SmartPointer<HandlerEvent>& handlerEvent) override;

  /**
   * Retrives the ConfigurationElement attribute according to the
   * <code>handlerAttributeName</code>.
   *
   * @return the handlerAttributeName value, may be <code>null</code>.
   */
  QString GetConfigurationElementAttribute() const;

  bool IsOkToLoad() const;

  void RefreshElements();

};

}

#endif // BERRYHANDLERPROXY_H
