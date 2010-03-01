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

#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryPlatform.h>

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
      m_MeasurementInfoAnnotation(0), m_SelectedPlanarFigures(0),
      m_SelectedImageNode(),
      m_LineCounter(0), m_PathCounter(0),
      m_AngleCounter(0), m_FourPointAngleCounter(0), m_EllipseCounter(0),
      m_RectangleCounter(0), m_PolygonCounter(0)
{

}

QmitkMeasurement::~QmitkMeasurement()
{

  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(
      m_SelectionListener);
  m_MeasurementInfoRenderer->Delete();

  this->GetDefaultDataStorage()->AddNodeEvent -= mitk::MessageDelegate1<QmitkMeasurement
    , const mitk::DataTreeNode*>( this, &QmitkMeasurement::NodeAddedInDataStorage );

  m_SelectedPlanarFigures->NodeChanged.RemoveListener( mitk::MessageDelegate1<QmitkMeasurement
    , const mitk::DataTreeNode*>( this, &QmitkMeasurement::NodeChanged ) );

  m_SelectedPlanarFigures->NodeRemoved.RemoveListener( mitk::MessageDelegate1<QmitkMeasurement
    , const mitk::DataTreeNode*>( this, &QmitkMeasurement::NodeRemoved ) );

  m_SelectedPlanarFigures->PropertyChanged.RemoveListener( mitk::MessageDelegate2<QmitkMeasurement
    , const mitk::DataTreeNode*, const mitk::BaseProperty*>( this, &QmitkMeasurement::PropertyChanged ) );

  m_SelectedImageNode->NodeChanged.RemoveListener( mitk::MessageDelegate1<QmitkMeasurement
    , const mitk::DataTreeNode*>( this, &QmitkMeasurement::NodeChanged ) );

  m_SelectedImageNode->NodeRemoved.RemoveListener( mitk::MessageDelegate1<QmitkMeasurement
    , const mitk::DataTreeNode*>( this, &QmitkMeasurement::NodeRemoved ) );

  m_SelectedImageNode->PropertyChanged.RemoveListener( mitk::MessageDelegate2<QmitkMeasurement
    , const mitk::DataTreeNode*, const mitk::BaseProperty*>( this, &QmitkMeasurement::PropertyChanged ) );
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
      ":/measurement/circle.png"), "Draw Circle");
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
  m_SelectedImage->setStyleSheet("font-weight: bold;");
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


  m_SelectedPlanarFigures = mitk::DataStorageSelection::New(this->GetDefaultDataStorage(), false);

  m_SelectedPlanarFigures->NodeChanged.AddListener( mitk::MessageDelegate1<QmitkMeasurement
    , const mitk::DataTreeNode*>( this, &QmitkMeasurement::NodeChanged ) );

  m_SelectedPlanarFigures->NodeRemoved.AddListener( mitk::MessageDelegate1<QmitkMeasurement
    , const mitk::DataTreeNode*>( this, &QmitkMeasurement::NodeRemoved ) );

  m_SelectedPlanarFigures->PropertyChanged.AddListener( mitk::MessageDelegate2<QmitkMeasurement
    , const mitk::DataTreeNode*, const mitk::BaseProperty*>( this, &QmitkMeasurement::PropertyChanged ) );

  m_SelectedImageNode = mitk::DataStorageSelection::New(this->GetDefaultDataStorage(), false);

  m_SelectedImageNode->PropertyChanged.AddListener( mitk::MessageDelegate2<QmitkMeasurement
    , const mitk::DataTreeNode*, const mitk::BaseProperty*>( this, &QmitkMeasurement::PropertyChanged ) );

  m_SelectedImageNode->NodeChanged.AddListener( mitk::MessageDelegate1<QmitkMeasurement
    , const mitk::DataTreeNode*>( this, &QmitkMeasurement::NodeChanged ) );

  m_SelectedImageNode->NodeRemoved.AddListener( mitk::MessageDelegate1<QmitkMeasurement
    , const mitk::DataTreeNode*>( this, &QmitkMeasurement::NodeRemoved ) );

  // Initialize selection listener mechanism
  m_SelectionListener = berry::ISelectionListener::Pointer(
      new berry::SelectionChangedAdapter<QmitkMeasurement>(this,
          &QmitkMeasurement::SelectionChanged));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(
      m_SelectionListener);
  berry::ISelection::ConstPointer
      selection(
          this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection(
              "org.mitk.views.datamanager"));
  if (selection.IsNotNull())
    this->SelectionChanged(berry::IWorkbenchPart::Pointer(0), selection);

  m_SelectionProvider = new mitk::MeasurementSelectionProvider;

  this->GetSite()->SetSelectionProvider(m_SelectionProvider);
  this->GetDefaultDataStorage()->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkMeasurement
    , const mitk::DataTreeNode*>( this, &QmitkMeasurement::NodeAddedInDataStorage ) );

}

