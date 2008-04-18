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

#ifndef __CHERRY_OR_EXPRESSION_H__
#define __CHERRY_OR_EXPRESSION_H__

#include "cherryCompositeExpression.h"

namespace cherry {

class OrExpression : public CompositeExpression { 

public:
  
  EvaluationResult Evaluate(IEvaluationContext* context);
	
	bool operator==(Expression& object);
};

} // namespace cherry

#endif // __CHERRY_OR_EXPRESSION_H__
