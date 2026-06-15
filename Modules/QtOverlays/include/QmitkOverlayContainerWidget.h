/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOverlayContainerWidget_h
#define QmitkOverlayContainerWidget_h

// Qt
#include <QWidget>

#include <MitkQtOverlaysExports.h>

/**
 * \class  QmitkOverlayContainerWidget
 * \brief Container widget for overlay positioning that correctly clears its background.
 *
 * This widget overrides the paintEvent method to use CompositionMode_Clear
 * before painting, ensuring a fully transparent background. This is necessary
 * for proper overlay rendering when using high-DPI scaling (QT_DEVICE_PIXEL_RATIO).
 *
 * \sa QmitkOverlayController
 * \sa QmitkOverlay
 */

class MITKQTOVERLAYS_EXPORT QmitkOverlayContainerWidget : public QWidget
{
public:
  /**
   * \brief Constructor.
   * \param[in] parent Optional parent widget.
   * \param[in] f Optional window flags.
   */
  QmitkOverlayContainerWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /** \brief Destructor. */
  ~QmitkOverlayContainerWidget() override;

protected:
  /**
   * \brief Clears the widget background with transparency before painting.
   * \param[in] event The paint event (unused).
   */
  void paintEvent(QPaintEvent *event) override;
};

#endif
