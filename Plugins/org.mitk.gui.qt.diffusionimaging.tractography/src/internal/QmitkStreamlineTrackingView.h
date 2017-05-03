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

#ifndef QmitkStreamlineTrackingView_h
#define QmitkStreamlineTrackingView_h

#include <QmitkAbstractView.h>

#include "ui_QmitkStreamlineTrackingViewControls.h"

#include <mitkTensorImage.h>
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <itkImage.h>
#include <itkStreamlineTrackingFilter.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerTensor.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerPeaks.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerOdf.h>


/*!
\brief View for tensor based deterministic streamline fiber tracking.
*/
class QmitkStreamlineTrackingView : public QmitkAbstractView
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    typedef itk::Image< unsigned char, 3 > ItkUCharImageType;
    typedef itk::Image< float, 3 > ItkFloatImageType;

    QmitkStreamlineTrackingView();
    virtual ~QmitkStreamlineTrackingView();

    virtual void CreateQtPartControl(QWidget *parent) override;

    ///
    /// Sets the focus to an internal widget.
    ///
    virtual void SetFocus() override;

protected slots:

    void DoFiberTracking();   ///< start fiber tracking

protected:

    /// \brief called by QmitkAbstractView when DataManager's selection has changed
    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

    Ui::QmitkStreamlineTrackingViewControls* m_Controls;

protected slots:

    /** update labels if parameters have changed */
    void OnSeedsPerVoxelChanged(int value);
    void OnMinTractLengthChanged(int value);
    void OnfChanged(int value);
    void OngChanged(int value);

private:

    std::vector< mitk::DataNode::Pointer > m_InputImageNodes; ///< input images
    std::vector< mitk::Image::Pointer > m_InputImages; ///< input image datanode
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

