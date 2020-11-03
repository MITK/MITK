/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOverlayContainerWidget_H_HEADER_INCLUDED_C10DC4EB
#define QmitkOverlayContainerWidget_H_HEADER_INCLUDED_C10DC4EB

// Qt
#include <QWidget>

#include <MitkQtOverlaysExports.h>

/**
* \class  QmitkOverlayContainerWidget
* \brief Widget that overrides the paintEvent method to correctly display
* the Qt based overlays when using the system-environment variable
* QT_DEVIDE_PIXEL_RATIO.
*/

class MITKQTOVERLAYS_EXPORT QmitkOverlayContainerWidget : public QWidget
{
public:
  /**
  * @brief Default Constructor
  **/
  QmitkOverlayContainerWidget(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);

  /**
  * @brief Default Destructor
  **/
  ~QmitkOverlayContainerWidget() override;

protected:
  /**
  * @brief overridden version of paintEvent that correctly clears its canvas before painting.
  **/
  void paintEvent(QPaintEvent *event) override;
};

#endif /* QmitkOverlayContainerWidget_H_HEADER_INCLUDED_C10DC4EB */
