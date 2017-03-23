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
#include "QmitkCommentTextView.h"

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

    void OnDrawCommentTriggered(bool checked = false);
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

    void CommentDoneTriggered( bool checked = false );
    void CommentCancelTriggered( bool checked = false );

  private:

    const static QString TR_REF_IMAGE;
    const static QString TR_CLIPBOARD_COPY;
    const static QString TR_NO_AVAIBLE_IMAGE;
    const static QString TR_COMMENT_DONE;
    const static QString TR_COMMENT_CANCEL;
    const static QString TR_COMMENT_TEXT;

    void CreateConnections();
    mitk::DataNode::Pointer AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name);
    void UpdateMeasurementText();
    void AddAllInteractors();
    mitk::DataNode::Pointer DetectTopMostVisibleImage();
    void EnableCrosshairNavigation();
    void DisableCrosshairNavigation();
    void PlanarFigureInitialized();

    void CheckForTopMostVisibleImage(mitk::DataNode* nodeToNeglect = nullptr);

    void AddInitialPoint();

    mitk::DataStorage::SetOfObjects::ConstPointer GetAllPlanarFigures() const;

    void hideCommentTextView();
    void showCommentTextView(const std::string& text = std::string());

    // holds configuration objects that have been deactivated
    std::map<us::ServiceReferenceU, mitk::EventConfig> m_DisplayInteractorConfigs;

    QmitkCommentTextView m_commentTextView;

    QmitkMeasurementViewData* d;
};

#endif // QMITK_MEASUREMENT_H__INCLUDED
