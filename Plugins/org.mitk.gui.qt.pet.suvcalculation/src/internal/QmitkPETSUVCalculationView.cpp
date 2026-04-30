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

#include <iostream>

#include <itkIndexedUnaryFunctorImageFilter.h>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>

#include <mitkSUVCalculation.h>
#include <mitkSUVFunctorPolicy.h>
#include <mitkWorkbenchUtil.h>

#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkHalfLifeConstants.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImageTimeSelector.h>
#include <mitkSUVCalculationHelper.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateFunction.h>
#include <mitkMultiLabelPredicateHelper.h>

#include <QInputDialog>
#include <QMessageBox>

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

}

void QmitkPETSUVCalculationView::SetFocus()
{
  m_Controls->btnCalculateSUV->setFocus();
}

void QmitkPETSUVCalculationView::CreateQtPartControl(QWidget *parent)
{
  m_ParentWidget = parent;
  m_Controls->setupUi(parent);

  connect(m_Controls->btnCalculateSUV, SIGNAL(clicked()), this, SLOT(OnCalculateSUVButtonClicked()));
  connect(m_Controls->btnNuclideLookup, SIGNAL(clicked()), this, SLOT(OnNuclideLookupClicked()));
  // Tree view for decay times
  m_Controls->decayTimeView->setAlternatingRowColors(true);
  m_Controls->decayTimeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls->decayTimeView->setRootIsDecorated(true);
  m_Controls->decayTimeView->setSortingEnabled(false);

  // Set up model and delegate
  m_decayTimeModel = std::make_unique<DecayTimeMapModel>(this);

  m_Controls->decayTimeView->setModel(m_decayTimeModel.get());
  m_Controls->decayTimeView->setItemDelegate(new DecayTimeDelegate(this));

  // Configure tree view appearance
  m_Controls->decayTimeView->header()->setStretchLastSection(false);
  m_Controls->decayTimeView->header()->resizeSection(0, 200);
  m_Controls->decayTimeView->header()->resizeSection(1, 150);
  m_Controls->decayTimeView->header()->setDefaultSectionSize(150);



  connect(m_Controls->halflifeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnHalfLifeChanged(double)));
  connect(m_Controls->activitySpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnInjectedActivityChanged(double)));
  connect(m_Controls->weightSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnBodyWeightChanged(double)));
  connect(m_Controls->timeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnTimeToMeasurementChanged(int)));

  connect(m_Controls->radioTimeUser, SIGNAL(toggled(bool)), m_Controls->timeSpinBox, SLOT(setEnabled(bool)));
  connect(m_Controls->radioTimeUser, &QRadioButton::toggled, this, &QmitkPETSUVCalculationView::UpdateWidgets);

  connect(m_Controls->checkPETonly, &QCheckBox::toggled, this, &QmitkPETSUVCalculationView::OnCheckPETOnlyToggled);

  connect(m_Controls->petNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkPETSUVCalculationView::OnPETSelectionChanged);


  m_Controls->petNodeSelector->SetSelectionIsOptional(false);

  this->m_Controls->petNodeSelector->SetInvalidInfo("Select PET image for conversion.");
  this->m_Controls->petNodeSelector->SetEmptyInfo("Select PET image for conversion.");
  this->m_Controls->petNodeSelector->SetPopUpTitel("Select PET image.");
  this->m_Controls->petNodeSelector->SetPopUpHint("Select a PET image that should be the source for the SUV conversion.");

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
  if (!this->m_internalUpdate)
  {
    this->m_injectedActivity = value * 1000.0; // widget is [kBq], internal is [Bq]
    this->UpdateWidgets();
  }
}

void QmitkPETSUVCalculationView::OnBodyWeightChanged(double value)
{
  if (!this->m_internalUpdate)
  {
    this->m_bodyweight = value;
    this->UpdateWidgets();
  }
}

void QmitkPETSUVCalculationView::OnTimeToMeasurementChanged(int value)
{
  if (!this->m_internalUpdate)
  {
//    this->m_userDecayTime = value * 60; // widget is [min], internal is [sec]
    this->UpdateWidgets();
  }
}

