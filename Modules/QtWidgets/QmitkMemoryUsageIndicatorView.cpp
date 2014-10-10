/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

#include "QmitkMemoryUsageIndicatorView.h"
#include <mitkMemoryUtilities.h>

#include <qtimer.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qeventloop.h>

#include <iostream>
#include <sstream>
#include <iomanip>

#include "QmitkMemoryUsageIndicatorImagesGreen.xpm"
#include "QmitkMemoryUsageIndicatorImagesYellow.xpm"
#include "QmitkMemoryUsageIndicatorImagesOrange.xpm"
#include "QmitkMemoryUsageIndicatorImagesRed.xpm"

QmitkMemoryUsageIndicatorView::QmitkMemoryUsageIndicatorView( QWidget *  /*parent*/, Qt::WindowFlags  /*f*/ )
{
  this->setupUi(this);

  QTimer *timer = new QTimer( this );
  QObject::connect( timer, SIGNAL( timeout() ), this, SLOT( UpdateMemoryUsage() ) );
  timer->start(1000);
  m_LEDGreen = QPixmap(QmitkMemoryUsageIndicatorImagesGreen_xpm);
  m_LEDYellow = QPixmap(QmitkMemoryUsageIndicatorImagesYellow_xpm);
  m_LEDOrange = QPixmap(QmitkMemoryUsageIndicatorImagesOrange_xpm);
  m_LEDRed = QPixmap(QmitkMemoryUsageIndicatorImagesRed_xpm);
  m_LED->setPixmap(m_LEDGreen);
  m_PreviousState = 0;
}

QmitkMemoryUsageIndicatorView::~QmitkMemoryUsageIndicatorView()
{
}

void QmitkMemoryUsageIndicatorView::UpdateMemoryUsage()
{
  size_t processSize = mitk::MemoryUtilities::GetProcessMemoryUsage();
  size_t totalSize =  mitk::MemoryUtilities::GetTotalSizeOfPhysicalRam();
  float percentage = ( (float) processSize / (float) totalSize ) * 100.0;
  m_Label->setText( GetMemoryDescription( processSize, percentage ).c_str() );
  if ( percentage < 50.0 )
  {
    if(m_PreviousState != 0)
    {
      m_LED->setPixmap(m_LEDGreen);
      m_PreviousState = 0;
      m_LED->update();
    }
  }
  else if ( percentage < 65.0 )
  {
    if(m_PreviousState != 1)
    {
      m_LED->setPixmap(m_LEDYellow);
      m_PreviousState = 1;
      m_LED->update();
    }
  }
  else if ( percentage < 80.0 )
  {
    if(m_PreviousState != 2)
    {
      m_LED->setPixmap(m_LEDOrange);
      m_PreviousState = 2;
      m_LED->update();
    }
  }
  else
  {
    if(m_PreviousState != 3)
    {
      m_LED->setPixmap(m_LEDRed);
      m_PreviousState = 3;
      m_LED->update();
    }
  }
}


std::string QmitkMemoryUsageIndicatorView::FormatMemorySize( size_t size )
{
  double val = size;
  std::string descriptor("B");
  if ( val >= 1000.0 )
  {
    val /= 1024.0;
    descriptor = "KB";
  }
  if ( val >= 1000.0 )
  {
    val /= 1024.0;
    descriptor = "MB";
  }
  if ( val >= 1000.0 )
  {
    val /= 1024.0;
    descriptor = "GB";
  }
  std::ostringstream str;
  str.imbue(std::locale::classic());
  str << std::fixed << std::setprecision(2) << val << " " << descriptor;
  return str.str();
}

std::string QmitkMemoryUsageIndicatorView::FormatPercentage( double val )
{
  std::ostringstream str;
  str.imbue(std::locale::classic());
  str << std::fixed << std::setprecision(2) << val << " " << "%";
  return str.str();
}

std::string QmitkMemoryUsageIndicatorView::GetMemoryDescription( size_t processSize, float percentage )
{
  std::ostringstream str;
  str.imbue(std::locale::classic());
  str << FormatMemorySize(processSize) << " (" << FormatPercentage( percentage ) <<")" ;
  return str.str();
}
