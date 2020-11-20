#include "ThemeUtils.h"

#include <QAction>
#include <QPainter>
#include <QVariant>
#include <QApplication>

const char* THEME_PROP = "Theme";

namespace mitk
{
namespace themeUtils
{
  QPixmap colorizePixmap(QPixmap originalPixmap, QColor targetColor, QIcon::Mode mode)
  {
    QPixmap targetPixmap(originalPixmap);
    QPainter painter(&targetPixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);

    if (qApp->property(THEME_PROP).toString() == "dark") {
      if (mode == QIcon::Normal) {
        painter.fillRect(targetPixmap.rect(), QColor(195, 195, 208));
      } else {
        painter.fillRect(targetPixmap.rect(), QColor(101, 101, 101));
      }
    } else {
      painter.fillRect(targetPixmap.rect(), targetColor);
    }

    return targetPixmap;
  }

  QPixmap colorizePixmap(QPixmap originalPixmap, QPalette targetPalette)
  {
    QColor color;
    if (qApp->property(THEME_PROP).toString() == "dark") {
      color = QColor(195,195,208);
    } else {
      color = targetPalette.color(QPalette::Normal, QPalette::WindowText);
    }
    return colorizePixmap(originalPixmap, color);
  }

  QIcon colorizeIcon(QIcon originalIcon, QPalette targetPalette)
  {
    QIcon colorizedIcon;
    QList<QSize> iconSizes = originalIcon.availableSizes();
    for (int i = 0; i < iconSizes.size(); ++i) {
      QPixmap originalPixmap(originalIcon.pixmap(iconSizes[i]));
      colorizedIcon.addPixmap(colorizePixmap(originalPixmap, targetPalette.color(QPalette::Normal, QPalette::WindowText)), QIcon::Normal);

      QPixmap disabledPixmap(originalIcon.pixmap(iconSizes[i], QIcon::Disabled));
      colorizedIcon.addPixmap(colorizePixmap(disabledPixmap, targetPalette.color(QPalette::Disabled, QPalette::WindowText), QIcon::Disabled), QIcon::Disabled);
    }
    return colorizedIcon;
  }

  void colorizeWidgetIcons(QWidget* widget)
  {
    QPalette palette = widget->palette();
    QVariant iconPropertyVariant = widget->property("icon");
    if (iconPropertyVariant.isValid()) {
      QIcon originalIcon = iconPropertyVariant.value<QIcon>();
      if (!originalIcon.isNull()) {
        QIcon colorizedIcon = colorizeIcon(originalIcon, palette);
        iconPropertyVariant.setValue(colorizedIcon);
        widget->setProperty("icon", iconPropertyVariant);
      }
    }
    foreach (QAction* action, widget->actions()) {
      action->setIcon(colorizeIcon(action->icon(), palette));
    }
    QList<QWidget*> childWidgets = widget->findChildren<QWidget*>();
    foreach (QWidget* childWidget, childWidgets) {
      colorizeWidgetIcons(childWidget);
    }
  }
}
}
