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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

// mitk
#include <QmitkRenderWindow.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QTableWidget>
#include <qwt_plot_marker.h>
#include "QmitkPlotWidget.h"

#include "mitkNodePredicateFunction.h"
#include "mitkScalarListLookupTableProperty.h"
#include "mitkModelFitConstants.h"
#include "mitkExtractTimeGrid.h"
#include "mitkModelGenerator.h"
#include "mitkModelFitException.h"
#include "mitkModelFitParameterValueExtraction.h"
#include "mitkTimeGridHelper.h"

#include "mitkModelFitPlotDataHelper.h"

#include "ModelFitInspectorView.h"

const std::string ModelFitInspectorView::VIEW_ID = "org.mitk.gui.gt.fit.inspector";
const unsigned int ModelFitInspectorView::INTERPOLATION_STEPS = 10;
const std::string DEFAULT_X_AXIS = "Time [s]";

ModelFitInspectorView::ModelFitInspectorView() :
  m_renderWindowPart(nullptr),
  m_internalUpdateFlag(false),
  m_currentFit(nullptr),
  m_currentModelParameterizer(nullptr),
  m_currentModelProviderService(nullptr),
  m_currentSelectedTimeStep(0),
  m_currentSelectedNode(nullptr)

{
  m_currentSelectedPosition.Fill(0.0);
  m_modelfitList.clear();
}

ModelFitInspectorView::~ModelFitInspectorView()
{
}

void ModelFitInspectorView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_renderWindowPart != renderWindowPart)
  {
    m_renderWindowPart = renderWindowPart;
  }

  this->m_SliceChangeListener.RenderWindowPartActivated(renderWindowPart);
}

void ModelFitInspectorView::RenderWindowPartDeactivated(
  mitk::IRenderWindowPart* renderWindowPart)
{
  m_renderWindowPart = nullptr;
  this->m_SliceChangeListener.RenderWindowPartDeactivated(renderWindowPart);
}

