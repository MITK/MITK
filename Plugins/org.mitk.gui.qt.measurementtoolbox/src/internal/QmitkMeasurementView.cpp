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

#include "QmitkMeasurementView.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QGridLayout>
#include <QToolBar>
#include <QTextBrowser>
#include <mitkInteractionEventObserver.h>
#include <mitkCoreServices.h>
#include <mitkIPropertyFilters.h>
#include <mitkPropertyFilter.h>
#include <mitkVtkLayerController.h>
#include <mitkWeakPointer.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarAngle.h>
#include <mitkPlanarRectangle.h>
#include <mitkPlanarLine.h>
#include <mitkPlanarCross.h>
#include <mitkPlanarFourPointAngle.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarBezierCurve.h>
#include <mitkPlanarSubdivisionPolygon.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkPlaneGeometry.h>
#include <mitkILinkedRenderWindowPart.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <QmitkRenderWindow.h>
#include <mitkImage.h>

#include "mitkPluginActivator.h"
#include "usModuleRegistry.h"
#include "mitkInteractionEventObserver.h"
#include "mitkDisplayInteractor.h"
#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

struct QmitkPlanarFigureData
{
  QmitkPlanarFigureData()
    : m_EndPlacementObserverTag(0),
      m_SelectObserverTag(0),
      m_StartInteractionObserverTag(0),
      m_EndInteractionObserverTag(0)
  {
  }

  mitk::PlanarFigure::Pointer m_Figure;
  unsigned int m_EndPlacementObserverTag;
  unsigned int m_SelectObserverTag;
  unsigned int m_StartInteractionObserverTag;
  unsigned int m_EndInteractionObserverTag;
};

struct QmitkMeasurementViewData
{
  QmitkMeasurementViewData()
    : m_LineCounter(0),
      m_PathCounter(0),
      m_AngleCounter(0),
      m_FourPointAngleCounter(0),
      m_CircleCounter(0),
      m_EllipseCounter(0),
      m_DoubleEllipseCounter(0),
      m_RectangleCounter(0),
      m_PolygonCounter(0),
      m_BezierCurveCounter(0),
      m_SubdivisionPolygonCounter(0),
      m_UnintializedPlanarFigure(false),
      m_ScrollEnabled(true),
      m_Parent(nullptr),
      m_SelectedImageLabel(nullptr),
      m_DrawLine(nullptr),
      m_DrawPath(nullptr),
      m_DrawAngle(nullptr),
      m_DrawFourPointAngle(nullptr),
      m_DrawRectangle(nullptr),
      m_DrawPolygon(nullptr),
      m_DrawCircle(nullptr),
      m_DrawEllipse(nullptr),
      m_DrawDoubleEllipse(nullptr),
      m_DrawBezierCurve(nullptr),
      m_DrawSubdivisionPolygon(nullptr),
      m_DrawActionsToolBar(nullptr),
      m_DrawActionsGroup(nullptr),
      m_SelectedPlanarFiguresText(nullptr),
      m_CopyToClipboard(nullptr),
      m_Layout(nullptr)
  {
  }

  unsigned int m_LineCounter;
  unsigned int m_PathCounter;
  unsigned int m_AngleCounter;
  unsigned int m_FourPointAngleCounter;
  unsigned int m_CircleCounter;
  unsigned int m_EllipseCounter;
  unsigned int m_DoubleEllipseCounter;
  unsigned int m_RectangleCounter;
  unsigned int m_PolygonCounter;
  unsigned int m_BezierCurveCounter;
  unsigned int m_SubdivisionPolygonCounter;
  QList<mitk::DataNode::Pointer> m_CurrentSelection;
  std::map<mitk::DataNode::Pointer, QmitkPlanarFigureData> m_DataNodeToPlanarFigureData;
  mitk::DataNode::Pointer m_SelectedImageNode;
  bool m_UnintializedPlanarFigure;
  bool m_ScrollEnabled;

  QWidget* m_Parent;
  QLabel* m_SelectedImageLabel;
  QAction* m_DrawLine;
  QAction* m_DrawPath;
  QAction* m_DrawAngle;
  QAction* m_DrawFourPointAngle;
  QAction* m_DrawRectangle;
  QAction* m_DrawPolygon;
  QAction* m_DrawCircle;
  QAction* m_DrawEllipse;
  QAction* m_DrawDoubleEllipse;
  QAction* m_DrawBezierCurve;
  QAction* m_DrawSubdivisionPolygon;
  QToolBar* m_DrawActionsToolBar;
  QActionGroup* m_DrawActionsGroup;
  QTextBrowser* m_SelectedPlanarFiguresText;
  QPushButton* m_CopyToClipboard;
  QGridLayout* m_Layout;
};

