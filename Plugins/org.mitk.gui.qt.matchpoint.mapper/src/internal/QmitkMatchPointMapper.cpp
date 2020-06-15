/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_matchpoint_mapper_Activator.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Mitk
#include <mitkImageAccessByItk.h>
#include <mitkStatusBar.h>
#include "mitkImageMappingHelper.h"
#include "mitkMAPRegistrationWrapper.h"
#include "mitkMatchPointPropertyTags.h"
#include "mitkRegistrationHelper.h"
#include <mitkResultNodeGenerationHelper.h>
#include <mitkUIDHelper.h>
#include <mitkMAPAlgorithmHelper.h>
#include <mitkResultNodeGenerationHelper.h>
#include <mitkNodePredicateFunction.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataProperty.h>

// Qmitk
#include "QmitkMatchPointMapper.h"

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QErrorMessage>
#include <QThreadPool>

const std::string QmitkMatchPointMapper::VIEW_ID = "org.mitk.views.matchpoint.mapper";

QmitkMatchPointMapper::QmitkMatchPointMapper()
    : m_Parent(nullptr), m_preparedForBinaryInput(false)
{
}

void QmitkMatchPointMapper::SetFocus()
{
    //m_Controls.buttonPerformImageProcessing->setFocus();
}

void QmitkMatchPointMapper::CreateConnections()
{
    connect(m_Controls.registrationNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointMapper::OnRegNodeSelectionChanged);
    connect(m_Controls.inputNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointMapper::OnInputNodeSelectionChanged);
    connect(m_Controls.referenceNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointMapper::OnReferenceNodeSelectionChanged);

    connect(m_Controls.m_cbManualRef, SIGNAL(clicked()), this, SLOT(OnManualRefChecked()));
    connect(m_Controls.m_cbLinkFactors, SIGNAL(clicked()), this, SLOT(OnLinkSampleFactorChecked()));

    connect(m_Controls.m_sbXFactor, SIGNAL(valueChanged(double)), this, SLOT(OnXFactorChanged(double)));

    connect(m_Controls.m_pbMap, SIGNAL(clicked()), this, SLOT(OnMapBtnPushed()));
    connect(m_Controls.m_pbRefine, SIGNAL(clicked()), this, SLOT(OnRefineBtnPushed()));
}

void QmitkMatchPointMapper::Error(QString msg)
{
    mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
    MITK_ERROR << msg.toStdString().c_str();

    m_Controls.m_teLog->append(QStringLiteral("<font color='red'><b>") + msg + QStringLiteral("</b></font>"));
}

void QmitkMatchPointMapper::CreateQtPartControl(QWidget* parent)
{
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls.setupUi(parent);

    m_Parent = parent;

    this->m_Controls.registrationNodeSelector->SetDataStorage(this->GetDataStorage());
    this->m_Controls.registrationNodeSelector->SetSelectionIsOptional(true);
    this->m_Controls.inputNodeSelector->SetDataStorage(this->GetDataStorage());
    this->m_Controls.inputNodeSelector->SetSelectionIsOptional(false);
    this->m_Controls.referenceNodeSelector->SetDataStorage(this->GetDataStorage());
    this->m_Controls.referenceNodeSelector->SetSelectionIsOptional(false);

    this->m_Controls.registrationNodeSelector->SetInvalidInfo("Select valid registration.");
    this->m_Controls.registrationNodeSelector->SetEmptyInfo("Assuming identity mapping. Select registration to change.");
    this->m_Controls.registrationNodeSelector->SetPopUpTitel("Select registration.");
    this->m_Controls.registrationNodeSelector->SetPopUpHint("Select a registration object that should be used for the mapping of the input data. If no registration is selected, identity will be assumed for the mapping.");

    this->m_Controls.inputNodeSelector->SetInvalidInfo("Select input data.");
    this->m_Controls.inputNodeSelector->SetPopUpTitel("Select input data.");
    this->m_Controls.inputNodeSelector->SetPopUpHint("Select the input data for the mapping. (Images or point sets are supported so far).");
    this->m_Controls.referenceNodeSelector->SetInvalidInfo("Select the reference image.");
    this->m_Controls.referenceNodeSelector->SetPopUpTitel("Select the reference image.");
    this->m_Controls.referenceNodeSelector->SetPopUpHint("Select the reference image that specifies the target geometrie the input should be mapped into.");

    this->ConfigureRegNodePredicate();
    this->ConfigureNodePredicates();

    // show first page
    m_Controls.m_tabs->setCurrentIndex(0);

    this->CreateConnections();
    this->CheckInputs();
    this->ConfigureProgressInfos();
    this->ConfigureMappingControls();
}

