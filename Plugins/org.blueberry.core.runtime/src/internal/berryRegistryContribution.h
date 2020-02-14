/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYCONTRIBUTION_H
#define BERRYREGISTRYCONTRIBUTION_H

#include "berryKeyedElement.h"

namespace berry {

class ExtensionRegistry;

// This object is used to keep track on a contributor basis of the extension and extension points being contributed.
// It is mainly used on removal so we can quickly  find objects to remove.
// Each contribution is made in the context of a namespace.
class RegistryContribution : public KeyedElement
{

public:

  static const int EXTENSION_POINT; // = 0;
  static const int EXTENSION; // = 1;

  berryObjectMacro(berry::RegistryContribution);

  RegistryContribution(const QString& contributorId, ExtensionRegistry* registry, bool persist);

  QString GetDefaultNamespace() const;

  QString ToString() const override;

  //Implements the KeyedElement interface
  QString GetKey() const override;

  bool ShouldPersist() const;

  void UnlinkChild(int id);

  /**
   * Contribution is empty if it has no children.
   */
  bool IsEmpty() const;

  /**
   * Find if this contribution has a children with ID = id.
   * @param id possible ID of the child
   * @return true: contribution has this child
   */
  bool HasChild(int id) const;

private:

  friend class RegistryObjectManager;
  friend class ExtensionRegistry;
  friend class ExtensionsParser;

  //The registry that owns this object
  ExtensionRegistry* registry;

  // The actual contributor of the contribution
  QString contributorId;

  // Value is derived from the contributorId and cached for performance
  mutable QString defaultNamespace;

  // indicates if this contribution needs to be saved in the registry cache
  bool persist;

  // This array stores the identifiers of both the extension points and the extensions.
  // The array has always a minimum size of 2.
  // The first element of the array is the number of extension points and the second the number of extensions.
  // [numberOfExtensionPoints, numberOfExtensions, extensionPoint#1, extensionPoint#2, extensionPoint..., ext#1, ext#2, ext#3, ... ].
  // The size of the array is 2 + (numberOfExtensionPoints +  numberOfExtensions).
  QList<int> children;


  void MergeContribution(const RegistryContribution::Pointer& addContribution);

  void SetRawChildren(const QList<int>& children);

  QString GetContributorId() const;

  QList<int> GetRawChildren() const;

  QList<int> GetExtensions() const;

  QList<int> GetExtensionPoints() const;

  bool IsEqual(const KeyedElement& other) const override;
};

}

#endif // BERRYREGISTRYCONTRIBUTION_H
