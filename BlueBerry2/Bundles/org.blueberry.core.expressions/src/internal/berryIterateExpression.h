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

#ifndef __BERRY_ITERATE_EXPRESSION_H__
#define __BERRY_ITERATE_EXPRESSION_H__

#include "berryCompositeExpression.h"

#include "service/berryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

#include <vector>

namespace berry {

struct IterateExpression : public CompositeExpression {

private:

  struct IteratePool : public IEvaluationContext {

    osgiObjectMacro(IteratePool)

  private:
    std::vector<Object::Pointer>::iterator fIterator;
    std::vector<Object::Pointer>::iterator fIterEnd;
    Object::Pointer fDefaultVariable;
    IEvaluationContext::Pointer fParent;

  public:

    IteratePool(IEvaluationContext::Pointer parent, std::vector<Object::Pointer>::iterator begin, std::vector<Object::Pointer>::iterator end)
    {
      poco_check_ptr(parent);

      fParent= parent;
      fIterator = begin;
      fIterEnd = end;
    }

    IEvaluationContext::Pointer GetParent() const {
      return fParent;
    }

    IEvaluationContext::Pointer GetRoot() {
      return fParent->GetRoot();
    }

    Object::Pointer GetDefaultVariable() const {
      return fDefaultVariable;
    }

    bool GetAllowPluginActivation() const {
      return fParent->GetAllowPluginActivation();
    }

    void SetAllowPluginActivation(bool value) {
      fParent->SetAllowPluginActivation(value);
    }

    void AddVariable(const std::string& name, Object::Pointer value) {
      fParent->AddVariable(name, value);
    }

    Object::Pointer RemoveVariable(const std::string& name) {
      return fParent->RemoveVariable(name);
    }

    Object::Pointer GetVariable(const std::string& name) const {
      return fParent->GetVariable(name);
    }

    Object::Pointer ResolveVariable(const std::string& name, std::vector<Object::Pointer>& args) {
      return fParent->ResolveVariable(name, args);
    }

    Poco::Any Next() {
      fDefaultVariable = *(++fIterator);
      return fDefaultVariable;
    }

    bool HasNext() {
      return (fIterator != fIterEnd);
    }
  };

  static const std::string ATT_OPERATOR;
  static const std::string ATT_IF_EMPTY;
  static const int OR;
  static const int AND;

  /**
   * The seed for the hash code for all iterate expressions.
   */
  static const std::size_t HASH_INITIAL;

  int fOperator;
  int fEmptyResult;


public:

  IterateExpression(SmartPointer<IConfigurationElement> configElement);

  IterateExpression(Poco::XML::Element* element);

  IterateExpression(const std::string& opValue);

  IterateExpression(const std::string& opValue, const std::string& ifEmpty);

private: void InitializeOperatorValue(const std::string& opValue);

private: void InitializeEmptyResultValue(const std::string& value);


public:

  /* (non-Javadoc)
    * @see Expression#evaluate(IVariablePool)
    */
  EvaluationResult Evaluate(IEvaluationContext::Pointer context);

  void CollectExpressionInfo(ExpressionInfo* info);

  bool operator==(Expression& object);


protected:

  std::size_t ComputeHashCode();
};

} // namespace berry

#endif // __BERRY_ITERATE_EXPRESSION_H__
