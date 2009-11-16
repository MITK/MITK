/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 15412 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>
#include <cherryPlatform.h>

#include "mitkGlobalInteraction.h"
#include "mitkPointSet.h"
#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkIDataStorageService.h"
#include "mitkDataTreeNodeObject.h"
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNOT.h>
#include <mitkNodePredicateAND.h>
#include <mitkDataTreeNodeSelection.h>

#include "mitkPlanarCircle.h"
#include "mitkPlanarPolygon.h"
#include "mitkPlanarAngle.h"
#include "mitkPlanarRectangle.h"
#include "mitkPlanarLine.h"
#include "mitkPlanarFourPointAngle.h"
#include "mitkPlanarFigureInteractor.h"
#include <mitkPlaneGeometry.h>
#include "QmitkPlanarFiguresTableModel.h"

#include "QmitkMeasurement.h"
#include <QmitkRenderWindow.h>

#include <QGridLayout>
#include <QMainWindow>
#include <QToolBar>
#include <QLabel>
#include <QTableView>
#include <QClipboard>
#include <QTextBrowser>

#include <QmitkDataStorageTableModel.h>
#include "mitkNodePredicateDataType.h"
#include "mitkPlanarFigure.h"

#include <vtkTextProperty.h>
#include <vtkRenderer.h>
#include <vtkCornerAnnotation.h>
#include <mitkVtkLayerController.h>

QmitkMeasurement::QmitkMeasurement() :
  m_Layout(0), m_DrawActionsToolBar(0), m_MeasurementInfoRenderer(0),
      m_MeasurementInfoAnnotation(0), m_LineCounter(0), m_PathCounter(0),
      m_AngleCounter(0), m_FourPointAngleCounter(0), m_EllipseCounter(0),
      m_RectangleCounter(0), m_PolygonCounter(0)
{

}

QmitkMeasurement::~QmitkMeasurement()
{

  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(
      m_SelectionListener);
  m_MeasurementInfoRenderer->Delete();
}

