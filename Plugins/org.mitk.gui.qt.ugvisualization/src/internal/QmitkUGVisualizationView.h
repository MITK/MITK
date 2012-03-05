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

#ifndef QmitkUGVisualizationView_h
#define QmitkUGVisualizationView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkUGVisualizationViewControls.h"

class QWidgetAction;

class QmitkBoolPropertyWidget;

namespace mitk {
  class PropertyObserver;
}

/*!
  \brief QmitkUGVisualizationView 

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkUGVisualizationView : public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  

    static const std::string VIEW_ID;

    QmitkUGVisualizationView();
    QmitkUGVisualizationView(const QmitkUGVisualizationView& other);
    virtual ~QmitkUGVisualizationView();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

  protected slots:

    void UpdateRenderWindow();

    void ShowTFGeneratorWidget(bool show);
    void ShowScalarOpacityWidget(bool show);
    void ShowColorWidget(bool show);
    void ShowGradientOpacityWidget(bool show);

  protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    void CreateConnections();

  private:

    friend class UGVisVolumeObserver;

    void UpdateGUI();

    void UpdateEnablement();

    Ui::QmitkUGVisualizationViewControls m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;

    QWidgetAction* m_Outline2DAction;
    QmitkBoolPropertyWidget* m_Outline2DWidget;
    QAction* m_LODAction;
    QWidgetAction* m_ScalarVisibilityAction;
    QmitkBoolPropertyWidget* m_ScalarVisibilityWidget;

    int m_FirstVolumeRepId;
    QHash<int, int> m_MapRepComboToEnumId;

    bool m_VolumeMode;

    bool m_ShowTFGeneratorWidget;
    bool m_ShowScalarOpacityWidget;
    bool m_ShowColorWidget;
    bool m_ShowGradientOpacityWidget;

    QAction* m_ShowTFGeneratorAction;
    QAction* m_ShowScalarOpacityAction;
    QAction* m_ShowColorAction;
    QAction* m_ShowGradientOpacityAction;

    mitk::PropertyObserver* m_VolumeModeObserver;

};



#endif // _QMITKUGVISUALIZATIONVIEW_H_INCLUDED

