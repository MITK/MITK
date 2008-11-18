#include "QmitkHelloWorldView.h"

#include <QPushButton>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QAbstractItemView>

#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageReference.h>

#include <mitkNodePredicateDataType.h>

#include <QmitkStdMultiWidget.h>
#include <QmitkDataTreeComboBox.h>
#include <QmitkDataStorageListModel.h>

#include <QmitkStdMultiWidgetEditor.h>

#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>

void QmitkHelloWorldView::CreateQtPartControl(QWidget* parent)
{
  QVBoxLayout* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0,0,0,0);

  layout->addWidget(new QPushButton("Hello World", parent));
}

void QmitkHelloWorldView::SetFocus()
{

}

QmitkHelloWorldView::~QmitkHelloWorldView()
{
}