void QmitkMeasurement::SelectionChanged(
    berry::IWorkbenchPart::Pointer sourcepart,
    berry::ISelection::ConstPointer selection)
{
  if ( sourcepart.GetPointer() == this)
    return;

  mitk::DataTreeNodeSelection::ConstPointer _DataTreeNodeSelection =
      selection.Cast<const mitk::DataTreeNodeSelection> ();
  if (_DataTreeNodeSelection.IsNull())
    return;

  m_SelectedImageNode->RemoveAllNodes();

  mitk::DataTreeNodeObject* _DataTreeNodeObject = 0;
  mitk::DataTreeNode* _DataTreeNode = 0;
  mitk::BaseData* _BaseData;
  mitk::PlanarFigure* _PlanarFigure;
  mitk::Image* selectedImage;
  m_SelectedPlanarFigures->RemoveAllNodes();

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
      m_SelectedPlanarFigures->AddNode(_DataTreeNode);
      // take parent image as the selected image
      mitk::DataStorage::SetOfObjects::ConstPointer parents =
          this->GetDefaultDataStorage()->GetSources(_DataTreeNode);
      if (parents->size() > 0)
      {
        mitk::DataTreeNode::Pointer parent = parents->front();
        if ((selectedImage = dynamic_cast<mitk::Image *> (parent->GetData())))
        {
          *m_SelectedImageNode = parent;
        }
      }

    }
    else if ((selectedImage = dynamic_cast<mitk::Image *> (_BaseData)))
    {
      *m_SelectedImageNode = _DataTreeNode;
      /*mitk::RenderingManager::GetInstance()->InitializeViews(
        selectedImage->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );*/
    }
  } // end for

  this->PlanarFigureSelectionChanged();
}