void QmitkMeasurement::CreateQtPartControl(QWidget* parent)
{
  m_MeasurementInfoRenderer = vtkRenderer::New();
  m_MeasurementInfoAnnotation = vtkCornerAnnotation::New();
  vtkTextProperty *textProp = vtkTextProperty::New();

  m_MeasurementInfoAnnotation->SetMaximumFontSize(12);
  textProp->SetColor(1.0, 1.0, 1.0);
  m_MeasurementInfoAnnotation->SetTextProperty(textProp);

  m_MeasurementInfoRenderer->AddActor(m_MeasurementInfoAnnotation);
  m_DrawActionsToolBar = new QToolBar;

  //# add actions
  QAction* currentAction = m_DrawActionsToolBar->addAction(QIcon(
      ":/measurement/line.png"), "Draw Line");
  m_DrawLine = currentAction;
  m_DrawLine->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
      , this, SLOT( ActionDrawLineTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(
      ":/measurement/path.png"), "Draw Path");
  m_DrawPath = currentAction;
  m_DrawPath->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
      , this, SLOT( ActionDrawPathTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(
      ":/measurement/angle.png"), "Draw Angle");
  m_DrawAngle = currentAction;
  m_DrawAngle->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
      , this, SLOT( ActionDrawAngleTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(
      ":/measurement/four-point-angle.png"), "Draw Four Point Angle");
  m_DrawFourPointAngle = currentAction;
  m_DrawFourPointAngle->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
      , this, SLOT( ActionDrawFourPointAngleTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(
      ":/measurement/circle.png"), "Draw Ellipse");
  m_DrawEllipse = currentAction;
  m_DrawEllipse->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
      , this, SLOT( ActionDrawEllipseTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(
      ":/measurement/rectangle.png"), "Draw Rectangle");
  m_DrawRectangle = currentAction;
  m_DrawRectangle->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
      , this, SLOT( ActionDrawRectangleTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(
      ":/measurement/polygon.png"), "Draw Polygon");
  m_DrawPolygon = currentAction;
  m_DrawPolygon->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
      , this, SLOT( ActionDrawPolygonTriggered(bool) ) );

  QLabel* selectedImageLabel = new QLabel("Selected Image: ");
  m_SelectedImage = new QLabel;
  m_SelectedPlanarFiguresText = new QTextBrowser;

  m_CopyToClipboard = new QPushButton("Copy to Clipboard");
  QObject::connect( m_CopyToClipboard, SIGNAL( clicked(bool) )
      , this, SLOT( CopyToClipboard(bool) ) );

  m_Layout = new QGridLayout;
  m_Layout->addWidget(selectedImageLabel, 0, 0, 1, 1);
  m_Layout->addWidget(m_SelectedImage, 0, 1, 1, 1);
  m_Layout->addWidget(m_DrawActionsToolBar, 1, 0, 1, 2);
  m_Layout->addWidget(m_SelectedPlanarFiguresText, 2, 0, 1, 2);
  m_Layout->addWidget(m_CopyToClipboard, 3, 0, 1, 2);
  m_Layout->setRowStretch(0, 1);
  m_Layout->setRowStretch(1, 1);
  m_Layout->setRowStretch(2, 10);
  m_Layout->setRowStretch(3, 1);
  m_Layout->setContentsMargins(2, 2, 2, 2);

  parent->setLayout(m_Layout);

  // Initialize selection listener mechanism
  m_SelectionListener = cherry::ISelectionListener::Pointer(
      new cherry::SelectionChangedAdapter<QmitkMeasurement>(this,
          &QmitkMeasurement::SelectionChanged));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(
      m_SelectionListener);
  cherry::ISelection::ConstPointer
      selection(
          this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection(
              "org.mitk.views.datamanager"));
  if (selection.IsNotNull())
    this->SelectionChanged(cherry::IWorkbenchPart::Pointer(0), selection);

  m_SelectionProvider = new mitk::MeasurementSelectionProvider;
  this->GetSite()->SetSelectionProvider(m_SelectionProvider);

}

void QmitkMeasurement::SelectionChanged(
    cherry::IWorkbenchPart::Pointer sourcepart,
    cherry::ISelection::ConstPointer selection)
{
  //if (sourcepart.GetPointer() == this)
    //return;

  mitk::DataTreeNodeSelection::ConstPointer _DataTreeNodeSelection =
      selection.Cast<const mitk::DataTreeNodeSelection> ();
  if (_DataTreeNodeSelection.IsNull())
    return;

  m_SelectedPlanarFigures.clear();
  m_SelectedImageNode = 0;

  mitk::DataTreeNodeObject* _DataTreeNodeObject = 0;
  mitk::DataTreeNode* _DataTreeNode = 0;
  mitk::BaseData* _BaseData;
  mitk::PlanarFigure* _PlanarFigure;

  for (mitk::DataTreeNodeSelection::iterator it =
      _DataTreeNodeSelection->Begin(); it != _DataTreeNodeSelection->End(); ++it)
  {
    _PlanarFigure = 0;

    _DataTreeNodeObject
        = dynamic_cast<mitk::DataTreeNodeObject*> ((*it).GetPointer());
    if (!_DataTreeNodeObject)
      continue;

    _DataTreeNode = _DataTreeNodeObject->GetDataTreeNode();

    if (!_DataTreeNode)
      continue;

    _BaseData = _DataTreeNode->GetData();

    if (!_BaseData)
      continue;

    // planar figure selected
    if ((_PlanarFigure = dynamic_cast<mitk::PlanarFigure *> (_BaseData)))
    {
      // add to the selected planar figures
      m_SelectedPlanarFigures.push_back(_DataTreeNode);
      // take parent image as the selected image
      mitk::DataStorage::SetOfObjects::ConstPointer parents =
          this->GetDataStorage()->GetSources(_DataTreeNode);
      if (parents->size() > 0)
      {
        mitk::DataTreeNode::Pointer parent = parents->front();
        if (dynamic_cast<mitk::Image *> (parent->GetData()))
        {
          if (m_SelectedImageNode != parent)
            m_SelectedImageNode = parent;
        }
      }

    } else if (m_SelectedImageNode != _DataTreeNode
        && dynamic_cast<mitk::Image *> (_BaseData))
    {
      m_SelectedImageNode = _DataTreeNode;
    }
  } // end for

  if (m_SelectedImageNode.IsNotNull())
  {
    m_SelectedImage->setText(QString::fromStdString(
        m_SelectedImageNode->GetName()));
    m_SelectedImage->setStyleSheet("font-weight: bold;");
    m_DrawActionsToolBar->setEnabled(true);
  } else
  {
    m_SelectedImage->setText(
        "None selected. Please select an image.");
    m_SelectedImage->setStyleSheet("font-weight: bold;");

    m_DrawActionsToolBar->setEnabled(false);
  }

  this->PlanarFigureSelectionChanged();
}

