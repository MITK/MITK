#include <QObject>

class QmitkPreferencesDialog;

class QmitkExtWorkbenchWindowAdvisorHack : public QObject
{
  Q_OBJECT

  public slots:

    void onUndo();
    void onRedo();
    void onEditPreferences();
    void onQuit();

    void onResetPerspective();
    void onClosePerspective();
    void onNewWindow();
    void onIntro();

  public:

    QmitkExtWorkbenchWindowAdvisorHack();

    static QmitkExtWorkbenchWindowAdvisorHack* undohack;
};

