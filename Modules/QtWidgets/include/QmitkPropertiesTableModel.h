/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertiesTableModel_h
#define QmitkPropertiesTableModel_h

#include <MitkQtWidgetsExports.h>

#include "mitkDataNode.h"
#include "mitkWeakPointer.h"

#include <QAbstractTableModel>
#include <string>
#include <vector>

/**
 * \ingroup QmitkModule
 * \brief A table model for showing and editing mitk::Properties.
 *
 * \see QmitkPropertyDelegate
 */
class MITKQTWIDGETS_EXPORT QmitkPropertiesTableModel : public QAbstractTableModel
{
public:
  static const int PROPERTY_NAME_COLUMN = 0;
  static const int PROPERTY_VALUE_COLUMN = 1;
  ///
  /// Typedef for the complete Property Datastructure, which may be written as follows:
  /// Name->(mitk::BaseProperty::Pointer)
  ///
  typedef std::pair<std::string, mitk::BaseProperty::Pointer> PropertyDataSet;

  ///
  /// Constructs a new QmitkDataStorageTableModel
  /// and sets the DataNode for this TableModel.
  QmitkPropertiesTableModel(QObject *parent = nullptr, mitk::PropertyList::Pointer _PropertyList = nullptr);

  ///
  /// Standard dtor. Nothing to do here.
  ~QmitkPropertiesTableModel() override;

public:
  ///
  /// Returns the property list of this table model.
  ///
  mitk::PropertyList::Pointer GetPropertyList() const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  ///
  /// Overwritten from QAbstractTableModel. Returns the number of columns. That is usually two in this model:
  /// the properties name and its value.
  int columnCount(const QModelIndex &parent) const override;

  ///
  /// Sets the Property List to show. Resets the whole model. If _PropertyList is nullptr the model is empty.
  ///
  void SetPropertyList(mitk::PropertyList *_PropertyList);

  ///
  /// \brief Gets called when the list is about to be deleted.
  ///
  virtual void PropertyListDelete();

  ///
  /// \brief Called when a single property was changed. Send a model changed event to the Qt-outer world.
  ///
  virtual void PropertyModified(const itk::Object *caller, const itk::EventObject &event);

  ///
  /// \brief Called when a single property was changed. Send a model changed event to the Qt-outer world.
  ///
  virtual void PropertyDelete(const itk::Object *caller, const itk::EventObject &event);

  ///
  /// \brief Set a keyword for filtering of properties. Only properties beginning with this string will be shown
  ///
  virtual void SetFilterPropertiesKeyWord(std::string _FilterKeyWord);

  ///
  /// Overridden from QAbstractTableModel. Sets data at index for given role.
  ///
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

  ///
  /// \brief Reimplemented sort function from QAbstractTableModel to enable sorting on the table.
  ///
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

protected:
  ///
  /// \struct PropertyDataSetCompareFunction
  /// \brief A struct that you can use in std::sort algorithm for sorting the
  /// property list elements.
  ///
  struct PropertyDataSetCompareFunction
  {
    ///
    /// \brief Specifies field of the property with which it will be sorted.
    ///
    enum CompareCriteria
    {
      CompareByName = 0,
      CompareByValue
    };

    ///
    /// \brief Specifies Ascending/descending ordering.
    ///
    enum CompareOperator
    {
      Less = 0,
      Greater
    };

    ///
    /// \brief Creates a PropertyDataSetCompareFunction. A CompareCriteria and a CompareOperator must be given.
    ///
    PropertyDataSetCompareFunction(CompareCriteria _CompareCriteria = CompareByName,
                                   CompareOperator _CompareOperator = Less);
    ///
    /// \brief The reimplemented compare function.
    ///
    bool operator()(const PropertyDataSet &_Left, const PropertyDataSet &_Right) const;

  protected:
    CompareCriteria m_CompareCriteria;
    CompareOperator m_CompareOperator;
  };

  ///
  /// An unary function for selecting Properties in a vector by a key word.
  ///
  struct PropertyListElementFilterFunction
  {
    PropertyListElementFilterFunction(const std::string &m_FilterKeyWord);
    ///
    /// \brief The reimplemented compare function.
    ///
    bool operator()(const PropertyDataSet &_Elem) const;

  protected:
    std::string m_FilterKeyWord;
  };

  ///
  /// \brief Searches for the specified property and returns the row of the element in this QTableModel.
  /// If any errors occur, the function returns -1.
  ///
  int FindProperty(const mitk::BaseProperty *_Property) const;

  ///
  /// Adds a property dataset to the current selection.
  /// When a property is added a modified and delete listener
  /// is appended.
  ///
  void AddSelectedProperty(PropertyDataSet &_PropertyDataSet);
  ///
  /// Removes a property dataset from the current selection.
  /// When a property is removed the modified and delete listener
  /// are also removed.
  ///
  void RemoveSelectedProperty(unsigned int _Index);
  ///
  /// Reset is called when a new filter keyword is set or a new
  /// PropertyList is set. First of all, all priorly selected
  /// properties are removed. Then all properties to be
  /// selected (specified by the keyword) are added to the selection.
  ///
  void Reset();

  ///
  /// Holds the pointer to the properties list. Dont use smart pointers here. Instead: Listen
  /// to the delete event.
  mitk::WeakPointer<mitk::PropertyList> m_PropertyList;

  ///
  /// Store the properties in a vector so that they may be sorted
  std::vector<PropertyDataSet> m_SelectedProperties;

  ///
  /// \brief Holds all tags of Modified Event Listeners. We need it to remove them again.
  ///
  std::vector<unsigned long> m_PropertyModifiedObserverTags;

  ///
  /// \brief Holds all tags of Modified Event Listeners. We need it to remove them again.
  ///
  std::vector<unsigned long> m_PropertyDeleteObserverTags;

  unsigned long m_PropertyListDeleteObserverTag;

  ///
  /// \brief Indicates if this class should neglect all incoming events because
  /// the class itself triggered the event (e.g. when a property was edited).
  ///
  bool m_BlockEvents;

  ///
  /// \brief The property is true when the property list is sorted in descending order.
  ///
  bool m_SortDescending;

  ///
  /// \brief If set to any value, only properties containing the specified keyword in their name will be shown.
  ///
  std::string m_FilterKeyWord;
};

#endif
