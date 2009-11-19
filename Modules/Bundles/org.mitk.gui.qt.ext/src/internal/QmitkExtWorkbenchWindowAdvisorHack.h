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
    
    /**
     * @brief This slot is called if the user klicks the menu "item help->active bundle" or presses F1. In this case a new window is opened which shows a help page.
     */
    void onHelp();
    
    /**
     * @brief This slot is called if the user clicks in help menu the about button
     */
    void onAbout();

  public:

    QmitkExtWorkbenchWindowAdvisorHack();
    ~QmitkExtWorkbenchWindowAdvisorHack();

    static QmitkExtWorkbenchWindowAdvisorHack* undohack;
    
};
