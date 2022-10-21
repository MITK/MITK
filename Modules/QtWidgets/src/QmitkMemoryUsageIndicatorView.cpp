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

#include <iomanip>

#include <QTimer>

#include <QmitkMemoryUsageIndicatorImagesGreen.xpm>
#include <QmitkMemoryUsageIndicatorImagesOrange.xpm>
#include <QmitkMemoryUsageIndicatorImagesRed.xpm>
#include <QmitkMemoryUsageIndicatorImagesYellow.xpm>

#include <ui_QmitkMemoryUsageIndicator.h>

QmitkMemoryUsageIndicatorView::QmitkMemoryUsageIndicatorView(QWidget*)
  : m_Ui(new Ui::QmitkMemoryUsageIndicator),
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

  m_Ui->label->setText(QString::fromStdString(this->GetMemoryDescription(processSize, percentage)));

  for (size_t i = m_States.size() - 1; i >= 0; --i)
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

std::string QmitkMemoryUsageIndicatorView::FormatMemorySize(size_t size)
{
  double val = size;
  std::string unit;

  if (val >= 1024.0)
  {
    val /= 1024.0;
    unit = "K";
  }

  if (val >= 1024.0)
  {
    val /= 1024.0;
    unit = "M";
  }

  if (val >= 1024.0)
  {
    val /= 1024.0;
    unit = "G";
  }

  unit += "B";

  std::ostringstream str;
  str.imbue(std::locale::classic());
  str << std::fixed << std::setprecision(2) << val << " " << unit;
  return str.str();
}

std::string QmitkMemoryUsageIndicatorView::FormatPercentage(double val)
{
  std::ostringstream str;
  str.imbue(std::locale::classic());
  str << std::fixed << std::setprecision(2) << val << " " << "%";
  return str.str();
}

std::string QmitkMemoryUsageIndicatorView::GetMemoryDescription(size_t processSize, float percentage)
{
  std::ostringstream str;
  str.imbue(std::locale::classic());
  str << FormatMemorySize(processSize) << " (" << FormatPercentage(percentage) << ")";
  return str.str();
}