const std::string QmitkMeasurementView::VIEW_ID = "org.mitk.views.measurement";


const QString QmitkMeasurementView::TR_REF_IMAGE = QLabel::tr("Reference Image: "); 
const QString QmitkMeasurementView::TR_CLIPBOARD_COPY = QPushButton::tr("Copy to Clipboard"); 
const QString QmitkMeasurementView::TR_NO_AVAIBLE_IMAGE = QLabel::tr("No visible image available."); 

QmitkMeasurementView::QmitkMeasurementView()
 : d(new QmitkMeasurementViewData)
{
}

QmitkMeasurementView::~QmitkMeasurementView()
{
  auto planarFigures = this->GetAllPlanarFigures();

  for (auto it = planarFigures->Begin(); it != planarFigures->End(); ++it)
    this->NodeRemoved(it.Value());

  delete d;
}

void QmitkMeasurementView::CreateQtPartControl(QWidget* parent)
{
  d->m_Parent = parent;

  // image label
  QLabel* selectedImageLabel = new QLabel(TR_REF_IMAGE);

  d->m_SelectedImageLabel = new QLabel;
  d->m_SelectedImageLabel->setStyleSheet("font-weight: bold;");

  d->m_DrawActionsToolBar = new QToolBar;
  d->m_DrawActionsGroup = new QActionGroup(this);
  d->m_DrawActionsGroup->setExclusive(true);

  auto* currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/line.png"), "Draw Line");
  currentAction->setCheckable(true);
  d->m_DrawLine = currentAction;

  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/path.png"), "Draw Path");
  currentAction->setCheckable(true);
  d->m_DrawPath = currentAction;

  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/angle.png"), "Draw Angle");
  currentAction->setCheckable(true);
  d->m_DrawAngle = currentAction;

  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/four-point-angle.png"), "Draw Four Point Angle");
  currentAction->setCheckable(true);
  d->m_DrawFourPointAngle = currentAction;

  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/circle.png"), "Draw Circle");
  currentAction->setCheckable(true);
  d->m_DrawCircle = currentAction;

  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/ellipse.png"), "Draw Ellipse");
  currentAction->setCheckable(true);
  d->m_DrawEllipse = currentAction;

  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/doubleellipse.png"), "Draw Double Ellipse");
  currentAction->setCheckable(true);
  d->m_DrawDoubleEllipse = currentAction;

  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/rectangle.png"), "Draw Rectangle");
  currentAction->setCheckable(true);
  d->m_DrawRectangle = currentAction;

  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/polygon.png"), "Draw Polygon");
  currentAction->setCheckable(true);
  d->m_DrawPolygon = currentAction;

  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/beziercurve.png"), "Draw Bezier Curve");
  currentAction->setCheckable(true);
  d->m_DrawBezierCurve = currentAction;

  currentAction = d->m_DrawActionsToolBar->addAction(QIcon(":/measurement/subdivisionpolygon.png"), "Draw Subdivision Polygon");
  currentAction->setCheckable(true);
  d->m_DrawSubdivisionPolygon = currentAction;

  // planar figure details text
  d->m_SelectedPlanarFiguresText = new QTextBrowser;

  // copy to clipboard button
  d->m_CopyToClipboard = new QPushButton(TR_CLIPBOARD_COPY);

  d->m_Layout = new QGridLayout;
  d->m_Layout->addWidget(selectedImageLabel, 0, 0, 1, 1);
  d->m_Layout->addWidget(d->m_SelectedImageLabel, 0, 1, 1, 1);
  d->m_Layout->addWidget(d->m_DrawActionsToolBar, 1, 0, 1, 2);
  d->m_Layout->addWidget(d->m_SelectedPlanarFiguresText, 2, 0, 1, 2);
  d->m_Layout->addWidget(d->m_CopyToClipboard, 3, 0, 1, 2);

  d->m_Parent->setLayout(d->m_Layout);

  this->CreateConnections();
  this->AddAllInteractors();
}
void QmitkMeasurementView::CreateConnections()
{
  connect(d->m_DrawLine, SIGNAL(triggered(bool)), this, SLOT(OnDrawLineTriggered(bool)));
  connect(d->m_DrawPath, SIGNAL(triggered(bool)), this, SLOT(OnDrawPathTriggered(bool)));
  connect(d->m_DrawAngle, SIGNAL(triggered(bool)), this, SLOT(OnDrawAngleTriggered(bool)));
  connect(d->m_DrawFourPointAngle, SIGNAL(triggered(bool)), this, SLOT(OnDrawFourPointAngleTriggered(bool)));
  connect(d->m_DrawCircle, SIGNAL(triggered(bool)), this, SLOT(OnDrawCircleTriggered(bool)));
  connect(d->m_DrawEllipse, SIGNAL(triggered(bool)), this, SLOT(OnDrawEllipseTriggered(bool)));
  connect(d->m_DrawDoubleEllipse, SIGNAL(triggered(bool)), this, SLOT(OnDrawDoubleEllipseTriggered(bool)));
  connect(d->m_DrawRectangle, SIGNAL(triggered(bool)), this, SLOT(OnDrawRectangleTriggered(bool)));
  connect(d->m_DrawPolygon, SIGNAL(triggered(bool)), this, SLOT(OnDrawPolygonTriggered(bool)));
  connect(d->m_DrawBezierCurve, SIGNAL(triggered(bool)), this, SLOT(OnDrawBezierCurveTriggered(bool)));
  connect(d->m_DrawSubdivisionPolygon, SIGNAL(triggered(bool)), this, SLOT(OnDrawSubdivisionPolygonTriggered(bool)));
  connect(d->m_CopyToClipboard, SIGNAL(clicked(bool)), this, SLOT(OnCopyToClipboard(bool)));
}

