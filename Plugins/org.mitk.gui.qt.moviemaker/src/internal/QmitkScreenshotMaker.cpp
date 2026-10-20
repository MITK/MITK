/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkScreenshotMaker.h"
#include <ui_QmitkScreenshotMakerControls.h>

#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowScreenshot.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkIRenderWindowPart.h>
#include <mitkLog.h>
#include <mitkStatusBar.h>
#include <mitkWorkbenchUtil.h>

#include <QAbstractButton>
#include <QApplication>
#include <QButtonGroup>
#include <QColorDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QShortcut>
#include <QTimer>
#include <QWidget>

#include <algorithm>

namespace
{
  QIcon CreateColorSwatch(const QColor& color, int size = 20)
  {
    QPixmap pixmap(size, size);
    pixmap.fill(color);
    return QIcon(pixmap);
  }

  QIcon CreateTransparentSwatch(int size = 20)
  {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    const int half = size / 2;
    const QColor gray(200, 200, 200);
    painter.fillRect(0, 0, half, half, gray);
    painter.fillRect(half, half, size - half, size - half, gray);
    painter.end();
    return QIcon(pixmap);
  }

  /** Return the first "<baseName>_NNN.png" in the directory that does not exist yet. */
  QString NextAvailableScreenshotName(const QString& directory, const QString& baseName)
  {
    for (int index = 1; index <= 9999; ++index)
    {
      const QString candidate = directory + "/" + baseName + "_" + QString::number(index).rightJustified(3, '0') + ".png";
      if (!QFileInfo::exists(candidate))
        return candidate;
    }

    return directory + "/" + baseName + ".png";
  }

  /**
   * Overlay drawn on a render window while pick mode is armed: dims the window
   * (spotlight effect) unless it is the one currently hovered, which instead gets
   * a bright accent border. It is transparent to mouse events, so the click still
   * reaches the render window, and follows the render window's size.
   */
  class ScreenshotPickOverlay : public QWidget
  {
  public:
    explicit ScreenshotPickOverlay(QWidget* parent)
      : QWidget(parent)
    {
      this->setAttribute(Qt::WA_TransparentForMouseEvents);
      this->setAttribute(Qt::WA_NoSystemBackground);
      this->setFocusPolicy(Qt::NoFocus);

      if (nullptr != parent)
      {
        parent->installEventFilter(this);
        this->setGeometry(parent->rect());
      }
    }

    void SetHighlighted(bool highlighted)
    {
      if (m_Highlighted != highlighted)
      {
        m_Highlighted = highlighted;
        this->update();
      }
    }

  protected:
    void paintEvent(QPaintEvent*) override
    {
      QPainter painter(this);

      if (m_Highlighted)
      {
        painter.setPen(QPen(QColor(255, 255, 255), 4));
        painter.drawRect(this->rect().adjusted(2, 2, -2, -2));
      }
      else
      {
        painter.fillRect(this->rect(), QColor(0, 0, 0, 120));
      }
    }

    bool eventFilter(QObject* watched, QEvent* event) override
    {
      if (watched == this->parentWidget() && QEvent::Resize == event->type())
        this->setGeometry(this->parentWidget()->rect());

      return QWidget::eventFilter(watched, event);
    }

  private:
    bool m_Highlighted = false;
  };
}

QmitkScreenshotMaker::QmitkScreenshotMaker(QObject * /*parent*/, const char * /*name*/)
{
}

QmitkScreenshotMaker::~QmitkScreenshotMaker()
{
  // Minimal cleanup without touching the (possibly torn-down) UI widgets.
  if (m_PickArmed)
  {
    for (const auto& window : m_FilteredWindows)
    {
      if (window)
        window->removeEventFilter(this);
    }
    for (const auto& overlay : m_PickOverlays)
    {
      if (overlay)
        delete overlay;
    }
    QApplication::restoreOverrideCursor();
  }
}

