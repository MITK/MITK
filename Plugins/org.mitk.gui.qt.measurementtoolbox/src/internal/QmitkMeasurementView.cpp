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

#define MEASUREMENT_DEBUG MITK_DEBUG("QmitkMeasurementView") << __LINE__ << ": "

#include "QmitkMeasurementView.h"

#include <QtGui>

#include <mitkVtkLayerController.h>
#include <mitkWeakPointer.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarAngle.h>
#include <mitkPlanarRectangle.h>
#include <mitkPlanarLine.h>
#include <mitkPlanarCross.h>
#include <mitkPlanarFourPointAngle.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkPlaneGeometry.h>
#include <mitkGlobalInteraction.h>
#include <mitkILinkedRenderWindowPart.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <QmitkRenderWindow.h>

struct QmitkPlanarFigureData
{
  QmitkPlanarFigureData()
    : m_Figure(0), m_EndPlacementObserverTag(0), m_SelectObserverTag(0), m_StartInteractionObserverTag(0), m_EndInteractionObserverTag(0)
  {
  }

  mitk::PlanarFigure* m_Figure;
  unsigned int m_EndPlacementObserverTag;
  unsigned int m_SelectObserverTag;
  unsigned int m_StartInteractionObserverTag;
  unsigned int m_EndInteractionObserverTag;
};

struct QmitkMeasurementViewData
{
  QmitkMeasurementViewData()
    : m_LineCounter(0), m_PathCounter(0), m_AngleCounter(0),
      m_FourPointAngleCounter(0), m_EllipseCounter(0),
      m_RectangleCounter(0), m_PolygonCounter(0), m_UnintializedPlanarFigure(false)
  {
  }

  // internal vars
  unsigned int m_LineCounter;
  unsigned int m_PathCounter;
  unsigned int m_AngleCounter;
  unsigned int m_FourPointAngleCounter;
  unsigned int m_EllipseCounter;
  unsigned int m_RectangleCounter;
  unsigned int m_PolygonCounter;
  QList<mitk::DataNode::Pointer> m_CurrentSelection;
  std::map<mitk::DataNode*, QmitkPlanarFigureData> m_DataNodeToPlanarFigureData;
  mitk::WeakPointer<mitk::DataNode> m_SelectedImageNode;
  bool m_UnintializedPlanarFigure;

  // WIDGETS
  QWidget* m_Parent;
  QLabel* m_SelectedImageLabel;
  QAction* m_DrawLine;
  QAction* m_DrawPath;
  QAction* m_DrawAngle;
  QAction* m_DrawFourPointAngle;
  QAction* m_DrawEllipse;
  QAction* m_DrawRectangle;
  QAction* m_DrawPolygon;
  QToolBar* m_DrawActionsToolBar;
  QActionGroup* m_DrawActionsGroup;
  QTextBrowser* m_SelectedPlanarFiguresText;
  QPushButton* m_CopyToClipboard;
  QGridLayout* m_Layout;
};

const std::string QmitkMeasurementView::VIEW_ID = "org.mitk.views.measurement";

