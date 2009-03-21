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

#include "cherryCommandUtils.h"

namespace cherry
{

int CommandUtils::Compare(const bool left, const bool right)
{
  return left == false ? (right == true ? -1 : 0) : (right == true ? 0 : 1);
}

int CommandUtils::Compare(const std::string& left, const std::string& right)
{
  return left.compare(right);
}

int CommandUtils::CompareObj(const Object::ConstPointer left,
    const Object::ConstPointer right)
{
  if (!left && !right)
  {
    return 0;
  }
  else if (!left)
  {
    return -1;
  }
  else if (!right)
  {
    return 1;
  }
  else
  {
    return left->ToString().compare(right->ToString());
  }
}

}
