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

#include "mitkStandaloneDataStorage.h"
#include "mitkDataNode.h"
#include "mitkTestInteractor.h"
#include "mitkVtkPropRenderer.h"
#include "mitkTestingMacros.h"
#include "mitkGlobalInteraction.h"
#include "mitkMousePressEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkInteractionEventConst.h"

// us
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include "mitkModuleResourceStream.h"
#include "mitkModuleRegistry.h"

int mitkDataInteractorTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("DataInteractor")

  /*
   * Objective: Testing the processing of user input by DataInteractors
   *
   * by     creating Renderer,DataStorage,DataNode and DataInteractor
   *        registering DataNode with Interactor, then
   *
   *        sending user input to the Dispatcher, and
   *
   * check  if DataNode changes according to user input and statemachine.
   *
   */

  // Global interaction must(!) be initialized if used, needed for RenderWindow

  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  // Here BindDispatcherInteractor and Dispatcher should be created automatically
  vtkRenderWindow* renWin = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer renderer = mitk::VtkPropRenderer::New( "ContourRenderer",renWin, mitk::RenderingManager::GetInstance() );

  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  mitk::DataNode::Pointer dn = mitk::DataNode::New();
  mitk::DataNode::Pointer dn2 = mitk::DataNode::New();
  mitk::TestInteractor::Pointer interactor = mitk::TestInteractor::New();



  interactor->LoadStateMachine("Tests/AddAndRemovePoints.xml");

  // execption für file exist nicht, und Modulname exist nicht
  // get modul in LoadConfig integrieren

  // zum laden mit methode für input stream benutzen (vtkXmlparser=
  // das ganze in die LoadMethode mit rein machen, damit user das GetModule krams erpsart bleibt
  // also modul context Mitk defaulten, wenn nicht angegeben

  // side note:
  // Informer krams, rollen sind vertauscht
  // interface machen das eventobserver registrieren lässtm
  // der jetzt-informer service,  hört nur auf registrier events und upateded seine liste dann


  mitk::Module* module = mitk::ModuleRegistry::GetModule("Mitk");
  MITK_TEST_CONDITION_REQUIRED(
      module != NULL, "01 Check that Module exists");
  mitk::ModuleResource resource =  module->GetResource("Interactions/globalConfig.xml");

  MITK_TEST_CONDITION_REQUIRED(
        resource.IsValid() == true, "02 Check if resource is valid");

  mitk::ModuleResourceStream stream(resource);

  //interactor->LoadEventConfig(&stream); //neu
  interactor->LoadEventConfig("Tests/globalConfig.xml");
  interactor->SetDataNode(dn);
  renderer->SetDataStorage(ds);
  ds->Add(dn);

  // Now generate events and see if interactor reacts:

    mitk::MouseButtons buttonStates = mitk::NoButton;
    mitk::MouseButtons eventButton = mitk::LeftMouseButton;
    mitk::ModifierKeys modifiers = mitk::NoKey;

    mitk::Point2D point;
    point[0] = 17;
    point[1] = 170;

    // MousePress Events
    mitk::MousePressEvent::Pointer me1 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);
    mitk::MouseMoveEvent::Pointer mm1 = mitk::MouseMoveEvent::New(renderer,point, buttonStates, modifiers);
    renderer->GetDispatcher()->ProcessEvent(me1.GetPointer());
    renderer->GetDispatcher()->ProcessEvent(me1.GetPointer());
    renderer->GetDispatcher()->ProcessEvent(mm1.GetPointer());
    renderer->GetDispatcher()->ProcessEvent(me1.GetPointer());
    renderer->GetDispatcher()->ProcessEvent(me1.GetPointer());


  renWin->Delete();

  MITK_TEST_END()

}