QmitkMeasurementView::QmitkMeasurementView()
: d( new QmitkMeasurementViewData )
{
}
QmitkMeasurementView::~QmitkMeasurementView()
{
  this->RemoveAllInteractors();
  delete d;
}
void QmitkMeasurementView::CreateQtPartControl(QWidget* parent)
{
  d->m_Parent = parent;

  // image label
  QLabel* selectedImageLabel = new QLabel("Reference Image: ");
  d->m_SelectedImageLabel = new QLabel;
  d->m_SelectedImageLabel->setStyleSheet("font-weight: bold;");

  d->m_DrawActionsToolBar = new QToolBar;
  d->m_DrawActionsGroup = new QActionGroup(this);
  d->m_DrawActionsGroup->setExclusive(true);

  //# add actions
  MEASUREMENT_DEBUG << "Draw Line";
  QAction* currentAction = d->m_DrawActionsToolBar->addAction(QIcon(
    ":/measurement/line.png"), "Draw Line");
  currentAction->setCheckable(true);
  d->m_DrawLine = currentAction;
  d->m_DrawActionsToolBar->addAction(currentAction);
  d->m_DrawActionsGroup->addAction(currentAction);

  MEASUREMENT_DEBUG << "Draw Path";
  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(
    ":/measurement/path.png"), "Draw Path");
  currentAction->setCheckable(true);
  d->m_DrawPath = currentAction;
  d->m_DrawActionsToolBar->addAction(currentAction);
  d->m_DrawActionsGroup->addAction(currentAction);

  MEASUREMENT_DEBUG << "Draw Angle";
  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(
    ":/measurement/angle.png"), "Draw Angle");
  currentAction->setCheckable(true);
  d->m_DrawAngle = currentAction;
  d->m_DrawActionsToolBar->addAction(currentAction);
  d->m_DrawActionsGroup->addAction(currentAction);

  MEASUREMENT_DEBUG << "Draw Four Point Angle";
  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(
    ":/measurement/four-point-angle.png"), "Draw Four Point Angle");
  currentAction->setCheckable(true);
  d->m_DrawFourPointAngle = currentAction;
  d->m_DrawActionsToolBar->addAction(currentAction);
  d->m_DrawActionsGroup->addAction(currentAction);

  MEASUREMENT_DEBUG << "Draw Circle";
  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(
    ":/measurement/circle.png"), "Draw Circle");
  currentAction->setCheckable(true);
  d->m_DrawEllipse = currentAction;
  d->m_DrawActionsToolBar->addAction(currentAction);
  d->m_DrawActionsGroup->addAction(currentAction);

  MEASUREMENT_DEBUG << "Draw Rectangle";
  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(
    ":/measurement/rectangle.png"), "Draw Rectangle");
  currentAction->setCheckable(true);
  d->m_DrawRectangle = currentAction;
  d->m_DrawActionsToolBar->addAction(currentAction);
  d->m_DrawActionsGroup->addAction(currentAction);

  MEASUREMENT_DEBUG << "Draw Polygon";
  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(
    ":/measurement/polygon.png"), "Draw Polygon");
  currentAction->setCheckable(true);
  d->m_DrawPolygon = currentAction;
  d->m_DrawActionsToolBar->addAction(currentAction);
  d->m_DrawActionsGroup->addAction(currentAction);

  // planar figure details text
  d->m_SelectedPlanarFiguresText = new QTextBrowser;

  // copy to clipboard button
  d->m_CopyToClipboard = new QPushButton("Copy to Clipboard");

  d->m_Layout = new QGridLayout;
  d->m_Layout->addWidget(selectedImageLabel, 0, 0, 1, 1);
  d->m_Layout->addWidget(d->m_SelectedImageLabel, 0, 1, 1, 1);
  d->m_Layout->addWidget(d->m_DrawActionsToolBar, 1, 0, 1, 2);
  d->m_Layout->addWidget(d->m_SelectedPlanarFiguresText, 2, 0, 1, 2);
  d->m_Layout->addWidget(d->m_CopyToClipboard, 3, 0, 1, 2);

  d->m_Parent->setLayout(d->m_Layout);

  // create connections
  this->CreateConnections();

  // readd interactors and observers
  this->AddAllInteractors();
}
void QmitkMeasurementView::CreateConnections()
{
  QObject::connect( d->m_DrawLine, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawLineTriggered(bool) ) );
  QObject::connect( d->m_DrawPath, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawPathTriggered(bool) ) );
  QObject::connect( d->m_DrawAngle, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawAngleTriggered(bool) ) );
  QObject::connect( d->m_DrawFourPointAngle, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawFourPointAngleTriggered(bool) ) );
  QObject::connect( d->m_DrawEllipse, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawEllipseTriggered(bool) ) );
  QObject::connect( d->m_DrawRectangle, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawRectangleTriggered(bool) ) );
  QObject::connect( d->m_DrawPolygon, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawPolygonTriggered(bool) ) );
  QObject::connect( d->m_CopyToClipboard, SIGNAL( clicked(bool) )
    , this, SLOT( CopyToClipboard(bool) ) );
}

