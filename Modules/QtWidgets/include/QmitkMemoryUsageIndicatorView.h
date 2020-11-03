/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKMEMORYUSAGEINDICATORVIEW_WIDGET
#define QMITKMEMORYUSAGEINDICATORVIEW_WIDGET

#include <MitkQtWidgetsExports.h>

#include "ui_QmitkMemoryUsageIndicator.h"
#include <QWidget>
#include <qpixmap.h>

/// \ingroup QmitkModule
class MITKQTWIDGETS_EXPORT QmitkMemoryUsageIndicatorView : public QWidget, public Ui::QmitkMemoryUsageIndicator
{
  Q_OBJECT

public:
  /// constructor
  QmitkMemoryUsageIndicatorView(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);

  /// destructor
  ~QmitkMemoryUsageIndicatorView() override;

protected slots:
  void UpdateMemoryUsage();

protected:
  std::string FormatMemorySize(size_t size);
  std::string FormatPercentage(double val);
  std::string GetMemoryDescription(size_t processSize, float percentage);

  QPixmap m_LEDGreen;
  QPixmap m_LEDYellow;
  QPixmap m_LEDOrange;
  QPixmap m_LEDRed;
  char m_PreviousState;
};
#endif // QMITKMEMORYUSAGEINDICATORVIEW_WIDGET
