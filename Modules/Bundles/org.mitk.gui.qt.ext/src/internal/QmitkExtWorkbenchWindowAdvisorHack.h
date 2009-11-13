#include <QObject>
#include <QKeyEvent>

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
    
    /**
     * @brief This slot is called if the user klicks the menu "item help->active bundle" or presses F1. In this case a new window is opened which shows a help page.
     */
    void onHelp();

  public:

    QmitkExtWorkbenchWindowAdvisorHack();

    static QmitkExtWorkbenchWindowAdvisorHack* undohack;
    
  protected:
   void keyPressEvent(QKeyEvent * event);
};
