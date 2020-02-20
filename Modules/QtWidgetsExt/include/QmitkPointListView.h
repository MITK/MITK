/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITK_POINTLIST_VIEW_H_INCLUDED
#define QMITK_POINTLIST_VIEW_H_INCLUDED

#include "MitkQtWidgetsExtExports.h"
#include "QmitkPointListModel.h"
#include <QLabel>
#include <QListView>
#include <mitkSliceNavigationController.h>

class QmitkStdMultiWidget;

/*!
* \brief GUI widget for handling mitk::PointSet
*
* Displays all the points in a mitk::PointSet graphically.
* Reacts automatically to changes in the PointSet's selection status.
* Updates PointSet's selection status when this list's selection changes.
*
* If a QmitkStdMultiWidget is assigned via SetMultiWidget(), the
* crosshair of the QmitkStdMultiWidget is moved to the currently selected
* point.
*
*/
class MITKQTWIDGETSEXT_EXPORT QmitkPointListView : public QListView
{
  Q_OBJECT

public:
  QmitkPointListView(QWidget *parent = nullptr);
  ~QmitkPointListView() override;

  /// assign a point set for observation
  void SetPointSetNode(mitk::DataNode *pointSetNode);

  /// which point set to work on
  const mitk::PointSet *GetPointSet() const;

  /**
  * \brief If Multiwidget is set, the crosshair is automatically centering to the selected point
  * As an alternative, if you dont have a multiwidget, you can call SetSnc1, SetSnc2, SetSnc3 to set the
  * SliceNavigationControllers directly to enable the focussing feature.
  */
  void SetMultiWidget(QmitkStdMultiWidget *multiWidget);

  QmitkStdMultiWidget *GetMultiWidget()
    const; ///< return the QmitkStdMultiWidget that is used for updating render window crosshair

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

signals:

  void SignalPointSelectionChanged(); ///< this signal is emmitted, if the selection of a point in the pointset is changed
  void SignalTimeStepChanged(int);

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
  QmitkStdMultiWidget *m_MultiWidget;
};

#endif
