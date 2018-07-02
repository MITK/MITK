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
#include <QMessageBox>
#include <QFileDialog>
#include <qwt_plot_marker.h>

#include "mitkFormulaParser.h"
#include "mitkScalarListLookupTableProperty.h"
#include "mitkModelFitConstants.h"
#include "mitkExtractTimeGrid.h"
#include "mitkModelGenerator.h"
#include "mitkModelFitException.h"
#include "mitkModelFitParameterValueExtraction.h"

#include "ModelFitInspectorView.h"

#include <QClipboard>
const std::string ModelFitInspectorView::VIEW_ID = "org.mitk.gui.gt.fit.inspector";
const unsigned int ModelFitInspectorView::INTERPOLATION_STEPS = 100;

ModelFitInspectorView::ObserverInfo::ObserverInfo(mitk::SliceNavigationController* controller,
  int observerTag, const std::string& renderWindowName, mitk::IRenderWindowPart* part) : controller(controller), observerTag(observerTag),
  renderWindowName(renderWindowName), renderWindowPart(part)
{
}

ModelFitInspectorView::ModelFitInspectorView() :
  m_renderWindowPart(nullptr),
  m_PendingSliceChangedEvent(false),
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
  this->RemoveAllObservers();
}


bool ModelFitInspectorView::InitObservers()
{

  bool result = true;

  typedef QHash<QString, QmitkRenderWindow*> WindowMapType;
  WindowMapType windowMap = m_renderWindowPart->GetQmitkRenderWindows();

  auto i = windowMap.begin();

  while (i != windowMap.end())
  {
    mitk::SliceNavigationController* sliceNavController =
      i.value()->GetSliceNavigationController();

    if (sliceNavController)
    {
      itk::ReceptorMemberCommand<ModelFitInspectorView>::Pointer cmdSliceEvent =
        itk::ReceptorMemberCommand<ModelFitInspectorView>::New();
      cmdSliceEvent->SetCallbackFunction(this, &ModelFitInspectorView::OnSliceChanged);
      int tag = sliceNavController->AddObserver(
                  mitk::SliceNavigationController::GeometrySliceEvent(nullptr, 0),
                  cmdSliceEvent);

      m_ObserverMap.insert(std::make_pair(sliceNavController, ObserverInfo(sliceNavController, tag,
        i.key().toStdString(), m_renderWindowPart)));

      itk::ReceptorMemberCommand<ModelFitInspectorView>::Pointer cmdTimeEvent =
        itk::ReceptorMemberCommand<ModelFitInspectorView>::New();
      cmdTimeEvent->SetCallbackFunction(this, &ModelFitInspectorView::OnSliceChanged);
      tag = sliceNavController->AddObserver(
              mitk::SliceNavigationController::GeometryTimeEvent(nullptr, 0),
              cmdTimeEvent);

      m_ObserverMap.insert(std::make_pair(sliceNavController, ObserverInfo(sliceNavController, tag,
        i.key().toStdString(), m_renderWindowPart)));

      itk::MemberCommand<ModelFitInspectorView>::Pointer cmdDelEvent =
        itk::MemberCommand<ModelFitInspectorView>::New();
      cmdDelEvent->SetCallbackFunction(this,
                                       &ModelFitInspectorView::OnSliceNavigationControllerDeleted);
      tag = sliceNavController->AddObserver(
              itk::DeleteEvent(), cmdDelEvent);

      m_ObserverMap.insert(std::make_pair(sliceNavController, ObserverInfo(sliceNavController, tag,
        i.key().toStdString(), m_renderWindowPart)));
    }

    ++i;

    result = result && sliceNavController;
  }

  return result;
}

