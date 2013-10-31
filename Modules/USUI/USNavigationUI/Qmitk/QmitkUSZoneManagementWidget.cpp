#include "QmitkUSZoneManagementWidget.h"
#include "ui_QmitkUSZoneManagementWidget.h"

QmitkUSZoneManagementWidget::QmitkUSZoneManagementWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::QmitkUSZoneManagementWidget)
{
  ui->setupUi(this);
}

QmitkUSZoneManagementWidget::~QmitkUSZoneManagementWidget()
{
  delete ui;
}
