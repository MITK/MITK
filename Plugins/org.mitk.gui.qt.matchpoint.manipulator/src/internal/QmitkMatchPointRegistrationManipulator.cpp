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

// Mitk
#include <mitkStatusBar.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkMAPRegistrationWrapper.h>
#include "mitkRegVisPropertyTags.h"
#include "mitkMatchPointPropertyTags.h"
#include "mitkRegEvaluationObject.h"
#include "mitkRegistrationHelper.h"
#include "mitkRegEvaluationMapper2D.h"
#include <mitkAlgorithmHelper.h>
#include <mitkResultNodeGenerationHelper.h>
#include <mitkUIDHelper.h>

// Qmitk
#include "QmitkRenderWindow.h"
#include "QmitkMatchPointRegistrationManipulator.h"
#include <QmitkMappingJob.h>

// Qt
#include <QMessageBox>
#include <QErrorMessage>
#include <QTimer>
#include <QThreadPool>

//MatchPoint
#include <mapRegistrationManipulator.h>
#include <mapPreCachedRegistrationKernel.h>
#include <mapCombinedRegistrationKernel.h>
#include <mapNullRegistrationKernel.h>
#include <mapRegistrationCombinator.h>

#include <itkCompositeTransform.h>

#include <boost/math/constants/constants.hpp>

const std::string QmitkMatchPointRegistrationManipulator::VIEW_ID =
    "org.mitk.views.matchpoint.registration.manipulator";

QmitkMatchPointRegistrationManipulator::QmitkMatchPointRegistrationManipulator()
  : m_Parent(NULL), m_activeManipulation(false), m_autoMoving(false), m_autoTarget(false), m_currentSelectedTimeStep(0), HelperNodeName("RegistrationManipulationEvaluationHelper"),
   m_internalUpdate(false)
{
  m_currentSelectedPosition.Fill(0.0);
}

QmitkMatchPointRegistrationManipulator::~QmitkMatchPointRegistrationManipulator()
{
  if (this->m_EvalNode.IsNotNull() && this->GetDataStorage().IsNotNull())
  {
    this->GetDataStorage()->Remove(this->m_EvalNode);
  }
}

void QmitkMatchPointRegistrationManipulator::SetFocus()
{

}

void QmitkMatchPointRegistrationManipulator::Error(QString msg)
{
	mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
	MITK_ERROR << msg.toStdString().c_str();
}

void QmitkMatchPointRegistrationManipulator::CreateQtPartControl(QWidget* parent)
{
	// create GUI widgets from the Qt Designer's .ui file
	m_Controls.setupUi(parent);

	m_Parent = parent;

  connect(m_Controls.pbStart, SIGNAL(clicked()), this, SLOT(OnStartBtnPushed()));
  connect(m_Controls.pbCancel, SIGNAL(clicked()), this, SLOT(OnCancelBtnPushed()));
  connect(m_Controls.pbStore, SIGNAL(clicked()), this, SLOT(OnStoreBtnPushed()));
  connect(m_Controls.evalSettings, SIGNAL(SettingsChanged(mitk::DataNode*)), this, SLOT(OnSettingsChanged(mitk::DataNode*)));
  connect(m_Controls.radioSelectedReg, SIGNAL(toggled(bool)), m_Controls.lbRegistrationName, SLOT(setVisible(bool)));

  connect(m_Controls.slideRotX, SIGNAL(valueChanged(int)), this, SLOT(OnRotXSlideChanged(int)));
  connect(m_Controls.sbRotX, SIGNAL(valueChanged(double)), this, SLOT(OnRotXChanged(double)));
  connect(m_Controls.slideRotY, SIGNAL(valueChanged(int)), this, SLOT(OnRotYSlideChanged(int)));
  connect(m_Controls.sbRotY, SIGNAL(valueChanged(double)), this, SLOT(OnRotYChanged(double)));
  connect(m_Controls.slideRotZ, SIGNAL(valueChanged(int)), this, SLOT(OnRotZSlideChanged(int)));
  connect(m_Controls.sbRotZ, SIGNAL(valueChanged(double)), this, SLOT(OnRotZChanged(double)));

  connect(m_Controls.slideTransX, SIGNAL(valueChanged(int)), this, SLOT(OnTransXSlideChanged(int)));
  connect(m_Controls.sbTransX, SIGNAL(valueChanged(double)), this, SLOT(OnTransXChanged(double)));
  connect(m_Controls.slideTransY, SIGNAL(valueChanged(int)), this, SLOT(OnTransYSlideChanged(int)));
  connect(m_Controls.sbTransY, SIGNAL(valueChanged(double)), this, SLOT(OnTransYChanged(double)));
  connect(m_Controls.slideTransZ, SIGNAL(valueChanged(int)), this, SLOT(OnTransZSlideChanged(int)));
  connect(m_Controls.sbTransZ, SIGNAL(valueChanged(double)), this, SLOT(OnTransZChanged(double)));

  connect(m_Controls.comboCenter, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCenterTypeChanged(int)));


  this->m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));

  m_Controls.radioNewReg->setChecked(true);
  m_Controls.groupScale->setVisible(false);
  m_Controls.lbRegistrationName->setVisible(false);

  m_EvalNode = this->GetDataStorage()->GetNamedNode(HelperNodeName);

  this->CheckInputs();
  this->StopSession();
	this->ConfigureControls();
}

