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

#ifndef QmitkTractbasedSpatialStatisticsView_h
#define QmitkTractbasedSpatialStatisticsView_h


#include <QmitkFunctionality.h>

#include "ui_QmitkTractbasedSpatialStatisticsViewControls.h"
#include <QListWidgetItem>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <mitkDataNodeFactory.h>

#include <itkDiffusionTensor3D.h>

#include <mitkTbssImage.h>
#include <mitkTbssRoiImage.h>

#include "QmitkTbssTableModel.h"
#include "QmitkTbssMetaTableModel.h"

#include <mitkFiberBundleX.h>


// Image types
typedef short DiffusionPixelType;
typedef itk::Image<char, 3> CharImageType;
typedef itk::Image<unsigned char, 3> UCharImageType;
typedef itk::Image<float, 4> Float4DImageType;
typedef itk::Image<float, 3> FloatImageType;
typedef itk::VectorImage<float, 3> VectorImageType;

// Readers/Writers
typedef itk::ImageFileReader< CharImageType > CharReaderType;
typedef itk::ImageFileReader< UCharImageType > UCharReaderType;
typedef itk::ImageFileWriter< CharImageType > CharWriterType;
typedef itk::ImageFileReader< FloatImageType > FloatReaderType;
typedef itk::ImageFileWriter< FloatImageType > FloatWriterType;
typedef itk::ImageFileReader< Float4DImageType > Float4DReaderType;
typedef itk::ImageFileWriter< Float4DImageType > Float4DWriterType;



/*!
  * \brief This plugin provides an extension for Tract-based spatial statistics (see Smith et al., 2009. http://dx.doi.org/10.1016/j.neuroimage.2006.02.024)
  * TBSS enables analyzing the brain by a pipeline of registration, skeletonization, and projection that results in a white matter skeleton
  * for all subjects that are analyzed statistically in a whole-brain manner.
  * This plugin provides functionality to select single tracts and analyze them separately.
  *
  * Prerequisites: the mean_FA_skeleton and all_FA_skeletonised datasets produced by the FSL TBSS pipeline: http://fsl.fmrib.ox.ac.uk/fsl/fsl4.0/tbss/index
*/

class QmitkTractbasedSpatialStatisticsView : public QmitkFunctionality
{

  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkTractbasedSpatialStatisticsView();
    virtual ~QmitkTractbasedSpatialStatisticsView();

    virtual void CreateQtPartControl(QWidget *parent);

    /// \brief Creation of the connections of main and control widget
    virtual void CreateConnections();

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

    /// \brief Called when the functionality is activated
    virtual void Activated();

    virtual void Deactivated();


  protected slots:

    // Creates Roi
    void CreateRoi();

    void Clicked(const QPointF& pos);

    // Import of FSL TBSS data
    void TbssImport();

    // Add a group as metadata. This metadata is required by the plotting functionality
    void AddGroup();

    // Remove a group
    void RemoveGroup();

    // Copies the values displayed in the plot widget to clipboard, i.e. exports the data
    void CopyToClipboard();

    // Method to cut away parts of fiber bundles that should not be plotted.
    void Cut();

    // Adjust plot widget
    void PerformChange();


  protected:


    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    // Creates a plot using a 4D image containing the projections of all subjects and a region of interest
    void Plot(mitk::TbssImage*, mitk::TbssRoiImage*);


    void PlotFiberBundle(mitk::FiberBundleX* fib, mitk::Image* img, mitk::PlanarFigure* startRoi=NULL, mitk::PlanarFigure* endRoi=NULL);


    void PlotFiber4D(mitk::TbssImage*, mitk::FiberBundleX* fib, mitk::PlanarFigure* startRoi=NULL, mitk::PlanarFigure* endRoi=NULL);

    // Create a point set. This point set defines the points through which a region of interest should go
    void InitPointsets();

    // Pointset and DataNode to contain the PointSet used in ROI creation
    mitk::PointSet::Pointer m_PointSetNode;
    mitk::DataNode::Pointer m_P1;

    // GUI widgets
    Ui::QmitkTractbasedSpatialStatisticsViewControls* m_Controls;

    /* A pointer to the QmitkStdMultiWidget. Used for interaction with the plot widget
    (clicking in the plot widget makes the image cross jump to the corresponding location
    on the skeleton).*/
    QmitkStdMultiWidget* m_MultiWidget;

    // Used to save the region of interest in a vector of itk::index.
    std::vector< itk::Index<3> > m_Roi;

    mitk::FiberBundleX* m_Fib;

    mitk::BaseGeometry* m_CurrentGeometry;

    // A table model for saving group information in a name,number pair.
    QmitkTbssTableModel* m_GroupModel;

    // Convenience function for adding a new image to the datastorage and giving it a name.
    void AddTbssToDataStorage(mitk::Image* image, std::string name);

    mitk::DataNode::Pointer m_CurrentFiberNode; // needed for the index property when interacting with the plot widget

    // needed when a plot should only show values between a start end end roi
    mitk::DataNode::Pointer m_CurrentStartRoi;
    mitk::DataNode::Pointer m_CurrentEndRoi;




};



#endif // _QMITKTRACTBASEDSPATIALSTATISTICSVIEW_H_INCLUDED

