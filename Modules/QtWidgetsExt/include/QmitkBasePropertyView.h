/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMITK_BASEPROPERTYVIEW_H_INCLUDED
#define QMITK_BASEPROPERTYVIEW_H_INCLUDED

#include "MitkQtWidgetsExtExports.h"
#include <QLabel>
#include <mitkProperties.h>
#include <mitkPropertyObserver.h>

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkBasePropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

public:
  QmitkBasePropertyView(const mitk::BaseProperty *, QWidget *parent);
  ~QmitkBasePropertyView() override;

protected:
  void PropertyChanged() override;
  void PropertyRemoved() override;

private:
};

#endif