void QmitkMatchPointRegistrationManipulator::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceChangeListener.RenderWindowPartActivated(renderWindowPart);
}

void QmitkMatchPointRegistrationManipulator::RenderWindowPartDeactivated(
  mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceChangeListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkMatchPointRegistrationManipulator::CheckInputs()
{
  if (!m_activeManipulation)
  {
  QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();
  this->m_autoMoving = false;
  this->m_autoTarget = false;
  this->m_SelectedMovingNode = NULL;
  this->m_SelectedTargetNode = NULL;
  this->m_SelectedPreRegNode = NULL;
  this->m_SelectedPreReg = NULL;

  if (dataNodes.size() > 0)
  {
    //test if auto select works
    if (mitk::MITKRegistrationHelper::IsRegNode(dataNodes[0]))
    {
      mitk::DataNode::Pointer regNode = dataNodes[0];
      dataNodes.pop_front();

      mitk::MAPRegistrationWrapper* regWrapper = dynamic_cast<mitk::MAPRegistrationWrapper*>(regNode->GetData());
      if (regWrapper)
      {
        this->m_SelectedPreReg = dynamic_cast<const MAPRegistrationType*>(regWrapper->GetRegistration());
      }

      if (this->m_SelectedPreReg.IsNotNull())
      {
        this->m_SelectedPreRegNode = regNode;

        mitk::BaseProperty* uidProp = m_SelectedPreRegNode->GetProperty(mitk::nodeProp_RegAlgMovingData);

        if (uidProp)
        {
          //search for the moving node
          mitk::NodePredicateProperty::Pointer predicate = mitk::NodePredicateProperty::New(mitk::nodeProp_UID,
            uidProp);
          this->m_SelectedMovingNode = this->GetDataStorage()->GetNode(predicate);
          this->m_autoMoving = this->m_SelectedMovingNode.IsNotNull();
        }

        uidProp = m_SelectedPreRegNode->GetProperty(mitk::nodeProp_RegAlgTargetData);

        if (uidProp)
        {
          //search for the target node
          mitk::NodePredicateProperty::Pointer predicate = mitk::NodePredicateProperty::New(mitk::nodeProp_UID,
            uidProp);
          this->m_SelectedTargetNode = this->GetDataStorage()->GetNode(predicate);
          this->m_autoTarget = this->m_SelectedTargetNode.IsNotNull();
        }
      }
    }

    //if still nodes are selected -> ignore possible auto select
    if (!dataNodes.empty())
    {
      mitk::Image* inputImage = dynamic_cast<mitk::Image*>(dataNodes[0]->GetData());

      if (inputImage)
      {
        this->m_SelectedMovingNode = dataNodes[0];
        this->m_autoMoving = false;
        dataNodes.pop_front();
      }
    }

    if (!dataNodes.empty())
    {
      mitk::Image* inputImage = dynamic_cast<mitk::Image*>(dataNodes[0]->GetData());

      if (inputImage)
      {
        this->m_SelectedTargetNode = dataNodes[0];
        this->m_autoTarget = false;
        dataNodes.pop_front();
      }
    }
  }
  }
}


void QmitkMatchPointRegistrationManipulator::OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
        const QList<mitk::DataNode::Pointer>& nodes)
{
  this->CheckInputs();
	this->ConfigureControls();
};