void ModelFitInspectorView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  m_SelectionServiceConnector = std::make_unique<QmitkSelectionServiceConnector>();
  m_SelectionServiceConnector->AddPostSelectionListener(this->GetSite()->GetWorkbenchWindow()->GetSelectionService());

  m_Controls.inputNodeSelector->SetDataStorage(GetDataStorage());
  m_Controls.inputNodeSelector->SetEmptyInfo(QString("Please select input data to be viewed."));
  m_Controls.inputNodeSelector->SetInvalidInfo(QString("<b><font color=\"red\">No input data is selected</font></b>"));
  m_Controls.inputNodeSelector->SetPopUpTitel(QString("Choose 3D+t input data that should be viewed!"));
  m_Controls.inputNodeSelector->SetSelectionIsOptional(false);
  m_Controls.inputNodeSelector->SetSelectOnlyVisibleNodes(true);

  auto predicate = mitk::NodePredicateFunction::New([](const mitk::DataNode *node) {
    bool isModelFitNode = node->GetData() && node->GetData()->GetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str()).IsNotNull();
    return isModelFitNode || (node && node->GetData() && node->GetData()->GetTimeSteps() > 1);
  });

  m_Controls.inputNodeSelector->SetNodePredicate(predicate);

  connect(m_SelectionServiceConnector.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged, m_Controls.inputNodeSelector, &QmitkSingleNodeSelectionWidget::SetCurrentSelection);
  connect(m_Controls.inputNodeSelector, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(OnInputChanged(const QList<mitk::DataNode::Pointer>&)));

  this->m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));

  connect(m_Controls.cmbFit, SIGNAL(currentIndexChanged(int)), this,
          SLOT(OnFitSelectionChanged(int)));

  connect(m_Controls.radioScaleFixed, SIGNAL(toggled(bool)), m_Controls.sbFixMin,
          SLOT(setEnabled(bool)));
  connect(m_Controls.radioScaleFixed, SIGNAL(toggled(bool)), m_Controls.sbFixMax,
          SLOT(setEnabled(bool)));
  connect(m_Controls.radioScaleFixed, SIGNAL(toggled(bool)), m_Controls.labelFixMin,
          SLOT(setEnabled(bool)));
  connect(m_Controls.radioScaleFixed, SIGNAL(toggled(bool)), m_Controls.labelFixMax,
          SLOT(setEnabled(bool)));
  connect(m_Controls.radioScaleFixed, SIGNAL(toggled(bool)), m_Controls.btnScaleToData,
          SLOT(setEnabled(bool)));

  connect(m_Controls.radioScaleFixed, SIGNAL(toggled(bool)), this, SLOT(OnScaleFixedYChecked(bool)));

  connect(m_Controls.btnScaleToData, SIGNAL(clicked()), this, SLOT(OnScaleToDataYClicked()));
  connect(m_Controls.sbFixMax, SIGNAL(valueChanged(double)), this,
          SLOT(OnFixedScalingYChanged(double)));
  connect(m_Controls.sbFixMin, SIGNAL(valueChanged(double)), this,
          SLOT(OnFixedScalingYChanged(double)));

  connect(m_Controls.radioScaleFixed_x, SIGNAL(toggled(bool)), m_Controls.sbFixMin_x,
    SLOT(setEnabled(bool)));
  connect(m_Controls.radioScaleFixed_x, SIGNAL(toggled(bool)), m_Controls.sbFixMax_x,
    SLOT(setEnabled(bool)));
  connect(m_Controls.radioScaleFixed_x, SIGNAL(toggled(bool)), m_Controls.labelFixMin_x,
    SLOT(setEnabled(bool)));
  connect(m_Controls.radioScaleFixed_x, SIGNAL(toggled(bool)), m_Controls.labelFixMax_x,
    SLOT(setEnabled(bool)));
  connect(m_Controls.radioScaleFixed_x, SIGNAL(toggled(bool)), m_Controls.btnScaleToData_x,
    SLOT(setEnabled(bool)));

  connect(m_Controls.radioScaleFixed_x, SIGNAL(toggled(bool)), this, SLOT(OnScaleFixedXChecked(bool)));

  connect(m_Controls.btnScaleToData_x, SIGNAL(clicked()), this, SLOT(OnScaleToDataXClicked()));
  connect(m_Controls.sbFixMax_x, SIGNAL(valueChanged(double)), this,
    SLOT(OnFixedScalingXChanged(double)));
  connect(m_Controls.sbFixMin_x, SIGNAL(valueChanged(double)), this,
    SLOT(OnFixedScalingXChanged(double)));

  connect(m_Controls.btnFullPlot, SIGNAL(clicked(bool)), this, SLOT(OnFullPlotClicked(bool)));

  this->EnsureBookmarkPointSet();
  m_Controls.inspectionPositionWidget->SetPositionBookmarkNode(m_PositionBookmarksNode.Lock());

  connect(m_Controls.inspectionPositionWidget, SIGNAL(PositionBookmarksChanged()), this, SLOT(OnPositionBookmarksChanged()));

  // For some reason this needs to be called to set the plot widget's minimum width to an
  // acceptable level (since Qwt 6).
  // Otherwise it tries to keep both axes equal in length, resulting in a minimum width of
  // 400-500px which is way too much.
  m_Controls.widgetPlot->GetPlot()->updateAxes();

  m_Controls.cmbFit->clear();

  mitk::IRenderWindowPart* renderWindowPart = GetRenderWindowPart();
  RenderWindowPartActivated(renderWindowPart);
}

void ModelFitInspectorView::SetFocus()
{
}

void ModelFitInspectorView::NodeRemoved(const mitk::DataNode* node)
{
  if (node == this->m_currentSelectedNode)
  {
    QmitkSingleNodeSelectionWidget::NodeList emptylist;
    this->m_Controls.inputNodeSelector->SetCurrentSelection(emptylist);
  }
}

void ModelFitInspectorView::OnScaleFixedYChecked(bool checked)
{
  m_Controls.widgetPlot->GetPlot()->setAxisAutoScale(QwtPlot::yLeft, !checked);

  if (checked)
  {
    OnScaleToDataYClicked();
  }

  m_Controls.widgetPlot->GetPlot()->replot();
};

void ModelFitInspectorView::OnScaleFixedXChecked(bool checked)
{
  m_Controls.widgetPlot->GetPlot()->setAxisAutoScale(QwtPlot::xBottom, !checked);

  if (checked)
  {
    OnScaleToDataXClicked();
  }

  m_Controls.widgetPlot->GetPlot()->replot();
};

