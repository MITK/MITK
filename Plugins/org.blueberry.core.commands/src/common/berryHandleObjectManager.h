/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYHANDLEOBJECTMANAGER_H_
#define BERRYHANDLEOBJECTMANAGER_H_

#include <org_blueberry_core_commands_Export.h>

#include <berrySmartPointer.h>

#include <QHash>
#include <QSet>

namespace berry {

class HandleObject;

/**
 * <p>
 * A manager of {@link HandleObject} instances. This has some common behaviour
 * which is shared between all such managers.
 * </p>
 * <p>
 * Clients may extend.
 * </p>
*/
class BERRY_COMMANDS HandleObjectManager
{

protected:

  typedef QSet<SmartPointer<HandleObject> > HandleObjectsSet;
  typedef QHash<QString, SmartPointer<HandleObject> > HandleObjectsByIdMap;

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
  void CheckId(const QString& id) const;

  /**
   * Returns the set of identifiers for those handle objects that are defined.
   *
   * @return The set of defined handle object identifiers; this value may be
   *         empty, but it is never <code>null</code>.
   */
  QSet<QString> GetDefinedHandleObjectIds() const;
};

}


#endif /* BERRYHANDLEOBJECTMANAGER_H_ */