void QmitkMatchPointRegistrationManipulator::ConfigureControls()
{
  //configure input data widgets
  if (this->m_SelectedPreRegNode.IsNull())
  {
    m_Controls.lbRegistrationName->setText(
        QString("<font color='red'>No registration selected!</font>"));
  }
  else
  {
    m_Controls.lbRegistrationName->setText(QString::fromStdString(
      this->m_SelectedPreRegNode->GetName()));
  }

  if (this->m_SelectedMovingNode.IsNull())
  {
    m_Controls.lbMovingName->setText(QString("<font color='red'>no moving image selected!</font>"));
  }
  else
  {
    if (this->m_autoMoving)
    {
      m_Controls.lbMovingName->setText(QString("<font color='gray'>") + QString::fromStdString(
        this->m_SelectedMovingNode->GetName()) + QString(" (auto selected)</font>"));
    }
    else
    {
      m_Controls.lbMovingName->setText(QString::fromStdString(this->m_SelectedMovingNode->GetName()));
    }
  }

  if (this->m_SelectedTargetNode.IsNull())
  {
    m_Controls.lbTargetName->setText(QString("<font color='red'>no target image selected!</font>"));
  }
  else
  {
    if (this->m_autoTarget)
    {
      m_Controls.lbTargetName->setText(QString("<font color='gray'>") + QString::fromStdString(
        this->m_SelectedTargetNode->GetName()) + QString(" (auto selected)</font>"));
    }
    else
    {
      m_Controls.lbTargetName->setText(QString::fromStdString(this->m_SelectedTargetNode->GetName()));
    }
  }

  if (!m_activeManipulation)
  {
    QString name = "ManuelRegistration";

    if (m_SelectedPreRegNode.IsNotNull())
    {
      name = QString::fromStdString(m_SelectedPreRegNode->GetName()) + " manual refined";
    }
    this->m_Controls.lbNewRegName->setText(name);
  }

  //config settings widget
  this->m_Controls.groupReg->setEnabled(!m_activeManipulation);

  this->m_Controls.lbMovingName->setEnabled(!m_activeManipulation);
  this->m_Controls.lbTargetName->setEnabled(!m_activeManipulation);

  this->m_Controls.pbStart->setEnabled(this->m_SelectedMovingNode.IsNotNull()
    && this->m_SelectedTargetNode.IsNotNull() && !m_activeManipulation);

  this->m_Controls.lbNewRegName->setEnabled(m_activeManipulation);
  this->m_Controls.checkMapEntity->setEnabled(m_activeManipulation);
  this->m_Controls.tabWidget->setEnabled(m_activeManipulation);
  this->m_Controls.pbCancel->setEnabled(m_activeManipulation);
  this->m_Controls.pbStore->setEnabled(m_activeManipulation);

  this->UpdateTransformWidgets();
}

