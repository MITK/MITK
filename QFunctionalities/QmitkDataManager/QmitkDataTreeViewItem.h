#ifndef DATA_TREE_VIEW_H
#define DATA_TREE_VIEW_H
#include <qlistview.h>
#include <qstring.h>

class QWidget;
#include <mitkDataTree.h>
#include "mitkProperties.h"
#include "mitkRenderWindow.h"
#include "itkCommand.h"
#include "QmitkDataManagerControls.h"
class NodeViewPropertyItem;
class NodeViewCheckboxItem;

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

class QmitkNodeViewBaseItem {
  public:
    QmitkNodeViewBaseItem(mitk::PropertyList::Pointer propList, mitk::BaseProperty::Pointer property) : m_PropertyList(propList),m_Property(property) {};
    bool isPropEnabled() {
      return m_Property->GetEnabled();
    }  
    void setPropEnabled(bool enable) {
      m_Property->SetEnabled(enable);
    } 
    void updateEnabledAppearance();    
    virtual ~QmitkNodeViewBaseItem() {}
  protected:
    bool  m_PropEnabled ;
    mitk::PropertyList::Pointer m_PropertyList;
    mitk::BaseProperty::Pointer m_Property;
};


class NodeViewCheckboxItem : public QCheckListItem, public QmitkNodeViewBaseItem {
  public:
    NodeViewCheckboxItem(QListView* parent,QString name,bool value, bool isDefault, mitk::PropertyList::Pointer propList,mitk::BaseProperty::Pointer property) : 
      QCheckListItem(parent,name,QCheckListItem::CheckBox) , QmitkNodeViewBaseItem(propList,property)
      {
        setOn(value);
      }

    virtual ~NodeViewCheckboxItem() {}
  protected:
    virtual void stateChange ( bool state) {
   mitk::BoolProperty* boolProp = dynamic_cast<mitk::BoolProperty*>(m_Property.GetPointer());
//      if (m_PropertyList->SetProperty((const char *)text(),new mitk::BoolProperty(state))) {
   if (boolProp != NULL && boolProp->GetValue() != state) {
         
        boolProp->SetValue(state);
	mitk::RenderWindow::UpdateAllInstances();
      };
        
       QCheckListItem::stateChange(state);
      listView()->repaintContents();      
    };
};

class NodeViewPropertyItem : public QListViewItem, public QmitkNodeViewBaseItem {
  public:
    NodeViewPropertyItem(QListView* parent,QString propClassName, QString name,QString value, bool isDefault, mitk::PropertyList::Pointer propList,mitk::BaseProperty::Pointer property) : 
      QListViewItem(parent,propClassName, name, value) , QmitkNodeViewBaseItem(propList,property){ };
    virtual ~NodeViewPropertyItem() {}
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