void ModelFitInspectorView::OnScaleToDataYClicked()
{
  auto minmax = this->m_PlotCurves.GetYMinMax();

  auto min = minmax.first - abs(minmax.first) * 0.01;
  auto max = minmax.second + abs(minmax.second) * 0.01;

  m_Controls.sbFixMin->setValue(min);
  m_Controls.sbFixMax->setValue(max);
};

void ModelFitInspectorView::OnScaleToDataXClicked()
{
  auto minmax = this->m_PlotCurves.GetXMinMax();

  auto min = minmax.first - abs(minmax.first) * 0.01;
  auto max = minmax.second + abs(minmax.second) * 0.01;

  m_Controls.sbFixMin_x->setValue(min);
  m_Controls.sbFixMax_x->setValue(max);
};

void ModelFitInspectorView::OnFixedScalingYChanged(double /*value*/)
{
  m_Controls.widgetPlot->GetPlot()->setAxisScale(QwtPlot::yLeft, m_Controls.sbFixMin->value(),
      m_Controls.sbFixMax->value());
  m_Controls.widgetPlot->GetPlot()->replot();
};

void ModelFitInspectorView::OnFixedScalingXChanged(double /*value*/)
{
  m_Controls.widgetPlot->GetPlot()->setAxisScale(QwtPlot::xBottom, m_Controls.sbFixMin_x->value(),
    m_Controls.sbFixMax_x->value());
  m_Controls.widgetPlot->GetPlot()->replot();
};

void ModelFitInspectorView::OnFullPlotClicked(bool checked)
{
  m_Controls.tabWidget->setVisible(!checked);
};

int ModelFitInspectorView::ActualizeFitSelectionWidget()
{
  mitk::NodeUIDType selectedFitUD = "";
  bool isModelFitNode = this->m_currentSelectedNode->GetData()->GetPropertyList()->GetStringProperty(
    mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), selectedFitUD);

  mitk::DataStorage::Pointer storage = this->GetDataStorage();

  mitk::modelFit::NodeUIDSetType fitUIDs = mitk::modelFit::GetFitUIDsOfNode(
    this->m_currentSelectedNode, storage);

  this->m_modelfitList.clear();
  this->m_Controls.cmbFit->clear();

  for (const auto & fitUID : fitUIDs)
  {
    mitk::modelFit::ModelFitInfo::ConstPointer info = mitk::modelFit::CreateFitInfoFromNode(fitUID,
      storage).GetPointer();

    if (info.IsNotNull())
    {
      this->m_modelfitList.insert(std::make_pair(info->uid, info));
      std::ostringstream nameStrm;
      if (info->fitName.empty())
      {
        nameStrm << info->uid;
      }
      else
      {
        nameStrm << info->fitName;
      }
      nameStrm << " (" << info->modelName << ")";
      QVariant data(info->uid.c_str());
      m_Controls.cmbFit->addItem(QString::fromStdString(nameStrm.str()), data);
    }
    else
    {
      MITK_ERROR <<
        "Was not able to extract model fit information from storage. Node properties in storage may be invalid. Failed fit UID:"
        << fitUID;
    }
  }

  int cmbIndex = 0;

  if (m_modelfitList.empty())
  {
    cmbIndex = -1;
  };

  if (isModelFitNode)
  {
    //model was selected, thus select this one in combobox
    QVariant data(selectedFitUD.c_str());
    cmbIndex = m_Controls.cmbFit->findData(data);

    if (cmbIndex == -1)
    {
      MITK_WARN <<
        "Model fit Inspector in invalid state. Selected fit seems to be not avaible in plugin selection. Failed fit UID:"
        << selectedFitUD;
    }
  };

  m_Controls.cmbFit->setCurrentIndex(cmbIndex);

  return cmbIndex;
}

