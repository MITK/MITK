/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKMULTIWIDGETLAYOUTMANAGER_H
#define QMITKMULTIWIDGETLAYOUTMANAGER_H

#include "MitkQtWidgetsExports.h"

#include <QObject>

class QmitkAbstractMultiWidget;
class QmitkRenderWindowWidget;

/**
* @brief The layout manager provides different layout-functions that can modify the layout of an QmitkAbstractMultiWidget.
*
*   The constructor is used with the specific multi widget that should be modified. The function 'SetLayoutDesign'
*   can then be used to modify the layout of the multi widget. The function needs to be called with the signal-slot
*   syntax from a 'QmitkRenderWindow' since it retrieves the specific render window and sets it as the current render window.
*   Some layouts work with a specific render window (e.g. ONE_BIG / SetOneBigLayout()), so the current render window is
*   then used by these layout functions.
*   The publicly accessible layout functions can also be called directly but in order to work the current render window
*   needs to be set before by using 'SetCurrentRenderWindowWidget'.
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

  /**
  * @brief The default layout shows all render windows in a rectangle.
  */
  void SetDefaultLayout();
  /**
  * @brief All 2D render windows are spread horizontally above all 3D render windows.
  */
  void SetAll2DTop3DBottomLayout();
  /**
  * @brief All 2D render windows are spread vertically beneath all 3D render windows.
  */
  void SetAll2DLeft3DRightLayout();
  /**
  * @brief The current render window is displayed as a single 'full screen' render window.
  */
  void SetOneBigLayout();
  /**
  * @brief All 2D render windows are spread horizontally, no 3D render windows.
  */
  void SetOnly2DHorizontalLayout();
  /**
  * @brief All 2D render windows are spread vertically, no 3D render windows.
  */
  void SetOnly2DVerticalLayout();
  /**
  * @brief The current render window is put above all 3D render windows, which are spread
  *        horizontally below the current render window.
  */
  void SetOneTop3DBottomLayout();
  /**
  * @brief The current render window is put beneath all 3D render windows, which are spread
  *        vertically beneath the current render window.
  */
  void SetOneLeft3DRightLayout();
  /**
  * @brief All 2D and 3D render windows are spread horizontally.
  */
  void SetAllHorizontalLayout();
  /**
  * @brief All 2D and 3D render windows are spread vertically.
  */
  void SetAllVerticalLayout();
  /**
  * @brief Hide the current render window.
  */
  void RemoveOneLayout();

private:

  QmitkAbstractMultiWidget* m_MultiWidget;
  QmitkRenderWindowWidget* m_CurrentRenderWindowWidget;

};

#endif // QMITKMULTIWIDGETLAYOUTMANAGER_H