void QmitkPETSUVCalculationView::OnHalfLifeChanged(double value)
{
  if (!this->m_internalUpdate)
  {
    this->m_halfLife = value * 60; // widget is [min], internal is [sec]
    this->m_DefinedNuclide.clear();
    this->UpdateWidgets();
  }
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
    HalfLifeMapType::const_iterator finding = this->m_HalfLifeMap.find(item.toStdString());

    if (finding != this->m_HalfLifeMap.end())
    {
      this->m_halfLife = finding->second;
      this->m_DefinedNuclide = finding->first;
    }

    this->UpdateWidgets();
  }
}

std::string GetBaseDatePropValueAsString(const mitk::BaseData *data, const mitk::DICOMTagPath &path)
{
  std::string result = "";

  if (data)
  {
    std::map<std::string, mitk::BaseProperty::Pointer> props = mitk::GetPropertyByDICOMTagPath(data, path);

    if (!props.empty())
    {
      result = props.begin()->second->GetValueAsString();
    }
  }

  return result;
}

void QmitkPETSUVCalculationView::OnCalculateSUVButtonClicked()
{
  auto inputNode = m_Controls->petNodeSelector->GetSelectedNode();
  mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
  std::string nameOfResultImage = inputNode->GetName();
  nameOfResultImage.append("_SUV");
  resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage));
  const auto image = dynamic_cast<mitk::Image *>(inputNode->GetData());

  if (nullptr == image)
  {
    mitkThrow() << "QmitkPETSUVCalculationView is in invalid state. Selected node does not contain an mitk::Image, despite the PET node selector should enforce it.";
  }

  mitk::DICOMTagPath modalityPath(0x0008, 0x0060);
  mitk::DICOMTagPath radioActivityUnitsPath(0x0054, 0x1001);

  QString modality = QString::fromStdString(GetBaseDatePropValueAsString(image, modalityPath));
  QString unit =
    QString::fromStdString(GetBaseDatePropValueAsString(image, radioActivityUnitsPath));

  bool isPET = modality.compare(QString("PT"), Qt::CaseInsensitive) == 0;
  bool isBqMl = unit.compare(QString("BQML"), Qt::CaseInsensitive) == 0;

  if (!isPET)
  {
    QMessageBox box;
    box.setText("No PET data!");
    box.setInformativeText(
      "Selected data seems to be no PET data. Dicom tag \"dicom.series.Modality\" is missing or has wrong value. Won't "
      "calculate SUV map.You may ignore and force computation at own risk.");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Ignore);
    box.setDefaultButton(QMessageBox::Ok);
    box.setIcon(QMessageBox::Warning);
    int ret = box.exec();
    if (ret == QMessageBox::Ignore)
    {
      isPET = true;
    }
  }

  if (isPET)
  {
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
      int ret = box.exec();
      if (ret == QMessageBox::Ignore)
      {
        isBqMl = true;
      }
    }
  }

  bool hasValidInputs =
    m_injectedActivity != 0 && m_bodyweight != 0 && (/*m_userDecayTime != 0 ||*/ m_validAutoTime) && m_halfLife != 0;
  //TODO valid input should also check user times
  if (isPET && isBqMl && hasValidInputs)
  {
    if (m_validAutoTime)
    {
      MITK_INFO << "Calculating SUV: Injected activity = " << m_injectedActivity / 1000.0
        << " kBq; Scaled body weight = " << m_bodyweight << " kg; Time to measurement = automatically detected"
        << " min; Half Life = " << m_halfLife / 60 << " min";
    }
    else
    {
      MITK_INFO << "Calculating SUV: Injected activity = " << m_injectedActivity / 1000.0
        << " kBq; Scaled body weight = " << m_bodyweight << " kg; Time to measurement = " /*<< m_userDecayTime / 60*/
        << " min; Half Life = " << m_halfLife / 60 << " min";
    }

    mitk::Image::Pointer imageSUV = CalcSUV(image);

    resultNode->SetData(imageSUV); // set data of new node
    this->GetDataStorage()->Add(resultNode, inputNode);
  }
}

