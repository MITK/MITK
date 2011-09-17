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


#ifndef BERRYHANDLEOBJECTMANAGER_H_
#define BERRYHANDLEOBJECTMANAGER_H_

#include <Poco/HashSet.h>
#include <Poco/HashMap.h>

#include "berryHandleObject.h"

namespace berry {

/**
 * <p>
 * A manager of {@link HandleObject} instances. This has some common behaviour
 * which is shared between all such managers.
 * </p>
 * <p>
 * Clients may extend.
 * </p>
 *
 * @since 3.2
 */
class BERRY_COMMANDS HandleObjectManager {

protected:

  typedef Poco::HashSet<HandleObject::Pointer, HandleObject::Hash> HandleObjectsSet;
  typedef Poco::HashMap<std::string, HandleObject::Pointer> HandleObjectsByIdMap;

  /**
   * The set of handle objects that are defined. This value may be empty, but
   * it is never <code>null</code>.
   */
  HandleObjectsSet definedHandleObjects;

  /**
   * The map of identifiers (<code>String</code>) to handle objects (
   * <code>HandleObject</code>). This collection may be empty, but it is
   * never <code>null</code>.
   */
  HandleObjectsByIdMap handleObjectsById;

  /**
   * Verifies that the identifier is valid. Exceptions will be thrown if the
   * identifier is invalid in some way.
   *
   * @param id
   *            The identifier to validate; may be anything.
   */
  void CheckId(const std::string& id) const;

  /**
   * Returns the set of identifiers for those handle objects that are defined.
   *
   * @return The set of defined handle object identifiers; this value may be
   *         empty, but it is never <code>null</code>.
   */
  Poco::HashSet<std::string> GetDefinedHandleObjectIds() const;
};

}


#endif /* BERRYHANDLEOBJECTMANAGER_H_ */
