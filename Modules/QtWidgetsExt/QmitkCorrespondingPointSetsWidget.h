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

#ifndef QmitkCorrespondingPointSetsWidget_H
#define QmitkCorrespondingPointSetsWidget_H

#include <QmitkCorrespondingPointSetsView.h>
#include <QmitkCorrespondingPointSetsModel.h>
#include "MitkQtWidgetsExtExports.h"

#include <mitkDataNode.h>
#include <mitkPointSet.h>
#include <mitkPointSetInteractor.h>
#include <QmitkStdMultiWidget.h>
#include <QToolButton>
#include <QPushButton>

/*!
 * \brief Widget for regular operations on two point sets
 *
 * Displays two sets of point coordinates, buttons and a context menu to enable modifications to the point sets
 *
 * \li creation of new point sets
 * \li adding/removing points
 * \li move points
 * \li clear all points from a set
 * \li clear all points in one time step of one set
 *
 * The user/application module of this widget needs to
 * assign two visible point sets to the widget
 * (via SetPointSetNodes(std::vector<mitk::DataNode*> nodes)),
 * preferrably by passing all visible data nodes contained
 * in the data storage to the widget every time
 * DataStorageChanged() is called.
 *
 * The user/application module of this widget needs to
 * assign one selected point set to the widget
 * (via UpdateSelection(mitk::DataNode* selectedNode)),
 * preferrably by passing the selected data node contained
 * in the data storage to the widget every time
 * OnSelectionChanged() is called.
 *
 * The user/application module of this widget needs to
 * assign a QmitkStdMultiWidget and a mitk::DataStorage
 * to the widget (via SetMultiWidget and SetDataStorage).
 */
class MitkQtWidgetsExt_EXPORT QmitkCorrespondingPointSetsWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool QTPropShowButtonBar READ QTPropButtonBarEnabled WRITE QTPropSetButtonBarEnabled)

public:
    QmitkCorrespondingPointSetsWidget(QWidget *parent = 0);
    ~QmitkCorrespondingPointSetsWidget();

    /// calls SetPointSetNodes of the according QmitkCorrespondingPointSetsView
    void SetPointSetNodes(std::vector<mitk::DataNode*> nodes);

    /// returns the point set nodes contained in the table model
    /// calls GetPointSetNodes of the according QmitkCorrespondingPointSetsView
    std::vector<mitk::DataNode*> GetPointSetNodes();

    /// calls SetMultiWidget of the according QmitkCorrespondingPointSetsView
    void SetMultiWidget(QmitkStdMultiWidget* multiWidget);

    /// calls SetDataStorage of the according QmitkCorrespondingPointSetsView
    void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

    /// calls UpdateSelection of the according QmitkCorrespondingPointSetsView
    void UpdateSelection(mitk::DataNode* selectedNode);

    /// returns the qt property which indicates an activated/deactivated button bar below the table
    bool QTPropButtonBarEnabled() const;

    /// sets the qt property which activates/deactivates the button bar below the table
    void QTPropSetButtonBarEnabled(bool showBB);

protected slots:

  /// enables/disables buttons if a/no point is selected
  void OnPointSelectionChanged();

  /// add new point set to data manager
  void AddPointSet();

  /// enable if new points should be a added
  void AddPointsMode(bool checked);

  void RemoveSelectedPoint();

  void MoveSelectedPointDown();

  void MoveSelectedPointUp();

  /// toggles m_AddPointsBtn checked state
  void OnAddPointsModeChanged(bool enabled);

  /// swap the two table columns columns
  void SwapPointSets(bool checked);

protected:

    void SetupUi();

    bool QTPropShowButtonBar;
    QmitkCorrespondingPointSetsView* m_CorrespondingPointSetsView;

    QToolButton* m_CreatePointSetBtn;
    QToolButton* m_MovePointUpBtn;
    QToolButton* m_MovePointDownBtn;
    QToolButton* m_RemovePointBtn;
    QToolButton* m_AddPointsBtn;
    QToolButton* m_SwapSetsBtn;
};

#endif

