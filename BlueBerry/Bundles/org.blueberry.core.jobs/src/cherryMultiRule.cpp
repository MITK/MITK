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

#include "cherryMultiRule.h"

namespace cherry
{

// this is just a dummy implementation of a MultiRule. 
bool MultiRule::IsConflicting(ISchedulingRule::Pointer  /*sptr_myRule*/) const
{
  return false;
}

bool MultiRule::Contains(ISchedulingRule::Pointer  /*sptr_myrule*/) const
{
  return false;
}

}