void ModelFitInspectorView::RemoveObservers(const mitk::SliceNavigationController*
    deletedSlicer)
{

  std::pair < ObserverMapType::const_iterator, ObserverMapType::const_iterator> obsRange =
    m_ObserverMap.equal_range(deletedSlicer);

  for (ObserverMapType::const_iterator pos = obsRange.first; pos != obsRange.second; ++pos)
  {
    pos->second.controller->RemoveObserver(pos->second.observerTag);
  }

  m_ObserverMap.erase(deletedSlicer);
}

void ModelFitInspectorView::RemoveAllObservers(mitk::IRenderWindowPart* deletedPart)
{
  for (ObserverMapType::const_iterator pos = m_ObserverMap.begin(); pos != m_ObserverMap.end(); )
  {
    ObserverMapType::const_iterator delPos = pos++;

    if (deletedPart == nullptr || deletedPart == delPos->second.renderWindowPart)
    {
      delPos->second.controller->RemoveObserver(delPos->second.observerTag);
      m_ObserverMap.erase(delPos);
    }
  }
}

void ModelFitInspectorView::OnSliceNavigationControllerDeleted(const itk::Object* sender,
    const itk::EventObject& /*e*/)
{
  const mitk::SliceNavigationController* sendingSlicer =
    dynamic_cast<const mitk::SliceNavigationController*>(sender);

  this->RemoveObservers(sendingSlicer);
}

void ModelFitInspectorView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_renderWindowPart != renderWindowPart)
  {
    m_renderWindowPart = renderWindowPart;

    if (!InitObservers())
    {
      QMessageBox::information(nullptr, "Error", "Unable to set up the event observers. The " \
                               "plot will not be triggered on changing the crosshair, " \
                               "position or time step.");
    }
  }
}

void ModelFitInspectorView::RenderWindowPartDeactivated(
  mitk::IRenderWindowPart* renderWindowPart)
{
  m_renderWindowPart = nullptr;
  this->RemoveAllObservers(renderWindowPart);
}

void ModelFitInspectorView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
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


  connect(m_Controls.btnExport, SIGNAL(clicked()), this, SLOT(OnExportClicked()));

  // Add SIGNAL and SLOT for "Copy to clipboard" functionality

  connect(m_Controls.btnCopyResultsToClipboard, SIGNAL(clicked()), this, SLOT(OnClipboardResultsButtonClicked()));


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

void CheckYMinMaxFromXYData(const QmitkPlotWidget::XYDataVector& data, double& min, double& max)
{
  for (const auto & pos : data)
  {
    if (max < pos.second)
    {
      max = pos.second;
    }

    if (min > pos.second)
    {
      min = pos.second;
    }
  }
}

void CheckXMinMaxFromXYData(const QmitkPlotWidget::XYDataVector& data, double& min, double& max)
{
  for (const auto & pos : data)
  {
    if (max < pos.first)
    {
      max = pos.first;
    }

    if (min > pos.first)
    {
      min = pos.first;
    }
  }
}

void ModelFitInspectorView::OnScaleToDataYClicked()
{
  double max = itk::NumericTraits<double>::NonpositiveMin();
  double min = itk::NumericTraits<double>::max();

  CheckYMinMaxFromXYData(this->m_ImagePlotCurve, min, max);
  CheckYMinMaxFromXYData(this->m_ModelPlotCurve, min, max);

  for (CurveMapType::const_iterator pos = this->m_InputDataPlotCurves.begin();
       pos != this->m_InputDataPlotCurves.end(); ++pos)
  {
    CheckYMinMaxFromXYData(pos->second, min, max);
  }

  min -= abs(min) * 0.01;
  max += abs(max) * 0.01;

  m_Controls.sbFixMax->setValue(max);
  m_Controls.sbFixMin->setValue(min);

};