void QmitkMatchPointRegistrationManipulator::InitSession()
{
  this->m_InverseCurrentTransform = TransformType::New();
  this->m_InverseCurrentTransform->SetIdentity();
  this->m_DirectCurrentTransform = TransformType::New();
  this->m_DirectCurrentTransform->SetIdentity();

  this->m_CurrentRegistrationWrapper = mitk::MAPRegistrationWrapper::New();
  m_CurrentRegistration = MAPRegistrationType::New();

  this->m_CurrentRegistrationWrapper->SetRegistration(m_CurrentRegistration);

  ::map::core::RegistrationManipulator<MAPRegistrationType> manipulator(m_CurrentRegistration);
  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
  manipulator.setDirectMapping(::map::core::NullRegistrationKernel < 3, 3 >::New());

  if (this->m_Controls.radioNewReg->isChecked())
  { //new registration
    kernel->setTransformModel(m_InverseCurrentTransform);
    manipulator.setInverseMapping(kernel);

    //init to map the image centers
    auto movingCenter = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
    auto targetCenter = m_SelectedTargetNode->GetData()->GetTimeGeometry()->GetCenterInWorld();

    auto offset = targetCenter - movingCenter;
    m_DirectCurrentTransform->SetOffset(offset);
    m_DirectCurrentTransform->GetInverse(m_InverseCurrentTransform);
  }
  else
  { //use selected pre registration as baseline
    itk::CompositeTransform < ::map::core::continuous::ScalarType, 3>::Pointer compTransform = itk::CompositeTransform < ::map::core::continuous::ScalarType, 3>::New();
    const map::core::RegistrationKernel<3, 3>* preKernel = dynamic_cast<const map::core::RegistrationKernel<3, 3>*>(&this->m_SelectedPreReg->getInverseMapping());
    compTransform->AddTransform(preKernel->getTransformModel()->Clone());
    compTransform->AddTransform(this->m_InverseCurrentTransform);

    kernel->setTransformModel(compTransform);
    manipulator.setInverseMapping(kernel);
  }

  m_Controls.comboCenter->setCurrentIndex(0);
  this->ConfigureTransformCenter(0);

  //set bounds of the translation slider widget to have sensible ranges
  auto currenttrans = m_DirectCurrentTransform->GetTranslation();
  this->m_Controls.slideTransX->setMinimum(currenttrans[0] - 250);
  this->m_Controls.slideTransY->setMinimum(currenttrans[1] - 250);
  this->m_Controls.slideTransZ->setMinimum(currenttrans[2] - 250);
  this->m_Controls.slideTransX->setMaximum(currenttrans[0] + 250);
  this->m_Controls.slideTransY->setMaximum(currenttrans[1] + 250);
  this->m_Controls.slideTransZ->setMaximum(currenttrans[2] + 250);

  //reinit view
  mitk::RenderingManager::GetInstance()->InitializeViews(m_SelectedTargetNode->GetData()->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

  //generate evaluation node
  mitk::RegEvaluationObject::Pointer regEval = mitk::RegEvaluationObject::New();
  regEval->SetRegistration(this->m_CurrentRegistrationWrapper);
  regEval->SetTargetNode(this->m_SelectedTargetNode);
  regEval->SetMovingNode(this->m_SelectedMovingNode);

  this->m_EvalNode = mitk::DataNode::New();
  this->m_EvalNode->SetData(regEval);

  mitk::RegEvaluationMapper2D::SetDefaultProperties(this->m_EvalNode);
  this->m_EvalNode->SetName(HelperNodeName);
  this->m_EvalNode->SetBoolProperty("helper object", true);
  this->GetDataStorage()->Add(this->m_EvalNode);

  this->m_Controls.evalSettings->SetNode(this->m_EvalNode);

  this->m_activeManipulation = true;
};

void QmitkMatchPointRegistrationManipulator::StopSession()
{
  this->m_InverseCurrentTransform = TransformType::New();
  this->m_InverseCurrentTransform->SetIdentity();
  this->m_DirectCurrentTransform = TransformType::New();
  this->m_DirectCurrentTransform->SetIdentity();
  this->m_CurrentRegistration = nullptr;
  this->m_CurrentRegistrationWrapper = nullptr;

  this->GetDataStorage()->Remove(this->m_EvalNode);
  this->m_EvalNode = nullptr;

  this->m_activeManipulation = false;
};

void QmitkMatchPointRegistrationManipulator::UpdateTransformWidgets()
{
  this->m_internalUpdate = true;
  this->m_Controls.sbTransX->setValue(this->m_DirectCurrentTransform->GetTranslation()[0]);
  this->m_Controls.sbTransY->setValue(this->m_DirectCurrentTransform->GetTranslation()[1]);
  this->m_Controls.sbTransZ->setValue(this->m_DirectCurrentTransform->GetTranslation()[2]);
  this->m_Controls.slideTransX->setValue(this->m_DirectCurrentTransform->GetTranslation()[0]);
  this->m_Controls.slideTransY->setValue(this->m_DirectCurrentTransform->GetTranslation()[1]);
  this->m_Controls.slideTransZ->setValue(this->m_DirectCurrentTransform->GetTranslation()[2]);

  this->m_Controls.sbRotX->setValue(this->m_DirectCurrentTransform->GetAngleX()*(180 / boost::math::double_constants::pi));
  this->m_Controls.sbRotY->setValue(this->m_DirectCurrentTransform->GetAngleY()*(180 / boost::math::double_constants::pi));
  this->m_Controls.sbRotZ->setValue(this->m_DirectCurrentTransform->GetAngleZ()*(180 / boost::math::double_constants::pi));
  this->m_Controls.slideRotX->setValue(this->m_DirectCurrentTransform->GetAngleX()*(180 / boost::math::double_constants::pi));
  this->m_Controls.slideRotY->setValue(this->m_DirectCurrentTransform->GetAngleY()*(180 / boost::math::double_constants::pi));
  this->m_Controls.slideRotZ->setValue(this->m_DirectCurrentTransform->GetAngleZ()*(180 / boost::math::double_constants::pi));
  this->m_internalUpdate = false;
};

void QmitkMatchPointRegistrationManipulator::UpdateTransform(bool updateRotation)
{
  if (updateRotation)
  {
    if (m_Controls.comboCenter->currentIndex() == 2)
    {
     ConfigureTransformCenter(2);
    }
    this->m_DirectCurrentTransform->SetRotation(this->m_Controls.sbRotX->value()*(boost::math::double_constants::pi / 180),
      this->m_Controls.sbRotY->value()*(boost::math::double_constants::pi / 180),
      this->m_Controls.sbRotZ->value()*(boost::math::double_constants::pi / 180));
  }
  else
  {
    TransformType::OutputVectorType trans;
    trans[0] = this->m_Controls.sbTransX->value();
    trans[1] = this->m_Controls.sbTransY->value();
    trans[2] = this->m_Controls.sbTransZ->value();

    this->m_DirectCurrentTransform->SetTranslation(trans);
  }

  this->m_DirectCurrentTransform->GetInverse(this->m_InverseCurrentTransform);

  this->UpdateTransformWidgets();

  this->m_EvalNode->Modified();
  this->m_CurrentRegistrationWrapper->Modified();
  this->GetRenderWindowPart()->RequestUpdate();
};

void QmitkMatchPointRegistrationManipulator::OnSliceChanged()
{
  mitk::Point3D currentSelectedPosition = GetRenderWindowPart()->GetSelectedPosition(NULL);
  unsigned int currentSelectedTimeStep = GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();

  if (m_currentSelectedPosition != currentSelectedPosition
    || m_currentSelectedTimeStep != currentSelectedTimeStep
    || m_selectedNodeTime > m_currentPositionTime)
  {
    //the current position has been changed or the selected node has been changed since the last position validation -> check position
    m_currentSelectedPosition = currentSelectedPosition;
    m_currentSelectedTimeStep = currentSelectedTimeStep;
    m_currentPositionTime.Modified();

    if (this->m_EvalNode.IsNotNull())
    {
      this->m_EvalNode->SetProperty(mitk::nodeProp_RegEvalCurrentPosition, mitk::GenericProperty<mitk::Point3D>::New(currentSelectedPosition));
    }

    if (m_activeManipulation && m_Controls.comboCenter->currentIndex() == 2)
    { //update transform with the current position.
      OnCenterTypeChanged(m_Controls.comboCenter->currentIndex());
    }
  }
}

void QmitkMatchPointRegistrationManipulator::OnSettingsChanged(mitk::DataNode*)
{
	this->GetRenderWindowPart()->RequestUpdate();
};

void QmitkMatchPointRegistrationManipulator::OnStartBtnPushed()
{
  this->InitSession();
  this->OnSliceChanged();

  this->GetRenderWindowPart()->RequestUpdate();

  this->CheckInputs();
  this->ConfigureControls();
}

void QmitkMatchPointRegistrationManipulator::OnCancelBtnPushed()
{
  this->StopSession();

  this->CheckInputs();
  this->ConfigureControls();
  this->GetRenderWindowPart()->RequestUpdate();
}

void QmitkMatchPointRegistrationManipulator::OnStoreBtnPushed()
{
  mitk::MAPRegistrationWrapper::Pointer newRegWrapper = mitk::MAPRegistrationWrapper::New();
  MAPRegistrationType::Pointer newReg = MAPRegistrationType::New();

  newRegWrapper->SetRegistration(newReg);

  ::map::core::RegistrationManipulator<MAPRegistrationType> manipulator(newReg);

  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
  kernel->setTransformModel(m_InverseCurrentTransform);

  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel2 = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
  kernel2->setTransformModel(m_InverseCurrentTransform->GetInverseTransform());

  manipulator.setInverseMapping(kernel);
  manipulator.setDirectMapping(kernel2);

  if (this->m_Controls.radioSelectedReg->isChecked())
  { //compine registration with selected pre registration as baseline
    typedef ::map::core::RegistrationCombinator<MAPRegistrationType, MAPRegistrationType> CombinatorType;
    CombinatorType::Pointer combinator = CombinatorType::New();
    newReg = combinator->process(*m_SelectedPreReg,*newReg);
    newRegWrapper->SetRegistration(newReg);
  }

  mitk::DataNode::Pointer spResultRegistrationNode = mitk::generateRegistrationResultNode(
    this->m_Controls.lbNewRegName->text().toStdString(), newRegWrapper, "org.mitk::manual_registration",
    mitk::EnsureUID(m_SelectedMovingNode), mitk::EnsureUID(m_SelectedTargetNode));

  this->GetDataStorage()->Add(spResultRegistrationNode);

  if (m_Controls.checkMapEntity->checkState() == Qt::Checked)
  {
    QmitkMappingJob* pMapJob = new QmitkMappingJob();
    pMapJob->setAutoDelete(true);

    pMapJob->m_spInputData = this->m_SelectedMovingNode->GetData();
    pMapJob->m_InputNodeUID = mitk::EnsureUID(m_SelectedMovingNode);
    pMapJob->m_spRegNode = spResultRegistrationNode;
    pMapJob->m_doGeometryRefinement = false;
    pMapJob->m_spRefGeometry = this->m_SelectedTargetNode->GetData()->GetGeometry()->Clone().GetPointer();

    pMapJob->m_MappedName = this->m_Controls.lbNewRegName->text().toStdString() + std::string(" mapped moving data");
    pMapJob->m_allowUndefPixels = true;
    pMapJob->m_paddingValue = 100;
    pMapJob->m_allowUnregPixels = true;
    pMapJob->m_errorValue = 200;
    pMapJob->m_InterpolatorLabel = "Linear Interpolation";
    pMapJob->m_InterpolatorType = mitk::ImageMappingInterpolator::Linear;

    connect(pMapJob, SIGNAL(Error(QString)), this, SLOT(OnMapJobError(QString)));
    connect(pMapJob, SIGNAL(MapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
      this, SLOT(OnMapResultIsAvailable(mitk::BaseData::Pointer, const QmitkMappingJob*)),
      Qt::BlockingQueuedConnection);

    QThreadPool* threadPool = QThreadPool::globalInstance();
    threadPool->start(pMapJob);
  }

  this->StopSession();

  this->CheckInputs();
  this->ConfigureControls();
  this->GetRenderWindowPart()->RequestUpdate();
}

void QmitkMatchPointRegistrationManipulator::OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData,
  const QmitkMappingJob* job)
{
  mitk::DataNode::Pointer spMappedNode = mitk::generateMappedResultNode(job->m_MappedName,
    spMappedData, job->GetRegistration()->getRegistrationUID(), job->m_InputNodeUID,
    job->m_doGeometryRefinement, job->m_InterpolatorLabel);
  this->GetDataStorage()->Add(spMappedNode);
  this->GetRenderWindowPart()->RequestUpdate();
};


void QmitkMatchPointRegistrationManipulator::OnRotXChanged(double x)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->m_Controls.slideRotX->setValue(x);
    m_internalUpdate = false;
    this->UpdateTransform(true);
  }
};

