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
  /** \brief Custom data role used to retrieve the underlying mitk::BaseProperty pointer. */
  enum
  {
    PropertyRole = Qt::UserRole + 1 ///< Returns a mitk::BaseProperty* wrapped in QVariant.
  };
}

/**
 * \brief A tree model that exposes the properties of a mitk::PropertyList.
 *
 * This model builds a two-column tree (property name and value) from a
 * mitk::PropertyList. Property names containing dots are automatically grouped
 * into a tree hierarchy. The model supports editing via QmitkPropertyItemDelegate,
 * filters properties based on the configured class name via mitk::IPropertyFilters,
 * and resolves property aliases via mitk::IPropertyAliases.
 *
 * \sa QmitkPropertyItemDelegate
 * \sa QmitkPropertyItem
 * \sa QmitkPropertiesTableEditor
 */
class MITKQTWIDGETS_EXPORT QmitkPropertyItemModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the property item model.
   * \param[in] parent Optional parent QObject.
   */
  explicit QmitkPropertyItemModel(QObject *parent = nullptr);
  ~QmitkPropertyItemModel() override;

  /**
   * \brief Returns the number of columns (always 2: name and value).
   * \param[in] parent The parent index.
   * \return 2.
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns data for the given index and role.
   * \param[in] index The model index.
   * \param[in] role  The data role (Qt::DisplayRole, Qt::EditRole, mitk::PropertyRole, etc.).
   * \return The data as QVariant.
   */
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  /**
   * \brief Returns item flags. The value column is editable for leaf items.
   * \param[in] index The model index.
   * \return The item flags.
   */
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  /**
   * \brief Returns the currently associated property list.
   * \return Pointer to the property list, or nullptr.
   */
  mitk::PropertyList *GetPropertyList() const;

  /**
   * \brief Returns header data for the given section.
   * \param[in] section     The section (0 = "Property", 1 = "Value").
   * \param[in] orientation The orientation.
   * \param[in] role        The data role.
   * \return The header data as QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * \brief Returns the model index for the given row and column.
   * \param[in] row    The row number.
   * \param[in] column The column number.
   * \param[in] parent The parent index.
   * \return The model index.
   */
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the parent index for the given child index.
   * \param[in] child The child index.
   * \return The parent index.
   */
  QModelIndex parent(const QModelIndex &child) const override;

  /**
   * \brief Returns the number of rows (child properties) under the given parent.
   * \param[in] parent The parent index.
   * \return The row count.
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Sets the property value at the given index.
   * \param[in] index The model index (must be in the value column).
   * \param[in] value The new value.
   * \param[in] role  The data role (must be Qt::EditRole).
   * \return True if the value was set successfully.
   */
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  /**
   * \brief Sets the property list and optional class name for filtering.
   * \param[in] propertyList The property list to display.
   * \param[in] className    Optional class name used for property filtering and alias resolution.
   */
  void SetPropertyList(mitk::PropertyList *propertyList, const QString &className = "");

  /**
   * \brief Forces a complete rebuild of the model from the current property list.
   */
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
