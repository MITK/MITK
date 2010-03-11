@PLUGIN_COPYRIGHT@

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "@VIEW_CLASS_H@"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>

@BEGIN_NAMESPACE@

const std::string @VIEW_CLASS@::VIEW_ID = "@VIEW_ID@";

@VIEW_CLASS@::@VIEW_CLASS@()
: @VIEW_BASE_CLASS@(),
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
}

@VIEW_CLASS@::~@VIEW_CLASS@()
{
}


void @VIEW_CLASS@::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::@VIEW_CLASS@Controls;
    m_Controls->setupUi( parent );
 
    connect( m_Controls->btnPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );
  }
}


void @VIEW_CLASS@::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void @VIEW_CLASS@::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}


void @VIEW_CLASS@::OnSelectionChanged( std::vector<mitk::DataTreeNode*> nodes )
{ 
  // iterate all selected objects, adjust warning visibility
  for( std::vector<mitk::DataTreeNode*>::iterator it = nodes.begin();
       it != nodes.end();
       ++it )
  {
    mitk::DataTreeNode::Pointer node = *it;
  
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls->lblWarning->setVisible( false );
      return;
    }
  }

  m_Controls->lblWarning->setVisible( true );
}


void @VIEW_CLASS@::DoImageProcessing()
{
  std::vector<mitk::DataTreeNode*> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataTreeNode* node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Template", "Please load and select an image before starting image processing.");
    return;
  }

  // here we have a valid mitk::DataTreeNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>( data );
    if (image)
    {
      std::stringstream message;
      std::string name;
      message << "Performing image processing for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataTreeNode
        message << "'" << name << "'";
      }
      message << ".";
      MITK_INFO << message.str();

      // TODO actually do something here...
    }
  }
}

@END_NAMESPACE@

