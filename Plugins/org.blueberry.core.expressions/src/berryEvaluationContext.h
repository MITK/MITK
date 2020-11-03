/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEVALUATIONCONTEXT_H_
#define BERRYEVALUATIONCONTEXT_H_

#include "berryIEvaluationContext.h"
#include "berryIVariableResolver.h"
#include <org_blueberry_core_expressions_Export.h>

#include "Poco/Any.h"

#include <vector>

#include <QHash>

namespace berry {

/**
 * A default implementation of an evaluation context.
 * <p>
 * Clients may instantiate this default context. The class is
 * not intended to be subclassed by clients.
 * </p>
 *
 * @since 3.0
 */
class BERRY_EXPRESSIONS EvaluationContext : public IEvaluationContext
{

private:
  IEvaluationContext* const fParent;
  Object::ConstPointer fDefaultVariable;
  QHash<QString, Object::ConstPointer> fVariables;
  std::vector<IVariableResolver*> fVariableResolvers;
  int fAllowPluginActivation;

public:

  /**
   * Create a new evaluation context with the given parent and default
   * variable.
   *
   * @param parent the parent context. Can be <code>null</code>.
   * @param defaultVariable the default variable
   */
  EvaluationContext(IEvaluationContext* parent, const Object::ConstPointer& defaultVariable);

  /**
   * Create a new evaluation context with the given parent and default
   * variable.
   *
   * @param parent the parent context. Can be <code>null</code>.
   * @param defaultVariable the default variable
   * @param resolvers an array of <code>IVariableResolvers</code> to
   *  resolve additional variables.
   *
   * @see #resolveVariable(String, Object[])
   */
  EvaluationContext(IEvaluationContext* parent, const Object::ConstPointer& defaultVariable,
                    const std::vector<IVariableResolver*>& resolvers);


  /**
   * {@inheritDoc}
   */
  IEvaluationContext* GetParent() const override;

  /**
   * {@inheritDoc}
   */
  IEvaluationContext* GetRoot() const override;

  /**
   * {@inheritDoc}
   */
  Object::ConstPointer GetDefaultVariable() const override;

  /**
   * {@inheritDoc}
   */
  void SetAllowPluginActivation(bool value) override;

  /**
   * {@inheritDoc}
   */
  bool GetAllowPluginActivation() const override;

  /**
   * {@inheritDoc}
   */
  void AddVariable(const QString& name, const Object::ConstPointer& value) override;

  /**
   * {@inheritDoc}
   */
  Object::ConstPointer RemoveVariable(const QString& name) override;

  /**
   * {@inheritDoc}
   */
  Object::ConstPointer GetVariable(const QString &name) const override;

  /**
   * {@inheritDoc}
   */
  Object::ConstPointer ResolveVariable(const QString& name, const QList<Object::Pointer>& args) const override;
};

}  // namespace berry

#endif /*BERRYEVALUATIONCONTEXT_H_*/