void QmitkMeasurementView::NodeAdded(const mitk::DataNode* node)
{
  // add observer for selection in renderwindow
  mitk::PlanarFigure::Pointer planarFigure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());

  auto isPositionMarker = false;
  node->GetBoolProperty("isContourMarker", isPositionMarker);

  if (planarFigure.IsNotNull() && !isPositionMarker)
  {
    auto nonConstNode = const_cast<mitk::DataNode*>(node);
    mitk::PlanarFigureInteractor::Pointer interactor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetDataInteractor().GetPointer());

    if (interactor.IsNull())
    {
      interactor = mitk::PlanarFigureInteractor::New();
      auto planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");

      interactor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule);
      interactor->SetEventConfig("PlanarFigureConfig.xml", planarFigureModule);
    }

    interactor->SetDataNode(nonConstNode);

    QmitkPlanarFigureData data;

    data.m_Figure = planarFigure;

    typedef itk::SimpleMemberCommand<QmitkMeasurementView> SimpleCommandType;
    typedef itk::MemberCommand<QmitkMeasurementView> MemberCommandType;

    planarFigure->m_ImageNode = this->DetectTopMostVisibleImage();

    // add observer for event when figure has been placed
    auto initializationCommand = SimpleCommandType::New();
    initializationCommand->SetCallbackFunction(this, &QmitkMeasurementView::PlanarFigureInitialized);
    data.m_EndPlacementObserverTag = planarFigure->AddObserver(mitk::EndPlacementPlanarFigureEvent(), initializationCommand);

    // add observer for event when figure is picked (selected)
    auto selectCommand = MemberCommandType::New();
    selectCommand->SetCallbackFunction(this, &QmitkMeasurementView::PlanarFigureSelected);
    data.m_SelectObserverTag = planarFigure->AddObserver(mitk::SelectPlanarFigureEvent(), selectCommand);

    // add observer for event when interaction with figure starts
    auto startInteractionCommand = SimpleCommandType::New();
    startInteractionCommand->SetCallbackFunction(this, &QmitkMeasurementView::DisableCrosshairNavigation);
    data.m_StartInteractionObserverTag = planarFigure->AddObserver(mitk::StartInteractionPlanarFigureEvent(), startInteractionCommand);

    // add observer for event when interaction with figure starts
    auto endInteractionCommand = SimpleCommandType::New();
    endInteractionCommand->SetCallbackFunction(this, &QmitkMeasurementView::EnableCrosshairNavigation);
    data.m_EndInteractionObserverTag = planarFigure->AddObserver(mitk::EndInteractionPlanarFigureEvent(), endInteractionCommand);

    // adding to the map of tracked planarfigures
    d->m_DataNodeToPlanarFigureData[nonConstNode] = data;
  }

  this->CheckForTopMostVisibleImage();
}

