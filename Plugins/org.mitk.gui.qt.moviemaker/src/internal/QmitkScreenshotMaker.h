/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkScreenshotMaker_h
#define QmitkScreenshotMaker_h

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include <QColor>
#include <QList>
#include <QPointer>
#include <QString>

#include <memory>

namespace Ui
{
  class QmitkScreenshotMakerControls;
}

class QmitkRenderWindow;
class QAbstractButton;
class QButtonGroup;
class QEvent;
class QObject;
class QShortcut;
class vtkRenderer;

/**
 * \brief View for taking screenshots of render windows.
 *
 * Works with any editor that provides render windows via mitk::IRenderWindowPart
 * (Standard Display and MxN Display). Screenshots can be taken of a single render
 * window picked by clicking it, or of all render windows of the active editor at
 * once. An optional upsampling factor produces higher-resolution screenshots and
 * the background color of the screenshot can be chosen (including transparent).
 */
class QmitkScreenshotMaker : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  QmitkScreenshotMaker(QObject *parent = nullptr, const char *name = nullptr);
  ~QmitkScreenshotMaker() override;

  void CreateQtPartControl(QWidget *parent) override;
  void SetFocus() override;

  void CreateConnections();

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart) override;

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

protected slots:

  void OnScreenshotWindow();
  void OnScreenshotAll();

private:
  /** \brief Arm pick mode: the next click on a render window triggers a screenshot of it. */
  void ArmPickMode();

  /** \brief Leave pick mode and restore cursor, button and status bar. */
  void DisarmPickMode();

  /** \brief Ask for a file name and take a screenshot of the given render window. */
  void CaptureSingleWindow(QmitkRenderWindow* window);

  /** \brief Create the exclusive background color buttons and select the default. */
  void SetupBackgroundButtons();

  /** \brief Remember the chosen screenshot background (color, or transparent). */
  void ApplyBackgroundSelection(const QColor& color, bool transparent, QAbstractButton* button);

  /** \brief Open a color dialog for the custom background; restore selection on cancel. */
  void OnSelectCustomBackground();

  /*!
  \brief Take a screenshot of the given render window on the chosen background.
  \param window the render window to capture
  \param scale upsampling factor (1 = on-screen resolution)
  \param fileName file location and name where the screenshot is saved
  \param filter selected file-type filter, used to derive the suffix when missing
  */
  void TakeScreenshot(QmitkRenderWindow* window, unsigned int scale, const QString& fileName, const QString& filter = QString());

  /*!
  \brief Write a screenshot with a transparent background.

  vtkRenderLargeImage produces opaque RGB and MITK's render windows have no alpha
  buffer, so alpha is recovered by rendering the scene over black and over white
  and computing the per-pixel opacity from the difference.
  */
  void WriteTransparentScreenshot(vtkRenderer* renderer, unsigned int scale, const QString& fileName);

  QWidget* m_Parent = nullptr;
  std::unique_ptr<Ui::QmitkScreenshotMakerControls> m_Controls;

  bool m_PickArmed = false;
  QList<QPointer<QmitkRenderWindow>> m_FilteredWindows;
  QList<QPointer<QWidget>> m_PickOverlays;
  QShortcut* m_CancelPickShortcut = nullptr;

  QButtonGroup* m_BackgroundButtonGroup = nullptr;
  QAbstractButton* m_CurrentBackgroundButton = nullptr;
  QColor m_BackgroundColor = Qt::black;
  QColor m_CustomColor = Qt::gray;
  bool m_TransparentBackground = false;

  QString m_LastPath;
  QString m_LastFile;
  QString m_PNGExtension = "PNG File (*.png)";
  QString m_JPGExtension = "JPEG File (*.jpg)";
};

#endif
