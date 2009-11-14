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
#include <QmitkDataStorageTableModel.h>
#include "mitkNodePredicateDataType.h"
#include "mitkPlanarFigure.h"

#include <vtkTextProperty.h>
#include <vtkRenderer.h>
#include <vtkCornerAnnotation.h>
#include <mitkVtkLayerController.h>

QmitkMeasurement::QmitkMeasurement()
: m_PointSetInteractor(0)
, m_CurrentPointSetNode(0)
, m_Layout(0)
, m_DrawActionsToolBar(0)
, m_LineCounter(0)
, m_PathCounter(0)
, m_AngleCounter(0)
, m_FourPointAngleCounter(0)
, m_EllipseCounter(0)
, m_RectangleCounter(0)
, m_PolygonCounter(0)
, m_MeasurementInfoRenderer(0)
, m_MeasurementInfoAnnotation(0)
{

}

QmitkMeasurement::~QmitkMeasurement()
{

  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(m_SelectionListener);
  m_MeasurementInfoRenderer->Delete();
}

void QmitkMeasurement::CreateQtPartControl( QWidget* parent )
{
  m_MeasurementInfoRenderer = vtkRenderer::New();
  m_MeasurementInfoAnnotation = vtkCornerAnnotation::New();
  vtkTextProperty *textProp = vtkTextProperty::New();

  m_MeasurementInfoAnnotation->SetMaximumFontSize(12);
  textProp->SetColor( 1.0, 1.0, 1.0 );
  m_MeasurementInfoAnnotation->SetTextProperty( textProp );

  m_MeasurementInfoRenderer->AddActor(m_MeasurementInfoAnnotation);
  m_DrawActionsToolBar = new QToolBar;

  //# add actions
  QAction* currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/line.png"), "Draw Line");
  m_DrawLine = currentAction;
  m_DrawLine->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawLineTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/path.png"), "Draw Path");
  m_DrawPath = currentAction;
  m_DrawPath->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawPathTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/angle.png"), "Draw Angle");
  m_DrawAngle = currentAction;
  m_DrawAngle->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawAngleTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/four-point-angle.png"), "Draw Four Point Angle");
  m_DrawFourPointAngle = currentAction;
  m_DrawFourPointAngle->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawFourPointAngleTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/circle.png"), "Draw Ellipse");
  m_DrawEllipse = currentAction;
  m_DrawEllipse->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawEllipseTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/rectangle.png"), "Draw Rectangle");
  m_DrawRectangle = currentAction;
  m_DrawRectangle->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawRectangleTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/polygon.png"), "Draw Polygon");
  m_DrawPolygon = currentAction;
  m_DrawPolygon->setCheckable(true);
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawPolygonTriggered(bool) ) );

  /*currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/arrow.png"), "Draw Arrow");
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawArrowTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/text.png"), "Draw Text");
  m_DrawActionsToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawTextTriggered(bool) ) );*/

  //# m_NodeTableModel
  mitk::NodePredicateProperty::Pointer isHelperObject = mitk::NodePredicateProperty::New("helper object"
    , mitk::BoolProperty::New(true));

  mitk::NodePredicateNOT::Pointer isNotHelperObject 
    = mitk::NodePredicateNOT::New(isHelperObject);// Show only nodes that really contain dat

  mitk::TNodePredicateDataType<mitk::PlanarFigure>::Pointer isPlanarFigure = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();

  mitk::NodePredicateAND::Pointer dataIsPlanarFigureAndIsNotHelperObject = mitk::NodePredicateAND::New(isPlanarFigure,
    isNotHelperObject);

  m_PlanarFiguresModel = new QmitkPlanarFiguresTableModel(this->GetDefaultDataStorage(), dataIsPlanarFigureAndIsNotHelperObject);
  QObject::connect( m_PlanarFiguresModel, SIGNAL(rowsInserted (const QModelIndex&, int, int))
    , this, SLOT(PlanarFigureTableModelRowsInserted ( const QModelIndex&, int, int )) );

  QLabel* selectedImageLabel = new QLabel("Selected Image: ");
  m_SelectedImage = new QLabel("None selected. Please select an image!");

  //# m_PlanarFiguresTable
  m_PlanarFiguresTable = new QTableView;
  //m_PlanarFiguresTable->setContextMenuPolicy(Qt::CustomContextMenu);
  m_PlanarFiguresTable->setSelectionMode( QAbstractItemView::ExtendedSelection );
  m_PlanarFiguresTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_PlanarFiguresTable->horizontalHeader()->setStretchLastSection(true);
  m_PlanarFiguresTable->setAlternatingRowColors(true);
  m_PlanarFiguresTable->setSortingEnabled(true);
  //m_PlanarFiguresTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_PlanarFiguresTable->setModel(m_PlanarFiguresModel);

  QObject::connect( m_PlanarFiguresTable->selectionModel()
    , SIGNAL( selectionChanged ( const QItemSelection &, const QItemSelection & ) )
    , this
    , SLOT( PlanarFigureSelectionChanged ( const QItemSelection &, const QItemSelection & ) ) );

  m_CopyToClipboard = new QPushButton("Copy to Clipboard");
  QObject::connect( m_CopyToClipboard, SIGNAL( clicked(bool) )
    , this, SLOT( CopyToClipboard(bool) ) );

  m_Layout = new QGridLayout;
  //m_Layout->addWidget(m_DrawActionsToolBar, 0, 0, 1, 1);
  m_Layout->addWidget(selectedImageLabel, 0, 0, 1, 1);
  m_Layout->addWidget(m_SelectedImage, 0, 1, 1, 1);
  m_Layout->addWidget(m_DrawActionsToolBar, 1, 0, 1, 2);
  m_Layout->addWidget(m_PlanarFiguresTable, 2, 0, 1, 2);
  m_Layout->addWidget(m_CopyToClipboard, 3, 0, 1, 2);
  m_Layout->setRowStretch(0, 1);
  m_Layout->setRowStretch(1, 1);
  m_Layout->setRowStretch(2, 10);
  m_Layout->setRowStretch(3, 1);
  m_Layout->setContentsMargins(2,2,2,2);

  parent->setLayout(m_Layout);

  // Initialize selection listener mechanism
  m_SelectionListener = cherry::ISelectionListener::Pointer(
    new cherry::SelectionChangedAdapter< QmitkMeasurement >(
      this, &QmitkMeasurement::SelectionChanged) );
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(m_SelectionListener);
  cherry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection());

  m_CurrentSelection = selection.Cast< const cherry::IStructuredSelection >();
  this->SelectionChanged(cherry::SmartPointer<IWorkbenchPart>(NULL), selection);

  m_SelectionProvider = new mitk::MeasurementSelectionProvider();
  //m_SelectionProvider->SetItemSelectionModel(m_NodeTreeView->selectionModel());
  //this->GetSite()->SetSelectionProvider(m_SelectionProvider);

}

