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

#ifndef QMITK_ENUMERATIONPROPERTYWIDGET_H_INCLUDED
#define QMITK_ENUMERATIONPROPERTYWIDGET_H_INCLUDED

#include "QmitkExtExports.h"


#include <QComboBox>
#include <QHash>

namespace mitk
{
  class EnumerationProperty;
}

class _EnumPropEditorImpl;

/// @ingroup Widgets
class QmitkExt_EXPORT QmitkEnumerationPropertyWidget : public QComboBox
{
  Q_OBJECT

  public:
    
    QmitkEnumerationPropertyWidget(QWidget* parent = 0);
    ~QmitkEnumerationPropertyWidget();

    void SetProperty(mitk::EnumerationProperty* property);

  protected slots:

    void OnIndexChanged(int index);
      
  protected:

    _EnumPropEditorImpl* propView;

};

#endif // QMITK_ENUMERATIONPROPERTYWIDGET_H_INCLUDED

