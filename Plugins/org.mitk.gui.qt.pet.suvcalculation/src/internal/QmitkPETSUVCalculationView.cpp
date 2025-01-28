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

#include "itkIndexedUnaryFunctorImageFilter.h"
#include "mitkITKImageImport.h"
#include "mitkImageCast.h"
#include "mitkSUVCalculation.h"
#include "mitkSUVFunctorPolicy.h"
#include "mitkWorkbenchUtil.h"
#include <QInputDialog>
#include <QMessageBox>
#include <iostream>
#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkHalfLifeConstants.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImageTimeSelector.h>
#include <mitkSUVCalculationHelper.h>

const std::string QmitkPETSUVCalculationView::VIEW_ID = "org.mitk.QmitkPETSUVCalculationView";

const std::string PROPERTY_NAME_DICOM_SERIES_MODALITY = "dicom.series.Modality";
const std::string PROPERTY_NAME_DICOM_PET_RADIOACTIVITYUNITS = "dicom.pet.RadioactivityUnits";

void QmitkPETSUVCalculationView::SetFocus()
{
  m_Controls.btnCalculateSUV->setFocus();
}

void QmitkPETSUVCalculationView::CreateQtPartControl(QWidget *parent)
{
  m_ParentWidget = parent;
  m_Controls.setupUi(parent);

  connect(m_Controls.btnCalculateSUV, SIGNAL(clicked()), this, SLOT(OnCalculateSUVButtonClicked()));
  connect(m_Controls.btnNuclideLookup, SIGNAL(clicked()), this, SLOT(OnNuclideLookupClicked()));

  connect(m_Controls.halflifeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnHalfLifeChanged(double)));

  connect(m_Controls.activitySpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnInjectedActivityChanged(double)));

  connect(m_Controls.weightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnBodyWeightChanged(int)));

  connect(m_Controls.timeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnTimeToMeasurementChanged(int)));

  connect(m_Controls.radioTimeUser, SIGNAL(toggled(bool)), m_Controls.timeSpinBox, SLOT(setEnabled(bool)));

  m_Controls.radioTimeAuto->setChecked(true);
  m_Controls.timeSpinBox->setEnabled(false);

  this->UpdateWidgets();
}

void QmitkPETSUVCalculationView::OnInjectedActivityChanged(double value)
{
  if (!this->m_internalUpdate)
  {
    this->m_injectedActivity = value * 1000.0; // widget is [kBq], internal is [Bq]
    this->UpdateWidgets();
  }
}

void QmitkPETSUVCalculationView::OnBodyWeightChanged(int value)
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
    this->m_userDecayTime = value * 60; // widget is [min], internal is [sec]
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
  mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
  std::string nameOfResultImage = m_selectedNode->GetName();
  nameOfResultImage.append("_SUV");
  resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage));
  mitk::Image *image = dynamic_cast<mitk::Image *>(m_selectedNode->GetData());

  mitk::DICOMTagPath modalityPath(0x0008, 0x0060);
  mitk::DICOMTagPath radioActivityUnitsPath(0x0054, 0x1001);

  QString modality = QString::fromStdString(GetBaseDatePropValueAsString(m_selectedNode->GetData(), modalityPath));
  QString unit =
    QString::fromStdString(GetBaseDatePropValueAsString(m_selectedNode->GetData(), radioActivityUnitsPath));

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
    m_injectedActivity != 0 && m_bodyweight != 0 && (m_userDecayTime != 0 || m_validAutoTime) && m_halfLife != 0;

  if (isPET && isBqMl && hasValidInputs)
  {
    MITK_INFO << "Calculating SUV: Injected activity = " << m_injectedActivity / 1000.0
              << "kBq; Scaled body weight = " << m_bodyweight << " kg; Time to measurement = " << m_userDecayTime / 60
              << " min; Half Life = " << m_halfLife / 60 << " min";

    mitk::Image::Pointer imageSUV = CalcSUV(image);

    resultNode->SetData(imageSUV); // set data of new node
    this->GetDataStorage()->Add(resultNode, m_selectedNode);
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

    if (this->m_Controls.radioTimeAuto->isChecked())
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
        [this](const mitk::SUVbwFunctorPolicy::IndexType & /*sliceIndex*/) { return this->m_userDecayTime; };
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

    m_Controls.activitySpinBox->setValue(this->m_injectedActivity / 1000.0); // widget is [kBq], internal is [Bq]

    m_Controls.weightSpinBox->setValue(this->m_bodyweight);

    m_Controls.timeInfo->clear();
    if (m_Controls.radioTimeUser->isChecked())
    {
      m_Controls.timeSpinBox->setValue(this->m_userDecayTime / 60.0); // widget is [min], internal is [sec]
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

        m_Controls.timeInfo->setText(QString::fromStdString(stream.str()));
      }
    }

    m_Controls.halflifeSpinBox->setValue(this->m_halfLife / 60.0); // widget is [min], internal is [sec]

    m_Controls.labelAutoNuclide->setText(QString::fromStdString(this->m_DefinedNuclide));

    bool valid = m_selectedNode.IsNotNull() && m_injectedActivity != 0 && m_bodyweight != 0 &&
                 (m_userDecayTime != 0 || m_validAutoTime) && m_halfLife != 0;
    m_Controls.btnCalculateSUV->setEnabled(valid);

    this->m_internalUpdate = false;
  }
}

void QmitkPETSUVCalculationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                    const QList<mitk::DataNode::Pointer> & /*nodes*/)
{
  m_Controls.btnCalculateSUV->setEnabled(false);
  mitk::DataNode *newNode = NULL;

  QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();

  if (dataNodes.empty())
  {
    newNode = NULL;
  }
  else
  {
    newNode = dataNodes[0];
  }

  if (newNode != this->m_selectedNode)
  {
    this->m_selectedNode = newNode;

    this->m_injectedActivity = 0;
    this->m_bodyweight = 0;
    this->m_userDecayTime = 0;
    this->m_autoDecayTime.clear();
    this->m_validAutoTime = false;
    this->m_DefinedNuclide.clear();
    this->m_halfLife = 0;

    if (this->m_selectedNode && this->m_Controls.checkAuto->isChecked())
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
      catch (const mitk::Exception &e)
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

      if (this->m_Controls.radioTimeAuto->isChecked())
      {
        try
        {
          this->m_autoDecayTime = mitk::DeduceDecayTime_AcquisitionMinusStartSliceResolved(newNode->GetData());
          m_validAutoTime = true;
        }
        catch (const mitk::Exception &e)
        {
          m_validAutoTime = false;
          MITK_ERROR << "Error deducing decay time. Error details:" << e;
        }
      }
    }

    this->UpdateWidgets();
  }
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
  : m_injectedActivity(0),
    m_bodyweight(0),
    m_userDecayTime(0),
    m_validAutoTime(false),
    m_halfLife(0),
    m_internalUpdate(false)
{
  GenerateHalfLifeMap();
}
