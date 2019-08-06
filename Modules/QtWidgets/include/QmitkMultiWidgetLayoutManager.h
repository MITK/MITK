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

#ifndef QMITKMULTIWIDGETLAYOUTMANAGER_H
#define QMITKMULTIWIDGETLAYOUTMANAGER_H

#include "MitkQtWidgetsExports.h"

#include <QObject>

class QmitkAbstractMultiWidget;
class QmitkRenderWindowWidget;

/**
* @brief
*
*
*/
class MITKQTWIDGETS_EXPORT QmitkMultiWidgetLayoutManager : public QObject
{
  Q_OBJECT

public:

  enum class LayoutDesign
  {
    DEFAULT = 0,
    ALL_2D_TOP_3D_BOTTOM,
    ALL_2D_LEFT_3D_RIGHT,
    ONE_BIG,
    ONLY_2D_HORIZONTAL,
    ONLY_2D_VERTICAL,
    ONE_TOP_3D_BOTTOM,
    ONE_LEFT_3D_RIGHT,
    ALL_HORIZONTAL,
    ALL_VERTICAL
  };

  QmitkMultiWidgetLayoutManager(QmitkAbstractMultiWidget* multiwidget);

  void OnLayoutDesignChanged(LayoutDesign layoutDesign);

private:

  void SetDefaultLayout();
  void SetAll2DTop3DBottomLayout();
  void SetAll2DLeft3DRightLayout();

  void SetOneBigLayout();

  void SetOnly2DLayoutHorizontal();
  void SetOnly2DLayoutVertical();

  void SetOneTop3DBottomLayout();
  void SetOneLeft3DRightLayout();

  void SetAllHorizontal();
  void SetAllVertical();

  QmitkAbstractMultiWidget* m_MultiWidget;
  QmitkRenderWindowWidget* m_CurrentRenderWindowWidget;

};

#endif // QMITKMULTIWIDGETLAYOUTMANAGER_H
