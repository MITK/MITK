#ifndef QMITKSTATUSBAR_H
#define QMITKSTATUSBAR_H
#include <mitkStatusBar.h>
#include <mitkCommon.h>
#include <qstatusbar.h>

//##Documentation
//## @brief provides to send a Message to the QT's StatusBar
//## A delay time can be set.
//##
//## The application sets the Instance to mitkStatusBar so that
//## all mitk-classes will call this class for output:
//## mitk::StatusBar::SetInstance(QmitkStatusBar::GetInstance());
//## 
//## Then the applikation sets the MainWindow StatusBar by:
//## QmitkStatusBar::SetStatusBar(QMainWindow::statusBar());

class QmitkStatusBar : public mitk::StatusBar
{
public:

  mitkClassMacro(QmitkStatusBar, mitk::StatusBar);
    
    //##Documentation
    //## @brief This is a singleton pattern New. 
    //##
    //## There will only be ONE reference to a QmitkStatusBar object per process. 
    //## The single instance will be unreferenced when the program exits.
    //## Reimplemented from itk::Object
    static mitk::StatusBar::Pointer New();

    //##Documentation
    //## @brief set the statusBar for Output; created singleton if not already done
    //##
    //## use: QmitkStatusBar::SetStatusBar([QMainWindow or this]->statusBar());
    static void SetStatusBar(QStatusBar* statusBar);
    
    //##Documentation
    //## @brief Send a string to the applications StatusBar (QStatusBar).
    virtual void DisplayText(const char* t);
    virtual void DisplayText(const char* t, int ms);
        
    //##Documentation
    //## @brief Send a string as an error message to StatusBar. 
    //## The implementation calls DisplayText()
    virtual void DisplayErrorText(const char *t) { this->DisplayText(t); };
    virtual void DisplayWarningText(const char *t) { this->DisplayText(t); };
    virtual void DisplayWarningText(const char *t, int ms) { this->DisplayText(t, ms); };
    virtual void DisplayGenericOutputText(const char *t) {this->DisplayText(t);}
    virtual void DisplayDebugText(const char *t) { this->DisplayText(t); };

    //##Documentation
    //## @brief removes any temporary message being shown.
    virtual void Clear();

    //##Documentation
    //## @brief Set the QSizeGrip of the window 
    //## (the triangle in the lower right Windowcorner for changing the size) 
    //## to enabled or disabled 
    virtual void SetSizeGripEnabled(bool enable);

protected:
    QmitkStatusBar();
    virtual ~QmitkStatusBar();
private:
    //static Pointer m_Instance;
    static QStatusBar* m_StatusBar;
};

#endif /* define QMITKSTATUSBAR_H */
