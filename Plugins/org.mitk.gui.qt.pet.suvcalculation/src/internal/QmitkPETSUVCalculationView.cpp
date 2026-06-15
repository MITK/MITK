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

#include "QmitkPETSUVCalculationView.h"
#include <ui_QmitkPETSUVCalculationViewControls.h>

#include <mitkSUVCalculationHelper.h>
#include <mitkSUVImageFilter.h>
#include <mitkSUVInputModel.h>
#include <mitkWorkbenchUtil.h>

#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkHalfLifeConstants.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateFunction.h>
#include <mitkNodePredicateNot.h>
#include <mitkMultiLabelPredicateHelper.h>

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>

#include <sstream>

const std::string QmitkPETSUVCalculationView::VIEW_ID = "org.mitk.QmitkPETSUVCalculationView";

namespace
{
  mitk::NodePredicateBase::Pointer GenerateSelectionPredicate(bool onlyPET)
  {
    auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    auto isNoMask = mitk::NodePredicateNot::New(mitk::GetMultiLabelSegmentationPredicate());

    auto petInputPredicate = mitk::NodePredicateAnd::New(isImage, isNoMask);

    if (onlyPET)
    {
      auto modalityCheck = [](const mitk::DataNode* node)
        {
          bool result = false;

          if (nullptr == node || nullptr == node->GetData())
            return result;

          auto props = mitk::GetPropertyByDICOMTagPath(node->GetData(), mitk::DICOMTagPath(0x0008, 0x0060));

          if (!props.empty())
          {
            result = props.begin()->second->GetValueAsString() == "PT";
          }

          return result;
        };
      auto modalityPredicate = mitk::NodePredicateFunction::New(modalityCheck);
      petInputPredicate->AddPredicate(modalityPredicate);
    }

    return petInputPredicate;
  }

  // Keep the human-readable label in sync with the actual decay-correction
  // strategy detected by the filter; the previous hard-coded text was
  // misleading whenever the input used Admin / Start / None.
  QString StrategyToString(mitk::DecayCorrectionStrategy s)
  {
    using S = mitk::DecayCorrectionStrategy;
    switch (s)
    {
      case S::Admin:  return QStringLiteral("Administration time (DICOM Decay Correction = ADMIN)");
      case S::Start:  return QStringLiteral("Series start time (DICOM Decay Correction = START)");
      case S::None:   return QStringLiteral("Per-slice acquisition time (DICOM Decay Correction = NONE)");
      case S::Manual: return QStringLiteral("User-defined decay time");
    }
    return QStringLiteral("Unknown");
  }

  std::string GetBaseDataPropValueAsString(const mitk::BaseData *data, const mitk::DICOMTagPath &path)
  {
    std::string result;

    if (data)
    {
      auto props = mitk::GetPropertyByDICOMTagPath(data, path);
      if (!props.empty())
      {
        result = props.begin()->second->GetValueAsString();
      }
    }

    return result;
  }
}

void QmitkPETSUVCalculationView::SetFocus()
{
  m_Controls->btnCalculateSUV->setFocus();
}

