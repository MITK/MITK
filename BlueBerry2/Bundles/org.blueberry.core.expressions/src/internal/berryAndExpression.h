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

#ifndef __BERRY_AND_EXPRESSION_H__
#define __BERRY_AND_EXPRESSION_H__

#include "berryCompositeExpression.h"

namespace berry {

class AndExpression : public CompositeExpression {

public:
  
  bool operator==(Expression& object) ;

  EvaluationResult Evaluate(IEvaluationContext::Pointer context);
};

}  // namespace berry

#endif // __BERRY_AND_EXPRESSION_H__
