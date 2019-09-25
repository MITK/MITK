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
    ALL_VERTICAL,
    REMOVE_ONE,
    NONE
  };

  QmitkMultiWidgetLayoutManager(QmitkAbstractMultiWidget* multiwidget);

  /**
  * @brief This function is called by render window widgets. Given a specific
  *        layout design the layout of the multi widget data member is set with this method.
  */
  void SetLayoutDesign(LayoutDesign layoutDesign);
  /**
  * @brief Allow setting the current render window widget without relying on the sending object.
  *
  *    Calling 'SetLayoutDesign' will overwrite the current render window widget but using the public
  *    layout setter the current render window widget can be defined using the function.
  *    This is necessary for layouts that work with a specific selected render window widget.
  */
  void SetCurrentRenderWindowWidget(QmitkRenderWindowWidget* renderWindowWidget);

  void SetDefaultLayout();
  void SetAll2DTop3DBottomLayout();
  void SetAll2DLeft3DRightLayout();

  void SetOneBigLayout();

  void SetOnly2DHorizontalLayout();
  void SetOnly2DVerticalLayout();

  void SetOneTop3DBottomLayout();
  void SetOneLeft3DRightLayout();

  void SetAllHorizontalLayout();
  void SetAllVerticalLayout();

  void RemoveOneLayout();

private:

  QmitkAbstractMultiWidget* m_MultiWidget;
  QmitkRenderWindowWidget* m_CurrentRenderWindowWidget;

};

#endif // QMITKMULTIWIDGETLAYOUTMANAGER_H
