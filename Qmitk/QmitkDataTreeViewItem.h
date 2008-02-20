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


#ifndef QMITK_DATA_TREE_VIEW_H
#define QMITK_DATA_TREE_VIEW_H
#include <qlistview.h>
#include <qstring.h>

class QWidget;
#include <mitkDataTree.h>
#include <itkCommand.h>

class NodeViewPropertyItem;
class NodeViewCheckboxItem;

class QMITK_EXPORT QmitkDataTreeViewItem : public QListViewItem
{

  public:
    QmitkDataTreeViewItem( QListView *parent, const QString &s1, const QString &s2, mitk::DataTreeIteratorBase * nodeIt );
    QmitkDataTreeViewItem( QmitkDataTreeViewItem * parent, mitk::DataTreeIteratorBase * nodeIt );
    virtual ~QmitkDataTreeViewItem() {}

    mitk::DataTreeNode::Pointer GetDataTreeNode() const;
    mitk::DataTreeIteratorBase* GetDataTreeIterator() {return m_DataTreeIterator.GetPointer();}

  protected:
    mitk::DataTreeNode::Pointer m_TreeNode;
    mitk::DataTreeIteratorClone m_DataTreeIterator;
};



class QmitkDataManagerControls;	
class QmitkFocusChangeCommand : public itk::Command {
  protected:
    QmitkDataManagerControls* m_QmitkDataManagerControls; 
  public: 
    mitkClassMacro(QmitkFocusChangeCommand,itk::Command);
    itkNewMacro(Self);
    virtual void Execute (itk::Object * /*caller*/, const itk::EventObject & /*event*/) {
      DoAction();
    }
    virtual void Execute (const itk::Object * /*caller*/, const itk::EventObject & /*event*/) {
      DoAction();
    } 
    void DoAction(); 
    void SetQmitkDataManagerControls(QmitkDataManagerControls* dmc) {
      m_QmitkDataManagerControls = dmc;
    }

};



#endif //QMITK_DATA_TREE_VIEW_H