void QmitkScreenshotMaker::CreateConnections()
{
  if (m_Controls)
  {
    // clicked() (not toggled()) so programmatic setChecked() during auto-disarm does not recurse.
    connect(m_Controls->btnScreenshotWindow, &QPushButton::clicked, this, &QmitkScreenshotMaker::OnScreenshotWindow);
    connect(m_Controls->btnScreenshotAll, &QPushButton::clicked, this, &QmitkScreenshotMaker::OnScreenshotAll);

    connect(m_Controls->btnBgWhite, &QAbstractButton::clicked, this,
            [this] { this->ApplyBackgroundSelection(Qt::white, false, m_Controls->btnBgWhite); });
    connect(m_Controls->btnBgBlack, &QAbstractButton::clicked, this,
            [this] { this->ApplyBackgroundSelection(Qt::black, false, m_Controls->btnBgBlack); });
    connect(m_Controls->btnBgTransparent, &QAbstractButton::clicked, this,
            [this] { this->ApplyBackgroundSelection(QColor(), true, m_Controls->btnBgTransparent); });
    connect(m_Controls->btnBgCustom, &QAbstractButton::clicked, this, &QmitkScreenshotMaker::OnSelectCustomBackground);

    connect(m_Controls->checkCropToContent, &QCheckBox::toggled, m_Controls->spinCropPadding, &QWidget::setEnabled);
  }
}

void QmitkScreenshotMaker::SetupBackgroundButtons()
{
  const QSize iconSize(20, 20);
  for (auto* button : {m_Controls->btnBgWhite, m_Controls->btnBgBlack, m_Controls->btnBgTransparent, m_Controls->btnBgCustom})
    button->setIconSize(iconSize);

  m_Controls->btnBgWhite->setIcon(CreateColorSwatch(Qt::white));
  m_Controls->btnBgBlack->setIcon(CreateColorSwatch(Qt::black));
  m_Controls->btnBgTransparent->setIcon(CreateTransparentSwatch());
  m_Controls->btnBgCustom->setIcon(CreateColorSwatch(m_CustomColor));

  m_BackgroundButtonGroup = new QButtonGroup(this);
  m_BackgroundButtonGroup->setExclusive(true);
  m_BackgroundButtonGroup->addButton(m_Controls->btnBgWhite);
  m_BackgroundButtonGroup->addButton(m_Controls->btnBgBlack);
  m_BackgroundButtonGroup->addButton(m_Controls->btnBgTransparent);
  m_BackgroundButtonGroup->addButton(m_Controls->btnBgCustom);

  m_Controls->btnBgBlack->setChecked(true);
  this->ApplyBackgroundSelection(Qt::black, false, m_Controls->btnBgBlack);
}

void QmitkScreenshotMaker::ApplyBackgroundSelection(const QColor& color, bool transparent, QAbstractButton* button)
{
  m_BackgroundColor = color;
  m_TransparentBackground = transparent;
  m_CurrentBackgroundButton = button;
}

void QmitkScreenshotMaker::OnSelectCustomBackground()
{
  const QColor color = QColorDialog::getColor(m_CustomColor, m_Parent, "Select screenshot background color");

  if (color.isValid())
  {
    m_CustomColor = color;
    m_Controls->btnBgCustom->setIcon(CreateColorSwatch(color));
    this->ApplyBackgroundSelection(color, false, m_Controls->btnBgCustom);
  }
  else if (nullptr != m_CurrentBackgroundButton)
  {
    // Dialog cancelled: restore the previously selected background button.
    m_CurrentBackgroundButton->setChecked(true);
  }
}

void QmitkScreenshotMaker::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    m_Parent = parent;
    m_Controls = std::make_unique<Ui::QmitkScreenshotMakerControls>();
    m_Controls->setupUi(parent);

    const int buttonHeight = static_cast<int>(m_Controls->btnScreenshotWindow->sizeHint().height() * 1.5);
    m_Controls->btnScreenshotWindow->setMinimumHeight(buttonHeight);
    m_Controls->btnScreenshotAll->setMinimumHeight(buttonHeight);

    // Esc cancels pick mode. A render window does not have keyboard focus while
    // arming, so an application-context shortcut is used instead of a key event.
    m_CancelPickShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), m_Parent);
    m_CancelPickShortcut->setContext(Qt::ApplicationShortcut);
    m_CancelPickShortcut->setEnabled(false);
    connect(m_CancelPickShortcut, &QShortcut::activated, this, &QmitkScreenshotMaker::DisarmPickMode);

    this->SetupBackgroundButtons();
    this->CreateConnections();
  }

  m_Parent->setEnabled(nullptr != this->GetRenderWindowPart());
}