void QmitkMeasurementView::NodeAdded( const mitk::DataNode* node )
{
  // add observer for selection in renderwindow
  mitk::PlanarFigure* figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
  bool isPositionMarker (false);
  node->GetBoolProperty("isContourMarker", isPositionMarker);
  if( figure && !isPositionMarker )
  {
    MEASUREMENT_DEBUG << "figure added. will add interactor if needed.";
    mitk::PlanarFigureInteractor::Pointer figureInteractor
        = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());

    mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>( node );
    if(figureInteractor.IsNull())
    {
      figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", nonConstNode);
    }
    else
    {
      // just to be sure that the interactor is not added twice
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor(figureInteractor);
    }

    MEASUREMENT_DEBUG << "adding interactor to globalinteraction";
    mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);

    MEASUREMENT_DEBUG << "will now add observers for planarfigure";
    QmitkPlanarFigureData data;
    data.m_Figure = figure;

    // add observer for event when figure has been placed
    typedef itk::SimpleMemberCommand< QmitkMeasurementView > SimpleCommandType;
    SimpleCommandType::Pointer initializationCommand = SimpleCommandType::New();
    initializationCommand->SetCallbackFunction( this, &QmitkMeasurementView::PlanarFigureInitialized );
    data.m_EndPlacementObserverTag = figure->AddObserver( mitk::EndPlacementPlanarFigureEvent(), initializationCommand );

    // add observer for event when figure is picked (selected)
    typedef itk::MemberCommand< QmitkMeasurementView > MemberCommandType;
    MemberCommandType::Pointer selectCommand = MemberCommandType::New();
    selectCommand->SetCallbackFunction( this, &QmitkMeasurementView::PlanarFigureSelected );
    data.m_SelectObserverTag = figure->AddObserver( mitk::SelectPlanarFigureEvent(), selectCommand );

    // add observer for event when interaction with figure starts
    SimpleCommandType::Pointer startInteractionCommand = SimpleCommandType::New();
    startInteractionCommand->SetCallbackFunction( this, &QmitkMeasurementView::DisableCrosshairNavigation);
    data.m_StartInteractionObserverTag = figure->AddObserver( mitk::StartInteractionPlanarFigureEvent(), startInteractionCommand );

    // add observer for event when interaction with figure starts
    SimpleCommandType::Pointer endInteractionCommand = SimpleCommandType::New();
    endInteractionCommand->SetCallbackFunction( this, &QmitkMeasurementView::EnableCrosshairNavigation);
    data.m_EndInteractionObserverTag = figure->AddObserver( mitk::EndInteractionPlanarFigureEvent(), endInteractionCommand );

    // adding to the map of tracked planarfigures
    d->m_DataNodeToPlanarFigureData[nonConstNode] = data;
  }
  this->CheckForTopMostVisibleImage();
}

void QmitkMeasurementView::NodeChanged(const mitk::DataNode* node)
{
  // DETERMINE IF WE HAVE TO RENEW OUR DETAILS TEXT (ANY NODE CHANGED IN OUR SELECTION?)
  bool renewText = false;
  for( int i=0; i < d->m_CurrentSelection.size(); ++i )
  {
    if( node == d->m_CurrentSelection.at(i) )
    {
      renewText = true;
      break;
    }
  }

  if(renewText)
  {
    MEASUREMENT_DEBUG << "Selected nodes changed. Refreshing text.";
    this->UpdateMeasurementText();
  }

  this->CheckForTopMostVisibleImage();
}

void QmitkMeasurementView::CheckForTopMostVisibleImage(mitk::DataNode* _NodeToNeglect)
{
  d->m_SelectedImageNode = this->DetectTopMostVisibleImage().GetPointer();
  if( d->m_SelectedImageNode.GetPointer() == _NodeToNeglect )
    d->m_SelectedImageNode = 0;

  if( d->m_SelectedImageNode.IsNotNull() && d->m_UnintializedPlanarFigure == false )
  {
    MEASUREMENT_DEBUG << "Reference image found";
    d->m_SelectedImageLabel->setText( QString::fromStdString( d->m_SelectedImageNode->GetName() ) );
    d->m_DrawActionsToolBar->setEnabled(true);
    MEASUREMENT_DEBUG << "Updating Measurement text";
  }
  else
  {
    MEASUREMENT_DEBUG << "No reference image available. Will disable actions for creating new planarfigures";
    if( d->m_UnintializedPlanarFigure == false )
      d->m_SelectedImageLabel->setText( "No visible image available." );

    d->m_DrawActionsToolBar->setEnabled(false);
  }
}

