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
#ifndef QMITK_NUMBERPROPERTYVIEW_H_INCLUDED
#define QMITK_NUMBERPROPERTYVIEW_H_INCLUDED

#include <mitkPropertyObserver.h>
#include "MitkQtWidgetsExtExports.h"
#include <mitkProperties.h>
#include <QLabel>

/// @ingroup Widgets
class MitkQtWidgetsExt_EXPORT QmitkNumberPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT
  Q_PROPERTY( short decimalPlaces READ decimalPlaces WRITE setDecimalPlaces )
  Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
  Q_PROPERTY( bool showPercent READ showPercent WRITE setShowPercent )

  public:

    QmitkNumberPropertyView( const mitk::IntProperty*, QWidget* parent );
    QmitkNumberPropertyView( const mitk::FloatProperty*, QWidget* parent );
    QmitkNumberPropertyView( const mitk::DoubleProperty*, QWidget* parent );

    virtual ~QmitkNumberPropertyView();

    short decimalPlaces() const;
    void setDecimalPlaces(short);

    QString suffix() const;
    void setSuffix(const QString&);

    bool showPercent() const;
    void setShowPercent(bool);

  protected:

    void initialize();

    virtual void PropertyChanged();
    virtual void PropertyRemoved();

    void DisplayNumber();

    union {
      const mitk::GenericProperty<int>*     m_IntProperty;
      const mitk::GenericProperty<float>*   m_FloatProperty;
      const mitk::GenericProperty<double>*  m_DoubleProperty;
    };

    const int m_DataType;

    short m_DecimalPlaces; /// -1 indicates "no limit to decimal places"
    QString m_Suffix;
    double m_DisplayFactor;
};

#endif

