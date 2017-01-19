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

#include "org_mitk_gui_qt_matchpoint_mapper_Activator.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Mitk
#include <mitkImageAccessByItk.h>
#include <mitkStatusBar.h>
#include <mitkNodePredicateDataProperty.h>
#include "mitkImageMappingHelper.h"
#include "mitkMAPRegistrationWrapper.h"
#include "mitkMatchPointPropertyTags.h"
#include "mitkRegistrationHelper.h"
#include <mitkResultNodeGenerationHelper.h>
#include <mitkUIDHelper.h>
#include <mitkAlgorithmHelper.h>
#include <mitkResultNodeGenerationHelper.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>

// Qmitk
#include "QmitkMatchPointMapper.h"

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QErrorMessage>
#include <QThreadPool>

const std::string QmitkMatchPointMapper::VIEW_ID = "org.mitk.views.matchpoint.mapper";

QmitkMatchPointMapper::QmitkMatchPointMapper()
    : m_Parent(NULL), m_preparedForBinaryInput(false)
{
}

void QmitkMatchPointMapper::SetFocus()
{
    //m_Controls.buttonPerformImageProcessing->setFocus();
}

void QmitkMatchPointMapper::CreateConnections()
{
    // show first page
    m_Controls.m_tabs->setCurrentIndex(0);

    connect(m_Controls.m_pbLockReg, SIGNAL(clicked()), this, SLOT(OnLockRegButtonPushed()));
    connect(m_Controls.m_pbLockInput, SIGNAL(clicked()), this, SLOT(OnLockInputButtonPushed()));
    connect(m_Controls.m_pbLockRef, SIGNAL(clicked()), this, SLOT(OnLockReferenceButtonPushed()));

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

    m_Controls.m_teLog->append(QString("<font color='red'><b>") + msg + QString("</b></font>"));
}

void QmitkMatchPointMapper::CreateQtPartControl(QWidget* parent)
{
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls.setupUi(parent);

    m_Parent = parent;

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
    mitk::NodePredicateDataType::Pointer isLabelSet = mitk::NodePredicateDataType::New("LabelSetImage");
    mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
    mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateAnd::Pointer isLegacyMask = mitk::NodePredicateAnd::New(isImage, isBinary);

    mitk::NodePredicateOr::Pointer maskPredicate = mitk::NodePredicateOr::New(isLegacyMask, isLabelSet);

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
        result = dynamic_cast<const mitk::PointSet*>(this->m_spSelectedInputNode->GetData()) != NULL;
    }

    return result;
}

mitk::DataNode::Pointer QmitkMatchPointMapper::GetSelectedRegNode()
{
    mitk::DataNode::Pointer spResult = NULL;

    typedef QList<mitk::DataNode::Pointer> NodeListType;

    NodeListType nodes = this->GetDataManagerSelection();

    for (NodeListType::iterator pos = nodes.begin(); pos != nodes.end(); ++pos)
    {
        if (mitk::MITKRegistrationHelper::IsRegNode(*pos))
        {
            spResult = *pos;
            break;
        }
    }

    return spResult;
}


QList<mitk::DataNode::Pointer> QmitkMatchPointMapper::GetSelectedDataNodes()
{
    typedef QList<mitk::DataNode::Pointer> NodeListType;

    NodeListType nodes = this->GetDataManagerSelection();
    NodeListType result;

    for (NodeListType::iterator pos = nodes.begin(); pos != nodes.end(); ++pos)
    {
        if (!mitk::MITKRegistrationHelper::IsRegNode(*pos))
        {
            result.push_back(*pos);
        }
    }

    return result;
}

mitk::DataNode::Pointer QmitkMatchPointMapper::GetAutoRefNodeByReg()
{
    mitk::DataNode::Pointer spResult = NULL;

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
        m_Controls.m_teLog->append(
            QString("<font color='gray'><i>Cannot determine reference automatically. Use input image as reference.</i></font>"));
    }

    return spResult;
}