void ModelFitInspectorView::OnInputChanged(const QList<mitk::DataNode::Pointer>& nodes)
{
  if (nodes.size() > 0)
  {
    if (nodes.front() != this->m_currentSelectedNode)
    {

      m_internalUpdateFlag = true;

      this->m_currentSelectedNode = nodes.front();

      mitk::NodeUIDType selectedFitUD = "";
      bool isModelFitNode = this->m_currentSelectedNode->GetData()->GetPropertyList()->GetStringProperty(
                              mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), selectedFitUD);

      if (isModelFitNode)
      {
        this->m_currentSelectedNode = this->GetParentNode(this->m_currentSelectedNode);
      }

      auto cmbIndex = ActualizeFitSelectionWidget();

      m_internalUpdateFlag = false;

      m_selectedNodeTime.Modified();

      if (cmbIndex == -1)
      {
        //only raw 4D data selected. Just update plots for current position
        m_currentFit = nullptr;
        m_currentFitTime.Modified();
        OnSliceChanged();
        m_Controls.plotDataWidget->SetXName(DEFAULT_X_AXIS);
      }
      else
      {
        //refresh fit selection (and implicitly update plots)
        OnFitSelectionChanged(cmbIndex);
      }
    }
  }
  else
  {
    if (this->m_currentSelectedNode.IsNotNull())
    {
      m_internalUpdateFlag = true;
      this->m_currentSelectedNode = nullptr;
      this->m_currentFit = nullptr;
      this->m_modelfitList.clear();
      this->m_Controls.cmbFit->clear();
      m_internalUpdateFlag = false;

      m_selectedNodeTime.Modified();
      OnFitSelectionChanged(0);
      RefreshPlotData();
      m_Controls.plotDataWidget->SetPlotData(&(this->m_PlotCurves));
      m_Controls.fitParametersWidget->setFits(QmitkFitParameterModel::FitVectorType());
      RenderPlot();
    }
  }

}

mitk::DataNode::ConstPointer
ModelFitInspectorView::GetParentNode(mitk::DataNode::ConstPointer node)
{
  if (node.IsNotNull())
  {
    mitk::DataStorage::SetOfObjects::ConstPointer parentNodeList =
      GetDataStorage()->GetSources(node);

    if (parentNodeList->size() > 0)
    {
      return parentNodeList->front().GetPointer();
    }
  }

  return mitk::DataNode::ConstPointer();
}


void ModelFitInspectorView::ValidateAndSetCurrentPosition()
{
  mitk::Point3D currentSelectedPosition = GetRenderWindowPart()->GetSelectedPosition(nullptr);
  unsigned int currentSelectedTimestep = m_renderWindowPart->GetTimeNavigationController()->GetTime()->
    GetPos();

  if (m_currentSelectedPosition != currentSelectedPosition
      || m_currentSelectedTimeStep != currentSelectedTimestep
      || m_selectedNodeTime > m_currentPositionTime)
  {
    //the current position has been changed or the selected node has been changed since the last position validation -> check position
    m_currentSelectedPosition = currentSelectedPosition;
    m_currentSelectedTimeStep = currentSelectedTimestep;
    m_currentPositionTime.Modified();
    m_validSelectedPosition = false;

    mitk::Image::Pointer inputImage = this->GetCurrentInputImage();

    if (inputImage.IsNull())
    {
      return;
    }

    mitk::BaseGeometry::Pointer geometry = inputImage->GetTimeGeometry()->GetGeometryForTimeStep(
      m_currentSelectedTimeStep);

    // check for invalid time step
    if (geometry.IsNull())
    {
      geometry = inputImage->GetTimeGeometry()->GetGeometryForTimeStep(0);
    }

    if (geometry.IsNull())
    {
      return;
    }

    m_validSelectedPosition = geometry->IsInside(m_currentSelectedPosition);

  }
}

mitk::Image::Pointer  ModelFitInspectorView::GetCurrentInputImage() const
{
  mitk::Image::Pointer result = nullptr;

  if (this->m_currentFit.IsNotNull())
  {
    result = m_currentFit->inputImage;
  }
  else if (this->m_currentSelectedNode.IsNotNull())
  {
    result = dynamic_cast<mitk::Image*>(this->m_currentSelectedNode->GetData());

    if (result.IsNotNull() && result->GetTimeSteps() <= 1)
    {
      //if the image is not dynamic, we can't use it.
      result = nullptr;
    }
  }

  return result;
};

