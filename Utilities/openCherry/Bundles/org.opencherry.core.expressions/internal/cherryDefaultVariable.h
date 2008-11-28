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

#ifndef CHERRYDEFAULTVARIABLE_H_
#define CHERRYDEFAULTVARIABLE_H_

#include "../cherryIEvaluationContext.h"

#include <vector>

namespace cherry {

class DefaultVariable : public IEvaluationContext {

private:
  ExpressionVariable::Pointer fDefaultVariable;
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
  DefaultVariable(IEvaluationContext* parent, ExpressionVariable::Pointer defaultVariable);

  /**
   * {@inheritDoc}
   */
  IEvaluationContext* GetParent();

  /**
   * {@inheritDoc}
   */
  IEvaluationContext* GetRoot();

  /**
   * {@inheritDoc}
   */
  ExpressionVariable::Pointer GetDefaultVariable();
  
  /**
   * {@inheritDoc}
   */
  void SetAllowPluginActivation(bool value);
  
  /**
   * {@inheritDoc}
   */
  bool GetAllowPluginActivation();

  /**
   * {@inheritDoc}
   */
  void AddVariable(const std::string& name, ExpressionVariable::Pointer value);

  /**
   * {@inheritDoc}
   */
  ExpressionVariable::Pointer RemoveVariable(const std::string& name);

  /**
   * {@inheritDoc}
   */
  ExpressionVariable::Pointer GetVariable(const std::string& name);

  /**
   * {@inheritDoc}
   */
  ExpressionVariable::Pointer ResolveVariable(const std::string& name, std::vector<ExpressionVariable::Pointer>& args);
};

}  // namespace cherry

#endif /*CHERRYDEFAULTVARIABLE_H_*/
