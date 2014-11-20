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

#ifndef QMITK_POINTLIST_VIEW_H_INCLUDED
#define QMITK_POINTLIST_VIEW_H_INCLUDED

#include <QTableView>
#include <QLabel>
#include <mitkDataStorage.h>
#include "MitkQtWidgetsExtExports.h"


#include "QmitkCorrespondingPointSetsModel.h"

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
* \deprecatedSince{2014_10} mitk::QmitkCorrespondingPointSetsModel is deprecated.
* Most Functionality is covered by QmitkPointListModel
*
* Alternatively this needs to be updated to the new interaction-framework.
* Refer to \see DataInteractionPage for general information about the concept of the new implementation.
*/
class MitkQtWidgetsExt_EXPORT QmitkCorrespondingPointSetsView : public QTableView
{
  Q_OBJECT

public:

  QmitkCorrespondingPointSetsView( QWidget* parent = 0 );
  ~QmitkCorrespondingPointSetsView();

  /// assign a point set for observation
  void SetPointSetNodes( std::vector<mitk::DataNode*> nodes );

  void SetMultiWidget( QmitkStdMultiWidget* multiWidget ); ///< assign a QmitkStdMultiWidget for updating render window crosshair

  QmitkStdMultiWidget* GetMultiWidget() const;  ///< return the QmitkStdMultiWidget that is used for updating render window crosshair

  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  std::vector<mitk::DataNode*> GetPointSetNodes();

  void UpdateSelection(mitk::DataNode* selectedNode);

  // return true if a point from one of the displayed point lists is selected
  bool IsPointSelected();

  QmitkCorrespondingPointSetsModel* GetModel();

signals:

  void SignalPointSelectionChanged();

  void SignalAddPointsModeChanged(bool);

public slots:

  void AddPointSet();

  void RemoveSelectedPoint();

  void MoveSelectedPointDown();

  void MoveSelectedPointUp();

  void AddPointsMode(bool checked);

  void SwapPointSets(bool checked);

protected slots:

  /// called when the selection of the view widget changes
  void OnPointSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

   /// fade the shown timestep out
  void fadeTimeStepOut();

  /// open ContextMenu
  void ctxMenu(const QPoint &pos);


  void ClearSelectedPointSet();

  void ClearCurrentTimeStep();

  void UpdateSelectionHighlighting();

protected:

  void keyPressEvent( QKeyEvent * e ); ///< react to F2, F3 and DEL keys
  void wheelEvent( QWheelEvent* event); ///< change timestep of the current pointset by mouse wheel
  void fadeTimeStepIn(); ///< fade a label with the currently shown timestep in

protected:

  std::vector<mitk::PointSet*> GetPointSets();

  QmitkCorrespondingPointSetsModel*    m_CorrespondingPointSetsModel;

  bool  m_SelfCall;
  bool  m_swapPointSets;
  bool  m_addPointsMode;

  QLabel*                m_TimeStepFaderLabel;
  mitk::DataStorage::Pointer m_DataStorage;
};

#endif

