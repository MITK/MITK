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

#ifndef _CHERRY_ISCHEDULING_RULE_H_
#define _CHERRY_ISCHEDULING_RULE_H_

#include "cherryJobsDll.h"
#include <cherryObject.h>

namespace cherry
{

/**
 * Scheduling rules are used by jobs to indicate when they need exclusive access
 * to a resource.  
 * @todo 
 * Scheduling rules can also be applied synchronously to a thread
 * using <tt>IJobManager.beginRule(ISchedulingRule)</tt> and 
 * <tt>IJobManager.endRule(ISchedulingRule)</tt>. 
 * 
 * The job manager guarantees that 
 * no two jobs with conflicting scheduling rules will run concurrently. 
 * @todo 
 * Multiple rules can be applied to a given thread only if the outer rule explicitly 
 * allows the nesting as specified by the <code>contains</code> method.  
 * 
 * <p>
 * Clients may implement this interface.
 * 
 * @see Job#GetRule()
 * @see Job#SetRule(ISchedulingRule)
 * @see Job#Schedule(long)
 * @see IJobManager#BeginRule(ISchedulingRule, org.eclipse.core.runtime.IProgressMonitor)
 * @see IJobManager#EndRule(ISchedulingRule)
 */
struct CHERRY_JOBS ISchedulingRule: public Object
{

  cherryInterfaceMacro(ISchedulingRule, cherry)

  /**
   * Returns whether this scheduling rule completely contains another scheduling
   * rule.  Rules can only be nested within a thread if the inner rule is completely
   * contained within the outer rule.
   * <p>
   * Implementations of this method must obey the rules of a partial order relation
   * on the set of all scheduling rules.  In particular, implementations must be reflexive
   * (a.contains(a) is always true), antisymmetric (a.contains(b) and b.contains(a) iff
   * equals(b), 
   * and transitive (if a.contains(b) and b.contains(c), then a.contains(c)).  Implementations
   * of this method must return <code>false</code> when compared to a rule they
   * know nothing about.
   *
   * @param rule the rule to check for containment
   * @return <code>true</code> if this rule contains the given rule, and
   * <code>false</code> otherwise.
   */
  virtual bool Contains(ISchedulingRule::Pointer rule) const = 0;

  /**
   * Returns whether this scheduling rule is compatible with another scheduling rule.
   * If <code>true</code> is returned, then no job with this rule will be run at the
   * same time as a job with the conflicting rule.  If <code>false</code> is returned,
   * then the job manager is free to run jobs with these rules at the same time.
   * <p>
   * Implementations of this method must be reflexive, symmetric, and consistent,
   * and must return <code>false</code> when compared  to a rule they know
   * nothing about.
   *
   * @param rule the rule to check for conflicts
   * @return <code>true</code> if the rule is conflicting, and <code>false</code>
   * 	otherwise.
   */
  virtual bool IsConflicting(ISchedulingRule::Pointer myRule) const = 0;

};

}

#endif // _CHERRY_ISCHEDULING_RULE_H_
