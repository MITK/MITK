/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPointListView_h
#define QmitkPointListView_h

#include <MitkQtWidgetsExtExports.h>

#include <QmitkPointListModel.h>

#include <QLabel>
#include <QListView>
#include <mitkSliceNavigationController.h>

class QmitkAbstractMultiWidget;

/**
 * \brief GUI widget for displaying and interacting with a mitk::PointSet.
 *
 * Displays all points in a mitk::PointSet as a list view. Automatically
 * reacts to changes in the PointSet's selection status and updates the
 * PointSet's selection when the list selection changes. Double-clicking
 * a point opens a QmitkEditPointDialog for coordinate editing.
 *
 * If a QmitkAbstractMultiWidget is assigned via SetMultiWidget(), or
 * SliceNavigationControllers are added, the crosshair navigates to the
 * selected point. The mouse wheel changes the displayed time step.
 *
 * Key bindings: F2 moves point up, F3 moves point down, Delete removes the point.
 *
 * \sa QmitkPointListModel, QmitkPointListWidget, QmitkEditPointDialog
 */
class MITKQTWIDGETSEXT_EXPORT QmitkPointListView : public QListView
{
  Q_OBJECT

public:
  /**
   * \brief Construct the point list view.
   * \param[in] parent The parent widget.
   */
  QmitkPointListView(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkPointListView() override;

  /**
   * \brief Set the data node containing the point set to display.
   * \param[in] pointSetNode The data node with a mitk::PointSet.
   */
  void SetPointSetNode(mitk::DataNode *pointSetNode);

  /**
   * \brief Get the displayed point set.
   * \return Const pointer to the mitk::PointSet, or nullptr.
   */
  const mitk::PointSet *GetPointSet() const;

  /**
   * \brief Set a multi widget for automatic crosshair centering on selected points.
   *
   * When set, all render windows of the multi widget are registered as
   * slice navigation controllers for point-based navigation.
   *
   * \param[in] multiWidget The multi widget to use, or nullptr to clear.
   */
  void SetMultiWidget(QmitkAbstractMultiWidget* multiWidget);

  /**
   * \brief Get the currently assigned multi widget.
   * \return Pointer to the QmitkAbstractMultiWidget, or nullptr.
   */
  QmitkAbstractMultiWidget* GetMultiWidget() const;

  /**
   * \brief Add a slice navigation controller for point-based crosshair navigation.
   * \param[in] snc The mitk::SliceNavigationController to add. Ignored if nullptr.
   */
  void AddSliceNavigationController(mitk::SliceNavigationController *snc);

  /**
   * \brief Remove a slice navigation controller.
   * \param[in] snc The mitk::SliceNavigationController to remove. Ignored if nullptr.
   */
  void RemoveSliceNavigationController(mitk::SliceNavigationController *snc);

signals:
  /** \brief Emitted when the point selection changes in the point set. */
  void SignalPointSelectionChanged();

  /**
   * \brief Emitted when the time step changes via mouse wheel.
   * \param[in] timeStep The new time step index.
   */
  void SignalTimeStepChanged(int timeStep);

protected slots:

  /// Filtering double click event for editing point coordinates via a dialog
  void OnPointDoubleClicked(const QModelIndex &index);

  /// called when the point set data structure changes
  void OnPointSetSelectionChanged();

  /// called when the selection of the view widget changes
  void OnListViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

  /// open ContextMenu
  void ctxMenu(const QPoint &pos);

  /// Turn TimeStep Fading On/Off
  void SetFading(bool onOff);

  /// Delete all points in the list
  void ClearPointList();

  /// delete all points in the list in the current timestep
  void ClearPointListTS();

protected:
  void keyPressEvent(QKeyEvent *e) override;    ///< react to F2, F3 and DEL keys
  void wheelEvent(QWheelEvent *event) override; ///< change timestep of the current pointset by mouse wheel

  std::set<mitk::SliceNavigationController *> m_Sncs;

  QmitkPointListModel *m_PointListModel;
  bool m_SelfCall;
  bool m_showFading;

  /// used to position the planes on a selected point
  QmitkAbstractMultiWidget* m_MultiWidget;
};

#endif