void QmitkMeasurementView::NodeChanged(const mitk::DataNode* node)
{
  // DETERMINE IF WE HAVE TO RENEW OUR DETAILS TEXT (ANY NODE CHANGED IN OUR SELECTION?)
  auto renewText = false;

  for (int i = 0; i < d->m_CurrentSelection.size(); ++i)
  {
    if (node == d->m_CurrentSelection[i])
    {
      renewText = true;
      break;
    }
  }

  if (renewText)
    this->UpdateMeasurementText();

  this->CheckForTopMostVisibleImage();
}

void QmitkMeasurementView::CheckForTopMostVisibleImage(mitk::DataNode* nodeToNeglect)
{
  d->m_SelectedImageNode = this->DetectTopMostVisibleImage();

  if (d->m_SelectedImageNode.GetPointer() == nodeToNeglect)
    d->m_SelectedImageNode = nullptr;

  auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto isHelpherObject = mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true));
  auto isNotHelpherObject = mitk::NodePredicateNot::New(isHelpherObject);

  auto nodeElements = this->GetDataStorage()->GetSubset(isNotHelpherObject);

  if (d->m_SelectedImageNode.IsNotNull() && d->m_UnintializedPlanarFigure == false)
  {
    d->m_SelectedImageLabel->setText(QString::fromStdString(d->m_SelectedImageNode->GetName()));
    d->m_DrawActionsToolBar->setEnabled(true);
  }
  else if (d->m_UnintializedPlanarFigure == false && nodeElements->size() != 0)
  {
    d->m_SelectedImageLabel->setText(QString::fromStdString("Working without an image..."));
    d->m_DrawActionsToolBar->setEnabled(true);
  }
  else
  {
    if( d->m_UnintializedPlanarFigure == false )
      d->m_SelectedImageLabel->setText(TR_NO_AVAIBLE_IMAGE);

    d->m_DrawActionsToolBar->setEnabled(false);
  }
}

void QmitkMeasurementView::NodeRemoved(const mitk::DataNode* node)
{
  auto nonConstNode = const_cast<mitk::DataNode*>(node);
  auto it = d->m_DataNodeToPlanarFigureData.find(nonConstNode);
  auto isFigureFinished = false;
  auto isPlaced = false;

  if (it != d->m_DataNodeToPlanarFigureData.end())
  {
    QmitkPlanarFigureData& data = it->second;

    data.m_Figure->RemoveObserver(data.m_EndPlacementObserverTag);
    data.m_Figure->RemoveObserver(data.m_SelectObserverTag);
    data.m_Figure->RemoveObserver(data.m_StartInteractionObserverTag);
    data.m_Figure->RemoveObserver(data.m_EndInteractionObserverTag);

    isFigureFinished = data.m_Figure->GetPropertyList()->GetBoolProperty("initiallyplaced", isPlaced);

    if (!isFigureFinished) // if the property does not yet exist or is false, drop the datanode
      this->PlanarFigureInitialized(); // normally called when a figure is finished, to reset all buttons

    d->m_DataNodeToPlanarFigureData.erase( it );
  }

  if (nonConstNode != nullptr)
    nonConstNode->SetDataInteractor(nullptr);

  auto isPlanarFigure = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();
  auto nodes = this->GetDataStorage()->GetDerivations(node, isPlanarFigure);

  for (unsigned int x = 0; x < nodes->size(); ++x)
  {
    mitk::PlanarFigure::Pointer planarFigure = dynamic_cast<mitk::PlanarFigure*>(nodes->at(x)->GetData());

    if (planarFigure.IsNotNull())
    {
      isFigureFinished = planarFigure->GetPropertyList()->GetBoolProperty("initiallyplaced",isPlaced);

      if (!isFigureFinished) // if the property does not yet exist or is false, drop the datanode
      {
        this->GetDataStorage()->Remove(nodes->at(x));

        if (!d->m_DataNodeToPlanarFigureData.empty())
        {
          it = d->m_DataNodeToPlanarFigureData.find(nodes->at(x));

          if (it != d->m_DataNodeToPlanarFigureData.end())
          {
            d->m_DataNodeToPlanarFigureData.erase(it);
            this->PlanarFigureInitialized(); // normally called when a figure is finished, to reset all buttons
            this->EnableCrosshairNavigation();
          }
        }
      }
    }
  }

  this->CheckForTopMostVisibleImage(nonConstNode);
}

