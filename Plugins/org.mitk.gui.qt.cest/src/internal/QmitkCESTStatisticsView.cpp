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

// itk
#include "itksys/SystemTools.hxx"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkCESTStatisticsView.h"

// Qt
#include <QMessageBox>
#include <qclipboard.h>

// qwt
#include <qwt_scale_engine.h>

// mitk
#include <mitkCESTImageNormalizationFilter.h>
#include <mitkCustomTagParser.h>
#include <mitkITKImageImport.h>
#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImageStatisticsContainerNodeHelper.h>
#include <mitkLocaleSwitch.h>
#include <mitkSliceNavigationController.h>
#include <mitkStatisticsToImageRelationRule.h>
#include <mitkStatisticsToMaskRelationRule.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTimeGeometry.h>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

// stl
#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace
{
  template <typename T, typename Compare>
  void GetSortPermutation(std::vector<unsigned> &out,
                          const std::vector<T> &determiningVector,
                          Compare compare = std::less<T>())
  {
    out.resize(determiningVector.size());
    std::iota(out.begin(), out.end(), 0);

    std::sort(out.begin(), out.end(), [&](unsigned i, unsigned j) {
      return compare(determiningVector[i], determiningVector[j]);
    });
  }

  template <typename T>
  void ApplyPermutation(const std::vector<unsigned> &order, std::vector<T> &vectorToSort)
  {
    assert(order.size() == vectorToSort.size());
    std::vector<T> tempVector(vectorToSort.size());
    for (unsigned i = 0; i < vectorToSort.size(); i++)
    {
      tempVector[i] = vectorToSort[order[i]];
    }
    vectorToSort = tempVector;
  }

  template <typename T, typename... S>
  void ApplyPermutation(const std::vector<unsigned> &order,
                        std::vector<T> &currentVector,
                        std::vector<S> &... remainingVectors)
  {
    ApplyPermutation(order, currentVector);
    ApplyPermutation(order, remainingVectors...);
  }

  template <typename T, typename Compare, typename... SS>
  void SortVectors(const std::vector<T> &orderDeterminingVector,
                   Compare comparison,
                   std::vector<SS> &... vectorsToBeSorted)
  {
    std::vector<unsigned> order;
    GetSortPermutation(order, orderDeterminingVector, comparison);
    ApplyPermutation(order, vectorsToBeSorted...);
  }
} // namespace

const std::string QmitkCESTStatisticsView::VIEW_ID = "org.mitk.views.ceststatistics";
static const int STAT_TABLE_BASE_HEIGHT = 180;

QmitkCESTStatisticsView::QmitkCESTStatisticsView(QObject * /*parent*/, const char * /*name*/)
{
  this->m_CalculatorJob = new QmitkImageStatisticsCalculationJob();

  m_currentSelectedPosition.Fill(0.0);
  m_currentSelectedTimeStep = 0;
  m_CrosshairPointSet = mitk::PointSet::New();
}

QmitkCESTStatisticsView::~QmitkCESTStatisticsView()
{
  while (this->m_CalculatorJob->isRunning()) // wait until thread has finished
  {
    itksys::SystemTools::Delay(100);
  }
  delete this->m_CalculatorJob;
}

void QmitkCESTStatisticsView::SetFocus()
{
  m_Controls.threeDimToFourDimPushButton->setFocus();
}

void QmitkCESTStatisticsView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(
    m_Controls.threeDimToFourDimPushButton, SIGNAL(clicked()), this, SLOT(OnThreeDimToFourDimPushButtonClicked()));
  connect((QObject *)this->m_CalculatorJob,
          SIGNAL(finished()),
          this,
          SLOT(OnThreadedStatisticsCalculationEnds()),
          Qt::QueuedConnection);
  connect((QObject *)(this->m_Controls.fixedRangeCheckBox),
          SIGNAL(toggled(bool)),
          (QObject *)this,
          SLOT(OnFixedRangeCheckBoxToggled(bool)));
  connect((QObject *)(this->m_Controls.fixedRangeLowerDoubleSpinBox),
          SIGNAL(editingFinished()),
          (QObject *)this,
          SLOT(OnFixedRangeDoubleSpinBoxChanged()));
  connect((QObject *)(this->m_Controls.fixedRangeUpperDoubleSpinBox),
          SIGNAL(editingFinished()),
          (QObject *)this,
          SLOT(OnFixedRangeDoubleSpinBoxChanged()));

  m_Controls.threeDimToFourDimPushButton->setEnabled(false);

  m_Controls.widget_statistics->SetDataStorage(this->GetDataStorage());

  this->m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));
}

