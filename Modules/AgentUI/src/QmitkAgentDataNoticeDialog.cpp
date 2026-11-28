/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAgentDataNoticeDialog.h"

#include <QmitkIconTheme.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
  constexpr int HeaderIconSize = 40;
  constexpr int GlyphSize = 32;
  constexpr int DialogWidth = 500;
  constexpr int SideMargin = 18;

  // The themes style widgets through the application style sheet only, so
  // the palette cannot tell light from dark. These match the feedback dialog.
  QString MutedColor()
  {
    return QmitkIconTheme::IsDarkTheme() ? "#a6a6a6" : "#5f5f5f";
  }

  QWidget* CreateHeader(const QIcon& icon, qreal ratio)
  {
    auto* header = new QFrame;
    header->setObjectName("agentDataNoticeHeader");
    header->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    // Translucent grey instead of a palette color reads as a band on the
    // light and the dark theme, like the headers of the feedback and crash
    // dump dialogs.
    header->setStyleSheet(QString(
      "QFrame#agentDataNoticeHeader { background-color: rgba(128, 128, 128, 28); border: none; "
      "border-left: 4px solid %1; border-bottom: 1px solid rgba(128, 128, 128, 90); }")
      .arg(QmitkIconTheme::GetAccentColor()));

    auto* title = new QLabel("<h3 style=\"margin: 0;\">Before you use the agent</h3>");
    title->setTextFormat(Qt::RichText);

    auto* subtitle = new QLabel("The agent does not run on this computer.");
    subtitle->setWordWrap(true);

    auto* text = new QVBoxLayout;
    text->setSpacing(2);
    text->addWidget(title);
    text->addWidget(subtitle);

    auto* layout = new QHBoxLayout(header);
    layout->setContentsMargins(14, 12, 16, 12);
    layout->setSpacing(14);

    if (!icon.isNull())
    {
      auto* iconLabel = new QLabel;
      iconLabel->setPixmap(icon.pixmap(QSize(HeaderIconSize, HeaderIconSize), ratio));
      iconLabel->setFixedSize(HeaderIconSize, HeaderIconSize);
      layout->addWidget(iconLabel, 0, Qt::AlignVCenter);
    }

    layout->addLayout(text, 1);

    return header;
  }

  QLayout* CreateRow(const QString& glyph, const QString& title, const QString& description, qreal ratio)
  {
    auto* icon = new QLabel;
    icon->setPixmap(QmitkIconTheme::GetIcon(":/AgentUI/DataNotice/" + glyph + ".svg")
      .pixmap(QSize(GlyphSize, GlyphSize), ratio));

    auto* titleLabel = new QLabel(QString("<b>%1</b>").arg(title));
    titleLabel->setTextFormat(Qt::RichText);

    auto* descriptionLabel = new QLabel(QString("<p style=\"line-height: 1.25; margin: 0;\">%1</p>").arg(description));
    descriptionLabel->setTextFormat(Qt::RichText);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setStyleSheet(QString("color: %1;").arg(MutedColor()));

    auto* text = new QVBoxLayout;
    text->setSpacing(3);
    text->addWidget(titleLabel);
    text->addWidget(descriptionLabel);

    auto* row = new QHBoxLayout;
    row->setSpacing(14);
    row->addWidget(icon, 0, Qt::AlignTop);
    row->addLayout(text, 1);

    return row;
  }
}

QmitkAgentDataNoticeDialog::QmitkAgentDataNoticeDialog(Mode mode, QWidget* parent)
  : QDialog(parent)
{
  this->setWindowTitle("Agent");

  // Without a native window yet, the dialog cannot know the screen it opens
  // on. It opens over the Workbench, whose window is on that screen.
  const auto ratio = parent != nullptr ? parent->window()->devicePixelRatioF() : this->devicePixelRatioF();
  const auto windowIcon = parent != nullptr ? parent->window()->windowIcon() : QIcon();

  auto* rows = new QVBoxLayout;
  rows->setContentsMargins(SideMargin, 16, SideMargin, 16);
  rows->setSpacing(16);

  rows->addLayout(CreateRow("sent", "Your data goes to your provider",
    "Messages, data properties, DICOM tags and screenshots are sent to the provider of your Claude Code "
    "account, usually Anthropic. The German Cancer Research Center (DKFZ) receives none of it.", ratio));

  rows->addLayout(CreateRow("identity", "Send only what you are allowed to",
    "Names, DICOM tags and screenshots can identify a patient. The laws that apply to you and the terms of "
    "your account decide what you may send.", ratio));

  rows->addLayout(CreateRow("research", "For research only",
    "Not for diagnosis or treatment of patients. Provided as is, without warranty, for use on your own "
    "responsibility.", ratio));

  auto* divider = new QFrame;
  divider->setFixedHeight(1);
  divider->setStyleSheet("background-color: rgba(128, 128, 128, 70); border: none;");

  auto* buttons = new QDialogButtonBox;
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto* footer = new QVBoxLayout;
  footer->setContentsMargins(SideMargin, 14, SideMargin, 14);
  footer->setSpacing(14);

  if (mode == Mode::Ask)
  {
    auto* commitment = new QCheckBox("I will use the agent only on data that I am allowed to send.");

    auto* acceptButton = buttons->addButton("I Understand", QDialogButtonBox::AcceptRole);
    auto* cancelButton = buttons->addButton(QDialogButtonBox::Cancel);
    acceptButton->setEnabled(false);
    cancelButton->setDefault(true);

    // Enter accepts only once the commitment is ticked.
    connect(commitment, &QCheckBox::toggled, this, [acceptButton, cancelButton](bool checked) {
      acceptButton->setEnabled(checked);
      acceptButton->setDefault(checked);
      cancelButton->setDefault(!checked);
    });

    footer->addWidget(commitment);
  }
  else
  {
    buttons->addButton(QDialogButtonBox::Close)->setDefault(true);
  }

  footer->addWidget(buttons);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(CreateHeader(windowIcon, ratio));
  layout->addLayout(rows);
  // Takes the spare height of an enlarged dialog, which would otherwise pull
  // titles, descriptions and glyphs apart.
  layout->addStretch(1);
  layout->addWidget(divider);
  layout->addLayout(footer);

  // The minimum height that the layout would set is the one for its minimum
  // width, at which the descriptions wrap to more lines, and would leave empty
  // space above the divider. Measured after polishing, which applies the fonts
  // of the style sheet.
  this->ensurePolished();
  const auto height = this->heightForWidth(DialogWidth);
  this->setMinimumSize(DialogWidth, height);
  this->resize(DialogWidth, height);
}
