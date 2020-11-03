/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertyViewFactory_h
#define QmitkPropertyViewFactory_h

#include "MitkQtWidgetsExtExports.h"
#include <mitkProperties.h>

class QWidget;

class MITKQTWIDGETSEXT_EXPORT QmitkPropertyViewFactory
{
public:
  enum ViewTypes
  {
    vtDEFAULT = 0
  };
  enum EditorTypes
  {
    etDEFAULT = 0,
    etALWAYS_EDIT = 1,
    etON_DEMAND_EDIT = 2
  };

  static QmitkPropertyViewFactory *GetInstance(); // singleton

  /// Views and editors are created via operator new. After creation they belong to the caller of
  /// CreateView()/CreateEditor(),
  /// i.e. the caller has to ensure, that the objects are properly deleted!
  QWidget *CreateView(const mitk::BaseProperty *property, unsigned int type = 0, QWidget *parent = nullptr);
  QWidget *CreateEditor(mitk::BaseProperty *property, unsigned int type = 0, QWidget *parent = nullptr);

protected:
  QmitkPropertyViewFactory(); // hidden, access through GetInstance()
  ~QmitkPropertyViewFactory();
};

#endif
