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

#ifndef __CHERRY_ENABLEMENT_EXPRESSION_H__
#define __CHERRY_ENABLEMENT_EXPRESSION_H__

#include "cherryCompositeExpression.h"

#include "org.opencherry.osgi/service/cherryIConfigurationElement.h"

#include "Poco/DOM/Node.h"

namespace cherry {

class EnablementExpression : public CompositeExpression
{

public:

  /**
   * Creates a {@link EnablementExpression}.
   * 
   * @param configElement the configuration element
   */
  EnablementExpression(IConfigurationElement* /*configElement*/)
  {
    // config element not used yet.
  }

  /**
   * Creates a {@link EnablementExpression}.
   * 
   * @param element the XML element
   */
  EnablementExpression(Poco::XML::Node* /*element*/)
  {
    // element not used yet.
  }

  bool operator==(Expression& object);

  EvaluationResult Evaluate(IEvaluationContext* context);
};

} // namespace cherry

#endif // __CHERRY_ENABLEMENT_EXPRESSION_H__

