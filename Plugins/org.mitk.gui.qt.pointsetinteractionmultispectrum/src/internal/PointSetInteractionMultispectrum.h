/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef PointSetInteractionMultispectrum_h
#define PointSetInteractionMultispectrum_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <qwt_plot.h>
#include "ui_PointSetInteractionMultispectrumControls.h"

/////////////added from the regiongrowing plugin//////////////
#include "mitkPointSet.h"                              ///////
#include "mitkIRenderWindowPartListener.h"             ///////
#include <itkImage.h>                                  ///////
//////////////////////////////////////////////////////////////
/**
  \brief PointSetInteractionMultispectrum

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/



class QmitkPointListWidget;



class PointSetInteractionMultispectrum : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;
    PointSetInteractionMultispectrum();


  protected slots:

    /// \brief Called when the user clicks the GUI button
    void DoImageProcessing();

  protected:


    void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart);
    void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart);


    void CreateQtPartControl(QWidget *parent) override;

    void SetFocus() override;

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes ) override;

    Ui::PointSetInteractionMultispectrumControls m_Controls;

  private:

    void PlotReflectance(mitk::PointSet::Pointer m_PointSet, QList<mitk::DataNode::Pointer> dataManagerNodes);

    mitk::PointSet::Pointer m_PointSet;
    QmitkPointListWidget* m_PointListWidget;
    QwtPlot* m_Plot;




};

#endif // PointSetInteractionMultispectrum_h