void QmitkMeasurement::SelectionChanged( cherry::IWorkbenchPart::Pointer sourcepart, 
  cherry::ISelection::ConstPointer selection )
{
  LOG_INFO << "SelectionChanged.";
  // By default: no image selected

  if ( sourcepart == this ||  // prevents being notified by own selection events
    !selection.Cast<const cherry::IStructuredSelection>() ) // checks that the selection is a IStructuredSelection and not NULL
  {
    LOG_INFO << "Selection failed.";
    return; // otherwise we get "null selection" events each time the view is activated/focussed
  }

  // save current selection in member variable
  m_CurrentSelection = selection.Cast< const cherry::IStructuredSelection >();

  if ( m_CurrentSelection.IsNull() || (m_CurrentSelection->Size() != 1) )
  {
    LOG_INFO << "Selection invalid.";
    return;
  }

  // Get selected element
  //mitk::DataTreeNodeObject *nodeObject = 
  //  m_CurrentSelection->GetFirstElement().Cast< mitk::DataTreeNodeObject >();
  cherry::IStructuredSelection::iterator it = m_CurrentSelection->Begin();
  mitk::DataTreeNodeObject::Pointer nodeObject = it->Cast <mitk::DataTreeNodeObject >();

  mitk::DataTreeNode *node = nodeObject->GetDataTreeNode();

  // Check if an image has been selected
  if ( node->GetData() && dynamic_cast< mitk::Image * >( node->GetData() ) )
  {
    // YES: node becomes new selected image node
    m_SelectedImageNode = node;
    m_SelectedImage->setText(QString::fromStdString(m_SelectedImageNode->GetName()));
  }
  else
  {
    m_SelectedImageNode = NULL;
    m_SelectedImage->setText("None selected!");
  }
}


void QmitkMeasurement::ActionDrawLineTriggered( bool  checked )
{
  if ( m_SelectedImageNode.IsNull() || !checked)
  {
    m_DrawLine->setChecked(false);
    return;
  }

  mitk::PlanarLine::Pointer figure = mitk::PlanarLine::New();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Line%1").arg(++m_LineCounter).toStdString());
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarLine initialized...";
  m_DrawLine->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}

void QmitkMeasurement::ActionDrawPathTriggered( bool  checked )
{
  if ( m_SelectedImageNode.IsNull() || !checked)
  {
    m_DrawPath->setChecked(false);
    return;
  }

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOff();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Path%1").arg(++m_PathCounter).toStdString());
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarPath initialized...";
  m_DrawPath->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}

