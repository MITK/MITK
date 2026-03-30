/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPointListModel_h
#define QmitkPointListModel_h

#include <MitkQtWidgetsExtExports.h>
#include <QAbstractListModel>

#include <mitkDataNode.h>
#include <mitkPointSet.h>

/**
 * \brief Qt list model providing access to points in a mitk::PointSet.
 *
 * Wraps a mitk::PointSet (referenced via a mitk::DataNode) as a
 * QAbstractListModel. Each row represents a point with its ID and
 * coordinates. Observes the point set for modifications and deletions,
 * resetting the model accordingly. Provides methods to move and remove
 * the currently selected point.
 *
 * \note The mitk::PointSet uses a map container where point IDs are not
 *       necessarily contiguous. This model translates between row indices
 *       and point IDs.
 *
 * \sa QmitkPointListView, QmitkPointListWidget
 */
class MITKQTWIDGETSEXT_EXPORT QmitkPointListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  /**
   * \brief Construct the model.
   * \param[in] pointSetNode Data node containing the mitk::PointSet. May be nullptr.
   * \param[in] t The time step to use.
   * \param[in] parent The parent QObject.
   */
  QmitkPointListModel(mitk::DataNode *pointSetNode = nullptr, int t = 0, QObject *parent = nullptr);

  /** \brief Destructor. Removes observers from the point set. */
  ~QmitkPointListModel() override;

  /**
   * \brief Return item flags (selectable and enabled).
   * \param[in] index The model index (unused).
   * \return Qt::ItemIsSelectable | Qt::ItemIsEnabled.
   */
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  /**
   * \brief Return the number of points in the current time step.
   * \param[in] parent The parent index (unused for list models).
   * \return The number of points.
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Return the display data for a point (ID and coordinates).
   * \param[in] index The model index.
   * \param[in] role The data role (only Qt::DisplayRole is supported).
   * \return The point string in format "ID: (x, y, z)".
   */
  QVariant data(const QModelIndex &index, int role) const override;

  /**
   * \brief Return header data.
   * \param[in] section The section index.
   * \param[in] orientation The header orientation.
   * \param[in] role The data role.
   * \return The header label.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  /**
   * \brief Set the data node containing the point set to model.
   * \param[in] pointSetNode The data node. May be nullptr to clear.
   */
  void SetPointSetNode(mitk::DataNode *pointSetNode);

  /**
   * \brief Get the modeled point set.
   * \return Pointer to the mitk::PointSet, or nullptr.
   */
  mitk::PointSet *GetPointSet() const;

  /**
   * \brief Get the data node containing the point set.
   * \return Pointer to the mitk::DataNode, or nullptr.
   */
  mitk::DataNode *GetPointSetNode() const;

  /**
   * \brief Set the time step to display.
   * \param[in] t The time step index.
   */
  void SetTimeStep(int t);

  /**
   * \brief Get the current time step.
   * \return The time step index.
   */
  int GetTimeStep() const;

  /**
   * \brief ITK observer callback for point set modification events.
   * \param[in] e The ITK event object.
   */
  void OnPointSetChanged(const itk::EventObject &e);

  /**
   * \brief ITK observer callback for point set deletion events.
   * \param[in] e The ITK event object.
   */
  void OnPointSetDeleted(const itk::EventObject &e);

  /**
  * \brief get point and point ID that correspond to a given QModelIndex
  *
  * The mitk::PointSet uses a map to store points in an ID<-->Point relation.
  * The IDs are not necessarily continuously numbered, therefore, we can not
  * directly use the QModelIndex as point ID. This method returns the point and
  * the corresponding point id for a given QModelIndex. The point and the point ID
  * are returned in the outgoing parameters p and id. If a valid point and ID were
  * found, the method returns true, otherwise it returns false
  * \param[in] index the index for which a point is requested.
  The row() part of the index is used to find a corresponding point
  * \param[out] p If a valid point is found, it will be stored in the p parameter
  * \param[out] id If a valid point is found, the corresponding ID will be stored in id
  * \return Returns true, if a valid point was found, false otherwise
  */
  bool GetPointForModelIndex(const QModelIndex &index,
                             mitk::PointSet::PointType &p,
                             mitk::PointSet::PointIdentifier &id) const;

  /**Documentation
  * \brief returns a QModelIndex for a given point ID
  *
  * The mitk::PointSet uses a map to store points in an ID<-->Point relation.
  * The IDs are not necessarily continuously numbered, therefore, we can not
  * directly use the point ID as a QModelIndex. This method returns a QModelIndex
  * for a given point ID in the outgoing parameter index.
  * \param[in] id The point ID for which the QModelIndex will be created
  * \param[out] index if a point with the ID id was found, index will contain a corresponding QModelIndex
  * for that point
  * \return returns true, if a valid QModelIndex was created, false otherwise
  */
  bool GetModelIndexForPointID(mitk::PointSet::PointIdentifier id, QModelIndex &index) const;

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
  void ObserveNewPointSet(mitk::DataNode *pointSetNode);

  // initially checks if there is a PointSet as data in the DataNode.
  // returns PointSet if so and nullptr if other data is set to node
  mitk::PointSet *CheckForPointSetInNode(mitk::DataNode *node) const;

protected:
  mitk::DataNode *m_PointSetNode;
  unsigned int m_PointSetModifiedObserverTag;
  unsigned int m_PointSetDeletedObserverTag;
  int m_TimeStep;
};

#endif