void QmitkMeasurementView::PlanarFigureSelected(itk::Object* object, const itk::EventObject&)
{
  d->m_CurrentSelection.clear();

  auto it = d->m_DataNodeToPlanarFigureData.begin();

  while (it != d->m_DataNodeToPlanarFigureData.end())
  {
    auto node = it->first;
    QmitkPlanarFigureData& data = it->second;

    if (data.m_Figure == object )
    {
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
  d->m_UnintializedPlanarFigure = false;

  d->m_DrawActionsToolBar->setEnabled(true);

  d->m_DrawLine->setChecked(false);
  d->m_DrawPath->setChecked(false);
  d->m_DrawAngle->setChecked(false);
  d->m_DrawFourPointAngle->setChecked(false);
  d->m_DrawCircle->setChecked(false);
  d->m_DrawEllipse->setChecked(false);
  d->m_DrawDoubleEllipse->setChecked(false);
  d->m_DrawRectangle->setChecked(false);
  d->m_DrawPolygon->setChecked(false);
  d->m_DrawBezierCurve->setChecked(false);
  d->m_DrawSubdivisionPolygon->setChecked(false);
}

void QmitkMeasurementView::SetFocus()
{
  d->m_SelectedImageLabel->setFocus();
}

void QmitkMeasurementView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  this->CheckForTopMostVisibleImage();

  d->m_CurrentSelection = nodes;
  this->UpdateMeasurementText();

  // bug 16600: deselecting all planarfigures by clicking on datamanager when no node is selected
  if (d->m_CurrentSelection.size() == 0)
  {
    // bug 18440: resetting the selected image label here because unselecting the
    // current node did not reset the label
    d->m_SelectedImageLabel->setText("No visible image available.");

    auto isPlanarFigure = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();
    auto planarFigures = this->GetDataStorage()->GetSubset(isPlanarFigure);

    // setting all planar figures which are not helper objects not selected
    for (mitk::DataStorage::SetOfObjects::ConstIterator it = planarFigures->Begin(); it != planarFigures->End(); ++it)
    {
      auto node = it.Value();

      auto isHelperObject = false;
      node->GetBoolProperty("helper object", isHelperObject);

      if (!isHelperObject)
        node->SetSelected(false);
    }
  }

  for (int i = d->m_CurrentSelection.size() - 1; i >= 0; --i)
  {
    auto node = d->m_CurrentSelection[i];
    mitk::PlanarFigure::Pointer planarFigure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());

    // the last selected planar figure

    if (planarFigure.IsNotNull() && planarFigure->GetPlaneGeometry())
    {
      auto planarFigureInitializedWindow = false;
      auto linkedRenderWindow = dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart());
      QmitkRenderWindow* selectedRenderWindow;

      if (!linkedRenderWindow)
        return;

      auto axialRenderWindow = linkedRenderWindow->GetQmitkRenderWindow("axial");
      auto sagittalRenderWindow = linkedRenderWindow->GetQmitkRenderWindow("sagittal");
      auto coronalRenderWindow = linkedRenderWindow->GetQmitkRenderWindow("coronal");
      auto threeDimRenderWindow = linkedRenderWindow->GetQmitkRenderWindow("3d");

      if (node->GetBoolProperty("planarFigureInitializedWindow", planarFigureInitializedWindow, axialRenderWindow->GetRenderer()))
      {
        selectedRenderWindow = axialRenderWindow;
      }
      else if (node->GetBoolProperty("planarFigureInitializedWindow", planarFigureInitializedWindow, sagittalRenderWindow->GetRenderer()))
      {
        selectedRenderWindow = sagittalRenderWindow;
      }
      else if (node->GetBoolProperty("planarFigureInitializedWindow", planarFigureInitializedWindow, coronalRenderWindow->GetRenderer()))
      {
        selectedRenderWindow = coronalRenderWindow;
      }
      else if (node->GetBoolProperty("planarFigureInitializedWindow", planarFigureInitializedWindow, threeDimRenderWindow->GetRenderer()))
      {
        selectedRenderWindow = threeDimRenderWindow;
      }
      else
      {
        selectedRenderWindow = nullptr;
      }

      auto planeGeometry = dynamic_cast<const mitk::PlaneGeometry*>(planarFigure->GetPlaneGeometry());
      auto normal = planeGeometry->GetNormalVnl();

      mitk::PlaneGeometry::ConstPointer axialPlane = axialRenderWindow->GetRenderer()->GetCurrentWorldPlaneGeometry();
      auto axialNormal = axialPlane->GetNormalVnl();

      mitk::PlaneGeometry::ConstPointer sagittalPlane = sagittalRenderWindow->GetRenderer()->GetCurrentWorldPlaneGeometry();
      auto sagittalNormal = sagittalPlane->GetNormalVnl();

      mitk::PlaneGeometry::ConstPointer coronalPlane = coronalRenderWindow->GetRenderer()->GetCurrentWorldPlaneGeometry();
      auto coronalNormal = coronalPlane->GetNormalVnl();

      normal[0]         = fabs(normal[0]);
      normal[1]         = fabs(normal[1]);
      normal[2]         = fabs(normal[2]);
      axialNormal[0]    = fabs(axialNormal[0]);
      axialNormal[1]    = fabs(axialNormal[1]);
      axialNormal[2]    = fabs(axialNormal[2]);
      sagittalNormal[0] = fabs(sagittalNormal[0]);
      sagittalNormal[1] = fabs(sagittalNormal[1]);
      sagittalNormal[2] = fabs(sagittalNormal[2]);
      coronalNormal[0]  = fabs(coronalNormal[0]);
      coronalNormal[1]  = fabs(coronalNormal[1]);
      coronalNormal[2]  = fabs(coronalNormal[2]);

      auto ang1 = angle(normal, axialNormal);
      auto ang2 = angle(normal, sagittalNormal);
      auto ang3 = angle(normal, coronalNormal);

      if (ang1 < ang2 && ang1 < ang3)
      {
        selectedRenderWindow = axialRenderWindow;
      }
      else
      {
        if (ang2 < ang3)
        {
          selectedRenderWindow = sagittalRenderWindow;
        }
        else
        {
          selectedRenderWindow = coronalRenderWindow;
        }
      }

      // re-orient view
      if (selectedRenderWindow)
        selectedRenderWindow->GetSliceNavigationController()->ReorientSlices(planeGeometry->GetOrigin(), planeGeometry->GetNormal());
    }

    break;
  }

  this->RequestRenderWindowUpdate();
}