void QmitkMeasurement::NewNodeInserted(mitk::DataTreeNode::Pointer node)
{
  m_SelectionProvider->FireSelectionChanged(node);
}

void QmitkMeasurement::ActionDrawLineTriggered(bool checked)
{
  if (!checked)
    return;

  if (m_SelectedImageNode.IsNull())
  {
    this->HandleException("Please select an image!", this->m_Parent, true);
    m_DrawLine->setChecked(false);
    return;
  }

  mitk::PlanarLine::Pointer figure = mitk::PlanarLine::New();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Line%1").arg(++m_LineCounter).toStdString());
  figureNode->SetData(figure);

  this->GetDataStorage()->Add(figureNode, m_SelectedImageNode);

  mitk::PlanarFigureInteractor::Pointer interactor =
      mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", figureNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

  LOG_INFO << "PlanarLine initialized...";
  m_DrawLine->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->NewNodeInserted(figureNode);
}

void QmitkMeasurement::ActionDrawPathTriggered(bool checked)
{
  if (!checked)
    return;

  if (m_SelectedImageNode.IsNull())
  {
    this->HandleException("Please select an image!", this->m_Parent, true);
    m_DrawPath->setChecked(false);
    return;
  }

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOff();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Path%1").arg(++m_PathCounter).toStdString());
  figureNode->SetData(figure);

  this->GetDataStorage()->Add(figureNode, m_SelectedImageNode);

  mitk::PlanarFigureInteractor::Pointer interactor =
      mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", figureNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

  LOG_INFO << "PlanarPath initialized...";
  m_DrawPath->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->NewNodeInserted(figureNode);
}

void QmitkMeasurement::ActionDrawAngleTriggered(bool checked)
{
  if (!checked)
    return;

  if (m_SelectedImageNode.IsNull())
  {
    this->HandleException("Please select an image!", this->m_Parent, true);
    m_DrawAngle->setChecked(false);
    return;
  }

  mitk::PlanarAngle::Pointer figure = mitk::PlanarAngle::New();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Angle%1").arg(++m_AngleCounter).toStdString());
  figureNode->SetData(figure);

  this->GetDataStorage()->Add(figureNode, m_SelectedImageNode);

  mitk::PlanarFigureInteractor::Pointer interactor =
      mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", figureNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

  LOG_INFO << "PlanarAngle initialized...";
  m_DrawAngle->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->NewNodeInserted(figureNode);
}

