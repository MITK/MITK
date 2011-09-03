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


#ifndef QmitkMasterView_h
#define QmitkMasterView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkMasterViewControls.h"

#include <string>

#include <QButtonGroup>
#include <QActionGroup>
#include <QToolButton>
#include <QToolBar>
#include <QAction>
#include <QHash>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <berryIPerspectiveListener.h>
#include <berryISizeProvider.h>
#include <berryConstants.h>
#include <berryIWorkbenchPage.h>

/*!
  \brief QmitkMasterView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
*/
class QmitkMasterView : public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  

    static const std::string VIEW_ID;

    QmitkMasterView();
    virtual ~QmitkMasterView();

    virtual void CreateQtPartControl(QWidget *parent);

  protected slots:
  
    void OpenDicomImport();
    void OpenPreprocessingReconstruction();
    void OpenQuantification();
    void OpenTractography();
    void OpenTBSS();
    void OpenConnectomics();
    void OpenIVIM();
    void OpenVolumeVisualization();
    void OpenScreenshotsMovies();

protected:

    void CreateConnections();

    void OpenPerspective(std::string id);

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    Ui::QmitkMasterViewControls m_Controls;

    friend struct QmitkMasterViewListener;

};

#endif // QmitkMasterView_h