void QmitkCESTStatisticsView::RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart)
{
  this->m_SliceChangeListener.RenderWindowPartActivated(renderWindowPart);
}

void QmitkCESTStatisticsView::RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart)
{
  this->m_SliceChangeListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkCESTStatisticsView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                 const QList<mitk::DataNode::Pointer> &nodes)
{
  if (nodes.empty())
  {
    std::stringstream message;
    message << "<font color='red'>Please select an image.</font>";
    m_Controls.labelWarning->setText(message.str().c_str());
    m_Controls.labelWarning->show();

    this->Clear();
    return;
  }

  // iterate all selected objects
  bool atLeastOneWasCESTImage = false;
  foreach (mitk::DataNode::Pointer node, nodes)
  {
    if (node.IsNull())
    {
      continue;
    }

    if (dynamic_cast<mitk::Image *>(node->GetData()) != nullptr)
    {
      m_Controls.labelWarning->setVisible(false);

      bool zSpectrumSet = SetZSpectrum(dynamic_cast<mitk::StringProperty *>(
        node->GetData()->GetProperty(mitk::CustomTagParser::m_OffsetsPropertyName.c_str()).GetPointer()));

      atLeastOneWasCESTImage = atLeastOneWasCESTImage || zSpectrumSet;

      if (zSpectrumSet)
      {
        m_ZImage = dynamic_cast<mitk::Image *>(node->GetData());
        m_Controls.widget_statistics->SetImageNodes({node.GetPointer()});
      }
      else
      {
        m_MaskImage = dynamic_cast<mitk::Image *>(node->GetData());
        m_Controls.widget_statistics->SetMaskNodes({node.GetPointer()});
      }
    }

    if (dynamic_cast<mitk::PlanarFigure *>(node->GetData()) != nullptr)
    {
      m_MaskPlanarFigure = dynamic_cast<mitk::PlanarFigure *>(node->GetData());
      m_Controls.widget_statistics->SetMaskNodes({node.GetPointer()});
    }

    if (dynamic_cast<mitk::PointSet *>(node->GetData()) != nullptr)
    {
      m_PointSet = dynamic_cast<mitk::PointSet *>(node->GetData());
    }
  }

  // We only want to offer normalization or timestep copying if one object is selected
  if (nodes.size() == 1)
  {
    if (dynamic_cast<mitk::Image *>(nodes.front()->GetData()))
    {
      m_Controls.threeDimToFourDimPushButton->setDisabled(atLeastOneWasCESTImage);
    }
    else
    {
      m_Controls.threeDimToFourDimPushButton->setEnabled(false);

      std::stringstream message;
      message << "<font color='red'>The selected node is not an image.</font>";
      m_Controls.labelWarning->setText(message.str().c_str());
      m_Controls.labelWarning->show();
    }
    this->Clear();
    return;
  }

  // we always need a mask, either image or planar figure as well as an image for further processing
  if (nodes.size() != 2)
  {
    this->Clear();
    return;
  }

  m_Controls.threeDimToFourDimPushButton->setEnabled(false);

  if (!atLeastOneWasCESTImage)
  {
    std::stringstream message;
    message << "<font color='red'>None of the selected data nodes contains required CEST meta information</font>";
    m_Controls.labelWarning->setText(message.str().c_str());
    m_Controls.labelWarning->show();
    this->Clear();
    return;
  }

  bool bothAreImages = (m_ZImage.GetPointer() != nullptr) && (m_MaskImage.GetPointer() != nullptr);

  if (bothAreImages)
  {
    bool geometriesMatch =
      mitk::Equal(*(m_ZImage->GetTimeGeometry()), *(m_MaskImage->GetTimeGeometry()), mitk::eps, false);

    if (!geometriesMatch)
    {
      std::stringstream message;
      message << "<font color='red'>The selected images have different geometries.</font>";
      m_Controls.labelWarning->setText(message.str().c_str());
      m_Controls.labelWarning->show();
      this->Clear();
      return;
    }
  }

  if (!this->DataSanityCheck())
  {
    this->Clear();
    return;
  }

  if (m_PointSet.IsNull())
  {
    // initialize thread and trigger it
    this->m_CalculatorJob->SetIgnoreZeroValueVoxel(false);
    this->m_CalculatorJob->Initialize(m_ZImage.GetPointer(), m_MaskImage.GetPointer(), m_MaskPlanarFigure.GetPointer());
    std::stringstream message;
    message << "<font color='red'>Calculating statistics...</font>";
    m_Controls.labelWarning->setText(message.str().c_str());
    m_Controls.labelWarning->show();

    try
    {
      // Compute statistics
      this->m_CalculatorJob->start();
    }
    catch (const mitk::Exception &e)
    {
      std::stringstream message;
      message << "<font color='red'>" << e.GetDescription() << "</font>";
      m_Controls.labelWarning->setText(message.str().c_str());
      m_Controls.labelWarning->show();
    }
    catch (const std::runtime_error &e)
    {
      // In case of exception, print error message on GUI
      std::stringstream message;
      message << "<font color='red'>" << e.what() << "</font>";
      m_Controls.labelWarning->setText(message.str().c_str());
      m_Controls.labelWarning->show();
    }
    catch (const std::exception &e)
    {
      MITK_ERROR << "Caught exception: " << e.what();
      // In case of exception, print error message on GUI
      std::stringstream message;
      message << "<font color='red'>Error! Unequal Dimensions of Image and Segmentation. No recompute possible </font>";
      m_Controls.labelWarning->setText(message.str().c_str());
      m_Controls.labelWarning->show();
    }

    while (this->m_CalculatorJob->isRunning()) // wait until thread has finished
    {
      itksys::SystemTools::Delay(100);
    }
  }

  if (m_PointSet.IsNotNull())
  {
    if (m_ZImage->GetDimension() == 4)
    {
      AccessFixedDimensionByItk(m_ZImage, PlotPointSet, 4);
    }
    else
    {
      MITK_WARN << "Expecting a 4D image.";
    }
  }
}