const mitk::ModelBase::TimeGridType ModelFitInspectorView::GetCurrentTimeGrid() const
{
  if (m_currentModelProviderService && m_currentFit.IsNotNull())
  {
    return m_currentModelProviderService->GetVariableGrid(m_currentFit);
  }
  else
  { //fall back if there is no model provider we assume to use the normal time grid.
    return ExtractTimeGrid(GetCurrentInputImage());
  }
};

void ModelFitInspectorView::OnSliceChanged()
{
  ValidateAndSetCurrentPosition();

  m_Controls.widgetPlot->setEnabled(m_validSelectedPosition);

  if (m_currentSelectedNode.IsNotNull())
  {
    m_Controls.inspectionPositionWidget->SetCurrentPosition(m_currentSelectedPosition);

    if (RefreshPlotData())
    {
      RenderPlot();
      m_Controls.plotDataWidget->SetPlotData(&m_PlotCurves);
      RenderFitInfo();
    }
  }
}

void ModelFitInspectorView::OnPositionBookmarksChanged()
{
    if (RefreshPlotData())
    {
      RenderPlot();
      m_Controls.plotDataWidget->SetPlotData(&m_PlotCurves);
      RenderFitInfo();
    }
}

void ModelFitInspectorView::OnFitSelectionChanged(int index)
{
  if (!m_internalUpdateFlag)
  {
    MITK_DEBUG << "selected fit index: " << index;

    std::string uid = "";


    if (m_Controls.cmbFit->count() > index)
    {
      uid = m_Controls.cmbFit->itemData(index).toString().toStdString();
    }

    mitk::modelFit::ModelFitInfo::ConstPointer newFit = nullptr;

    ModelFitInfoListType::iterator finding = m_modelfitList.find(uid);

    if (finding != m_modelfitList.end())
    {
      newFit = finding->second;
    }

    if (m_currentFit != newFit)
    {
      m_currentModelParameterizer = nullptr;
      m_currentModelProviderService = nullptr;

      if (newFit.IsNotNull())
      {
        m_currentModelParameterizer = mitk::ModelGenerator::GenerateModelParameterizer(*newFit);
        m_currentModelProviderService = mitk::ModelGenerator::GetProviderService(newFit->functionClassID);
      }

      m_currentFit = newFit;

      m_currentFitTime.Modified();

      auto name = m_currentFit->xAxisName;
      if (!m_currentFit->xAxisUnit.empty())
      {
        name += " [" + m_currentFit->xAxisUnit + "]";
      }
      m_Controls.plotDataWidget->SetXName(name);

      OnSliceChanged();
    }
  }
}

mitk::PlotDataCurveCollection::Pointer ModelFitInspectorView::RefreshPlotDataCurveCollection(const mitk::Point3D& position,
  const mitk::Image* input, const mitk::modelFit::ModelFitInfo* fitInfo,
  const mitk::ModelBase::TimeGridType& timeGrid, mitk::ModelParameterizerBase* parameterizer)
{
  mitk::PlotDataCurveCollection::Pointer result = mitk::PlotDataCurveCollection::New();

  //sample curve
  if (input)
  {
    result->InsertElement(mitk::MODEL_FIT_PLOT_SAMPLE_NAME(), GenerateImageSamplePlotData(position, input, timeGrid));
  }

  //model signal curve
  if (fitInfo)
  {
    // Interpolate time grid (x values) so the curve looks smooth
    const mitk::ModelBase::TimeGridType interpolatedTimeGrid = mitk::GenerateSupersampledTimeGrid(timeGrid, INTERPOLATION_STEPS);
    auto hires_curve = mitk::GenerateModelSignalPlotData(position, fitInfo, interpolatedTimeGrid, parameterizer);
    result->InsertElement(mitk::MODEL_FIT_PLOT_INTERPOLATED_SIGNAL_NAME(), hires_curve);
    auto curve = mitk::GenerateModelSignalPlotData(position, fitInfo, timeGrid, parameterizer);
    result->InsertElement(mitk::MODEL_FIT_PLOT_SIGNAL_NAME(), curve);
  }

  return result;
};

