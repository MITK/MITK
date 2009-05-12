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
#ifndef QMITK_STRINGPROPERTYEDITOR_H_INCLUDED
#define QMITK_STRINGPROPERTYEDITOR_H_INCLUDED

#include <mitkPropertyObserver.h>
#include <mitkStringProperty.h>
#include <qlineedit.h>

/// @ingroup Widgets
class QMITK_EXPORT QmitkStringPropertyEditor : public QLineEdit, public mitk::PropertyEditor
{
  Q_OBJECT

  public:
    
    QmitkStringPropertyEditor( mitk::StringProperty*, QWidget* parent, const char* name = 0 );
    virtual ~QmitkStringPropertyEditor();
      
  protected:

    virtual void PropertyChanged();
    virtual void PropertyRemoved();
    
    mitk::StringProperty* m_StringProperty;

  protected slots:
    
    void onTextChanged(const QString&);

  private:

};

#endif