void QmitkCESTStatisticsView::OnThreadedStatisticsCalculationEnds()
{
  this->m_Controls.m_DataViewWidget->SetAxisTitle(QwtPlot::Axis::xBottom, "delta w");
  this->m_Controls.m_DataViewWidget->SetAxisTitle(QwtPlot::Axis::yLeft, "z");

  if (this->m_CalculatorJob->GetStatisticsUpdateSuccessFlag())
  {
    auto statistics = this->m_CalculatorJob->GetStatisticsData();

    std::string statisticsNodeName = "CEST_statistics";
    auto statisticsNode = mitk::CreateImageStatisticsNode(statistics, statisticsNodeName);
    auto imageRule = mitk::StatisticsToImageRelationRule::New();
    imageRule->Connect(statistics, m_CalculatorJob->GetStatisticsImage());

    if (m_CalculatorJob->GetMaskImage())
    {
      auto maskRule = mitk::StatisticsToMaskRelationRule::New();
      maskRule->Connect(statistics, m_CalculatorJob->GetMaskImage());
    }
    else if (m_CalculatorJob->GetPlanarFigure())
    {
      auto planarFigureRule = mitk::StatisticsToMaskRelationRule::New();
      planarFigureRule->Connect(statistics, m_CalculatorJob->GetPlanarFigure());
    }

    this->GetDataStorage()->Add(statisticsNode);

    QmitkPlotWidget::DataVector::size_type numberOfSpectra = this->m_zSpectrum.size();

    QmitkPlotWidget::DataVector means(numberOfSpectra);
    QmitkPlotWidget::DataVector stdevs(numberOfSpectra);

    for (unsigned int index = 0; index < numberOfSpectra; ++index)
    {
      means[index] =
        statistics->GetStatisticsForTimeStep(index).GetValueConverted<mitk::ImageStatisticsContainer::RealType>(
          mitk::ImageStatisticsConstants::MEAN());
      stdevs[index] =
        statistics->GetStatisticsForTimeStep(index).GetValueConverted<mitk::ImageStatisticsContainer::RealType>(
          mitk::ImageStatisticsConstants::STANDARDDEVIATION());
    }

    QmitkPlotWidget::DataVector xValues = this->m_zSpectrum;

    RemoveMZeros(xValues, means, stdevs);
    ::SortVectors(xValues, std::less<qreal>(), xValues, means, stdevs);

    unsigned int curveId = this->m_Controls.m_DataViewWidget->InsertCurve("Spectrum");
    this->m_Controls.m_DataViewWidget->SetCurveData(curveId, xValues, means, stdevs, stdevs);
    this->m_Controls.m_DataViewWidget->SetErrorPen(curveId, QPen(Qt::blue));
    QwtSymbol *blueSymbol = new QwtSymbol(QwtSymbol::Rect, QColor(Qt::blue), QColor(Qt::blue), QSize(8, 8));
    this->m_Controls.m_DataViewWidget->SetCurveSymbol(curveId, blueSymbol);
    this->m_Controls.m_DataViewWidget->SetLegendAttribute(curveId, QwtPlotCurve::LegendShowSymbol);

    QwtLegend *legend = new QwtLegend();
    legend->setFrameShape(QFrame::Box);
    legend->setFrameShadow(QFrame::Sunken);
    legend->setLineWidth(1);
    this->m_Controls.m_DataViewWidget->SetLegend(legend, QwtPlot::BottomLegend);

    m_Controls.m_DataViewWidget->GetPlot()
      ->axisScaleEngine(QwtPlot::Axis::xBottom)
      ->setAttributes(QwtScaleEngine::Inverted);

    this->m_Controls.m_DataViewWidget->Replot();
    m_Controls.labelWarning->setVisible(false);

    m_Controls.m_StatisticsGroupBox->setEnabled(true);
    m_Controls.m_StatisticsGroupBox->setEnabled(true);

    if (this->m_Controls.fixedRangeCheckBox->isChecked())
    {
      this->m_Controls.m_DataViewWidget->GetPlot()->setAxisAutoScale(2, false);
      this->m_Controls.m_DataViewWidget->GetPlot()->setAxisScale(
        2,
        this->m_Controls.fixedRangeLowerDoubleSpinBox->value(),
        this->m_Controls.fixedRangeUpperDoubleSpinBox->value());
    }
    else
    {
      this->m_Controls.m_DataViewWidget->GetPlot()->setAxisAutoScale(2, true);
    }
  }
  else
  {
    m_Controls.labelWarning->setText(m_CalculatorJob->GetLastErrorMessage().c_str());
    m_Controls.labelWarning->setVisible(true);
    this->Clear();
  }
}

