#ifndef QmitkCorrespondingPointSetsWidget_H
#define QmitkCorrespondingPointSetsWidget_H

#include <QmitkCorrespondingPointSetsView.h>
#include <QmitkCorrespondingPointSetsModel.h>
#include "QmitkExtExports.h"

#include <mitkDataNode.h>
#include <mitkPointSet.h>
#include <mitkPointSetInteractor.h>
#include <QmitkStdMultiWidget.h>


#include <QPushButton>

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


class QmitkExt_EXPORT QmitkCorrespondingPointSetsWidget : public QWidget
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

    bool QTPropButtonBarEnabled() const;

    void QTPropSetButtonBarEnabled(bool showBB);

signals:

protected slots:

  void OnPointSelectionChanged();
  
  void AddPointSet();

  void AddPointsMode(bool checked);
  
  void RemoveSelectedPoint();

  void MoveSelectedPointDown();
  
  void MoveSelectedPointUp();

  void OnAddPointsModeChanged(bool enabled);

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