void QmitkMatchPointMapper::CheckInputs()
{
    mitk::DataNode::Pointer regNode = this->GetSelectedRegNode();

    QList<mitk::DataNode::Pointer> dataNodes = this->GetSelectedDataNodes();

    //first set the internal nodes according to selection
    if (!m_Controls.m_pbLockReg->isChecked())
    {
        this->m_spSelectedRegNode = regNode;
    }

    if (!m_Controls.m_pbLockInput->isChecked())
    {
        mitk::DataNode::Pointer inputNode = NULL;

        if (dataNodes.size() > 0)
        {
            inputNode = dataNodes[0];
        }

        this->m_spSelectedInputNode = inputNode;
    }

    if (!(m_Controls.m_cbManualRef->isChecked()))
    {
        this->m_spSelectedRefNode = this->GetAutoRefNodeByReg();
    }
    else
    {
        if (!m_Controls.m_pbLockRef->isChecked())
        {
            mitk::DataNode::Pointer refNode = NULL;

            int relevantIndex = 1;

            if (m_Controls.m_pbLockInput->isChecked())
            {
                //the input is locked thus use the first selected data node
                relevantIndex = 0;
            }

            if (dataNodes.size() > relevantIndex)
            {
                refNode = dataNodes[relevantIndex];
            }

            this->m_spSelectedRefNode = refNode;
        }
    }

    //second, check validity of the selected nodes
    this->CheckNodesValidity(this->m_ValidReg, this->m_ValidInput, this->m_ValidRef);

    //third, configure widgets
    if (this->m_spSelectedRegNode.IsNull())
    {
        m_Controls.m_lbRegistrationName->setText(
            QString("<font color='red'>no registration selected!</font>"));
    }
    else
    {
        if (this->m_ValidReg)
        {
            m_Controls.m_lbRegistrationName->setText(QString::fromStdString(
                this->m_spSelectedRegNode->GetName()));
        }
        else
        {
            m_Controls.m_lbRegistrationName->setText(QString("<font color='red'>") + QString::fromStdString(
                this->m_spSelectedRegNode->GetName()) + QString("</font>"));
        }
    }

    if (this->m_spSelectedInputNode.IsNull())
    {
        m_Controls.m_lbInputName->setText(QString("<font color='red'>no input selected!</font>"));
    }
    else
    {
        if (this->m_ValidInput)
        {
            m_Controls.m_lbInputName->setText(QString::fromStdString(this->m_spSelectedInputNode->GetName()));
        }
        else
        {
            m_Controls.m_lbInputName->setText(QString("<font color='red'>") + QString::fromStdString(
                this->m_spSelectedInputNode->GetName()) + QString("</font>"));
        }

    }

    if (this->m_spSelectedRefNode.IsNull())
    {
        if (this->m_ValidRef)
        {
            m_Controls.m_lbReferenceName->setText(QString("input needs no reference needed"));
        }
        else
        {
            m_Controls.m_lbReferenceName->setText(QString("<font color='red'>no reference selected!</font>"));
        }
    }
    else
    {
        if (this->m_ValidRef)
        {
            m_Controls.m_lbReferenceName->setText(QString::fromStdString(this->m_spSelectedRefNode->GetName()));

            if (this->m_spSelectedRefNode->GetData()
                && this->m_spSelectedRefNode->GetData()->GetTimeSteps() > 1)
            {
                m_Controls.m_teLog->append(
                    QString("<font color='gray'><i>Selected reference image has multiple time steps. Only geometry of time step 1 is used as reference.</i></font>"));
            }
        }
        else
        {
            m_Controls.m_lbReferenceName->setText(QString("<font color='red'>") + QString::fromStdString(
                this->m_spSelectedRefNode->GetName()) + QString("</font>"));
        }
    }

    this->m_Controls.m_pbLockInput->setEnabled(this->m_spSelectedInputNode.IsNotNull());
    this->m_Controls.m_pbLockReg->setEnabled(this->m_spSelectedRegNode.IsNotNull());
    this->m_Controls.m_pbLockRef->setEnabled(this->m_spSelectedRefNode.IsNotNull());
}


