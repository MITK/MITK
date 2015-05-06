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
  IEvaluationContext* GetParent() const;

  /**
   * {@inheritDoc}
   */
  IEvaluationContext* GetRoot() const;

  /**
   * {@inheritDoc}
   */
  Object::ConstPointer GetDefaultVariable() const;

  /**
   * {@inheritDoc}
   */
  void SetAllowPluginActivation(bool value);

  /**
   * {@inheritDoc}
   */
  bool GetAllowPluginActivation() const;

  /**
   * {@inheritDoc}
   */
  void AddVariable(const QString& name, const Object::ConstPointer& value);

  /**
   * {@inheritDoc}
   */
  Object::ConstPointer RemoveVariable(const QString& name);

  /**
   * {@inheritDoc}
   */
  Object::ConstPointer GetVariable(const QString& name) const;

  /**
   * {@inheritDoc}
   */
  Object::ConstPointer ResolveVariable(const QString& name, const QList<Object::Pointer>& args) const;
};

}  // namespace berry

#endif /*BERRYDEFAULTVARIABLE_H_*/