void QmitkCESTStatisticsView::OnFixedRangeDoubleSpinBoxChanged()
{
  if (this->m_Controls.fixedRangeCheckBox->isChecked())
  {
    this->m_Controls.m_DataViewWidget->GetPlot()->setAxisAutoScale(2, false);
    this->m_Controls.m_DataViewWidget->GetPlot()->setAxisScale(2,
                                                               this->m_Controls.fixedRangeLowerDoubleSpinBox->value(),
                                                               this->m_Controls.fixedRangeUpperDoubleSpinBox->value());
  }

  this->m_Controls.m_DataViewWidget->Replot();
}

template <typename TPixel, unsigned int VImageDimension>
void QmitkCESTStatisticsView::PlotPointSet(itk::Image<TPixel, VImageDimension> *image)
{
  this->m_Controls.m_DataViewWidget->SetAxisTitle(QwtPlot::Axis::xBottom, "delta w");
  this->m_Controls.m_DataViewWidget->SetAxisTitle(QwtPlot::Axis::yLeft, "z");

  QmitkPlotWidget::DataVector::size_type numberOfSpectra = this->m_zSpectrum.size();
  mitk::PointSet::Pointer internalPointset;

  if (m_PointSet.IsNotNull())
  {
    internalPointset = m_PointSet;
  }
  else
  {
    internalPointset = m_CrosshairPointSet;
  }

  if (internalPointset.IsNull())
  {
    return;
  }

  if (!this->DataSanityCheck())
  {
    m_Controls.labelWarning->setText("Data can not be plotted, internally inconsistent.");
    m_Controls.labelWarning->show();
    return;
  }

  auto maxIndex = internalPointset->GetMaxId().Index();

  for (std::size_t number = 0; number < maxIndex + 1; ++number)
  {
    mitk::PointSet::PointType point;
    if (!internalPointset->GetPointIfExists(number, &point))
    {
      continue;
    }

    if (!this->m_ZImage->GetGeometry()->IsInside(point))
    {
      continue;
    }

    itk::Index<3> itkIndex;

    this->m_ZImage->GetGeometry()->WorldToIndex(point, itkIndex);

    itk::Index<VImageDimension> itkIndexTime;
    itkIndexTime[0] = itkIndex[0];
    itkIndexTime[1] = itkIndex[1];
    itkIndexTime[2] = itkIndex[2];

    QmitkPlotWidget::DataVector values(numberOfSpectra);

    for (std::size_t step = 0; step < numberOfSpectra; ++step)
    {
      if (VImageDimension == 4)
      {
        itkIndexTime[3] = step;
      }

      values[step] = image->GetPixel(itkIndexTime);
    }

    std::stringstream name;
    name << "Point " << number;

    // Qcolor enums go from 0 to 19, but 19 is transparent and 0,1 are for bitmaps
    // 3 is white and thus not visible
    QColor color(static_cast<Qt::GlobalColor>(number % 17 + 4));

    QmitkPlotWidget::DataVector xValues = this->m_zSpectrum;

    RemoveMZeros(xValues, values);
    ::SortVectors(xValues, std::less<qreal>(), xValues, values);

    unsigned int curveId = this->m_Controls.m_DataViewWidget->InsertCurve(name.str().c_str());
    this->m_Controls.m_DataViewWidget->SetCurveData(curveId, xValues, values);
    this->m_Controls.m_DataViewWidget->SetCurvePen(curveId, QPen(color));
    QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Rect, color, color, QSize(8, 8));
    this->m_Controls.m_DataViewWidget->SetCurveSymbol(curveId, symbol);
    this->m_Controls.m_DataViewWidget->SetLegendAttribute(curveId, QwtPlotCurve::LegendShowSymbol);
  }

  if (this->m_Controls.fixedRangeCheckBox->isChecked())
  {
    this->m_Controls.m_DataViewWidget->GetPlot()->setAxisAutoScale(2, false);
    this->m_Controls.m_DataViewWidget->GetPlot()->setAxisScale(2,
                                                               this->m_Controls.fixedRangeLowerDoubleSpinBox->value(),
                                                               this->m_Controls.fixedRangeUpperDoubleSpinBox->value());
  }
  else
  {
    this->m_Controls.m_DataViewWidget->GetPlot()->setAxisAutoScale(2, true);
  }

  QwtLegend *legend = new QwtLegend();
  legend->setFrameShape(QFrame::Box);
  legend->setFrameShadow(QFrame::Sunken);
  legend->setLineWidth(1);
  this->m_Controls.m_DataViewWidget->SetLegend(legend, QwtPlot::BottomLegend);

  m_Controls.m_DataViewWidget->GetPlot()
    ->axisScaleEngine(QwtPlot::Axis::xBottom)
    ->setAttributes(QwtScaleEngine::Inverted);

  this->m_Controls.m_DataViewWidget->Replot();
  m_Controls.labelWarning->setVisible(false);
}

