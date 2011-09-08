/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef IGTTrackingLabView_h
#define IGTTrackingLabView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_IGTTrackingLabViewControls.h"

#include <QToolBox>

class QmitkNDIConfigurationWidget;
class QmitkFiducialRegistrationWidget;
class QmitkUpdateTimerWidget;
class QmitkToolTrackingStatusWidget;



/*!
  \brief IGTTrackingLabView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
*/
class IGTTrackingLabView : public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  

    static const std::string VIEW_ID;

    IGTTrackingLabView();
    virtual ~IGTTrackingLabView();

    virtual void CreateQtPartControl(QWidget *parent);

  protected slots:
  
    /// \brief Called when the user clicks the GUI button
//    void DoImageProcessing();

  protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
//    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    Ui::IGTTrackingLabViewControls m_Controls;
    void CreateBundleWidgets( QWidget* parent );


private:

  QToolBox* m_ToolBox;

  QmitkNDIConfigurationWidget* m_NDIConfigWidget;  // tracking device configuration widget
  QmitkFiducialRegistrationWidget* m_RegistrationWidget; // landmark registration widget
  QmitkUpdateTimerWidget* m_RenderingTimerWidget; // update timer widget for rendering and updating
  QmitkToolTrackingStatusWidget* m_ToolStatusWidget; // tracking status visualizer widget
};

#endif // IGTTrackingLabView_h

