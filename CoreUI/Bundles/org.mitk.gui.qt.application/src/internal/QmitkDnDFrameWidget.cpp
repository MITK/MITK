
#include <internal/QmitkDnDFrameWidget.h>
#include <QtGui>


// cherry Includes
#include <cherryPlatform.h>

#include <mitkDataTreeNodeFactory.h>
#include "mitkIDataStorageService.h"
#include "mitkDataStorageEditorInput.h"
#include "mitkRenderingManager.h"

QmitkDnDFrameWidget::QmitkDnDFrameWidget(QWidget *parent)
  : QWidget(parent)
{
   setAcceptDrops(true);
}

void QmitkDnDFrameWidget::dragEnterEvent( QDragEnterEvent *event )
{   // accept drags
  event->accept();
}
void QmitkDnDFrameWidget::dropEvent( QDropEvent * event )
{ //open dragged files
  
  mitk::IDataStorageService::Pointer service = 
    cherry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);
  
  mitk::DataStorage::Pointer ds;
  if (service.IsNotNull())
    ds = service->GetActiveDataStorage()->GetDataStorage();
  else
    return;
  
  QList<QUrl> fileNames = event->mimeData()->urls();
  mitk::DataTreeNode::Pointer node;
  for (QList<QUrl>::Iterator fileName = fileNames.begin();
    fileName != fileNames.end(); ++fileName)
  {
    mitk::DataTreeNodeFactory::Pointer nodeReader = mitk::DataTreeNodeFactory::New();
    try
    {
      nodeReader->SetFileName(fileName->toLocalFile().toLatin1().data());
      nodeReader->Update();
      node = nodeReader->GetOutput();
      ds->Add(node);
    }
    catch(...)
    {

    }
  }
  if(node.IsNotNull() && node->GetData() != 0)
  {
    mitk::RenderingManager::GetInstance()->InitializeViews(ds->ComputeBoundingGeometry3D());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}