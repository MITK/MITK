/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRemeshingView.h"

#include <ui_QmitkRemeshingViewControls.h>

#include <QmitkIconTheme.h>

#include <mitkCoreServices.h>
#include <mitkException.h>
#include <mitkIProgressService.h>
#include <mitkMemoryUtilities.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProgressTask.h>
#include <mitkRemeshing.h>
#include <mitkRenderingManager.h>
#include <mitkScopedProgressTask.h>
#include <mitkVtkRepresentationProperty.h>

#include <itkMacro.h>

#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>

#include <QColor>
#include <QLocale>
#include <QMessageBox>
#include <QtConcurrentRun>

#include <mutex>
#include <new>
#include <set>
#include <string>
#include <vector>

const std::string QmitkRemeshingView::VIEW_ID = "org.mitk.views.remeshing";

namespace
{
  QColor GetMemoryColor(double fractionOfAvailableMemory)
  {
    if (fractionOfAvailableMemory <= 0.5)
      return QColor(76, 175, 80);

    if (fractionOfAvailableMemory <= 0.85)
      return QColor(255, 152, 0);

    return QColor(244, 67, 54);
  }

  /** \brief The data of the surface node that remeshing works on, or nullptr if there is none. */
  vtkPolyData* GetPolyData(const mitk::DataNode* node)
  {
    return nullptr != node
      ? static_cast<const mitk::Surface*>(node->GetData())->GetVtkPolyData(0)
      : nullptr;
  }

  void SetPulsing(mitk::DataNode* node, bool pulsing)
  {
    node->SetBoolProperty("animated.pulse", pulsing);

    // The first frame of the pulse, or the surface without it.
    mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
  }

  /** \brief The base name, or the base name with the first number from 2 on that no child of the parent is called.
   *
   * Without a parent, no node of the storage may be called like that.
   */
  std::string GetUniqueChildName(const mitk::DataStorage* storage, const mitk::DataNode* parent, const std::string& baseName)
  {
    const auto nodes = nullptr != parent
      ? storage->GetDerivations(parent)
      : storage->GetAll();

    std::set<std::string> takenNames;

    for (const auto& node : *nodes)
      takenNames.insert(node->GetName());

    auto name = baseName;

    for (int number = 2; takenNames.count(name) != 0; ++number)
      name = baseName + "-" + std::to_string(number);

    return name;
  }

  // In the colors of the progress notification cards.
  QString GetInfoCardStyleSheet()
  {
    const auto darkTheme = QmitkIconTheme::IsDarkTheme();

    const auto surface = darkTheme ? QStringLiteral("#3f3f46") : QStringLiteral("palette(base)");
    const auto border = darkTheme ? QStringLiteral("#54545a") : QStringLiteral("palette(mid)");

    return QStringLiteral(
      "#infoFrame { background-color: %1; border: 1px solid %2; border-radius: 4px; }"
      "#infoFrame QLabel { background-color: transparent; border: none; }").arg(surface, border);
  }

  void ReportRemeshingError(const std::exception_ptr& error)
  {
    try
    {
      std::rethrow_exception(error);
    }
    catch (const std::bad_alloc&)
    {
      MITK_ERROR << "Not enough memory to remesh the surface.";
      QMessageBox::warning(nullptr, "Remeshing", "There is not enough memory to remesh the surface. Try a lower density or quality.");
    }
    catch (const itk::ExceptionObject& e)
    {
      // GetDescription() is the plain message; what() prepends source file and line.
      MITK_ERROR << e.what();
      QMessageBox::warning(nullptr, "Remeshing", QString::fromUtf8(e.GetDescription()));
    }
    catch (const std::exception& e)
    {
      MITK_ERROR << e.what();
      QMessageBox::warning(nullptr, "Remeshing", QString::fromUtf8(e.what()));
    }
    catch (...)
    {
      MITK_ERROR << "Remeshing failed.";
      QMessageBox::warning(nullptr, "Remeshing", "Remeshing failed.");
    }
  }
}

struct QmitkRemeshingView::RemeshingRun
{
  // A cancel can be requested before the worker has created the task to
  // cancel, so whichever of the two comes second acts on it.
  std::mutex Mutex;
  mitk::ProgressTaskId TaskId = 0;
  bool CancelRequested = false;
};