void QmitkMeasurementView::OnDrawLineTriggered(bool)
{
   this->AddFigureToDataStorage(
     mitk::PlanarLine::New(),
     QString("Line%1").arg(++d->m_LineCounter));
}

void QmitkMeasurementView::OnDrawPathTriggered(bool)
{

  auto propertyFilters = mitk::CoreServices::GetPropertyFilters();

  if (propertyFilters != nullptr)
  {
    mitk::PropertyFilter filter;
    filter.AddEntry("ClosedPlanarPolygon", mitk::PropertyFilter::Blacklist);

    propertyFilters->AddFilter(filter, "PlanarPolygon");
  }

  mitk::PlanarPolygon::Pointer planarFigure = mitk::PlanarPolygon::New();
  planarFigure->ClosedOff();

  auto node = this->AddFigureToDataStorage(
    planarFigure,
    QString("Path%1").arg(++d->m_PathCounter));

  node->SetProperty("ClosedPlanarPolygon", mitk::BoolProperty::New(false));
  node->SetProperty("planarfigure.isextendable", mitk::BoolProperty::New(true));
}

void QmitkMeasurementView::OnDrawAngleTriggered(bool)
{
  this->AddFigureToDataStorage(
    mitk::PlanarAngle::New(),
    QString("Angle%1").arg(++d->m_AngleCounter));
}

void QmitkMeasurementView::OnDrawFourPointAngleTriggered(bool)
{
  this->AddFigureToDataStorage(
    mitk::PlanarFourPointAngle::New(),
    QString("Four Point Angle%1").arg(++d->m_FourPointAngleCounter));
}

void QmitkMeasurementView::OnDrawCircleTriggered(bool)
{
  Q_UNUSED(checked)

  mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();

  QString qString = QString("Circle%1").arg(++d->m_CircleCounter);
  this->AddFigureToDataStorage(figure, qString);

  MEASUREMENT_DEBUG << "PlanarCircle initialized...";
}

