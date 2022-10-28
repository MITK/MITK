/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMemoryUsageIndicatorView_h
#define QmitkMemoryUsageIndicatorView_h

#include <MitkQtWidgetsExports.h>

#include <QWidget>

#include <array>
#include <utility>

namespace Ui
{
  class QmitkMemoryUsageIndicator;
}

/// \ingroup QmitkModule
class MITKQTWIDGETS_EXPORT QmitkMemoryUsageIndicatorView : public QWidget
{
  Q_OBJECT

public:
  QmitkMemoryUsageIndicatorView(QWidget* parent = nullptr);
  ~QmitkMemoryUsageIndicatorView() override;

private:
  void UpdateMemoryUsage();
  std::string FormatMemorySize(size_t size);
  std::string FormatPercentage(double val);
  std::string GetMemoryDescription(size_t processSize, float percentage);

  Ui::QmitkMemoryUsageIndicator* m_Ui;
  size_t m_PreviousState;
  std::array<std::pair<float, QPixmap>, 4> m_States;
};

#endif
