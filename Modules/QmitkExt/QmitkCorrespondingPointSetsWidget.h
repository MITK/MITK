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

public:
    QmitkCorrespondingPointSetsWidget(QWidget *parent = 0);
    ~QmitkCorrespondingPointSetsWidget();

    /// assign point sets (contained in a node of DataStorage) for observation
    void SetPointSetNodes(std::vector<mitk::DataNode*> nodes);

    std::vector<mitk::DataNode*> GetPointSetNodes();

    /// assign a QmitkStdMultiWidget for updating render window crosshair
    void SetMultiWidget(QmitkStdMultiWidget* multiWidget);

    void UpdateSelection(mitk::DataNode* selectedNode);

public slots:

signals:

protected slots:

protected:

    void SetupUi();

    QmitkCorrespondingPointSetsView* m_CorrespondingPointSetsView;
};

#endif