mitk::Image::Pointer QmitkPETSUVCalculationView::CalcSUV(mitk::Image *inputImage) const
{
  typedef itk::Image<double, 3> ImageType;
  typedef itk::Image<double, 3> SUVImageType;

  mitk::Image::Pointer tempImage = mitk::Image::New();
  tempImage->Initialize(inputImage);
  tempImage->SetTimeGeometry(inputImage->GetTimeGeometry()->Clone());

  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
  imageTimeSelector->SetInput(inputImage);

  for (unsigned int i = 0; i < inputImage->GetTimeSteps(); ++i)
  {
    ImageType::Pointer itkImage = ImageType::New();

    imageTimeSelector->SetTimeNr(i);
    imageTimeSelector->UpdateLargestPossibleRegion();

    mitk::Image::Pointer mitkInputImage = imageTimeSelector->GetOutput();

    mitk::CastToItkImage(mitkInputImage, itkImage);

    typedef itk::IndexedUnaryFunctorImageFilter<ImageType, SUVImageType, mitk::SUVbwFunctorPolicy> SUVFilterType;
    SUVFilterType::Pointer suvFilter = SUVFilterType::New();
    mitk::SUVbwFunctorPolicy functor;
    functor.SetBodyWeight(m_bodyweight);
    functor.SetHalfLife(m_halfLife);
    functor.SetInjectedActivity(m_injectedActivity);

    if (this->m_Controls->radioTimeAuto->isChecked())
    {
      const auto &sliceIter = m_autoDecayTime.find(i);
      if (sliceIter == m_autoDecayTime.cend())
      {
        mitkThrow() << "Error while generating SUV image. No decay time available for at least one image timestep. "
                       "Problematic time step: "
                    << i;
      }
      const auto sliceDecayMap = sliceIter->second;

      mitk::SUVbwFunctorPolicy::DecayTimeFunctionType decayFunction =
        [sliceDecayMap](const mitk::SUVbwFunctorPolicy::IndexType &sliceIndex)
      {
        const auto &finding = sliceDecayMap.find(sliceIndex[2]);
        if (finding == sliceDecayMap.cend())
        {
          mitkThrow() << "Error while generating SUV image. No decay time available for the current slice. Problematic "
                         "slice index:"
                      << sliceIndex;
        }

        return finding->second;
      };

      functor.SetDecayTimeFunctor(decayFunction);
    }
    else
    {
      mitk::SUVbwFunctorPolicy::DecayTimeFunctionType decayFunction =
        [this](const mitk::SUVbwFunctorPolicy::IndexType& /*sliceIndex*/) { throw 0;
      return 0; };
      functor.SetDecayTimeFunctor(decayFunction);
    }

    suvFilter->SetFunctor(functor);

    suvFilter->SetInput(itkImage);

    mitk::Image::Pointer outputImage = mitk::ImportItkImage(suvFilter->GetOutput())->Clone();

    mitk::ImageReadAccessor accessor(outputImage);
    tempImage->SetVolume(accessor.GetData(), i);
  }

  mitk::Image::Pointer newImage = tempImage;

  return newImage;
}