/** Method checks if the currently selected reg node has a direct kernel that
* can be decomposed in a rotation matrix and a offset. If this is true, true
* is returned. In all other cases false is returned.*/
bool  QmitkMatchPointMapper::IsAbleToRefineGeometry() const
{
    bool result = false;

    if (this->m_spSelectedRegNode.IsNotNull())
    {
        const mitk::MAPRegistrationWrapper* wrapper = dynamic_cast<const mitk::MAPRegistrationWrapper*>
            (this->m_spSelectedRegNode->GetData());

        //if the helper does not return null, we can refine the geometry.
        result = mitk::MITKRegistrationHelper::getAffineMatrix(wrapper, false).IsNotNull();
    }

    return result;
}

bool  QmitkMatchPointMapper::IsBinaryInput() const
{
    auto maskPredicate = mitk::MITKRegistrationHelper::MaskNodePredicate();

    bool result = false;

    if(this->m_spSelectedInputNode.IsNotNull())
    {
      result = maskPredicate->CheckNode(this->m_spSelectedInputNode);
    }

    return result;
}

bool  QmitkMatchPointMapper::IsPointSetInput() const
{
    bool result = false;

    if (this->m_spSelectedInputNode.IsNotNull())
    {
        result = dynamic_cast<const mitk::PointSet*>(this->m_spSelectedInputNode->GetData()) != nullptr;
    }

    return result;
}

mitk::DataNode::Pointer QmitkMatchPointMapper::GetAutoRefNodeByReg()
{
    mitk::DataNode::Pointer spResult = nullptr;

    if (this->m_spSelectedRegNode.IsNotNull() && this->m_spSelectedRegNode->GetData())
    {
        std::string nodeName;
        mitk::BaseProperty* uidProp = m_spSelectedRegNode->GetData()->GetProperty(mitk::Prop_RegAlgTargetData);

        if (uidProp)
        {
            //search for the target node
            mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID,
                uidProp);
            spResult = this->GetDataStorage()->GetNode(predicate);
        }
    }
    if (spResult.IsNull() && this->m_spSelectedInputNode.IsNotNull())
    {
        //no really reference is available -> use the input as reference
        spResult = this->m_spSelectedInputNode;
        if (this->m_spSelectedRefNode != spResult)
        {
          m_Controls.m_teLog->append(
            QStringLiteral("<font color='gray'><i>Cannot determine reference automatically. Use input image as reference.</i></font>"));
        }
    }

    return spResult;
}

void QmitkMatchPointMapper::ConfigureRegNodePredicate(const mitk::DataNode* input)
{
  mitk::NodePredicateBase::ConstPointer nodePredicate = mitk::MITKRegistrationHelper::RegNodePredicate();

  if (input != nullptr)
  {
    unsigned int dimension = 0;

    auto inputImage = dynamic_cast<mitk::Image*>(input->GetData());
    auto pointset = dynamic_cast<const mitk::PointSet*>(input->GetData());
    if (inputImage)
    {
      dimension = inputImage->GetDimension();

      if (inputImage->GetTimeSteps() > 1)
      {
        //images has multiple time steps -> remove one dimension.
        dimension -= 1;
      }
    }
    else if (pointset)
    {
      dimension = 3;
    }

    auto dimCheck = [dimension](const mitk::DataNode * node)
    {
      const mitk::MAPRegistrationWrapper* wrapper = dynamic_cast < const mitk::MAPRegistrationWrapper* >(node->GetData());

      return wrapper != nullptr && wrapper->GetMovingDimensions() == dimension;
    };
    mitk::NodePredicateFunction::Pointer hasCorrectDim = mitk::NodePredicateFunction::New(dimCheck);

    nodePredicate = mitk::NodePredicateAnd::New(nodePredicate, hasCorrectDim).GetPointer();
  }

  this->m_Controls.registrationNodeSelector->SetNodePredicate(nodePredicate);
}

std::function<bool(const mitk::DataNode *)> GenerateDimCheckLambda(unsigned int dim)
{
  auto dimCheck = [dim](const mitk::DataNode * node)
  {
    auto inputImage = dynamic_cast<mitk::Image*>(node->GetData());

    return inputImage != nullptr &&
      (inputImage->GetDimension() == dim ||
      (inputImage->GetDimension() == dim + 1 && inputImage->GetTimeSteps()>1));
  };

  return dimCheck;
}

