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

#ifndef QMITKPROPERTYLISTVIEWITEMFACTORY_H_INCLUDED
#define QMITKPROPERTYLISTVIEWITEMFACTORY_H_INCLUDED
#include <string>
#include <mitkCommon.h>

class QWidget;
class QmitkPropertyListViewItem;
namespace mitk {
  class PropertyList;
}
class QMITK_EXPORT QmitkPropertyListViewItemFactory {
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
