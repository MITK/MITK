#ifndef QMITKPROPERTYLISTVIEWITEMFACTORY_H_INCLUDED
#define QMITKPROPERTYLISTVIEWITEMFACTORY_H_INCLUDED
#include <string>
class QWidget;
class QmitkPropertyListViewItem;
namespace mitk {
  class PropertyList;
}
class QmitkPropertyListViewItemFactory {
  public:
    static QmitkPropertyListViewItemFactory* GetInstance();
    virtual QmitkPropertyListViewItem* CreateQmitkPropertyListViewItem(mitk::PropertyList *propList, const std::string name, QWidget* parent, bool createOnlyControl) const;
    virtual void RegisterExtension(QmitkPropertyListViewItemFactory* extension) {
      m_Extension = extension;
    }
  protected:
    QmitkPropertyListViewItemFactory() : m_Extension(NULL) {}
    virtual ~QmitkPropertyListViewItemFactory() {}
    QmitkPropertyListViewItemFactory* m_Extension;
};

#endif
