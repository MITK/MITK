@PLUGIN_COPYRIGHT@

#include "@VIEW_CLASS_H@"

#include "mitkNodePredicateDataType.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include <qmessagebox.h>

@BEGIN_NAMESPACE@

const std::string @VIEW_CLASS@::VIEW_ID = "@VIEW_ID@";

@VIEW_CLASS@::@VIEW_CLASS@()
: @VIEW_BASE_CLASS@(), 
  m_Controls(NULL),
  m_MultiWidget(NULL)
{
}

@VIEW_CLASS@::~@VIEW_CLASS@()
{
}

void @VIEW_CLASS@::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::@VIEW_CONTROLS_CLASS@;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    // define data type for combobox
    m_Controls->m_ImageSelector->SetDataStorage( this->GetDefaultDataStorage() );
    m_Controls->m_ImageSelector->SetPredicate( new mitk::NodePredicateDataType("Image") );
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

void @VIEW_CLASS@::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_Button), SIGNAL(clicked()), this, SLOT(DoSomething()) );
  }
}

void @VIEW_CLASS@::Activated()
{
  QmitkFunctionality::Activated();
}

void @VIEW_CLASS@::Deactivated()
{
  QmitkFunctionality::Activated();
}

void @VIEW_CLASS@::DoSomething()
{
  mitk::DataTreeNode* node = m_Controls->m_ImageSelector->GetSelectedNode();
  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Template functionality", "Please load and select an image before starting some action.");
    return;
  }

  // here we have a valid mitk::DataTreeNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is really an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>( data );
    if (image)
    {
      std::string name("(no name)");
      node->GetName(name);

      QMessageBox::information( NULL, "Image processing",
                                QString( "Doing something to '%1'" ).arg(name.c_str()) );

      // at this point anything can be done using the mitk::Image image.
    }
  }
}

@END_NAMESPACE@
