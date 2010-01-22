/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include <qstring.h>
#include <qinputdialog.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>

#include <mitkRenderingManager.h>
#include <mitkPointSetWriter.h>
#include <mitkPointSetReader.h>

#include <QmitkCommonFunctionality.h>

#include <itksys/SystemTools.hxx>


void QmitkPointSetInteractionControls::init()
{
    m_TreeNodeSelector->m_FilterFunction = IsPointSet;
    m_PointListWidget->ShowPointSetActionButtons(true);
}



void QmitkPointSetInteractionControls::SetDataTreeNodeIterator( mitk::DataTreeIteratorClone it )
{
    m_DataTreeIteratorClone = it;
    m_TreeNodeSelector->SetDataTreeNodeIterator( it );
}



void QmitkPointSetInteractionControls::DataTreeNodeSelected( mitk::DataTreeNode * /* node */ )
{
    emit DataTreeNodeSelected();
}



bool QmitkPointSetInteractionControls::IsPointSet( mitk::DataTreeNode * node )
{
    if ( node == NULL )
      return false;
    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>( node->GetData() );
    if ( pointSet.IsNotNull() )
      return true;
    else
      return false;
}



void QmitkPointSetInteractionControls::OnAddPointSetClicked()
{
  //Ask for the name of the point set
  //Ask for the maximal number of points in the pointset
  bool ok = false;
  QString name = QInputDialog::getText( "Add point set...", "Enter name for the new point set", QLineEdit::Normal, "PointSet", &ok );
  if ( ! ok || name.isEmpty() )
    return;

  //
  //Create a new empty pointset
  //
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  //
  // Create a new data tree node 
  //
  mitk::DataTreeNode::Pointer pointSetNode = mitk::DataTreeNode::New();

  //
  // fill the data tree node with the appropriate information
  //
  pointSetNode->SetData( pointSet );
  pointSetNode->SetProperty( "name", mitk::StringProperty::New( name.latin1() ) );
  pointSetNode->SetProperty( "layer", mitk::IntProperty::New( 1 ) );
  pointSetNode->SetProperty( "opacity", mitk::FloatProperty::New( 1 ) );
  pointSetNode->SetColor( 1.0, 1.0, 0.0 );

  //
  // add the node to the data tree
  //
  m_DataTreeIteratorClone->Add( pointSetNode );
  m_TreeNodeSelector->SelectNode( pointSetNode );

  // the point set is automatically shown in the
  // point list widget due to signal /slot connections
  // from the TreeNodeSelector/Activated signal to OnCurrentPointSetActivated
}
 

void QmitkPointSetInteractionControls::OnRemoveCurrentPointSetClicked()
{
  //Ask if the user really wants to delete the current point set (by the name of the point set)
  //unregister its interactor
  //delete the point set from the data tree
  mitk::DataTreeNode::Pointer currentPointSetNode = this->GetCurrentPointSetNode();
  mitk::PointSet::Pointer currentPointSet = this->GetCurrentPointSet();
  if ( currentPointSetNode.IsNull() || currentPointSet.IsNull() )
  {
    QMessageBox::warning( this, "Remove current point set", "A valid point set has to be selected first..." );
    return;
  }

  switch( QMessageBox::question(this, "Remove current pointset...", "Do you really want do delete the current point set?",
           QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape)) 
  {
    case QMessageBox::Yes: 
    {
      mitk::DataTreeIteratorClone selectedIterator = mitk::DataTree::GetIteratorToNode( m_DataTreeIteratorClone->GetTree(), this->GetCurrentPointSetNode() );
      assert( selectedIterator.GetPointer() != NULL );
      selectedIterator.GetPointer()->Remove();
      UpdateTreeNodeSelector();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    default: 
      return;
  }
}


mitk::DataTreeNode* QmitkPointSetInteractionControls::GetCurrentPointSetNode()
{
  return this->GetSelectedNode();
}


mitk::PointSet* QmitkPointSetInteractionControls::GetCurrentPointSet()
{
  if ( GetCurrentPointSetNode() != NULL ) 
    return dynamic_cast<mitk::PointSet*>( GetCurrentPointSetNode()->GetData() );
  else
    return NULL;
}



mitk::DataTreeNode* QmitkPointSetInteractionControls::GetSelectedNode()
{
  return m_TreeNodeSelector->GetSelectedNode();
}



void QmitkPointSetInteractionControls::UpdateTreeNodeSelector()
{
  // if there are no interactable point sets left,
  // deactivate interaction capability of the point list widget.
  if( CommonFunctionality::GetNumberOfNodesOfType<mitk::PointSet>( m_DataTreeIteratorClone ) == 0 )
  {
    m_PointListWidget->SetPointSetNode(NULL);
  }
  m_TreeNodeSelector->UpdateContent();
}



void QmitkPointSetInteractionControls::OnCurrentPointSetActivated( mitk::DataTreeNode* currentNode )
{
  // get the current pointset
  currentNode = this->GetCurrentPointSetNode();
  if (currentNode != NULL)
  {
  mitk::DataTreeNode::Pointer currentPointSetNode = this->GetCurrentPointSetNode();
  mitk::PointSet::Pointer currentPointSet = this->GetCurrentPointSet();

  if ( currentPointSetNode.IsNull() || currentPointSet.IsNull() )
  {
    //QMessageBox::critical( NULL, "Point set interactor", "Pointset, node or interactor were NULL. This should not happen..." );
    return;
  }

  // hide all other pointsets
  this->ExclusivelyShowPointSet( currentPointSetNode );

  // let the point list widget show the current pointset
  ShowPointListInPointListWidget( currentPointSetNode );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkPointSetInteractionControls::ShowAllPointSets()
{
  //
  // traverses the data tree and turns all point sets visible
  //
  mitk::DataTreeIteratorClone it = m_DataTreeIteratorClone;
  while ( !it->IsAtEnd() )
  {
    mitk::DataTreeNode::Pointer node = it->Get();
    if ( node.IsNotNull() )
    {
      if ( node->GetData() != NULL )
      {
        mitk::PointSet::Pointer pointSet = dynamic_cast< mitk::PointSet* >( node->GetData() );
        if ( pointSet.IsNotNull() )
        {
          node->SetVisibility(true);
        }
      }
    }
    ++it;
  }
}

void QmitkPointSetInteractionControls::ExclusivelyShowPointSet( mitk::DataTreeNode* pointSetNode )
{
  //
  // traverses the data tree and hides als point sets except the one
  // which is provided via the given pointSetNode
  //
  mitk::DataTreeIteratorClone it = m_DataTreeIteratorClone;
  while ( it.IsNotNull() && !it->IsAtEnd() )
  {
    //
    // Look if the encountered vessel tree has an interactor associated. If not, create one.
    // Store all interactors in the interactor list m_VesselTreeInteractors
    //
    mitk::DataTreeNode::Pointer node = it->Get();
    if ( node.IsNotNull() )
    {
      if ( node->GetData() != NULL )
      {
        mitk::PointSet::Pointer pointSet = dynamic_cast< mitk::PointSet* >( node->GetData() );
        if ( pointSet.IsNotNull() )
        {
          if ( node == pointSetNode )
            node->SetVisibility(true);
          else
            node->SetVisibility(false);
        }
      }
    }
    ++it;
  }
}



void QmitkPointSetInteractionControls::ShowPointListInPointListWidget( mitk::DataTreeNode* pointSetNode )
{
  mitk::DataTreeNode::Pointer currentPointSetNode = pointSetNode;
  m_PointListWidget->SetPointSetNode(currentPointSetNode);
}