void QmitkPETSUVCalculationView::CreateQtPartControl(QWidget *parent)
{
  m_ParentWidget = parent;
  m_Controls->setupUi(parent);

  m_Controls->decayTimeView->setAlternatingRowColors(true);
  m_Controls->decayTimeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls->decayTimeView->setRootIsDecorated(true);
  m_Controls->decayTimeView->setSortingEnabled(false);

  m_decayTimeModel = std::make_unique<DecayTimeMapModel>(this);

  m_Controls->decayTimeView->setModel(m_decayTimeModel.get());
  m_Controls->decayTimeView->setItemDelegate(new DecayTimeDelegate(this));

  m_Controls->decayTimeView->header()->setStretchLastSection(false);
  m_Controls->decayTimeView->header()->resizeSection(0, 200);
  m_Controls->decayTimeView->header()->resizeSection(1, 150);
  m_Controls->decayTimeView->header()->setDefaultSectionSize(150);

  connect(m_Controls->btnCalculateSUV,  &QPushButton::clicked,        this, &QmitkPETSUVCalculationView::OnCalculateSUVButtonClicked);
  connect(m_Controls->btnNuclideLookup, &QPushButton::clicked,        this, &QmitkPETSUVCalculationView::OnNuclideLookupClicked);

  connect(m_Controls->halflifeSpinBox,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QmitkPETSUVCalculationView::OnHalfLifeChanged);
  connect(m_Controls->activitySpinBox,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QmitkPETSUVCalculationView::OnInjectedActivityChanged);
  connect(m_Controls->weightSpinBox,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QmitkPETSUVCalculationView::OnBodyWeightChanged);
  connect(m_Controls->timeSpinBox,      QOverload<int>::of(&QSpinBox::valueChanged),          this, &QmitkPETSUVCalculationView::OnTimeToMeasurementChanged);

  connect(m_Controls->radioTimeAuto, &QRadioButton::toggled, this, &QmitkPETSUVCalculationView::OnDecayTimeRadioToggled);
  connect(m_Controls->radioTimeUser, &QRadioButton::toggled, this, &QmitkPETSUVCalculationView::OnDecayTimeRadioToggled);

  connect(m_Controls->checkPETonly, &QCheckBox::toggled, this, &QmitkPETSUVCalculationView::OnCheckPETOnlyToggled);

  connect(m_Controls->petNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkPETSUVCalculationView::OnPETSelectionChanged);

  m_Controls->radioTimeAuto->setChecked(true);
  m_Controls->timeSpinBox->setEnabled(false);

  m_Controls->petNodeSelector->SetSelectionIsOptional(false);
  m_Controls->petNodeSelector->SetInvalidInfo("Select PET image for conversion.");
  m_Controls->petNodeSelector->SetEmptyInfo("Select PET image for conversion.");
  m_Controls->petNodeSelector->SetPopUpTitel("Select PET image.");
  m_Controls->petNodeSelector->SetPopUpHint("Select a PET image that should be the source for the SUV conversion.");

  m_Controls->petNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls->petNodeSelector->SetNodePredicate(GenerateSelectionPredicate(m_Controls->checkPETonly->isChecked()));

  // Should be done last, if everything else is configured because it triggers the autoselection of data.
  m_Controls->petNodeSelector->SetAutoSelectNewNodes(true);

  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnCheckPETOnlyToggled(bool)
{
  m_Controls->petNodeSelector->SetNodePredicate(GenerateSelectionPredicate(m_Controls->checkPETonly->isChecked()));
}

void QmitkPETSUVCalculationView::OnInjectedActivityChanged(double value)
{
  // widget is [kBq], filter override is [Bq]
  m_Filter->SetInjectedActivityInBq(value * 1000.0);
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnBodyWeightChanged(double value)
{
  // widget is [kg], filter override is [g]
  m_Filter->SetPatientWeightInGram(value * 1000.0);
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnTimeToMeasurementChanged(int value)
{
  // The override slot is only meaningful while the user has chosen the
  // user-defined branch. When auto is active the override must stay clear
  // so the filter falls back to the DICOM-derived per-slice values.
  if (m_Controls->radioTimeUser->isChecked())
  {
    m_Filter->SetDecayTimeOverrideInSec(static_cast<double>(value) * 60.0);
  }
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnHalfLifeChanged(double value)
{
  // widget is [min], filter override is [s]
  m_Filter->SetHalfLifeInSec(value * 60.0);
  // The auto-detected nuclide name no longer matches an arbitrary user
  // value; clear the label so we don't claim a wrong nuclide.
  this->m_DefinedNuclide.clear();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnDecayTimeRadioToggled()
{
  if (m_Controls->radioTimeUser->isChecked())
  {
    m_Filter->SetDecayTimeOverrideInSec(static_cast<double>(m_Controls->timeSpinBox->value()) * 60.0);
  }
  else
  {
    m_Filter->ClearDecayTimeOverrideInSec();
  }
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnNuclideLookupClicked()
{
  QStringList items;
  for (const auto &nuclide : this->m_HalfLifeMap)
  {
    items << tr(nuclide.first.c_str());
  }

  bool ok;
  QString item = QInputDialog::getItem(
    m_ParentWidget, tr("Radio nuclide selection"), tr("Select predefined radio nuclide:"), items, 0, false, &ok);

  if (ok && !item.isEmpty())
  {
    auto finding = this->m_HalfLifeMap.find(item.toStdString());
    if (finding != this->m_HalfLifeMap.end())
    {
      m_Filter->SetHalfLifeInSec(finding->second);
      this->m_DefinedNuclide = finding->first;
    }
    this->UpdateWidgets();
  }
}

void QmitkPETSUVCalculationView::OnCalculateSUVButtonClicked()
{
  auto inputNode = m_Controls->petNodeSelector->GetSelectedNode();
  if (inputNode.IsNull())
  {
    return;
  }
  auto image = dynamic_cast<mitk::Image *>(inputNode->GetData());
  if (nullptr == image)
  {
    mitkThrow() << "QmitkPETSUVCalculationView is in invalid state. Selected node does not contain an mitk::Image, despite the PET node selector should enforce it.";
  }

  // The modality / units sanity dialogs duplicate checks the filter would
  // also perform, but they let the user opt out of a hard fail when the
  // DICOM tags are missing or wrong. Kept until the diagnostics surface is
  // redesigned.
  const mitk::DICOMTagPath modalityPath(0x0008, 0x0060);
  const mitk::DICOMTagPath unitsPath(0x0054, 0x1001);

  const QString modality = QString::fromStdString(GetBaseDataPropValueAsString(image, modalityPath));
  const QString unit     = QString::fromStdString(GetBaseDataPropValueAsString(image, unitsPath));

  bool isPET   = modality.compare(QString("PT"), Qt::CaseInsensitive) == 0;
  bool isBqMl  = unit.compare(QString("BQML"), Qt::CaseInsensitive) == 0;

  if (!isPET)
  {
    QMessageBox box;
    box.setText("No PET data!");
    box.setInformativeText(
      "Selected data seems to be no PET data. Dicom tag \"dicom.series.Modality\" is missing or has wrong value. Won't "
      "calculate SUV map. You may ignore and force computation at own risk.");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Ignore);
    box.setDefaultButton(QMessageBox::Ok);
    box.setIcon(QMessageBox::Warning);
    if (box.exec() == QMessageBox::Ignore)
    {
      isPET = true;
    }
  }
  if (!isPET)
  {
    return;
  }

  if (!isBqMl)
  {
    QMessageBox box;
    box.setText("Wrong PET unit!");
    box.setInformativeText(
      "Selected data seems to have no or wrong PET unit (required: BQML). Dicom tag \"dicom.series.Unit\" is missing "
      "or has wrong value. Won't calculate SUV map. You may ignore and force computation at own risk.");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Ignore);
    box.setDefaultButton(QMessageBox::Ok);
    box.setIcon(QMessageBox::Warning);
    if (box.exec() == QMessageBox::Ignore)
    {
      // Force activity-concentration semantics through the filter so the
      // SUV math runs as if the input were Bq/mL. Without re-running
      // ConfigureFromProperties the override would not take effect, since
      // the filter already cached the effective input model.
      mitk::SUVInputModel forced;
      forced.semantics     = mitk::SUVPixelSemantics::ActivityConcentration;
      forced.activityScale = 1.0;
      m_Filter->SetInputModelOverride(forced);
      try
      {
        m_Filter->ConfigureFromProperties(image);
        m_Configured = true;
        m_LastConfigError.clear();
      }
      catch (const mitk::Exception& e)
      {
        m_LastConfigError = e.GetDescription();
        MITK_ERROR << "PET SUV reconfiguration after units override failed: " << e;
        this->UpdateWidgets();
        return;
      }
    }
    else
    {
      return;
    }
  }

  MITK_INFO << "Calculating SUV via mitk::SUVImageFilter for node '" << inputNode->GetName() << "'.";

  try
  {
    m_Filter->Update();
    auto suvImage = m_Filter->GetOutput();

    auto resultNode = mitk::DataNode::New();
    resultNode->SetName(inputNode->GetName() + "_SUV");
    resultNode->SetData(suvImage);
    this->GetDataStorage()->Add(resultNode, inputNode);
  }
  catch (const mitk::Exception& e)
  {
    QMessageBox::critical(m_ParentWidget, tr("SUV computation failed"),
      QString::fromStdString(e.GetDescription()));
    MITK_ERROR << "SUV Update() failed: " << e;
  }
}

void QmitkPETSUVCalculationView::UpdateWidgets()
{
  // Programmatic writes to the spinboxes would otherwise re-enter the
  // user-edit slots and clobber state (notably m_DefinedNuclide). Block
  // signals at the call site instead of relying on a class-wide flag.
  const QSignalBlocker blockA(m_Controls->activitySpinBox);
  const QSignalBlocker blockW(m_Controls->weightSpinBox);
  const QSignalBlocker blockH(m_Controls->halflifeSpinBox);
  const QSignalBlocker blockT(m_Controls->timeSpinBox);

  if (m_Configured)
  {
    m_Controls->activitySpinBox->setValue(m_Filter->GetEffectiveInjectedActivityInBq() / 1000.0);
    m_Controls->weightSpinBox  ->setValue(m_Filter->GetEffectivePatientWeightInGram() / 1000.0);
    m_Controls->halflifeSpinBox->setValue(m_Filter->GetEffectiveHalfLifeInSec() / 60.0);
  }
  else
  {
    m_Controls->activitySpinBox->setValue(0.0);
    m_Controls->weightSpinBox  ->setValue(0.0);
    m_Controls->halflifeSpinBox->setValue(0.0);
  }

  m_Controls->timeSpinBox->setEnabled(m_Controls->radioTimeUser->isChecked());
  if (auto override = m_Filter->GetDecayTimeOverrideInSec())
  {
    m_Controls->timeSpinBox->setValue(static_cast<int>(*override / 60.0));
  }

  m_Controls->timeInfo->clear();
  if (!m_LastConfigError.empty())
  {
    m_Controls->timeInfo->setText(
      QString::fromStdString("Configuration error: " + m_LastConfigError));
  }
  else if (m_Configured)
  {
    const auto info = m_Filter->GetEffectiveDecayCorrection();
    std::ostringstream stream;
    stream << "Detection strategy: " << StrategyToString(info.strategy).toStdString() << '\n';
    for (const auto &timePos : info.decayTimes)
    {
      for (const auto &slicePos : timePos.second)
      {
        stream << '[' << timePos.first << "][" << slicePos.first
               << "]: " << slicePos.second / 60.0 << " [min]; ";
      }
    }
    m_Controls->timeInfo->setText(QString::fromStdString(stream.str()));
  }

  m_Controls->labelAutoNuclide->setText(QString::fromStdString(this->m_DefinedNuclide));

  // Refresh the tree-view contents from current state. In auto mode the
  // model mirrors the filter's per-(timestep, slice) decay times; in
  // user-defined mode the override is applied uniformly, so we synthesize
  // a one-entry-per-timestep map keyed on the timesteps the filter sees.
  // Without this rebuild the view would keep showing the auto values from
  // the previous mode.
  const bool userMode = m_Controls->radioTimeUser->isChecked();
  if (m_Configured)
  {
    const auto info = m_Filter->GetEffectiveDecayCorrection();
    if (userMode)
    {
      const double overrideSec = m_Filter->GetDecayTimeOverrideInSec().value_or(0.0);
      mitk::DecayTimeMapType userMap;
      for (const auto &timePos : info.decayTimes)
      {
        userMap[timePos.first][0] = overrideSec;
      }
      m_decayTimeModel->SetDecayTimeMap(userMap);
    }
    else
    {
      m_decayTimeModel->SetDecayTimeMap(info.decayTimes);
    }
  }
  else
  {
    m_decayTimeModel->SetDecayTimeMap({});
  }
  m_decayTimeModel->SetMode(
    userMode ? DecayTimeMapModel::Mode::UserDefined : DecayTimeMapModel::Mode::Auto);

  const bool valid =
    m_Configured
    && m_LastConfigError.empty()
    && m_Controls->petNodeSelector->GetSelectedNode().IsNotNull();
  m_Controls->btnCalculateSUV->setEnabled(valid);
}

void QmitkPETSUVCalculationView::OnPETSelectionChanged(QList<mitk::DataNode::Pointer> /*nodes*/)
{
  m_Controls->btnCalculateSUV->setEnabled(false);
  m_DefinedNuclide.clear();
  m_LastConfigError.clear();
  m_Configured = false;
  m_decayTimeModel->SetDecayTimeMap({});

  // Replace the filter rather than clearing every slot individually so
  // overrides from a previous selection cannot bleed into the new one.
  m_Filter = mitk::SUVImageFilter::New();

  auto newNode = m_Controls->petNodeSelector->GetSelectedNode();
  if (newNode.IsNull())
  {
    this->UpdateWidgets();
    return;
  }
  auto image = dynamic_cast<mitk::Image *>(newNode->GetData());
  if (nullptr == image)
  {
    m_LastConfigError = "Selected node does not contain an mitk::Image.";
    this->UpdateWidgets();
    return;
  }

  m_Filter->SetInput(image);

  try
  {
    m_Filter->ConfigureFromProperties(image);
    m_Configured = true;

    // Best-effort lookup of a display name for the nuclide. The half-life
    // itself has already been resolved by the filter; failure here is
    // cosmetic and must not invalidate the configuration.
    try
    {
      const auto rpis = mitk::GetRadiopharmaceuticalInfos(image);
      if (!rpis.empty())
      {
        m_DefinedNuclide = rpis.front().name;
      }
    }
    catch (const mitk::Exception&)
    {
      // leave m_DefinedNuclide empty
    }
  }
  catch (const mitk::Exception& e)
  {
    m_LastConfigError = e.GetDescription();
    MITK_ERROR << "PET SUV configuration failed: " << e;
  }

  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::GenerateHalfLifeMap()
{
  this->m_HalfLifeMap.clear();

  this->m_HalfLifeMap.insert(std::make_pair(mitk::HALFLIFECONSTANTS::NAME_18F, mitk::HALFLIFECONSTANTS::VALUE_18F));
  this->m_HalfLifeMap.insert(std::make_pair(mitk::HALFLIFECONSTANTS::NAME_68Ga, mitk::HALFLIFECONSTANTS::VALUE_68Ga));
  this->m_HalfLifeMap.insert(std::make_pair(mitk::HALFLIFECONSTANTS::NAME_11C, mitk::HALFLIFECONSTANTS::VALUE_11C));
  this->m_HalfLifeMap.insert(std::make_pair(mitk::HALFLIFECONSTANTS::NAME_15O, mitk::HALFLIFECONSTANTS::VALUE_15O));
}

QmitkPETSUVCalculationView::QmitkPETSUVCalculationView()
  : m_Controls(std::make_unique<Ui::QmitkPETSUVCalculationViewControls>()),
    m_Filter(mitk::SUVImageFilter::New()),
    m_ParentWidget(nullptr)
{
  GenerateHalfLifeMap();
}

// Out-of-line destructor: required because m_Controls is a
// std::unique_ptr to a forward-declared Ui type; the implicit
// destructor needs the complete type, which is only visible in
// this translation unit.
QmitkPETSUVCalculationView::~QmitkPETSUVCalculationView() = default;



DecayTimeMapModel::DecayTimeMapModel(QObject* parent)
  : QAbstractItemModel(parent)
  , m_Mode(Mode::Auto)
{
}

QModelIndex DecayTimeMapModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!hasSingleTimeStep() && m_Mode == Mode::Auto)
  {
    if (!parent.isValid())
    { //we have a time step level and thats it
      return createIndex(row, column);
    }
    else
    { //we also have a 2nd slice index level and that is it.
      //we encode the row of the time level as internal data
      return createIndex(row, column, static_cast<quintptr>(parent.row()+1)); // mark as 2nd level child
    }
  }

  return createIndex(row, column);
}

QModelIndex DecayTimeMapModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  if (m_Mode == Mode::Auto && !hasSingleTimeStep())
  {
    if (child.internalPointer() != nullptr)
    { //we are on the second level
      const auto timeStepRow = static_cast<int>(child.internalId()-1);
      return createIndex(timeStepRow, 0);
    }
  }

  return QModelIndex();
}

int DecayTimeMapModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
  {
    if (m_Mode != Mode::UserDefined && hasSingleTimeStep())
    {
      return static_cast<int>(m_DecayTimeMap.begin()->second.size());
    }

    return static_cast<int>(m_DecayTimeMap.size());
  }

  if (m_Mode == Mode::Auto && !hasSingleTimeStep())
  {
    auto timeStep = GetTimeStep(parent);
    if (timeStep && m_DecayTimeMap.count(*timeStep))
    {
      return static_cast<int>(m_DecayTimeMap.at(*timeStep).size());
    }
  }

  return 0;
}

int DecayTimeMapModel::columnCount(const QModelIndex&) const
{
  return 2;
}

QVariant DecayTimeMapModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || role != Qt::DisplayRole)
    return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    const int col = index.column();

    auto timeStepOpt = GetTimeStep(index);
    auto sliceOpt = GetSliceIndex(index);

    if (m_Mode == Mode::UserDefined)
    {
      if (!timeStepOpt)
        return QVariant();

      if (col == 0)
      {
        return QString("Time Step %1").arg(*timeStepOpt);
      }
      else
      {
        auto decayTime = m_DecayTimeMap.at(*timeStepOpt).at(*sliceOpt);
        return QVariant(decayTime);
      }
    }
    else
    {
      if (!index.parent().isValid() && !this->hasSingleTimeStep())
      {
        if (col == 0 && timeStepOpt)
          return QString("Time Step %1").arg(*timeStepOpt);
      }
      else
      {
        if (timeStepOpt && sliceOpt)
        {
          auto decayTime = m_DecayTimeMap.at(*timeStepOpt).at(*sliceOpt);
          if (col == 0)
            return QString("Slice %1").arg(*sliceOpt);
          else
            return QVariant(decayTime);
        }
      }
    }
  }
  else if (role == Qt::ToolTipRole && index.column() == 1)
  {
    return QString("Decay time in seconds");
  }

  return {};
}

