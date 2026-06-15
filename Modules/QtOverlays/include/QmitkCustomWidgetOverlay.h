/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCustomWidgetOverlay_h
#define QmitkCustomWidgetOverlay_h

// MITK
#include <QmitkOverlay.h>

#include <MitkQtOverlaysExports.h>

/**
 * \class  QmitkCustomWidgetOverlay
 * \brief Overlay that wraps an externally provided custom QWidget.
 *
 * A QmitkCustomWidgetOverlay is a generic sub-class of QmitkOverlay. It
 * offers the possibility to set the internal widget from the outside.
 *
 * This allows positioning custom widgets 'on top of' other widgets using
 * the positioning mechanism of the overlay framework.
 *
 * \warning The custom widgets need to be configured and connected manually.
 * Properties set via GenerateData() have no effect on custom widgets.
 *
 * \sa QmitkOverlay
 * \sa QmitkOverlayController
 */

class MITKQTOVERLAYS_EXPORT QmitkCustomWidgetOverlay : public QmitkOverlay
{
public:
  /**
   * \brief Constructor.
   * \param[in] id String identifier used to look up overlay properties.
   */
  QmitkCustomWidgetOverlay(const char *id);

  /** \brief Destructor. */
  ~QmitkCustomWidgetOverlay() override;

  /**
   * \brief Sets the custom widget to be managed as an overlay.
   *
   * The widget replaces the internal widget pointer. Ownership is not transferred;
   * the widget will not be deleted when this overlay is destroyed.
   *
   * \param[in] widget The custom QWidget to display as an overlay. Ignored if nullptr.
   */
  void SetWidget(QWidget *widget);

  /**
   * \brief Returns the current size of the custom widget.
   * \return The QSize of the internal widget.
   */
  QSize GetNeededSize() override;
};

#endif