void QmitkMeasurement::ActionDrawFourPointAngleTriggered(bool checked)
{
  if (!checked)
    return;

  if (m_SelectedImageNode.IsNull())
  {
    this->HandleException("Please select an image!", this->m_Parent, true);
    m_DrawFourPointAngle->setChecked(false);
    return;
  }
  mitk::PlanarFourPointAngle::Pointer figure =
      mitk::PlanarFourPointAngle::New();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Four Point Angle%1").arg(
      ++m_FourPointAngleCounter).toStdString());

  figureNode->SetData(figure);

  this->GetDataStorage()->Add(figureNode, m_SelectedImageNode);

  mitk::PlanarFigureInteractor::Pointer interactor =
      mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", figureNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

  LOG_INFO << "PlanarFourPointAngle initialized...";
  m_DrawFourPointAngle->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->NewNodeInserted(figureNode);
}

void QmitkMeasurement::ActionDrawEllipseTriggered(bool checked)
{
  if (!checked)
    return;

  if (m_SelectedImageNode.IsNull())
  {
    this->HandleException("Please select an image!", this->m_Parent, true);
    m_DrawEllipse->setChecked(false);
    return;
  }

  mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(
      QString("Ellipse%1").arg(++m_EllipseCounter).toStdString());
  figureNode->SetData(figure);

  this->GetDataStorage()->Add(figureNode, m_SelectedImageNode);

  mitk::PlanarFigureInteractor::Pointer interactor =
      mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", figureNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

  LOG_INFO << "PlanarCircle initialized...";
  m_DrawEllipse->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->NewNodeInserted(figureNode);
}

void QmitkMeasurement::ActionDrawRectangleTriggered(bool checked)
{
  if (!checked)
    return;

  if (m_SelectedImageNode.IsNull())
  {
    this->HandleException("Please select an image!", this->m_Parent, true);
    m_DrawRectangle->setChecked(false);
    return;
  }

  mitk::PlanarRectangle::Pointer figure = mitk::PlanarRectangle::New();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(
      QString("Rectangle%1").arg(++m_RectangleCounter).toStdString());
  figureNode->SetData(figure);

  this->GetDataStorage()->Add(figureNode, m_SelectedImageNode);

  mitk::PlanarFigureInteractor::Pointer interactor =
      mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", figureNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

  LOG_INFO << "PlanarRectangle initialized...";
  m_DrawRectangle->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->NewNodeInserted(figureNode);
}

void QmitkMeasurement::ActionDrawPolygonTriggered(bool checked)
{
  if (!checked)
    return;

  if (m_SelectedImageNode.IsNull())
  {
    this->HandleException("Please select an image!", this->m_Parent, true);
    m_DrawPolygon->setChecked(false);
    return;
  }

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOn();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(
      QString("Polygon%1").arg(++m_PolygonCounter).toStdString());
  figureNode->SetData(figure);

  this->GetDataStorage()->Add(figureNode, m_SelectedImageNode);

  mitk::PlanarFigureInteractor::Pointer interactor =
      mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", figureNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

  LOG_INFO << "PlanarPolygon initialized...";
  m_DrawPolygon->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->NewNodeInserted(figureNode);
}

void QmitkMeasurement::ActionDrawArrowTriggered(bool checked)
{
  if (!checked)
    return;

  if (m_SelectedImageNode.IsNull())
  {
    this->HandleException("Please select an image!", this->m_Parent, true);
    return;
  }

  LOG_WARN << "Draw Arrow not implemented yet.";
}

void QmitkMeasurement::ActionDrawTextTriggered(bool checked)
{
  if (!checked)
    return;

  if (m_SelectedImageNode.IsNull())
  {
    this->HandleException("Please select an image!", this->m_Parent, true);
    return;
  }

  LOG_WARN << "Draw Text not implemented yet.";
}

void QmitkMeasurement::Visible()
{
  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(false);
  this->GetActiveStdMultiWidget()->changeLayoutToWidget1();
}

void QmitkMeasurement::Hidden()
{
  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(true);
  this->GetActiveStdMultiWidget()->changeLayoutToDefault();
  this->setMeasurementInfo("", 0);
}

