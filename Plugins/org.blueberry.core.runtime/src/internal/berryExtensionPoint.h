/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXTENSIONPOINT_H_
#define BERRYEXTENSIONPOINT_H_

#include "berryRegistryObject.h"

namespace berry {

struct IContributor;

/**
 * An object which represents the user-defined extension point in a
 * plug-in manifest.
 */
class ExtensionPoint : public RegistryObject
{

private:

  friend class ExtensionPointHandle;
  friend class ExtensionRegistry;
  friend class ExtensionsParser;

  //Place holder for the label and the schema. It contains either a String[] or a SoftReference to a String[].
  //The array layout is [label, schemaReference, fullyQualifiedName, namespace, contributorId]
  QList<QString> extraInformation;

  //Indexes of the various fields
  static const int LABEL; // = 0; //The human readable name for the extension point
  static const int SCHEMA; // = 1; //The schema of the extension point
  static const int QUALIFIED_NAME; // = 2; //The fully qualified name of the extension point
  static const int NAMESPACE; // = 3; //The name of the namespace of the extension point
  static const int CONTRIBUTOR_ID; // = 4; //The ID of the actual contributor of the extension point
  static const int EXTRA_SIZE; // = 5;

  QList<QString> GetExtraData() const;

protected:

  QString GetSimpleIdentifier() const;

  QString GetSchemaReference() const;

  QString GetLabel() const;

  QString GetContributorId() const;

  void SetSchema(const QString& value);

  void SetLabel(const QString& value);

  void SetUniqueIdentifier(const QString& value);

  void SetNamespace(const QString& value);

  void SetContributorId(const QString id);

  QString GetLabelAsIs() const;

  QString GetLabel(const QLocale& locale);

public:

  berryObjectMacro(berry::ExtensionPoint);

  ExtensionPoint(ExtensionRegistry* registry, bool persist);

  ExtensionPoint(int self, const QList<int>& children, int dataOffset,
                 ExtensionRegistry* registry, bool persist);

  QString GetUniqueIdentifier() const;

  SmartPointer<IContributor> GetContributor() const;

  QString GetNamespace() const;

  QString ToString() const override;

};

}

#endif /*BERRYEXTENSIONPOINT_H_*/
