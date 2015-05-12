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

#include <QmitkFunctionality.h>

#include "ui_QmitkStreamlineTrackingViewControls.h"

#include <mitkTensorImage.h>
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <itkImage.h>
#include <itkStreamlineTrackingFilter.h>


/*!
\brief View for tensor based deterministic streamline fiber tracking.
\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkStreamlineTrackingView : public QmitkFunctionality
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

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget) override;
    virtual void StdMultiWidgetNotAvailable() override;

protected slots:

    void DoFiberTracking();   ///< start fiber tracking

protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes ) override;
    Ui::QmitkStreamlineTrackingViewControls* m_Controls;
    QmitkStdMultiWidget* m_MultiWidget;

protected slots:

    /** update labels if parameters have changed */
    void OnSeedsPerVoxelChanged(int value);
    void OnMinTractLengthChanged(int value);
    void OnFaThresholdChanged(int value);
    void OnAngularThresholdChanged(int value);
    void OnfChanged(int value);
    void OngChanged(int value);
    void OnStepsizeChanged(int value);

private:

    mitk::Image::Pointer          m_MaskImage;        ///< abort tracking if leaving mask
    mitk::Image::Pointer          m_SeedRoi;          ///< binary image defining seed voxels for tracking process
    std::vector< mitk::DataNode::Pointer > m_TensorImageNodes; ///< input images
    std::vector< mitk::TensorImage::Pointer > m_TensorImages; ///< input image datanode
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

