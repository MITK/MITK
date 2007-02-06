#include <QmitkPropertyListViewItemFactory.h>
#include <QmitkPropertyListViewItem.h>

QmitkPropertyListViewItemFactory* QmitkPropertyListViewItemFactory::GetInstance() {
   static QmitkPropertyListViewItemFactory* instance = new QmitkPropertyListViewItemFactory();
   return instance;
}
QmitkPropertyListViewItem* QmitkPropertyListViewItemFactory::CreateQmitkPropertyListViewItem(mitk::PropertyList *propList, const std::string name, QWidget* parent, bool createOnlyControl) const {
  // legacy
  QmitkPropertyListViewItem* newItem = QmitkPropertyListViewItem::CreateInstance(propList,name,parent,createOnlyControl);
  if (!newItem && m_Extension) {
    // try the registered extension
    newItem = m_Extension->CreateQmitkPropertyListViewItem(propList,name,parent,createOnlyControl);
  }
  return newItem;
}
