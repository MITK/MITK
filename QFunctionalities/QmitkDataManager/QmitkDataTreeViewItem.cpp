#include <sstream>
#include "QmitkDataTreeViewItem.h"
#include "mitkPropertyList.h"
#include "mitkStringProperty.h"
#include "enabled.xpm"
#include "disabled.xpm"
QmitkDataTreeViewItem::QmitkDataTreeViewItem( QListView *parent, const QString &s1 , const QString &s2 , mitk::DataTreeIterator * nodeIt )
    : QListViewItem( parent, s1, s2 ),m_DataTreeIterator(nodeIt)
{
  m_TreeNode = nodeIt->get();
  QListViewItem::setText(0,QString("All Datasets"));

  mitk::DataTreeIterator * it = nodeIt->clone()->children();
  if (it!= NULL) 
  {
    while (it->hasNext())
    {
      it->next();
      QmitkDataTreeViewItem * nextItem = new QmitkDataTreeViewItem(this,it);
    }
  }
  delete it;
  setOpen(true);
}

QmitkDataTreeViewItem::QmitkDataTreeViewItem( QmitkDataTreeViewItem * parent, mitk::DataTreeIterator * nodeIt )
: QListViewItem(parent),m_DataTreeIterator(nodeIt) {
  m_DataTreeIterator = nodeIt;
  m_TreeNode = nodeIt->get();
  char name[256];
  
  if (m_TreeNode->GetName(*name, NULL)) {
	  QListViewItem::setText(0,QString(name));
  } else {
	  QListViewItem::setText(0,"No name!");
  }  	
  
  std::stringstream ss;
  ss << m_TreeNode->GetReferenceCount() << "/";
  mitk::BaseData::ConstPointer bd = m_TreeNode->GetData();
   if (bd.IsNotNull()) {
	  QListViewItem::setText(1,QString(bd->GetNameOfClass())) ;
     ss << m_TreeNode->GetData()->GetReferenceCount();
   } else {
	  QListViewItem::setText(1,QString("empty DataTreeNode"));
	ss << "-";	
  }
  
  QListViewItem::setText(2,QString(ss.str().c_str()));
  mitk::DataTreeIterator * it = nodeIt->clone()->children();
  if (it!= NULL && it->hasNext()) 
  {
    while (it->hasNext())
    {
      it->next();
      QmitkDataTreeViewItem * nextItem = new QmitkDataTreeViewItem(this,it);
    }
  } 
  delete it;
  setOpen(true);
}
mitk::DataTreeNode::ConstPointer QmitkDataTreeViewItem::GetDataTreeNode() const {
	return m_TreeNode;
}

void QmitkNodeViewBaseItem::updateEnabledAppearance() {
      static const QPixmap enabledPix((const char **)enabled_xpm);
      static const QPixmap disabledPix((const char **)disabled_xpm);
      // ok, we are instanceof QListViewItem, but the cast
      // is ugly:
      QListViewItem* listViewItem = NULL;
      listViewItem = dynamic_cast<NodeViewPropertyItem*>(this);
      if ((listViewItem = dynamic_cast<NodeViewPropertyItem*>(this)) ||
	  (listViewItem = dynamic_cast<NodeViewCheckboxItem*>(this))) {
	if (isPropEnabled()) {
	  listViewItem->setPixmap(0,enabledPix);
	} else {
	  listViewItem->setPixmap(0,disabledPix);

	}


      }
    }
