/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <utility>
#include "mitkBoolProperty.h"
void QmitkDataManagerControls::init() { 
while (m_DataTreeView->columns() > 0 ) {
	m_DataTreeView->removeColumn(0);
    }
    m_DataTreeView->addColumn( "Name" );
    m_DataTreeView->addColumn( "NodeType" );
    m_DataTreeView->addColumn( "RefCount");
    m_RemoveButton->setEnabled(false);
}

void QmitkDataManagerControls::SetDataTreeIterator(mitk::DataTreeIterator * it)
{
    while (m_DataTreeView->firstChild()) {
	    delete m_DataTreeView->firstChild();
     }
    m_DataTreeIterator = it->clone();
    mitk::DataTreeIterator* tempIt = m_DataTreeIterator->clone();

  if (tempIt->hasNext()) 
  {
    tempIt->next();
    DataTreeViewItem * rootItem = new DataTreeViewItem(m_DataTreeView, "Loaded Data", "root", tempIt->clone());
  }
  delete tempIt;
}

void QmitkDataManagerControls::RemoveButtonClicked()
{
  DataTreeViewItem *selected = dynamic_cast<DataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected == NULL) {
    std::cout << "nothing selected" << std::endl;
  } else {
    mitk::DataTreeIterator* selectedIterator = selected->GetDataTreeIterator();
    assert(selectedIterator != NULL);
selectedIterator->remove();
    delete selected;

  } 
}


