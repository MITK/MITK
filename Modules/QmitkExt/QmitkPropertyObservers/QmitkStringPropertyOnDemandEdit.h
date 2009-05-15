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
#ifndef QMITK_STRINGPROPERTYONDEMANDDEDITOR_H_INCLUDED
#define QMITK_STRINGPROPERTYONDEMANDDEDITOR_H_INCLUDED

#include <mitkPropertyObserver.h>
#include <mitkStringProperty.h>
#include <QLayout>
#include <QLabel>

class QMITKEXT_EXPORT QClickableLabel2 : public QLabel
{
  Q_OBJECT

  signals:
    void clicked();

  public:

    QClickableLabel2( QWidget * parent, Qt::WindowFlags f = 0 )
    :QLabel(parent, f)
    {
    }
  
    virtual void mouseReleaseEvent( QMouseEvent* )
    {
      emit clicked();
    }
};


/// @ingroup Widgets
class QMITKEXT_EXPORT QmitkStringPropertyOnDemandEdit : public QFrame, public mitk::PropertyEditor
{
  Q_OBJECT

  public:
    
    QmitkStringPropertyOnDemandEdit( mitk::StringProperty*, QWidget* parent );
    virtual ~QmitkStringPropertyOnDemandEdit();
      
    //virtual void setPalette ( const QPalette & );
    //virtual void setBackgroundMode ( BackgroundMode );
  protected:

    virtual void PropertyChanged();
    virtual void PropertyRemoved();
    
    mitk::StringProperty* m_StringProperty;

    QHBoxLayout* m_layout;
    QLabel* m_label;
    QClickableLabel2* m_toolbutton;

  protected slots:
    
    void onToolButtonClicked();

  private:

};

#endif

