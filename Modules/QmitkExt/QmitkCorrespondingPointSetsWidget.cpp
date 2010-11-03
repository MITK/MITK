#include "QmitkCorrespondingPointSetsWidget.h"
#include <mitkGlobalInteraction.h>
#include <mitkPointSetReader.h>
#include <mitkPointSetWriter.h>

#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#include "btnLoad.xpm"
#include "btnSave.xpm"
#include "btnClear.xpm"
#include "btnSetPoints.xpm"
#include "btnUp.xpm"
#include "btnDown.xpm"



QmitkCorrespondingPointSetsWidget::QmitkCorrespondingPointSetsWidget(QWidget *parent, int orientation):
  QWidget(parent), 
  m_CorrespondingPointSetsView(NULL),
  m_MultiWidget(NULL),
  m_Orientation(0),
  m_ToggleAddPoint(NULL),
  m_Interactor(NULL),
  m_ReferenceInteractor(NULL),
  m_TimeStep(0)
{
  m_CorrespondingPointSetsView = new QmitkCorrespondingPointSetsView();

  if(orientation != 0)
      m_Orientation = orientation;

  SetupUi();
  connect(this->m_ToggleAddPoint, SIGNAL(toggled(bool)), this, SLOT(OnBtnAddPoint()));
}

QmitkCorrespondingPointSetsWidget::~QmitkCorrespondingPointSetsWidget()
{
  m_MultiWidget = NULL;
  delete m_CorrespondingPointSetsView;
}


void QmitkCorrespondingPointSetsWidget::SetupUi()
{
    //Setup the buttons

    m_ToggleAddPoint = new QPushButton();//iconSetPoints, "", this);
    m_ToggleAddPoint->setMaximumSize(25,25);
    m_ToggleAddPoint->setCheckable(true);
    m_ToggleAddPoint->setToolTip("Toggle point editing (use SHIFT  + Left Mouse Button to add Points)");
    QIcon iconAdd(btnSetPoints_xpm);
    m_ToggleAddPoint->setIcon(iconAdd);

    int i;

      QBoxLayout* lay1;
      QBoxLayout* lay2;

      switch (m_Orientation)
      {
      case 0:
          lay1 = new QVBoxLayout(this);
          lay2 = new QHBoxLayout();
          i = 0;
          break;

      case 1:
          lay1 = new QHBoxLayout(this);
          lay2 = new QVBoxLayout();
          i=-1;
          break;

      case 2:
          lay1 = new QHBoxLayout(this);
          lay2 = new QVBoxLayout();
          i=0;
          break;

      default:
          lay1 = new QVBoxLayout(this);
          lay2 = new QHBoxLayout();
          i=-1;
          break;

      }

      //setup Layouts

      this->setLayout(lay1);
      lay1->addLayout(lay2);

      lay2->stretch(true);
      lay2->addWidget(m_ToggleAddPoint);

      lay1->insertWidget(i,m_CorrespondingPointSetsView);
      this->setLayout(lay1);
}
void QmitkCorrespondingPointSetsWidget::SetPointSetNodes(std::vector<mitk::DataNode*> nodes)
{
  this->m_CorrespondingPointSetsView->SetPointSetNodes(nodes);
}
void QmitkCorrespondingPointSetsWidget::NodeRemoved( const mitk::DataNode* removedNode)
{
  this->m_CorrespondingPointSetsView->NodeRemoved(removedNode);
}
std::vector<mitk::DataNode*> QmitkCorrespondingPointSetsWidget::GetPointSetNodes()
{
  return this->m_CorrespondingPointSetsView->GetPointSetNodes();
}
void QmitkCorrespondingPointSetsWidget::SetMultiWidget(QmitkStdMultiWidget *multiWidget)
{
    this->m_MultiWidget = multiWidget;
    m_CorrespondingPointSetsView->SetMultiWidget(multiWidget);
}
void QmitkCorrespondingPointSetsWidget::OnBtnAddPoint()
{
  std::vector<mitk::DataNode*> pointSetNodes = GetPointSetNodes();
  std::vector<mitk::DataNode*>::iterator it;
  for ( it = pointSetNodes.begin(); it < pointSetNodes.end(); it++ )
  {
    mitk::DataNode* dataNode = dynamic_cast<mitk::DataNode*>(*it);
    bool selected = false;
    dataNode->GetPropertyValue<bool>("selected", selected);

    if (selected) {
      this->UpdateSelection(dataNode);
      break;
    }
  }
}
void QmitkCorrespondingPointSetsWidget::UpdateSelection(mitk::DataNode* selectedNode)
{
  if (m_Interactor){
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_Interactor );
    m_Interactor = NULL;
  }

  if ( this->m_ToggleAddPoint->isChecked() && selectedNode )
  {
    bool visible = false;
    selectedNode->GetPropertyValue<bool>("visible", visible);

    if (visible){
      // set new interactor
      m_Interactor = dynamic_cast<mitk::PointSetInteractor*>(selectedNode->GetInteractor());

      if (m_Interactor.IsNull())//if not present, instanciate one
        m_Interactor = mitk::PointSetInteractor::New("pointsetinteractor", selectedNode);
      
      //add it to global interaction to activate it
      mitk::GlobalInteraction::GetInstance()->AddInteractor( m_Interactor );
    }
  }
}