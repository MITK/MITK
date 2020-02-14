/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYDEFAULTVARIABLE_H_
#define BERRYDEFAULTVARIABLE_H_

#include "berryIEvaluationContext.h"

namespace berry {

class DefaultVariable : public IEvaluationContext
{

private:
  Object::ConstPointer fDefaultVariable;
  IEvaluationContext* fParent;
  IEvaluationContext* fManagedPool;


public:

  /**
     * Constructs a new variable pool for a single default variable.
     *
     * @param parent the parent context for the default variable. Must not
     *  be <code>null</code>.
     * @param defaultVariable the default variable
     */
  DefaultVariable(IEvaluationContext* parent, const Object::ConstPointer& defaultVariable);

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
  Object::ConstPointer GetVariable(const QString& name) const override;

  /**
   * {@inheritDoc}
   */
  Object::ConstPointer ResolveVariable(const QString& name, const QList<Object::Pointer>& args) const override;
};

}  // namespace berry

#endif /*BERRYDEFAULTVARIABLE_H_*/