void QmitkMeasurementView::NodeRemoved(const mitk::DataNode* node)
{
  MEASUREMENT_DEBUG << "node removed from data storage";

  mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
  std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it =
      d->m_DataNodeToPlanarFigureData.find(nonConstNode);

  bool isFigureFinished = false;
  bool isPlaced = false;

  if( it != d->m_DataNodeToPlanarFigureData.end() )
  {
    QmitkPlanarFigureData& data = it->second;
    // remove observers
    data.m_Figure->RemoveObserver( data.m_EndPlacementObserverTag );
    data.m_Figure->RemoveObserver( data.m_SelectObserverTag );
    data.m_Figure->RemoveObserver( data.m_StartInteractionObserverTag );
    data.m_Figure->RemoveObserver( data.m_EndInteractionObserverTag );

    MEASUREMENT_DEBUG << "removing from the list of tracked planar figures";

    isFigureFinished = data.m_Figure->GetPropertyList()->GetBoolProperty("initiallyplaced",isPlaced);
    if (!isFigureFinished) { // if the property does not yet exist or is false, drop the datanode
      PlanarFigureInitialized(); // normally called when a figure is finished, to reset all buttons
    }
    d->m_DataNodeToPlanarFigureData.erase( it );
  }

  mitk::TNodePredicateDataType<mitk::PlanarFigure>::Pointer isPlanarFigure = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();

  mitk::DataStorage::SetOfObjects::ConstPointer nodes =
   GetDataStorage()->GetDerivations(node,isPlanarFigure);

  for (unsigned int x = 0; x < nodes->size(); x++)
  {
    mitk::PlanarFigure* planarFigure  = dynamic_cast<mitk::PlanarFigure*>  (nodes->at(x)->GetData());
    if (planarFigure != NULL) {

      isFigureFinished = planarFigure->GetPropertyList()->GetBoolProperty("initiallyplaced",isPlaced);
      if (!isFigureFinished) { // if the property does not yet exist or is false, drop the datanode
        GetDataStorage()->Remove(nodes->at(x));
        if( !d->m_DataNodeToPlanarFigureData.empty() )
        {
          std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it2 =
            d->m_DataNodeToPlanarFigureData.find(nodes->at(x));
          //check if returned it2 valid
          if( it2 != d->m_DataNodeToPlanarFigureData.end() )
          {
            d->m_DataNodeToPlanarFigureData.erase( it2 );// removing planar figure from tracked figure list
            PlanarFigureInitialized(); // normally called when a figure is finished, to reset all buttons
            EnableCrosshairNavigation();
          }
        }
      }
    }
  }
  this->CheckForTopMostVisibleImage(nonConstNode);
}


void QmitkMeasurementView::PlanarFigureSelected( itk::Object* object, const itk::EventObject& )
{
  MEASUREMENT_DEBUG << "planar figure " << object << " selected";
  std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it =
      d->m_DataNodeToPlanarFigureData.begin();

  d->m_CurrentSelection.clear();
  while( it != d->m_DataNodeToPlanarFigureData.end())
  {
    mitk::DataNode* node = it->first;
    QmitkPlanarFigureData& data = it->second;

    if( data.m_Figure == object )
    {
      MITK_DEBUG << "selected node found. enabling selection";
      node->SetSelected(true);
      d->m_CurrentSelection.push_back( node );
    }
    else
    {
      node->SetSelected(false);
    }

    ++it;
  }
  this->UpdateMeasurementText();
  this->RequestRenderWindowUpdate();
}

