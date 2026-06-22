/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
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
#include <mitkSUVNormalizationStrategy.h>
#include <mitkWorkbenchUtil.h>

#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkHalfLifeConstants.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateFunction.h>
#include <mitkNodePredicateNot.h>
#include <mitkMultiLabelPredicateHelper.h>

#include <QApplication>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStringList>
#include <QTextDocument>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <limits>

const QString QmitkPETSUVCalculationView::NUCLIDE_CUSTOM_LABEL = QStringLiteral("(custom)");

namespace
{
  std::string GetBaseDataPropValueAsString(const mitk::BaseData* data, const mitk::DICOMTagPath& path)
  {
    if (nullptr == data) return {};
    auto props = mitk::GetPropertyByDICOMTagPath(data, path);
    return props.empty() ? std::string{} : props.begin()->second->GetValueAsString();
  }

  std::string TrimUpper(const std::string& s)
  {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    std::string out = s.substr(first, last - first + 1);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return out;
  }

  bool IsPETModality(const mitk::BaseData* image)
  {
    return TrimUpper(GetBaseDataPropValueAsString(image, mitk::DICOMTagPath(0x0008, 0x0060))) == "PT";
  }

  mitk::NodePredicateBase::Pointer GenerateSelectionPredicate(bool onlyPET)
  {
    // Predicate shape is determined entirely by the boolean argument, so
    // build each variant once and reuse on every modality toggle.
    static const mitk::NodePredicateBase::Pointer kAnyImage = []() {
      auto isImage  = mitk::TNodePredicateDataType<mitk::Image>::New();
      auto isNoMask = mitk::NodePredicateNot::New(mitk::GetMultiLabelSegmentationPredicate());
      return mitk::NodePredicateBase::Pointer(mitk::NodePredicateAnd::New(isImage, isNoMask).GetPointer());
    }();
    static const mitk::NodePredicateBase::Pointer kPETOnly = []() {
      auto isImage  = mitk::TNodePredicateDataType<mitk::Image>::New();
      auto isNoMask = mitk::NodePredicateNot::New(mitk::GetMultiLabelSegmentationPredicate());
      // Route through IsPETModality so the selector and the calculation
      // gate apply the same trimmed, upper-cased Modality comparison; a
      // raw "== PT" here would hide " PT " / "pt" images the gate accepts.
      auto modalityCheck = [](const mitk::DataNode* node)
      {
        return nullptr != node && IsPETModality(node->GetData());
      };
      auto pred = mitk::NodePredicateAnd::New(isImage, isNoMask);
      pred->AddPredicate(mitk::NodePredicateFunction::New(modalityCheck));
      return mitk::NodePredicateBase::Pointer(pred.GetPointer());
    }();
    return onlyPET ? kPETOnly : kAnyImage;
  }

  QString VariantToString(mitk::SUVVariant v)
  {
    using V = mitk::SUVVariant;
    switch (v)
    {
      case V::BW:                return QStringLiteral("BW");
      case V::LBM_Janmahasatian: return QStringLiteral("LBM-Janmahasatian");
      case V::LBM_James128:      return QStringLiteral("LBM-James128");
      case V::IBW:               return QStringLiteral("IBW (Sugawara)");
      case V::BSA:               return QStringLiteral("BSA (DuBois)");
    }
    return QStringLiteral("Unknown");
  }

  QString SemanticsToString(mitk::SUVPixelSemantics s)
  {
    using S = mitk::SUVPixelSemantics;
    switch (s)
    {
      case S::ActivityConcentration: return QStringLiteral("Activity concentration");
      case S::PrenormalizedSUV:      return QStringLiteral("Pre-normalized SUV");
    }
    return QStringLiteral("Unknown");
  }

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

  struct VariantEntry { mitk::SUVVariant value; const char* label; };
  const std::array<VariantEntry, 5> kVariantEntries = {{
    {mitk::SUVVariant::BW,                "BW (body weight)"},
    {mitk::SUVVariant::LBM_Janmahasatian, "LBM-Janmahasatian"},
    {mitk::SUVVariant::LBM_James128,      "LBM-James128"},
    {mitk::SUVVariant::IBW,               "IBW (Sugawara)"},
    {mitk::SUVVariant::BSA,               "BSA (DuBois)"},
  }};

  struct SexEntry { mitk::Sex value; const char* label; };
  const std::array<SexEntry, 3> kSexEntries = {{
    {mitk::Sex::Male,   "M (male)"},
    {mitk::Sex::Female, "F (female)"},
    {mitk::Sex::Other,  "O (other, IBSI mean adaptation)"},
  }};

  // Decay-timing problems that the time group's strategy line surfaces
  // directly (in addition to the diagnostics box), so an ambiguous
  // auto-detection or a rejected manual override is not missed.
  bool IsDecayTimingException(const mitk::Exception& e)
  {
    using namespace mitk;
    return nullptr != dynamic_cast<const AmbiguousDecayTimingException*>(&e)
        || nullptr != dynamic_cast<const InvalidDecayTimeMapException*>(&e)
        || nullptr != dynamic_cast<const ConflictingDecayTimeOverrideException*>(&e);
  }

  // Warning colour of the active BlueBerry theme (font.warning in the light /
  // dark QSS). The diagnostics QTextEdit inherits it through its document
  // stylesheet; a QLabel cannot, so read the value directly and re-read on
  // each call so a runtime theme switch is honoured. Mirrors the stylesheet
  // colour parsing in berry::QtStyleManager.
  QString ThemeWarningColor()
  {
    static const QRegularExpression re(
      QStringLiteral("font\\.warning\\s*\\{[^}]*?color\\s*:\\s*([^;}]+)"),
      QRegularExpression::CaseInsensitiveOption);
    const auto match = re.match(qApp->styleSheet());
    return match.hasMatch() ? match.captured(1).trimmed() : QStringLiteral("#ff5c33");
  }

