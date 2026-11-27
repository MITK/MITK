/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkInfoCard.h>

#include <QmitkIconTheme.h>

#include <mitkExceptionMacro.h>

#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

namespace
{
  QByteArray ReadResource(const QString& path)
  {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
      mitkThrow() << "Could not open resource \"" << path.toStdString() << "\"!";

    return file.readAll();
  }
}

QmitkInfoCard::QmitkInfoCard(QWidget* parent)
  : QFrame(parent),
    m_IconLabel(new QLabel(this)),
    m_MessageLabel(new QLabel(this)),
    m_CloseButton(new QToolButton(this))
{
  m_MessageLabel->setTextFormat(Qt::RichText);
  m_MessageLabel->setWordWrap(true);
  m_MessageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  // Next to the icon also if the card gets more height than the message needs.
  m_MessageLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  connect(m_MessageLabel, &QLabel::linkActivated, this, &QmitkInfoCard::LinkActivated);

  // Like the close button of the progress notification cards.
  m_CloseButton->setFixedSize(18, 18);
  m_CloseButton->setIconSize(QSize(12, 12));
  m_CloseButton->setAutoRaise(true);
  m_CloseButton->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/times.svg")));
  m_CloseButton->setToolTip("Close");
  m_CloseButton->setAccessibleName("Close");
  m_CloseButton->hide();

  connect(m_CloseButton, &QToolButton::clicked, this, [this]() {
    this->hide();
    emit this->Closed();
  });

  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(8, 6, 8, 6);
  layout->setSpacing(8);
  layout->addWidget(m_IconLabel, 0, Qt::AlignTop);
  layout->addWidget(m_MessageLabel);
  layout->addWidget(m_CloseButton, 0, Qt::AlignTop);

  this->UpdateStyle();

  // Neither the style sheet nor the pixmap of the icon follows a theme switch on its own.
  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, &QmitkInfoCard::UpdateStyle);
}

QmitkInfoCard::~QmitkInfoCard() = default;

void QmitkInfoCard::SetMessage(const QString& message, Severity severity)
{
  m_MessageLabel->setText(message);

  if (severity != m_Severity)
  {
    m_Severity = severity;
    this->UpdateStyle();
  }
}

QString QmitkInfoCard::GetMessage() const
{
  return m_MessageLabel->text();
}

QmitkInfoCard::Severity QmitkInfoCard::GetSeverity() const
{
  return m_Severity;
}

void QmitkInfoCard::SetClosable(bool closable)
{
  m_CloseButton->setVisible(closable);
}

bool QmitkInfoCard::IsClosable() const
{
  return !m_CloseButton->isHidden();
}

void QmitkInfoCard::UpdateStyle()
{
  const auto darkTheme = QmitkIconTheme::IsDarkTheme();

  QString surface;
  QString border;
  QString hover;
  QIcon icon;

  if (m_Severity == Severity::Error)
  {
    // Not the icon color of the theme, so that an error stands out in either theme.
    surface = darkTheme ? QStringLiteral("#4a2b2e") : QStringLiteral("#fdecea");
    border = darkTheme ? QStringLiteral("#8c3b42") : QStringLiteral("#e6a19c");
    hover = border;
    icon = QmitkIconTheme::GetIcon(ReadResource(QStringLiteral(":/Qmitk/error.svg")), QmitkIconTheme::GetErrorColor());
  }
  else if (m_Severity == Severity::Warning)
  {
    surface = darkTheme ? QStringLiteral("#4a3d24") : QStringLiteral("#fff4e0");
    border = darkTheme ? QStringLiteral("#8c6d2e") : QStringLiteral("#f0c36d");
    hover = border;
    icon = QmitkIconTheme::GetIcon(ReadResource(QStringLiteral(":/Qmitk/error.svg")), QmitkIconTheme::GetWarningColor());
  }
  else
  {
    // In the colors of the progress notification cards.
    surface = darkTheme ? QStringLiteral("#3f3f46") : QStringLiteral("palette(base)");
    border = darkTheme ? QStringLiteral("#54545a") : QStringLiteral("palette(mid)");
    hover = darkTheme ? QStringLiteral("#54545a") : QStringLiteral("palette(midlight)");
    icon = QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/info.svg"));
  }

  this->setStyleSheet(QStringLiteral(
    "QmitkInfoCard { background-color: %1; border: 1px solid %2; border-radius: 4px; }"
    "QmitkInfoCard QLabel { background-color: transparent; border: none; }"
    "QmitkInfoCard QToolButton { background-color: transparent; border: none; border-radius: 2px; }"
    "QmitkInfoCard QToolButton:hover { background-color: %3; }").arg(surface, border, hover));

  m_IconLabel->setPixmap(icon.pixmap(QSize(16, 16), m_IconLabel->devicePixelRatioF()));
}
