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


#ifndef BERRYSOURCEPRIORITYNAMEMAPPING_H_
#define BERRYSOURCEPRIORITYNAMEMAPPING_H_

#include "../berryISources.h"

#include <berryExpression.h>

#include <map>
#include <string>

namespace berry {

/**
 * <p>
 * A static class linking the names of variables in an IEvaluationContext to the
 * priority they should be given when doing conflict resolution.
 * </p>
 * <p>
 * In the future, it will possible to define a new variable (i.e., piece of
 * application state) that you want to use inside of the
 * <code>org.blueberry.ui.contexts</code>, <code>org.blueberry.ui.handlers</code>
 * or <code>org.blueberry.ui.menus</code> extension points. As it stands right
 * now, it is not possible to run code soon enough for the
 * <code>IHandlerService</code>, <code>IMenuService</code> or
 * <code>IContextService</code> to become aware of the new variables. This
 * will likely be fixed with a new extension point.
 * </p>
 * <p>
 * TODO Move to "org.blueberry.ui" and resolve the above issue.
 * </p>
 *
 * @since 3.2
 * @see org.blueberry.ui.ISources
 * @see org.blueberry.ui.contexts.IContextService
 * @see org.blueberry.ui.handlers.IHandlerService
 * @see org.blueberry.ui.menus.IMenuService
 */

class SourcePriorityNameMapping : public ISources {

private:

  /**
  * The map of source priorities indexed by name. This value is never
  * <code>null</code>.
  */
  static std::map<std::string,int> sourcePrioritiesByName;

  /**
   * This class should not be instantiated.
   */
  SourcePriorityNameMapping();

public:

  /**
   * The variable name to use when boosting priority on an activation.
   */
  static const std::string LEGACY_LEGACY_NAME(); // = "LEGACY"; //$NON-NLS-1$

  /**
   * The value returned if there is source priority for the given name
   *
   * @see SourcePriorityNameMapping#getMapping(String)
   */
  static int NO_SOURCE_PRIORITY(); // = 0;


private:

  struct Initializer {
    Initializer();
  };

  static Initializer initializer;


public:

  /**
   * Adds a mapping between a source name and a source priority. This method
   * also cleans up any existing mappings using the same name or priority.
   * There is a one-to-one relationship between name and priority.
   *
   * @param sourceName
   *            The name of the variable as it would appear in an XML
   *            expression; must not be <code>null</code>.
   * @param sourcePriority
   *            The priority of the source with respect to other sources. A
   *            higher value means that expressions including this priority
   *            will win ties more often. It is recommended that this value is
   *            simply a single bit shifted to a particular place.
   * @see ISources
   */
  static void AddMapping(const std::string& sourceName,
      int sourcePriority);

  /**
   * Computes the source priority for the given expression. The source
   * priority is a bit mask of all of the variables references by the
   * expression. The default variable is considered to be
   * {@link ISources#ACTIVE_CURRENT_SELECTION}. The source priority is used
   * to minimize recomputations of the expression, and it can also be used for
   * conflict resolution.
   *
   * @param expression
   *            The expression for which the source priority should be
   *            computed; may be <code>null</code>.
   * @return The bit mask of all the sources required for this expression;
   *         <code>0</code> if none.
   */
  static int ComputeSourcePriority(Expression::ConstPointer expression);

  /**
   * Gets the priority for the source with the given name.
   *
   * @param sourceName
   *            The name of the variable as it would appear in an XML
   *            expression; should not be <code>null</code>.
   * @return The source priority that matches, if any;
   *         <code>NO_SOURCE_PRIORITY</code> if none is found.
   */
  static int GetMapping(const std::string& sourceName);

};

}

#endif /* BERRYSOURCEPRIORITYNAMEMAPPING_H_ */