  // Map a typed SUV exception to UI-friendly per-category guidance.
  // Catching at the dedicated leaf type lets each category render its
  // own actionable hint rather than collapsing every failure to the
  // root description text.
  QString ExceptionToCategoryMessage(const mitk::Exception& e)
  {
    using namespace mitk;
    const QString raw = QString::fromStdString(e.GetDescription());

    if (dynamic_cast<const VendorEmpiricalDecayFallbackRefusedException*>(&e))
    {
      return QObject::tr(
        "Strict DICOM input policy refused a vendor-specific decay-timing "
        "fallback. Either uncheck 'Strict DICOM input policy', supply spec-clean "
        "timing (vendor private datetime, or AcquisitionTime == SeriesTime), or "
        "switch to 'User defined' decay time.\n\nDetails: %1").arg(raw);
    }
    if (dynamic_cast<const ImplausibleRadionuclideDoseException*>(&e))
    {
      return QObject::tr(
        "Strict DICOM input policy refused to reinterpret a sub-1e4 "
        "Radionuclide Total Dose as MBq. Either uncheck 'Strict DICOM input "
        "policy' or supply an explicit injected-activity override.\n\nDetails: %1")
        .arg(raw);
    }
    if (dynamic_cast<const AmbiguousPatientSexAdaptationRefusedException*>(&e))
    {
      return QObject::tr(
        "Strict DICOM input policy refused the IBSI mean-of-M-and-F adaptation "
        "for ambiguous patient sex. Either uncheck 'Strict DICOM input policy' "
        "or supply an explicit patient-sex override.\n\nDetails: %1").arg(raw);
    }
    if (dynamic_cast<const BenchmarkAdaptationRequiredException*>(&e))
    {
      return QObject::tr(
        "Strict DICOM input policy refused a benchmark-recommended adaptation. "
        "Either uncheck 'Strict DICOM input policy' or supply explicit "
        "overrides.\n\nDetails: %1").arg(raw);
    }
    if (dynamic_cast<const MissingPhilipsPETScaleException*>(&e))
    {
      return QObject::tr(
        "Philips CNTS input is missing both the SUV-scale and the activity-scale "
        "private factors. Re-export the data with the Philips private group "
        "intact.\n\nDetails: %1").arg(raw);
    }
    if (dynamic_cast<const UnsupportedPETUnitsException*>(&e))
    {
      return QObject::tr(
        "The input's PET Units (0054,1001) value is not supported. To force "
        "activity-concentration semantics, check 'Force activity-concentration "
        "semantics ([Bq/mL])' below.\n\nDetails: %1").arg(raw);
    }
    if (dynamic_cast<const AmbiguousDecayTimingException*>(&e))
    {
      return QObject::tr(
        "Decay timing is ambiguous. Switch the 'Time to measurement' mode to "
        "'User defined' and supply a value, or re-export the input with "
        "Radiopharmaceutical Start DateTime (0018,1078).\n\nDetails: %1").arg(raw);
    }
    if (dynamic_cast<const MissingSUVInputException*>(&e))
    {
      return QObject::tr(
        "An input required by the chosen SUV variant is missing or invalid. "
        "Fill in the highlighted field (e.g. patient height / sex for LBM, IBW, "
        "BSA).\n\nDetails: %1").arg(raw);
    }
    if (dynamic_cast<const MissingDICOMPropertyException*>(&e))
    {
      return QObject::tr(
        "A required DICOM property is missing on the input. Supply it via the "
        "override field above.\n\nDetails: %1").arg(raw);
    }
    if (dynamic_cast<const InvalidDICOMPropertyValueException*>(&e))
    {
      return QObject::tr("Invalid DICOM property value.\n\nDetails: %1").arg(raw);
    }
    if (dynamic_cast<const InvalidDecayTimeMapException*>(&e))
    {
      return QObject::tr(
        "The per-slice decay-time map shape no longer matches the image. "
        "Toggle 'Time to measurement' back to 'Auto-detect' and then re-engage "
        "'User defined (per-slice)' to seed a fresh map from the current "
        "image.\n\nDetails: %1").arg(raw);
    }
    if (dynamic_cast<const ConflictingDecayTimeOverrideException*>(&e))
    {
      return QObject::tr(
        "A uniform decay-time override and a per-slice override cannot be "
        "active at the same time. Toggle 'Time to measurement' back to "
        "'Auto-detect' and re-engage the desired mode.\n\nDetails: %1").arg(raw);
    }
    return QObject::tr("SUV configuration error.\n\nDetails: %1").arg(raw);
  }
}

void QmitkPETSUVCalculationView::SetFocus()
{
  m_Controls->btnCalculateSUV->setFocus();
}

