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

#ifndef QmitkPlanarFiguresTableModel_h
#define QmitkPlanarFiguresTableModel_h

/// Toolkit includes.
#include <QmitkDataStorageTableModel.h>

/// Forward declarations.

///
/// \class QmitkPlanarFiguresTableModel
///
/// \brief A table model for a set of DataNodes defined by a predicate.
/// \TODO make columns interchangeable, select which properties to show as columns
///
class QmitkPlanarFiguresTableModel : public QmitkDataStorageTableModel
{
  Q_OBJECT

    //#Ctors/Dtor
public:
  ///
  /// Constructs a new QmitkPlanarFiguresTableModel and sets a predicate that defines
  /// this list.
  /// \see setPredicate()
  ///
  QmitkPlanarFiguresTableModel(mitk::DataStorage::Pointer _DataStorage, mitk::NodePredicateBase* _Predicate = nullptr
    , QObject* parent = nullptr );

  ///
  /// Standard dtor. Delete predicate, disconnect from DataStorage.
  ///
  virtual ~QmitkPlanarFiguresTableModel();

  //# Public GETTER
public:
  ///
  /// Overridden from QAbstractTableModel. Returns the header data at section
  /// for given orientation and role.
  ///
  QVariant headerData(int section, Qt::Orientation orientation,
    int role) const override;
  ///
  /// Overridden from QAbstractTableModel. Returns what can be done
  /// with an item.
  ///
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  ///
  /// Overridden from QAbstractTableModel. Returns the number of features (columns) to display.
  ///
  int columnCount(const QModelIndex &parent) const override;
  ///
  /// Overridden from QAbstractTableModel. Returns the data at index for given role.
  ///
  QVariant data(const QModelIndex &index, int role) const override;

};

#endif

