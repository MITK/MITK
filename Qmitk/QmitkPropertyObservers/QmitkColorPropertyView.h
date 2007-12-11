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
#ifndef QMITK_COLORPROPERTYVIEW_H_INCLUDED
#define QMITK_COLORPROPERTYVIEW_H_INCLUDED

#include <mitkPropertyObserver.h>
#include <mitkColorProperty.h>
#include <qlabel.h>

/// @ingroup Widgets
class QmitkColorPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

  public:
    
    QmitkColorPropertyView( const mitk::ColorProperty*, QWidget* parent, const char* name = 0 );
    virtual ~QmitkColorPropertyView();
      
    virtual void unsetPalette();
    virtual void setPalette( const QPalette & );
    virtual void setBackgroundMode( QWidget::BackgroundMode );
    virtual void setPaletteBackgroundColor( const QColor & );
  protected:

    virtual void PropertyChanged();
    virtual void PropertyRemoved();

    void DisplayColor();

    const mitk::ColorProperty* m_ColorProperty;

  private:
    
    bool m_SelfCall;
};

#endif