void QmitkMeasurementView::PlanarFigureInitialized()
{
  MEASUREMENT_DEBUG << "planar figure initialized";
  d->m_UnintializedPlanarFigure = false;
  d->m_DrawActionsToolBar->setEnabled(true);

  d->m_DrawLine->setChecked(false);
  d->m_DrawPath->setChecked(false);
  d->m_DrawAngle->setChecked(false);
  d->m_DrawFourPointAngle->setChecked(false);
  d->m_DrawEllipse->setChecked(false);
  d->m_DrawRectangle->setChecked(false);
  d->m_DrawPolygon->setChecked(false);
}

void QmitkMeasurementView::SetFocus()
{
  d->m_SelectedImageLabel->setFocus();
}

void QmitkMeasurementView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/,
                                              const QList<mitk::DataNode::Pointer> &nodes)
{
  MEASUREMENT_DEBUG << "Determine the top most visible image";
  MEASUREMENT_DEBUG << "The PlanarFigure interactor will take the currently visible PlaneGeometry from the slice navigation controller";

  this->CheckForTopMostVisibleImage();

  MEASUREMENT_DEBUG << "refreshing selection and detailed text";
  d->m_CurrentSelection = nodes;
  this->UpdateMeasurementText();

  for( int i=d->m_CurrentSelection.size()-1; i>= 0; --i)
  {
    mitk::DataNode* node = d->m_CurrentSelection.at(i);

    mitk::PlanarFigure* _PlanarFigure = dynamic_cast<mitk::PlanarFigure*> (node->GetData());

    // the last selected planar figure
    if( _PlanarFigure )
    {
      mitk::ILinkedRenderWindowPart* linkedRenderWindow =
          dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart());
      if( linkedRenderWindow )
      {
        mitk::Point3D centerP = _PlanarFigure->GetGeometry()->GetOrigin();
        linkedRenderWindow->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->SelectSliceByPoint(centerP);
      }
      break;
    }
  }

  this->RequestRenderWindowUpdate();
}

void QmitkMeasurementView::ActionDrawLineTriggered(bool checked)
{
  Q_UNUSED(checked)

  mitk::PlanarLine::Pointer figure = mitk::PlanarLine::New();
  QString qString = QString("Line%1").arg(++d->m_LineCounter);
  this->AddFigureToDataStorage(figure, qString);

  MEASUREMENT_DEBUG << "PlanarLine initialized...";
}

void QmitkMeasurementView::ActionDrawPathTriggered(bool checked)
{
  Q_UNUSED(checked)

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOff();
  QString qString = QString("Path%1").arg(++d->m_PathCounter);
  mitk::DataNode::Pointer node = this->AddFigureToDataStorage(figure, qString);
  mitk::BoolProperty::Pointer closedProperty = mitk::BoolProperty::New( false );
  node->SetProperty("ClosedPlanarPolygon", closedProperty);

  MEASUREMENT_DEBUG << "PlanarPath initialized...";
}

void QmitkMeasurementView::ActionDrawAngleTriggered(bool checked)
{
  Q_UNUSED(checked)

  mitk::PlanarAngle::Pointer figure = mitk::PlanarAngle::New();
  QString qString = QString("Angle%1").arg(++d->m_AngleCounter);
  this->AddFigureToDataStorage(figure, qString);

  MEASUREMENT_DEBUG << "PlanarAngle initialized...";
}

void QmitkMeasurementView::ActionDrawFourPointAngleTriggered(bool checked)
{
  Q_UNUSED(checked)

  mitk::PlanarFourPointAngle::Pointer figure =
    mitk::PlanarFourPointAngle::New();
  QString qString = QString("Four Point Angle%1").arg(++d->m_FourPointAngleCounter);
  this->AddFigureToDataStorage(figure, qString);

  MEASUREMENT_DEBUG << "PlanarFourPointAngle initialized...";
}

void QmitkMeasurementView::ActionDrawEllipseTriggered(bool checked)
{
  Q_UNUSED(checked)

  mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();
  QString qString = QString("Circle%1").arg(++d->m_EllipseCounter);
  this->AddFigureToDataStorage(figure, qString);

  MEASUREMENT_DEBUG << "PlanarCircle initialized...";
}

