/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void QmitkDataManagerControls::SetDataTreeIterator(mitk::DataTreeIterator * it)
{
  m_DataTreeIterator = it->clone();
  mitk::DataTreeIterator* tempIt = m_DataTreeIterator->clone();
  
  //if (treeLayout != NULL && treeLayout->parent() != NULL && treeLayout->findWidget(dataTreeView) != -1) {
  //  treeLayout->remove(dataTreeView);
  //}

  QHBoxLayout * hl = (QHBoxLayout *)this->layout(); 

  if (hl->findWidget(dataTreeView) == -1) {
    dataTreeView = new DataTreeView(this);
    dataTreeView->addColumn( "Name" );
    dataTreeView->addColumn( "Properties" );
    dataTreeView->setRootIsDecorated(true);
    hl->addWidget( dataTreeView );
  }
  else {
    dataTreeView->removeItem(dataTreeView->firstChild());
  }

  if (tempIt->hasNext()) 
  {
    tempIt->next();
    DataTreeNodeItem * rootItem = new DataTreeNodeItem(dataTreeView, "Loaded Data", "root", tempIt->clone());
  }
  delete tempIt;
}