void QmitkMeasurement::PlanarFigureSelectionChanged()
{
  if (m_SelectedImageNode->GetNode().IsNotNull())
  {
    mitk::Image* selectedImage = dynamic_cast<mitk::Image*>(m_SelectedImageNode->GetNode()->GetData());
    if(selectedImage && selectedImage->GetDimension() > 3)
    {
      m_SelectedImageNode->RemoveAllNodes();
      m_SelectedImage->setText( "4D images are not supported." );
      m_DrawActionsToolBar->setEnabled(false);
    }
    else
    {
      m_SelectedImage->setText(QString::fromStdString(
          m_SelectedImageNode->GetNode()->GetName()));
      m_DrawActionsToolBar->setEnabled(true);
    }
  }
  else
  {
    m_SelectedImage->setText(
        "None. Please select an image.");

    m_DrawActionsToolBar->setEnabled(false);
  }

  if (m_SelectedPlanarFigures->GetSize() == 0)
    this->SetMeasurementInfoToRenderWindow("", 0);


  unsigned int j = 1;
  mitk::PlanarFigure* _PlanarFigure = 0;
  mitk::PlanarAngle* planarAngle = 0;
  mitk::PlanarFourPointAngle* planarFourPointAngle = 0;
  mitk::DataTreeNode::Pointer node = 0;
  m_SelectedPlanarFiguresText->clear();
  QString infoText;
  QString plainInfoText;
  std::vector<mitk::DataTreeNode*> nodes = m_SelectedPlanarFigures->GetNodes();

  for (std::vector<mitk::DataTreeNode*>::iterator it = nodes.begin(); it
      != nodes.end(); ++it, ++j)
  {
    plainInfoText.clear();
    node = *it;
    if(j>1)
      infoText.append("<br />");

    infoText.append(QString("<b>%1</b><hr />").arg(QString::fromStdString(
        node->GetName())));
    plainInfoText.append(QString("%1").arg(QString::fromStdString(
        node->GetName())));

    _PlanarFigure = dynamic_cast<mitk::PlanarFigure*> (node->GetData());

    planarAngle = dynamic_cast<mitk::PlanarAngle*> (_PlanarFigure);
    if(!planarAngle)
    {
      planarFourPointAngle = dynamic_cast<mitk::PlanarFourPointAngle*> (_PlanarFigure);
    }

    if(!_PlanarFigure)
      continue;

    double featureQuantity = 0.0;
    for (unsigned int i = 0; i < _PlanarFigure->GetNumberOfFeatures(); ++i)
    {
      if ( !_PlanarFigure->IsFeatureActive( i ) ) continue;

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

    if (j != nodes.size())
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
      //selectedRenderWindow->GetSliceNavigationController()->ReorientSlices(
          //centerP, _PlaneGeometry->GetNormal());
      selectedRenderWindow->GetSliceNavigationController()->SelectSliceByPoint(
          centerP);

      // now paint infos also on renderwindow
      this->SetMeasurementInfoToRenderWindow(plainInfoText, selectedRenderWindow);
    }
  }
  // no last planarfigure
  else
    this->SetMeasurementInfoToRenderWindow("", 0);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMeasurement::NodeAddedInDataStorage(const mitk::DataTreeNode* node)
{
  if(!m_Visible)
    return;
  mitk::DataTreeNode* nonConstNode = const_cast<mitk::DataTreeNode*>(node);
  mitk::PlanarFigure* figure = dynamic_cast<mitk::PlanarFigure*>(nonConstNode->GetData());
  if(figure)
  {
    mitk::PlanarFigureInteractor::Pointer figureInteractor
      = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());

    if(figureInteractor.IsNull())
      figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", nonConstNode);

    mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);
  }
}

void QmitkMeasurement::PlanarFigureInitialized()
{
  if(m_CurrentFigureNode.IsNull())
    return;

  this->PlanarFigureSelectionChanged();

  m_DrawLine->setChecked(false);
  m_DrawPath->setChecked(false);
  m_DrawAngle->setChecked(false);
  m_DrawFourPointAngle->setChecked(false);
  m_DrawEllipse->setChecked(false);
  m_DrawRectangle->setChecked(false);
  m_DrawPolygon->setChecked(false);
}

