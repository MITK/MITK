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
#ifndef QMITK_NUMBERPROPERTYEDITOR_H_INCLUDED
#define QMITK_NUMBERPROPERTYEDITOR_H_INCLUDED

#include <mitkPropertyObserver.h>
#include <mitkProperties.h>
#include <QSpinBox>

/// @ingroup Widgets
class QMITK_EXPORT QmitkNumberPropertyEditor : public QSpinBox, public mitk::PropertyEditor
{
  Q_OBJECT
  Q_PROPERTY( short decimalPlaces READ getDecimalPlaces WRITE setDecimalPlaces )
  Q_PROPERTY( bool showPercent READ getShowPercent WRITE setShowPercent )
  Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
  Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )

  public:
    
    //QmitkNumberPropertyEditor( mitk::GenericProperty<short>*, QWidget* parent, const char* name = 0 );
    QmitkNumberPropertyEditor( mitk::IntProperty*, QWidget* parent );
    QmitkNumberPropertyEditor( mitk::FloatProperty*, QWidget* parent );
    QmitkNumberPropertyEditor( mitk::DoubleProperty*, QWidget* parent );
    
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

    virtual QString textFromValue(int) const;
    virtual int valueFromText(const QString&) const;

    virtual void PropertyChanged();
    virtual void PropertyRemoved();

    void DisplayNumber();

    union {
      //mitk::GenericProperty<short>*   m_ShortProperty;
      mitk::GenericProperty<int>*     m_IntProperty;
      mitk::GenericProperty<float>*   m_FloatProperty;
      mitk::GenericProperty<double>*  m_DoubleProperty;
    };

    const int m_DataType;
    
    short m_DecimalPlaces;            // how many decimal places are shown
    double m_FactorPropertyToSpinbox; // internal conversion factor. neccessary because spinbox ranges work only with ints
    double m_FactorSpinboxToDisplay;  // internal conversion factor. neccessary because spinbox ranges work only with ints
    bool m_ShowPercents;              // whether values are given in percent (0.5 -> 50%)
  
  protected slots:

    void onValueChanged(int);

  private:

    void adjustFactors(short, bool);

    bool m_SelfChangeLock;
};

#endif

