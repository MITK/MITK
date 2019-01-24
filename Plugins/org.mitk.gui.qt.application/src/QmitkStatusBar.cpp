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


#include "QmitkStatusBar.h"

#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qapplication.h>
#include <qdesktopwidget.h>

#include <mitkStatusBar.h>

#include <itkObjectFactory.h>


/**
 * Display the text in the statusbar of the applikation
 */
void QmitkStatusBar::DisplayText(const char* t)
{
  if (m_StatusBar != nullptr)
    m_StatusBar->showMessage(t);
  // TODO bug #1357
  //qApp->processEvents(); // produces crashes!
}

/**
 * Display the text in the statusbar of the applikation for ms seconds
 */
void QmitkStatusBar::DisplayText(const char* t, int ms)
{
  if (m_StatusBar != nullptr)
    m_StatusBar->showMessage(t, ms);
  // TODO bug #1357
  //qApp->processEvents(); // produces crashes!
}

/**
 * Show the grey value text in the statusbar
 */
void QmitkStatusBar::DisplayLicenseText(const char* t)
{
  QString text(t);
  if (!m_LicenseLabel->isVisible()) {
    m_LicenseWarningIcon->show();
    m_LicenseLabel->show();
  }
  m_LicenseLabel->setText(text);
}

void QmitkStatusBar::HideLicenseWarning()
{
  if (m_LicenseLabel->isVisible()) {
    m_LicenseLabel->clear();
    m_LicenseWarningIcon->hide();
    m_LicenseLabel->hide();
  }
}

/**
 * Show the grey value text in the statusbar
 */
void QmitkStatusBar::DisplayGreyValueText(const char* t)
{
  QString text(t);
  m_GreyValueLabel->setText(text);
}

void QmitkStatusBar::DisplayImageInfo(mitk::Point3D point, itk::Index<3> index, mitk::ScalarType time, mitk::ScalarType pixelValue)
{
  QString text = "";
  text += qApp->translate("StatusBar", "Position: <%1, %2, %3> mm; ");
  for (int i = 0; i < 3; i++) {
    text = text.arg(point[i], 0, 'f', 2);
  }

  text += qApp->translate("StatusBar", "Index: <%1, %2, %3>; ");
  for (int i = 0; i < 3; i++) {
    text = text.arg(QString::number(index[i] + 1));
  }

  text += qApp->translate("StatusBar", "Time: %1 ms; ");
  text = text.arg(time);

  text += qApp->translate("StatusBar", "Pixel value: %1");
  if (fabs(pixelValue) > 1000000 || fabs(pixelValue) < 0.01) {
    text = text.arg(pixelValue, 0, 'g', 2);
  } else {
    text = text.arg(pixelValue);
  }

  m_GreyValueLabel->setText(text);
}

void QmitkStatusBar::DisplayImageInfo(mitk::Point3D point, itk::Index<3> index, mitk::ScalarType time, const char* pixelValue)
{
  QString text = "";
  text += qApp->translate("StatusBar", "Position: <%1, %2, %3> mm; ");
  for (int i = 0; i < 3; i++) {
    text = text.arg(point[i], 0, 'f', 2);
  }

  text += qApp->translate("StatusBar", "Index: <%1, %2, %3>; ");
  for (int i = 0; i < 3; i++) {
    text = text.arg(QString::number(index[i] + 1));
  }

  text += qApp->translate("StatusBar", "Time: %1 ms; ");
  text = text.arg(time);

  text += qApp->translate("StatusBar", "Pixel value: %1");
  text = text.arg(pixelValue);

  m_GreyValueLabel->setText(text);
}

void QmitkStatusBar::DisplayImageInfoInvalid() 
{
  m_GreyValueLabel->setText(qApp->translate("StatusBar", "No image information at this position!"));
}

/**
 * Clear the text in the StatusBar
 */
void QmitkStatusBar::Clear()
{
  if (m_StatusBar != nullptr)
    m_StatusBar->clearMessage();
  // TODO bug #1357
  //qApp->processEvents(); // produces crashes!
}

/**
 * enable or disable the QSizeGrip
 */
void QmitkStatusBar::SetSizeGripEnabled(bool enable)
{
  if (m_StatusBar != nullptr)
    m_StatusBar->setSizeGripEnabled(enable);
}

QmitkStatusBar::QmitkStatusBar(QStatusBar* instance)
:StatusBarImplementation()
{
    m_StatusBar = instance;
    m_LicenseWarningIcon = new QLabel(m_StatusBar,nullptr);
    m_LicenseLabel = new QLabel(m_StatusBar,nullptr);
    m_GreyValueLabel = new QLabel(m_StatusBar,nullptr);
    int xResolution = QApplication::desktop()->screenGeometry(0).width()-100;
    m_GreyValueLabel->setMaximumSize(QSize(xResolution,50));
    m_GreyValueLabel->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);

    m_LicenseLabel->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
    m_LicenseWarningIcon->setPixmap(QPixmap(":/org.mitk.gui.qt.application/warning.png"));

    m_LicenseWarningIcon->hide();
    m_LicenseLabel->hide();

    m_StatusBar->addPermanentWidget(m_LicenseWarningIcon, 0);
    m_StatusBar->addPermanentWidget(m_LicenseLabel, 0);
    m_StatusBar->addPermanentWidget(m_GreyValueLabel, 1);
    mitk::StatusBar::SetImplementation(this);
    QObject::connect(m_StatusBar, &QObject::destroyed, [=]() {
      m_StatusBar = nullptr;
    });
}

QmitkStatusBar::~QmitkStatusBar()
{
  if (m_StatusBar != nullptr)
    QObject::disconnect(m_StatusBar, &QObject::destroyed, 0, 0);
}

