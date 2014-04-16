/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkNavigationDataPlayerView_h
#define QmitkNavigationDataPlayerView_h

#include <berryISelectionListener.h>

//Qmitk
#include <QmitkFunctionality.h>
#include <QmitkIGTPlayerWidget.h>

// ui
#include "ui_QmitkNavigationDataPlayerViewControls.h"

//mitk
#include <mitkNavigationDataObjectVisualizationFilter.h>

/*!
\brief QmitkNavigationDataPlayerView

\warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkNavigationDataPlayerView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkNavigationDataPlayerView();
  virtual ~QmitkNavigationDataPlayerView();

  virtual void CreateQtPartControl(QWidget *parent);

  /**
  \brief This method creates this bundle's SIGNAL and SLOT connections
  */
  virtual void CreateConnections();

  protected slots:

    /*!
    \brief FIXME not currently used
    */
    void OnPlayingStarted();

    /*!
    \brief loads a file and triggers creation of players and the pipeline
    */
    void OnOpenFile();

    /*!
    \brief Creates the correct player and displays the according widget
    */
    void OnSelectPlayer();

    /*!
    \brief FIXME not currently used
    */
    void OnSetRepeat();

    /*!
    \brief FIXME not currently used
    */
    void OnSetMicroservice();

    /*!
    \brief Triggers the creation and destruction of the rendering pipeline
    */
    void OnSetDisplay();

    /*!
    \brief FIXME not currently used
    */
    void OnUpdate();

protected:

  /**
  * \brief configures the player according to the checkboxes set in the GUI
  */
  void ConfigurePlayer();

  /**
  * \brief Creates the Rendering Pipeline necessary to Render the images
  */
  void CreatePipeline();

  /**
  * \brief Destroys the Rendering Pipeline (but not the player)
  */
  void DestroyPipeline();

  /**
  * \brief Makes player component active or inactive.
  *
  * Used to activate all components once data is loaded
  */
  void SetInteractionComponentsEnabledState(bool isActive);

  void CreateBundleWidgets(QWidget* parent);

  Ui::QmitkNavigationDataPlayerViewControls* m_Controls;

  mitk::NavigationDataObjectVisualizationFilter::Pointer m_VisFilter;
  std::vector<mitk::DataNode::Pointer> m_RenderingNodes;
  mitk::NavigationDataPlayerBase::Pointer m_Player;
  mitk::NavigationDataSet::Pointer m_Data;

  QmitkStdMultiWidget* m_MultiWidget;
  QmitkIGTPlayerWidget* m_PlayerWidget; ///< this bundle's playback widget

private:
};

#endif // _QMITKNAVIGATIONDATAPLAYERVIEW_H_INCLUDED