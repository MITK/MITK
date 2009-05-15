/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13599 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef QmitkNumberPropertySliderhincludecd
#define QmitkNumberPropertySliderhincludecd

#include <mitkPropertyObserver.h>
#include <mitkProperties.h>
#include <qslider.h>

/// @ingroup Widgets
class QMITKEXT_EXPORT QmitkNumberPropertySlider : public QSlider, public mitk::PropertyEditor
{
  Q_OBJECT
  Q_PROPERTY( short decimalPlaces READ getDecimalPlaces WRITE setDecimalPlaces )
  Q_PROPERTY( bool showPercent READ getShowPercent WRITE setShowPercent )
  Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
  Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )

  public:
    
    //QmitkNumberPropertySlider( mitk::GenericProperty<short>*, QWidget* parent, const char* name = 0 );
    QmitkNumberPropertySlider( mitk::IntProperty*, QWidget* parent, const char* name = 0 );
    QmitkNumberPropertySlider( mitk::FloatProperty*, QWidget* parent, const char* name = 0 );
    QmitkNumberPropertySlider( mitk::DoubleProperty*, QWidget* parent, const char* name = 0 );
    
    virtual ~QmitkNumberPropertySlider();

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
    double m_FactorPropertyToSlider; // internal conversion factor. neccessary because slider ranges work only with ints
    double m_FactorSliderToDisplay;  // internal conversion factor. neccessary because slider ranges work only with ints
    bool m_ShowPercents;              // whether values are given in percent (0.5 -> 50%)
  
  protected slots:

    void onValueChanged(int);

  private:

    void adjustFactors(short, bool);

    bool m_SelfChangeLock;
};

#endif

