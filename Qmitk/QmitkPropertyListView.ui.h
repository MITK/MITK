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
//
#include <qlabel.h>
#include <qlayout.h>
#include <assert.h>
#include <qgroupbox.h>
#include <qobjectlist.h>
#include <vector>

#include "QmitkPropertyListViewItem.h"

void QmitkPropertyListView::init() {
  std::cout << "init()" << std::endl;
  m_Group->setColumns(2);
}
void QmitkPropertyListView::SetPropertyList( mitk::PropertyList *propertyList )
{
  if (propertyList != m_PropertyList) {
    m_PropertyList = propertyList;
    int row = 0; 
    const mitk::PropertyList::PropertyMap* propertyMap = propertyList->GetMap();

    // clear the group
    //
    std::cout << "before clear count:" << m_Group->children()->count() << std::endl;  
    for (std::vector<QmitkPropertyListViewItem*>::iterator it = m_Items.begin() ; it != m_Items.end() ; it++) {
      delete (*it)->m_Label;
      delete (*it)->m_Control;
    } 
    std::cout << "after clear count:" << m_Group->children()->count() << std::endl;  
    m_Items.clear();
   std::cout << "after cevtor clear count:" << m_Group->children()->count() << std::endl;  
    for (mitk::PropertyList::PropertyMap::const_iterator iter = propertyMap->begin(); iter!=propertyMap->end(); iter++) {
      QmitkPropertyListViewItem* item = QmitkPropertyListViewItem::CreateInstance(propertyList,iter->first,m_Group);
 /*     item.m_Label->reparent(m_Group);
      item.m_Label->show();
      item.m_Control->reparent(m_Group);
      item.m_Control->show();
 */
     /*
      mitk::BaseProperty* bpp = iter->second.GetPointer();
      QString propClassName(iter->second->GetNameOfClass());
      QString propName(iter->first.c_str());
      QString propValue(bpp->GetValueAsString().c_str());
      std::cout << "adding prop: " << propName << std::endl;
      QLabel* label = new QLabel(propName,m_Group);
      label->show();
      QLabel* control = new QLabel(propValue,m_Group);
      control->show();
      */
      m_Items.push_back(item);
   }
 }
}

