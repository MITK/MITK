#include "QmitkPropertyListPopupProvider.h"

#include "QmitkPropertyListPopup.h"

QmitkPropertyListPopupProvider::QmitkPropertyListPopupProvider()
{
}

QmitkPropertyListPopupProvider::~QmitkPropertyListPopupProvider()
{
}

QmitkPropertyListPopup* QmitkPropertyListPopupProvider::CreatePopup(mitk::PropertyList* list, QObject* parent, bool disableBoolProperties, const char* name)
{
  return new QmitkPropertyListPopup(list, parent, disableBoolProperties, name);
}
