/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
  QmitkPlanarFiguresTableModel(mitk::DataStorage::Pointer _DataStorage, mitk::NodePredicateBase* _Predicate = 0
    , QObject* parent = 0 ); 

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
    int role) const;
  ///
  /// Overridden from QAbstractTableModel. Returns what can be done
  /// with an item.
  ///
  Qt::ItemFlags flags(const QModelIndex &index) const;
  ///
  /// Overridden from QAbstractTableModel. Returns the number of features (columns) to display.
  ///
  int columnCount(const QModelIndex &parent) const;
  ///
  /// Overridden from QAbstractTableModel. Returns the data at index for given role.
  ///
  QVariant data(const QModelIndex &index, int role) const;

};

#endif