void QmitkMatchPointMapper::CheckNodesValidity(bool& validReg, bool& validInput, bool& validRef)
{
    validReg = this->m_spSelectedRegNode.IsNotNull();
    validInput = this->m_spSelectedInputNode.IsNotNull();
    validRef = this->m_spSelectedRefNode.IsNotNull();

    if (this->m_spSelectedRegNode.IsNotNull())
    {
        if (this->m_spSelectedInputNode.IsNotNull())
        {
            validInput = false;

            const mitk::MAPRegistrationWrapper* wrapper = dynamic_cast < const mitk::MAPRegistrationWrapper* >
                (m_spSelectedRegNode->GetData());
            mitk::Image* inputImage = dynamic_cast<mitk::Image*>(m_spSelectedInputNode->GetData());

            if (inputImage && wrapper)
            {
                if (inputImage->GetDimension() - 1 == wrapper->GetMovingDimensions()
                    && inputImage->GetTimeSteps() > 1)
                {
                    //images has multiple time steps and a time step has the correct dimensionality
                    validInput = true;
                }

                if (inputImage->GetDimension() == wrapper->GetMovingDimensions())
                {
                    validInput = true;
                }
            }

            if (this->IsPointSetInput() && wrapper)
            {
                if (wrapper->GetMovingDimensions() == 3)
                {
                    validInput = true;
                }
            }
        }

        if (this->m_spSelectedRefNode.IsNotNull())
        {
            validRef = false;

            const mitk::MAPRegistrationWrapper* wrapper = dynamic_cast < const mitk::MAPRegistrationWrapper* >
                (m_spSelectedRegNode->GetData());
            mitk::BaseGeometry* geometry = NULL;

            if (m_spSelectedRefNode->GetData())
            {
                geometry = m_spSelectedRefNode->GetData()->GetGeometry();
            }

            if (geometry && wrapper)
            {
                if (wrapper->GetTargetDimensions() == 3)
                {
                    validRef = true;
                }
                else if (wrapper->GetTargetDimensions() == 2)
                {
                    mitk::BaseGeometry::BoundsArrayType bounds = geometry->GetBounds();

                    if (bounds[4] != 0 || bounds[5] != 0)
                    {
                        //array "bounds" is constructed as [min Dim1, max Dim1, min Dim2, max Dim2, min Dim3, max Dim3]
                        //therfore [4] and [5] must be 0

                        validRef = true;
                    }

                }
            }
        }
        else if (this->IsPointSetInput())
        {
            validRef = true;
        }
    }
}

void QmitkMatchPointMapper::ConfigureMappingControls()
{
    this->m_Controls.m_pbMap->setEnabled(this->m_ValidInput  && this->m_ValidRef && (this->m_ValidReg || this->m_spSelectedRegNode.IsNull()));
    this->m_Controls.m_pbRefine->setEnabled(this->m_ValidInput && this->m_ValidReg
        && this->IsAbleToRefineGeometry() && !this->IsPointSetInput());
    this->m_Controls.m_pbLockRef->setEnabled(this->m_Controls.m_cbManualRef->isChecked());
    this->m_Controls.m_lbReferenceName->setEnabled(this->m_Controls.m_cbManualRef->isChecked());

    if (this->m_ValidInput && this->m_ValidReg)
    {
        this->m_Controls.m_leMappedName->setText(tr("mapped_by_") + QString::fromStdString(
            m_spSelectedRegNode->GetName()));
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
                QString("<font color='gray'><i>Binary input (mask) detected. Preparing for mask mapping (default interpolation: nearest neigbour; padding value: 0)</i></font>"));

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

void QmitkMatchPointMapper::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
    const QList<mitk::DataNode::Pointer>& nodes)
{
    this->CheckInputs();
    this->ConfigureMappingControls();
}

