/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertyItemModel_h
#define QmitkPropertyItemModel_h

#include <MitkQtWidgetsExports.h>
#include <QAbstractItemModel>
#include <mitkPropertyList.h>
#include <mitkWeakPointer.h>
#include <mitkCoreServices.h>

class QmitkPropertyItem;

namespace berry
{
  struct IBerryPreferences;
}

namespace mitk
{
  enum
  {
    PropertyRole = Qt::UserRole + 1
  };
}

class MITKQTWIDGETS_EXPORT QmitkPropertyItemModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit QmitkPropertyItemModel(QObject *parent = nullptr);
  ~QmitkPropertyItemModel() override;

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  mitk::PropertyList *GetPropertyList() const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  void SetPropertyList(mitk::PropertyList *propertyList, const QString &className = "");
  void Update();

private:
  void CreateRootItem();
  QModelIndex FindProperty(const mitk::BaseProperty *property);
  void OnPropertyListModified();
  void OnPropertyListDeleted();
  void OnPropertyModified(const itk::Object *property, const itk::EventObject &event);
  void SetNewPropertyList(mitk::PropertyList *newPropertyList);

  mitk::CoreServicePointer<mitk::IPropertyAliases> m_PropertyAliases;
  mitk::CoreServicePointer<mitk::IPropertyFilters> m_PropertyFilters;
  mitk::WeakPointer<mitk::PropertyList> m_PropertyList;
  QString m_ClassName;
  std::unique_ptr<QmitkPropertyItem> m_RootItem;
  std::map<std::string, unsigned long> m_PropertyDeletedTags;
  std::map<std::string, unsigned long> m_PropertyModifiedTags;
  unsigned long m_PropertyListDeletedTag;
  unsigned long m_PropertyListModifiedTag;
};

#endif
