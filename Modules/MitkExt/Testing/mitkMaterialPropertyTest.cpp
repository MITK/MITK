/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright () German Cancer Research Center, Division of Medical and
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
#include <mitkColorProperty.h>

#include <iostream>

/**
 *  Simple example for a test for the () class "ClassName".
 *
 *  argc and argv are the command line parameters which were passed to
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests ().
 */




class MaterialPropTest
{
  public:
mitk::DataTreeNode::Pointer node;
mitk::MaterialProperty::Pointer myMP;

MaterialPropTest(){node = NULL; myMP = NULL;}

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
    mitk::MaterialProperty::Pointer reference = myMP;
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
void testSetColor()
{
  mitk::BaseRenderer *renderer = NULL;
  
  mitk::MaterialProperty::Color color;
  color.Set(0,0,0);
  myMP = mitk::MaterialProperty::New(node, NULL);
  myMP->SetColor(color);
  MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
  MITK_TEST_CONDITION(node==myMP->GetDataTreeNode(), "Testing if a DataTreeNode object was set correctly" )
  MITK_TEST_CONDITION(renderer==myMP->GetRenderer(), "Testing if a Renderer object was set correctly" )
  MITK_TEST_CONDITION(color==myMP->GetColor(),"Testing if a color was set correctly")
  color.Set(0,0,0);
  myMP->SetColor(color);
  MITK_TEST_CONDITION(color==myMP->GetColor(),"Testing if a color was set correctly")
}

void testSetColorCoefficient()
{
  
  vtkFloatingPointType colorCoefficient = 0;
  myMP = mitk::MaterialProperty::New(node, NULL);
  myMP->SetColorCoefficient(colorCoefficient);
  MITK_TEST_CONDITION(colorCoefficient == myMP->GetColorCoefficient(), "Testing if a colorcoefficent was set correctly")

}
void testSetSpecularColor()
{
    mitk::MaterialProperty::Color color;
    color.Set(0,0,0);
    myMP = mitk::MaterialProperty::New();
    myMP->SetSpecularColor(color);
    MITK_TEST_CONDITION(color == myMP->GetSpecularColor(),"Testing if a SpecularColor was set correctly")



}

void testSetSpecularCoefficient()
{
  myMP = mitk::MaterialProperty::New(node, NULL);
  vtkFloatingPointType specularCoefficient = 1;
  myMP->SetSpecularCoefficient(specularCoefficient);
  MITK_TEST_CONDITION(specularCoefficient == myMP->GetSpecularCoefficient(),"Testing if a SpecularCoefficient was set correctly")

}
void testSetSpecularPower()
{
  myMP = mitk::MaterialProperty::New(node, NULL);
  vtkFloatingPointType specularPower = 1;
  myMP->SetSpecularPower(specularPower);
  MITK_TEST_CONDITION(specularPower==myMP->GetSpecularPower(), "Testing if a SpecularPower was set correctly")


}
void testSetOpacity()
{
  myMP = mitk::MaterialProperty::New(node, NULL);
  vtkFloatingPointType opacity = 1;
  myMP->SetOpacity(opacity);
  MITK_TEST_CONDITION(opacity==myMP->GetOpacity(), "Testing if a Opacity was set correctly")

}


void testSetInterpolation()
{
  myMP = mitk::MaterialProperty::New(node, NULL);
  mitk::MaterialProperty::InterpolationType interpolation = mitk::MaterialProperty::Flat;
  myMP->SetInterpolation(interpolation);
  MITK_TEST_CONDITION(interpolation == myMP->GetInterpolation(), "Testing if a Interpolation was set correctly")
}
void testSetRepresentation()
{
  myMP = mitk::MaterialProperty::New(node, NULL);
  mitk::MaterialProperty::RepresentationType representation = mitk::MaterialProperty::Wireframe;
  myMP->SetRepresentation(representation);
  MITK_TEST_CONDITION(representation == myMP->GetRepresentation(), "Testing if a Representation was set correctly")

}

void testSetLineWidth()
{
  myMP = mitk::MaterialProperty::New(node, NULL);
  vtkFloatingPointType lineWidth = 1;
  myMP->SetLineWidth(lineWidth);
  MITK_TEST_CONDITION(lineWidth==myMP->GetLineWidth(), "Testing if a LineWidth was set correctly")
}

void testInitialize()
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
  vtkFloatingPointType lineWidth = 1;
  myMP->SetLineWidth(lineWidth);
  mitk::MaterialProperty::RepresentationType representation = mitk::MaterialProperty::Wireframe;
  myMP->SetRepresentation(representation);mitk::MaterialProperty::InterpolationType interpolation = mitk::MaterialProperty::Flat;
  myMP->SetInterpolation(interpolation);
  myMP->SetSpecularColor(color);
  std::string name = "Hans Wurst";
  myMP->SetName(name);
  mitk::MaterialProperty::Pointer myMP2 = mitk::MaterialProperty::New(node, NULL);
  myMP2->Initialize(*myMP);
  MITK_TEST_CONDITION(*myMP == *myMP2, "testing equality after .Intitialize")
}
void testOperatorequality()
{
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
    vtkFloatingPointType lineWidth = 1;
    myMP->SetLineWidth(lineWidth);
    mitk::MaterialProperty::RepresentationType representation = mitk::MaterialProperty::Wireframe;
    myMP->SetRepresentation(representation);mitk::MaterialProperty::InterpolationType interpolation = mitk::MaterialProperty::Flat;
    myMP->SetInterpolation(interpolation);
    myMP->SetSpecularColor(color);
    std::string name = "Hans Wurst";
    myMP->SetName(name);


    mitk::BaseRenderer *renderer2 = NULL;
    mitk::MaterialProperty::Color color2;
    color2.Set(0, 0, 0);
    vtkFloatingPointType opacity2 = 1.0f;
    vtkFloatingPointType rgb2 = 0;
    vtkFloatingPointType colorCoefficient2 = 0;
    vtkFloatingPointType specularCoefficient2 = 0;
    vtkFloatingPointType specularPower2 = 0;

    mitk::MaterialProperty::Pointer myMP2 = mitk::MaterialProperty::New(rgb2, rgb2, rgb2, colorCoefficient2,
      specularCoefficient2, specularPower2, opacity2, node, renderer2);
    vtkFloatingPointType lineWidth2 = 1;
    myMP2->SetLineWidth(lineWidth2);
    mitk::MaterialProperty::RepresentationType representation2 = mitk::MaterialProperty::Wireframe;
    myMP2->SetRepresentation(representation2);
    mitk::MaterialProperty::InterpolationType interpolation2 = mitk::MaterialProperty::Flat;
    myMP2->SetInterpolation(interpolation2);
    myMP2->SetSpecularColor(color2);
    std::string name2 = "Hans Wurst";
    myMP2->SetName(name2);
    MITK_TEST_CONDITION(*myMP == *myMP2, "testing equality Operator")
  }
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
    vtkFloatingPointType lineWidth = 1;
    myMP->SetLineWidth(lineWidth);
    mitk::MaterialProperty::RepresentationType representation = mitk::MaterialProperty::Wireframe;
    myMP->SetRepresentation(representation);mitk::MaterialProperty::InterpolationType interpolation = mitk::MaterialProperty::Flat;
    myMP->SetInterpolation(interpolation);
    myMP->SetSpecularColor(color);
    std::string name = "Hans Wurst";
    myMP->SetName(name);


    mitk::BaseRenderer *renderer2 = NULL;
    mitk::MaterialProperty::Color color2;
    color2.Set(0, 0, 0);
    vtkFloatingPointType opacity2 = 1.0f;
    vtkFloatingPointType rgb2 = 1;
    vtkFloatingPointType colorCoefficient2 = 0;
    vtkFloatingPointType specularCoefficient2 = 0;
    vtkFloatingPointType specularPower2 = 0;

    mitk::MaterialProperty::Pointer myMP2 = mitk::MaterialProperty::New(rgb2, rgb2, rgb2, colorCoefficient2,
      specularCoefficient2, specularPower2, opacity2, node, renderer2);
    vtkFloatingPointType lineWidth2 = 1;
    myMP2->SetLineWidth(lineWidth2);
    mitk::MaterialProperty::RepresentationType representation2 = mitk::MaterialProperty::Wireframe;
    myMP2->SetRepresentation(representation2);
    mitk::MaterialProperty::InterpolationType interpolation2 = mitk::MaterialProperty::Flat;
    myMP2->SetInterpolation(interpolation2);
    myMP2->SetSpecularColor(color2);
    std::string name2 = "Hans Wurst";
    myMP2->SetName(name2);
    MITK_TEST_CONDITION(!(*myMP == *myMP2), "testing equality Operator")
  }
 

}
void testForwardToDataTreeNode()
{
  myMP = NULL;
  mitk::BaseRenderer *renderer = NULL;
  myMP = mitk::MaterialProperty::New(node, renderer);
  MITK_TEST_CONDITION(myMP->ForwardToDataTreeNode(), "testing ForwardToDataTreeNode() with (node = mitk::DataTreeNode::New())")
  myMP->SetDataTreeNode(NULL);
  MITK_TEST_CONDITION(!(myMP->ForwardToDataTreeNode()), "testing ForwardToDataTreeNode() with (node = NULL)")


}