void QmitkMeasurementView::ActionDrawRectangleTriggered(bool checked)
{
  Q_UNUSED(checked)

  mitk::PlanarRectangle::Pointer figure = mitk::PlanarRectangle::New();
  QString qString = QString("Rectangle%1").arg(++d->m_RectangleCounter);
  this->AddFigureToDataStorage(figure, qString);

  MEASUREMENT_DEBUG << "PlanarRectangle initialized...";
}

void QmitkMeasurementView::ActionDrawPolygonTriggered(bool checked)
{
  Q_UNUSED(checked)

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOn();
  QString qString = QString("Polygon%1").arg(++d->m_PolygonCounter);
  this->AddFigureToDataStorage(figure, qString);

  MEASUREMENT_DEBUG << "PlanarPolygon initialized...";
}

void QmitkMeasurementView::CopyToClipboard( bool checked )
{
  Q_UNUSED(checked)

  MEASUREMENT_DEBUG << "Copying current Text to clipboard...";
  QString clipboardText = d->m_SelectedPlanarFiguresText->toPlainText();
  QApplication::clipboard()->setText(clipboardText, QClipboard::Clipboard);
}

mitk::DataNode::Pointer QmitkMeasurementView::AddFigureToDataStorage(
  mitk::PlanarFigure* figure, const QString& name)
{
  // add as
  MEASUREMENT_DEBUG << "Adding new figure to datastorage...";
  if( d->m_SelectedImageNode.IsNull() )
  {
    MITK_ERROR << "No reference image available";
    return 0;
  }

  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetName(name.toStdString());
  newNode->SetData(figure);
  // set as selected
  newNode->SetSelected( true );
  this->GetDataStorage()->Add(newNode, d->m_SelectedImageNode);

  // set all others in selection as deselected
  for( int i=0; i<d->m_CurrentSelection.size(); ++i)
    d->m_CurrentSelection.at(i)->SetSelected(false);
  d->m_CurrentSelection.clear();
  d->m_CurrentSelection.push_back( newNode );
  this->UpdateMeasurementText();
  this->DisableCrosshairNavigation();
  d->m_DrawActionsToolBar->setEnabled(false);
  d->m_UnintializedPlanarFigure = true;
  return newNode;
}

void QmitkMeasurementView::UpdateMeasurementText()
{
  d->m_SelectedPlanarFiguresText->clear();

  QString infoText;
  QString plainInfoText;
  int j = 1;
  mitk::PlanarFigure* _PlanarFigure = 0;
  mitk::PlanarAngle* planarAngle = 0;
  mitk::PlanarFourPointAngle* planarFourPointAngle = 0;
  mitk::DataNode::Pointer node = 0;

  for (int i=0; i<d->m_CurrentSelection.size(); ++i, ++j)
  {
    plainInfoText.clear();
    node = d->m_CurrentSelection.at(i);
    _PlanarFigure = dynamic_cast<mitk::PlanarFigure*> (node->GetData());

    if( !_PlanarFigure )
      continue;

    if(j>1)
      infoText.append("<br />");

    infoText.append(QString("<b>%1</b><hr />").arg(QString::fromStdString(
      node->GetName())));
    plainInfoText.append(QString("%1").arg(QString::fromStdString(
      node->GetName())));

    planarAngle = dynamic_cast<mitk::PlanarAngle*> (_PlanarFigure);
    if(!planarAngle)
    {
      planarFourPointAngle = dynamic_cast<mitk::PlanarFourPointAngle*> (_PlanarFigure);
    }

    double featureQuantity = 0.0;
    for (unsigned int k = 0; k < _PlanarFigure->GetNumberOfFeatures(); ++k)
    {
      if ( !_PlanarFigure->IsFeatureActive( k ) ) continue;

      featureQuantity = _PlanarFigure->GetQuantity(k);
      if ((planarAngle && k == planarAngle->FEATURE_ID_ANGLE)
        || (planarFourPointAngle && k == planarFourPointAngle->FEATURE_ID_ANGLE))
        featureQuantity = featureQuantity * 180 / vnl_math::pi;

      infoText.append(
        QString("<i>%1</i>: %2 %3") .arg(QString(
        _PlanarFigure->GetFeatureName(k))) .arg(featureQuantity, 0, 'f',
        2) .arg(QString(_PlanarFigure->GetFeatureUnit(k))));

      plainInfoText.append(
        QString("\n%1: %2 %3") .arg(QString(_PlanarFigure->GetFeatureName(k))) .arg(
        featureQuantity, 0, 'f', 2) .arg(QString(
        _PlanarFigure->GetFeatureUnit(k))));

      if(k+1 != _PlanarFigure->GetNumberOfFeatures())
        infoText.append("<br />");
    }

    if (j != d->m_CurrentSelection.size())
      infoText.append("<br />");
  }

  d->m_SelectedPlanarFiguresText->setHtml(infoText);
}

