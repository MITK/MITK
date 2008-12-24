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

#ifndef CHERRYSTANDARDELEMENTHANDLER_H_
#define CHERRYSTANDARDELEMENTHANDLER_H_

#include "service/cherryIConfigurationElement.h"

#include "../cherryExpression.h"
#include "../cherryElementHandler.h"
#include "../cherryExpressionConverter.h"

namespace cherry
{

class StandardElementHandler : public ElementHandler 
{
  
public:
  Expression::Pointer Create(ExpressionConverter* converter, IConfigurationElement* element);

  Expression::Pointer Create(ExpressionConverter* converter, Poco::XML::Element* element);
};

}  // namespace cherry

#endif /*CHERRYSTANDARDELEMENTHANDLER_H_*/
