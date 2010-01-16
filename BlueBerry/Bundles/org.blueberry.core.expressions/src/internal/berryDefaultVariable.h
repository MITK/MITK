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

#ifndef BERRYDEFAULTVARIABLE_H_
#define BERRYDEFAULTVARIABLE_H_

#include "../berryIEvaluationContext.h"

#include <vector>

namespace berry {

class DefaultVariable : public IEvaluationContext {

private:
  Object::Pointer fDefaultVariable;
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
  DefaultVariable(IEvaluationContext* parent, Object::Pointer defaultVariable);

  /**
   * {@inheritDoc}
   */
  IEvaluationContext* GetParent() const;

  /**
   * {@inheritDoc}
   */
  IEvaluationContext* GetRoot();

  /**
   * {@inheritDoc}
   */
  Object::Pointer GetDefaultVariable() const;

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
  void AddVariable(const std::string& name, Object::Pointer value);

  /**
   * {@inheritDoc}
   */
  Object::Pointer RemoveVariable(const std::string& name);

  /**
   * {@inheritDoc}
   */
  Object::Pointer GetVariable(const std::string& name) const;

  /**
   * {@inheritDoc}
   */
  Object::Pointer ResolveVariable(const std::string& name, std::vector<Object::Pointer>& args);
};

}  // namespace berry

#endif /*BERRYDEFAULTVARIABLE_H_*/
