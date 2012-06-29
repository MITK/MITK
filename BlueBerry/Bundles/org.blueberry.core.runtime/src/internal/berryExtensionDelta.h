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

#ifndef BERRYEXTENSIONDELTA_H
#define BERRYEXTENSIONDELTA_H

#include <QExplicitlySharedDataPointer>

namespace berry {

template<class T> class SmartPointer;

struct IExtension;
struct IExtensionPoint;

class ExtensionDeltaData;
class RegistryDelta;

class ExtensionDelta
{

public:

  enum Kind {
    /**
     * Delta kind constant indicating that an extension has been added to an
     * extension point.
     * @see ExtensionDelta#getKind()
     */
    ADDED = 1,

    /**
     * Delta kind constant indicating that an extension has been removed from an
     * extension point.
     * @see ExtensionDelta#getKind()
     */
    REMOVED = 2
  };

  ExtensionDelta();
  ExtensionDelta(const ExtensionDelta& other);

  ~ExtensionDelta();

  ExtensionDelta& operator=(const ExtensionDelta& other);

  bool operator==(const ExtensionDelta& other) const;

  uint GetHashCode() const;

  void SetContainingDelta(RegistryDelta* containingDelta);

  int GetExtensionId() const;

  int GetExtensionPointId() const;

  /**
   * Returns the affected extension point.
   *
   * @return the affected extension point
   */
  SmartPointer<IExtensionPoint> GetExtensionPoint() const;

  void SetExtensionPoint(int extensionPoint);

  /**
   * The kind of this extension delta.
   *
   * @return the kind of change this delta represents
   * @see #ADDED
   * @see #REMOVED
   */
  Kind GetKind() const;

  /**
   * Returns the affected extension.
   *
   * @return the affected extension
   */
  SmartPointer<IExtension> GetExtension() const;

  void SetExtension(int extension);

  void SetKind(Kind kind) const;

  QString ToString() const;

private:

  QExplicitlySharedDataPointer<ExtensionDeltaData> d;

};

}

uint qHash(const berry::ExtensionDelta& delta);

#endif // BERRYEXTENSIONDELTA_H
