/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPointListViewWidget_h
#define QmitkPointListViewWidget_h

#include "MitkQtWidgetsExtExports.h"
#include <QListWidget>

#include <mitkPointSet.h>
#include <mitkWeakPointer.h>

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
class MITKQTWIDGETSEXT_EXPORT QmitkPointListViewWidget : public QListWidget
{
  Q_OBJECT

signals:
  void PointSelectionChanged(); ///< this signal is emmitted, if the selection of a point in the pointset is changed
public:
  QmitkPointListViewWidget(QWidget *parent = nullptr);
  ~QmitkPointListViewWidget() override;

  /// assign a point set for observation
  void SetPointSet(mitk::PointSet *pointSet);

  /// which point set to work on
  const mitk::PointSet *GetPointSet() const;

  void SetMultiWidget(
    QmitkStdMultiWidget *multiWidget); ///< assign a QmitkStdMultiWidget for updating render window crosshair

  QmitkStdMultiWidget *GetMultiWidget()
    const; ///< return the QmitkStdMultiWidget that is used for updating render window crosshair

  /// which time step to display/model
  void SetTimeStep(int t);

  /// which time step to display/model
  int GetTimeStep() const;

  /// observer for point set "modified" events
  void OnPointSetChanged();

  /// observer for point set "delete" events
  void OnPointSetDeleted();

protected slots:
  ///
  /// Filtering double click event for editing point coordinates via a dialog
  ///
  void OnItemDoubleClicked(QListWidgetItem *item);

  /// called when the selection of the view widget changes
  void OnCurrentRowChanged(int /*currentRow*/);

protected:
  void keyPressEvent(QKeyEvent *e) override; ///< react to F2, F3 and DEL keys
  void MoveSelectedPointUp();
  void MoveSelectedPointDown();
  void RemoveSelectedPoint();
  void Update(bool currentRowChanged = false);

protected:
  mitk::WeakPointer<mitk::PointSet> m_PointSet;

  unsigned long m_PointSetDeletedTag;
  unsigned long m_PointSetModifiedTag;

  int m_TimeStep;

  bool m_SelfCall;

  /// used to position the planes on a selected point
  QmitkStdMultiWidget *m_MultiWidget;
};

#endif
