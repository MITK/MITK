#include "QmitkPropertiesTableEditor.h"

#include "QmitkPropertiesTableModel.h"
#include "QmitkPropertyDelegate.h"

#include "mitkBaseRenderer.h"
#include "mitkFocusManager.h"
#include "mitkGlobalInteraction.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QCheckBox>
#include <QComboBox>

#include <vtkRenderWindow.h>

QmitkPropertiesTableEditor::QmitkPropertiesTableEditor(mitk::DataTreeNode::Pointer _Node, QWidget* parent
                                                       , Qt::WindowFlags f)
: QWidget(parent, f)
, m_SelectedNode(0)
, m_NodePropertiesTableView(0)
, m_Model(0)
{
  // set up empty gui elements
  this->init();

  // set up model
  m_Model = new QmitkPropertiesTableModel(0, m_NodePropertiesTableView);    
  m_NodePropertiesTableView->setModel(m_Model);
}

QmitkPropertiesTableEditor::~QmitkPropertiesTableEditor()
{
}

void QmitkPropertiesTableEditor::setNode(mitk::DataTreeNode* _Node)
{
  m_SelectedNode = _Node;
  if(m_SelectedNode)
  {
    m_Model->setPropertyList(m_SelectedNode->GetPropertyList());
    m_NodePropertiesTableView->resizeRowsToContents();

    // refill the combo boxes
    m_ComboRenderer->clear();

    for(mitk::BaseRenderer::BaseRendererMapType::iterator mapit = mitk::BaseRenderer::baseRendererMap.begin(); 
      mapit != mitk::BaseRenderer::baseRendererMap.end(); mapit++)
    {
      if((*mapit).second->GetName())
      {
        std::string winName((*mapit).second->GetName());
        //  winName.erase(0,winName.find("::")+2);
        m_ComboRenderer->insertItem(winName.c_str());
      }
    }

    // try to select focused RenderWindow
    vtkRenderWindow* focusedRenderWindow = NULL;

    mitk::FocusManager* fm =
      mitk::GlobalInteraction::GetInstance()->GetFocusManager();
    mitk::BaseRenderer::ConstPointer br = fm->GetFocused();
    if (br.IsNotNull())
    {
      focusedRenderWindow = br->GetRenderWindow();
    }

    if (focusedRenderWindow)
    {

      m_ComboRenderer->setCurrentText(mitk::BaseRenderer::GetInstance(focusedRenderWindow)->GetName());
    }
    else
    {
      m_ComboRenderer->setCurrentText("no focused window");
    }
  }
  else
  {
    m_Model->setPropertyList(0);
    m_ComboRenderer->clear();
  }
}

QmitkPropertiesTableModel* QmitkPropertiesTableEditor::getModel() const
{
  return m_Model;
}

void QmitkPropertiesTableEditor::init()
{
  // read
  QVBoxLayout* _NodePropertiesLayout = new QVBoxLayout;
  //QWidget* _PaneRendererSelection = new QWidget(this);
  //QVBoxLayout* _RendererSelectionLayout = new QVBoxLayout;
  m_ChkShowRenderProperties = new QCheckBox("Show Renderer Properties", QWidget::parentWidget());
  m_ComboRenderer = new QComboBox(QWidget::parentWidget());
  m_NodePropertiesTableView = new QTableView(QWidget::parentWidget());
  
  // write
  _NodePropertiesLayout->setMargin(0);
  setLayout(_NodePropertiesLayout);
  
  //_NodePropertiesLayout->addWidget(_PaneRendererSelection);
  _NodePropertiesLayout->addWidget(m_ChkShowRenderProperties);
  _NodePropertiesLayout->addWidget(m_ComboRenderer);
  _NodePropertiesLayout->addWidget(m_NodePropertiesTableView);

// 
//   _PaneRendererSelection->setLayout(_RendererSelectionLayout);
//   _RendererSelectionLayout->addWidget(m_ChkShowRenderProperties);
//   _RendererSelectionLayout->addWidget(m_ComboRenderer);

  m_ChkShowRenderProperties->setChecked(false);

  m_ComboRenderer->setEditable(false);
  m_ComboRenderer->setEnabled(false);
  
  m_NodePropertiesTableView->setSelectionMode( QAbstractItemView::SingleSelection );
  m_NodePropertiesTableView->setSelectionBehavior( QAbstractItemView::SelectItems );
  m_NodePropertiesTableView->verticalHeader()->hide();
  m_NodePropertiesTableView->setItemDelegate(new QmitkPropertyDelegate(this));
  m_NodePropertiesTableView->setAlternatingRowColors(true);
  m_NodePropertiesTableView->horizontalHeader()->setStretchLastSection(true);
  m_NodePropertiesTableView->setSortingEnabled(true);
  //m_NodePropertiesTableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
  //m_NodePropertiesTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  m_NodePropertiesTableView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  QObject::connect( m_ChkShowRenderProperties, SIGNAL( toggled(bool) )
    , this, SLOT( ChkShowRenderPropertiesToggled(bool) ) );

  QObject::connect( m_ComboRenderer, SIGNAL( currentIndexChanged(int) )
    , this, SLOT( ComboRendererCurrentIndexChanged(int) ) );

//   QObject::connect( m_NodePropertiesTableView->horizontalHeader(), SIGNAL( sectionClicked(int) )
//     , m_NodePropertiesTableView, SLOT( sortByColumn(int) ) );
}

void QmitkPropertiesTableEditor::ChkShowRenderPropertiesToggled( bool checked )
{
  m_ComboRenderer->setEnabled(checked);
  if(checked) {
    ComboRendererCurrentIndexChanged( m_ComboRenderer->currentIndex() );
  } else {
    if (m_SelectedNode != NULL) { 
      m_Model->setPropertyList( m_SelectedNode->GetPropertyList() );
    } else {
      m_Model->setPropertyList( NULL );
    }
  }
}

void QmitkPropertiesTableEditor::ComboRendererCurrentIndexChanged( int index )
{
  if ( m_SelectedNode )
  {
    if  (m_ComboRenderer->isEnabled())
    {
      QString rendererName = m_ComboRenderer->currentText();
      if(rendererName.isEmpty() == false)
      {
        vtkRenderWindow* renWin =  mitk::BaseRenderer::GetRenderWindowByName(rendererName.toAscii().data());

        if (renWin)
        {
          m_Model->setPropertyList(m_SelectedNode->GetPropertyList(mitk::BaseRenderer::GetInstance(renWin)) );
        }
      }
    }
    m_NodePropertiesTableView->resizeRowsToContents();
  }
}