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

#include <mitkInteractionPositionEvent.h>
#include <sofa/component/collision/AttachBodyPerformer.h>
#include <sofa/component/collision/ComponentMouseInteraction.h>
#include <sofa/component/collision/FixParticlePerformer.h>
#include <sofa/component/collision/RayContact.h>
#include <sofa/component/collision/RayModel.h>
#include <sofa/component/container/MechanicalObject.h>
#include <sofa/simulation/common/CollisionVisitor.h>
#include <sofa/simulation/common/DeleteVisitor.h>
#include <sofa/simulation/common/MechanicalVisitor.h>
#include <sofa/simulation/common/Node.h>
#include <vtkCamera.h>
#include "mitkSimulation.h"
#include "mitkSimulationInteractor.h"

using sofa::component::collision::AttachBodyPerformer;
using sofa::component::collision::BaseRayContact;
using sofa::component::collision::BodyPicked;
using sofa::component::collision::ComponentMouseInteraction;
using sofa::component::collision::FixParticlePerformerConfiguration;
using sofa::component::collision::InteractionPerformer;
using sofa::component::collision::Ray;
using sofa::component::collision::RayModel;
using sofa::component::container::MechanicalObject;
using sofa::core::behavior::BaseMechanicalState;
using sofa::core::collision::DetectionOutput;
using sofa::core::collision::Pipeline;
using sofa::core::objectmodel::BaseContext;
using sofa::core::CollisionElementIterator;
using sofa::core::ExecParams;
using sofa::core::MechanicalParams;
using sofa::core::VecCoordId;
using sofa::defaulttype::dot;
using sofa::defaulttype::Vec3d;
using sofa::defaulttype::Vec3Types;
using sofa::helper::vector;
using sofa::simulation::CollisionVisitor;
using sofa::simulation::DeleteVisitor;
using sofa::simulation::MechanicalPickParticlesVisitor;
using sofa::simulation::MechanicalPropagatePositionVisitor;
using sofa::simulation::Node;

namespace mitk
{
  class SimulationInteractor::Impl
  {
  public:
    Impl();
    ~Impl();

    void Initialize(Node::SPtr rootNode);
    void Uninitialize();
    void AttachMouseNode();
    void DetachMouseNode();
    bool IsInteractionPerformerNotNull() const;
    void UpdatePickRay(InteractionPositionEvent* event);
    void FindCollision();
    void AttachCompatibleInteraction();
    void DetachInteraction(bool setNull);
    void StartInteraction(const std::string& type);
    void ExecuteInteraction();
    void StopInteraction();

  private:
    Impl(const Impl&);
    Impl& operator=(const Impl&);

    BodyPicked FindCollisionUsingPipeline();
    BodyPicked FindCollisionUsingBruteForce();
    void ConfigureInteractionPerformer();

    Node::SPtr m_RootNode;
    Node::SPtr m_MouseNode;
    MechanicalObject<Vec3Types>::SPtr m_PickRayContainer;
    RayModel::SPtr m_PickRayModel;
    std::vector<ComponentMouseInteraction*> m_InteractionComponents;
    ComponentMouseInteraction* m_Interaction;
    std::auto_ptr<InteractionPerformer> m_InteractionPerformer;
    bool m_IsMouseNodeAttached;
    bool m_UseCollisionPipeline;
    BodyPicked m_LastBodyPicked;
  };
}

mitk::SimulationInteractor::Impl::Impl()
  : m_Interaction(NULL),
    m_IsMouseNodeAttached(false),
    m_UseCollisionPipeline(true)
{
}

mitk::SimulationInteractor::Impl::~Impl()
{
  this->Uninitialize();
}

void mitk::SimulationInteractor::Impl::Initialize(const Node::SPtr rootNode)
{
  this->Uninitialize();

  m_RootNode = rootNode;

  m_PickRayContainer = sofa::core::objectmodel::New<MechanicalObject<Vec3Types> >();
  m_PickRayContainer->setName("MousePosition");
  m_PickRayContainer->resize(1);

  m_PickRayModel = sofa::core::objectmodel::New<RayModel>();
  m_PickRayModel->setName("MouseCollisionModel");
  m_PickRayModel->setNbRay(1);

  m_MouseNode = rootNode->createChild("Mouse");
  m_MouseNode->addObject(m_PickRayContainer);
  m_MouseNode->addObject(m_PickRayModel);

  m_MouseNode->init(ExecParams::defaultInstance());
  m_PickRayContainer->init();
  m_PickRayModel->init();

  typedef ComponentMouseInteraction::ComponentMouseInteractionFactory Factory;
  const Factory* factory = Factory::getInstance();
  
  for (Factory::const_iterator it = factory->begin(); it != factory->end(); ++it)
    m_InteractionComponents.push_back(it->second->createInstance(NULL));

  m_MouseNode->detachFromGraph();

  Pipeline* collisionPipeline;
  rootNode->getContext()->get(collisionPipeline, BaseContext::SearchRoot);

  m_UseCollisionPipeline = collisionPipeline != NULL;
}

