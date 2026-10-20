/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include <QmitkMemoryUsageIndicatorView.h>

#include <mitkMemoryUtilities.h>

#include <QLocale>
#include <QTimer>

#include <QmitkMemoryUsageIndicatorImagesGreen.xpm>
#include <QmitkMemoryUsageIndicatorImagesOrange.xpm>
#include <QmitkMemoryUsageIndicatorImagesRed.xpm>
#include <QmitkMemoryUsageIndicatorImagesYellow.xpm>

#include <ui_QmitkMemoryUsageIndicator.h>

QmitkMemoryUsageIndicatorView::QmitkMemoryUsageIndicatorView(QWidget*)
  : m_Ui(std::make_unique<Ui::QmitkMemoryUsageIndicator>()),
    m_PreviousState(0),
    m_States {
      std::make_pair(0.0f, QPixmap(QmitkMemoryUsageIndicatorImagesGreen_xpm)),
      std::make_pair(50.0f, QPixmap(QmitkMemoryUsageIndicatorImagesYellow_xpm)),
      std::make_pair(65.0f, QPixmap(QmitkMemoryUsageIndicatorImagesOrange_xpm)),
      std::make_pair(85.0f, QPixmap(QmitkMemoryUsageIndicatorImagesRed_xpm))
    }
{
  m_Ui->setupUi(this);
  m_Ui->led->setPixmap(m_States[0].second);

  auto timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &QmitkMemoryUsageIndicatorView::UpdateMemoryUsage);
  timer->start(1000);
}

QmitkMemoryUsageIndicatorView::~QmitkMemoryUsageIndicatorView()
{
}

void QmitkMemoryUsageIndicatorView::UpdateMemoryUsage()
{
  size_t processSize = mitk::MemoryUtilities::GetProcessMemoryUsage();
  size_t totalSize = mitk::MemoryUtilities::GetTotalSizeOfPhysicalRam();
  float percentage = static_cast<float>(processSize) / totalSize * 100.0f;

  const QLocale locale;

  m_Ui->label->setText(QString("%1 (%2 %)")
    .arg(locale.formattedDataSize(static_cast<qint64>(processSize), 2, QLocale::DataSizeTraditionalFormat))
    .arg(locale.toString(percentage, 'f', 2)));

  for (size_t i = m_States.size(); i-- > 0;)
  {
    if (percentage >= m_States[i].first)
    {
      if (m_PreviousState != i)
      {
        m_Ui->led->setPixmap(m_States[i].second);
        m_PreviousState = i;
      }

      break;
    }
  }
}
