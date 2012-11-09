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
#ifndef QMITK_COLORPROPERTYVIEW_H_INCLUDED
#define QMITK_COLORPROPERTYVIEW_H_INCLUDED

#include <mitkPropertyObserver.h>
#include "QmitkExtExports.h"
#include <mitkColorProperty.h>
#include <QLabel>

/// @ingroup Widgets
class QmitkExt_EXPORT QmitkColorPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

  public:

    QmitkColorPropertyView( const mitk::ColorProperty*, QWidget* parent );
    virtual ~QmitkColorPropertyView();

    //virtual void unsetPalette();
    //virtual void setPalette( const QPalette & );
    //virtual void setBackgroundMode( QWidget::BackgroundMode );
    //virtual void setPaletteBackgroundColor( const QColor & );
  protected:

    virtual void PropertyChanged();
    virtual void PropertyRemoved();

    void DisplayColor();

    const mitk::ColorProperty* m_ColorProperty;

    QPalette m_WidgetPalette;

  private:

    //bool m_SelfCall;
};

#endif

