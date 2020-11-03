/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMITK_COLORPROPERTYVIEW_H_INCLUDED
#define QMITK_COLORPROPERTYVIEW_H_INCLUDED

#include "MitkQtWidgetsExtExports.h"
#include <QLabel>
#include <mitkColorProperty.h>
#include <mitkPropertyObserver.h>

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkColorPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

public:
  QmitkColorPropertyView(const mitk::ColorProperty *, QWidget *parent);
  ~QmitkColorPropertyView() override;

protected:
  void PropertyChanged() override;
  void PropertyRemoved() override;

  void DisplayColor();

  const mitk::ColorProperty *m_ColorProperty;

  QPalette m_WidgetPalette;
};

#endif
