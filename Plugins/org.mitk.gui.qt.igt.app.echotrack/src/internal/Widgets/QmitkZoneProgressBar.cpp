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

#include "QmitkZoneProgressBar.h"

QmitkZoneProgressBar::QmitkZoneProgressBar(QString name, int maxRange, int warnRange, QWidget *parent)
  : QProgressBar(parent), m_TextFormatValid(name), m_MaxRange(maxRange), m_WarnRange(warnRange),
    m_ColorString("red"), m_WarnColorString("red"), m_BorderColorString("gray"),
    m_StyleSheet("QProgressBar:horizontal {\nborder: 1px solid %1;\nborder-radius: 3px;\nbackground: white;\npadding: 1px;\ntext-align: center;\n}\nQProgressBar::chunk:horizontal {\nbackground: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 %2, stop: 0.8 %2, stop: 1 %3);\n}")
{
  this->setMinimum(0);
  this->setMaximum(maxRange);

  this->UpdateStyleSheet(m_ColorString, m_ColorString);
}

void QmitkZoneProgressBar::setValue(int value)
{
  int adjustedValue = 0;
  QString text;
  QString startColor, stopColor;

  // There Are now four possible outcomes
  // 1) Needle is inside zone (bad news)
  if (value < 0)
  {
    startColor = m_WarnColorString;
    stopColor = m_WarnColorString;

    text = m_TextFormatValid.arg(0) + " (VIOLATED)";
    adjustedValue = m_MaxRange;
  } // 2) Needle is close to zone
  else if (value < m_WarnRange)
  {
    startColor = m_ColorString;
    stopColor = m_WarnColorString;

    text = m_TextFormatValid.arg(value);
    adjustedValue = m_MaxRange - value;
  } // 3) Needle is away from zone
  else if (value < m_MaxRange)
  {
    startColor = m_ColorString;
    stopColor = m_ColorString;

    text = m_TextFormatValid.arg(value);
    adjustedValue = m_MaxRange - value;
  } // 4) Needle is far away from zone
  else
  {
    startColor = m_ColorString;
    stopColor = m_ColorString;

    text = m_TextFormatValid.arg(value);
    adjustedValue = 0;
  }

  this->setStyleSheet(m_StyleSheet.arg(m_BorderColorString, startColor, stopColor));
  this->setFormat(text);
  QProgressBar::setValue(adjustedValue);
}

void QmitkZoneProgressBar::setValueInvalid()
{
  this->setStyleSheet(m_StyleSheet.arg("#DD0000", "red", "red"));
  this->setFormat(m_TextFormatInvalid);
  QProgressBar::setValue(0);
}

void QmitkZoneProgressBar::SetTextFormatValid(QString format)
{
  m_TextFormatValid = format;
}

void QmitkZoneProgressBar::SetTextFormatInvalid(QString format)
{
  m_TextFormatInvalid = format;
}

void QmitkZoneProgressBar::SetColor(float color[3])
{
  m_ColorString = this->ColorToString(color);
}

void QmitkZoneProgressBar::SetWarnColor(float color[3])
{
  m_WarnColorString = this->ColorToString(color);
}

void QmitkZoneProgressBar::SetBorderColor(float color[3])
{
  m_BorderColorString = this->ColorToString(color);
}

void QmitkZoneProgressBar::UpdateStyleSheet(QString startColor, QString stopColor)
{
  this->setStyleSheet(m_StyleSheet.arg(m_BorderColorString, startColor, stopColor));
}

QString QmitkZoneProgressBar::ColorToString(float color[3])
{
  return QString("#%1%2%3").arg(static_cast<unsigned int>(color[0]*255), 2, 16, QChar('0'))
      .arg(static_cast<unsigned int>(color[1]*255), 2, 16, QChar('0'))
      .arg(static_cast<unsigned int>(color[2]*255), 2, 16, QChar('0'));
}
