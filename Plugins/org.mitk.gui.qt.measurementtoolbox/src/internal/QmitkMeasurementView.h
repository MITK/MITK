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

#ifndef QMITK_MEASUREMENT_H__INCLUDED
#define QMITK_MEASUREMENT_H__INCLUDED

#include <QmitkAbstractView.h>
#include <mitkEventConfig.h>
#include <mitkILifecycleAwarePart.h>
#include "usServiceRegistration.h"

/// forward declarations
struct QmitkMeasurementViewData;
namespace mitk
{
  class PlanarFigure;
}

///
/// A view for doing measurements in digital images by means of
/// mitk::Planarfigures which can represent drawing primitives (Lines, circles, ...).
/// The view consists of only three main elements:
/// 1. A toolbar for activating PlanarFigure drawing
/// 2. A textbrowser which shows details for the selected PlanarFigures
/// 3. A button for copying all details to the clipboard
///
class QmitkMeasurementView : public QmitkAbstractView
{
  Q_OBJECT

  public:

    static const std::string VIEW_ID;
    QmitkMeasurementView();
    virtual ~QmitkMeasurementView();

    void CreateQtPartControl(QWidget* parent) override;
    void SetFocus() override;

    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part,
      const QList<mitk::DataNode::Pointer> &nodes) override;

    void NodeAdded(const mitk::DataNode* node) override;
    void NodeChanged(const mitk::DataNode* node) override;
    void NodeRemoved(const mitk::DataNode* node) override;

    void PlanarFigureSelected( itk::Object* object, const itk::EventObject& );
  protected slots:
    ///# draw actions
    void ActionDrawLineTriggered( bool checked = false );
    void ActionDrawPathTriggered( bool checked = false );
    void ActionDrawAngleTriggered( bool checked = false );
    void ActionDrawFourPointAngleTriggered( bool checked = false );
    void ActionDrawCircleTriggered( bool checked = false );
    void ActionDrawEllipseTriggered( bool checked = false );
    void ActionDrawDoubleEllipseTriggered( bool checked = false );
    void ActionDrawRectangleTriggered( bool checked = false );
    void ActionDrawPolygonTriggered( bool checked = false );
    void ActionDrawBezierCurveTriggered( bool checked = false );
    void ActionDrawSubdivisionPolygonTriggered( bool checked = false );
    void CopyToClipboard( bool checked = false );

  private:
    void CreateConnections();
    mitk::DataNode::Pointer AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name);
    void UpdateMeasurementText();
    void AddAllInteractors();
    void RemoveAllInteractors();
    mitk::DataNode::Pointer DetectTopMostVisibleImage();
    void EnableCrosshairNavigation();
    void DisableCrosshairNavigation();
    void PlanarFigureInitialized();
    void CheckForTopMostVisibleImage(mitk::DataNode* _NodeToNeglect=nullptr);
    mitk::DataStorage::SetOfObjects::ConstPointer GetAllPlanarFigures() const;

    QmitkMeasurementViewData* d;

    // holds configuration objects that have been deactivated
    std::map<us::ServiceReferenceU, mitk::EventConfig> m_DisplayInteractorConfigs;
};

#endif // QMITK_MEASUREMENT_H__INCLUDED