void mitk::SimulationInteractor::Impl::Uninitialize()
{
  this->DetachMouseNode();

  if (!m_InteractionComponents.empty())
  {
    for (std::vector<ComponentMouseInteraction*>::iterator it = m_InteractionComponents.begin(); it != m_InteractionComponents.end(); ++it)
      delete *it;

    m_InteractionComponents.clear();
    m_Interaction = NULL;
  }

  if (m_MouseNode)
  {
    m_MouseNode->execute<DeleteVisitor>(ExecParams::defaultInstance());

    m_PickRayModel.reset();
    m_PickRayContainer.reset();
    m_MouseNode.reset();
    m_RootNode.reset();    
  }
}

void mitk::SimulationInteractor::Impl::AttachMouseNode()
{
  if (!m_IsMouseNodeAttached)
  {
    m_RootNode->addChild(m_MouseNode);
    m_IsMouseNodeAttached = true;
  }
}

void mitk::SimulationInteractor::Impl::DetachMouseNode()
{
  if (m_IsMouseNodeAttached)
  {
    this->DetachInteraction(false);
    m_MouseNode->detachFromGraph();
    m_IsMouseNodeAttached = false;
  }
}

bool mitk::SimulationInteractor::Impl::IsInteractionPerformerNotNull() const
{
  return m_InteractionPerformer.get() != NULL;
}

void mitk::SimulationInteractor::Impl::UpdatePickRay(InteractionPositionEvent* event)
{
  if (!m_IsMouseNodeAttached)
    return;

  vtkCamera* camera = event->GetSender()->GetVtkRenderer()->GetActiveCamera();

  Vec3d cameraOrigin(camera->GetPosition());
  Vec3d pickedPosition(event->GetPositionInWorld().GetDataPointer());
  Vec3d pickRayDirection(pickedPosition - cameraOrigin);
  Vec3d focalPoint(camera->GetFocalPoint());

  Vec3d cameraDirection(focalPoint - cameraOrigin);
  cameraDirection.normalize();

  std::pair<double, double> clippingRange;
  camera->GetClippingRange(clippingRange.first, clippingRange.second);

  double dotProduct = dot(cameraDirection, pickRayDirection);
  double norm = pickRayDirection.norm();

  clippingRange.first = clippingRange.first / dotProduct * norm;
  clippingRange.second = clippingRange.second / dotProduct * norm;

  pickRayDirection.normalize();

  Ray pickRay = m_PickRayModel->getRay(0);
  pickRay.setOrigin(cameraOrigin + pickRayDirection * clippingRange.first);
  pickRay.setDirection(pickRayDirection);
  pickRay.setL(clippingRange.second - clippingRange.first);

  MechanicalPropagatePositionVisitor(MechanicalParams::defaultInstance(), 0, VecCoordId::position(), true)
    .execute(m_PickRayModel->getContext());

  MechanicalPropagatePositionVisitor(MechanicalParams::defaultInstance(), 0, VecCoordId::freePosition(), true)
    .execute(m_PickRayModel->getContext());
}

void mitk::SimulationInteractor::Impl::FindCollision()
{
  CollisionVisitor(ExecParams::defaultInstance()).execute(m_RootNode->getContext());

  if (m_UseCollisionPipeline)
  {
    m_LastBodyPicked = this->FindCollisionUsingPipeline();

    if (m_LastBodyPicked.body != NULL)
      return;
  }

  m_LastBodyPicked = this->FindCollisionUsingBruteForce();
}

