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

#include <mitkLineVtkMapper3D.h>
#include <mitkSplineVtkMapper3D.h>

// ui
#include "ui_QmitkNavigationDataPlayerViewControls.h"

//mitk
#include "MitkIGTUIExports.h"
#include <mitkColorSequenceCycleH.h>
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
    \brief TODO
    */
    void OnPlayingStarted();

    void OnOpenFile();
    void OnSelectPlayer();
    void OnSetRepeat();
    void OnSetMicroservice();
    void OnSetDisplay();

protected:

  enum TrajectoryStyle {
    Points = 1,
    Splines = 2
  };

  void CreateBundleWidgets(QWidget* parent);

  Ui::QmitkNavigationDataPlayerViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
  QmitkIGTPlayerWidget* m_PlayerWidget; ///< this bundle's playback widget

private:
};

#endif // _QMITKNAVIGATIONDATAPLAYERVIEW_H_INCLUDED