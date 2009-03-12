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


#ifndef QmitkPropertiesTableModel_h
#define QmitkPropertiesTableModel_h

/// Own includes.
#include "mitkDataTreeNode.h"

/// Toolkit includes.
#include <vector>
#include <string>
#include <QAbstractTableModel>

/// Forward declarations.

/// 
/// \class QmitkPropertiesTableModel
/// \brief A table model for the properties of a node.
///
/// \see QmitkPropertyDelegate
class QMITK_EXPORT QmitkPropertiesTableModel : public QAbstractTableModel
{
public:

  struct PropertyListElementCompareFunction 
    : public std::binary_function<std::pair<std::string,std::pair<mitk::BaseProperty::Pointer,bool> >
      , std::pair<std::string,std::pair<mitk::BaseProperty::Pointer,bool> >, bool>
  {
    enum CompareCriteria {
      CompareByName = 0,
      CompareByValue,
      CompareByActivity
    };

    enum CompareOperator {
      Less = 0,
      Greater
    };
    
    PropertyListElementCompareFunction(CompareCriteria _CompareCriteria = CompareByName, CompareOperator _CompareOperator = Less);

    bool operator()(const std::pair<std::string,std::pair<mitk::BaseProperty::Pointer,bool> >& _Left
      , const std::pair<std::string,std::pair<mitk::BaseProperty::Pointer,bool> >& _Right) const;

    protected:
      CompareCriteria m_CompareCriteria;
      CompareOperator m_CompareOperator;
  };

  ///
  /// Constructs a new QmitkDataStorageTableModel 
  /// and sets the DataTreeNode for this TableModel.
  QmitkPropertiesTableModel(mitk::PropertyList* _PropertyList=0, QObject* parent = 0);

  ///
  /// Standard dtor. Nothing to do here.
  virtual ~QmitkPropertiesTableModel();

  ///
  /// Sets the node. Resets the whole model. If _Node is NULL the model is empty.
  ///
  void setPropertyList(mitk::PropertyList* _PropertyList);

  ///
  /// Get the node.
  /// 
  //mitk::DataTreeNode::Pointer getNode() const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  Qt::ItemFlags flags(const QModelIndex& index) const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  ///
  /// Overridden from QAbstractTableModel. Sets data at index for given role.
  ///
  bool setData(const QModelIndex &index, const QVariant &value,
    int role);

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the number of columns. That is usually two in this model:
  /// the properties name and its value.
  int columnCount(const QModelIndex &parent) const;

  ///
  /// \brief Called when a single property was changed. Send a model changed event to the Qt-outer world.
  ///
  virtual void PropertyModified(const itk::Object *caller, const itk::EventObject &event);

  ///
  /// \brief Called when the nodes property list got modified. Call reset() on the whole model.
  ///
  virtual void PropertyListModified(const itk::Object *caller, const itk::EventObject &event);

  void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );

protected:
  /// 
  /// Holds the the properties list.
  mitk::PropertyList* m_PropertyList;

  /// Store the properties in a vector so that they may be sorted
  std::vector<std::pair<std::string,std::pair<mitk::BaseProperty::Pointer,bool> > > m_PropertyListElements;

  unsigned long m_PropertyListModifiedObserverTag;

  ///
  /// \brief Indicates if this class should neglect all incoming events because
  /// the class itself triggered the event (in this case when a property was edited).
  ///
  bool m_BlockEvents;

  std::map<std::string, unsigned long> m_PropertyModifiedObserverTags;

  bool m_SortDescending;
};

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
