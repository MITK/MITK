/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderWindowScreenshot.h"

#include <QmitkRenderWindow.h>

#include <mitkException.h>
#include <mitkIRenderWindowPart.h>
#include <mitkRenderingManager.h>

#include <vtkImageData.h>
#include <vtkRenderLargeImage.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

#include <QPainter>
#include <QStringList>

#include <algorithm>
#include <cstdlib>
#include <memory>

namespace
{
  /** The scene as it renders now, in RGB32. */
  QImage Capture(QmitkRenderWindow* window, vtkRenderer* renderer, unsigned int magnification)
  {
    // Also before tiles: mappers that place things by the edges of the window,
    // like the labels of planar figures, keep in tiles what this render decided.
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(window->GetVtkRenderWindow());

    if (magnification <= 1)
    {
      auto image = window->grabFramebuffer().convertToFormat(QImage::Format_RGB32);

      // Qt gives it the screen's ratio, with which painters would draw on and
      // with it in logical pixels, while this class deals in image pixels.
      image.setDevicePixelRatio(1.0);

      return image;
    }

    // vtkRenderLargeImage tiles the renderer's camera; vtkWindowToImageFilter
    // would repeat the whole window instead, as MITK's windows ignore tiling.
    auto magnifier = vtkSmartPointer<vtkRenderLargeImage>::New();
    magnifier->SetInput(renderer);
    magnifier->SetMagnification(static_cast<int>(magnification));

    // Tells mappers that the renders are tiles, also on the way out by an exception.
    auto* baseRenderer = window->GetRenderer();
    baseRenderer->SetTileMagnification(magnification);
    const std::unique_ptr<mitk::BaseRenderer, void (*)(mitk::BaseRenderer*)> resetMagnification(
      baseRenderer, [](mitk::BaseRenderer* tiled) { tiled->SetTileMagnification(1); });

    magnifier->Update();

    auto* output = magnifier->GetOutput();
    int dimensions[3];
    output->GetDimensions(dimensions);

    const auto components = output->GetNumberOfScalarComponents();
    const auto* pixels = static_cast<const unsigned char*>(output->GetScalarPointer());

    QImage image(dimensions[0], dimensions[1], QImage::Format_RGB32);

    // A magnified image may not fit into memory.
    if (image.isNull() || pixels == nullptr)
      return QImage();

    for (int y = 0; y < dimensions[1]; ++y)
    {
      // VTK stores the bottom row first.
      const auto* source = pixels + static_cast<std::size_t>(dimensions[1] - 1 - y) * dimensions[0] * components;
      auto* target = reinterpret_cast<QRgb*>(image.scanLine(y));

      for (int x = 0; x < dimensions[0]; ++x, source += components)
        target[x] = qRgb(source[0], source[1], source[2]);
    }

    return image;
  }

  void SetBackground(vtkRenderer* renderer, const QColor& color)
  {
    renderer->SetBackground(color.redF(), color.greenF(), color.blueF());
  }

  /**
   * The scene over a transparent background. Render windows have no alpha
   * channel, so the opacity of each pixel comes from rendering the scene over
   * black and over white.
   */
  QImage CaptureTransparent(QmitkRenderWindow* window, vtkRenderer* renderer, unsigned int magnification)
  {
    SetBackground(renderer, Qt::black);
    const auto overBlack = Capture(window, renderer, magnification);

    SetBackground(renderer, Qt::white);
    const auto overWhite = Capture(window, renderer, magnification);

    if (overBlack.isNull() || overWhite.isNull())
      return QImage();

    QImage image(overBlack.size(), QImage::Format_ARGB32);

    for (int y = 0; y < image.height(); ++y)
    {
      const auto* black = reinterpret_cast<const QRgb*>(overBlack.constScanLine(y));
      const auto* white = reinterpret_cast<const QRgb*>(overWhite.constScanLine(y));
      auto* target = reinterpret_cast<QRgb*>(image.scanLine(y));

      for (int x = 0; x < image.width(); ++x)
      {
        // A pixel of opacity a is a*C over black and a*C + (1-a)*255 over white,
        // so the difference averaged over the channels is (1-a)*255.
        const int backgroundShare = ((qRed(white[x]) - qRed(black[x])) + (qGreen(white[x]) - qGreen(black[x])) +
                                     (qBlue(white[x]) - qBlue(black[x]))) / 3;
        const int alpha = std::clamp(255 - backgroundShare, 0, 255);

        if (alpha == 0)
        {
          target[x] = qRgba(0, 0, 0, 0);
          continue;
        }

        // Undo the premultiplication of the color over black.
        target[x] = qRgba(std::min(255, qRed(black[x]) * 255 / alpha),
                          std::min(255, qGreen(black[x]) * 255 / alpha),
                          std::min(255, qBlue(black[x]) * 255 / alpha),
                          alpha);
      }
    }

    return image;
  }

