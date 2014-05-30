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


#ifndef QmitkRegionGrowingView_h
#define QmitkRegionGrowingView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkRegionGrowingViewControls.h"

//! [includes]
#include "mitkPointSet.h"
#include "mitkIRenderWindowPartListener.h"
#include <itkImage.h>

class QmitkPointListWidget;
//! [includes]

/**
  \brief QmitkRegionGrowingView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QmitkRegionGrowingView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkRegionGrowingView();

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void DoImageProcessing();

  protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes );

    //! [render-window-part-listener]
    void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart);
    void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart);
    //! [render-window-part-listener]

    Ui::QmitkRegionGrowingViewControls m_Controls;

  private:

    //! [itkimageprocessing]
    /**
      \brief ITK image processing function
      This function is templated like an ITK image. The MITK-Macro AccessByItk determines the actual pixel type and dimensionality of
      a given MITK image and calls this function for further processing (in our case region growing)
    */
    template < typename TPixel, unsigned int VImageDimension >
    void ItkImageProcessing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::BaseGeometry* imageGeometry );
    //! [itkimageprocessing]

    //! [members]
    /// \brief This is the actual seed point data object
    mitk::PointSet::Pointer m_PointSet;

    QmitkPointListWidget* m_PointListWidget;
    //! [members]

};

#endif // QmitkRegionGrowingView_h