bool ModelFitInspectorView::RefreshPlotData()
{
  bool changed = false;

  if (m_currentSelectedNode.IsNull())
  {
    this->m_PlotCurves = mitk::ModelFitPlotData();

    changed = m_selectedNodeTime > m_lastRefreshTime;
    m_lastRefreshTime.Modified();
  }
  else
  {
    assert(GetRenderWindowPart() != NULL);

    const mitk::Image* input = GetCurrentInputImage();
    const mitk::ModelBase::TimeGridType timeGrid = GetCurrentTimeGrid();

    if (m_currentFitTime > m_lastRefreshTime || m_currentPositionTime > m_lastRefreshTime)
    {
      if (m_validSelectedPosition)
      {
        m_PlotCurves.currentPositionPlots = RefreshPlotDataCurveCollection(m_currentSelectedPosition,input,m_currentFit, timeGrid, m_currentModelParameterizer);
      }
      else
      {
        m_PlotCurves.currentPositionPlots = mitk::PlotDataCurveCollection::New();
      }

      changed = true;
    }

    auto bookmarks = m_PositionBookmarks.Lock();
    if (bookmarks.IsNotNull())
    {
      if (m_currentFitTime > m_lastRefreshTime || bookmarks->GetMTime() > m_lastRefreshTime)
      {
        m_PlotCurves.positionalPlots.clear();

        auto endIter = bookmarks->End();
        for (auto iter = bookmarks->Begin(); iter != endIter; iter++)
        {
          auto collection = RefreshPlotDataCurveCollection(iter.Value(), input, m_currentFit, timeGrid, m_currentModelParameterizer);
          m_PlotCurves.positionalPlots.emplace(iter.Index(), std::make_pair(iter.Value(), collection));
        }

        changed = true;
      }
    }
    else
    {
      m_PlotCurves.positionalPlots.clear();
    }

    // input data curve
    if (m_currentFitTime > m_lastRefreshTime)
    {
      m_PlotCurves.staticPlots->clear();

      if (m_currentFit.IsNotNull())
      {
        m_PlotCurves.staticPlots = GenerateAdditionalModelFitPlotData(m_currentSelectedPosition, m_currentFit, timeGrid);
      }

      changed = true;
    }

    m_lastRefreshTime.Modified();
  }

  return changed;
}

void ModelFitInspectorView::RenderFitInfo()
{
    assert(m_renderWindowPart != nullptr);

    // configure fit information

    if (m_currentFit.IsNull())
    {
        m_Controls.lFitType->setText("");
        m_Controls.lFitUID->setText("");
        m_Controls.lModelName->setText("");
        m_Controls.lModelType->setText("");
    }
    else
    {
        m_Controls.lFitType->setText(QString::fromStdString(m_currentFit->fitType));
        m_Controls.lFitUID->setText(QString::fromStdString(m_currentFit->uid));
        m_Controls.lModelName->setText(QString::fromStdString(m_currentFit->modelName));
        m_Controls.lModelType->setText(QString::fromStdString(m_currentFit->modelType));
    }

    // print results
    std::stringstream infoOutput;

  m_Controls.fitParametersWidget->setVisible(false);
    m_Controls.groupSettings->setVisible(false);

    if (m_currentFit.IsNull())
    {
        infoOutput << "No fit selected. Only raw image data is plotted.";
    }
    else if (!m_validSelectedPosition)
    {
        infoOutput <<
               "Current position is outside of the input image of the selected fit.\nInspector is deactivated.";
    }
    else
    {
        m_Controls.fitParametersWidget->setVisible(true);
    m_Controls.fitParametersWidget->setFits({ m_currentFit });

    m_Controls.fitParametersWidget->setPositionBookmarks(m_PositionBookmarks.Lock());
    m_Controls.fitParametersWidget->setCurrentPosition(m_currentSelectedPosition);
    }

    // configure data table
    m_Controls.tableInputData->clearContents();

    if (m_currentFit.IsNull())
    {
        infoOutput << "No fit selected. Only raw image data is plotted.";
    }
    else
    {
        m_Controls.groupSettings->setVisible(true);
        m_Controls.tableInputData->setRowCount(m_PlotCurves.staticPlots->size());

        unsigned int rowIndex = 0;

    for (mitk::PlotDataCurveCollection::const_iterator pos = m_PlotCurves.staticPlots->begin();
            pos != m_PlotCurves.staticPlots->end(); ++pos, ++rowIndex)
        {
            QColor dataColor;

            if (pos->first == "ROI")
            {
                dataColor = QColor(0, 190, 0);
            }
            else
            {
                //Use HSV schema of QColor to calculate a different color depending on the
                //number of already existing free iso lines.
                dataColor.setHsv(((rowIndex + 1) * 85) % 360, 255, 255);
            }

            QTableWidgetItem* newItem = new QTableWidgetItem(QString::fromStdString(pos->first));
            m_Controls.tableInputData->setItem(rowIndex, 0, newItem);
            newItem = new QTableWidgetItem();
            newItem->setBackgroundColor(dataColor);
            m_Controls.tableInputData->setItem(rowIndex, 1, newItem);
        }
    }

    m_Controls.lInfo->setText(QString::fromStdString(infoOutput.str()));
}