void QmitkScreenshotMaker::SetFocus()
{
  m_Controls->btnScreenshotWindow->setFocus();
}

void QmitkScreenshotMaker::OnScreenshotWindow()
{
  if (m_PickArmed)
    this->DisarmPickMode();
  else
    this->ArmPickMode();
}

void QmitkScreenshotMaker::ArmPickMode()
{
  if (auto* renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN))
  {
    const auto renderWindows = renderWindowPart->GetQmitkRenderWindows();
    for (auto* window : renderWindows)
    {
      if (nullptr != window)
      {
        window->installEventFilter(this);
        m_FilteredWindows.append(QPointer<QmitkRenderWindow>(window));

        auto* overlay = new ScreenshotPickOverlay(window);
        overlay->show();
        overlay->raise();
        m_PickOverlays.append(overlay);
      }
    }
  }

  if (m_FilteredWindows.isEmpty())
  {
    m_Controls->btnScreenshotWindow->setChecked(false);
    return;
  }

  m_PickArmed = true;
  m_Controls->btnScreenshotWindow->setChecked(true);
  m_Controls->labelPickHint->setVisible(true);
  m_CancelPickShortcut->setEnabled(true);
  QApplication::setOverrideCursor(Qt::CrossCursor);
  mitk::StatusBar::GetInstance()->DisplayText("Click a display window to take a screenshot of it (Esc to cancel).");
}

void QmitkScreenshotMaker::DisarmPickMode()
{
  if (!m_PickArmed)
    return;

  for (const auto& window : m_FilteredWindows)
  {
    if (window)
      window->removeEventFilter(this);
  }
  m_FilteredWindows.clear();

  for (const auto& overlay : m_PickOverlays)
  {
    if (overlay)
      overlay->deleteLater();
  }
  m_PickOverlays.clear();

  m_PickArmed = false;
  m_Controls->btnScreenshotWindow->setChecked(false);
  m_Controls->labelPickHint->setVisible(false);
  m_CancelPickShortcut->setEnabled(false);
  QApplication::restoreOverrideCursor();
  mitk::StatusBar::GetInstance()->Clear();
}

bool QmitkScreenshotMaker::eventFilter(QObject* watched, QEvent* event)
{
  if (m_PickArmed)
  {
    if (QEvent::Enter == event->type() || QEvent::Leave == event->type())
    {
      // Highlight the hovered window (Enter) and dim the rest; on Leave, dim all.
      auto* hoveredWindow = (QEvent::Enter == event->type()) ? qobject_cast<QmitkRenderWindow*>(watched) : nullptr;
      for (const auto& overlay : m_PickOverlays)
      {
        if (overlay)
          static_cast<ScreenshotPickOverlay*>(overlay.data())->SetHighlighted(overlay->parentWidget() == hoveredWindow);
      }
    }
    else if (QEvent::MouseButtonPress == event->type())
    {
      if (Qt::LeftButton == static_cast<QMouseEvent*>(event)->button())
      {
        auto* window = qobject_cast<QmitkRenderWindow*>(watched);
        this->DisarmPickMode();

        if (nullptr != window)
        {
          // Defer so the current event unwinds before the modal save dialog and re-render.
          QPointer<QmitkRenderWindow> windowPointer(window);
          QTimer::singleShot(0, this, [this, windowPointer]()
          {
            if (windowPointer)
              this->CaptureSingleWindow(windowPointer);
          });
        }
      }
      else
      {
        this->DisarmPickMode();
      }

      return true; // consume the press so the crosshair does not move
    }
    else if (QEvent::KeyPress == event->type() && Qt::Key_Escape == static_cast<QKeyEvent*>(event)->key())
    {
      this->DisarmPickMode();
      return true;
    }
  }

  return QmitkAbstractView::eventFilter(watched, event);
}

void QmitkScreenshotMaker::CaptureSingleWindow(QmitkRenderWindow* window)
{
  if (nullptr == window)
    return;

  const QString directory = m_LastFile.isEmpty()
    ? QDir::currentPath()
    : QFileInfo(m_LastFile).absolutePath();

  QString filter;
  const QString fileName = QFileDialog::getSaveFileName(
    nullptr, "Save screenshot to...", NextAvailableScreenshotName(directory, "screenshot"), m_PNGExtension + ";;" + m_JPGExtension, &filter);

  if (fileName.isEmpty())
    return;

  m_LastFile = fileName;
  this->TakeScreenshot(window, static_cast<unsigned int>(m_Controls->m_MagFactor->value()), fileName, filter);
}