BodyPicked mitk::SimulationInteractor::Impl::FindCollisionUsingPipeline()
{
  BodyPicked bodyPicked;

  Ray ray = m_PickRayModel->getRay(0);
  const Vec3d& origin = ray.origin();
  const Vec3d& direction = ray.direction();
  const double length = ray.l();

  const std::set<BaseRayContact*>& contacts = m_PickRayModel->getContacts();

  for (std::set<BaseRayContact*>::const_iterator contact = contacts.begin(); contact != contacts.end(); ++contact)
  {
    const vector<DetectionOutput*>& detectionOutputs = (*contact)->getDetectionOutputs();

    for (vector<DetectionOutput*>::const_iterator detectionOutput = detectionOutputs.begin(); detectionOutput != detectionOutputs.end(); ++detectionOutput)
    {
      CollisionElementIterator collisionElement;
      int pointIndex;

      if ((*detectionOutput)->elem.first.getCollisionModel() == m_PickRayModel)
      {
        collisionElement = (*detectionOutput)->elem.second;
        pointIndex = 1;
      }
      else if ((*detectionOutput)->elem.second.getCollisionModel() == m_PickRayModel)
      {
        collisionElement = (*detectionOutput)->elem.first;
        pointIndex = 0;
      }
      else
      {
        continue;
      }

      if (!collisionElement.getCollisionModel()->isSimulated())
        continue;

      const double t = ((*detectionOutput)->point[pointIndex] - origin) * direction;

      if (t < 0.0 || t > length)
        continue;

      if (bodyPicked.body == NULL || t < bodyPicked.rayLength)
      {
        bodyPicked.body = collisionElement.getCollisionModel();
        bodyPicked.indexCollisionElement = collisionElement.getIndex();
        bodyPicked.point = (*detectionOutput)->point[pointIndex];
        bodyPicked.dist = ((*detectionOutput)->point[1] - (*detectionOutput)->point[0]).norm();
        bodyPicked.rayLength = t;
      }
    }
  }

  return bodyPicked;
}

BodyPicked mitk::SimulationInteractor::Impl::FindCollisionUsingBruteForce()
{
  BodyPicked bodyPicked;

  Ray ray = m_PickRayModel->getRay(0);
  const Vec3d& origin = ray.origin();
  const Vec3d& direction = ray.direction();
  const double length = ray.l();

  MechanicalPickParticlesVisitor pickVisitor(ExecParams::defaultInstance(), origin, direction, length);
  pickVisitor.execute(m_RootNode->getContext());

  if (!pickVisitor.particles.empty())
  {
    bodyPicked.mstate = pickVisitor.particles.begin()->second.first;
    bodyPicked.indexCollisionElement = pickVisitor.particles.begin()->second.second;
    bodyPicked.point[0] = bodyPicked.mstate->getPX(bodyPicked.indexCollisionElement);
    bodyPicked.point[1] = bodyPicked.mstate->getPY(bodyPicked.indexCollisionElement);
    bodyPicked.point[2] = bodyPicked.mstate->getPZ(bodyPicked.indexCollisionElement);
    bodyPicked.dist = 0;
    bodyPicked.rayLength = (bodyPicked.point - origin) * direction;
  }

  return bodyPicked;
}

void mitk::SimulationInteractor::Impl::AttachCompatibleInteraction()
{
  BaseContext* context;

  if (m_LastBodyPicked.body == NULL)
  {
    context = m_LastBodyPicked.mstate != NULL
      ? m_LastBodyPicked.mstate->getContext()
      : NULL;
  }
  else
  {
    context = m_LastBodyPicked.body->getContext();
  }

  if (context != NULL)
  {
    if (m_Interaction == NULL || !m_Interaction->isCompatible(context))
    {
      bool foundCompatibleInteractor = false;

      for (std::vector<ComponentMouseInteraction*>::const_iterator it = m_InteractionComponents.begin(); it != m_InteractionComponents.end(); ++it)
      {
        if (*it != m_Interaction && (*it)->isCompatible(context))
        {
          this->DetachInteraction(false);
          m_Interaction = *it;
          m_Interaction->attach(m_MouseNode.get());

          foundCompatibleInteractor = true;
          break;
        }
      }

      if (!foundCompatibleInteractor)
        this->DetachInteraction(true);
    }
  }
  else
  {
    this->DetachInteraction(true);
  }

  if (m_Interaction != NULL)
  {
    m_Interaction->mouseInteractor->setMouseRayModel(m_PickRayModel.get());
    m_Interaction->mouseInteractor->setBodyPicked(m_LastBodyPicked);
  }
}

void mitk::SimulationInteractor::Impl::DetachInteraction(bool setNull)
{
  if (m_Interaction != NULL)
  {
    m_Interaction->detach();

    if (setNull)
      m_Interaction = NULL;
  }
}

