/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkPointListWidget_H
#define QmitkPointListWidget_H

#include "MitkQtWidgetsExtExports.h"
#include <QmitkPointListModel.h>
#include <QmitkPointListView.h>

#include <QmitkStdMultiWidget.h>
#include <mitkDataInteractor.h>
#include <mitkDataNode.h>
#include <mitkPointSet.h>

#include <QPushButton>
#include <QToolButton>

/*!
 * \brief Widget for regular operations on point sets
 *
 * Displays a list of point coordinates and a couple of
 * buttons which
 *
 * \li enable point set interaction
 * \li clear all points from a set
 * \li load points from file
 * \li save points to file
 *
 * The user/application module of this widget needs to
 * assign a mitk::PointSet object to this widget. The user
 * also has to decide whether it wants to put the point set
 * into (a) DataStorage. This widget will not add/remove
 * point sets to DataStorage.
 *
 * If the render window crosshair should be moved to the
 * currently selected point, the widget user has to provide
 * a QmitkStdMultiWidget object.
 */

class MITKQTWIDGETSEXT_EXPORT QmitkPointListWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkPointListWidget(QWidget *parent = nullptr, int orientation = 0);
  ~QmitkPointListWidget() override;

  void SetupConnections();

  /**
   * @brief Add a mitk::SliceNavigationController instance.
   * @param snc The mitk::SliceNavigationController instance.
   *
   * This method adds \c snc to the set of slice navigation controllers which are
   * used to navigate to the selected point.
   */
  void AddSliceNavigationController(mitk::SliceNavigationController *snc);

  /**
   * @brief Remove a mitk::SliceNavigationController instance.
   * @param snc The mitk::SliceNavigationController instance.
   *
   * This method removes \c snc from the set of slice navigation controllers which are
   * used to navigate to the selected point.
   */
  void RemoveSliceNavigationController(mitk::SliceNavigationController *snc);

  /** @brief assign a point set (contained in a node of DataStorage) for observation */
  void SetPointSet(mitk::PointSet *newPs);
  mitk::PointSet *GetPointSet();

  /** @brief assign a point set (contained in a node of DataStorage) for observation */
  void SetPointSetNode(mitk::DataNode *newNode);
  mitk::DataNode *GetPointSetNode();

  /** @brief assign a QmitkStdMultiWidget for updating render window crosshair */
  void SetMultiWidget(QmitkStdMultiWidget *multiWidget);

  /** @brief itk observer for node "delete" events */
  void OnNodeDeleted(const itk::EventObject &e);

  /** @brief Unselects the edit button if it is selected. */
  void UnselectEditButton();

public slots:
  void DeactivateInteractor(bool deactivate);
  void EnableEditButton(bool enabled);

signals:
  /** @brief signal to inform about the state of the EditPointSetButton, whether an interactor for setting points is
   * active or not */
  void EditPointSets(bool active);
  /// signal to inform that the selection of a point in the pointset has changed
  void PointSelectionChanged();
  /// signal to inform about cleared or loaded point sets
  void PointListChanged();

protected slots:
  void OnBtnSavePoints();
  void OnBtnLoadPoints();
  void RemoveSelectedPoint();
  void MoveSelectedPointDown();
  void MoveSelectedPointUp();
  void OnBtnAddPoint(bool checked);
  void OnBtnAddPointManually();
  void OnTimeStepChanged(int timeStep);

  /*!
  \brief pass through signal from PointListView that point selection has changed
  */
  void OnPointSelectionChanged();

  void OnListDoubleClick();

protected:
  void SetupUi();
  void ObserveNewNode(mitk::DataNode *node);

  QmitkPointListView *m_PointListView;

  mitk::DataNode::Pointer m_PointSetNode;

  int m_Orientation;

  QPushButton *m_MovePointUpBtn;
  QPushButton *m_MovePointDownBtn;
  QPushButton *m_RemovePointBtn;
  QPushButton *m_SavePointsBtn;
  QPushButton *m_LoadPointsBtn;
  QPushButton *m_ToggleAddPoint;
  QPushButton *m_AddPoint;
  QLabel *m_TimeStepDisplay;
  QLabel *m_TimeStepLabel;

  mitk::DataInteractor::Pointer m_DataInteractor;
  int m_TimeStep;
  bool m_EditAllowed;
  unsigned long m_NodeObserverTag;

  QmitkPointListModel *m_PointListModel;
};

#endif
