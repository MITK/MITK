/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 15350 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#ifndef _BERRY_MULTIRULE_H
#define _BERRY_MULTIRULE_H

#include "berryISchedulingRule.h"

namespace berry
{

/**
 * A MultiRule is a compound scheduling rule that represents a fixed group of child 
 * scheduling rules.  A MultiRule conflicts with another rule if any of its children conflict 
 * with that rule.  More formally, a compound rule represents a logical intersection 
 * of its child rules with respect to the <code>isConflicting</code> equivalence 
 * relation.
 * <p>
 * A MultiRule will never contain other MultiRules as children.  If a MultiRule is provided
 * as a child, its children will be added instead.
 * </p>
 * 
 */
class MultiRule: public ISchedulingRule
{

  bool IsConflicting(ISchedulingRule::Pointer myRule) const;

  bool Contains(ISchedulingRule::Pointer rule) const;

};

}

#endif /* _BERRY_MULTIRULE_H */ 
