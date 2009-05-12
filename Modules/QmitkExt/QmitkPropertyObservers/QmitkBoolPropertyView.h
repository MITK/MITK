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
#ifndef QMITK_BOOLPROPERTYVIEW_H_INCLUDED
#define QMITK_BOOLPROPERTYVIEW_H_INCLUDED

#include <mitkPropertyObserver.h>
#include <mitkProperties.h>
#include <QCheckBox>

/// @ingroup Widgets
class QMITK_EXPORT QmitkBoolPropertyView : public QCheckBox, public mitk::PropertyView
{
  Q_OBJECT

  public:
    
    QmitkBoolPropertyView( const mitk::BoolProperty*, QWidget* parent );
    virtual ~QmitkBoolPropertyView();
      
  protected:

    virtual void PropertyChanged();
    virtual void PropertyRemoved();

    const mitk::BoolProperty* m_BoolProperty;

  private:

};

#endif

