#ifndef DATA_TREE_VIEW_H
#define DATA_TREE_VIEW_H
#include <qlistview.h>
#include <qstring.h>

class QWidget;
#include <mitkDataTree.h>
#include "mitkBoolProperty.h"
#include "mitkRenderWindow.h"
#include "itkCommand.h"
#include "QmitkDataManagerControls.h"

class QmitkDataTreeViewItem : public QListViewItem
{
public:
  QmitkDataTreeViewItem( QListView *parent, const QString &s1 = 0, const QString &s2 = 0, mitk::DataTreeIterator * nodeIt = 0 );
  QmitkDataTreeViewItem( QmitkDataTreeViewItem * parent, mitk::DataTreeIterator * nodeIt );
  mitk::DataTreeNode::ConstPointer GetDataTreeNode() const;
  mitk::DataTreeIterator* GetDataTreeIterator() {return m_DataTreeIterator;}
protected:
  mitk::DataTreeNode::ConstPointer m_TreeNode;   
  mitk::DataTreeIterator* m_DataTreeIterator;
};


class NodeViewCheckboxItem : public QCheckListItem {
  public:
    NodeViewCheckboxItem(QListView* parent,QString name,bool value, bool isDefault, mitk::PropertyList::Pointer propList) : 
	QCheckListItem(parent,name,QCheckListItem::CheckBox),m_PropertyList(propList) 
    {
      setTristate(true);
      if(isDefault) {
        setState(QCheckListItem::NoChange);
      } else { 
        setState(value?QCheckListItem::On:QCheckListItem::Off);	
      }
    };

  protected:
    mitk::PropertyList::Pointer m_PropertyList;
    virtual void newStateChange ( bool newState ) {
      std::cout << "newStateChange called:" << std::endl;
      if (!newState) {
      // newState can be off or unchanged
        if(state()==QCheckListItem::NoChange) {
          m_PropertyList->DeleteProperty((const char *)text());
          mitk::RenderWindow::UpdateAllInstances();
        } 
      }
      if (m_PropertyList->SetProperty((const char *)text(),new mitk::BoolProperty(newState))) {
	mitk::RenderWindow::UpdateAllInstances();
      }      
      listView()->repaintContents();      
    };
};

class NodeViewPropertyItem : public QListViewItem {
  public:
    NodeViewPropertyItem(QListView* parent,QString propClassName, QString name,QString value, bool isDefault, mitk::PropertyList::Pointer propList) : 
	QListViewItem(parent,propClassName, name, value),m_PropertyList(propList) {

};
 protected:
    mitk::PropertyList::Pointer m_PropertyList;
  
};
class QmitkDataManagerControls;	
class QmitkFocusChangeCommand : public itk::Command {
 protected:
    QmitkDataManagerControls* m_QmitkDataManagerControls; 
public: 
    mitkClassMacro(QmitkFocusChangeCommand,itk::Command);
    itkNewMacro(Self);
    virtual void Execute (itk::Object *caller, const itk::EventObject &event) {
      DoAction();
    }
    virtual void Execute (const itk::Object *caller, const itk::EventObject &event) {
 	DoAction();
    } 
    void DoAction(); 
    void SetQmitkDataManagerControls(QmitkDataManagerControls* dmc) {
      m_QmitkDataManagerControls = dmc;
    }
  
};



#endif //DATA_TREE_VIEW_H