QVariant DecayTimeMapModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0: return "Item";
    case 1: return "Decay Time [s]";
    default: return QVariant();
    }
  }
  return QVariant();
}

Qt::ItemFlags DecayTimeMapModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  // Only allow editing decay time column in user-defined mode
  if (m_Mode == Mode::UserDefined && index.column() == 1) {
    flags |= Qt::ItemIsEditable;
  }

  return flags;
}

bool DecayTimeMapModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role != Qt::EditRole || !index.isValid() || index.column() != 1)
    return false;

  if (m_Mode == Mode::Auto)
    return false;

  bool ok = false;
  double newDecay = value.toDouble(&ok);
  if (!ok)
    return false;

  auto timeStepOpt = GetTimeStep(index.parent());
  auto sliceOpt = GetSliceIndex(index);
  if (!timeStepOpt || !sliceOpt)
    return false;

  auto& decay = m_DecayTimeMap[*timeStepOpt][*sliceOpt];
  decay = newDecay;
  emit dataChanged(index, index);
  return true;
}

void DecayTimeMapModel::SetDecayTimeMap(const mitk::DecayTimeMapType& decayTimeMap)
{
  beginResetModel();
  m_DecayTimeMap = decayTimeMap;
  endResetModel();
}

void DecayTimeMapModel::SetMode(Mode mode)
{
  if (m_Mode != mode) {
    beginResetModel();
    m_Mode = mode;
    endResetModel();
  }
}