QmitkRemeshingView::QmitkRemeshingView()
  : m_Controls(std::make_unique<Ui::QmitkRemeshingViewControls>()),
    m_Parent(nullptr),
    m_OriginalColor({ 1.0f, 1.0f, 1.0f }),
    m_OriginalWasPulsing(false)
{
}

QmitkRemeshingView::~QmitkRemeshingView()
{
  // Without the view, a result would have nowhere to go.
  this->CancelRemeshing();

  if (auto original = m_Original.Lock(); original.IsNotNull())
    SetPulsing(original, m_OriginalWasPulsing);
}

void QmitkRemeshingView::CreateQtPartControl(QWidget* parent)
{
  m_Controls->setupUi(parent);
  m_Parent = parent;

  m_Controls->remeshPushButton->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Remeshing/RemeshingIcon.svg")));
  m_Controls->densityMemoryBar->SetSlider(m_Controls->densitySlider);

  // Neither the style sheet nor a pixmap follows a theme switch on its own.
  const auto updateInfoStyle = [this]()
  {
    auto* iconLabel = m_Controls->infoIconLabel;
    const auto icon = QmitkIconTheme::GetIcon(QStringLiteral(":/Remeshing/info.svg"));

    m_Controls->infoFrame->setStyleSheet(GetInfoCardStyleSheet());
    iconLabel->setPixmap(icon.pixmap(QSize(16, 16), iconLabel->devicePixelRatioF()));
  };

  updateInfoStyle();
  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, m_Controls->infoFrame, updateInfoStyle);

  m_Controls->infoFrame->setVisible(false);
  this->SetUpHoverInfo(parent);

  m_Controls->selectionWidget->SetDataStorage(this->GetDataStorage());
  m_Controls->selectionWidget->SetSelectionIsOptional(true);
  m_Controls->selectionWidget->SetEmptyInfo(QStringLiteral("Select a surface"));
  m_Controls->selectionWidget->SetAutoSelectNewNodes(true);
  m_Controls->selectionWidget->SetNodePredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::Surface>::New(),
    mitk::NodePredicateNot::New(mitk::NodePredicateOr::New(
      mitk::NodePredicateProperty::New("helper object"),
      mitk::NodePredicateProperty::New("hidden object")))));

  connect(&m_Watcher, &QFutureWatcher<RemeshingResult>::finished, this, &QmitkRemeshingView::OnRemeshingFinished);
  connect(m_Controls->selectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkRemeshingView::OnSurfaceChanged);
  connect(m_Controls->densitySlider, SIGNAL(valueChanged(int)), this, SLOT(OnDensityChanged(int)));
  connect(m_Controls->densitySpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnDensityChanged(int)));
  connect(m_Controls->qualityComboBox, &QComboBox::currentIndexChanged, this, &QmitkRemeshingView::UpdateMemoryEstimate);
  connect(m_Controls->remeshPushButton, SIGNAL(clicked()), this, SLOT(OnRemeshButtonClicked()));

  // A replaced original has nothing left to hide.
  connect(m_Controls->replaceCheckBox, &QCheckBox::toggled, this, [this]() {
    this->EnableWidgets(nullptr != GetPolyData(m_Controls->selectionWidget->GetSelectedNode()));
  });

  this->OnSurfaceChanged(m_Controls->selectionWidget->GetSelectedNodes());
}

