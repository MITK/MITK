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

#ifndef __CHERRY_ITERATE_EXPRESSION_H__
#define __CHERRY_ITERATE_EXPRESSION_H__

#include "cherryCompositeExpression.h"

#include "service/cherryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

#include <vector>

namespace cherry {

struct IterateExpression : public CompositeExpression {
	
private:
  
  struct IteratePool : public IEvaluationContext {
		
  private:
    std::vector<ExpressionVariable::Pointer>::iterator fIterator;
		std::vector<ExpressionVariable::Pointer>::iterator fIterEnd;
		ExpressionVariable::Pointer fDefaultVariable;
		IEvaluationContext* fParent;
		
  public:
    
    IteratePool(IEvaluationContext* parent, std::vector<ExpressionVariable::Pointer>::iterator begin, std::vector<ExpressionVariable::Pointer>::iterator end)
    {
			poco_check_ptr(parent);
			
			fParent= parent;
			fIterator = begin;
			fIterEnd = end;
		}
		
    IEvaluationContext* GetParent() {
			return fParent;
		}
		
    IEvaluationContext* GetRoot() {
			return fParent->GetRoot();
		}
		
    ExpressionVariable::Pointer GetDefaultVariable() {
			return fDefaultVariable;
		}
		
    bool GetAllowPluginActivation() {
			return fParent->GetAllowPluginActivation();
		}
		
    void SetAllowPluginActivation(bool value) {
			fParent->SetAllowPluginActivation(value);
		}
		
    void AddVariable(const std::string& name, ExpressionVariable::Pointer value) {
			fParent->AddVariable(name, value);
		}
		
    ExpressionVariable::Pointer RemoveVariable(const std::string& name) {
			return fParent->RemoveVariable(name);
		}
		
    ExpressionVariable::Pointer GetVariable(const std::string& name) {
			return fParent->GetVariable(name);
		}
		
    ExpressionVariable::Pointer ResolveVariable(const std::string& name, std::vector<ExpressionVariable::Pointer>& args) {
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
	static const intptr_t HASH_INITIAL;
	
	int fOperator;
	int fEmptyResult;
	
	
public:

  IterateExpression(IConfigurationElement* configElement);

	IterateExpression(Poco::XML::Element* element);

	IterateExpression(const std::string& opValue);
	
	IterateExpression(const std::string& opValue, const std::string& ifEmpty);
	
private: void InitializeOperatorValue(const std::string& opValue);
	
private: void InitializeEmptyResultValue(const std::string& value);

	
public:
  
  /* (non-Javadoc)
    * @see Expression#evaluate(IVariablePool)
    */
  EvaluationResult Evaluate(IEvaluationContext* context);

	void CollectExpressionInfo(ExpressionInfo* info);

	bool operator==(Expression& object);

	
protected:
  
  intptr_t ComputeHashCode();
};

} // namespace cherry

#endif // __CHERRY_ITERATE_EXPRESSION_H__
