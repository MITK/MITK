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

#ifndef QmitkUGVisualizationView_h
#define QmitkUGVisualizationView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkUGVisualizationViewControls.h"

class QWidgetAction;

class QmitkBoolPropertyWidget;

namespace mitk {
  class PropertyObserver;
}

/*!
  \brief QmitkUGVisualizationView

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.
*/
class QmitkUGVisualizationView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkUGVisualizationView();
    virtual ~QmitkUGVisualizationView();

    virtual void CreateQtPartControl(QWidget *parent) override;

    ///
    /// Sets the focus to an internal widget.
    ///
    virtual void SetFocus() override;

  protected slots:

    void UpdateRenderWindow();

    void ShowTFGeneratorWidget(bool show);
    void ShowScalarOpacityWidget(bool show);
    void ShowColorWidget(bool show);
    void ShowGradientOpacityWidget(bool show);

  protected:

    /// \brief called by QmitkAbstractView when DataManager's selection has changed
    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

    void CreateConnections();

  private:

    friend class UGVisVolumeObserver;

    void UpdateGUI();

    void UpdateEnablement();

    Ui::QmitkUGVisualizationViewControls m_Controls;

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

