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

#ifndef QmitkStochasticFiberTrackingView_h
#define QmitkStochasticFiberTrackingView_h

#include <QmitkAbstractView.h>
#include "ui_QmitkStochasticFiberTrackingViewControls.h"
#include <mitkDataStorage.h>
#include <itkVectorImage.h>
#include <itkImage.h>
#include <itkDTITubeSpatialObject.h>
#include <itkDTITubeSpatialObjectPoint.h>
#include <itkSceneSpatialObject.h>
#include <itkStochasticTractographyFilter.h>
#include <mitkDiffusionPropertyHelper.h>

/*!
\brief View for probabilistic streamline fiber tracking
*/
class QmitkStochasticFiberTrackingView : public QmitkAbstractView
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

  typedef short                                                           DiffusionPixelType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionType            GradientDirectionType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType  GradientDirectionContainerType;
  typedef mitk::DiffusionPropertyHelper::BValueMapType                    BValueMapType;
  typedef itk::VectorImage< DiffusionPixelType, 3 >                       ITKDiffusionImageType;

    typedef itk::Image< unsigned char, 3 >  BinaryImageType;

    static const std::string VIEW_ID;

    QmitkStochasticFiberTrackingView();
    virtual ~QmitkStochasticFiberTrackingView();

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

    Ui::QmitkStochasticFiberTrackingViewControls* m_Controls;

protected slots:

    /** update labels if parameters have changed */
    void OnSeedsPerVoxelChanged(int value);
    void OnMaxTractLengthChanged(int value);
    void OnMaxCacheSizeChanged(int value);

private:

    mitk::Image::Pointer                  m_SeedRoi;              ///< binary image defining seed voxels for tracking process
    mitk::Image::Pointer                  m_DiffusionImage;       ///< input image
    mitk::DataNode::Pointer               m_DiffusionImageNode;   ///< data node containing input image
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

