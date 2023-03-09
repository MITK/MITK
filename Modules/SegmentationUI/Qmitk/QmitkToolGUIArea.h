/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkToolGUIArea_h
#define QmitkToolGUIArea_h

#include "mitkCommon.h"
#include <MitkSegmentationUIExports.h>

#include <qwidget.h>

/**
  \brief Dummy class for putting into a GUI (mainly using Qt Designer).

  This class is nothing more than a QWidget. It is good for use with QmitkToolSelectionBox as a place to display GUIs
  for active tools.

  Last contributor: $Author$
*/
class MITKSEGMENTATIONUI_EXPORT QmitkToolGUIArea : public QWidget
{
  Q_OBJECT

public:
  QmitkToolGUIArea(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkToolGUIArea() override;

signals:

public slots:

protected slots:

protected:
};

#endif
