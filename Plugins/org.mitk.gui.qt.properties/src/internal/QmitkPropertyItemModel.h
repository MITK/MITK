/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkPropertyItemModel_h
#define QmitkPropertyItemModel_h

#include <QAbstractItemModel>
#include <mitkPropertyList.h>
#include <mitkWeakPointer.h>

class QmitkPropertyItem;

namespace berry
{
  struct IBerryPreferences;
}

namespace mitk
{
  class IPropertyAliases;
  class IPropertyFilters;

  enum
  {
    PropertyRole = Qt::UserRole + 1
  };
}

class QmitkPropertyItemModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit QmitkPropertyItemModel(QObject* parent = NULL);
  ~QmitkPropertyItemModel();

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  mitk::PropertyList* GetPropertyList() const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  void OnPreferencesChanged(const berry::IBerryPreferences* preferences);
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  void SetPropertyList(mitk::PropertyList* propertyList, const QString& className = "");
  void Update();

private:
  void CreateRootItem();
  QModelIndex FindProperty(const mitk::BaseProperty* property);
  void OnPropertyDeleted(const itk::Object* property, const itk::EventObject& event);
  void OnPropertyListDeleted(const itk::Object* propertyList);
  void OnPropertyModified(const itk::Object* property, const itk::EventObject& event);
  void SetNewPropertyList(mitk::PropertyList* propertyList);

  mitk::IPropertyAliases* m_PropertyAliases;
  mitk::IPropertyFilters* m_PropertyFilters;
  mitk::WeakPointer<mitk::PropertyList> m_PropertyList;
  QString m_ClassName;
  std::auto_ptr<QmitkPropertyItem> m_RootItem;
  std::map<std::string, unsigned long> m_PropertyDeletedTags;
  std::map<std::string, unsigned long> m_PropertyModifiedTags;
};

#endif
