/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17224 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKMEMORYUSAGEINDICATORVIEW_WIDGET
#define QMITKMEMORYUSAGEINDICATORVIEW_WIDGET

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
