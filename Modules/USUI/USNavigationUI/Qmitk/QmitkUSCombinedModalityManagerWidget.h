#ifndef QMITKUSCOMBINEDMODALITYMANAGERWIDGET_H
#define QMITKUSCOMBINEDMODALITYMANAGERWIDGET_H

#include <QWidget>

namespace Ui {
class QmitkUSCombinedModalityManagerWidget;
}

class QmitkUSCombinedModalityManagerWidget : public QWidget
{
    Q_OBJECT

protected slots:
    void OnCreateCombinedModalityButtonClicked();
    void OnRemoveCombinedModalityButtonClicked();

public:
    explicit QmitkUSCombinedModalityManagerWidget(QWidget *parent = 0);
    ~QmitkUSCombinedModalityManagerWidget();

private:
    Ui::QmitkUSCombinedModalityManagerWidget *ui;
};

#endif // QMITKUSCOMBINEDMODALITYMANAGERWIDGET_H
