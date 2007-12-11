/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include <sstream>
#include "QmitkDataTreeViewItem.h"
#include "mitkPropertyList.h"
#include "mitkStringProperty.h"

  QmitkDataTreeViewItem::QmitkDataTreeViewItem( QListView *parent, const QString &s1 , const QString &s2 , mitk::DataTreeIteratorBase* nodeIt )
  : QListViewItem( parent, s1, s2 ), m_DataTreeIterator(NULL)
{
  assert(nodeIt!=NULL);
  m_DataTreeIterator = nodeIt;

  m_TreeNode = nodeIt->Get();
  QListViewItem::setText(0,QString("All Datasets"));

  mitk::DataTreeChildIterator it(*nodeIt);
  while (!it.IsAtEnd())
  {
    new QmitkDataTreeViewItem(this,&it);
    ++it;
  }
  setOpen(true);
}

QmitkDataTreeViewItem::QmitkDataTreeViewItem( QmitkDataTreeViewItem * parent, mitk::DataTreeIteratorBase * nodeIt )
: QListViewItem(parent),m_DataTreeIterator(NULL) 
{
  assert(nodeIt!=NULL);
  m_DataTreeIterator = nodeIt;

  m_TreeNode = nodeIt->Get();
  char name[256];

  if(m_TreeNode.IsNotNull())
  {
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
  }
  else
  {
    QListViewItem::setText(0,"NULL node!");
  }

  mitk::DataTreeChildIterator it(*nodeIt);
  while (!it.IsAtEnd())
  {
    new QmitkDataTreeViewItem(this,&it);
    ++it;
  }
  setOpen(true);
}

mitk::DataTreeNode::Pointer QmitkDataTreeViewItem::GetDataTreeNode() const {
  return m_TreeNode;
}

