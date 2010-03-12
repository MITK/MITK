
#include <QmitkExtDnDFrameWidget.h>
#include <QtGui>


// berry Includes
#include <berryPlatform.h>

#include <mitkDataNodeFactory.h>
#include "mitkIDataStorageService.h"
#include "mitkDataStorageEditorInput.h"
#include "mitkRenderingManager.h"

#include "mitkProperties.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNOT.h"
#include "mitkNodePredicateProperty.h"


#include "mitkSceneIO.h"
#include "mitkProgressBar.h"

QmitkExtDnDFrameWidget::QmitkExtDnDFrameWidget(QWidget *parent)
  : QWidget(parent)
{
   setAcceptDrops(true);
}

void QmitkExtDnDFrameWidget::dragEnterEvent( QDragEnterEvent *event )
{   // accept drags
  event->accept();
}
void QmitkExtDnDFrameWidget::dropEvent( QDropEvent * event )
{ //open dragged files
  
  mitk::IDataStorageService::Pointer service = 
    berry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);
  
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
    if ( fileName->toLocalFile().right(5) == ".mitk" ) 
    {
      mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

      bool clearDataStorageFirst(false);
      mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
      ds = sceneIO->LoadScene( fileName->toLocalFile().toLocal8Bit().constData(), ds, clearDataStorageFirst );
      dsmodified = true;
      mitk::ProgressBar::GetInstance()->Progress(2);
    }
    else
    {
      mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
      try
      {
        nodeReader->SetFileName(fileName->toLocalFile().toLatin1().data());
        nodeReader->Update();
        for ( unsigned int i = 0 ; i < nodeReader->GetNumberOfOutputs( ); ++i )
        {
          mitk::DataNode::Pointer node;
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
  }
  
  if(dsmodified)
  {
    // get all nodes that have not set "includeInBoundingBox" to false
    mitk::NodePredicateNOT::Pointer pred 
      = mitk::NodePredicateNOT::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
      , mitk::BoolProperty::New(false)));

    mitk::DataStorage::SetOfObjects::ConstPointer rs = ds->GetSubset(pred);
    // calculate bounding geometry of these nodes
    mitk::TimeSlicedGeometry::Pointer bounds = ds->ComputeBoundingGeometry3D(rs);
    // initialize the views to the bounding geometry
    mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
  }
}