  /** The bounds of the pixels that are not background, empty if there are none. */
  QRect FindContent(const QImage& image, const QColor& background)
  {
    const bool transparent = background.alpha() == 0;

    const auto isContent = [&](QRgb pixel) {
      if (transparent)
        return qAlpha(pixel) != 0;

      // The color passes through the frame buffer in 8 bits, which may round it.
      return std::abs(qRed(pixel) - background.red()) > 1 || std::abs(qGreen(pixel) - background.green()) > 1 ||
             std::abs(qBlue(pixel) - background.blue()) > 1;
    };

    int left = image.width();
    int right = -1;
    int top = image.height();
    int bottom = -1;

    for (int y = 0; y < image.height(); ++y)
    {
      const auto* line = reinterpret_cast<const QRgb*>(image.constScanLine(y));

      for (int x = 0; x < image.width(); ++x)
      {
        if (!isContent(line[x]))
          continue;

        left = std::min(left, x);
        right = std::max(right, x);
        top = std::min(top, y);
        bottom = y;
      }
    }

    return right < 0 ? QRect() : QRect(QPoint(left, top), QPoint(right, bottom));
  }

  QImage Pad(const QImage& image, int padding, const QColor& background)
  {
    QImage padded(image.width() + 2 * padding, image.height() + 2 * padding, image.format());
    padded.fill(background);

    QPainter painter(&padded);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(padding, padding, image);

    return padded;
  }
}

QImage QmitkRenderWindowScreenshot::Take(mitk::IRenderWindowPart* renderWindowPart, QmitkRenderWindow* window, const Options& options)
{
  if (renderWindowPart == nullptr || window == nullptr)
    mitkThrow() << "The render window part and the window must not be null.";

  auto* renderer = window->GetRenderer()->GetVtkRenderer();
  auto* renderWindow = window->GetRenderer()->GetRenderWindow();

  if (renderer == nullptr || renderWindow == nullptr)
    mitkThrow() << "The window has no renderer or no render window.";

  const auto requestedMagnification = std::max(1u, options.Magnification);
  auto magnification = requestedMagnification;

  // Scaling down to MaxSize throws away what magnifying beyond it renders. A
  // cropped image can be much smaller than the window, so it keeps its pixels.
  if (options.MaxSize > 0 && !options.CropToContent)
  {
    const auto* size = renderWindow->GetSize();
    const auto longest = static_cast<unsigned int>(std::max(1, std::max(size[0], size[1])));

    magnification = std::min(magnification, std::max(1u, (options.MaxSize + longest - 1) / longest));
  }

  // A background of its own replaces the gradient, which is a decoration.
  QStringList decorations;

  if (!options.Decorations || requestedMagnification > 1 || options.CropToContent)
  {
    decorations = {
      mitk::IRenderWindowPart::DECORATION_LOGO,
      mitk::IRenderWindowPart::DECORATION_CORNER_ANNOTATION,
      mitk::IRenderWindowPart::DECORATION_BACKGROUND };
  }
  else if (options.BackgroundMode != Background::AsShown)
  {
    decorations = { mitk::IRenderWindowPart::DECORATION_BACKGROUND };
  }

  QStringList decorationsToRestore;

  for (const auto& decoration : decorations)
  {
    if (renderWindowPart->IsDecorationEnabled(decoration))
      decorationsToRestore << decoration;
  }

  if (!decorations.isEmpty())
    renderWindowPart->EnableDecorations(false, decorations);

  const bool doubleBuffering = renderWindow->GetDoubleBuffer();

  if (magnification > 1)
    renderWindow->DoubleBufferOff();

  double background[3];
  renderer->GetBackground(background);

  const auto restore = [&]() {
    renderer->SetBackground(background);
    renderWindow->SetDoubleBuffer(doubleBuffering);

    if (!decorationsToRestore.isEmpty())
      renderWindowPart->EnableDecorations(true, decorationsToRestore);

    // Tiling moves the camera of 2D windows; rendering again restores it, the
    // background and the decorations.
    if (!decorations.isEmpty() || options.BackgroundMode != Background::AsShown)
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  };

  QImage image;

  // Rendering may throw, for example when a magnified image does not fit into memory.
  try
  {
    switch (options.BackgroundMode)
    {
      case Background::AsShown:
        image = Capture(window, renderer, magnification);
        break;
      case Background::Color:
        SetBackground(renderer, options.BackgroundColor);
        image = Capture(window, renderer, magnification);
        break;
      case Background::Transparent:
        image = CaptureTransparent(window, renderer, magnification);
        break;
    }
  }
  catch (...)
  {
    restore();
    throw;
  }

  restore();

  if (image.isNull())
    mitkThrow() << "The window could not be captured.";

  // Without the gradient, which cropping leaves out, the window shows the renderer's own background.
  const auto shownBackground = options.BackgroundMode == Background::Transparent ? QColor(Qt::transparent)
    : options.BackgroundMode == Background::Color ? options.BackgroundColor
    : QColor::fromRgbF(static_cast<float>(background[0]), static_cast<float>(background[1]), static_cast<float>(background[2]));

  int padding = 0;

  // Cropping at full resolution lets the content, not the empty background, take up MaxSize.
  if (options.CropToContent)
  {
    const auto content = FindContent(image, shownBackground);

    if (content.isEmpty())
      return QImage();

    image = image.copy(content);
    padding = static_cast<int>(options.Padding);
  }

  if (options.MaxSize > 0)
  {
    const auto maxSize = std::max(1, static_cast<int>(options.MaxSize) - 2 * padding);
    const auto longest = std::max(image.width(), image.height());

    if (longest > maxSize)
    {
      image = image.scaled(std::max(1, image.width() * maxSize / longest), std::max(1, image.height() * maxSize / longest),
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
  }

  if (padding > 0)
    image = Pad(image, padding, shownBackground);

  return image;
}
