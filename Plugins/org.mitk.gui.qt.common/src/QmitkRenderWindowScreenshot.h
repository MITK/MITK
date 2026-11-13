/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowScreenshot_h
#define QmitkRenderWindowScreenshot_h

#include <org_mitk_gui_qt_common_Export.h>

#include <QColor>
#include <QImage>

class QmitkRenderWindow;

namespace mitk
{
  struct IRenderWindowPart;
}

/**
 * \brief Captures the scene of a render window as an image, as the Screenshot
 *        Maker saves it.
 *
 * Unmagnified images come from the window's frame buffer, which also works
 * while the window is hidden behind another editor. Magnified images are
 * rendered in tiles, which would repeat the decorations of the window (logo,
 * corner annotation, border, gradient background) in every tile, so these
 * are always left out then. Cropped images leave them out as well: they sit
 * at the edges of the window and would keep the crop from cutting anything.
 */
class MITK_QT_COMMON QmitkRenderWindowScreenshot
{
public:
  enum class Background
  {
    /** The background the window shows, its gradient included. */
    AsShown,
    /** A solid color. */
    Color,
    /** Transparent where the scene shows its background, partly so behind semitransparent objects. */
    Transparent
  };

  struct Options
  {
    /** \brief Image pixels per pixel of the window, at least 1. */
    unsigned int Magnification = 1;

    Background BackgroundMode = Background::AsShown;

    /** \brief For Background::Color. */
    QColor BackgroundColor = Qt::black;

    /**
     * \brief Whether the logo, corner annotation and border of the window are part of the image.
     *
     * The gradient of the background counts as a decoration: without
     * decorations, the background is flat for Background::AsShown as well.
     */
    bool Decorations = true;

    /** \brief Whether the image is cut down to the part of the scene that is not background. */
    bool CropToContent = false;

    /** \brief Pixels of background around the content when cropping to it. */
    unsigned int Padding = 0;

    /** \brief The longest side of the image at most, padding included, or 0 for no limit. */
    unsigned int MaxSize = 0;
  };

  /**
   * \brief The image of a render window of the render window part, with an
   *        alpha channel for a transparent background.
   *
   * Returns a null image only when cropping finds nothing but background.
   * Throws mitk::Exception when the capture fails (no renderer or render
   * window, or a capture that gave no image, for example because the window
   * has no size or a magnified image does not fit into memory) and when an
   * argument is null. The window looks as before afterwards. Call in the GUI
   * thread.
   */
  static QImage Take(mitk::IRenderWindowPart* renderWindowPart, QmitkRenderWindow* window, const Options& options);
};

#endif
