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


#ifndef QmitkCESTStatisticsView_h
#define QmitkCESTStatisticsView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkCESTStatisticsViewControls.h"
#include <QmitkImageStatisticsCalculationThread.h>

#include <mitkPointSet.h>

/**
  \brief QmitkCESTStatisticsView

  \warning  Basic statistics view for CEST data.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QmitkCESTStatisticsView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    /*!
    \brief default constructor */
    QmitkCESTStatisticsView(QObject *parent = nullptr, const char *name = nullptr);
    /*!
    \brief default destructor */
    virtual ~QmitkCESTStatisticsView();

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void OnThreeDimToFourDimPushButtonClicked();

    /// \brief takes care of processing the computed data
    void OnThreadedStatisticsCalculationEnds();

    /// \brief copy statistics to clipboard
    void OnCopyStatisticsToClipboardPushButtonClicked();

    /// \brief normalize cest image
    void OnNormalizeImagePushButtonClicked();

    /// \brief Toggle whether or not the plot uses a fixed x range
    void OnFixedRangeCheckBoxToggled(bool state);

    /// \brief Adapt axis scale when manual ranges are set
    void OnFixedRangeDoubleSpinBoxChanged();

  protected:

    virtual void CreateQtPartControl(QWidget *parent) override;

    virtual void SetFocus() override;

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes ) override;

    /** \brief  Writes the calculated statistics to the GUI */
    void FillStatisticsTableView(const std::vector<mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer> &s,
      const mitk::Image *image);


    /** \brief  Removes statistics from the GUI */
    void InvalidateStatisticsTableView();

    /// parse string and set data vector returns true if succesfull
    bool SetZSpectrum(mitk::StringProperty* zSpectrumProperty);

    /** Checks whether the currently set data appears reasonable
    */
    bool DataSanityCheck();

    /** Fills the plot based on a point set
    *
    * This will only use the first timestep
    */
    template <typename TPixel, unsigned int VImageDimension>
    void PlotPointSet(itk::Image<TPixel, VImageDimension>* image);

    /** Deletes all data
    */
    void Clear();

    /** Remove MZeros
    *
    * Will remove the data for the M0 images from the given input
    */
    void RemoveMZeros(QmitkPlotWidget::DataVector& xValues, QmitkPlotWidget::DataVector& yValues);
    void RemoveMZeros(QmitkPlotWidget::DataVector& xValues, QmitkPlotWidget::DataVector& yValues, QmitkPlotWidget::DataVector& stdDevs);

    /** Copies the first timestep of a segmentation to all others
    */
    template <typename TPixel, unsigned int VImageDimension>
    void CopyTimesteps(itk::Image<TPixel, VImageDimension>* image);

    Ui::QmitkCESTStatisticsViewControls m_Controls;
    QmitkImageStatisticsCalculationThread* m_CalculatorThread;
    QmitkPlotWidget::DataVector m_zSpectrum;
    mitk::Image::Pointer m_ZImage;
    mitk::Image::Pointer m_MaskImage;
    mitk::PlanarFigure::Pointer m_MaskPlanarFigure;
    mitk::PointSet::Pointer m_PointSet;

};

#endif // QmitkCESTStatisticsView_h