void QmitkMeasurement::AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name,
  const char *propertyKey, mitk::BaseProperty *property )
{
  if(m_CurrentFigureNode.IsNotNull())
    m_CurrentFigureNode->GetData()->RemoveObserver(m_InitializedObserverTag);

  mitk::DataTreeNode::Pointer newNode = mitk::DataTreeNode::New();
  m_CurrentFigureNode = newNode;
  m_CurrentFigureNode->SetName(name.toStdString());
  m_CurrentFigureNode->SetData(figure);

  // Add custom property, if available
  if ( (propertyKey != NULL) && (property != NULL) )
  {
    m_CurrentFigureNode->AddProperty( propertyKey, property );
  }

  typedef itk::SimpleMemberCommand< QmitkMeasurement > ITKCommandType;
  ITKCommandType::Pointer initializationCommand;
  initializationCommand = ITKCommandType::New();

  // set the callback function of the member command
  initializationCommand->SetCallbackFunction( this, &QmitkMeasurement::PlanarFigureInitialized );

  // add an observer
  m_InitializedObserverTag = figure->AddObserver( itk::InitializeEvent(), initializationCommand );

  this->GetDataStorage()->Add(newNode, m_SelectedImageNode->GetNode());

  *m_SelectedPlanarFigures = m_CurrentFigureNode;
  //m_SelectionProvider->FireSelectionChanged(
      //mitk::DataTreeNodeSelection::Pointer(new mitk::DataTreeNodeSelection(m_CurrentFigureNode.GetPointer())));

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

bool QmitkMeasurement::AssertDrawingIsPossible(bool checked)
{
  if (m_SelectedImageNode->GetNode().IsNull())
  {
    checked = false;
    this->HandleException("Please select an image!", this->m_Parent, true);
    m_DrawLine->setChecked(false);
    return false;
  }

  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(false);
  //this->GetActiveStdMultiWidget()->GetRenderWindow1()->FullScreenMode(true);

  return checked;
}

void QmitkMeasurement::ActionDrawLineTriggered(bool checked)
{
  if(!this->AssertDrawingIsPossible(checked))
    return;
  mitk::PlanarLine::Pointer figure = mitk::PlanarLine::New();
  this->AddFigureToDataStorage(figure, QString("Line%1").arg(++m_LineCounter));

  MITK_INFO << "PlanarLine initialized...";
}

void QmitkMeasurement::ActionDrawPathTriggered(bool checked)
{
  if(!this->AssertDrawingIsPossible(checked))
    return;

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOff();

  mitk::BoolProperty::Pointer closedProperty = mitk::BoolProperty::New( false );
  this->AddFigureToDataStorage(figure, QString("Path%1").arg(++m_PathCounter),
    "ClosedPlanarPolygon", closedProperty);

  MITK_INFO << "PlanarPath initialized...";
}

void QmitkMeasurement::ActionDrawAngleTriggered(bool checked)
{
  if(!this->AssertDrawingIsPossible(checked))
    return;

  mitk::PlanarAngle::Pointer figure = mitk::PlanarAngle::New();
  this->AddFigureToDataStorage(figure, QString("Angle%1").arg(++m_AngleCounter));

  MITK_INFO << "PlanarAngle initialized...";
}

void QmitkMeasurement::ActionDrawFourPointAngleTriggered(bool checked)
{
  if(!this->AssertDrawingIsPossible(checked))
    return;

  mitk::PlanarFourPointAngle::Pointer figure =
      mitk::PlanarFourPointAngle::New();
  this->AddFigureToDataStorage(figure, QString("Four Point Angle%1").arg(++m_FourPointAngleCounter));

  MITK_INFO << "PlanarFourPointAngle initialized...";
}

void QmitkMeasurement::ActionDrawEllipseTriggered(bool checked)
{
  if(!this->AssertDrawingIsPossible(checked))
    return;

  mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();
  this->AddFigureToDataStorage(figure, QString("Circle%1").arg(++m_EllipseCounter));

  MITK_INFO << "PlanarCircle initialized...";
}

void QmitkMeasurement::ActionDrawRectangleTriggered(bool checked)
{
  if(!this->AssertDrawingIsPossible(checked))
    return;

  mitk::PlanarRectangle::Pointer figure = mitk::PlanarRectangle::New();
  this->AddFigureToDataStorage(figure, QString("Rectangle%1").arg(++m_RectangleCounter));

  MITK_INFO << "PlanarRectangle initialized...";
}

void QmitkMeasurement::ActionDrawPolygonTriggered(bool checked)
{
  if(!this->AssertDrawingIsPossible(checked))
    return;

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOn();
  this->AddFigureToDataStorage(figure, QString("Polygon%1").arg(++m_PolygonCounter));

  MITK_INFO << "PlanarPolygon initialized...";
}

void QmitkMeasurement::ActionDrawArrowTriggered(bool checked)
{
  if(!this->AssertDrawingIsPossible(checked))
    return;

  MITK_WARN << "Draw Arrow not implemented yet.";
}

void QmitkMeasurement::ActionDrawTextTriggered(bool checked)
{
  if(!this->AssertDrawingIsPossible(checked))
    return;

  MITK_WARN << "Draw Text not implemented yet.";
}

void QmitkMeasurement::Activated()
{
  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(false);
  //this->GetActiveStdMultiWidget()->GetRenderWindow1()->FullScreenMode(true);

  mitk::TNodePredicateDataType<mitk::PlanarFigure>::Pointer isPlanarFigure
    = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();

  mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = this->GetDefaultDataStorage()->GetAll();
  mitk::DataTreeNode* node = 0;
  mitk::PlanarFigure* figure = 0;
  mitk::PlanarFigureInteractor::Pointer figureInteractor = 0;
  // finally add all nodes to the model
  for(mitk::DataStorage::SetOfObjects::ConstIterator it=_NodeSet->Begin(); it!=_NodeSet->End()
    ; it++)
  {
    node = const_cast<mitk::DataTreeNode*>(it->Value().GetPointer());
    figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
    if(figure)
    {
      figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());

      if(figureInteractor.IsNull())
        figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", node);

      mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);
    }
  }

  m_Visible = true;

}