void QmitkMatchPointMapper::ConfigureNodePredicates(const mitk::DataNode* reg)
{
  auto isImage = mitk::MITKRegistrationHelper::ImageNodePredicate();
  auto isPointSet = mitk::MITKRegistrationHelper::PointSetNodePredicate();

  auto isData = mitk::NodePredicateOr::New(isImage, isPointSet);

  mitk::NodePredicateBase::ConstPointer inputPredicate = isData.GetPointer();
  mitk::NodePredicateBase::ConstPointer refPredicate = isImage.GetPointer();

  if (reg != nullptr)
  {
    const mitk::MAPRegistrationWrapper* wrapper = dynamic_cast <const mitk::MAPRegistrationWrapper*>(reg->GetData());

    if (wrapper != nullptr)
    {
      auto movingDim = wrapper->GetMovingDimensions();

      auto dimCheck = GenerateDimCheckLambda(movingDim);
      auto hasCorrectDim = mitk::NodePredicateFunction::New(dimCheck);

      if (movingDim == 3)
      {
        //Remark: Point sets are always 3D
        auto is3DInput = mitk::NodePredicateOr::New(isPointSet, mitk::NodePredicateAnd::New(isImage, hasCorrectDim));
        inputPredicate = is3DInput.GetPointer();
      }
      else
      {
        auto is2DInput = mitk::NodePredicateAnd::New(isImage, hasCorrectDim);
        inputPredicate = is2DInput.GetPointer();
      }

      auto targetDim = wrapper->GetTargetDimensions();

      auto targetDimCheck = GenerateDimCheckLambda(targetDim);
      auto hasCorrectTargetDim = mitk::NodePredicateFunction::New(targetDimCheck);

      auto isRef = mitk::NodePredicateAnd::New(isImage, hasCorrectTargetDim);
      refPredicate = isRef;

    }
  }
  this->m_Controls.inputNodeSelector->SetNodePredicate(inputPredicate);
  this->m_Controls.referenceNodeSelector->SetNodePredicate(refPredicate);
}

void QmitkMatchPointMapper::CheckInputs()
{
    this->m_spSelectedRegNode = this->m_Controls.registrationNodeSelector->GetSelectedNode();
    this->m_spSelectedInputNode = this->m_Controls.inputNodeSelector->GetSelectedNode();
    this->m_spSelectedRefNode = this->m_Controls.referenceNodeSelector->GetSelectedNode();

    if (!(m_Controls.m_cbManualRef->isChecked()))
    {
        auto autoRefNode = this->GetAutoRefNodeByReg();
        if (this->m_spSelectedRefNode != autoRefNode)
        {
          this->m_spSelectedRefNode = autoRefNode;
          QmitkSingleNodeSelectionWidget::NodeList selection;

          if (this->m_spSelectedRefNode.IsNotNull())
          {
            selection.append(this->m_spSelectedRefNode);
          }
          this->m_Controls.referenceNodeSelector->SetCurrentSelection(selection);
        }
    }

    if (this->m_spSelectedRefNode.IsNotNull() && this->m_spSelectedRefNode->GetData()
        && this->m_spSelectedRefNode->GetData()->GetTimeSteps() > 1)
    {
        m_Controls.m_teLog->append(
          QStringLiteral("<font color='gray'><i>Selected reference image has multiple time steps. Only geometry of time step 1 is used as reference.</i></font>"));
    }
}

void QmitkMatchPointMapper::ConfigureMappingControls()
{
    bool validInput = m_spSelectedInputNode.IsNotNull();
    bool validRef = m_spSelectedRefNode.IsNotNull();

    this->m_Controls.referenceNodeSelector->setEnabled(this->m_Controls.m_cbManualRef->isChecked());
    this->m_Controls.m_pbMap->setEnabled(validInput  && validRef);
    this->m_Controls.m_pbRefine->setEnabled(validInput && this->IsAbleToRefineGeometry() && !this->IsPointSetInput());

    if (validInput)
    {
      if (m_spSelectedRegNode.IsNotNull())
      {
        this->m_Controls.m_leMappedName->setText(tr("mapped_") + QString::fromStdString(m_spSelectedInputNode->GetName())
          + tr("_by_") + QString::fromStdString(m_spSelectedRegNode->GetName()));
      }
      else
      {
        this->m_Controls.m_leMappedName->setText(tr("resampled_") + QString::fromStdString(m_spSelectedInputNode->GetName()));
      }
    }
    else
    {
        this->m_Controls.m_leMappedName->setText(tr("mappedData"));
    }

    if (this->IsBinaryInput() != this->m_preparedForBinaryInput)
    {
        if (this->IsBinaryInput())
        {
            m_Controls.m_teLog->append(
              QStringLiteral("<font color='gray'><i>Binary input (mask) detected. Preparing for mask mapping (default interpolation: nearest neigbour; padding value: 0)</i></font>"));

            this->m_Controls.m_comboInterpolator->setCurrentIndex(0);
            this->m_Controls.m_sbErrorValue->setValue(0);
            this->m_Controls.m_sbPaddingValue->setValue(0);
        }
        else
        {
            this->m_Controls.m_comboInterpolator->setCurrentIndex(1);
        }

        this->m_preparedForBinaryInput = this->IsBinaryInput();
    }

    OnLinkSampleFactorChecked();
}

