#include "QmitkToolReferenceDataSelectionBox.h"
#include "QmitkDataTreeComboBox.h"
#include "QmitkPropertyListPopup.h"

#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateDimension.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateNOT.h"

#include "mitkRenderingManager.h"

#include <itkCommand.h>
 

QmitkToolReferenceDataSelectionBox::QmitkToolReferenceDataSelectionBox(QWidget* parent, const char* name)
:QVBox(parent, name),
 m_SelfCall(false)
{
  m_ToolManager = mitk::ToolManager::New();

  m_ReferenceDataSelectionBox = new QmitkDataTreeComboBox( this );
  

  connect( m_ReferenceDataSelectionBox, SIGNAL(activated(const mitk::DataTreeFilter::Item*)),
           this, SLOT(OnReferenceDataSelected(const mitk::DataTreeFilter::Item*)) );

  {
  itk::ReceptorMemberCommand<QmitkToolReferenceDataSelectionBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolReferenceDataSelectionBox>::New();
  command->SetCallbackFunction( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolReferenceDataChangedObserverTag = m_ToolManager->AddObserver( mitk::ToolReferenceDataChangedEvent(), command );
  }


}

QmitkToolReferenceDataSelectionBox::~QmitkToolReferenceDataSelectionBox()
{
}

void QmitkToolReferenceDataSelectionBox::Initialize(mitk::DataTreeBase* tree )
{
  m_ReferenceDataSelectionBox->SetDataTree( tree );
  
  UpdateDataDisplay();
}
    
void QmitkToolReferenceDataSelectionBox::SetToolGroups(const char* toolGroups)
{
  mitk::ToolManager::Pointer newManager = mitk::ToolManager::New(toolGroups);
  this->SetToolManager( *newManager );
}

mitk::ToolManager* QmitkToolReferenceDataSelectionBox::GetToolManager()
{
  return m_ToolManager;
}

void QmitkToolReferenceDataSelectionBox::SetToolManager(mitk::ToolManager& newManager) // no NULL pointer allowed here, a manager is required
{
  m_ToolManager->RemoveObserver( m_ToolReferenceDataChangedObserverTag );

  m_ToolManager = &newManager;

  {
  itk::ReceptorMemberCommand<QmitkToolReferenceDataSelectionBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolReferenceDataSelectionBox>::New();
  command->SetCallbackFunction( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolReferenceDataChangedObserverTag = m_ToolManager->AddObserver( mitk::ToolReferenceDataChangedEvent(), command );
  }

  UpdateDataDisplay();
}
     
void QmitkToolReferenceDataSelectionBox::UpdateDataDisplay()
{ 
  m_ReferenceDataSelectionBox->GetFilter()->SetDataStorageResultset( GetAllPossibleReferenceImages() ); /// \todo Also forward the current selected. Perhaps wait for new combobox.
}

void QmitkToolReferenceDataSelectionBox::OnReferenceDataSelected(const mitk::DataTreeFilter::Item* item)
{
  mitk::DataTreeNode* selectedNode = const_cast<mitk::DataTreeNode*>( item->GetNode() );

  emit ReferenceNodeSelected(selectedNode);

  mitk::DataStorage::SetOfObjects::ConstPointer allImageNodes = GetAllPossibleReferenceImages();
  for ( mitk::DataStorage::SetOfObjects::const_iterator nodeIter = allImageNodes->begin();
        nodeIter != allImageNodes->end();
        ++nodeIter )
  {
    mitk::DataTreeNode* currentNode = (*nodeIter).GetPointer();

    currentNode->SetVisibility( currentNode == selectedNode ); // only the selected one is visible, everything else is invisible
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();


  m_SelfCall = true;
  m_ToolManager->SetReferenceData(selectedNode); // maybe NULL
  m_SelfCall = false;
}

void QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified(const itk::EventObject&)
{
  if (m_SelfCall) return;

  const mitk::DataTreeNode* node = m_ToolManager->GetReferenceData(0);
  emit ReferenceNodeSelected(node);

  UpdateDataDisplay();
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkToolReferenceDataSelectionBox::GetAllPossibleReferenceImages()
{
  // update reference images
  mitk::NodePredicateDataType images("Image");
  mitk::NodePredicateDimension dim3(3);
  mitk::NodePredicateAND image3D( images, dim3 );

  mitk::NodePredicateProperty binary("binary", new mitk::BoolProperty(true));
  mitk::NodePredicateNOT notBinary( binary );

  mitk::NodePredicateProperty segmentation("segmentation", new mitk::BoolProperty(true));
  mitk::NodePredicateNOT notSegmentation( segmentation );
  
  mitk::NodePredicateAND imageColorful( notBinary, notSegmentation );
  
  mitk::NodePredicateAND completePredicate( image3D, imageColorful );

  mitk::DataStorage::SetOfObjects::ConstPointer sceneImages = mitk::DataStorage::GetInstance()->GetSubset( completePredicate );
  return sceneImages;
}

