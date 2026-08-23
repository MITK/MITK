/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkProgressNotification.h>
#include <ui_QmitkProgressNotification.h>

#include <QmitkIconTheme.h>

#include <QColor>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

#include <algorithm>
#include <limits>

namespace
{
  constexpr int LINGER_DURATION_IN_MS = 1500;

  /**
   * How long a task may show no progress at all before its bar starts
   * spinning. A bar sitting at zero is indistinguishable from one that is
   * stuck.
   */
  constexpr qint64 SPIN_DELAY_IN_MS = 1000;
  constexpr int FADE_DURATION_IN_MS = 300;

  QString CardStyleSheet()
  {
    // The dark theme replaces widget colors wholesale in its style sheet and
    // leaves the palette untouched, so palette() is a reliable source of
    // colors in the light theme only. The theme is identified by the icon
    // color it publishes, which is light in the dark theme and vice versa.
    const auto darkTheme = QColor(QmitkIconTheme::GetColor()).lightness() > 127;

    const auto surface = darkTheme ? QStringLiteral("#3f3f46") : QStringLiteral("palette(base)");
    const auto border = darkTheme ? QStringLiteral("#54545a") : QStringLiteral("palette(mid)");
    const auto hover = darkTheme ? QStringLiteral("#54545a") : QStringLiteral("palette(midlight)");

    return QStringLiteral(
      "#QmitkProgressNotification { background-color: %1; border: 1px solid %2; border-radius: 4px; }"
      "#QmitkProgressNotification QLabel { background-color: transparent; }"
      "#QmitkProgressNotification QToolButton { background-color: transparent; border: none; border-radius: 2px; }"
      "#QmitkProgressNotification QToolButton:hover { background-color: %3; }").arg(surface, border, hover);
  }

  int ToProgressBarValue(unsigned int value)
  {
    return static_cast<int>(std::min<unsigned int>(value, std::numeric_limits<int>::max()));
  }
}

QmitkProgressNotification::QmitkProgressNotification(const mitk::ProgressTaskInfo& info, QWidget* parent)
  : QWidget(parent),
    m_Controls(new Ui::QmitkProgressNotification),
    m_Info(info),
    m_Name(QString::fromStdString(info.Name))
{
  m_Controls->setupUi(this);

  m_SinceCreation.start();

  this->setObjectName(QStringLiteral("QmitkProgressNotification"));

  // Qt draws no style sheet background for a plain QWidget subclass unless
  // it is asked to, which would leave the card without its background and
  // border and let the render windows show through it.
  this->setAttribute(Qt::WA_StyledBackground, true);

  // The card colors are baked into a style sheet, so unlike the icons they do
  // not follow a theme switch on their own.
  const auto updateCardStyle = [this]
  {
    this->setStyleSheet(CardStyleSheet());
  };

  updateCardStyle();
  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, updateCardStyle);

  m_Controls->closeButton->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/times.svg")));

  connect(m_Controls->closeButton, &QToolButton::clicked, this, &QmitkProgressNotification::OnCloseButtonClicked);

  this->ApplyState();

  // A task that goes quiet rather than reporting would otherwise never be
  // re-evaluated, as the state is only recomputed when a snapshot arrives.
  QTimer::singleShot(SPIN_DELAY_IN_MS, this, [this]()
    {
      if (!m_Info.Finished)
        this->ApplyState();
    });
}

QmitkProgressNotification::~QmitkProgressNotification()
{
  delete m_Controls;
}

mitk::ProgressTaskId QmitkProgressNotification::GetTaskId() const
{
  return m_Info.Id;
}

void QmitkProgressNotification::Update(const mitk::ProgressTaskInfo& info)
{
  m_Info = info;
  m_Name = QString::fromStdString(info.Name);

  this->ApplyState();
}

void QmitkProgressNotification::Finish(const mitk::ProgressTaskInfo& info)
{
  this->Update(info);

  m_Controls->closeButton->setEnabled(false);

  QTimer::singleShot(LINGER_DURATION_IN_MS, this, &QmitkProgressNotification::FadeOut);
}

void QmitkProgressNotification::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  this->UpdateNameLabel();
}

void QmitkProgressNotification::OnCloseButtonClicked()
{
  if (m_Info.Cancelable && !m_Info.CancelRequested)
  {
    emit CancelRequested(m_Info.Id);
    return;
  }

  emit Closed(m_Info.Id);
}

bool QmitkProgressNotification::ShouldSpin() const
{
  // A task whose extent is unknown can only ever spin.
  if (0 == m_Info.StepsToDo)
    return true;

  return 0 == m_Info.Progress
      && !m_Info.Finished
      && m_SinceCreation.elapsed() >= SPIN_DELAY_IN_MS;
}

void QmitkProgressNotification::ApplyState()
{
  this->UpdateNameLabel();

  if (this->ShouldSpin())
  {
    m_Controls->progressBar->setRange(0, 0);
  }
  else
  {
    m_Controls->progressBar->setRange(0, ToProgressBarValue(m_Info.StepsToDo));
    m_Controls->progressBar->setValue(ToProgressBarValue(m_Info.Progress));
  }

  m_Controls->closeButton->setEnabled(!m_Info.CancelRequested);
  m_Controls->closeButton->setToolTip(m_Info.Cancelable
    ? QStringLiteral("Cancel this operation")
    : QStringLiteral("Hide this notification. The operation keeps running."));
}

void QmitkProgressNotification::UpdateNameLabel()
{
  const auto text = m_Info.CancelRequested
    ? QStringLiteral("%1 (cancelling)").arg(m_Name)
    : m_Name;

  const auto metrics = m_Controls->nameLabel->fontMetrics();

  m_Controls->nameLabel->setText(metrics.elidedText(text, Qt::ElideRight, m_Controls->nameLabel->width()));
  m_Controls->nameLabel->setToolTip(text);
}

void QmitkProgressNotification::FadeOut()
{
  auto* effect = new QGraphicsOpacityEffect(this);
  this->setGraphicsEffect(effect);

  auto* animation = new QPropertyAnimation(effect, "opacity", this);
  animation->setDuration(FADE_DURATION_IN_MS);
  animation->setStartValue(1.0);
  animation->setEndValue(0.0);

  connect(animation, &QAbstractAnimation::finished, this, [this]() {
    emit Closed(m_Info.Id);
  });

  animation->start(QAbstractAnimation::DeleteWhenStopped);
}