void QmitkMatchPointMapper::OnLockRegButtonPushed()
{
    if (this->m_Controls.m_pbLockReg->isChecked())
    {
        if (this->m_spSelectedRegNode.IsNotNull())
        {
            this->m_spSelectedRegNode->SetSelected(false);
            this->GetDataStorage()->Modified();
        }
    }

    this->CheckInputs();
    this->ConfigureMappingControls();
}

void QmitkMatchPointMapper::OnLockInputButtonPushed()
{
    if (this->m_Controls.m_pbLockInput->isChecked())
    {
        if (this->m_spSelectedInputNode.IsNotNull())
        {
            this->m_spSelectedInputNode->SetSelected(false);
            this->GetDataStorage()->Modified();
        }
    }

    this->CheckInputs();
    this->ConfigureMappingControls();
}

void QmitkMatchPointMapper::OnLockReferenceButtonPushed()
{
    if (this->m_Controls.m_pbLockRef->isChecked())
    {
        if (this->m_spSelectedRefNode.IsNotNull())
        {
            this->m_spSelectedRefNode->SetSelected(false);
            this->GetDataStorage()->Modified();
        }
    }

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
            QString("<font color='gray'><i>No registration selected. Preforming mapping with identity transform</i></font>"));
    }

    if (!doGeometryRefinement)
    {
        pJob->m_spRefGeometry = m_spSelectedRefNode->GetData()->GetGeometry()->Clone().GetPointer();

        //check for super/sub sampling
        if (m_Controls.m_groupActivateSampling->isChecked())
        {
            //change the pixel count and  spacing of the geometry
            mitk::BaseGeometry::BoundsArrayType geoBounds = pJob->m_spRefGeometry->GetBounds();
            mitk::Vector3D geoSpacing = pJob->m_spRefGeometry->GetSpacing();

            geoSpacing[0] = geoSpacing[0] / m_Controls.m_sbXFactor->value();
            geoSpacing[1] = geoSpacing[1] / m_Controls.m_sbYFactor->value();
            geoSpacing[2] = geoSpacing[2] / m_Controls.m_sbZFactor->value();

            geoBounds[1] = geoBounds[1] * m_Controls.m_sbXFactor->value();
            geoBounds[3] = geoBounds[3] * m_Controls.m_sbYFactor->value();
            geoBounds[5] = geoBounds[5] * m_Controls.m_sbZFactor->value();

            pJob->m_spRefGeometry->SetBounds(geoBounds);
            pJob->m_spRefGeometry->SetSpacing(geoSpacing);
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

    m_Controls.m_teLog->append(QString("<b><font color='blue'>Started mapping job. Name: ") +
        m_Controls.m_leMappedName->text() + QString("</font></b>"));

    QThreadPool* threadPool = QThreadPool::globalInstance();
    threadPool->start(pJob);
}



void QmitkMatchPointMapper::OnMapJobError(QString err)
{
    Error(err);
};

void QmitkMatchPointMapper::OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData,
    const QmitkMappingJob* job)
{
    m_Controls.m_teLog->append(QString("<b><font color='blue'>Mapped entity stored. Name: ") +
        QString::fromStdString(job->m_MappedName) + QString("</font></b>"));

    mitk::DataNode::Pointer spMappedNode = mitk::generateMappedResultNode(job->m_MappedName,
        spMappedData, job->GetRegistration()->getRegistrationUID(), job->m_InputDataUID,
        job->m_doGeometryRefinement, job->m_InterpolatorLabel);
    this->GetDataStorage()->Add(spMappedNode);
    this->GetRenderWindowPart()->RequestUpdate();

    this->CheckInputs();
    this->ConfigureMappingControls();
};

void QmitkMatchPointMapper::OnMappingInfo(QString info)
{
    m_Controls.m_teLog->append(QString("<font color='gray'><i>") + info + QString("</i></font>"));
};

void QmitkMatchPointMapper::OnXFactorChanged(double d)
{
    if (m_Controls.m_cbLinkFactors->isChecked())
    {
        this->m_Controls.m_sbYFactor->setValue(d);
        this->m_Controls.m_sbZFactor->setValue(d);
    }
}
