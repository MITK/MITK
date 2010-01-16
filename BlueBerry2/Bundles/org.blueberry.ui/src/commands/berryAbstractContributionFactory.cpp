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

#include "berryAbstractContributionFactory.h"

#include "../services/berryIServiceLocator.h"
#include "berryIContributionRoot.h"
#include "berryIContributionItem.h"

#include <berryExpression.h>

namespace berry
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
