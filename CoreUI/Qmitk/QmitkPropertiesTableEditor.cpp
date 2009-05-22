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
#include <QLineEdit>
#include <QLabel>

#include <vtkRenderWindow.h>

QmitkPropertiesTableEditor::QmitkPropertiesTableEditor(mitk::DataTreeNode::Pointer _Node, QWidget* parent
                                                       , Qt::WindowFlags f)
: QWidget(parent, f)
, m_NodePropertiesTableView(0)
, m_Model(0)
{
  // set up empty gui elements
  this->init();

  // set up model
  m_Model = new QmitkPropertiesTableModel(m_NodePropertiesTableView, 0);    
  m_NodePropertiesTableView->setModel(m_Model);
}

QmitkPropertiesTableEditor::~QmitkPropertiesTableEditor()
{
}

void QmitkPropertiesTableEditor::SetPropertyList( mitk::PropertyList::Pointer _List )
{
  if(_List.IsNotNull())
  {
    m_Model->SetPropertyList(_List);
    m_NodePropertiesTableView->resizeColumnsToContents();
    m_NodePropertiesTableView->resizeRowsToContents();
    m_NodePropertiesTableView->horizontalHeader()->setStretchLastSection(true);

/*
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
*/

  }
  else
  {
    m_Model->SetPropertyList(0);
  }
}

QmitkPropertiesTableModel* QmitkPropertiesTableEditor::getModel() const
{
  return m_Model;
}

void QmitkPropertiesTableEditor::init()
{
  // read/ dim
  QVBoxLayout* _NodePropertiesLayout = new QVBoxLayout;
  QWidget* _PropertyFilterKeyWordPane = new QWidget(QWidget::parentWidget());
  QHBoxLayout* _PropertyFilterKeyWordLayout = new QHBoxLayout;
  QLabel* _LabelPropertyFilterKeyWord = new QLabel("Filter: ",_PropertyFilterKeyWordPane);
  m_TxtPropertyFilterKeyWord = new QLineEdit(_PropertyFilterKeyWordPane);
  m_NodePropertiesTableView = new QTableView(QWidget::parentWidget());

  // write  
  setLayout(_NodePropertiesLayout);

  _PropertyFilterKeyWordPane->setLayout(_PropertyFilterKeyWordLayout);
  
  _PropertyFilterKeyWordLayout->setMargin(0);
  _PropertyFilterKeyWordLayout->addWidget(_LabelPropertyFilterKeyWord);
  _PropertyFilterKeyWordLayout->addWidget(m_TxtPropertyFilterKeyWord);

  _NodePropertiesLayout->setMargin(0);
  _NodePropertiesLayout->addWidget(_PropertyFilterKeyWordPane);
  _NodePropertiesLayout->addWidget(m_NodePropertiesTableView);

  m_NodePropertiesTableView->setSelectionMode( QAbstractItemView::SingleSelection );
  m_NodePropertiesTableView->setSelectionBehavior( QAbstractItemView::SelectItems );
  m_NodePropertiesTableView->verticalHeader()->hide();
  m_NodePropertiesTableView->setItemDelegate(new QmitkPropertyDelegate(this));
  m_NodePropertiesTableView->setAlternatingRowColors(true);
  //m_NodePropertiesTableView->horizontalHeader()->setStretchLastSection(true);
  m_NodePropertiesTableView->setSortingEnabled(true);
  m_NodePropertiesTableView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  QObject::connect( m_TxtPropertyFilterKeyWord, SIGNAL( editingFinished() )
    , this, SLOT( PropertyFilterKeyWordEditingFinished() ) );

}

void QmitkPropertiesTableEditor::PropertyFilterKeyWordEditingFinished()
{
  m_Model->SetFilterPropertiesKeyWord(m_TxtPropertyFilterKeyWord->text().toStdString());
}

/*
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
*/