void QmitkMeasurementView::OnDrawEllipseTriggered(bool)
{
  Q_UNUSED(checked)

  mitk::PlanarEllipse::Pointer figure = mitk::PlanarEllipse::New();
  QString qString = QString("Ellipse%1").arg(++d->m_EllipseCounter);
  this->AddFigureToDataStorage(figure, qString);

  MEASUREMENT_DEBUG << "PlanarEllipse initialized...";
}

void QmitkMeasurementView::OnDrawDoubleEllipseTriggered(bool)
{
  this->AddFigureToDataStorage(
    mitk::PlanarDoubleEllipse::New(),
    QString("DoubleEllipse%1").arg(++d->m_DoubleEllipseCounter));
}

void QmitkMeasurementView::OnDrawBezierCurveTriggered(bool)
{
  this->AddFigureToDataStorage(
    mitk::PlanarBezierCurve::New(),
    QString("BezierCurve%1").arg(++d->m_BezierCurveCounter));
}

void QmitkMeasurementView::OnDrawSubdivisionPolygonTriggered(bool)
{
  this->AddFigureToDataStorage(
    mitk::PlanarSubdivisionPolygon::New(),
    QString("SubdivisionPolygon%1").arg(++d->m_SubdivisionPolygonCounter));
}

void QmitkMeasurementView::OnDrawRectangleTriggered(bool)
{
  this->AddFigureToDataStorage(
    mitk::PlanarRectangle::New(),
    QString("Rectangle%1").arg(++d->m_RectangleCounter));
}

void QmitkMeasurementView::OnDrawPolygonTriggered(bool)
{
  auto planarFigure = mitk::PlanarPolygon::New();
  planarFigure->ClosedOn();

  auto node = this->AddFigureToDataStorage(
    planarFigure,
    QString("Polygon%1").arg(++d->m_PolygonCounter));

  node->SetProperty("planarfigure.isextendable", mitk::BoolProperty::New(true));
}

void QmitkMeasurementView::OnCopyToClipboard(bool)
{
  QApplication::clipboard()->setText(d->m_SelectedPlanarFiguresText->toPlainText(), QClipboard::Clipboard);
}

mitk::DataNode::Pointer QmitkMeasurementView::AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name)
{
  auto newNode = mitk::DataNode::New();
  newNode->SetName(name.toStdString());
  newNode->SetData(figure);
  newNode->SetSelected(true);

  if (d->m_SelectedImageNode.IsNotNull())
  {
    this->GetDataStorage()->Add(newNode, d->m_SelectedImageNode);
  }
  else
  {
    this->GetDataStorage()->Add(newNode);
  }

  for (auto &node : d->m_CurrentSelection)
    node->SetSelected(false);

  d->m_CurrentSelection.clear();
  d->m_CurrentSelection.push_back(newNode);

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

  mitk::PlanarFigure::Pointer planarFigure;
  mitk::PlanarAngle::Pointer planarAngle;
  mitk::PlanarFourPointAngle::Pointer planarFourPointAngle;
  mitk::DataNode::Pointer node;

  for (int i = 0; i < d->m_CurrentSelection.size(); ++i, ++j)
  {
    plainInfoText.clear();
    node = d->m_CurrentSelection[i];
    planarFigure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());

    if (planarFigure.IsNull())
      continue;

    if (j > 1)
      infoText.append("<br />");

    infoText.append(QString("<b>%1</b><hr />").arg(QString::fromStdString(node->GetName())));
    plainInfoText.append(QString("%1").arg(QString::fromStdString(node->GetName())));

    planarAngle = dynamic_cast<mitk::PlanarAngle*> (planarFigure.GetPointer());

    if (planarAngle.IsNull())
      planarFourPointAngle = dynamic_cast<mitk::PlanarFourPointAngle*> (planarFigure.GetPointer());

    double featureQuantity = 0.0;

    for (unsigned int k = 0; k < planarFigure->GetNumberOfFeatures(); ++k)
    {
      if (!planarFigure->IsFeatureActive(k))
        continue;

      featureQuantity = planarFigure->GetQuantity(k);

      if ((planarAngle.IsNotNull() && k == planarAngle->FEATURE_ID_ANGLE) || (planarFourPointAngle.IsNotNull() && k == planarFourPointAngle->FEATURE_ID_ANGLE))
        featureQuantity = featureQuantity * 180 / vnl_math::pi;

      infoText.append(QString("<i>%1</i>: %2 %3")
        .arg(QString(planarFigure->GetFeatureName(k)))
        .arg(featureQuantity, 0, 'f', 2)
        .arg(QString(planarFigure->GetFeatureUnit(k))));

      plainInfoText.append(QString("\n%1: %2 %3")
        .arg(QString(planarFigure->GetFeatureName(k)))
        .arg(featureQuantity, 0, 'f', 2)
        .arg(QString(planarFigure->GetFeatureUnit(k))));

      if (k + 1 != planarFigure->GetNumberOfFeatures())
        infoText.append("<br />");
    }

    if (j != d->m_CurrentSelection.size())
      infoText.append("<br />");
  }

  d->m_SelectedPlanarFiguresText->setHtml(infoText);
}