void QmitkPETSUVCalculationView::UpdateWidgets()
{
  if (!this->m_internalUpdate)
  {
    this->m_internalUpdate = true;

    m_Controls->activitySpinBox->setValue(this->m_injectedActivity / 1000.0); // widget is [kBq], internal is [Bq]

    m_Controls->weightSpinBox->setValue(this->m_bodyweight);

    m_Controls->timeInfo->clear();
    m_Controls->timeSpinBox->setEnabled(m_Controls->radioTimeUser->isChecked());

    if (m_Controls->radioTimeUser->isChecked())
    {
      //m_Controls->timeSpinBox->setValue(this->m_userDecayTime / 60.0); // widget is [min], internal is [sec]
    }
    else
    {
      if (this->m_validAutoTime)
      {
        std::ostringstream stream;
        stream << "Detection strategy: Duration start time and acquisition time" << std::endl;

        for (const auto &timePos : m_autoDecayTime)
        {
          for (const auto &slicePos : timePos.second)
          {
            stream << "[" << timePos.first << "][" << slicePos.first << "]: " << slicePos.second / 60.0 << " [min]; ";
          }
        }

        m_Controls->timeInfo->setText(QString::fromStdString(stream.str()));
      }
    }

    m_Controls->halflifeSpinBox->setValue(this->m_halfLife / 60.0); // widget is [min], internal is [sec]

    m_Controls->labelAutoNuclide->setText(QString::fromStdString(this->m_DefinedNuclide));

    bool valid = m_Controls->petNodeSelector->GetSelectedNode().IsNotNull() && m_injectedActivity != 0 && m_bodyweight != 0 &&
                 (/*m_userDecayTime != 0 || */ m_validAutoTime) && m_halfLife != 0;
    m_Controls->btnCalculateSUV->setEnabled(valid);

    m_decayTimeModel->SetMode(m_Controls->radioTimeAuto->isChecked() ? DecayTimeMapModel::Mode::Auto : DecayTimeMapModel::Mode::UserDefined);

    this->m_internalUpdate = false;
  }
}

void QmitkPETSUVCalculationView::OnPETSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  m_Controls->btnCalculateSUV->setEnabled(false);
  auto newNode = m_Controls->petNodeSelector->GetSelectedNode();

  this->m_injectedActivity = 0.;
  this->m_bodyweight = 0.;
  this->m_autoDecayTime.clear();
  this->m_validAutoTime = false;
  this->m_DecayStrategy = mitk::DecayCorrectionStrategy::None;
  this->m_DefinedNuclide.clear();
  this->m_halfLife = 0.;

  if (newNode.IsNotNull() && this->m_Controls->checkAuto->isChecked())
  {
    auto activities = mitk::GetRadionuclideTotalDose(newNode->GetData());
    if (activities.empty())
    {
      MITK_ERROR << "Error reading injected activity.";
    }
    else if (activities.size() > 1)
    {
      MITK_WARN << "There are more then one radonuclide total doses stored for the node. First one will be used: "
        << activities[0];
    }
    else
    {
      this->m_injectedActivity = activities[0];
    }

    try
    {
      m_bodyweight = mitk::GetPatientsWeight(newNode->GetData());
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << "Error reading patient body weight. Error details:" << e;
    }

    m_DefinedNuclide = mitk::GetRadionuclideNames(newNode->GetData());

    auto halflifes = mitk::GetRadionuclideHalfLife(newNode->GetData());
    if (halflifes.empty())
    {
      MITK_ERROR << "Error reading radio nuclide half life.";
    }
    else if (halflifes.size() > 1)
    {
      MITK_WARN << "There are more then one radonuclide half life stored for the node. First one will be used: "
        << halflifes[0];
    }
    else
    {
      this->m_halfLife = halflifes[0];
    }

    if (this->m_Controls->radioTimeAuto->isChecked())
    {
      try
      {
        const auto* slicedData = dynamic_cast<const mitk::SlicedData*>(newNode->GetData());
        const auto info = mitk::DeduceDecayCorrection(slicedData);
        this->m_autoDecayTime = info.decayTimes;
        this->m_DecayStrategy = info.strategy;
        m_validAutoTime = true;
      }
      catch (const mitk::SUVHelperException& e)
      {
        m_validAutoTime = false;
        MITK_ERROR << "Error deducing decay time (" << e.GetNameOfClass()
                   << "). Error details: " << e;
      }
      catch (const mitk::Exception& e)
      {
        m_validAutoTime = false;
        MITK_ERROR << "Error deducing decay time. Error details: " << e;
      }
    }
    else
    {
      m_autoDecayTime.clear();
      for (mitk::TimeStepType ts = 0; ts < newNode->GetData()->GetTimeSteps(); ++ts)
      {
        m_autoDecayTime[ts][0] = 0.;
      }
    }
    m_decayTimeModel->SetDecayTimeMap(m_autoDecayTime);
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
    m_injectedActivity(0),
    m_bodyweight(0),
    m_validAutoTime(false),
    m_halfLife(0),
    m_internalUpdate(false)
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
