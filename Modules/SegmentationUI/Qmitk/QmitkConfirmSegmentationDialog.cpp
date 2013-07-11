#include "QmitkConfirmSegmentationDialog.h"
#include "ui_QmitkConfirmSegmentationDialog.h"

QmitkConfirmSegmentationDialog::QmitkConfirmSegmentationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::QmitkConfirmSegmentationDialog)
{
  ui->setupUi(this);
}

QmitkConfirmSegmentationDialog::~QmitkConfirmSegmentationDialog()
{
  delete ui;
}
