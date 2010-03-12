/*=========================================================================

 Program:   MITK
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 14921 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

/// Header guard.
#ifndef QmitkPropertiesTableModel_h
#define QmitkPropertiesTableModel_h

//# Own includes
#include "mitkDataNode.h"
#include "mitkWeakPointer.h"

//# Toolkit includes
#include <QAbstractTableModel>
#include <vector>
#include <string>

//# Forward declarations

/// 
/// \class QmitkPropertiesTableModel
/// \brief A table model for showing and editing mitk::Properties.
///
/// \see QmitkPropertyDelegate
///
class QMITK_EXPORT QmitkPropertiesTableModel : public QAbstractTableModel
{

//# PUBLIC CTORS,DTOR,TYPEDEFS,CONSTANTS
public:
  static const int PROPERTY_NAME_COLUMN = 0;
  static const int PROPERTY_VALUE_COLUMN = 1;
  static const int PROPERTY_ACTIVE_COLUMN = 2;
  ///
  /// Typedef for the complete Property Datastructure, which may be written as follows:
  /// Name->(mitk::BaseProperty::Pointer->ActiveFlag)
  ///
  typedef std::pair<std::string,std::pair<mitk::BaseProperty::Pointer,bool> > PropertyDataSet;

  ///
  /// Constructs a new QmitkDataStorageTableModel 
  /// and sets the DataNode for this TableModel.
  QmitkPropertiesTableModel(QObject* parent = 0, mitk::PropertyList::Pointer _PropertyList=0);

  ///
  /// Standard dtor. Nothing to do here.
  virtual ~QmitkPropertiesTableModel();

//# PUBLIC GETTER
public:
  ///
  /// Returns the property list of this table model.
  ///
  mitk::PropertyList::Pointer GetPropertyList() const; 

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  Qt::ItemFlags flags(const QModelIndex& index) const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the number of columns. That is usually two in this model:
  /// the properties name and its value.
  int columnCount(const QModelIndex &parent) const;

//# PUBLIC SETTER
public:
  ///
  /// Sets the Property List to show. Resets the whole model. If _PropertyList is NULL the model is empty.
  ///
  void SetPropertyList(mitk::PropertyList* _PropertyList);

  ///
  /// \brief Gets called when the list is about to be deleted.
  ///
  virtual void PropertyListDelete(const itk::Object *_PropertyList);

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
  bool setData(const QModelIndex &index, const QVariant &value,
    int role);

  ///
  /// \brief Reimplemented sort function from QAbstractTableModel to enable sorting on the table.
  ///
  void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );

//#PROTECTED INNER CLASSES
protected:
  ///
  /// \struct PropertyDataSetCompareFunction
  /// \brief A struct that inherits from std::binary_function. You can use it in std::sort algorithm for sorting the property list elements.
  ///
  struct PropertyDataSetCompareFunction 
    : public std::binary_function<PropertyDataSet, PropertyDataSet, bool>
  {
    ///
    /// \brief Specifies field of the property with which it will be sorted.
    ///
    enum CompareCriteria {
      CompareByName = 0,
      CompareByValue,
      CompareByActivity
    };

    ///
    /// \brief Specifies Ascending/descending ordering.
    ///
    enum CompareOperator {
      Less = 0,
      Greater
    };

    ///
    /// \brief Creates a PropertyDataSetCompareFunction. A CompareCriteria and a CompareOperator must be given.
    ///
    PropertyDataSetCompareFunction(CompareCriteria _CompareCriteria = CompareByName, CompareOperator _CompareOperator = Less);
    ///
    /// \brief The reimplemented compare function.
    ///
    bool operator()(const PropertyDataSet& _Left
      , const PropertyDataSet& _Right) const;

  protected:
    CompareCriteria m_CompareCriteria;
    CompareOperator m_CompareOperator;
  };

  ///
  /// An unary function for selecting Properties in a vector by a key word.
  ///
  struct PropertyListElementFilterFunction
    : public std::unary_function<PropertyDataSet, bool>
  {
    PropertyListElementFilterFunction(const std::string& m_FilterKeyWord);
    ///
    /// \brief The reimplemented compare function.
    ///
    bool operator()(const PropertyDataSet& _Elem) const;

  protected:
    std::string m_FilterKeyWord;
  };

//# PROTECTED GETTER
protected:
  ///
  /// \brief Searches for the specified property and returns the row of the element in this QTableModel. 
  /// If any errors occur, the function returns -1.
  ///
  int FindProperty(const mitk::BaseProperty* _Property) const;
  
//# PROTECTED SETTER
protected:
  ///
  /// Adds a property dataset to the current selection.
  /// When a property is added a modified and delete listener
  /// is appended.
  ///
  void AddSelectedProperty(PropertyDataSet& _PropertyDataSet);
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

//# PROTECTED MEMBERS
protected:
  /// 
  /// Holds the pointer to the properties list. Dont use smart pointers here. Instead: Listen
  /// to the delete event.
  mitk::WeakPointer<mitk::PropertyList> m_PropertyList;

  /// 
  /// Store the properties in a vector so that they may be sorted
  std::vector<PropertyDataSet > m_SelectedProperties;

  ///
  /// \brief Holds all tags of Modified Event Listeners. We need it to remove them again.
  ///
  std::vector<unsigned long> m_PropertyModifiedObserverTags;

  ///
  /// \brief Holds all tags of Modified Event Listeners. We need it to remove them again.
  ///
  std::vector<unsigned long> m_PropertyDeleteObserverTags;

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

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
