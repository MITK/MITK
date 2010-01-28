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
#ifndef QMITK_BOOLPROPERTYEDITOR_H_INCLUDED
#define QMITK_BOOLPROPERTYEDITOR_H_INCLUDED

#include "QmitkBoolPropertyView.h"
#include "QmitkExtExports.h"

/// @ingroup Widgets
class QmitkExt_EXPORT QmitkBoolPropertyEditor : public QmitkBoolPropertyView
{
  Q_OBJECT

  public:
    
    QmitkBoolPropertyEditor( const mitk::BoolProperty*, QWidget* parent );
    virtual ~QmitkBoolPropertyEditor();
      
  protected:

    virtual void PropertyRemoved();

  protected slots:

    void onToggle(bool on);

};

#endif