void QmitkMatchPointRegistrationManipulator::OnRotXSlideChanged(int x)
{
  if (!m_internalUpdate)
  {
    this->m_Controls.sbRotX->setValue(x);
  }
};

void QmitkMatchPointRegistrationManipulator::OnRotYChanged(double y)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->m_Controls.slideRotY->setValue(y);
    m_internalUpdate = false;
    this->UpdateTransform(true);
  }
};

void QmitkMatchPointRegistrationManipulator::OnRotYSlideChanged(int y)
{
  if (!m_internalUpdate)
  {
    this->m_Controls.sbRotY->setValue(y);
  }
};

void QmitkMatchPointRegistrationManipulator::OnRotZChanged(double z)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->m_Controls.slideRotZ->setValue(z);
    m_internalUpdate = false;
    this->UpdateTransform(true);
  }
};

void QmitkMatchPointRegistrationManipulator::OnRotZSlideChanged(int z)
{
  if (!m_internalUpdate)
  {
    this->m_Controls.sbRotZ->setValue(z);
  }
};

void QmitkMatchPointRegistrationManipulator::OnTransXChanged(double x)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->m_Controls.slideTransX->setValue(x);
    m_internalUpdate = false;
    this->UpdateTransform();
  }
};

void QmitkMatchPointRegistrationManipulator::OnTransXSlideChanged(int x)
{
  if (!m_internalUpdate)
  {
    this->m_Controls.sbTransX->setValue(x);
  }
};