void QmitkCESTStatisticsView::OnFixedRangeCheckBoxToggled(bool state)
{
  this->m_Controls.fixedRangeLowerDoubleSpinBox->setEnabled(state);
  this->m_Controls.fixedRangeUpperDoubleSpinBox->setEnabled(state);
}

void QmitkCESTStatisticsView::RemoveMZeros(QmitkPlotWidget::DataVector &xValues, QmitkPlotWidget::DataVector &yValues)
{
  QmitkPlotWidget::DataVector tempX;
  QmitkPlotWidget::DataVector tempY;
  for (std::size_t index = 0; index < xValues.size(); ++index)
  {
    if ((xValues.at(index) < -299) || (xValues.at(index)) > 299)
    {
      // do not include
    }
    else
    {
      tempX.push_back(xValues.at(index));
      tempY.push_back(yValues.at(index));
    }
  }

  xValues = tempX;
  yValues = tempY;
}

void QmitkCESTStatisticsView::RemoveMZeros(QmitkPlotWidget::DataVector &xValues,
                                           QmitkPlotWidget::DataVector &yValues,
                                           QmitkPlotWidget::DataVector &stdDevs)
{
  QmitkPlotWidget::DataVector tempX;
  QmitkPlotWidget::DataVector tempY;
  QmitkPlotWidget::DataVector tempDevs;
  for (std::size_t index = 0; index < xValues.size(); ++index)
  {
    if ((xValues.at(index) < -299) || (xValues.at(index)) > 299)
    {
      // do not include
    }
    else
    {
      tempX.push_back(xValues.at(index));
      tempY.push_back(yValues.at(index));
      tempDevs.push_back(stdDevs.at(index));
    }
  }

  xValues = tempX;
  yValues = tempY;
  stdDevs = tempDevs;
}