void ModelFitInspectorView::OnScaleToDataXClicked()
{
  double max = itk::NumericTraits<double>::NonpositiveMin();
  double min = itk::NumericTraits<double>::max();

  CheckXMinMaxFromXYData(this->m_ImagePlotCurve, min, max);
  CheckXMinMaxFromXYData(this->m_ModelPlotCurve, min, max);

  for (CurveMapType::const_iterator pos = this->m_InputDataPlotCurves.begin();
    pos != this->m_InputDataPlotCurves.end(); ++pos)
  {
    CheckXMinMaxFromXYData(pos->second, min, max);
  }

  min -= abs(min) * 0.01;
  max += abs(max) * 0.01;

  m_Controls.sbFixMax_x->setValue(max);
  m_Controls.sbFixMin_x->setValue(min);

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

void ModelFitInspectorView::OnExportClicked()
{
  QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save File"));

  if (fileName.isEmpty())
  {
    QMessageBox::critical(nullptr, tr("No file selected!"),
                          tr("Cannot export pixel dump. Please selected a file."));
  }
  else
  {
    std::ofstream file;

    std::ios_base::openmode iOpenFlag = std::ios_base::out | std::ios_base::trunc;
    file.open(fileName.toStdString().c_str(), iOpenFlag);

    if (!file.is_open())
    {
      QMessageBox::critical(nullptr, tr("Cannot create/open selected file!"),
                            tr("Cannot open or create the selected file. Export will be aborted. Selected file name: ") +
                            fileName);
      return;
    }

    for (int i = 0; i < m_Controls.tableData->columnCount(); ++i)
    {
      file << m_Controls.tableData->horizontalHeaderItem(i)->text().toStdString();

      if (i < m_Controls.tableData->columnCount() - 1)
      {
        file << ",";
      }
    }

    file << std::endl;

    for (int row = 0; row < m_Controls.tableData->rowCount(); ++row)
    {
      for (int i = 0; i < m_Controls.tableData->columnCount(); ++i)
      {
        file << m_Controls.tableData->item(row, i)->text().toStdString();

        if (i < m_Controls.tableData->columnCount() - 1)
        {
          file << ",";
        }
      }

      file << std::endl;
    }

    file.close();
  }
}

void ModelFitInspectorView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
    const QList<mitk::DataNode::Pointer>& nodes)
{
  if (nodes.size() > 0)
  {
    if (nodes.front() != this->m_currentSelectedNode)
    {

      m_internalUpdateFlag = true;

      this->m_currentSelectedNode = nodes.front();

      mitk::DataStorage::Pointer storage = this->GetDataStorage();

      mitk::NodeUIDType selectedFitUD = "";
      bool isModelFitNode = this->m_currentSelectedNode->GetData()->GetPropertyList()->GetStringProperty(
                              mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), selectedFitUD);

      if (isModelFitNode)
      {
        this->m_currentSelectedNode = this->GetParentNode(this->m_currentSelectedNode);
      }

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

          QVariant data(info->uid.c_str());
          m_Controls.cmbFit->addItem(QString::fromStdString(info->modelName), data);
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

      m_internalUpdateFlag = false;

      m_selectedNodeTime.Modified();

      if (cmbIndex == -1)
      {
        //only raw 4D data selected. Just update plots for current position
        m_currentFit = nullptr;
        m_currentFitTime.Modified();
        OnSliceChangedDelayed();
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

void ModelFitInspectorView::OnSliceChanged(const itk::EventObject&)
{
  // Taken from QmitkStdMultiWidget::HandleCrosshairPositionEvent().
  // Since there are always 3 events arriving (one for each render window) every time the slice
  // or time changes, the slot OnSliceChangedDelayed is triggered - and only if it hasn't been
  // triggered yet - so it is only executed once for every slice/time change.
  if (!m_PendingSliceChangedEvent)
  {
    m_PendingSliceChangedEvent = true;
    QTimer::singleShot(0, this, SLOT(OnSliceChangedDelayed()));
  }
}

void ModelFitInspectorView::OnSliceChangedDelayed()
{
  m_PendingSliceChangedEvent = false;

  ValidateAndSetCurrentPosition();

  m_Controls.widgetPlot->setEnabled(m_validSelectedPosition);

  if (m_currentSelectedNode.IsNotNull())
  {
    if (RefreshPlotData())
    {
      RenderPlot();
      RenderFitInfo();
    }
  }
}

mitk::ModelBase::TimeGridType
ModelFitInspectorView::GenerateInterpolatedTimeGrid(const mitk::ModelBase::TimeGridType& grid)
const
{
  unsigned int origGridSize = grid.size();

  mitk::ModelBase::TimeGridType interpolatedTimeGrid(((origGridSize - 1) * INTERPOLATION_STEPS) + 1);

  for (unsigned int t = 0; t < origGridSize - 1; ++t)
  {
    double delta = (grid[t + 1] - grid[t]) / INTERPOLATION_STEPS;

    for (unsigned int i = 0; i < INTERPOLATION_STEPS; ++i)
    {
      interpolatedTimeGrid[(t * INTERPOLATION_STEPS) + i] = grid[t] + i * delta;
    }
  }

  interpolatedTimeGrid[interpolatedTimeGrid.size() - 1] = grid[grid.size() - 1];

  return interpolatedTimeGrid;
};


QmitkPlotWidget::XYDataVector
ModelFitInspectorView::CalcCurveFromModel(const mitk::Point3D& position)
{
  assert(m_currentModelParameterizer.IsNotNull());
  assert(m_currentFit.IsNotNull());

  mitk::Image::Pointer inputImage = this->GetCurrentInputImage();
  assert(inputImage.IsNotNull());

  // Calculate index
  ::itk::Index<3> index;
  mitk::BaseGeometry::Pointer geometry = inputImage->GetTimeGeometry()->GetGeometryForTimeStep(0);

  geometry->WorldToIndex(position, index);

  //model generation
  mitk::ModelBase::Pointer model = m_currentModelParameterizer->GenerateParameterizedModel(index);

  mitk::ParameterValueMapType parameterMap = mitk::ExtractParameterValueMapFromModelFit(m_currentFit, position);

  mitk::ModelBase::ParametersType paramArray = mitk::ConvertParameterMapToParameterVector(parameterMap, model);

  mitk::ModelBase::ModelResultType curveDataY = model->GetSignal(paramArray);
  QmitkPlotWidget::XYDataVector result;
  mitk::ModelBase::TimeGridType timeGrid = model->GetTimeGrid();

  for (unsigned int t = 0; t < timeGrid.size(); ++t)
  {
    double x = timeGrid[t];
    double y = curveDataY[t];
    result.push_back(std::make_pair(x, y));
  }

  return result;
}

QmitkPlotWidget::XYDataVector
ModelFitInspectorView::CalcCurveFromFunction(const mitk::Point3D& position,
    const mitk::ModelBase::TimeGridType& timeGrid)
{
  assert(m_currentFit.IsNotNull());
  assert(m_renderWindowPart != nullptr);

  mitk::Image::Pointer inputImage = this->GetCurrentInputImage();
  assert(inputImage.IsNotNull());

  QmitkPlotWidget::XYDataVector result;

  // Calculate index
  ::itk::Index<3> index;;
  mitk::BaseGeometry::Pointer geometry = inputImage->GetTimeGeometry()->GetGeometryForTimeStep(0);

  geometry->WorldToIndex(position, index);

  mitk::ParameterValueMapType parameterMap = mitk::ExtractParameterValueMapFromModelFit(m_currentFit, position);

  mitk::FormulaParser parser(&parameterMap);
  unsigned int timestep = m_renderWindowPart->GetTimeNavigationController()->GetTime()->
                          GetPos();

  for (unsigned int t = 0; t < timeGrid.size(); ++t)
  {
    // Set up static parameters
    foreach (const mitk::modelFit::StaticParameterMap::StaticParameterType& var,
             m_currentFit->staticParamMap)
    {
      const std::string& name = var.first;
      const mitk::modelFit::StaticParameterMap::ValueType& list = var.second;

      if (list.size() == 1)
      {
        parameterMap[name] = list.front();
      }
      else
      {
        parameterMap[name] = list.at(timestep);
      }
    }

    // Calculate curve data
    double x = timeGrid[t];
    parameterMap[m_currentFit->x] = x;

    double y = parser.parse(m_currentFit->function);
    result.push_back(std::make_pair(x, y));
  }

  return result;
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

      OnSliceChangedDelayed();
    }
  }
}

bool ModelFitInspectorView::RefreshPlotData()
{
  bool changed = false;

  if (m_currentSelectedNode.IsNull())
  {
    this->m_ImagePlotCurve.clear();
    this->m_ModelPlotCurve.clear();
    this->m_InputDataPlotCurves.clear();

    changed = m_selectedNodeTime > m_lastRefreshTime;
    m_lastRefreshTime.Modified();
  }
  else
  {
    assert(GetRenderWindowPart() != NULL);

    const mitk::Image* input = GetCurrentInputImage();
    const mitk::ModelBase::TimeGridType timeGrid = GetCurrentTimeGrid();

    //image data curve
    if (m_selectedNodeTime > m_lastRefreshTime || m_currentFitTime > m_lastRefreshTime
        || m_currentPositionTime > m_lastRefreshTime)
    {
      m_ImagePlotCurve.clear();

      if (input && m_validSelectedPosition)
      {
        QmitkPlotWidget::XYDataVector pointData;

        for (unsigned int t = 0; t < timeGrid.size(); ++t)
        {
          double x = timeGrid[t];
          double y = ReadVoxel(input, m_currentSelectedPosition, t);
          pointData.push_back(std::make_pair(x, y));
        }

        m_ImagePlotCurve = pointData;
      }

      changed = true;
    }

    //model curve
    if (m_currentFitTime > m_lastRefreshTime || m_currentPositionTime > m_lastRefreshTime)
    {
      m_ModelPlotCurve.clear();

      if (m_currentFit.IsNotNull() && m_validSelectedPosition)
      {
        QmitkPlotWidget::XYDataVector curveData;

        // Interpolate time grid (x values) so the curve looks smooth
        const mitk::ModelBase::TimeGridType interpolatedTimeGrid = GenerateInterpolatedTimeGrid(timeGrid);

        if (m_currentModelParameterizer.IsNotNull())
        {
          // Use model instead of formula parser
          m_currentModelParameterizer->SetDefaultTimeGrid(interpolatedTimeGrid);
          curveData = CalcCurveFromModel(m_currentSelectedPosition);
        }
        else
        {
          // Use formula parser to parse function string
          try
          {
            curveData = CalcCurveFromFunction(m_currentSelectedPosition, interpolatedTimeGrid);
          }
          catch (const mitk::FormulaParserException& e)
          {
            MITK_ERROR << "Error while parsing modelfit function: " << e.what();
          }
        }

        m_ModelPlotCurve = curveData;
      }

      changed = true;
    }

    // input data curve
    if (m_currentFitTime > m_lastRefreshTime)
    {
      m_InputDataPlotCurves.clear();

      if (m_currentFit.IsNotNull())
      {
        for (mitk::ScalarListLookupTable::LookupTableType::const_iterator pos =
               m_currentFit->inputData.GetLookupTable().begin();
             pos != m_currentFit->inputData.GetLookupTable().end(); ++pos)
        {
          if (pos->second.size() != timeGrid.size())
          {
            MITK_ERROR <<
                       "Error while refreshing input data for visualization. Size of data and input image time grid differ. Invalid data name: "
                       << pos->first;
          }
          else
          {
            QmitkPlotWidget::XYDataVector pointData;

            for (unsigned int t = 0; t < timeGrid.size(); ++t)
            {
              double x = timeGrid[t];
              double y = pos->second[t];
              pointData.push_back(std::make_pair(x, y));
            }

            m_InputDataPlotCurves.insert(std::make_pair(pos->first, pointData));
          }
        }
      }

      changed = true;
    }

    if (m_selectedNodeTime > m_lastRefreshTime || m_currentFitTime > m_lastRefreshTime
        || m_currentPositionTime > m_lastRefreshTime)
    {
      InitDataTable();
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

	m_Controls.grpParams->setVisible(false);
	m_Controls.groupSettings->setVisible(false);
	m_Controls.tableResults->clearContents();

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
		m_Controls.grpParams->setVisible(true);
		//  Set sorting to false. Wait with sorting until all parameters are set in the table.
		
		m_Controls.tableResults->setSortingEnabled(false);
		m_Controls.tableResults->setRowCount(m_currentFit->GetParameters().size() +
			m_currentFit->staticParamMap.Size());

		unsigned int timestep = m_renderWindowPart->GetTimeNavigationController()->GetTime()->GetPos();
		unsigned int rowIndex = 0;

    assert(GetRenderWindowPart() != NULL);

		for (mitk::modelFit::ModelFitInfo::ConstIterType paramIter = m_currentFit->GetParameters().begin();
			paramIter != m_currentFit->GetParameters().end();
			++paramIter, ++rowIndex)
		{
			mitk::modelFit::Parameter::ConstPointer p =
				static_cast<mitk::modelFit::Parameter::ConstPointer>(*paramIter);
			double value = ReadVoxel(p->image, m_currentSelectedPosition, timestep);

			std::string paramType = mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_PARAMETER();

			if (p->type == mitk::modelFit::Parameter::DerivedType)
			{
				paramType = mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_DERIVED_PARAMETER();
			}
			else if (p->type == mitk::modelFit::Parameter::CriterionType)
			{
				paramType = mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_CRITERION();
			}
			else if (p->type == mitk::modelFit::Parameter::EvaluationType)
			{
				paramType = mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_EVALUATION_PARAMETER();
			}

			QTableWidgetItem* newItem = new QTableWidgetItem(QString::fromStdString(p->name));
			m_Controls.tableResults->setItem(rowIndex, 0, newItem);
			newItem = new QTableWidgetItem(QString::number(value));
			m_Controls.tableResults->setItem(rowIndex, 1, newItem);
			newItem = new QTableWidgetItem(QString::fromStdString(paramType));
			m_Controls.tableResults->setItem(rowIndex, 2, newItem);
		}

		foreach(const mitk::modelFit::StaticParameterMap::StaticParameterType& var,
			m_currentFit->staticParamMap)
		{
			const std::string& name = var.first;
			const mitk::modelFit::StaticParameterMap::ValueType& list = var.second;

			QTableWidgetItem* newItem = new QTableWidgetItem(QString::fromStdString(name));
			m_Controls.tableResults->setItem(rowIndex, 0, newItem);
			newItem = new QTableWidgetItem(QString::number(list.front()));
			m_Controls.tableResults->setItem(rowIndex, 1, newItem);
			newItem = new QTableWidgetItem(tr("static"));
			m_Controls.tableResults->setItem(rowIndex, 2, newItem);
			++rowIndex;
		}
		//  Here the sorting function is enabled and the table entries are sorted alphabetically in descending order.
		// This way the parameters are always displayed in the same order.
		
		m_Controls.tableResults->setSortingEnabled(true);
		m_Controls.tableResults->sortItems(0, Qt::DescendingOrder);


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
		m_Controls.tableInputData->setRowCount(m_InputDataPlotCurves.size());

		unsigned int rowIndex = 0;

		for (CurveMapType::const_iterator pos = m_InputDataPlotCurves.begin();
			pos != m_InputDataPlotCurves.end(); ++pos, ++rowIndex)
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

void ModelFitInspectorView::RenderPlot()
{
  m_Controls.widgetPlot->Clear();

  std::string xAxis = "Time [s]";
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

  // Draw input data points
  unsigned int colorIndex = 0;

  for (CurveMapType::const_iterator pos = m_InputDataPlotCurves.begin();
       pos != m_InputDataPlotCurves.end(); ++pos)
  {
    QColor dataColor;

    unsigned int curveId = m_Controls.widgetPlot->InsertCurve(pos->first.c_str());
    m_Controls.widgetPlot->SetCurveData(curveId, pos->second);

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
      //number of already existing free iso lines.
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

  // Draw image points
  if (!m_ImagePlotCurve.empty())
  {
    unsigned int curveId = m_Controls.widgetPlot->InsertCurve("measurement");
    m_Controls.widgetPlot->SetCurveData(curveId, m_ImagePlotCurve);
    m_Controls.widgetPlot->SetCurvePen(curveId, QPen(Qt::NoPen));

    // QwtSymbol needs to passed as a real pointer from MITK v2013.09.0 on
    // (QwtPlotCurve deletes it on destruction and assignment).
    QwtSymbol* redSymbol = new QwtSymbol(QwtSymbol::Diamond, QColor(Qt::red), QColor(Qt::red),
                                         QSize(8, 8));
    m_Controls.widgetPlot->SetCurveSymbol(curveId, redSymbol);

    // Again, there is no way to set a curve's legend attributes via QmitkPlotWidget so this
    // gets unnecessarily complicated.
    QwtPlotCurve* measurementCurve = dynamic_cast<QwtPlotCurve*>(m_Controls.widgetPlot->
                                     GetPlot()->itemList(QwtPlotItem::Rtti_PlotCurve).back());
    measurementCurve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
    measurementCurve->setLegendIconSize(QSize(8, 8));

    // Highlight the current time step
    unsigned int timestep = m_renderWindowPart->GetTimeNavigationController()->GetTime()->
                            GetPos();

    if (timestep < m_ImagePlotCurve.size())
    {
      QwtSymbol* blueSymbol = new QwtSymbol(QwtSymbol::Diamond, QColor(Qt::blue),
                                            QColor(Qt::blue), QSize(8, 8));
      QwtPlotMarker* marker = new QwtPlotMarker();
      marker->setSymbol(blueSymbol);
      marker->setValue(m_ImagePlotCurve[timestep].first, m_ImagePlotCurve[timestep].second);
      marker->attach(m_Controls.widgetPlot->GetPlot());
    }

    for (QmitkPlotWidget::XYDataVector::size_type i = 0; i < m_ImagePlotCurve.size(); ++i)
    {
      m_Controls.tableData->item(i, 1)->setText(QString::number(m_ImagePlotCurve[i].second));
    }
  }

  //draw model curve
  if (!m_ModelPlotCurve.empty())
  {
    QPen pen;
    pen.setColor(QColor(Qt::black));
    pen.setWidth(2);
    unsigned int curveId = m_Controls.widgetPlot->InsertCurve("fit");
    m_Controls.widgetPlot->SetCurveData(curveId, m_ModelPlotCurve);
    m_Controls.widgetPlot->SetCurvePen(curveId, pen);

    // Manually set the legend attribute to use the symbol as the legend icon and alter its
    // size. Otherwise it would revert to default which is drawing a square which is the color
    // of the curve's pen, so in this case none which defaults to black.
    // Unfortunately, QmitkPlotWidget offers no way to set the legend attribute and icon size so
    // this looks a bit hacky.
    QwtPlotCurve* fitCurve = dynamic_cast<QwtPlotCurve*>(m_Controls.widgetPlot->GetPlot()->
                             itemList(QwtPlotItem::Rtti_PlotCurve).back());
    fitCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);

    for (QmitkPlotWidget::XYDataVector::size_type i = 0; i < m_ImagePlotCurve.size(); ++i)
    {
      m_Controls.tableData->item(i, 2)->setText(QString::number(m_ModelPlotCurve[i *
          INTERPOLATION_STEPS].second));
    }
  }

  QwtLegend* legend = new QwtLegend();
  legend->setFrameShape(QFrame::Box);
  legend->setFrameShadow(QFrame::Sunken);
  legend->setLineWidth(1);
  m_Controls.widgetPlot->SetLegend(legend, QwtPlot::BottomLegend);

  m_Controls.widgetPlot->Replot();
}

void ModelFitInspectorView::InitDataTable()
{
  QStringList headers;
  headers.push_back(QString("Time"));
  headers.push_back(QString("Input image"));

  if (!this->m_ModelPlotCurve.empty())
  {
    headers.push_back(QString("Model"));
  }

  for (CurveMapType::const_iterator pos = this->m_InputDataPlotCurves.begin();
       pos != this->m_InputDataPlotCurves.end(); ++pos)
  {
    headers.push_back(QString::fromStdString(pos->first));
  }

  m_Controls.tableData->setRowCount(m_ImagePlotCurve.size());
  m_Controls.tableData->setColumnCount(headers.size());

  m_Controls.tableData->setHorizontalHeaderLabels(headers);
  m_Controls.tableData->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

  for (QmitkPlotWidget::XYDataVector::size_type i = 0; i < m_ImagePlotCurve.size(); ++i)
  {
    int column = 0;
    QTableWidgetItem* newItem = new QTableWidgetItem(QString::number(m_ImagePlotCurve[i].first));
    m_Controls.tableData->setItem(i, column++, newItem);
    newItem = new QTableWidgetItem(QString::number(m_ImagePlotCurve[i].second));
    m_Controls.tableData->setItem(i, column++, newItem);

    if (!m_ModelPlotCurve.empty())
    {
      newItem = new QTableWidgetItem(QString::number(m_ModelPlotCurve[i * INTERPOLATION_STEPS].second));
      m_Controls.tableData->setItem(i, column++, newItem);
    }

    for (CurveMapType::const_iterator pos = this->m_InputDataPlotCurves.begin();
         pos != this->m_InputDataPlotCurves.end(); ++pos, ++column)
    {
      newItem = new QTableWidgetItem(QString::number(pos->second[i].second));
      m_Controls.tableData->setItem(i, column, newItem);
    }
  }
}


void ModelFitInspectorView::OnClipboardResultsButtonClicked()
{
	QLocale tempLocal;
	QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
	std::stringstream infoOutput;
	QString clipboard;
	QVector< QVector<QString> > resultsTable;
	if (m_currentFit.IsNotNull())
	{
		int rowNumber = m_currentFit->GetParameters().size() + m_currentFit->staticParamMap.Size();

		// set headline as in results table: "Name", "Value", "Type"
		QStringList headlineText;
		QVector <QString> headline;
		// Create Headline
		headlineText << "Name" << "Value" << "Type";

		for (int i = 0; i < headlineText.size(); i++)
		{
			headline.append(headlineText.at(i));
		}
		resultsTable.append(headline);

		// fill table with values from the displayed results table
		for (int i = 0; i < rowNumber; i++)
		{
			QVector <QString> rowValues;
			for (int j = 0; j < 3; j++)
			{
				rowValues.append((m_Controls.tableResults->item(i, j)->text()));

			}
			resultsTable.append(rowValues);
			
		}

		// Create output string for clipboard
		for (int i = 0; i < resultsTable.size(); i++)
		{
			for (int t = 0; t < resultsTable.at(i).size(); t++)
			{
				clipboard.append(resultsTable.at(i).at(t));
				clipboard.append("\t");
			}
			clipboard.append("\n");
		}
		QApplication::clipboard()->setText(clipboard, QClipboard::Clipboard);
	}
	else
	{
		infoOutput << "Results table is empty.";
	}
	QLocale::setDefault(tempLocal);
}