void QmitkMatchPointRegistrationManipulator::OnTransYChanged(double y)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->m_Controls.slideTransY->setValue(y);
    m_internalUpdate = false;
    this->UpdateTransform();
  }
};

void QmitkMatchPointRegistrationManipulator::OnTransYSlideChanged(int y)
{
  if (!m_internalUpdate)
  {
    this->m_Controls.sbTransY->setValue(y);
  }
};

void QmitkMatchPointRegistrationManipulator::OnTransZChanged(double z)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->m_Controls.slideTransZ->setValue(z);
    m_internalUpdate = false;
    this->UpdateTransform();
  }
};

void QmitkMatchPointRegistrationManipulator::OnTransZSlideChanged(int z)
{
  if (!m_internalUpdate)
  {
    this->m_Controls.sbTransZ->setValue(z);
  }
};

void QmitkMatchPointRegistrationManipulator::OnCenterTypeChanged(int index)
{
  ConfigureTransformCenter(index);

  this->UpdateTransformWidgets();

  if (this->m_EvalNode.IsNotNull())
  {
    this->m_EvalNode->Modified();
  }
  this->m_CurrentRegistrationWrapper->Modified();
  this->GetRenderWindowPart()->RequestUpdate();
};

void QmitkMatchPointRegistrationManipulator::ConfigureTransformCenter(int centerType)
{
  auto offset = m_DirectCurrentTransform->GetOffset();

  if (centerType == 0)
  { //image center
    auto center = m_SelectedMovingNode->GetData()->GetTimeGeometry()->GetCenterInWorld();
    m_DirectCurrentTransform->SetCenter(center);
  }
  else if (centerType == 1)
  { //world origin
    TransformType::OutputPointType itkCenter;
    itkCenter.Fill(0.0);
    m_DirectCurrentTransform->SetCenter(itkCenter);
  }
  else
  { //current selected point
    auto newCenter = m_InverseCurrentTransform->TransformPoint(m_currentSelectedPosition);
    m_DirectCurrentTransform->SetCenter(newCenter);
  }

  m_DirectCurrentTransform->SetOffset(offset);
  m_DirectCurrentTransform->GetInverse(m_InverseCurrentTransform);
};