void QmitkMeasurementView::AddAllInteractors()
{
  auto planarFigures = this->GetAllPlanarFigures();

  for (auto it = planarFigures->Begin(); it != planarFigures->End(); ++it)
    this->NodeAdded(it.Value());
}

mitk::DataNode::Pointer QmitkMeasurementView::DetectTopMostVisibleImage()
{
  // get all images from the data storage which are not a segmentation
  auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  auto isNotBinary = mitk::NodePredicateNot::New(isBinary);
  auto isNormalImage = mitk::NodePredicateAnd::New(isImage, isNotBinary);

  auto images = this->GetDataStorage()->GetSubset(isNormalImage);

  mitk::DataNode::Pointer currentNode;

  int maxLayer = std::numeric_limits<int>::min();
  int layer = 0;

  // iterate over selection
  for (auto it = images->Begin(); it != images->End(); ++it)
  {
    auto node = it->Value();

    if (node.IsNull())
      continue;

    if (node->IsVisible(nullptr) == false)
      continue;

    // we also do not want to assign planar figures to helper objects ( even if they are of type image )
    if (node->GetProperty("helper object") != nullptr)
      continue;

    node->GetIntProperty("layer", layer);

    if (layer < maxLayer)
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
  // enable the crosshair navigation
  // Re-enabling InteractionEventObservers that have been previously disabled for legacy handling of Tools
  // in new interaction framework
  for (const auto& displayInteractorConfig : m_DisplayInteractorConfigs)
  {
    if (displayInteractorConfig.first)
    {
      auto displayInteractor = static_cast<mitk::DisplayInteractor*>(us::GetModuleContext()->GetService<mitk::InteractionEventObserver>(displayInteractorConfig.first));

      if (displayInteractor != nullptr)
      {
        // here the regular configuration is loaded again
        displayInteractor->SetEventConfig(displayInteractorConfig.second);
      }
    }
  }

  m_DisplayInteractorConfigs.clear();
  d->m_ScrollEnabled = true;
}

void QmitkMeasurementView::DisableCrosshairNavigation()
{
  // dont deactivate twice, else we will clutter the config list ...
  if (d->m_ScrollEnabled == false)
      return;

  // As a legacy solution the display interaction of the new interaction framework is disabled here  to avoid conflicts with tools
  // Note: this only affects InteractionEventObservers (formerly known as Listeners) all DataNode specific interaction will still be enabled
  m_DisplayInteractorConfigs.clear();

  auto eventObservers = us::GetModuleContext()->GetServiceReferences<mitk::InteractionEventObserver>();

  for (const auto& eventObserver : eventObservers)
  {
    auto displayInteractor = dynamic_cast<mitk::DisplayInteractor*>(us::GetModuleContext()->GetService<mitk::InteractionEventObserver>(eventObserver));

    if (displayInteractor != nullptr)
    {
      // remember the original configuration
      m_DisplayInteractorConfigs.insert(std::make_pair(eventObserver, displayInteractor->GetEventConfig()));
      // here the alternative configuration is loaded
      displayInteractor->SetEventConfig("DisplayConfigMITKLimited.xml");
    }
  }

  d->m_ScrollEnabled = false;
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkMeasurementView::GetAllPlanarFigures() const
{
  auto isPlanarFigure = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();
  auto isNotHelperObject = mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(false));
  auto isNotHelperButPlanarFigure = mitk::NodePredicateAnd::New( isPlanarFigure, isNotHelperObject );

  return this->GetDataStorage()->GetSubset(isPlanarFigure);
}
