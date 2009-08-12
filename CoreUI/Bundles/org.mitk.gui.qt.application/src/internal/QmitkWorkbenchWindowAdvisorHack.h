#include <QObject>

class QmitkWorkbenchWindowAdvisorHelperHack : public QObject
{
  Q_OBJECT

  public slots:

    void onUndo();
    void onRedo();
  public:

    QmitkWorkbenchWindowAdvisorHelperHack();

    static QmitkWorkbenchWindowAdvisorHelperHack* undohack;
};

