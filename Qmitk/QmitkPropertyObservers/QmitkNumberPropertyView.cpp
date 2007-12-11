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
#include <QmitkNumberPropertyView.h>

#define DT_SHORT   1
#define DT_INT     2
#define DT_FLOAT   3
#define DT_DOUBLE  4
/*
QmitkNumberPropertyView::QmitkNumberPropertyView( const mitk::GenericProperty<short>* property, QWidget* parent, const char* name )
: QLabel( parent, name ),
  PropertyView( property ),
  m_ShortProperty(property),
  m_DataType(DT_SHORT)
{
  initialize();
}
*/
QmitkNumberPropertyView::QmitkNumberPropertyView( const mitk::IntProperty* property, QWidget* parent, const char* name )
: QLabel( parent, name ),
  PropertyView( property ),
  m_IntProperty(property),
  m_DataType(DT_INT)
{
  initialize();
}

QmitkNumberPropertyView::QmitkNumberPropertyView( const mitk::FloatProperty* property, QWidget* parent, const char* name )
: QLabel( parent, name ),
  PropertyView( property ),
  m_FloatProperty(property),
  m_DataType(DT_FLOAT)
{
  initialize();
}

QmitkNumberPropertyView::QmitkNumberPropertyView( const mitk::DoubleProperty* property, QWidget* parent, const char* name )
: QLabel( parent, name ),
  PropertyView( property ),
  m_DoubleProperty(property),
  m_DataType(DT_DOUBLE)
{
  initialize();
}

QmitkNumberPropertyView::~QmitkNumberPropertyView()
{
}

void QmitkNumberPropertyView::initialize()
{ // only to be called from constructors
  m_Suffix = "";
  m_DisplayFactor = 1.0;
  //setDecimalPlaces(-1); // unlimited
  setDecimalPlaces(2);
}
    
short QmitkNumberPropertyView::decimalPlaces() const
{
  return m_DecimalPlaces;
}

void QmitkNumberPropertyView::setDecimalPlaces(short places)
{
  m_DecimalPlaces = places;
  
  if (m_DecimalPlaces >= 0)
  {
    m_FormatString = "%%.%if";  
    m_FormatString.sprintf(QString(m_FormatString), m_DecimalPlaces); // do copy before sprintf
  }
  else
  {
    m_FormatString = "%g";  
  }
  
  DisplayNumber();
}

QString QmitkNumberPropertyView::suffix() const
{
  if (m_Suffix =="")
    return QString::null;
  else
    return m_Suffix;
}

void QmitkNumberPropertyView::setSuffix(const QString& suffix)
{
  m_Suffix = suffix;

  DisplayNumber();
}

bool QmitkNumberPropertyView::showPercent() const
{
  return m_DisplayFactor == 100.0;
}

void QmitkNumberPropertyView::setShowPercent(bool show)
{
  if (show)
  {
    m_DisplayFactor = 100.0;
    setSuffix("%");
  }
  else
  {
    m_DisplayFactor = 1.0;
    setSuffix("");
  }
  
  //DisplayNumber(); // gets called in setSuffix
}

void QmitkNumberPropertyView::PropertyChanged()
{
  if ( m_Property )
    DisplayNumber();
}

void QmitkNumberPropertyView::PropertyRemoved()
{
  m_Property = NULL;
  setText("n/a");
}

void QmitkNumberPropertyView::DisplayNumber()
{
  QString displayedText;

  switch (m_DataType)
  {
	/*
    case DT_SHORT:
      {
        short s = m_ShortProperty->GetValue();
        displayedText.sprintf(m_FormatString, s * m_DisplayFactor);
        break;
      }
    */
    case DT_INT:
      {
        int i = m_IntProperty->GetValue();
        displayedText.sprintf(m_FormatString, i * m_DisplayFactor);
        break;
      }
    case DT_FLOAT:
      {
        float f = m_FloatProperty->GetValue();
        displayedText.sprintf(m_FormatString, f * m_DisplayFactor);
        break;
      }
    case DT_DOUBLE:
      {
        double d = m_DoubleProperty->GetValue();
        displayedText.sprintf(m_FormatString, d * m_DisplayFactor);
        break;
      }
    default:
      break;
  }
    
  setText( displayedText );
}

