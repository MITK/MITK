/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkScreenshotWidget.h"
#include <ui_QmitkScreenshotWidget.h>

#include <mitkLogMacros.h>

#include <QFileInfo>

using Self = QmitkScreenshotWidget;

QmitkScreenshotWidget::QmitkScreenshotWidget(QWidget* parent)
  : QWidget(parent),
    m_Ui(new Ui::QmitkScreenshotWidget)
{
  m_Ui->setupUi(this);

  connect(m_Ui->removeButton, &QPushButton::clicked, this, &Self::OnRemoveButtonClicked);
}

QmitkScreenshotWidget::QmitkScreenshotWidget(const QString& screenshotPath, QWidget* parent)
  : QmitkScreenshotWidget(parent)
{
  this->SetScreenshot(screenshotPath);
}

QmitkScreenshotWidget::~QmitkScreenshotWidget()
{
}

void QmitkScreenshotWidget::SetScreenshot(const QString& path)
{
  QFileInfo fileInfo(path);

  if (!fileInfo.exists())
  {
    MITK_ERROR << "File \"" << path.toStdString() << "\" does not exist!";
    return;
  }

  QPixmap pixmap(path);

  if (pixmap.isNull())
  {
    MITK_ERROR << "File \"" << path.toStdString() << "\" is not a valid image!";
    return;
  }

  m_ScreenshotPath = path;

  m_Ui->thumbnailLabel->setPixmap(pixmap.scaled(m_Ui->thumbnailLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

  m_Ui->fileNameLabel->setText(fileInfo.fileName());

  m_Ui->detailsLabel->setText(QString("<i>%1x%2 pixels, %3 KB</i>")
    .arg(pixmap.width())
    .arg(pixmap.height())
    .arg(fileInfo.size() / 1024));
}

QString QmitkScreenshotWidget::GetScreenshotPath() const
{
  return m_ScreenshotPath;
}

void QmitkScreenshotWidget::OnRemoveButtonClicked()
{
  emit Remove(this);
}