void QmitkPETSUVCalculationView::CreateQtPartControl(QWidget* parent)
{
  m_ParentWidget = parent;
  m_Controls->setupUi(parent);

  m_Controls->decayTimeView->setAlternatingRowColors(true);
  m_Controls->decayTimeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls->decayTimeView->setRootIsDecorated(true);
  m_Controls->decayTimeView->setSortingEnabled(false);

  m_decayTimeModel = new QmitkDecayTimeMapModel(this);
  m_Controls->decayTimeView->setModel(m_decayTimeModel);
  m_Controls->decayTimeView->setItemDelegate(new QmitkDecayTimeDelegate(this));

  m_Controls->decayTimeView->header()->setStretchLastSection(false);
  m_Controls->decayTimeView->header()->resizeSection(0, 200);
  m_Controls->decayTimeView->header()->resizeSection(1, 150);
  m_Controls->decayTimeView->header()->setDefaultSectionSize(150);

  // ---- Populate variant combo ----
  for (const auto& v : kVariantEntries)
  {
    m_Controls->variantCombo->addItem(QString::fromLatin1(v.label),
                                      QVariant(static_cast<int>(v.value)));
  }

  // ---- Populate sex combo ----
  for (const auto& s : kSexEntries)
  {
    m_Controls->sexCombo->addItem(QString::fromLatin1(s.label),
                                  QVariant(static_cast<int>(s.value)));
  }

  // ---- Populate nuclide combo ----
  this->GenerateHalfLifeMap();
  for (const auto& kv : m_HalfLifeMap)
  {
    m_Controls->nuclideCombo->addItem(QString::fromStdString(kv.first),
                                      QVariant(kv.second));
  }
  m_Controls->nuclideCombo->addItem(NUCLIDE_CUSTOM_LABEL, QVariant());

  // ---- Connections ----
  connect(m_Controls->btnCalculateSUV,  &QPushButton::clicked,            this, &QmitkPETSUVCalculationView::OnCalculateSUVButtonClicked);

  connect(m_Controls->variantCombo,     QOverload<int>::of(&QComboBox::currentIndexChanged),    this, &QmitkPETSUVCalculationView::OnVariantChanged);
  connect(m_Controls->heightSpinBox,    QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this, &QmitkPETSUVCalculationView::OnPatientHeightChanged);
  connect(m_Controls->sexCombo,         QOverload<int>::of(&QComboBox::currentIndexChanged),    this, &QmitkPETSUVCalculationView::OnPatientSexChanged);

  connect(m_Controls->halflifeSpinBox,  QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this, &QmitkPETSUVCalculationView::OnHalfLifeChanged);
  connect(m_Controls->activitySpinBox,  QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this, &QmitkPETSUVCalculationView::OnInjectedActivityChanged);
  connect(m_Controls->weightSpinBox,    QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this, &QmitkPETSUVCalculationView::OnBodyWeightChanged);
  connect(m_Controls->timeSpinBox,      QOverload<int>::of(&QSpinBox::valueChanged),            this, &QmitkPETSUVCalculationView::OnTimeToMeasurementChanged);
  connect(m_Controls->nuclideCombo,     QOverload<int>::of(&QComboBox::currentIndexChanged),    this, &QmitkPETSUVCalculationView::OnNuclideComboChanged);

  connect(m_Controls->radioTimeAuto,     &QRadioButton::toggled, this, &QmitkPETSUVCalculationView::OnDecayTimeRadioToggled);
  connect(m_Controls->radioTimeUser,     &QRadioButton::toggled, this, &QmitkPETSUVCalculationView::OnDecayTimeRadioToggled);
  connect(m_Controls->radioTimePerSlice, &QRadioButton::toggled, this, &QmitkPETSUVCalculationView::OnDecayTimeRadioToggled);

  // Per-slice edits flow from the model's dataChanged signal back through
  // the view, which packages the full map and pushes it to the filter.
  connect(m_decayTimeModel, &QmitkDecayTimeMapModel::dataChanged,
          this, &QmitkPETSUVCalculationView::OnPerSliceDecayMapEdited);

  connect(m_Controls->tracerCombo,      QOverload<int>::of(&QComboBox::currentIndexChanged),    this, &QmitkPETSUVCalculationView::OnTracerIndexChanged);

  connect(m_Controls->checkForcePET,    &QCheckBox::toggled, this, &QmitkPETSUVCalculationView::OnForcePETToggled);
  connect(m_Controls->checkForceBqMl,   &QCheckBox::toggled, this, &QmitkPETSUVCalculationView::OnForceBqMlToggled);
  connect(m_Controls->checkStrictDicom, &QCheckBox::toggled, this, &QmitkPETSUVCalculationView::OnStrictDicomToggled);

  connect(m_Controls->checkPETonly,     &QCheckBox::toggled, this, &QmitkPETSUVCalculationView::OnCheckPETOnlyToggled);

  connect(m_Controls->petNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkPETSUVCalculationView::OnPETSelectionChanged);

  // ---- Initial state ----
  m_Controls->radioTimeAuto->setChecked(true);
  m_Controls->timeSpinBox->setEnabled(false);

  m_Controls->petNodeSelector->SetSelectionIsOptional(false);
  m_Controls->petNodeSelector->SetInvalidInfo("Select PET image for conversion.");
  m_Controls->petNodeSelector->SetEmptyInfo("Select PET image for conversion.");
  m_Controls->petNodeSelector->SetPopUpTitel("Select PET image.");
  m_Controls->petNodeSelector->SetPopUpHint("Select a PET image that should be the source for the SUV conversion.");

  m_Controls->petNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls->petNodeSelector->SetNodePredicate(GenerateSelectionPredicate(m_Controls->checkPETonly->isChecked()));
  m_Controls->petNodeSelector->SetAutoSelectNewNodes(true);

  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnCheckPETOnlyToggled(bool)
{
  m_Controls->petNodeSelector->SetNodePredicate(GenerateSelectionPredicate(m_Controls->checkPETonly->isChecked()));
}

mitk::Image* QmitkPETSUVCalculationView::CurrentInputImage() const
{
  auto node = m_Controls->petNodeSelector->GetSelectedNode();
  return node.IsNull() ? nullptr : dynamic_cast<mitk::Image*>(node->GetData());
}

void QmitkPETSUVCalculationView::ReconfigureFilterFromCurrentImage()
{
  m_LastConfigError.clear();
  m_LastConfigActionable.clear();
  m_DecayTimingProblem = false;
  m_Configured = false;

  auto* image = this->CurrentInputImage();
  if (nullptr == image)
  {
    m_LastConfigError = "No PET image selected.";
    return;
  }

  // Modality gate. Lives in the plugin (not the filter) because the
  // filter is modality-agnostic: it operates on any image with the
  // expected PET DICOM properties.
  if (!IsPETModality(image) && !m_Controls->checkForcePET->isChecked())
  {
    m_LastConfigError = "Selected node is not flagged as PET (Modality != 'PT').";
    m_LastConfigActionable = QObject::tr(
      "The selected node's DICOM Modality (0008,0060) is not 'PT'. If the "
      "data really is PET, check 'Force PET semantics' below.").toStdString();
    return;
  }

  // Multi-tracer gate. Detected eagerly so the UI can offer a tracer
  // combo before ConfigureFromProperties would reject the input.
  if (m_MultiTracerDetected && !m_Filter->GetTracerIndex().has_value())
  {
    m_LastConfigError = "Input contains a multi-item Radiopharmaceutical Information Sequence (0054,0016).";
    m_LastConfigActionable = QObject::tr(
      "The input lists more than one radiopharmaceutical. Pick one in the "
      "'Radiopharmaceutical (multi-tracer)' combo above.").toStdString();
    return;
  }

  try
  {
    m_Filter->ConfigureFromProperties(image);
    m_Configured = true;
  }
  catch (const mitk::Exception& e)
  {
    m_LastConfigError      = e.GetDescription();
    m_LastConfigActionable = ExceptionToCategoryMessage(e).toStdString();
    m_DecayTimingProblem   = IsDecayTimingException(e);
    MITK_ERROR << "PET SUV configuration failed: " << e.GetDescription();
  }
  catch (const std::exception& e)
  {
    m_LastConfigError      = e.what();
    m_LastConfigActionable = QObject::tr("SUV configuration error.\n\nDetails: %1")
                               .arg(QString::fromUtf8(e.what())).toStdString();
    MITK_ERROR << "PET SUV configuration failed: " << e.what();
  }
  catch (...)
  {
    m_LastConfigError      = "Unknown error during SUV configuration.";
    m_LastConfigActionable = m_LastConfigError;
    MITK_ERROR << "PET SUV configuration failed with an unknown exception.";
  }
}

void QmitkPETSUVCalculationView::DetectAndPopulateTracers()
{
  m_MultiTracerDetected = false;
  {
    const QSignalBlocker block(m_Controls->tracerCombo);
    m_Controls->tracerCombo->clear();
  }

  auto* image = this->CurrentInputImage();
  if (nullptr == image) return;

  try
  {
    const auto rpis = mitk::GetRadiopharmaceuticalInfos(image);
    if (rpis.size() > 1U)
    {
      m_MultiTracerDetected = true;
      const QSignalBlocker block(m_Controls->tracerCombo);
      for (std::size_t i = 0; i < rpis.size(); ++i)
      {
        const QString label = QStringLiteral("[%1] %2")
          .arg(i)
          .arg(rpis[i].name.empty() ? QStringLiteral("<unnamed>")
                                    : QString::fromStdString(rpis[i].name));
        m_Controls->tracerCombo->addItem(label, QVariant(static_cast<int>(i)));
      }
      m_Controls->tracerCombo->setCurrentIndex(-1);
    }
  }
  catch (const mitk::Exception&)
  {
    // Best-effort: if even reading the RPI throws (e.g. strict-DICOM dose
    // refusal at this stage), let ConfigureFromProperties surface it.
  }
}

void QmitkPETSUVCalculationView::RefreshNuclideComboFromImage()
{
  const QSignalBlocker block(m_Controls->nuclideCombo);

  auto* image = this->CurrentInputImage();
  if (nullptr == image)
  {
    m_Controls->nuclideCombo->setCurrentIndex(m_Controls->nuclideCombo->findText(NUCLIDE_CUSTOM_LABEL));
    return;
  }

  try
  {
    const auto rpis = mitk::GetRadiopharmaceuticalInfos(image);
    if (!rpis.empty())
    {
      const int idx = m_Controls->nuclideCombo->findText(QString::fromStdString(rpis.front().name));
      if (idx >= 0)
      {
        m_Controls->nuclideCombo->setCurrentIndex(idx);
        return;
      }
    }
  }
  catch (const mitk::Exception&)
  {
    // ignore; fall through to (custom)
  }
  m_Controls->nuclideCombo->setCurrentIndex(m_Controls->nuclideCombo->findText(NUCLIDE_CUSTOM_LABEL));
}

void QmitkPETSUVCalculationView::OnVariantChanged(int idx)
{
  if (idx < 0 || idx >= static_cast<int>(kVariantEntries.size())) return;
  m_Filter->SetTargetVariant(kVariantEntries[idx].value);
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnPatientHeightChanged(double valueCm)
{
  // The spinbox minimum (0) is the "unknown, derive from DICOM" sentinel,
  // not a real 0 cm override; map it back to Clear so the DICOM value is
  // used rather than installing a 0 the filter would reject.
  if (valueCm <= 0.0)
    m_Filter->ClearPatientHeightInCm();
  else
    m_Filter->SetPatientHeightInCm(valueCm);
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnPatientSexChanged(int idx)
{
  if (idx < 0 || idx >= static_cast<int>(kSexEntries.size())) return;
  m_Filter->SetPatientSex(kSexEntries[idx].value);
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnInjectedActivityChanged(double value)
{
  // widget is [kBq], filter override is [Bq]
  m_Filter->SetInjectedActivityInBq(value * 1000.0);
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnBodyWeightChanged(double value)
{
  // widget is [kg], filter override is [g]. The spinbox minimum (0) is the
  // "unknown, derive from DICOM" sentinel, not a real 0 kg override; map it
  // back to Clear so the DICOM value is used rather than installing a 0 the
  // filter would reject.
  if (value <= 0.0)
    m_Filter->ClearPatientWeightInGram();
  else
    m_Filter->SetPatientWeightInGram(value * 1000.0);
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnHalfLifeChanged(double value)
{
  // widget is [min], filter override is [s]
  m_Filter->SetHalfLifeInSec(value * 60.0);
  // Manually edited half-life no longer matches a named nuclide; flip
  // the combo to (custom) without emitting another change signal.
  {
    const QSignalBlocker block(m_Controls->nuclideCombo);
    const int customIdx = m_Controls->nuclideCombo->findText(NUCLIDE_CUSTOM_LABEL);
    if (customIdx >= 0) m_Controls->nuclideCombo->setCurrentIndex(customIdx);
  }
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnNuclideComboChanged(int idx)
{
  if (idx < 0) return;
  const QVariant data = m_Controls->nuclideCombo->itemData(idx);
  if (!data.isValid())
  {
    // "(custom)" selection: leave existing half-life override in place.
    return;
  }
  const double halfLifeSec = data.toDouble();
  m_Filter->SetHalfLifeInSec(halfLifeSec);
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnTimeToMeasurementChanged(int value)
{
  if (m_Controls->radioTimeUser->isChecked())
  {
    // The uniform override is intentionally an integer-minute control: it
    // is the quick "single decay time for the whole image" affordance. Sub-
    // minute / per-slice precision is offered by the per-slice tree editor,
    // not here. Minutes are converted to the filter's [s] contract.
    m_Filter->SetDecayTimeOverrideInSec(static_cast<double>(value) * 60.0);
    this->ReconfigureFilterFromCurrentImage();
  }
  this->UpdateWidgets();
}

mitk::DecayTimeMapType QmitkPETSUVCalculationView::SeedPerSliceMapFromEffective() const
{
  if (!m_Configured) return {};
  try
  {
    return m_Filter->GetEffectiveDecayCorrection().decayTimes;
  }
  catch (const mitk::Exception&)
  {
    return {};
  }
}

void QmitkPETSUVCalculationView::PushPerSliceMapToFilter()
{
  // SetDecayTimeOverrideMap throws if the uniform override is still set;
  // the radio handler clears it eagerly, but be defensive about ordering.
  m_Filter->ClearDecayTimeOverrideInSec();
  try
  {
    m_Filter->SetDecayTimeOverrideMap(m_decayTimeModel->GetDecayTimeMap());
  }
  catch (const mitk::Exception& e)
  {
    m_LastConfigError      = e.GetDescription();
    m_LastConfigActionable = ExceptionToCategoryMessage(e).toStdString();
    m_Configured           = false;
    m_DecayTimingProblem   = IsDecayTimingException(e);
    MITK_ERROR << "Failed to set per-slice decay-time override map: "
               << e.GetDescription();
    this->UpdateWidgets();
    return;
  }
  this->ReconfigureFilterFromCurrentImage();
}

void QmitkPETSUVCalculationView::OnPerSliceDecayMapEdited(
  const QModelIndex&, const QModelIndex&, const QList<int>& /*roles*/)
{
  // Edits only ever reach here in PerSlice mode (the model gates editing
  // by mode). Push the now-mutated full map to the filter; reconfigure
  // surfaces any validation failure (e.g. shape mismatch after a future
  // model bug) via the diagnostics widget.
  //
  // This slot fires synchronously from the model's dataChanged emission
  // while the delegate is still committing the edit. PushPerSliceMapToFilter
  // and UpdateWidgets reset the tree model, which must not happen while a
  // cell edit is mid-commit, so the work is deferred to the event loop.
  if (m_Controls->radioTimePerSlice->isChecked())
  {
    QMetaObject::invokeMethod(
      this,
      [this]()
      {
        this->PushPerSliceMapToFilter();
        this->UpdateWidgets();
      },
      Qt::QueuedConnection);
  }
}

void QmitkPETSUVCalculationView::OnDecayTimeRadioToggled()
{
  // Qt fires `toggled` once per radio button on a group change (one false
  // and one true). Act only on the now-checked transition so the filter
  // is not reconfigured twice per user click.
  auto* sender = qobject_cast<QRadioButton*>(this->sender());
  if (nullptr != sender && !sender->isChecked()) return;

  if (m_Controls->radioTimePerSlice->isChecked())
  {
    // Seed the override map from the currently effective decay info so
    // the user starts editing from real DICOM-derived values rather than
    // zeros. Requires the filter to be configured; if it is not, the
    // seed is empty and Configure will reject it -- which is the right
    // outcome (the user gets actionable feedback in the diagnostics).
    const auto seed = this->SeedPerSliceMapFromEffective();
    m_Filter->ClearDecayTimeOverrideInSec();
    m_decayTimeModel->SetDecayTimeMap(seed);
    this->PushPerSliceMapToFilter();
  }
  else if (m_Controls->radioTimeUser->isChecked())
  {
    m_Filter->ClearDecayTimeOverrideMap();
    m_Filter->SetDecayTimeOverrideInSec(
      static_cast<double>(m_Controls->timeSpinBox->value()) * 60.0);
    this->ReconfigureFilterFromCurrentImage();
  }
  else  // radioTimeAuto
  {
    m_Filter->ClearDecayTimeOverrideInSec();
    m_Filter->ClearDecayTimeOverrideMap();
    this->ReconfigureFilterFromCurrentImage();
  }
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnTracerIndexChanged(int idx)
{
  if (idx < 0)
  {
    m_Filter->ClearTracerIndex();
  }
  else
  {
    const QVariant data = m_Controls->tracerCombo->itemData(idx);
    if (data.isValid())
    {
      m_Filter->SetTracerIndex(data.toInt());
    }
  }
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnForcePETToggled(bool)
{
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnForceBqMlToggled(bool checked)
{
  if (checked)
  {
    mitk::SUVInputModel forced;
    forced.semantics     = mitk::SUVPixelSemantics::ActivityConcentration;
    forced.activityScale = 1.0;
    m_Filter->SetInputModelOverride(forced);
  }
  else
  {
    m_Filter->ClearInputModelOverride();
  }
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnStrictDicomToggled(bool checked)
{
  m_Filter->SetDICOMReadPolicy(checked ? mitk::DICOMReadPolicy::Strict
                                       : mitk::DICOMReadPolicy::Lenient);
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnCalculateSUVButtonClicked()
{
  auto inputNode = m_Controls->petNodeSelector->GetSelectedNode();
  if (inputNode.IsNull() || !m_Configured) return;

  MITK_INFO << "Calculating SUV via mitk::SUVImageFilter for node '" << inputNode->GetName() << "'.";

  try
  {
    // Update() runs synchronously on the GUI thread; show a busy cursor for
    // its duration so a multi-timestep whole-body PET does not look frozen.
    // Scoped tightly around the heavy call and restored on success or throw.
    QApplication::setOverrideCursor(Qt::WaitCursor);
    try
    {
      m_Filter->Update();
    }
    catch (...)
    {
      QApplication::restoreOverrideCursor();
      throw;
    }
    QApplication::restoreOverrideCursor();

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
    MITK_ERROR << "SUV Update() failed: " << e.GetDescription();
  }
  catch (const std::exception& e)
  {
    QMessageBox::critical(m_ParentWidget, tr("SUV computation failed"),
      QString::fromUtf8(e.what()));
    MITK_ERROR << "SUV Update() failed: " << e.what();
  }
  catch (...)
  {
    QMessageBox::critical(m_ParentWidget, tr("SUV computation failed"),
      tr("Unknown error during SUV computation."));
    MITK_ERROR << "SUV Update() failed with an unknown exception.";
  }
}

void QmitkPETSUVCalculationView::UpdateWidgets()
{
  // Programmatic writes to widgets must not re-enter the user-edit slots.
  const QSignalBlocker blockVariant(m_Controls->variantCombo);
  const QSignalBlocker blockHeight (m_Controls->heightSpinBox);
  const QSignalBlocker blockSex    (m_Controls->sexCombo);
  const QSignalBlocker blockA(m_Controls->activitySpinBox);
  const QSignalBlocker blockW(m_Controls->weightSpinBox);
  const QSignalBlocker blockH(m_Controls->halflifeSpinBox);
  const QSignalBlocker blockT(m_Controls->timeSpinBox);

  // Current variant gates variant-input visibility.
  const auto variantIdx = m_Controls->variantCombo->currentIndex();
  const auto variant    = (variantIdx >= 0 && variantIdx < static_cast<int>(kVariantEntries.size()))
                            ? kVariantEntries[variantIdx].value
                            : mitk::SUVVariant::BW;

  // The chosen target variant's requirements drive the variant-input
  // affordances. Additionally, if the input has been detected as a
  // pre-normalized SUV, the cross-variant renormalization needs the
  // source variant's scale numerator -- which itself can depend on
  // height / sex even when the target does not. Surfacing those
  // affordances here lets the user override the corresponding patient
  // input even after the source-variant sex / height gate refused.
  bool needsHeight = mitk::VariantRequiresPatientHeight(variant);
  bool needsSex    = mitk::VariantRequiresPatientSex(variant);
  if (const auto detected = m_Filter->GetDetectedInputModel();
      detected.has_value() &&
      mitk::SUVPixelSemantics::PrenormalizedSUV == detected->semantics)
  {
    needsHeight = needsHeight || mitk::VariantRequiresPatientHeight(detected->sourceVariant);
    needsSex    = needsSex    || mitk::VariantRequiresPatientSex(detected->sourceVariant);
  }
  // groupPatient is always shown (weight is always required); only the
  // height and sex rows toggle with the variant's requirements.
  m_Controls->labelHeight     ->setVisible(needsHeight);
  m_Controls->heightSpinBox   ->setVisible(needsHeight);
  m_Controls->labelHeightUnit ->setVisible(needsHeight);
  m_Controls->labelSex        ->setVisible(needsSex);
  m_Controls->sexCombo        ->setVisible(needsSex);

  // Acquisition fields: prefer the filter's resolved values when Configure
  // succeeded. When it failed (e.g. Strict refused a borderline DICOM
  // adaptation on one field), fall back to user overrides where set, and
  // otherwise read directly from DICOM via the standalone helpers. The
  // failure path keeps the unrelated fields visible to the user instead
  // of blanking the whole panel because of one strict refusal.
  //
  // Pre-normalized SUV input (GML / CM2ML / Philips CNTS prenorm) does
  // not consume activity / half-life / decay correction; detection is
  // sticky across configure failures (GetDetectedInputModel), so the
  // gating below stays correct even on the failure path.
  auto effectiveOrDetectedSemantics = [&]() -> std::optional<mitk::SUVPixelSemantics> {
    if (m_Configured) return m_Filter->GetEffectiveInputModel().semantics;
    if (const auto d = m_Filter->GetDetectedInputModel(); d.has_value())
      return d->semantics;
    return std::nullopt;
  };
  const auto semOpt = effectiveOrDetectedSemantics();
  const bool needsActivity =
    semOpt.has_value() &&
    mitk::SUVPixelSemantics::ActivityConcentration == semOpt.value();

  auto* image = this->CurrentInputImage();

  // Weight: always shown.
  if (m_Configured)
  {
    m_Controls->weightSpinBox->setValue(m_Filter->GetEffectivePatientWeightInGram() / 1000.0);
  }
  else if (auto ov = m_Filter->GetPatientWeightInGram(); ov.has_value())
  {
    m_Controls->weightSpinBox->setValue(ov.value() / 1000.0);
  }
  else if (nullptr != image)
  {
    try { m_Controls->weightSpinBox->setValue(mitk::GetPatientsWeight(image)); }
    catch (const mitk::Exception&) { m_Controls->weightSpinBox->setValue(0.0); }
  }
  else
  {
    m_Controls->weightSpinBox->setValue(0.0);
  }

  // Activity + half-life: only meaningful on the activity-to-SUV path.
  // When Configure failed but the input is activity-typed, try the
  // helpers under Lenient policy so the displayed autodetect is not
  // re-blocked by the same Strict refusal that broke Configure.
  auto populateActivityAndHalfLife = [&]() {
    if (m_Configured && needsActivity)
    {
      m_Controls->activitySpinBox->setValue(m_Filter->GetEffectiveInjectedActivityInBq() / 1000.0);
      m_Controls->halflifeSpinBox->setValue(m_Filter->GetEffectiveHalfLifeInSec() / 60.0);
      return;
    }
    if (m_Configured)
    {
      // Pre-normalized SUV: activity / half-life are not consumed.
      m_Controls->activitySpinBox->setValue(0.0);
      m_Controls->halflifeSpinBox->setValue(0.0);
      return;
    }

    const auto actOverride = m_Filter->GetInjectedActivityInBq();
    const auto hlOverride  = m_Filter->GetHalfLifeInSec();
    double activityBq = std::numeric_limits<double>::quiet_NaN();
    double halfLifeS  = std::numeric_limits<double>::quiet_NaN();
    if (nullptr != image)
    {
      try
      {
        const auto rpis = mitk::GetRadiopharmaceuticalInfos(image, mitk::DICOMReadPolicy::Lenient);
        const int idx   = m_Filter->GetTracerIndex().value_or(0);
        if (idx >= 0 && static_cast<std::size_t>(idx) < rpis.size())
        {
          activityBq = rpis[idx].totalDoseBq;
          halfLifeS  = rpis[idx].halfLifeSeconds;
        }
      }
      catch (const mitk::Exception&) { /* ignore; widgets fall back to 0 below */ }
    }
    const double effActivityBq = actOverride.value_or(activityBq);
    const double effHalfLifeS  = hlOverride.value_or(halfLifeS);
    m_Controls->activitySpinBox->setValue(std::isfinite(effActivityBq) ? effActivityBq / 1000.0 : 0.0);
    m_Controls->halflifeSpinBox->setValue(std::isfinite(effHalfLifeS)  ? effHalfLifeS  / 60.0   : 0.0);
  };
  populateActivityAndHalfLife();

  // Patient height: only shown for variants that consume it.
  if (needsHeight)
  {
    if (m_Configured)
    {
      try { m_Controls->heightSpinBox->setValue(m_Filter->GetEffectivePatientHeightInCm()); }
      catch (const mitk::Exception&) { /* unresolved */ }
    }
    else if (auto ov = m_Filter->GetPatientHeightInCm(); ov.has_value())
    {
      m_Controls->heightSpinBox->setValue(ov.value());
    }
    else if (nullptr != image)
    {
      try { m_Controls->heightSpinBox->setValue(mitk::GetPatientsHeight(image) * 100.0); }
      catch (const mitk::Exception&) { /* leave at previous value */ }
    }
  }

  // Patient sex: closed-set combo. Pick override > effective > DICOM.
  if (needsSex)
  {
    std::optional<mitk::Sex> sx;
    if (auto ov = m_Filter->GetPatientSex(); ov.has_value())
    {
      sx = ov;
    }
    else if (m_Configured)
    {
      try { sx = m_Filter->GetEffectivePatientSex(); }
      catch (const mitk::Exception&) { /* unresolved */ }
    }
    else if (nullptr != image)
    {
      try { sx = mitk::GetPatientsSex(image); }
      catch (const mitk::Exception&) { /* leave at previous value */ }
    }
    if (sx.has_value())
    {
      for (std::size_t i = 0; i < kSexEntries.size(); ++i)
      {
        if (kSexEntries[i].value == sx.value())
        {
          m_Controls->sexCombo->setCurrentIndex(static_cast<int>(i));
          break;
        }
      }
    }
  }

  // Activity / tracer and decay timing apply only to the activity-to-SUV
  // path. Pre-normalized SUV input consumes neither, so both groups are
  // hidden rather than merely disabled, keeping the panel focused on the
  // inputs the selected image actually needs.
  m_Controls->groupActivity->setVisible(needsActivity);
  m_Controls->groupTime->setVisible(needsActivity);

  // The tracer row is relevant only for a multi-radiopharmaceutical input;
  // its parent group already hides it off the activity path.
  m_Controls->labelTracer->setVisible(m_MultiTracerDetected);
  m_Controls->tracerCombo->setVisible(m_MultiTracerDetected);

  const bool perSliceMode = m_Controls->radioTimePerSlice->isChecked();
  m_Controls->timeSpinBox->setEnabled(m_Controls->radioTimeUser->isChecked());
  if (auto override = m_Filter->GetDecayTimeOverrideInSec())
  {
    m_Controls->timeSpinBox->setValue(static_cast<int>(*override / 60.0));
  }

  // Time-group status line. Normally shows the decay-correction strategy
  // behind the values in decayTimeView; when a decay-timing problem blocked
  // configuration it instead surfaces the actionable warning in the theme's
  // warning colour, because the compact diagnostics box is easy to miss.
  // GetEffectiveDecayCorrection would throw on a pre-normalized input, but
  // groupTime is hidden on that path.
  if (m_DecayTimingProblem && !m_LastConfigActionable.empty())
  {
    const QString actionable = QString::fromStdString(m_LastConfigActionable);
    m_Controls->strategyLabel->setStyleSheet(QStringLiteral("color: %1;").arg(ThemeWarningColor()));
    m_Controls->strategyLabel->setText(actionable.section(QStringLiteral("\n\nDetails:"), 0, 0).trimmed());
  }
  else if (m_Configured && needsActivity)
  {
    const auto info = m_Filter->GetEffectiveDecayCorrection();
    m_Controls->strategyLabel->setStyleSheet(QString());
    m_Controls->strategyLabel->setText(tr("Strategy: %1").arg(StrategyToString(info.strategy)));
  }
  else
  {
    m_Controls->strategyLabel->setStyleSheet(QString());
    m_Controls->strategyLabel->clear();
  }

  // Diagnostics widget: auto-detected summary on top, then either an
  // actionable error or an OK confirmation. Both blocks coexist so the
  // user always sees what the filter deduced from the input, even when
  // the configuration ultimately failed.
  //
  // Rendered as HTML so the actionable error portion can carry the
  // shared `font.warning` styling (red/bold; see the BlueBerry QSS).
  auto toHtml = [](const QString& plain) {
    return plain.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br>"));
  };
  QString diagHtml = toHtml(this->BuildDetectedInfoText());
  if (!m_LastConfigActionable.empty())
  {
    if (!diagHtml.isEmpty()) diagHtml.append(QStringLiteral("<br><br>"));
    diagHtml.append(QStringLiteral("<font class=\"warning\">"))
            .append(toHtml(QString::fromStdString(m_LastConfigActionable)))
            .append(QStringLiteral("</font>"));
  }
  else if (m_Configured)
  {
    if (!diagHtml.isEmpty()) diagHtml.append(QStringLiteral("<br><br>"));
    diagHtml.append(tr("Configuration OK."));
  }
  // QTextEdit's rich-text rendering picks up the surrounding app
  // stylesheet only via its QTextDocument's default style sheet.
  // Refreshed on every update so runtime theme switches are honoured.
  // Same pattern as QmitkNodeSelectionButton::paintEvent.
  if (auto* doc = m_Controls->diagnosticsWidget->document())
  {
    doc->setDefaultStyleSheet(qApp->styleSheet());
  }
  m_Controls->diagnosticsWidget->setHtml(diagHtml);

  // Decay-time tree model refresh. Pre-normalized SUV inputs have no
  // decay-correction info to show. SetDecayTimeMap / SetMode reset the
  // model, which the tree view relies on to repaint; edit-driven refreshes
  // are deferred to the event loop by OnPerSliceDecayMapEdited so this reset
  // never runs while a cell edit is mid-commit.
  const bool uniformMode = m_Controls->radioTimeUser->isChecked();
  {
    if (m_Configured && needsActivity)
    {
      const auto info = m_Filter->GetEffectiveDecayCorrection();
      if (uniformMode)
      {
        const double overrideSec = m_Filter->GetDecayTimeOverrideInSec().value_or(0.0);
        mitk::DecayTimeMapType userMap;
        for (const auto& timePos : info.decayTimes) userMap[timePos.first][0] = overrideSec;
        m_decayTimeModel->SetDecayTimeMap(userMap);
      }
      else
      {
        // Auto and PerSlice both render info.decayTimes hierarchically;
        // in PerSlice mode the cells become editable via flags().
        m_decayTimeModel->SetDecayTimeMap(info.decayTimes);
      }
    }
    else
    {
      m_decayTimeModel->SetDecayTimeMap({});
    }
    const auto modelMode = uniformMode    ? QmitkDecayTimeMapModel::Mode::UserDefined
                         : perSliceMode   ? QmitkDecayTimeMapModel::Mode::PerSlice
                                          : QmitkDecayTimeMapModel::Mode::Auto;
    m_decayTimeModel->SetMode(modelMode);
  }

  // Calculate button gating. The filter is the authority on whether the
  // input is processable: m_Configured == true means ConfigureFromProperties
  // accepted the input (including legitimate GML / CM2ML / Philips-CNTS via
  // the pre-normalized path). The 'Force activity-concentration' checkbox
  // is the user-driven escape that converts an UnsupportedPETUnitsException
  // into a successful configuration.
  const bool valid = m_Configured && m_LastConfigError.empty()
                     && (nullptr != this->CurrentInputImage());
  m_Controls->btnCalculateSUV->setEnabled(valid);
}

void QmitkPETSUVCalculationView::OnPETSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  // The selector is single-select; `nodes` holds at most one item.
  auto* selectedImage = nodes.isEmpty() ? nullptr
                                        : dynamic_cast<mitk::Image*>(nodes.front()->GetData());
  // Replace the filter rather than clearing every slot individually so
  // overrides from a previous selection cannot bleed into the new one.
  m_Filter = mitk::SUVImageFilter::New();
  m_Filter->SetDICOMReadPolicy(m_Controls->checkStrictDicom->isChecked()
                               ? mitk::DICOMReadPolicy::Strict
                               : mitk::DICOMReadPolicy::Lenient);

  // Honour the persistent force-units checkbox across selections.
  if (m_Controls->checkForceBqMl->isChecked())
  {
    mitk::SUVInputModel forced;
    forced.semantics     = mitk::SUVPixelSemantics::ActivityConcentration;
    forced.activityScale = 1.0;
    m_Filter->SetInputModelOverride(forced);
  }

  // Honour the current variant combo state.
  const int variantIdx = m_Controls->variantCombo->currentIndex();
  if (variantIdx >= 0 && variantIdx < static_cast<int>(kVariantEntries.size()))
  {
    m_Filter->SetTargetVariant(kVariantEntries[variantIdx].value);
  }

  m_LastConfigError.clear();
  m_LastConfigActionable.clear();
  m_Configured = false;
  m_MultiTracerDetected = false;
  m_decayTimeModel->SetDecayTimeMap({});

  // Force the decay-time mode back to Auto whenever the input changes so
  // a stale PerSlice / UserDefined selection from the previous node cannot
  // leave the radio state and the (now-empty) filter overrides out of sync.
  {
    const QSignalBlocker block(m_Controls->radioTimeAuto);
    m_Controls->radioTimeAuto->setChecked(true);
  }

  if (nullptr == selectedImage)
  {
    this->UpdateWidgets();
    return;
  }

  m_Filter->SetInput(selectedImage);

  this->DetectAndPopulateTracers();
  this->RefreshNuclideComboFromImage();
  this->ReconfigureFilterFromCurrentImage();
  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::GenerateHalfLifeMap()
{
  m_HalfLifeMap.clear();
  m_HalfLifeMap.insert({mitk::HALFLIFECONSTANTS::NAME_18F,  mitk::HALFLIFECONSTANTS::VALUE_18F});
  m_HalfLifeMap.insert({mitk::HALFLIFECONSTANTS::NAME_68Ga, mitk::HALFLIFECONSTANTS::VALUE_68Ga});
  m_HalfLifeMap.insert({mitk::HALFLIFECONSTANTS::NAME_11C,  mitk::HALFLIFECONSTANTS::VALUE_11C});
  m_HalfLifeMap.insert({mitk::HALFLIFECONSTANTS::NAME_15O,  mitk::HALFLIFECONSTANTS::VALUE_15O});
}

QmitkPETSUVCalculationView::QmitkPETSUVCalculationView()
  : m_Controls(std::make_unique<Ui::QmitkPETSUVCalculationViewControls>()),
    m_Filter(mitk::SUVImageFilter::New()),
    m_ParentWidget(nullptr)
{
  this->GenerateHalfLifeMap();
}

// Out-of-line (not =default in header): ~unique_ptr needs the
// complete Ui type, visible only in this translation unit.
QmitkPETSUVCalculationView::~QmitkPETSUVCalculationView() = default;

QString QmitkPETSUVCalculationView::BuildDetectedInfoText() const
{
  auto* image = this->CurrentInputImage();
  if (nullptr == image) return {};

  QStringList lines;
  lines << tr("[Auto-detected]");

  const std::string manufacturer = GetBaseDataPropValueAsString(image, mitk::DICOMTagPath(0x0008, 0x0070));
  if (!manufacturer.empty())
  {
    lines << tr("Vendor: %1").arg(QString::fromStdString(manufacturer));
  }

  const std::string units = GetBaseDataPropValueAsString(image, mitk::DICOMTagPath(0x0054, 0x1001));
  if (!units.empty())
  {
    lines << tr("DICOM Units (0054,1001): %1%2")
                 .arg(QString::fromStdString(units))
                 .arg(m_Controls->checkForceBqMl->isChecked()
                      ? tr(" -- overridden to Bq/mL")
                      : QString());
  }

  if (m_Configured)
  {
    const auto& model = m_Filter->GetEffectiveInputModel();
    lines << tr("Pixel semantics: %1").arg(SemanticsToString(model.semantics));
    if (mitk::SUVPixelSemantics::ActivityConcentration == model.semantics)
    {
      if (model.activityScale != 1.0)
      {
        lines << tr("Activity pre-scale: %1").arg(model.activityScale, 0, 'g', 6);
      }

      try
      {
        const double doseMbq = m_Filter->GetEffectiveInjectedActivityInBq() / 1.0e6;
        lines << tr("Injected dose: %1 MBq").arg(doseMbq, 0, 'f', 3);
      }
      catch (const std::exception&) { /* unresolved */ }

      try
      {
        const double halfLifeMin = m_Filter->GetEffectiveHalfLifeInSec() / 60.0;
        lines << tr("Half-life: %1 min").arg(halfLifeMin, 0, 'f', 3);
      }
      catch (const std::exception&) { /* unresolved */ }

      try
      {
        const auto& decay = m_Filter->GetEffectiveDecayCorrection();
        lines << tr("Decay strategy: %1").arg(StrategyToString(decay.strategy));
      }
      catch (const std::exception&) { /* unresolved */ }
    }
    else
    {
      lines << tr("Source variant: %1").arg(VariantToString(model.sourceVariant));
      if (model.prenormScale != 1.0)
      {
        lines << tr("Prenorm scale: %1").arg(model.prenormScale, 0, 'g', 6);
      }
      lines << tr("Decay correction: not applied (input is already SUV)");
    }

    if (m_MultiTracerDetected)
    {
      const auto idx = m_Filter->GetTracerIndex();
      lines << tr("Tracer index: %1")
                   .arg(idx.has_value() ? QString::number(*idx) : tr("not selected"));
    }
  }
  else if (!m_LastConfigError.empty())
  {
    lines << tr("(Configuration failed; auto-detected fields above are partial.)");
  }

  if (m_Controls->checkForcePET->isChecked())
  {
    lines << tr("Override: forcing PET semantics (Modality check bypassed)");
  }
  if (m_Controls->checkStrictDicom->isChecked())
  {
    lines << tr("Policy: Strict DICOM input");
  }
  else
  {
    lines << tr("Policy: Lenient (IBSI-SUV adaptations allowed)");
  }

  return lines.join(QStringLiteral("\n"));
}


// =============================================================================
// QmitkDecayTimeMapModel
// =============================================================================

QmitkDecayTimeMapModel::QmitkDecayTimeMapModel(QObject* parent)
  : QAbstractItemModel(parent)
  , m_Mode(Mode::Auto)
{
}

namespace
{
  // Hierarchical layout (timestep -> slices) applies to both Auto and PerSlice.
  // The (legacy) UserDefined mode keeps its flat layout.
  bool IsHierarchicalMode(QmitkDecayTimeMapModel::Mode m)
  {
    return m == QmitkDecayTimeMapModel::Mode::Auto ||
           m == QmitkDecayTimeMapModel::Mode::PerSlice;
  }
}

QModelIndex QmitkDecayTimeMapModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) return QModelIndex();

  if (!hasSingleTimeStep() && IsHierarchicalMode(m_Mode))
  {
    if (!parent.isValid())
    {
      return createIndex(row, column);
    }
    return createIndex(row, column, static_cast<quintptr>(parent.row() + 1));
  }
  return createIndex(row, column);
}

QModelIndex QmitkDecayTimeMapModel::parent(const QModelIndex& child) const
{
  if (!child.isValid()) return QModelIndex();

  if (IsHierarchicalMode(m_Mode) && !hasSingleTimeStep())
  {
    if (child.internalPointer() != nullptr)
    {
      const auto timeStepRow = static_cast<int>(child.internalId() - 1);
      return createIndex(timeStepRow, 0);
    }
  }
  return QModelIndex();
}

int QmitkDecayTimeMapModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
  {
    if (IsHierarchicalMode(m_Mode) && hasSingleTimeStep())
      return static_cast<int>(m_DecayTimeMap.begin()->second.size());
    return static_cast<int>(m_DecayTimeMap.size());
  }
  if (IsHierarchicalMode(m_Mode) && !hasSingleTimeStep())
  {
    auto timeStep = GetTimeStep(parent);
    if (timeStep && m_DecayTimeMap.count(*timeStep))
      return static_cast<int>(m_DecayTimeMap.at(*timeStep).size());
  }
  return 0;
}

int QmitkDecayTimeMapModel::columnCount(const QModelIndex&) const
{
  return 2;
}

QVariant QmitkDecayTimeMapModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    const int col = index.column();
    auto timeStepOpt = GetTimeStep(index);
    auto sliceOpt    = GetSliceIndex(index);

    if (m_Mode == Mode::UserDefined)
    {
      if (!timeStepOpt) return QVariant();
      if (col == 0)    return tr("Time Step %1").arg(*timeStepOpt);

      const auto decayTime = m_DecayTimeMap.at(*timeStepOpt).at(*sliceOpt);
      return QVariant(decayTime);
    }
    else  // Auto or PerSlice (both hierarchical)
    {
      if (!index.parent().isValid() && !this->hasSingleTimeStep())
      {
        if (col == 0 && timeStepOpt)
          return tr("Time Step %1").arg(*timeStepOpt);
      }
      else if (timeStepOpt && sliceOpt)
      {
        const auto decayTime = m_DecayTimeMap.at(*timeStepOpt).at(*sliceOpt);
        if (col == 0) return tr("Slice %1").arg(*sliceOpt);
        return QVariant(decayTime);
      }
    }
  }
  else if (role == Qt::ToolTipRole && index.column() == 1)
  {
    return tr("Decay time in seconds");
  }
  return {};
}

QVariant QmitkDecayTimeMapModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch (section)
    {
      case 0: return tr("Item");
      case 1: return tr("Decay Time [s]");
      default: return QVariant();
    }
  }
  return QVariant();
}

Qt::ItemFlags QmitkDecayTimeMapModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  // Only PerSlice mode is editable. UserDefined (uniform) is driven by the
  // spinbox; Auto is read-only.
  if (m_Mode == Mode::PerSlice && index.column() == 1)
  {
    // In hierarchical mode the column-1 child rows hold values; parent
    // (timestep) rows are headers and are not editable.
    if (hasSingleTimeStep() || index.parent().isValid())
      flags |= Qt::ItemIsEditable;
  }
  return flags;
}

bool QmitkDecayTimeMapModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role != Qt::EditRole || !index.isValid() || index.column() != 1) return false;
  if (m_Mode != Mode::PerSlice) return false;

  bool ok = false;
  const double newDecay = value.toDouble(&ok);
  if (!ok) return false;

  // GetTimeStep walks to a valid parent when applicable; GetSliceIndex
  // resolves to the row's slice both in the single-step root layout and
  // in the multi-step child layout.
  auto timeStepOpt = GetTimeStep(index);
  auto sliceOpt    = GetSliceIndex(index);
  if (!timeStepOpt || !sliceOpt) return false;

  m_DecayTimeMap[*timeStepOpt][*sliceOpt] = newDecay;
  emit dataChanged(index, index);
  return true;
}

void QmitkDecayTimeMapModel::SetDecayTimeMap(const mitk::DecayTimeMapType& decayTimeMap)
{
  beginResetModel();
  m_DecayTimeMap = decayTimeMap;
  endResetModel();
}

const mitk::DecayTimeMapType& QmitkDecayTimeMapModel::GetDecayTimeMap() const
{
  return m_DecayTimeMap;
}

void QmitkDecayTimeMapModel::SetMode(Mode mode)
{
  if (m_Mode != mode)
  {
    beginResetModel();
    m_Mode = mode;
    endResetModel();
  }
}

QmitkDecayTimeMapModel::Mode QmitkDecayTimeMapModel::GetMode() const
{
  return m_Mode;
}

bool QmitkDecayTimeMapModel::hasSingleTimeStep() const
{
  return m_DecayTimeMap.size() == 1;
}

std::optional<mitk::TimeStepType> QmitkDecayTimeMapModel::GetTimeStep(const QModelIndex& index) const
{
  if (!index.isValid()) return std::nullopt;
  if (hasSingleTimeStep()) return 0;
  if (index.parent().isValid()) return GetTimeStep(index.parent());

  auto it = m_DecayTimeMap.begin();
  std::advance(it, index.row());
  return it->first;
}

std::optional<mitk::SlicedData::IndexValueType> QmitkDecayTimeMapModel::GetSliceIndex(const QModelIndex& index) const
{
  if (!index.isValid()) return std::nullopt;
  if (m_Mode == Mode::UserDefined) return 0;

  if (hasSingleTimeStep())
  {
    const auto& slices = m_DecayTimeMap.begin()->second;
    auto it = slices.begin();
    std::advance(it, index.row());
    return it->first;
  }
  if (index.parent().isValid())
  {
    auto tsOpt = GetTimeStep(index);
    if (!tsOpt || !m_DecayTimeMap.count(*tsOpt)) return std::nullopt;
    const auto& slices = m_DecayTimeMap.at(*tsOpt);
    auto it = slices.begin();
    std::advance(it, index.row());
    return it->first;
  }
  return std::nullopt;
}


// =============================================================================
// QmitkDecayTimeDelegate
// =============================================================================

QmitkDecayTimeDelegate::QmitkDecayTimeDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* QmitkDecayTimeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
  Q_UNUSED(option)
  if (index.column() != 1) return nullptr;

  auto* spinBox = new QDoubleSpinBox(parent);
  spinBox->setRange(MIN_DECAY_TIME, MAX_DECAY_TIME);
  spinBox->setDecimals(DECIMALS);
  spinBox->setSingleStep(SINGLE_STEP);
  spinBox->setSuffix(" [s]");
  return spinBox;
}

void QmitkDecayTimeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  auto* spinBox = qobject_cast<QDoubleSpinBox*>(editor);
  if (!spinBox) return;
  spinBox->setValue(index.model()->data(index, Qt::EditRole).toDouble());
}

void QmitkDecayTimeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                       const QModelIndex& index) const
{
  auto* spinBox = qobject_cast<QDoubleSpinBox*>(editor);
  if (!spinBox) return;
  spinBox->interpretText();
  model->setData(index, spinBox->value(), Qt::EditRole);
}

void QmitkDecayTimeDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                               const QModelIndex& index) const
{
  Q_UNUSED(index)
  editor->setGeometry(option.rect);
}
