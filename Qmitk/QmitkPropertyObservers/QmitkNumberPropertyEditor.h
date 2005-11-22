/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#ifndef QMITK_NUMBERPROPERTYEDITOR_H_INCLUDED
#define QMITK_NUMBERPROPERTYEDITOR_H_INCLUDED

#include <mitkPropertyObserver.h>
#include <mitkProperties.h>
#include <qspinbox.h>

class QmitkNumberPropertyEditor : public QSpinBox, public mitk::PropertyEditor
{
  Q_OBJECT
  Q_PROPERTY( short decimalPlaces READ getDecimalPlaces WRITE setDecimalPlaces )
  Q_PROPERTY( bool showPercent READ getShowPercent WRITE setShowPercent )
  Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
  Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )

  public:
    
    QmitkNumberPropertyEditor( mitk::GenericProperty<short>*, QWidget* parent, const char* name = 0 );
    QmitkNumberPropertyEditor( mitk::IntProperty*, QWidget* parent, const char* name = 0 );
    QmitkNumberPropertyEditor( mitk::FloatProperty*, QWidget* parent, const char* name = 0 );
    QmitkNumberPropertyEditor( mitk::DoubleProperty*, QWidget* parent, const char* name = 0 );
    
    virtual ~QmitkNumberPropertyEditor();

    short getDecimalPlaces() const;
    void setDecimalPlaces(short);
    
    bool getShowPercent() const;
    void setShowPercent(bool);

    int minValue() const;
    void setMinValue(int);
    int maxValue() const;
    void setMaxValue(int);
    double doubleValue() const;
    void setDoubleValue(double);

  protected:

    void initialize();

    virtual QString mapValueToText(int);
    virtual int mapTextToValue(bool*);

    virtual void PropertyChanged();
    virtual void PropertyRemoved();

    void DisplayNumber();

    union {
      const mitk::GenericProperty<short>*   m_ShortProperty;
      const mitk::GenericProperty<int>*     m_IntProperty;
      const mitk::GenericProperty<float>*   m_FloatProperty;
      const mitk::GenericProperty<double>*  m_DoubleProperty;
    };

    const int m_DataType;
    
    short m_DecimalPlaces; /// -1 indicates "no limit to decimal places"
    double m_DisplayFactor;

  private:

};

#endif