void QmitkCESTStatisticsView::OnThreeDimToFourDimPushButtonClicked()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty())
    return;

  mitk::DataNode *node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information(nullptr, "CEST View", "Please load and select an image before starting image processing.");
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData *data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image *image = dynamic_cast<mitk::Image *>(data);
    if (image)
    {
      if (image->GetDimension() == 4)
      {
        AccessFixedDimensionByItk(image, CopyTimesteps, 4);
      }

      this->Clear();
    }
  }
}

template <typename TPixel, unsigned int VImageDimension>
void QmitkCESTStatisticsView::CopyTimesteps(itk::Image<TPixel, VImageDimension> *image)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  // typedef itk::PasteImageFilter<ImageType, ImageType>    PasteImageFilterType;

  unsigned int numberOfTimesteps = image->GetLargestPossibleRegion().GetSize(3);

  typename ImageType::RegionType sourceRegion = image->GetLargestPossibleRegion();
  sourceRegion.SetSize(3, 1);
  typename ImageType::RegionType targetRegion = image->GetLargestPossibleRegion();
  targetRegion.SetSize(3, 1);

  for (unsigned int timestep = 1; timestep < numberOfTimesteps; ++timestep)
  {
    targetRegion.SetIndex(3, timestep);
    itk::ImageRegionConstIterator<ImageType> sourceIterator(image, sourceRegion);
    itk::ImageRegionIterator<ImageType> targetIterator(image, targetRegion);
    while (!sourceIterator.IsAtEnd())
    {
      targetIterator.Set(sourceIterator.Get());

      ++sourceIterator;
      ++targetIterator;
    }
  }
}