void QmitkMatchPointMapper::ConfigureProgressInfos()
{

}

void QmitkMatchPointMapper::OnRegNodeSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  mitk::DataNode::Pointer regNode;
  if (!nodes.isEmpty())
  {
    regNode = nodes.front();
  }

  this->ConfigureNodePredicates(regNode);
  this->CheckInputs();
  this->ConfigureMappingControls();
}

void QmitkMatchPointMapper::OnInputNodeSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  mitk::DataNode::Pointer inputNode;
  if (!nodes.isEmpty())
  {
    inputNode = nodes.front();
  }

  this->ConfigureRegNodePredicate(inputNode);
  this->CheckInputs();
  this->ConfigureMappingControls();
}

void QmitkMatchPointMapper::OnReferenceNodeSelectionChanged(QList<mitk::DataNode::Pointer> /*nodes*/)
{
  this->CheckInputs();
  this->ConfigureMappingControls();
}

void QmitkMatchPointMapper::OnManualRefChecked()
{
    this->CheckInputs();
    this->ConfigureMappingControls();
}

void QmitkMatchPointMapper::OnLinkSampleFactorChecked()
{
    this->m_Controls.m_sbYFactor->setEnabled(!(this->m_Controls.m_cbLinkFactors->isChecked()));
    this->m_Controls.m_sbZFactor->setEnabled(!(this->m_Controls.m_cbLinkFactors->isChecked()));

    if (m_Controls.m_cbLinkFactors->isChecked())
    {
        this->m_Controls.m_sbYFactor->setValue(this->m_Controls.m_sbXFactor->value());
        this->m_Controls.m_sbZFactor->setValue(this->m_Controls.m_sbXFactor->value());
    }
}


void QmitkMatchPointMapper::OnMapBtnPushed()
{
    SpawnMappingJob();
}

void QmitkMatchPointMapper::OnRefineBtnPushed()
{
    SpawnMappingJob(true);
}