void QmitkRemeshingView::SetUpHoverInfo(QWidget* parent)
{
  const QString quality = "<b>Quality:</b> High suits nearly every surface. Higher settings make triangles a little "
                          "more uniform, but take much longer and need far more memory.";

  const QString density = "<b>Density:</b> Vertex count of the result relative to the input. 50% halves it, and "
                          "overly dense surfaces often do well with 1-10%.";

  const QString remeshing = "<b>Remeshing:</b> Adaptive places more vertices where the surface bends and fewer where "
                            "it is flat, which keeps fine details even at low density. Regular spreads them evenly.";

  const QString replace = "<b>Replace original surface:</b> The remeshed surface takes the place of the original in "
                          "its node, which keeps all its properties. The original surface is gone afterwards.";

  const QString hide = "<b>Hide original surface:</b> Once the remeshed surface is added, the original is hidden so "
                       "that it does not cover the result.";

  const QString wireframe = "<b>Show remeshed surface as wireframe:</b> Shows the edges of the triangles instead of "
                            "a closed surface.";

  m_HoverInfo = {
    { parent, QString() },
    { m_Controls->surfaceLabel, QString() },
    { m_Controls->selectionWidget, QString() },
    { m_Controls->qualityLabel, quality },
    { m_Controls->qualityComboBox, quality },
    { m_Controls->densityLabel, density },
    { m_Controls->densitySlider, density },
    { m_Controls->densitySpinBox, density },
    { m_Controls->densityMemoryBar, density },
    { m_Controls->remeshingLabel, remeshing },
    { m_Controls->adaptiveButton, remeshing },
    { m_Controls->regularButton, remeshing },
    { m_Controls->remeshPushButton, QString() },
    { m_Controls->advancedGroupBox, QString() },
    { m_Controls->replaceCheckBox, replace },
    { m_Controls->hideCheckBox, hide },
    { m_Controls->wireframeCheckBox, wireframe }
  };

  for (const auto& [widget, info] : m_HoverInfo)
    widget->installEventFilter(this);
}

bool QmitkRemeshingView::eventFilter(QObject* watched, QEvent* event)
{
  if (event->type() == QEvent::Enter)
  {
    if (auto info = m_HoverInfo.find(watched); info != m_HoverInfo.end())
    {
      m_Controls->infoLabel->setText(info->second);
      m_Controls->infoFrame->setVisible(!info->second.isEmpty());
    }
  }
  else if (event->type() == QEvent::Leave && watched == m_Parent)
  {
    // Only here: crossing the gaps between the widgets of a row keeps what is
    // shown, which would otherwise flicker.
    m_Controls->infoLabel->clear();
    m_Controls->infoFrame->setVisible(false);
  }

  return QmitkAbstractView::eventFilter(watched, event);
}

void QmitkRemeshingView::OnSurfaceChanged(const QmitkSingleNodeSelectionWidget::NodeList& nodes)
{
  auto* polyData = !nodes.empty()
    ? GetPolyData(nodes.front())
    : nullptr;

  if (nullptr != polyData)
  {
    m_MaxNumberOfVertices = static_cast<int>(polyData->GetNumberOfPoints());
    this->EnableWidgets(true);
  }
  else
  {
    m_MaxNumberOfVertices = 0;
    this->EnableWidgets(false);
  }

  this->UpdateMemoryEstimate();
}

void QmitkRemeshingView::OnDensityChanged(int density)
{
  if (density != m_Controls->densitySlider->value())
    m_Controls->densitySlider->setValue(density);

  if (density != m_Controls->densitySpinBox->value())
    m_Controls->densitySpinBox->setValue(density);

  this->UpdateMemoryEstimate();
}

int QmitkRemeshingView::GetNumberOfVertices(int density) const
{
  return std::max(100, static_cast<int>(m_MaxNumberOfVertices * (density * 0.01)));
}

int QmitkRemeshingView::GetSubsampling() const
{
  switch (m_Controls->qualityComboBox->currentIndex())
  {
    case 1: // Very high
      return 50;

    case 2: // Maximum
      return 500;

    default: // High
      return 10;
  }
}

