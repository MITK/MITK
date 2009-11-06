
#include <QmitkDnDFrameWidget.h>
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
  
  bool dsmodified = false;
  for (QList<QUrl>::Iterator fileName = fileNames.begin();
    fileName != fileNames.end(); ++fileName)
  {
    mitk::DataTreeNodeFactory::Pointer nodeReader = mitk::DataTreeNodeFactory::New();
    try
    {
      nodeReader->SetFileName(fileName->toLocalFile().toLatin1().data());
      nodeReader->Update();
      for ( unsigned int i = 0 ; i < nodeReader->GetNumberOfOutputs( ); ++i )
      {
        mitk::DataTreeNode::Pointer node;
        node = nodeReader->GetOutput(i);
        if ( node->GetData() != NULL )
        {  
          ds->Add(node);
          dsmodified = true;
        }
      }
    }
    catch(...)
    {

    }
  }
  if(dsmodified)
  {
    mitk::RenderingManager::GetInstance()->InitializeViews(ds->ComputeBoundingGeometry3D());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}