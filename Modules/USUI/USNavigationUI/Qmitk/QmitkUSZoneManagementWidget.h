#ifndef QMITKUSZONEMANAGEMENTWIDGET_H
#define QMITKUSZONEMANAGEMENTWIDGET_H

#include <QWidget>

namespace Ui {
class QmitkUSZoneManagementWidget;
}

class QmitkUSZoneManagementWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkUSZoneManagementWidget(QWidget *parent = 0);
  ~QmitkUSZoneManagementWidget();

private:
  Ui::QmitkUSZoneManagementWidget *ui;
};

#endif // QMITKUSZONEMANAGEMENTWIDGET_H
