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

#ifndef __BERRY_OR_EXPRESSION_H__
#define __BERRY_OR_EXPRESSION_H__

#include "berryCompositeExpression.h"

namespace berry {

class OrExpression : public CompositeExpression { 

public:
  
  EvaluationResult Evaluate(IEvaluationContext::Pointer context);
  
  bool operator==(Expression& object);
};

} // namespace berry

#endif // __BERRY_OR_EXPRESSION_H__
