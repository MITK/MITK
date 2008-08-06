/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "mitkMaterialProperty.h"
#include "mitkBaseProperty.h"
#include "mitkBaseRenderer.h"
#include "mitkTestingMacros.h"
#include "mitkDataTreeNode.h"
#include <mitkVtkPropRenderer.h>

#include <iostream>

/**
 *  Simple example for a test for the (non-existent) class "ClassName".
 *
 *  argc and argv are the command line parameters which were passed to
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
mitk::DataTreeNode::Pointer node = NULL;
mitk::MaterialProperty::Pointer myMP = NULL;




void testConstructorWithNodeAndRenderer();
void testConstructorWithColorOpacityNodeAndRenderer();
void testConstructorWithRedGreenBlueOpacityNodeAndRenderer();
void testConstructorRedGreenBlueColorCoefficientSpecularCoefficientSpecularPowerOpacityNodeAndRenderer();
void testConstructorColorColorCoefficientSpecularCoefficientSpecularPowerOpacityNodeAndRenderer();
void testConstructorPropertyRedGreenBlueOpacityAndName();


int
mitkMaterialPropertyTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("MaterialProperty")
      
      node = mitk::DataTreeNode::New();

      testConstructorWithNodeAndRenderer();
      testConstructorWithColorOpacityNodeAndRenderer();
      testConstructorWithRedGreenBlueOpacityNodeAndRenderer();
      testConstructorRedGreenBlueColorCoefficientSpecularCoefficientSpecularPowerOpacityNodeAndRenderer();
      testConstructorColorColorCoefficientSpecularCoefficientSpecularPowerOpacityNodeAndRenderer();
      testConstructorPropertyRedGreenBlueOpacityAndName();

      // first test: did this work?
      // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
      // it makes no sense to continue without an object.

      // write your own tests here and use the macros from mitkTestingMacros.h !!!
      // do not write to std::cout and do not return from this function yourself!

      // always end with this!
MITK_TEST_END    ()
  }

void testConstructorWithNodeAndRenderer()
  {
    mitk::BaseRenderer *renderer = NULL;
    myMP = mitk::MaterialProperty::New(node, renderer);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
    MITK_TEST_CONDITION( node == myMP->GetDataTreeNode(), "Testing if a DataTreeNode object was set correctly" )
    MITK_TEST_CONDITION( renderer == myMP->GetRenderer(), "Testing if a Renderer object was set correctly" )
  }

  void testConstructorWithColorOpacityNodeAndRenderer()
  {
    mitk::BaseRenderer *renderer = NULL;
    mitk::Color color;
    color.Set(0, 0, 0);
    vtkFloatingPointType opacity = 1.0f;
    myMP = mitk::MaterialProperty::New(color, opacity, node, renderer);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
    MITK_TEST_CONDITION( node==myMP->GetDataTreeNode(), "Testing if a DataTreeNode object was set correctly" )
    MITK_TEST_CONDITION( renderer==myMP->GetRenderer(), "Testing if a Renderer object was set correctly" )
    MITK_TEST_CONDITION( color==myMP->GetColor(), "Testing if a Color object was set correctly" )
    MITK_TEST_CONDITION( opacity==myMP->GetOpacity(), "Testing if a Opacity object was set correctly" )
  }

  void testConstructorWithRedGreenBlueOpacityNodeAndRenderer()
  {
    mitk::BaseRenderer *renderer = NULL;
    mitk::MaterialProperty::Color color;
    color.Set(0, 0, 0);
    vtkFloatingPointType opacity = 1.0f;
    vtkFloatingPointType rgb = 0;

    myMP = mitk::MaterialProperty::New(rgb, rgb, rgb, opacity, node,
      renderer);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
    MITK_TEST_CONDITION( node==myMP->GetDataTreeNode(), "Testing if a DataTreeNode object was set correctly" )
    MITK_TEST_CONDITION( renderer==myMP->GetRenderer(), "Testing if a Renderer object was set correctly" )
    MITK_TEST_CONDITION( color==myMP->GetColor(), "Testing if a Color object was set correctly" )
    MITK_TEST_CONDITION( opacity==myMP->GetOpacity(), "Testing if a Opacity object was set correctly" )
   }

  void testConstructorRedGreenBlueColorCoefficientSpecularCoefficientSpecularPowerOpacityNodeAndRenderer()
  {
    mitk::BaseRenderer *renderer = NULL;
    mitk::MaterialProperty::Color color;
    color.Set(0, 0, 0);
    vtkFloatingPointType opacity = 1.0f;
    vtkFloatingPointType rgb = 0;
    vtkFloatingPointType colorCoefficient = 0;
    vtkFloatingPointType specularCoefficient = 0;
    vtkFloatingPointType specularPower = 0;

    myMP = mitk::MaterialProperty::New(rgb, rgb, rgb, colorCoefficient,
      specularCoefficient, specularPower, opacity, node, renderer);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
      MITK_TEST_CONDITION( node==myMP->GetDataTreeNode(), "Testing if a DataTreeNode object was set correctly" )
      MITK_TEST_CONDITION( renderer==myMP->GetRenderer(), "Testing if a Renderer object was set correctly" )
      MITK_TEST_CONDITION( color==myMP->GetColor(), "Testing if a Color object was set correctly" )
      MITK_TEST_CONDITION( opacity==myMP->GetOpacity(), "Testing if a Opacity object was set correctly" )
      MITK_TEST_CONDITION( specularCoefficient==myMP->GetSpecularCoefficient(), "Testing if a Coefficient object was set correctly" )
      MITK_TEST_CONDITION( specularPower==myMP->GetSpecularPower(), "Testing if a SpecularPower object was set correctly" )
      MITK_TEST_CONDITION( colorCoefficient==myMP->GetColorCoefficient(), "Testing if a colorCoefficient object was set correctly" )
  }

  void testConstructorColorColorCoefficientSpecularCoefficientSpecularPowerOpacityNodeAndRenderer()
  {
    mitk::BaseRenderer *renderer = NULL;
    mitk::MaterialProperty::Color color;
    color.Set(0, 0, 0);
    vtkFloatingPointType opacity = 1.0f;
    vtkFloatingPointType rgb = 0;
    vtkFloatingPointType colorCoefficient = 0;
    vtkFloatingPointType specularCoefficient = 0;
    vtkFloatingPointType specularPower = 0;

    myMP = mitk::MaterialProperty::New(rgb, rgb, rgb, colorCoefficient,
      specularCoefficient, specularPower, opacity, node, renderer);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
    MITK_TEST_CONDITION( node==myMP->GetDataTreeNode(), "Testing if a DataTreeNode object was set correctly" )
    MITK_TEST_CONDITION( renderer==myMP->GetRenderer(), "Testing if a Renderer object was set correctly" )
    MITK_TEST_CONDITION( color==myMP->GetColor(), "Testing if a Color object was set correctly" )
    MITK_TEST_CONDITION( opacity==myMP->GetOpacity(), "Testing if a Opacity object was set correctly" )
    MITK_TEST_CONDITION( specularCoefficient==myMP->GetSpecularCoefficient(), "Testing if a Coefficient object was set correctly" )
    MITK_TEST_CONDITION( specularPower==myMP->GetSpecularPower(), "Testing if a SpecularPower object was set correctly" )
  }

  void testConstructorPropertyRedGreenBlueOpacityAndName()
  {
    mitk::MaterialProperty::ConstPointer reference = myMP;
    mitk::BaseRenderer *renderer = NULL;
    vtkFloatingPointType opacity = 1.0f;
    vtkFloatingPointType rgb = 0;
    std::string name = "Hans Wurst";

    myMP = mitk::MaterialProperty::New(*reference, rgb, rgb, rgb, opacity, name);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
    MITK_TEST_CONDITION( node==myMP->GetDataTreeNode(), "Testing if a DataTreeNode object was set correctly" )
    MITK_TEST_CONDITION( renderer==myMP->GetRenderer(), "Testing if a Renderer object was set correctly" )
    MITK_TEST_CONDITION( opacity==myMP->GetOpacity(), "Testing if a Opacity object was set correctly" )
    //MITK_TEST_CONDITION( name.compare(myMP->GetName(),0,9), "Testing if a Name object was set correctly" )
  }
