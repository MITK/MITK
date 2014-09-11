/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "QmitkNumberPropertyView.h"
#include <QTextStream>

#define DT_SHORT   1
#define DT_INT     2
#define DT_FLOAT   3
#define DT_DOUBLE  4

QmitkNumberPropertyView::QmitkNumberPropertyView( const mitk::IntProperty* property, QWidget* parent )
: QLabel( parent ),
  PropertyView( property ),
  m_IntProperty(property),
  m_DataType(DT_INT)
{
  initialize();
}

QmitkNumberPropertyView::QmitkNumberPropertyView( const mitk::FloatProperty* property, QWidget* parent )
: QLabel( parent ),
  PropertyView( property ),
  m_FloatProperty(property),
  m_DataType(DT_FLOAT)
{
  initialize();
}

QmitkNumberPropertyView::QmitkNumberPropertyView( const mitk::DoubleProperty* property, QWidget* parent )
: QLabel( parent ),
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
  setDecimalPlaces(2);
}

short QmitkNumberPropertyView::decimalPlaces() const
{
  return m_DecimalPlaces;
}

void QmitkNumberPropertyView::setDecimalPlaces(short places)
{
  m_DecimalPlaces = places;
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
  QTextStream stream(&displayedText);

  stream.setRealNumberPrecision(m_DecimalPlaces);

  switch (m_DataType)
  {
    case DT_INT:
      {
        int i = m_IntProperty->GetValue();
        stream << (i * m_DisplayFactor);
        break;
      }
    case DT_FLOAT:
      {
        float f = m_FloatProperty->GetValue();
        stream << (f * m_DisplayFactor);
        break;
      }
    case DT_DOUBLE:
      {
        double d = m_DoubleProperty->GetValue();
        stream << (d * m_DisplayFactor);
        break;
      }
    default:
      break;
  }

  setText( displayedText );
}

