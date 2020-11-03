/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKSTATUSBAR_H
#define QMITKSTATUSBAR_H
#include <mitkStatusBarImplementation.h>
#include <mitkCommon.h>
#include <QStatusBar>
#include <org_mitk_gui_qt_application_Export.h>
#include <QLabel>

/**
 * \ingroup org_mitk_gui_qt_application
 *
 * \brief QT-Toolkit/GUI dependent class that provides to send a Message to the QT's StatusBar
 *
 * A delay time can be set.
 *
 * All mitk-classes will call this class for output:
 * mitk::StatusBar::GetInstance();
 */
class MITK_QT_APP QmitkStatusBar : public mitk::StatusBarImplementation
{
public:

    mitkClassMacro(QmitkStatusBar, mitk::StatusBarImplementation);

    //##Documentation
    //##@brief Constructor;
    //## holds param instance internaly and connects this to the mitkStatusBar
    QmitkStatusBar(QStatusBar* instance);

    //##Documentation
    //##@brief Destructor
    ~QmitkStatusBar() override;


    //##Documentation
    //## @brief Send a string to the applications StatusBar (QStatusBar).
    void DisplayText(const char* t) override;
    void DisplayText(const char* t, int ms) override;

    //##Documentation
    //## @brief Send a string as an error message to StatusBar.
    //## The implementation calls DisplayText()
    void DisplayErrorText(const char *t) override { this->DisplayText(t); };
    void DisplayWarningText(const char *t) override { this->DisplayText(t); };
    void DisplayWarningText(const char *t, int ms) override { this->DisplayText(t, ms); };
    void DisplayGenericOutputText(const char *t) override {this->DisplayText(t);}
    void DisplayDebugText(const char *t) override { this->DisplayText(t); };
    void DisplayGreyValueText(const char *t) override;

    //##Documentation
    //## @brief removes any temporary message being shown.
    void Clear() override;

    //##Documentation
    //## @brief Set the QSizeGrip of the window
    //## (the triangle in the lower right Windowcorner for changing the size)
    //## to enabled or disabled
    void SetSizeGripEnabled(bool enable) override;

private:
    //static Pointer m_Instance;
    QStatusBar* m_StatusBar;
    QLabel* m_GreyValueLabel;

};

#endif /* define QMITKSTATUSBAR_H */