void QmitkScreenshotMaker::OnScreenshotAll()
{
  auto* renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN);
  if (nullptr == renderWindowPart)
    return;

  if (m_LastPath.isEmpty())
    m_LastPath = QDir::currentPath();

  const QString directory = QFileDialog::getExistingDirectory(nullptr, "Save screenshots to...", m_LastPath);
  if (directory.isEmpty())
    return;

  m_LastPath = directory;

  const auto renderWindows = renderWindowPart->GetQmitkRenderWindows();
  const auto scale = static_cast<unsigned int>(m_Controls->m_MagFactor->value());

  for (auto it = renderWindows.constBegin(); it != renderWindows.constEnd(); ++it)
  {
    auto* window = it.value();
    if (nullptr == window)
      continue;

    QString baseName = it.key();
    if (auto* renderWindowWidget = dynamic_cast<QmitkRenderWindowWidget*>(window->parentWidget()))
    {
      const auto cornerAnnotation = renderWindowWidget->GetCornerAnnotationText();
      if (!cornerAnnotation.empty())
        baseName = QString::fromStdString(cornerAnnotation);
    }
    baseName.replace(QRegularExpression("[^\\w.-]"), "_");

    this->TakeScreenshot(window, scale, NextAvailableScreenshotName(directory, baseName), m_PNGExtension);
  }
}

void QmitkScreenshotMaker::TakeScreenshot(QmitkRenderWindow* window, unsigned int scale, const QString& fileName, const QString& filter)
{
  if (nullptr == window || scale < 1 || fileName.isEmpty())
    return;

  auto* renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN);
  if (nullptr == renderWindowPart)
    return;

  // Resolve the output file name and format. Transparency requires PNG (alpha).
  QString outputFileName = fileName;
  QString suffix = QFileInfo(outputFileName).suffix().toLower();

  bool useJpeg = false;
  if (!m_TransparentBackground)
  {
    if ("jpg" == suffix || "jpeg" == suffix)
      useJpeg = true;
    else if ("png" != suffix && filter == m_JPGExtension)
      useJpeg = true;
  }

  if ("png" != suffix && "jpg" != suffix && "jpeg" != suffix)
  {
    suffix = useJpeg ? "jpg" : "png";
    outputFileName += "." + suffix;
  }
  else if (m_TransparentBackground && "png" != suffix)
  {
    outputFileName.chop(suffix.length());
    outputFileName += "png";
  }

  QmitkRenderWindowScreenshot::Options options;
  options.Magnification = scale;
  options.Decorations = false;
  options.BackgroundMode = m_TransparentBackground
    ? QmitkRenderWindowScreenshot::Background::Transparent
    : QmitkRenderWindowScreenshot::Background::Color;
  options.BackgroundColor = m_BackgroundColor;
  options.CropToContent = m_Controls->checkCropToContent->isChecked();
  options.Padding = static_cast<unsigned int>(m_Controls->spinCropPadding->value());

  const auto image = QmitkRenderWindowScreenshot::Take(renderWindowPart, window, options);

  if (image.isNull() && options.CropToContent)
  {
    MITK_WARN << "Did not save the screenshot " << outputFileName.toStdString() << ": the window shows only background, there is nothing to crop to.";
    return;
  }

  if (image.isNull() || !image.save(outputFileName, useJpeg ? "JPG" : "PNG", useJpeg ? 100 : -1))
    MITK_ERROR << "Cannot save the screenshot " << outputFileName.toStdString();
}

void QmitkScreenshotMaker::RenderWindowPartActivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  m_Parent->setEnabled(true);
}

void QmitkScreenshotMaker::RenderWindowPartInputChanged(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  // The set of render windows may have changed (e.g. an MxN relayout); cancel a pending pick.
  this->DisarmPickMode();
}

void QmitkScreenshotMaker::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  this->DisarmPickMode();
  m_Parent->setEnabled(false);
}
