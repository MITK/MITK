#include "QmitkHelloWorldView.h"

#include <QDockWidget>
#include <QVBoxLayout>
#include <QAbstractItemView>
#include <QMessageBox>

#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageReference.h>

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateAND.h>
#include <mitkNodePredicateNOT.h>
#include <mitkNodePredicateProperty.h>

#include <mitkProperties.h>

#include <QmitkStdMultiWidget.h>
#include <QmitkDataStorageComboBox.h>
#include <QmitkDataStorageListModel.h>

#include <QmitkStdMultiWidgetEditor.h>
#include <QmitkDataStorageComboBox.h>

#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>

#include "cherryPlatform.h"
#include "cherryPlatformException.h"
#include "service/cherryIExtensionPointService.h"
#include "IDialog.h"
using namespace cherry;

void QmitkHelloWorldView::CreateQtPartControl(QWidget* parent)
{
  QVBoxLayout* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0,0,0,0);
  m_Parent = parent;
  m_ButtonHelloWorld = new QPushButton("Hello World", m_Parent);
  m_DataComboBox = new QmitkDataStorageComboBox(m_Parent, true);
  mitk::BoolProperty::Pointer myProp = mitk::BoolProperty::New(true);
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", myProp);
  mitk::NodePredicateNOT::Pointer notBinary = mitk::NodePredicateNOT::New(isBinary); 
  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image"); 
  mitk::NodePredicateAND::Pointer completePredicate = mitk::NodePredicateAND::New( notBinary, isImage );
  m_DataComboBox->SetPredicate(  completePredicate  );

  QObject::connect(m_ButtonHelloWorld, SIGNAL(clicked()),
         this, SLOT(buttonClicked()));
  layout->addWidget(m_ButtonHelloWorld);
  layout->addWidget(m_DataComboBox);
}


void QmitkHelloWorldView::StdMultiWidgetAvailable()
{
  m_ButtonHelloWorld->setEnabled(true);
}

void QmitkHelloWorldView::StdMultiWidgetNotAvailable()
{
  m_ButtonHelloWorld->setEnabled(false);
}

void QmitkHelloWorldView::SetFocus()
{

}

void QmitkHelloWorldView::buttonClicked()
{
  IExtensionPointService::Pointer service = Platform::GetExtensionPointService();
  IConfigurationElement::vector ces(
    service->GetConfigurationElementsFor("org.mitk.gui.qt.helloworld.dialogs"));

  IDialog* _Dialog = 0;
  for (IConfigurationElement::vector::iterator i= ces.begin(); i != ces.end(); ++i)
  {
    std::string cid;
    if ((*i)->GetAttribute("id", cid))
    {
      if (cid == "helloworlddialog")
      {
        _Dialog = (*i)->CreateExecutableExtension<IDialog>("class");
        _Dialog->show();
        break;
      }
    }
  }
}

QmitkHelloWorldView::~QmitkHelloWorldView()
{
}