void QmitkRemeshingView::UpdateMemoryEstimate()
{
  auto* memoryBar = m_Controls->densityMemoryBar;

  memoryBar->SetColors({});
  memoryBar->SetCaption(QString());

  auto selectedNode = m_Controls->selectionWidget->GetSelectedNode();

  if (selectedNode.IsNull())
    return;

  const auto* surface = static_cast<const mitk::Surface*>(selectedNode->GetData());
  const int subsampling = this->GetSubsampling();
  size_t needed = 0;

  try
  {
    needed = mitk::EstimateRemeshingMemory(surface, 0, this->GetNumberOfVertices(m_Controls->densitySpinBox->value()), subsampling);
  }
  catch (const mitk::Exception&)
  {
    // Remeshing will refuse this surface and say why.
    return;
  }

  const QLocale locale;
  const size_t available = mitk::MemoryUtilities::GetAvailableSizeOfPhysicalRam();

  if (available == 0)
  {
    memoryBar->SetCaption(QString("Needs about %1 of memory")
      .arg(locale.formattedDataSize(static_cast<qint64>(needed), 1, QLocale::DataSizeTraditionalFormat)));
  }
  else
  {
    const double fraction = static_cast<double>(needed) / available;
    const double percentage = 100.0 * fraction;

    memoryBar->SetCaption(QString("Needs %1 of %2 RAM available")
        .arg(percentage < 1.0 ? QStringLiteral("<1%") : QString("%1%").arg(qRound(percentage)))
        .arg(locale.formattedDataSize(static_cast<qint64>(available), 1, QLocale::DataSizeTraditionalFormat)),
      fraction > 0.85 ? GetMemoryColor(fraction) : QColor());

    const auto* densitySlider = m_Controls->densitySlider;
    std::vector<QColor> colors;

    for (int density = densitySlider->minimum(); density <= densitySlider->maximum(); ++density)
    {
      const auto neededAtDensity = mitk::EstimateRemeshingMemory(surface, 0, this->GetNumberOfVertices(density), subsampling);
      colors.push_back(GetMemoryColor(static_cast<double>(neededAtDensity) / available));
    }

    memoryBar->SetColors(colors);
  }
}

void QmitkRemeshingView::OnRemeshButtonClicked()
{
  auto selectedNode = m_Controls->selectionWidget->GetSelectedNode();
  auto* selectedPolyData = GetPolyData(selectedNode);

  if (this->IsRemeshing() || nullptr == selectedPolyData)
    return;

  int numVertices = this->GetNumberOfVertices(m_Controls->densitySpinBox->value());

  // A stronger gradation follows sharp bends more closely but turns a growing
  // share of the triangles into slivers.
  double gradation = m_Controls->adaptiveButton->isChecked()
    ? 0.5
    : 0.0;

  int subsampling = this->GetSubsampling();

  // A copy, as the original stays on display and open to changes while the
  // worker reads it. Only of the time step that is remeshed, which is also
  // the copy of the input that the memory estimate includes.
  auto polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->DeepCopy(selectedPolyData);

  auto input = mitk::Surface::New();
  input->SetVtkPolyData(polyData);

  auto remesher = mitk::RemeshFilter::New();
  remesher->SetInput(input);
  remesher->SetTimeStep(0);
  remesher->SetNumVertices(numVertices);
  remesher->SetGradation(gradation);
  remesher->SetSubsampling(subsampling);
  remesher->SetEdgeSplitting(0.0);
  remesher->SetOptimizationLevel(1.0);
  remesher->SetForceManifold(false);
  remesher->SetBoundaryFixing(false);

  // What the result takes from the original, in case the original is gone
  // by the time the result is ready.
  m_Original = selectedNode;
  m_OriginalName = selectedNode->GetName();
  m_OriginalColor = { 1.0f, 1.0f, 1.0f };
  selectedNode->GetColor(m_OriginalColor.data());

  m_ResultOptions.ReplaceOriginal = m_Controls->replaceCheckBox->isChecked();
  m_ResultOptions.HideOriginal = m_Controls->hideCheckBox->isChecked();
  m_ResultOptions.Wireframe = m_Controls->wireframeCheckBox->isChecked();

  // The original pulses until remeshing ends, however it ends.
  m_OriginalWasPulsing = false;
  selectedNode->GetBoolProperty("animated.pulse", m_OriginalWasPulsing);
  SetPulsing(selectedNode, true);

  m_Run = std::make_shared<RemeshingRun>();

  // Everything the worker uses is its own, so that the view stays usable and
  // may even be closed while it runs. The task is created there, since a
  // task belongs to the thread that reports into it.
  auto future = QtConcurrent::run([remesher, run = m_Run]()
    {
      RemeshingResult result;
      mitk::ProgressTask task("Remeshing surface", mitk::ProgressTask::Indeterminate, true);

      {
        std::lock_guard<std::mutex> lock(run->Mutex);
        run->TaskId = task.GetId();
        result.Canceled = run->CancelRequested;
      }

      if (result.Canceled)
        return result;

      mitk::ScopedProgressTask<mitk::RemeshFilter> scopedTask(remesher.GetPointer(), &task);

      try
      {
        remesher->Update();

        // Remeshing polls for a cancel only while it clusters, and one
        // requested while it finishes the result is still a cancel.
        result.Canceled = task.IsCancelRequested();

        if (!result.Canceled)
        {
          result.Surface = remesher->GetOutput();
          result.Surface->DisconnectPipeline();
        }
      }
      catch (const itk::ProcessAborted&)
      {
        result.Canceled = true;
      }
      catch (...)
      {
        result.Error = std::current_exception();
      }

      return result;
    });

  m_Watcher.setFuture(future);
  this->EnableWidgets(false);
}