DecayTimeMapModel::Mode DecayTimeMapModel::GetMode() const
{
  return m_Mode;
}

bool DecayTimeMapModel::hasSingleTimeStep() const
{
  return m_DecayTimeMap.size() == 1;
}

std::optional<mitk::TimeStepType> DecayTimeMapModel::GetTimeStep(const QModelIndex& index) const
{
  if (!index.isValid())
    return std::nullopt;

  if (hasSingleTimeStep())
  { //if there is only one time step it is always clear
    return 0;
  }

  if (index.parent().isValid())
  { //we are at the slice level. Deduce timestep from parent
    return GetTimeStep(index.parent());
  }

  //we are top level (time step) so we can take just the row
  auto it = m_DecayTimeMap.begin();
  std::advance(it, index.row());
  return it->first;
}

std::optional<mitk::SlicedData::IndexValueType> DecayTimeMapModel::GetSliceIndex(const QModelIndex& index) const
{
  if (!index.isValid())
    return std::nullopt;

  if (m_Mode == Mode::UserDefined)
  {// in user mode we always only have slice 0. As the decay time is set for every slice in the timestep the same
    return 0;
  }

  if (hasSingleTimeStep())
  { //auto mode with only one time step only has one level which is the slice level -> get directly the row
    const auto& slices = m_DecayTimeMap.begin()->second;
    auto it = slices.begin();
    std::advance(it, index.row());
    return it->first;
  }

  if (index.parent().isValid())
  {
    auto tsOpt = GetTimeStep(index);
    if (!tsOpt || !m_DecayTimeMap.count(*tsOpt))
      return std::nullopt;

    const auto& slices = m_DecayTimeMap.at(*tsOpt);
    auto it = slices.begin();
    std::advance(it, index.row());
    return it->first;
  }

  return std::nullopt;
}


//=============================================================================
// DecayTimeDelegate Implementation
//=============================================================================

DecayTimeDelegate::DecayTimeDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* DecayTimeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  Q_UNUSED(option)

    if (index.column() != 1) // Only column 1 (decay time) is editable
      return nullptr;

  auto* spinBox = new QDoubleSpinBox(parent);
  spinBox->setRange(MIN_DECAY_TIME, MAX_DECAY_TIME);
  spinBox->setDecimals(DECIMALS);
  spinBox->setSingleStep(SINGLE_STEP);
  spinBox->setSuffix(" [s]");

  return spinBox;
}

void DecayTimeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  auto* spinBox = qobject_cast<QDoubleSpinBox*>(editor);
  if (!spinBox)
    return;

  double value = index.model()->data(index, Qt::EditRole).toDouble();
  spinBox->setValue(value);
}

void DecayTimeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
  const QModelIndex& index) const
{
  auto* spinBox = qobject_cast<QDoubleSpinBox*>(editor);
  if (!spinBox)
    return;

  spinBox->interpretText();
  double value = spinBox->value();
  model->setData(index, value, Qt::EditRole);
}

void DecayTimeDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
