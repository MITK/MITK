/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkPointListWidget_h
#define QmitkPointListWidget_h

#include <MitkQtWidgetsExtExports.h>

#include <QmitkPointListModel.h>
#include <QmitkPointListView.h>

#include <mitkDataInteractor.h>
#include <mitkDataNode.h>
#include <mitkPointSet.h>

#include <QPushButton>
#include <QToolButton>

class QmitkAbstractMultiWidget;

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
 * a QmitkAbstractMultiWidget object.
 */

class MITKQTWIDGETSEXT_EXPORT QmitkPointListWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Construct the point list widget.
   * \param[in] parent The parent widget.
   * \param[in] orientation Layout orientation: 0 = vertical (default), 1 or 2 = horizontal variants.
   */
  QmitkPointListWidget(QWidget *parent = nullptr, int orientation = 0);

  /** \brief Destructor. */
  ~QmitkPointListWidget() override;

  /** \brief Connect internal widget signals and slots. Called during construction. */
  void SetupConnections();

  /**
   * \brief Add a mitk::SliceNavigationController for point-based crosshair navigation.
   * \param[in] snc The mitk::SliceNavigationController instance to add.
   */
  void AddSliceNavigationController(mitk::SliceNavigationController *snc);

  /**
   * \brief Remove a mitk::SliceNavigationController.
   * \param[in] snc The mitk::SliceNavigationController instance to remove.
   */
  void RemoveSliceNavigationController(mitk::SliceNavigationController *snc);

  /**
   * \brief Set the point set data on the existing data node.
   * \param[in] newPs The point set to assign.
   */
  void SetPointSet(mitk::PointSet *newPs);

  /**
   * \brief Get the currently managed point set.
   * \return Pointer to the mitk::PointSet, or nullptr.
   */
  mitk::PointSet *GetPointSet();

  /**
   * \brief Set the data node containing the point set.
   * \param[in] newNode The data node with a mitk::PointSet.
   */
  void SetPointSetNode(mitk::DataNode *newNode);

  /**
   * \brief Get the current point set data node.
   * \return Pointer to the mitk::DataNode.
   */
  mitk::DataNode *GetPointSetNode();

  /**
   * \brief Set a multi widget for automatic crosshair navigation to selected points.
   * \param[in] multiWidget The QmitkAbstractMultiWidget, or nullptr to clear.
   */
  void SetMultiWidget(QmitkAbstractMultiWidget*multiWidget);

  /**
   * \brief ITK observer callback for data node deletion events.
   * \param[in] e The ITK event object.
   */
  void OnNodeDeleted(const itk::EventObject &e);

  /** \brief Programmatically uncheck the "add point" toggle button. */
  void UnselectEditButton();

public slots:
  /**
   * \brief Deactivate the point set interactor.
   * \param[in] deactivate True to deactivate (currently unused).
   */
  void DeactivateInteractor(bool deactivate);

  /**
   * \brief Enable or disable the "add point" toggle button.
   * \param[in] enabled True to enable the button.
   */
  void EnableEditButton(bool enabled);

signals:
  /**
   * \brief Emitted when the point editing interactor is activated or deactivated.
   * \param[in] active True if point editing is now active.
   */
  void EditPointSets(bool active);

  /** \brief Emitted when the point selection changes in the point set. */
  void PointSelectionChanged();

  /** \brief Emitted when points are loaded, cleared, added, or removed. */
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