bool QmitkCESTStatisticsView::SetZSpectrum(mitk::StringProperty *zSpectrumProperty)
{
  if (zSpectrumProperty == nullptr)
  {
    return false;
  }
  mitk::LocaleSwitch localeSwitch("C");

  std::string zSpectrumString = zSpectrumProperty->GetValueAsString();
  std::istringstream iss(zSpectrumString);
  std::vector<std::string> zSpectra;
  std::copy(
    std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(zSpectra));

  m_zSpectrum.clear();
  m_zSpectrum.resize(0);

  for (auto const &spectrumString : zSpectra)
  {
    m_zSpectrum.push_back(std::stod(spectrumString));
  }

  return (m_zSpectrum.size() > 0);
}

bool QmitkCESTStatisticsView::DataSanityCheck()
{
  QmitkPlotWidget::DataVector::size_type numberOfSpectra = m_zSpectrum.size();

  // if we do not have a spectrum, the data can not be processed
  if (numberOfSpectra == 0)
  {
    return false;
  }

  // if we do not have CEST image data, the data can not be processed
  if (m_ZImage.IsNull())
  {
    return false;
  }

  // if the CEST image data and the meta information do not match, the data can not be processed
  if (numberOfSpectra != m_ZImage->GetTimeSteps())
  {
    MITK_INFO << "CEST meta information and number of volumes does not match.";
    return false;
  }

  // if we have neither a mask image, a point set nor a mask planar figure, we can not do statistics
  // statistics on the whole image would not make sense
  if (m_MaskImage.IsNull() && m_MaskPlanarFigure.IsNull() && m_PointSet.IsNull() && m_CrosshairPointSet->IsEmpty())
  {
    return false;
  }

  // if we have a mask image and a mask planar figure, we can not do statistics
  // we do not know which one to use
  if (m_MaskImage.IsNotNull() && m_MaskPlanarFigure.IsNotNull())
  {
    return false;
  }

  return true;
}

void QmitkCESTStatisticsView::Clear()
{
  this->m_zSpectrum.clear();
  this->m_zSpectrum.resize(0);
  this->m_ZImage = nullptr;
  this->m_MaskImage = nullptr;
  this->m_MaskPlanarFigure = nullptr;
  this->m_PointSet = nullptr;
  this->m_Controls.m_DataViewWidget->Clear();
  this->m_Controls.m_StatisticsGroupBox->setEnabled(false);
  this->m_Controls.widget_statistics->SetImageNodes({});
  this->m_Controls.widget_statistics->SetMaskNodes({});
}

void QmitkCESTStatisticsView::OnSliceChanged()
{
  mitk::Point3D currentSelectedPosition = this->GetRenderWindowPart()->GetSelectedPosition(nullptr);
  unsigned int currentSelectedTimeStep =
    this->GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();

  if (m_currentSelectedPosition != currentSelectedPosition || m_currentSelectedTimeStep != currentSelectedTimeStep)
  //|| m_selectedNodeTime > m_currentPositionTime)
  {
    // the current position has been changed or the selected node has been changed since the last position validation ->
    // check position
    m_currentSelectedPosition = currentSelectedPosition;
    m_currentSelectedTimeStep = currentSelectedTimeStep;
    m_currentPositionTime.Modified();

    m_CrosshairPointSet->Clear();

    m_CrosshairPointSet->SetPoint(0, m_currentSelectedPosition);

    QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
    if (nodes.empty() || nodes.size() > 1)
      return;

    mitk::DataNode *node = nodes.front();

    if (!node)
    {
      return;
    }

    if (dynamic_cast<mitk::Image *>(node->GetData()) != nullptr)
    {
      m_Controls.labelWarning->setVisible(false);
      bool zSpectrumSet = SetZSpectrum(dynamic_cast<mitk::StringProperty *>(
        node->GetData()->GetProperty(mitk::CustomTagParser::m_OffsetsPropertyName.c_str()).GetPointer()));

      if (zSpectrumSet)
      {
        m_ZImage = dynamic_cast<mitk::Image *>(node->GetData());
      }
      else
      {
        return;
      }
    }
    else
    {
      return;
    }

    this->m_Controls.m_DataViewWidget->Clear();

    AccessFixedDimensionByItk(m_ZImage, PlotPointSet, 4);
  }
}
