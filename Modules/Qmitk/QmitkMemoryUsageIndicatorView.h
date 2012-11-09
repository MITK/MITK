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

#ifndef QMITKMEMORYUSAGEINDICATORVIEW_WIDGET
#define QMITKMEMORYUSAGEINDICATORVIEW_WIDGET

#include <QmitkExports.h>

#include <QWidget>
#include <qpixmap.h>
#include "ui_QmitkMemoryUsageIndicator.h"

class QMITK_EXPORT QmitkMemoryUsageIndicatorView : public QWidget, public Ui::QmitkMemoryUsageIndicator
{

  Q_OBJECT

public:

  /// constructor
  QmitkMemoryUsageIndicatorView( QWidget * parent=0, Qt::WindowFlags f = 0 );

  /// destructor
  ~QmitkMemoryUsageIndicatorView();

protected slots:
    void UpdateMemoryUsage();

protected:

  std::string FormatMemorySize( size_t size );
  std::string FormatPercentage( double val );
  std::string GetMemoryDescription( size_t processSize, float percentage );

  QPixmap m_LEDGreen;
  QPixmap m_LEDYellow;
  QPixmap m_LEDOrange;
  QPixmap m_LEDRed;
  char m_PreviousState;
};
#endif //QMITKMEMORYUSAGEINDICATORVIEW_WIDGET
