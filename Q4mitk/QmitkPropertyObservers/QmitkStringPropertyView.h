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
#ifndef QMITK_STRINGPROPERTYVIEW_H_INCLUDED
#define QMITK_STRINGPROPERTYVIEW_H_INCLUDED

#include <mitkPropertyObserver.h>
#include <mitkStringProperty.h>
#include <QLabel>

/// @ingroup Widgets
class QMITK_EXPORT QmitkStringPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

  public:
    
    QmitkStringPropertyView( const mitk::StringProperty*, QWidget* parent );
    virtual ~QmitkStringPropertyView();
      
  protected:

    virtual void PropertyChanged();
    virtual void PropertyRemoved();

    const mitk::StringProperty* m_StringProperty;
  
  private:


};

#endif