void QmitkMeasurementView::AddAllInteractors()
{
  MEASUREMENT_DEBUG << "Adding interactors to all planar figures";

  mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = this->GetDataStorage()->GetAll();
  const mitk::DataNode* node = 0;

  for(mitk::DataStorage::SetOfObjects::ConstIterator it=_NodeSet->Begin(); it!=_NodeSet->End()
    ; it++)
  {
    node = const_cast<mitk::DataNode*>(it->Value().GetPointer());
    this->NodeAdded( node );
  }
}

void QmitkMeasurementView::RemoveAllInteractors()
{
  MEASUREMENT_DEBUG << "Removing interactors and observers from all planar figures";

  mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = this->GetDataStorage()->GetAll();
  const mitk::DataNode* node = 0;

  for(mitk::DataStorage::SetOfObjects::ConstIterator it=_NodeSet->Begin(); it!=_NodeSet->End()
    ; it++)
  {
    node = const_cast<mitk::DataNode*>(it->Value().GetPointer());
    this->NodeRemoved( node );
  }
}

mitk::DataNode::Pointer QmitkMeasurementView::DetectTopMostVisibleImage()
{
  // get all images from the data storage which are not a segmentation
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateNot::Pointer isNotBinary = mitk::NodePredicateNot::New( isBinary );
  mitk::NodePredicateAnd::Pointer isNormalImage = mitk::NodePredicateAnd::New( isImage, isNotBinary );

  mitk::DataStorage::SetOfObjects::ConstPointer Images
      = this->GetDataStorage()->GetSubset( isNormalImage );


  mitk::DataNode::Pointer currentNode;
  int maxLayer = itk::NumericTraits<int>::min();

  // iterate over selection
  for (mitk::DataStorage::SetOfObjects::ConstIterator sofIt = Images->Begin(); sofIt != Images->End(); ++sofIt)
  {
    mitk::DataNode::Pointer node = sofIt->Value();
    if ( node.IsNull() )
      continue;
    if (node->IsVisible(NULL) == false)
      continue;
    // we also do not want to assign planar figures to helper objects ( even if they are of type image )
    if (node->GetProperty("helper object"))
      continue;

    int layer = 0;
    node->GetIntProperty("layer", layer);
    if ( layer < maxLayer )
    {
      continue;
    }
    else
    {
      maxLayer = layer;
      currentNode = node;
    }
  }

  return currentNode;
}

void QmitkMeasurementView::EnableCrosshairNavigation()
{
  MEASUREMENT_DEBUG << "EnableCrosshairNavigation";

  // enable the crosshair navigation
  if (mitk::ILinkedRenderWindowPart* linkedRenderWindow =
      dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart()))
  {
    MEASUREMENT_DEBUG << "enabling linked navigation";
    linkedRenderWindow->EnableLinkedNavigation(true);
    linkedRenderWindow->EnableSlicingPlanes(true);
  }
}

void QmitkMeasurementView::DisableCrosshairNavigation()
{
  MEASUREMENT_DEBUG << "DisableCrosshairNavigation";

  // disable the crosshair navigation during the drawing
  if (mitk::ILinkedRenderWindowPart* linkedRenderWindow =
      dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart()))
  {
    MEASUREMENT_DEBUG << "disabling linked navigation";
    linkedRenderWindow->EnableLinkedNavigation(false);
    linkedRenderWindow->EnableSlicingPlanes(false);
  }
}
