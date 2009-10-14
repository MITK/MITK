#include <QObject>

class QmitkPreferencesDialog;

class QmitkExtAppWorkbenchWindowAdvisorHack : public QObject
{
  Q_OBJECT

  public slots:

    void onUndo();
    void onRedo();
    void onEditPreferences();
    void onQuit();

  public:

    QmitkExtAppWorkbenchWindowAdvisorHack();

    static QmitkExtAppWorkbenchWindowAdvisorHack* undohack;
};

