#ifndef QMITKCONFIRMSEGMENTATIONDIALOG_H
#define QMITKCONFIRMSEGMENTATIONDIALOG_H

#include <QDialog>

namespace Ui {
class QmitkConfirmSegmentationDialog;
}

class QmitkConfirmSegmentationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkConfirmSegmentationDialog(QWidget *parent = 0);
  ~QmitkConfirmSegmentationDialog();

private:
  Ui::QmitkConfirmSegmentationDialog *ui;
};

#endif // QMITKCONFIRMSEGMENTATIONDIALOG_H
