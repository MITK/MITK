/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMeasurementView_h
#define QmitkMeasurementView_h

#include <QmitkAbstractView.h>

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
    ~QmitkMeasurementView() override;

    void CreateQtPartControl(QWidget* parent) override;
    void SetFocus() override { };

    void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

    void NodeAdded(const mitk::DataNode* node) override;
    /**
    * @brief  Determine if we have to update the information of the currently
    *         selected planar figures.
    */
    void NodeChanged(const mitk::DataNode* node) override;
    void NodeRemoved(const mitk::DataNode* node) override;

    void PlanarFigureSelected( itk::Object* object, const itk::EventObject& );

protected Q_SLOTS:

    void OnDrawLineTriggered( bool checked = false );
    void OnDrawPathTriggered( bool checked = false );
    void OnDrawAngleTriggered( bool checked = false );
    void OnDrawFourPointAngleTriggered( bool checked = false );
    void OnDrawCircleTriggered( bool checked = false );
    void OnDrawEllipseTriggered( bool checked = false );
    void OnDrawDoubleEllipseTriggered( bool checked = false );
    void OnDrawRectangleTriggered( bool checked = false );
    void OnDrawPolygonTriggered( bool checked = false );
    void OnDrawBezierCurveTriggered( bool checked = false );
    void OnDrawSubdivisionPolygonTriggered( bool checked = false );
    void OnCopyToClipboard( bool checked = false );

private:

    void CreateConnections();
    mitk::DataNode::Pointer AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name);

    void OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

    void UpdateMeasurementText();
    void AddAllInteractors();
    void PlanarFigureInitialized();
    mitk::DataStorage::SetOfObjects::ConstPointer GetAllPlanarFigures() const;

    QmitkMeasurementViewData* d;
};

#endif
