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
#ifndef QMITK_BASEPROPERTYVIEW_H_INCLUDED
#define QMITK_BASEPROPERTYVIEW_H_INCLUDED

#include <mitkPropertyObserver.h>
#include "QmitkExtExports.h"
#include <mitkProperties.h>
#include <QLabel>

/// @ingroup Widgets
class QmitkExt_EXPORT QmitkBasePropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

  public:

    QmitkBasePropertyView( const mitk::BaseProperty*, QWidget* parent);
    virtual ~QmitkBasePropertyView();

  protected:

    virtual void PropertyChanged();
    virtual void PropertyRemoved();

  private:

};

#endif