void QmitkMeasurement::ActionDrawAngleTriggered( bool  checked )
{
  if ( m_SelectedImageNode.IsNull() || !checked )
  {
    m_DrawAngle->setChecked(false);
    return;
  }

  mitk::PlanarAngle::Pointer figure = mitk::PlanarAngle::New();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Angle%1").arg(++m_AngleCounter).toStdString());
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarAngle initialized...";
  m_DrawAngle->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}

void QmitkMeasurement::ActionDrawFourPointAngleTriggered( bool  checked )
{
  if ( m_SelectedImageNode.IsNull() || !checked )
  {
    m_DrawFourPointAngle->setChecked(false);
    return;
  }
  mitk::PlanarFourPointAngle::Pointer figure = mitk::PlanarFourPointAngle::New();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Four Point Angle%1").arg(++m_FourPointAngleCounter).toStdString());

  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarFourPointAngle initialized...";
  m_DrawFourPointAngle->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMeasurement::ActionDrawEllipseTriggered( bool  checked )
{
  if ( m_SelectedImageNode.IsNull() || !checked )
  {
    m_DrawEllipse->setChecked(false);
    return;
  }

  mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Ellipse%1").arg(++m_EllipseCounter).toStdString());
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarCircle initialized...";
  m_DrawEllipse->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}

void QmitkMeasurement::ActionDrawRectangleTriggered( bool  checked )
{
  LOG_WARN << "PlanarRectangle is not implemented yet";
  if ( m_SelectedImageNode.IsNull() || !checked )
  {
    m_DrawRectangle->setChecked(false);
    return;
  }

  mitk::PlanarRectangle::Pointer figure = mitk::PlanarRectangle::New();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Rectangle%1").arg(++m_RectangleCounter).toStdString());
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarRectangle initialized...";
  m_DrawRectangle->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMeasurement::ActionDrawPolygonTriggered( bool  checked )
{
  if ( m_SelectedImageNode.IsNull() || !checked )
  {
    m_DrawPolygon->setChecked(false);
    return;
  }

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOn();

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetName(QString("Polygon%1").arg(++m_PolygonCounter).toStdString());
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarPolygon initialized...";
  m_DrawPolygon->setChecked(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}


void QmitkMeasurement::ActionDrawArrowTriggered( bool  checked )
{
  if ( m_SelectedImageNode.IsNull() || !checked )
  {
    return;
  }
}

void QmitkMeasurement::ActionDrawTextTriggered( bool  checked )
{
  if ( m_SelectedImageNode.IsNull() || !checked )
  {
    return;
  }
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


void QmitkMeasurement::PlanarFigureTableModelRowsInserted( const QModelIndex & parent, int start, int end )
{
    m_PlanarFiguresTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void QmitkMeasurement::CopyToClipboard( bool checked /*= false */ )
{
    std::vector<mitk::DataTreeNode*> nodes = m_PlanarFiguresModel->GetNodeSet();
    std::vector<QString> headerRow;
    std::vector< std::vector<QString> > rows;
    QString featureName;
    QString featureQuantity;
    std::vector<QString> newRow;
    headerRow.push_back("Name");

    for(std::vector<mitk::DataTreeNode*>::iterator it=nodes.begin(); it!=nodes.end(); ++it)
    {
      mitk::PlanarFigure* planarFigure = dynamic_cast<mitk::PlanarFigure*>((*it)->GetData());
      if(!planarFigure)
        continue;
  
      newRow.clear();
      newRow.push_back(QString::fromStdString((*it)->GetName()));
      newRow.resize(headerRow.size());
      for(int i=0; i<planarFigure->GetNumberOfFeatures(); ++i)
      {
        featureName = planarFigure->GetFeatureName(i);
        featureName.append(QString(" [%1]").arg(planarFigure->GetFeatureUnit(i)));
        std::vector<QString>::iterator itColumn = std::find(headerRow.begin(), headerRow.end(), featureName);
  
        featureQuantity = QString("%1").arg(planarFigure->GetQuantity(i)).replace( QChar('.'), "," );
        if(itColumn == headerRow.end())
        {
          headerRow.push_back(featureName);
          newRow.push_back(featureQuantity);
        }
        else
        {
          newRow[std::distance(headerRow.begin(), itColumn)] = featureQuantity;
        }

      }    
      rows.push_back(newRow);
    }

    QString clipboardText;
    for(std::vector<QString>::iterator it=headerRow.begin(); it!=headerRow.end(); ++it)
      clipboardText.append(QString("%1 \t").arg(*it));        

    for(std::vector<std::vector<QString> >::iterator it=rows.begin(); it!=rows.end(); ++it)
    {
      clipboardText.append("\n");
      for(std::vector<QString>::iterator it2=(*it).begin(); it2!=(*it).end(); ++it2)
      {
        clipboardText.append(QString("%1 \t").arg(*it2));    
      }
    }

    QApplication::clipboard()->setText( clipboardText, QClipboard::Clipboard);
 
}

void QmitkMeasurement::PlanarFigureSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected )
{
  QModelIndexList indexesOfSelectedRows = deselected.indexes();
  mitk::DataTreeNode* node = 0;

  for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
    ; it != indexesOfSelectedRows.end(); it++)
  {
    node = m_PlanarFiguresModel->GetNode(*it);
    if ( node )
      node->SetBoolProperty("selected", false);
  }

  std::vector<mitk::DataTreeNode::Pointer > nodes;

  indexesOfSelectedRows = selected.indexes();
  if(indexesOfSelectedRows.size() == 0)
    this->setMeasurementInfo("", 0);

  QModelIndex lastIndex;
  int i = 1;
  QmitkRenderWindow* selectedRenderWindow = 0;
  const mitk::PlaneGeometry* _PlaneGeometry = 0;
  mitk::PlanarFigure* _PlanarFigure = 0;

  for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
    ; it != indexesOfSelectedRows.end(); ++it,++i)
  {
    lastIndex = *it;
    node = m_PlanarFiguresModel->GetNode(*it);
    if ( node )
    {
      nodes.push_back(node);
      node->SetBoolProperty("selected", true);
      if(i == indexesOfSelectedRows.size())
      {
        _PlanarFigure = dynamic_cast<mitk::PlanarFigure*>( node->GetData() );
        if(!_PlanarFigure)
          continue;
        _PlaneGeometry = dynamic_cast<const mitk::PlaneGeometry*>( _PlanarFigure->GetGeometry2D() );
        if(!_PlaneGeometry)
          continue;

        QmitkRenderWindow* RenderWindow1 = this->GetActiveStdMultiWidget()->GetRenderWindow1();
        QmitkRenderWindow* RenderWindow2 = this->GetActiveStdMultiWidget()->GetRenderWindow2();
        QmitkRenderWindow* RenderWindow3 = this->GetActiveStdMultiWidget()->GetRenderWindow3();
        QmitkRenderWindow* RenderWindow4 = this->GetActiveStdMultiWidget()->GetRenderWindow4();
        bool PlanarFigureInitializedWindow = false;

        // find initialized renderwindow
        if(node->GetBoolProperty("PlanarFigureInitializedWindow", PlanarFigureInitializedWindow
            , RenderWindow1->GetRenderer()))
          selectedRenderWindow = RenderWindow1;
        if(!selectedRenderWindow && node->GetBoolProperty("PlanarFigureInitializedWindow", PlanarFigureInitializedWindow
            , RenderWindow2->GetRenderer()))
          selectedRenderWindow = RenderWindow2;
        if(!selectedRenderWindow && node->GetBoolProperty("PlanarFigureInitializedWindow", PlanarFigureInitializedWindow
            , RenderWindow3->GetRenderer()))
          selectedRenderWindow = RenderWindow3;
        if(!selectedRenderWindow && node->GetBoolProperty("PlanarFigureInitializedWindow", PlanarFigureInitializedWindow
            , RenderWindow4->GetRenderer()))
          selectedRenderWindow = RenderWindow4;

        // make node visible
        if(selectedRenderWindow)
        {
          mitk::Point3D centerP = _PlaneGeometry->GetOrigin();
          //selectedRenderWindow->GetSliceNavigationController()->ReorientSlices( centerP,_PlaneGeometry->GetNormal() );
          selectedRenderWindow->GetSliceNavigationController()->SelectSliceByPoint(centerP);
        }
      }
    }

    m_SelectionProvider->SetSelection( cherry::ISelection::Pointer(new mitk::DataTreeNodeSelection(nodes)) );
  }

  if(lastIndex.isValid() && selectedRenderWindow && node && _PlaneGeometry)
  {
    // now paint infos also on renderwindow
    QString measurementInfo = (m_PlanarFiguresModel->data(lastIndex, Qt::DisplayRole)).toString();
    measurementInfo.prepend(": ");
    measurementInfo.prepend(QString::fromStdString(node->GetName()));
    this->setMeasurementInfo(measurementInfo, selectedRenderWindow);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}

void QmitkMeasurement::setMeasurementInfo(const QString& text, QmitkRenderWindow* _RenderWindow)
{
  static QmitkRenderWindow* lastRenderWindow = _RenderWindow;
  if(!text.isEmpty())
  {
    m_MeasurementInfoAnnotation->SetText(1, text.toLatin1().data());
    mitk::VtkLayerController::GetInstance(_RenderWindow->GetRenderWindow())->InsertForegroundRenderer(m_MeasurementInfoRenderer,true);
  }
  else
  {
    mitk::VtkLayerController::GetInstance(lastRenderWindow->GetRenderWindow())->RemoveRenderer(m_MeasurementInfoRenderer);
  }
}
