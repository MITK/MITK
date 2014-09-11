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
#include "MitkQtWidgetsExtExports.h"
#include <mitkColorProperty.h>
#include <QLabel>

/// @ingroup Widgets
class MitkQtWidgetsExt_EXPORT QmitkColorPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

  public:

    QmitkColorPropertyView( const mitk::ColorProperty*, QWidget* parent );
    virtual ~QmitkColorPropertyView();

  protected:

    virtual void PropertyChanged();
    virtual void PropertyRemoved();

    void DisplayColor();

    const mitk::ColorProperty* m_ColorProperty;

    QPalette m_WidgetPalette;
};

#endif