void QmitkMeasurement::Deactivated()
{
  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(true);
  //this->GetActiveStdMultiWidget()->GetRenderWindow1()->FullScreenMode(false);
  this->SetMeasurementInfoToRenderWindow("", 0);

  mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = this->GetDefaultDataStorage()->GetAll();
  mitk::DataTreeNode* node = 0;
  mitk::PlanarFigure* figure = 0;
  mitk::PlanarFigureInteractor::Pointer figureInteractor = 0;
  // finally add all nodes to the model
  for(mitk::DataStorage::SetOfObjects::ConstIterator it=_NodeSet->Begin(); it!=_NodeSet->End()
    ; it++)
  {
    node = const_cast<mitk::DataTreeNode*>(it->Value().GetPointer());
    figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
    if(figure)
    {
      figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());

      if(figureInteractor)
        mitk::GlobalInteraction::GetInstance()->RemoveInteractor(figureInteractor);
    }
  }

  m_Visible = false;
}


void QmitkMeasurement::PropertyChanged(const mitk::DataTreeNode* node, const mitk::BaseProperty* prop)
{
  this->PlanarFigureSelectionChanged();
}

void QmitkMeasurement::NodeChanged(const mitk::DataTreeNode* node)
{
  this->PlanarFigureSelectionChanged();
}

void QmitkMeasurement::NodeRemoved(const mitk::DataTreeNode* node)
{
  this->PlanarFigureSelectionChanged();
}

void QmitkMeasurement::CopyToClipboard(bool)
{
  std::vector<QString> headerRow;
  std::vector<std::vector<QString> > rows;
  QString featureName;
  QString featureQuantity;
  std::vector<QString> newRow;
  headerRow.push_back("Name");

  std::vector<mitk::DataTreeNode*> nodes = m_SelectedPlanarFigures->GetNodes();

  for (std::vector<mitk::DataTreeNode*>::iterator it = nodes.begin(); it
      != nodes.end(); ++it)
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
      if ( !planarFigure->IsFeatureActive( i ) ) continue;

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

void QmitkMeasurement::SetMeasurementInfoToRenderWindow(const QString& text,
    QmitkRenderWindow* _RenderWindow)
{
  static QmitkRenderWindow* lastRenderWindow = 0;
  if(_RenderWindow != 0)
    lastRenderWindow = _RenderWindow;

  if (!text.isEmpty())
  {
    m_MeasurementInfoAnnotation->SetText(1, text.toLatin1().data());
    mitk::VtkLayerController::GetInstance(_RenderWindow->GetRenderWindow())->InsertForegroundRenderer(
        m_MeasurementInfoRenderer, true);
  }
  else
  {
    if (lastRenderWindow != 0 && mitk::VtkLayerController::GetInstance(
        lastRenderWindow->GetRenderWindow()) ->IsRendererInserted(
        m_MeasurementInfoRenderer))
      mitk::VtkLayerController::GetInstance(lastRenderWindow->GetRenderWindow())->RemoveRenderer(
          m_MeasurementInfoRenderer);
  }
}
