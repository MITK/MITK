#include <QObject>

class QmitkPreferencesDialog;

class QmitkWorkbenchWindowAdvisorHelperHack : public QObject
{
  Q_OBJECT

  public slots:

    void onUndo();
    void onRedo();
    void onEditPreferences();
    void onQuit();

  public:

    QmitkWorkbenchWindowAdvisorHelperHack();

    static QmitkWorkbenchWindowAdvisorHelperHack* undohack;
};