bool QmitkRemeshingView::IsRemeshing() const
{
  return nullptr != m_Run;
}

void QmitkRemeshingView::CancelRemeshing()
{
  if (!this->IsRemeshing())
    return;

  mitk::ProgressTaskId taskId = 0;

  {
    std::lock_guard<std::mutex> lock(m_Run->Mutex);
    m_Run->CancelRequested = true;
    taskId = m_Run->TaskId;
  }

  if (taskId == 0)
    return;

  if (auto* progressService = mitk::CoreServices::GetProgressService(); nullptr != progressService)
  {
    progressService->RequestCancel(taskId);
    mitk::CoreServices::Unget(progressService);
  }
}

void QmitkRemeshingView::OnRemeshingFinished()
{
  const auto result = m_Watcher.result();
  m_Run.reset();

  auto original = m_Original.Lock();
  m_Original = nullptr;

  if (original.IsNotNull())
    SetPulsing(original, m_OriginalWasPulsing);

  this->EnableWidgets(nullptr != GetPolyData(m_Controls->selectionWidget->GetSelectedNode()));

  if (result.Error)
  {
    ReportRemeshingError(result.Error);
    return;
  }

  if (result.Canceled || result.Surface.IsNull())
    return;

  auto* dataStorage = this->GetDataStorage().GetPointer();

  // The original may have been removed while the remeshing ran.
  if (original.IsNotNull() && !dataStorage->Exists(original))
    original = nullptr;

  const auto& options = m_ResultOptions;

  if (options.ReplaceOriginal && original.IsNotNull())
  {
    // The node keeps everything else it had, its opacity and its children included.
    original->SetData(result.Surface);

    if (options.Wireframe)
      original->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));

    // Unlike an added node, this changes no selection that would update the
    // vertex count the density and the memory estimate are based on.
    this->OnSurfaceChanged(m_Controls->selectionWidget->GetSelectedNodes());
  }
  else
  {
    auto newNode = mitk::DataNode::New();
    newNode->SetName(GetUniqueChildName(dataStorage, original, m_OriginalName + "-remeshed"));
    newNode->SetColor(m_OriginalColor.data());
    newNode->SetData(result.Surface);

    if (options.Wireframe)
      newNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));

    if (original.IsNotNull())
    {
      dataStorage->Add(newNode, original);

      if (options.HideOriginal)
        original->SetVisibility(false);
    }
    else
    {
      dataStorage->Add(newNode);
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkRemeshingView::EnableWidgets(bool enable)
{
  // A setting changed during a remeshing would not be the one it remeshes with.
  const bool remeshing = this->IsRemeshing();
  enable = enable && !remeshing;

  m_Controls->selectionWidget->setEnabled(!remeshing);
  m_Controls->densitySlider->setEnabled(enable);
  m_Controls->densitySpinBox->setEnabled(enable);
  m_Controls->densityMemoryBar->setEnabled(enable);
  m_Controls->adaptiveButton->setEnabled(enable);
  m_Controls->regularButton->setEnabled(enable);
  m_Controls->qualityComboBox->setEnabled(enable);
  m_Controls->remeshPushButton->setEnabled(enable);
  m_Controls->replaceCheckBox->setEnabled(enable);
  m_Controls->hideCheckBox->setEnabled(enable && !m_Controls->replaceCheckBox->isChecked());
  m_Controls->wireframeCheckBox->setEnabled(enable);
}

void QmitkRemeshingView::SetFocus()
{
  m_Controls->selectionWidget->setFocus();
}