void ModelFitInspectorView::RenderPlotCurve(const mitk::PlotDataCurveCollection* curveCollection, const QColor& sampleColor, const QColor& signalColor, const std::string& posString)
{
  auto sampleCurve = mitk::ModelFitPlotData::GetSamplePlot(curveCollection);
  if (sampleCurve)
  {
    std::string name = mitk::MODEL_FIT_PLOT_SAMPLE_NAME() + posString;
    unsigned int curveId = m_Controls.widgetPlot->InsertCurve(name.c_str());
    m_Controls.widgetPlot->SetCurveData(curveId, sampleCurve->GetValues());
    m_Controls.widgetPlot->SetCurvePen(curveId, QPen(Qt::NoPen));

    // QwtSymbol needs to passed as a real pointer from MITK v2013.09.0 on
    // (QwtPlotCurve deletes it on destruction and assignment).
    QwtSymbol* dataSymbol = new QwtSymbol(QwtSymbol::Diamond, sampleColor, sampleColor, QSize(8, 8));
    m_Controls.widgetPlot->SetCurveSymbol(curveId, dataSymbol);

    // Again, there is no way to set a curve's legend attributes via QmitkPlotWidget so this
    // gets unnecessarily complicated.
    QwtPlotCurve* measurementCurve = dynamic_cast<QwtPlotCurve*>(m_Controls.widgetPlot->
      GetPlot()->itemList(QwtPlotItem::Rtti_PlotCurve).back());
    measurementCurve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
    measurementCurve->setLegendIconSize(QSize(8, 8));
  }

  //draw model curve
  auto signalCurve = mitk::ModelFitPlotData::GetInterpolatedSignalPlot(curveCollection);
  if (signalCurve)
  {
    std::string name = mitk::MODEL_FIT_PLOT_SIGNAL_NAME() + posString;
    QPen pen;
    pen.setColor(signalColor);
    pen.setWidth(2);
    unsigned int curveId = m_Controls.widgetPlot->InsertCurve(name.c_str());
    m_Controls.widgetPlot->SetCurveData(curveId, signalCurve->GetValues());
    m_Controls.widgetPlot->SetCurvePen(curveId, pen);

    // Manually set the legend attribute to use the symbol as the legend icon and alter its
    // size. Otherwise it would revert to default which is drawing a square which is the color
    // of the curve's pen, so in this case none which defaults to black.
    // Unfortunately, QmitkPlotWidget offers no way to set the legend attribute and icon size so
    // this looks a bit hacky.
    QwtPlotCurve* fitCurve = dynamic_cast<QwtPlotCurve*>(m_Controls.widgetPlot->GetPlot()->
      itemList(QwtPlotItem::Rtti_PlotCurve).back());
    fitCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
  }

}

