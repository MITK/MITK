/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXTENSION_H_
#define BERRYEXTENSION_H_

#include "berryRegistryObject.h"

namespace berry {

struct IContributor;

/**
 * An object which represents the user-defined extension in a plug-in manifest.
 */
class Extension : public RegistryObject
{

private:

  friend class ExtensionHandle;
  friend class ExtensionsParser;
  friend class ExtensionPointHandle;
  friend class ExtensionRegistry;
  friend class RegistryObjectManager;

  //Extension simple identifier
  QString simpleId;
  //The namespace for the extension.
  QString namespaceIdentifier;

  //  Place holder for the label and  the extension point. It contains either a String[] or a SoftReference to a String[].
  //The array layout is [label, extension point name]
  QList<QString> extraInformation;

  static const int LABEL; // = 0; //The human readable name of the extension
  static const int XPT_NAME; // = 1; // The fully qualified name of the extension point to which this extension is attached to
  static const int CONTRIBUTOR_ID; // = 2; // ID of the actual contributor of this extension
  static const int EXTRA_SIZE; // = 3;

  QList<QString> GetExtraData() const;

protected:

  QString GetExtensionPointIdentifier() const;

  QString GetSimpleIdentifier() const;

  QString GetUniqueIdentifier() const;

  void SetExtensionPointIdentifier(const QString& value);

  void SetSimpleIdentifier(const QString& value);

  QString GetLabel() const;

  void SetLabel(const QString& value);

  QString GetContributorId() const;

  void SetContributorId(const QString& value);

  void SetNamespaceIdentifier(const QString& value);

  QString GetLabelAsIs() const;

  QString GetLabel(const QLocale& locale) const;

public:

  berryObjectMacro(berry::Extension);

  Extension(ExtensionRegistry* registry, bool persist);

  Extension(int self, const QString& simpleId, const QString& namespaze,
            const QList<int>& children, int extraData,
            ExtensionRegistry* registry, bool persist);

  SmartPointer<IContributor> GetContributor() const;

  QString GetNamespaceIdentifier() const;

  QString ToString() const override;

};

}

#endif /*BERRYEXTENSION_H_*/