void QmitkMatchPointMapper::SpawnMappingJob(bool doGeometryRefinement)
{
    if (m_Controls.m_checkClearLog->checkState() == Qt::Checked)
    {
        this->m_Controls.m_teLog->clear();
    }

    /////////////////////////
    //create job and put it into the thread pool
    QmitkMappingJob* pJob = new QmitkMappingJob();
    pJob->setAutoDelete(true);

    pJob->m_spInputData = this->m_spSelectedInputNode->GetData();
    pJob->m_InputDataUID = mitk::EnsureUID(this->m_spSelectedInputNode->GetData());
    pJob->m_doGeometryRefinement = doGeometryRefinement;

    pJob->m_spRegNode = m_spSelectedRegNode;
    if (m_spSelectedRegNode.IsNull())
    {
        pJob->m_spRegNode = mitk::DataNode::New();
        pJob->m_spRegNode->SetData(mitk::GenerateIdentityRegistration3D().GetPointer());
        pJob->m_spRegNode->SetName("Auto_Generated_Identity_Transform");
        m_Controls.m_teLog->append(
          QStringLiteral("<font color='gray'><i>No registration selected. Preforming mapping with identity transform</i></font>"));
    }

    if (!doGeometryRefinement)
    {
        pJob->m_spRefGeometry = m_spSelectedRefNode->GetData()->GetGeometry()->Clone().GetPointer();

        //check for super/sub sampling
        if (m_Controls.m_groupActivateSampling->isChecked())
        {
            //change the pixel count and  spacing of the geometry
            mitk::BaseGeometry::BoundsArrayType geoBounds = pJob->m_spRefGeometry->GetBounds();
            auto oldSpacing = pJob->m_spRefGeometry->GetSpacing();
            mitk::Vector3D geoSpacing;

            geoSpacing[0] = oldSpacing[0] / m_Controls.m_sbXFactor->value();
            geoSpacing[1] = oldSpacing[1] / m_Controls.m_sbYFactor->value();
            geoSpacing[2] = oldSpacing[2] / m_Controls.m_sbZFactor->value();

            geoBounds[1] = geoBounds[1] * m_Controls.m_sbXFactor->value();
            geoBounds[3] = geoBounds[3] * m_Controls.m_sbYFactor->value();
            geoBounds[5] = geoBounds[5] * m_Controls.m_sbZFactor->value();

            pJob->m_spRefGeometry->SetBounds(geoBounds);
            pJob->m_spRefGeometry->SetSpacing(geoSpacing);

            auto oldOrigin = pJob->m_spRefGeometry->GetOrigin();

            //if we change the spacing we must also correct the origin to ensure
            //that the voxel matrix still covers the same space. This is due the fact
            //that the origin is not in the corner of the voxel matrix, but in the center
            // of the voxel that is in the corner.
            mitk::Point3D newOrigin;
            for (mitk::Point3D::SizeType i = 0; i < 3; ++i)
            {
              newOrigin[i] = 0.5* (geoSpacing[i] - oldSpacing[i]) + oldOrigin[i];
            }

            pJob->m_spRefGeometry->SetOrigin(newOrigin);
        }
    }

    pJob->m_MappedName = m_Controls.m_leMappedName->text().toStdString();
    pJob->m_allowUndefPixels = m_Controls.m_groupAllowUndefPixels->isChecked();
    pJob->m_paddingValue = m_Controls.m_sbPaddingValue->value();
    pJob->m_allowUnregPixels = m_Controls.m_groupAllowUnregPixels->isChecked();
    pJob->m_errorValue = m_Controls.m_sbErrorValue->value();
    pJob->m_InterpolatorLabel = m_Controls.m_comboInterpolator->currentText().toStdString();

    switch (m_Controls.m_comboInterpolator->currentIndex())
    {
    case 0:
        pJob->m_InterpolatorType = mitk::ImageMappingInterpolator::NearestNeighbor;
        break;

    case 1:
        pJob->m_InterpolatorType = mitk::ImageMappingInterpolator::Linear;
        break;

    case 2:
        pJob->m_InterpolatorType = mitk::ImageMappingInterpolator::BSpline_3;
        break;

    case 3:
        pJob->m_InterpolatorType = mitk::ImageMappingInterpolator::WSinc_Hamming;
        break;

    case 4:
        pJob->m_InterpolatorType = mitk::ImageMappingInterpolator::WSinc_Welch;
        break;
    }

    connect(pJob, SIGNAL(Error(QString)), this, SLOT(OnMapJobError(QString)));
    connect(pJob, SIGNAL(MapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)), this,
        SLOT(OnMapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
        Qt::BlockingQueuedConnection);
    connect(pJob, SIGNAL(AlgorithmInfo(QString)), this, SLOT(OnMappingInfo(QString)));

    m_Controls.m_teLog->append(QStringLiteral("<b><font color='blue'>Started mapping job. Name: ") +
        m_Controls.m_leMappedName->text() + QStringLiteral("</font></b>"));

    QThreadPool* threadPool = QThreadPool::globalInstance();
    threadPool->start(pJob);
}



void QmitkMatchPointMapper::OnMapJobError(QString err)
{
    Error(err);
}

void QmitkMatchPointMapper::OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData,
    const QmitkMappingJob* job)
{
    m_Controls.m_teLog->append(QStringLiteral("<b><font color='blue'>Mapped entity stored. Name: ") +
        QString::fromStdString(job->m_MappedName) + QStringLiteral("</font></b>"));

    mitk::DataNode::Pointer spMappedNode = mitk::generateMappedResultNode(job->m_MappedName,
        spMappedData, job->GetRegistration()->getRegistrationUID(), job->m_InputDataUID,
        job->m_doGeometryRefinement, job->m_InterpolatorLabel);
    this->GetDataStorage()->Add(spMappedNode);
    this->GetRenderWindowPart()->RequestUpdate();

    this->CheckInputs();
    this->ConfigureMappingControls();
}

void QmitkMatchPointMapper::OnMappingInfo(QString info)
{
    m_Controls.m_teLog->append(QStringLiteral("<font color='gray'><i>") + info + QStringLiteral("</i></font>"));
}

void QmitkMatchPointMapper::OnXFactorChanged(double d)
{
    if (m_Controls.m_cbLinkFactors->isChecked())
    {
        this->m_Controls.m_sbYFactor->setValue(d);
        this->m_Controls.m_sbZFactor->setValue(d);
    }
}