void testAssignable()
{
  mitk::ColorProperty::Pointer colorProp = mitk::ColorProperty::New();
  mitk::MaterialProperty::Pointer materialProp = mitk::MaterialProperty::New();
  MITK_TEST_CONDITION(!(myMP->Assignable(*colorProp)), "testing Assignable with ColorProperty")
  MITK_TEST_CONDITION(myMP->Assignable(*materialProp),"testing Assignable with MaterialProperty" )
} 

void testOperatorAssign()
{
  mitk::MaterialProperty::Pointer myMP2 = mitk::MaterialProperty::New();
  *myMP2 = *myMP;
  MITK_TEST_CONDITION(*myMP == *myMP2, "Testing Assignment Operator")
}
};





int
mitkMaterialPropertyTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("MaterialProperty")
      
      MaterialPropTest materialPropTest;
      materialPropTest.node = mitk::DataTreeNode::New();

      materialPropTest.testConstructorWithNodeAndRenderer();
      materialPropTest.testConstructorWithColorOpacityNodeAndRenderer();
      materialPropTest.testConstructorWithRedGreenBlueOpacityNodeAndRenderer();
      materialPropTest.testConstructorRedGreenBlueColorCoefficientSpecularCoefficientSpecularPowerOpacityNodeAndRenderer();
      materialPropTest.testConstructorColorColorCoefficientSpecularCoefficientSpecularPowerOpacityNodeAndRenderer();
      materialPropTest.testConstructorPropertyRedGreenBlueOpacityAndName();
      materialPropTest.testAssignable();
      materialPropTest.testOperatorAssign();
      materialPropTest.testSetColor();
      materialPropTest.testSetColorCoefficient();
      materialPropTest.testSetSpecularColor();
      materialPropTest.testSetSpecularCoefficient();
      materialPropTest.testSetSpecularPower();
      materialPropTest.testSetOpacity();
      materialPropTest.testSetInterpolation();
      materialPropTest.testSetRepresentation();
      materialPropTest.testSetLineWidth();
      materialPropTest.testInitialize();
      materialPropTest.testOperatorequality();
      materialPropTest.testForwardToDataTreeNode();
   
      // first test: did this work?
      // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
      // it makes no sense to continue without an object.

      // write your own tests here and use the macros from mitkTestingMacros.h !!!
      // do not write to std::cout and do not return from this function yourself!

      // always end with this!
MITK_TEST_END    ()  
}