void mitk::SimulationInteractor::Impl::StartInteraction(const std::string& type)
{
  if (m_Interaction == NULL)
    return;

  InteractionPerformer::InteractionPerformerFactory* factory = InteractionPerformer::InteractionPerformerFactory::getInstance();
  m_InteractionPerformer.reset(factory->createObject(type, m_Interaction->mouseInteractor.get()));

  if (m_InteractionPerformer.get() != NULL)
  {
    this->ConfigureInteractionPerformer();
    m_Interaction->mouseInteractor->addInteractionPerformer(m_InteractionPerformer.get());
    m_InteractionPerformer->start();
  }
}

void mitk::SimulationInteractor::Impl::ConfigureInteractionPerformer()
{
  AttachBodyPerformer<Vec3Types>* attachBodyPerformer = dynamic_cast<AttachBodyPerformer<Vec3Types>*>(m_InteractionPerformer.get());

  if (attachBodyPerformer != NULL)
  {
    attachBodyPerformer->setStiffness(1000);
    attachBodyPerformer->setArrowSize(0);
    attachBodyPerformer->setShowFactorSize(1);
    return;
  }

  FixParticlePerformerConfiguration* fixParticlePerformer = dynamic_cast<FixParticlePerformerConfiguration*>(m_InteractionPerformer.get());

  if (fixParticlePerformer != NULL)
    fixParticlePerformer->setStiffness(10000);
}

void mitk::SimulationInteractor::Impl::ExecuteInteraction()
{
  if (m_InteractionPerformer.get() == NULL)
    return;

  m_InteractionPerformer->execute();
}

void mitk::SimulationInteractor::Impl::StopInteraction()
{
  if (m_InteractionPerformer.get() == NULL)
    return;

  AttachBodyPerformer<Vec3Types>* attachBodyPerformer = dynamic_cast<AttachBodyPerformer<Vec3Types>*>(m_InteractionPerformer.get());

  if (attachBodyPerformer != NULL)
    attachBodyPerformer->clear();

  m_Interaction->mouseInteractor->removeInteractionPerformer(m_InteractionPerformer.get());
  m_InteractionPerformer.release();
}

mitk::SimulationInteractor::SimulationInteractor()
  : m_Impl(new Impl)
{
}

mitk::SimulationInteractor::~SimulationInteractor()
{
}

void mitk::SimulationInteractor::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("startPrimaryInteraction", StartPrimaryInteraction);
  CONNECT_FUNCTION("startSecondaryInteraction", StartSecondaryInteraction);
  CONNECT_FUNCTION("stopInteraction", StopInteraction);
  CONNECT_FUNCTION("executeInteraction", ExecuteInteraction);
  CONNECT_CONDITION("isInteractionPerformerNotNull", IsInteractionPerformerNotNull);
}

void mitk::SimulationInteractor::DataNodeChanged()
{
  this->ResetToStartState();

  NodeType dataNode = this->GetDataNode();

  if (dataNode.IsNotNull())
  {
    Simulation::Pointer simulation = dynamic_cast<Simulation*>(dataNode->GetData());

    if (simulation.IsNotNull())
    {
      m_Impl->Initialize(simulation->GetRootNode());
      return;
    }    
  }

  m_Impl->Uninitialize();
}

void mitk::SimulationInteractor::StartInteraction(const std::string& type, InteractionPositionEvent* event)
{
  m_Impl->AttachMouseNode();
  m_Impl->UpdatePickRay(event);
  m_Impl->FindCollision();
  m_Impl->AttachCompatibleInteraction();
  m_Impl->StartInteraction(type);
}

bool mitk::SimulationInteractor::StartPrimaryInteraction(StateMachineAction*, InteractionEvent* event)
{
  this->StartInteraction("AttachBody", dynamic_cast<InteractionPositionEvent*>(event));
  return true;
}

bool mitk::SimulationInteractor::StartSecondaryInteraction(StateMachineAction*, InteractionEvent* event)
{
  this->StartInteraction("FixParticle", dynamic_cast<InteractionPositionEvent*>(event));
  return true;
}

bool mitk::SimulationInteractor::StopInteraction(StateMachineAction*, InteractionEvent*)
{
  m_Impl->StopInteraction();
  m_Impl->DetachMouseNode();
  return true;
}

bool mitk::SimulationInteractor::ExecuteInteraction(StateMachineAction*, InteractionEvent* event)
{
  m_Impl->UpdatePickRay(dynamic_cast<InteractionPositionEvent*>(event));
  m_Impl->ExecuteInteraction();
  return true;
}

bool mitk::SimulationInteractor::IsInteractionPerformerNotNull(const InteractionEvent*)
{
  return m_Impl->IsInteractionPerformerNotNull();
}
