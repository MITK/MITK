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

#include "berryOrExpression.h"

namespace berry {

EvaluationResult OrExpression::Evaluate(IEvaluationContext::Pointer context)
{
  return this->EvaluateOr(context);
}

bool OrExpression::operator==(Expression& object)
{
  try {
    OrExpression& that = dynamic_cast<OrExpression&>(object);
    return this->fExpressions == that.fExpressions;
  }
  catch (std::bad_cast)
  {
    return false;
  }
} 

}
