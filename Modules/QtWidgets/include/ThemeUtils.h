#pragma once

#include <MitkQtWidgetsExports.h>

#include <QWidget>
#include <QPixmap>
#include <QIcon>
#include <QColor>
#include <QPalette>

namespace mitk {
namespace themeUtils {
  MITKQTWIDGETS_EXPORT QPixmap colorizePixmap(QPixmap originalPixmap, QColor targetColor, QIcon::Mode mode = QIcon::Normal);
  MITKQTWIDGETS_EXPORT QPixmap colorizePixmap(QPixmap originalPixmap, QPalette targetPalette);
  MITKQTWIDGETS_EXPORT QIcon colorizeIcon(QIcon originalIcon, QPalette targetPalette);
  MITKQTWIDGETS_EXPORT void colorizeWidgetIcons(QWidget* widget);
}
}