void QmitkMeasurement::CopyToClipboard(bool)
{
  std::vector<QString> headerRow;
  std::vector<std::vector<QString> > rows;
  QString featureName;
  QString featureQuantity;
  std::vector<QString> newRow;
  headerRow.push_back("Name");

  for (DataTreeNodes::iterator it = m_SelectedPlanarFigures.begin(); it
      != m_SelectedPlanarFigures.end(); ++it)
  {
    mitk::PlanarFigure* planarFigure =
        dynamic_cast<mitk::PlanarFigure*> ((*it)->GetData());
    if (!planarFigure)
      continue;

    newRow.clear();
    newRow.push_back(QString::fromStdString((*it)->GetName()));
    newRow.resize(headerRow.size());
    for (unsigned int i = 0; i < planarFigure->GetNumberOfFeatures(); ++i)
    {
      featureName = planarFigure->GetFeatureName(i);
      featureName.append(QString(" [%1]").arg(planarFigure->GetFeatureUnit(i)));
      std::vector<QString>::iterator itColumn = std::find(headerRow.begin(),
          headerRow.end(), featureName);

      featureQuantity
          = QString("%1").arg(planarFigure->GetQuantity(i)).replace(QChar('.'),
              ",");
      if (itColumn == headerRow.end())
      {
        headerRow.push_back(featureName);
        newRow.push_back(featureQuantity);
      } else
      {
        newRow[std::distance(headerRow.begin(), itColumn)] = featureQuantity;
      }

    }
    rows.push_back(newRow);
  }

  QString clipboardText;
  for (std::vector<QString>::iterator it = headerRow.begin(); it
      != headerRow.end(); ++it)
    clipboardText.append(QString("%1 \t").arg(*it));

  for (std::vector<std::vector<QString> >::iterator it = rows.begin(); it
      != rows.end(); ++it)
  {
    clipboardText.append("\n");
    for (std::vector<QString>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
    {
      clipboardText.append(QString("%1 \t").arg(*it2));
    }
  }

  QApplication::clipboard()->setText(clipboardText, QClipboard::Clipboard);

}

void QmitkMeasurement::PlanarFigureSelectionChanged()
{
  if (m_SelectedPlanarFigures.size() == 0)
    this->setMeasurementInfo("", 0);

  unsigned int j = 1;
  mitk::PlanarFigure* _PlanarFigure = 0;
  mitk::PlanarAngle* planarAngle = 0;
  mitk::PlanarFourPointAngle* planarFourPointAngle = 0;
  mitk::DataTreeNode::Pointer node = 0;
  m_SelectedPlanarFiguresText->clear();
  QString infoText;
  QString plainInfoText;

  for (DataTreeNodes::iterator it = m_SelectedPlanarFigures.begin(); it
      != m_SelectedPlanarFigures.end(); ++it, ++j)
  {
    plainInfoText.clear();
    node = *it;
    if(j>1)
      infoText.prepend("<br />");

    infoText.append(QString("<b>%1</b><hr />").arg(QString::fromStdString(
        node->GetName())));
    plainInfoText.append(QString("%1").arg(QString::fromStdString(
        node->GetName())));

    planarAngle = dynamic_cast<mitk::PlanarAngle*> (_PlanarFigure);
    if(!planarAngle)
    {
      planarFourPointAngle = dynamic_cast<mitk::PlanarFourPointAngle*> (_PlanarFigure);
      if(planarFourPointAngle)
        _PlanarFigure = planarFourPointAngle;
    }
    else
      _PlanarFigure = planarAngle;

    if(!planarAngle || !planarFourPointAngle)
      _PlanarFigure = dynamic_cast<mitk::PlanarFigure*> (node->GetData());

    if(!_PlanarFigure)
      continue;

    double featureQuantity;
    for (unsigned int i = 0; i < _PlanarFigure->GetNumberOfFeatures(); ++i)
    {
      featureQuantity = _PlanarFigure->GetQuantity(i);
      if ((planarAngle && i == planarAngle->FEATURE_ID_ANGLE)
          || (planarFourPointAngle && i == planarFourPointAngle->FEATURE_ID_ANGLE))
        featureQuantity = featureQuantity * 180 / vnl_math::pi;

      infoText.append(
          QString("<i>%1</i>: %2 %3") .arg(QString(
              _PlanarFigure->GetFeatureName(i))) .arg(featureQuantity, 0, 'f',
              2) .arg(QString(_PlanarFigure->GetFeatureUnit(i))));

      plainInfoText.append(
          QString("\n%1: %2 %3") .arg(QString(_PlanarFigure->GetFeatureName(i))) .arg(
              featureQuantity, 0, 'f', 2) .arg(QString(
              _PlanarFigure->GetFeatureUnit(i))));

      if(i+1 != _PlanarFigure->GetNumberOfFeatures())
        infoText.append("<br />");
    }

    if (j != m_SelectedPlanarFigures.size())
      infoText.append("<br />");
  }

  m_SelectedPlanarFiguresText->setHtml(infoText);

  // for the last selected planar figure ...
  if (_PlanarFigure)
  {
    const mitk::PlaneGeometry
        * _PlaneGeometry =
            dynamic_cast<const mitk::PlaneGeometry*> (_PlanarFigure->GetGeometry2D());

    QmitkRenderWindow* selectedRenderWindow = 0;
    QmitkRenderWindow* RenderWindow1 =
        this->GetActiveStdMultiWidget()->GetRenderWindow1();
    QmitkRenderWindow* RenderWindow2 =
        this->GetActiveStdMultiWidget()->GetRenderWindow2();
    QmitkRenderWindow* RenderWindow3 =
        this->GetActiveStdMultiWidget()->GetRenderWindow3();
    QmitkRenderWindow* RenderWindow4 =
        this->GetActiveStdMultiWidget()->GetRenderWindow4();
    bool PlanarFigureInitializedWindow = false;

    // find initialized renderwindow
    if (node->GetBoolProperty("PlanarFigureInitializedWindow",
        PlanarFigureInitializedWindow, RenderWindow1->GetRenderer()))
      selectedRenderWindow = RenderWindow1;
    if (!selectedRenderWindow && node->GetBoolProperty(
        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
        RenderWindow2->GetRenderer()))
      selectedRenderWindow = RenderWindow2;
    if (!selectedRenderWindow && node->GetBoolProperty(
        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
        RenderWindow3->GetRenderer()))
      selectedRenderWindow = RenderWindow3;
    if (!selectedRenderWindow && node->GetBoolProperty(
        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
        RenderWindow4->GetRenderer()))
      selectedRenderWindow = RenderWindow4;

    // make node visible
    if (selectedRenderWindow)
    {
      mitk::Point3D centerP = _PlaneGeometry->GetOrigin();
      selectedRenderWindow->GetSliceNavigationController()->ReorientSlices(
          centerP, _PlaneGeometry->GetNormal());
      selectedRenderWindow->GetSliceNavigationController()->SelectSliceByPoint(
          centerP);
    }

    // now paint infos also on renderwindow
    this->setMeasurementInfo(plainInfoText, selectedRenderWindow);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMeasurement::setMeasurementInfo(const QString& text,
    QmitkRenderWindow* _RenderWindow)
{
  static QmitkRenderWindow* lastRenderWindow = _RenderWindow;
  if (!text.isEmpty())
  {
    m_MeasurementInfoAnnotation->SetText(1, text.toLatin1().data());
    mitk::VtkLayerController::GetInstance(_RenderWindow->GetRenderWindow())->InsertForegroundRenderer(
        m_MeasurementInfoRenderer, true);
  } else
  {
    if (_RenderWindow != 0 && mitk::VtkLayerController::GetInstance(
        lastRenderWindow->GetRenderWindow()) ->IsRendererInserted(
        m_MeasurementInfoRenderer))
      mitk::VtkLayerController::GetInstance(lastRenderWindow->GetRenderWindow())->RemoveRenderer(
          m_MeasurementInfoRenderer);
  }
}
