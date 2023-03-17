/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkStringPropertyView_h
#define QmitkStringPropertyView_h

#include "MitkQtWidgetsExtExports.h"
#include <QLabel>
#include <mitkPropertyObserver.h>
#include <mitkStringProperty.h>

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkStringPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

public:
  QmitkStringPropertyView(const mitk::StringProperty *, QWidget *parent);
  ~QmitkStringPropertyView() override;

protected:
  void PropertyChanged() override;
  void PropertyRemoved() override;

  const mitk::StringProperty *m_StringProperty;
};

#endif
