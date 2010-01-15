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

#include "cherryAbstractContributionFactory.h"

#include "../services/cherryIServiceLocator.h"
#include "cherryIContributionRoot.h"
#include "cherryIContributionItem.h"

#include <cherryExpression.h>

namespace cherry
{

AbstractContributionFactory::AbstractContributionFactory(
    const std::string& location, const std::string& namespaze) :
  location(location), namespaze(namespaze)
{

}

std::string AbstractContributionFactory::GetLocation() const
{
  return location;
}

std::string AbstractContributionFactory::GetNamespace() const
{
  return namespaze;
}

}
