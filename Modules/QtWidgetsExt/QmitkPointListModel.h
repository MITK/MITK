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

#ifndef QMITK_POINTLIST_MODEL_H_INCLUDED
#define QMITK_POINTLIST_MODEL_H_INCLUDED

#include <QAbstractListModel>
#include "MitkQtWidgetsExtExports.h"

#include "mitkDataNode.h"
#include "mitkPointSet.h"

class MitkQtWidgetsExt_EXPORT QmitkPointListModel : public QAbstractListModel
{
  Q_OBJECT

public:

  QmitkPointListModel( mitk::DataNode* = NULL, int t = 0, QObject* parent = 0 );
  ~QmitkPointListModel();

  Qt::ItemFlags flags(const QModelIndex&) const;

  /// interface of QAbstractListModel
  int rowCount( const QModelIndex& parent = QModelIndex() ) const;

  /// interface of QAbstractListModel
  QVariant data(const QModelIndex& index, int role) const;

  /// interface of QAbstractListModel
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;

  /// which point set to work on
  void SetPointSetNode( mitk::DataNode* pointSetNode );

  /// which point set to work on
  mitk::PointSet* GetPointSet() const;

  // which point set to work on
  mitk::DataNode* GetPointSetNode() const;

  /// which time step to display/model
  void SetTimeStep(int t);

  /// which time step to display/model
  int GetTimeStep() const;

  /// itk observer for point set "modified" events
  void OnPointSetChanged( const itk::EventObject & e );

  /// itk observer for point set "delete" events
  void OnPointSetDeleted( const itk::EventObject & e );

  /**
  * \brief get point and point ID that correspond to a given QModelIndex
  *
  * The mitk::PointSet uses a map to store points in an ID<-->Point relation.
  * The IDs are not neccesarily continuously numbered, therefore, we can not
  * directly use the QModelIndex as point ID. This method returns the point and
  * the corresponding point id for a given QModelIndex. The point and the point ID
  * are returned in the outgoing parameters p and id. If a valid point and ID were
  * found, the method returns true, otherwise it returns false
  * \param[in] QModelIndex &index the index for which a point is requested.
  The row() part of the index is used to find a corresponding point
  * \param[out] mitk::Point3D& p If a valid point is found, it will be stored in the p parameter
  * \param[out] mitk::PointSet::PointIdentifier& id If a valid point is found, the corresponding ID will be stored in id
  * \return Returns true, if a valid point was found, false otherwise
  */
  bool GetPointForModelIndex( const QModelIndex &index, mitk::PointSet::PointType& p, mitk::PointSet::PointIdentifier& id) const;

  /**Documentation
  * \brief returns a QModelIndex for a given point ID
  *
  * The mitk::PointSet uses a map to store points in an ID<-->Point relation.
  * The IDs are not neccesarily continuously numbered, therefore, we can not
  * directly use the point ID as a QModelIndex. This method returns a QModelIndex
  * for a given point ID in the outgoing parameter index.
  * \param[in] mitk::PointSet::PointIdentifier id The point ID for which the QModelIndex will be created
  * \param[out] QModelIndex& index if a point with the ID id was found, index will contain a corresponding QModelIndex for that point
  * \return returns true, if a valid QModelIndex was created, false otherwise
  */
  bool GetModelIndexForPointID(mitk::PointSet::PointIdentifier id, QModelIndex& index) const;


public slots:

  void MoveSelectedPointUp();

  void MoveSelectedPointDown();

  void RemoveSelectedPoint();


signals:

    /// emitted, when views should update their selection status
    /// (because mouse interactions in render windows can change
    /// the selection status of points)
    void SignalUpdateSelection();

protected:

  /// internally observe different point set
  void ObserveNewPointSet( mitk::DataNode* pointSetNode );

  //initially checks if there is a PointSet as data in the DataNode.
  //returns PointSet if so and NULL if other data is set to node
  mitk::PointSet* CheckForPointSetInNode(mitk::DataNode* node) const;

protected:

  mitk::DataNode* m_PointSetNode;
  unsigned int    m_PointSetModifiedObserverTag;
  unsigned int    m_PointSetDeletedObserverTag;
  int             m_TimeStep;
};

#endif

