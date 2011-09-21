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

#include "berryAndExpression.h"

namespace berry
{

bool AndExpression::operator==(Expression& object)
{
  try
  {
    AndExpression& that = dynamic_cast<AndExpression&>(object);
    return this->Equals(this->fExpressions, that.fExpressions);
  }
  catch (std::bad_cast&)
  {
    return false;
  }

}

EvaluationResult
AndExpression::Evaluate(IEvaluationContext* context)
{
  return this->EvaluateAnd(context);
}

} // namespace berry