void ModelFitInspectorView::RenderPlot()
{
  m_Controls.widgetPlot->Clear();

  std::string xAxis = DEFAULT_X_AXIS;
  std::string yAxis = "Intensity";
  std::string plotTitle = "Raw data plot: no data";

  if (m_currentSelectedNode.IsNotNull())
  {
    plotTitle = "Raw data plot: " + m_currentSelectedNode->GetName();
  }

  if (m_currentFit.IsNotNull())
  {
    plotTitle = m_currentFit->modelName.c_str();
    xAxis = m_currentFit->xAxisName;

    if (!m_currentFit->xAxisUnit.empty())
    {
      xAxis += " [" + m_currentFit->xAxisUnit + "]";
    }

    yAxis = m_currentFit->yAxisName;

    if (!m_currentFit->yAxisUnit.empty())
    {
      yAxis += " [" + m_currentFit->yAxisUnit + "]";
    }
  }

  m_Controls.widgetPlot->SetAxisTitle(QwtPlot::xBottom, xAxis.c_str());
  m_Controls.widgetPlot->SetAxisTitle(QwtPlot::yLeft, yAxis.c_str());
  m_Controls.widgetPlot->SetPlotTitle(plotTitle.c_str());

  // Draw static curves
  unsigned int colorIndex = 0;

  for (mitk::PlotDataCurveCollection::const_iterator pos = m_PlotCurves.staticPlots->begin();
       pos != m_PlotCurves.staticPlots->end(); ++pos)
  {
    QColor dataColor;

    unsigned int curveId = m_Controls.widgetPlot->InsertCurve(pos->first.c_str());
    m_Controls.widgetPlot->SetCurveData(curveId, pos->second->GetValues());

    if (pos->first == "ROI")
    {
      dataColor = QColor(0, 190, 0);
      QPen pen;
      pen.setColor(dataColor);
      pen.setStyle(Qt::SolidLine);
      m_Controls.widgetPlot->SetCurvePen(curveId, pen);
    }
    else
    {
      //Use HSV schema of QColor to calculate a different color depending on the
      //number of already existing curves.
      dataColor.setHsv((++colorIndex * 85) % 360, 255, 150);
      m_Controls.widgetPlot->SetCurvePen(curveId, QPen(Qt::NoPen));
    }

    // QwtSymbol needs to passed as a real pointer from MITK v2013.09.0 on
    // (QwtPlotCurve deletes it on destruction and assignment).
    QwtSymbol* dataSymbol = new QwtSymbol(QwtSymbol::Triangle, dataColor, dataColor,
                                          QSize(8, 8));
    m_Controls.widgetPlot->SetCurveSymbol(curveId, dataSymbol);

    // Again, there is no way to set a curve's legend attributes via QmitkPlotWidget so this
    // gets unnecessarily complicated.
    QwtPlotCurve* measurementCurve = dynamic_cast<QwtPlotCurve*>(m_Controls.widgetPlot->
                                     GetPlot()->itemList(QwtPlotItem::Rtti_PlotCurve).back());
    measurementCurve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
    measurementCurve->setLegendIconSize(QSize(8, 8));
  }

  // Draw positional curves
  for (const auto& posIter : this->m_PlotCurves.positionalPlots)
  {
    QColor dataColor;
    dataColor.setHsv((++colorIndex * 85) % 360, 255, 150);

    this->RenderPlotCurve(posIter.second.second, dataColor, dataColor, " @ "+mitk::ModelFitPlotData::GetPositionalCollectionName(posIter));
  }

  // Draw current pos curve
  this->RenderPlotCurve(m_PlotCurves.currentPositionPlots, QColor(Qt::red), QColor(Qt::black), "");

  QwtLegend* legend = new QwtLegend();
  legend->setFrameShape(QFrame::Box);
  legend->setFrameShadow(QFrame::Sunken);
  legend->setLineWidth(1);
  m_Controls.widgetPlot->SetLegend(legend, QwtPlot::BottomLegend);

  m_Controls.widgetPlot->Replot();
}

void ModelFitInspectorView::EnsureBookmarkPointSet()
{
  if (m_PositionBookmarks.IsExpired() || m_PositionBookmarksNode.IsExpired())
  {
    const char* nodeName = "org.mitk.gui.qt.fit.inspector.positions";
    mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode(nodeName);

    if (!node)
    {
      node = mitk::DataNode::New();
      node->SetName(nodeName);
      node->SetBoolProperty("helper object", true);
      this->GetDataStorage()->Add(node);
    }
    m_PositionBookmarksNode = node;

    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());
    if (pointSet.IsNull())
    {
      pointSet = mitk::PointSet::New();
      node->SetData(pointSet);
    }

    m_PositionBookmarks = pointSet;
  }
}
