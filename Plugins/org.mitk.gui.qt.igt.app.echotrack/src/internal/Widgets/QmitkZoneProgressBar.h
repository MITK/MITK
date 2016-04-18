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

#ifndef QMITKZONEPROGRESSBAR_H
#define QMITKZONEPROGRESSBAR_H

#include <QProgressBar>

/**
 * \brief QProgressBar for displaying distances to zones.
 * Colors are changed according to the distance to the zone and and the
 * progress will be filled more the smaller the distance to the zone
 * becomes.
 */
class QmitkZoneProgressBar : public QProgressBar
{
  Q_OBJECT
public:
  /**
   * \brief Initializes the progress bar with the given name, max range and warn range.
   * The colors are set to default values (color: red, warn color: red, border
   * color: gray).
   *
   * \param name will be shown on the progress bar
   * \param maxRange the progress bar will be empty for distances greater or equal to this distance
   * \param warnRange the progess bar will change its color for distances smaller than this distance
   */
  explicit QmitkZoneProgressBar(QString name, int maxRange, int warnRange, QWidget *parent = 0);

  /**
   * @param value the current distance to the zone
   */
  void setValue(int value);

  /**
   * \brief Can be called to indicate that there is currently no valid distance value available.
   * E.g. if tracking data is not available at the moment. If there is a valid
   * value again, the method setValue() can be called.
   */
  void setValueInvalid();

  /**
   * \brief Setter for the text on the progress bar.
   * Defaults to the string given as name to the constructor. The format string
   * can contain '%1' which will be replaced by the current distance value.
   *
   * \param format the text to be displayed on the progress bar
   */
  void SetTextFormatValid(QString format);

  /**
   * \param format the text to be displayed when setValueInvalid() was called
   */
  void SetTextFormatInvalid(QString format);

  /**
   * \param color the color for the progress bar fill
   */
  void SetColor(float color[3]);

  /**
   * \param color the color, the progress bar fill changes to if the distance falls below the warn range
   */
  void SetWarnColor(float color[3]);

  /**
   * @param color the color for the border of the progress bar
   */
  void SetBorderColor(float color[3]);

protected:
  void UpdateStyleSheet(QString startColor, QString stopColor);

  QString ColorToString(float color[3]);

  QString m_TextFormatValid;
  QString m_TextFormatInvalid;

  int     m_MaxRange;
  int     m_WarnRange;

  QString m_ColorString;
  QString m_WarnColorString;
  QString m_BorderColorString;

  QString m_StyleSheet;
};

#endif // QMITKZONEPROGRESSBAR_H
