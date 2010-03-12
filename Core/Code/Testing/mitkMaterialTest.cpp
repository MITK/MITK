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

#include "mitkMaterial.h"
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




class MaterialTest
{
  public:
mitk::Material::Pointer myMP;

MaterialTest(){myMP = NULL;}

void testConstructor()
  {
    myMP = mitk::Material::New();
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
  }

  void testConstructorWithColorOpacity()
  {
    mitk::Color color;
    color.Set(0, 0, 0);
    vtkFloatingPointType opacity = 1.0f;
    myMP = mitk::Material::New(color, opacity);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
    MITK_TEST_CONDITION( color==myMP->GetColor(), "Testing if a Color object was set correctly" )
    MITK_TEST_CONDITION( opacity==myMP->GetOpacity(), "Testing if a Opacity object was set correctly" )
  }

  void testConstructorWithRedGreenBlueOpacity()
  {
    mitk::Material::Color color;
    color.Set(0, 0, 0);
    vtkFloatingPointType opacity = 1.0f;
    vtkFloatingPointType rgb = 0;

    myMP = mitk::Material::New(rgb, rgb, rgb, opacity);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
    MITK_TEST_CONDITION( color==myMP->GetColor(), "Testing if a Color object was set correctly" )
    MITK_TEST_CONDITION( opacity==myMP->GetOpacity(), "Testing if a Opacity object was set correctly" )
   }

  void testConstructorRedGreenBlueColorCoefficientSpecularCoefficientSpecularPowerOpacity()
  {
    mitk::Material::Color color;
    color.Set(0, 0, 0);
    vtkFloatingPointType opacity = 1.0f;
    vtkFloatingPointType rgb = 0;
    vtkFloatingPointType colorCoefficient = 0;
    vtkFloatingPointType specularCoefficient = 0;
    vtkFloatingPointType specularPower = 0;

    myMP = mitk::Material::New(rgb, rgb, rgb, colorCoefficient,
      specularCoefficient, specularPower, opacity);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
      MITK_TEST_CONDITION( color==myMP->GetColor(), "Testing if a Color object was set correctly" )
      MITK_TEST_CONDITION( opacity==myMP->GetOpacity(), "Testing if a Opacity object was set correctly" )
      MITK_TEST_CONDITION( specularCoefficient==myMP->GetSpecularCoefficient(), "Testing if a Coefficient object was set correctly" )
      MITK_TEST_CONDITION( specularPower==myMP->GetSpecularPower(), "Testing if a SpecularPower object was set correctly" )
      MITK_TEST_CONDITION( colorCoefficient==myMP->GetColorCoefficient(), "Testing if a colorCoefficient object was set correctly" )
  }

  void testConstructorColorColorCoefficientSpecularCoefficientSpecularPowerOpacity()
  {
    mitk::Material::Color color;
    color.Set(0, 0, 0);
    vtkFloatingPointType opacity = 1.0f;
    vtkFloatingPointType rgb = 0;
    vtkFloatingPointType colorCoefficient = 0;
    vtkFloatingPointType specularCoefficient = 0;
    vtkFloatingPointType specularPower = 0;

    myMP = mitk::Material::New(rgb, rgb, rgb, colorCoefficient,
      specularCoefficient, specularPower, opacity);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
    MITK_TEST_CONDITION( color==myMP->GetColor(), "Testing if a Color object was set correctly" )
    MITK_TEST_CONDITION( opacity==myMP->GetOpacity(), "Testing if a Opacity object was set correctly" )
    MITK_TEST_CONDITION( specularCoefficient==myMP->GetSpecularCoefficient(), "Testing if a Coefficient object was set correctly" )
    MITK_TEST_CONDITION( specularPower==myMP->GetSpecularPower(), "Testing if a SpecularPower object was set correctly" )
  }

  void testConstructorPropertyRedGreenBlueOpacityAndName()
  {
    mitk::Material::Pointer reference = myMP;
    vtkFloatingPointType opacity = 1.0f;
    vtkFloatingPointType rgb = 0;
    std::string name = "Hans Wurst";

    myMP = mitk::Material::New(*reference, rgb, rgb, rgb, opacity, name);
    MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
    MITK_TEST_CONDITION( opacity==myMP->GetOpacity(), "Testing if a Opacity object was set correctly" )
    //MITK_TEST_CONDITION( name.compare(myMP->GetName(),0,9), "Testing if a Name object was set correctly" )
  }
void testSetColor()
{
  mitk::Material::Color color;
  color.Set(0,0,0);
  myMP = mitk::Material::New();
  myMP->SetColor(color);
  MITK_TEST_CONDITION_REQUIRED(myMP.IsNotNull(),"Testing instantiation")
  MITK_TEST_CONDITION(color==myMP->GetColor(),"Testing if a color was set correctly")
  color.Set(0,0,0);
  myMP->SetColor(color);
  MITK_TEST_CONDITION(color==myMP->GetColor(),"Testing if a color was set correctly")
}

void testSetColorCoefficient()
{
  
  vtkFloatingPointType colorCoefficient = 0;
  myMP = mitk::Material::New();
  myMP->SetColorCoefficient(colorCoefficient);
  MITK_TEST_CONDITION(colorCoefficient == myMP->GetColorCoefficient(), "Testing if a colorcoefficent was set correctly")

}
void testSetSpecularColor()
{
    mitk::Material::Color color;
    color.Set(0,0,0);
    myMP = mitk::Material::New();
    myMP->SetSpecularColor(color);
    MITK_TEST_CONDITION(color == myMP->GetSpecularColor(),"Testing if a SpecularColor was set correctly")



}

void testSetSpecularCoefficient()
{
  myMP = mitk::Material::New();
  vtkFloatingPointType specularCoefficient = 1;
  myMP->SetSpecularCoefficient(specularCoefficient);
  MITK_TEST_CONDITION(specularCoefficient == myMP->GetSpecularCoefficient(),"Testing if a SpecularCoefficient was set correctly")

}
void testSetSpecularPower()
{
  myMP = mitk::Material::New();
  vtkFloatingPointType specularPower = 1;
  myMP->SetSpecularPower(specularPower);
  MITK_TEST_CONDITION(specularPower==myMP->GetSpecularPower(), "Testing if a SpecularPower was set correctly")


}
void testSetOpacity()
{
  myMP = mitk::Material::New();
  vtkFloatingPointType opacity = 1;
  myMP->SetOpacity(opacity);
  MITK_TEST_CONDITION(opacity==myMP->GetOpacity(), "Testing if a Opacity was set correctly")

}


void testSetInterpolation()
{
  myMP = mitk::Material::New();
  mitk::Material::InterpolationType interpolation = mitk::Material::Flat;
  myMP->SetInterpolation(interpolation);
  MITK_TEST_CONDITION(interpolation == myMP->GetInterpolation(), "Testing if a Interpolation was set correctly")
}
void testSetRepresentation()
{
  myMP = mitk::Material::New();
  mitk::Material::RepresentationType representation = mitk::Material::Wireframe;
  myMP->SetRepresentation(representation);
  MITK_TEST_CONDITION(representation == myMP->GetRepresentation(), "Testing if a Representation was set correctly")

}

void testSetLineWidth()
{
  myMP = mitk::Material::New();
  vtkFloatingPointType lineWidth = 1;
  myMP->SetLineWidth(lineWidth);
  MITK_TEST_CONDITION(lineWidth==myMP->GetLineWidth(), "Testing if a LineWidth was set correctly")
}

void testInitialize()
{
  mitk::Material::Color color;
  color.Set(0, 0, 0);
  vtkFloatingPointType opacity = 1.0f;
  vtkFloatingPointType rgb = 0;
  vtkFloatingPointType colorCoefficient = 0;
  vtkFloatingPointType specularCoefficient = 0;
  vtkFloatingPointType specularPower = 0;

  myMP = mitk::Material::New(rgb, rgb, rgb, colorCoefficient,
    specularCoefficient, specularPower, opacity);
  vtkFloatingPointType lineWidth = 1;
  myMP->SetLineWidth(lineWidth);
  mitk::Material::RepresentationType representation = mitk::Material::Wireframe;
  myMP->SetRepresentation(representation);mitk::Material::InterpolationType interpolation = mitk::Material::Flat;
  myMP->SetInterpolation(interpolation);
  myMP->SetSpecularColor(color);
  std::string name = "Hans Wurst";
  myMP->SetName(name);
  mitk::Material::Pointer myMP2 = mitk::Material::New();
  myMP2->Initialize(*myMP);
  MITK_TEST_CONDITION(*myMP == *myMP2, "testing equality after .Intitialize")
}
void testOperatorequality()
{
  {
    mitk::Material::Color color;
    color.Set(0, 0, 0);
    vtkFloatingPointType opacity = 1.0f;
    vtkFloatingPointType rgb = 0;
    vtkFloatingPointType colorCoefficient = 0;
    vtkFloatingPointType specularCoefficient = 0;
    vtkFloatingPointType specularPower = 0;

    myMP = mitk::Material::New(rgb, rgb, rgb, colorCoefficient,
      specularCoefficient, specularPower, opacity);
    vtkFloatingPointType lineWidth = 1;
    myMP->SetLineWidth(lineWidth);
    mitk::Material::RepresentationType representation = mitk::Material::Wireframe;
    myMP->SetRepresentation(representation);mitk::Material::InterpolationType interpolation = mitk::Material::Flat;
    myMP->SetInterpolation(interpolation);
    myMP->SetSpecularColor(color);
    std::string name = "Hans Wurst";
    myMP->SetName(name);


    mitk::Material::Color color2;
    color2.Set(0, 0, 0);
    vtkFloatingPointType opacity2 = 1.0f;
    vtkFloatingPointType rgb2 = 0;
    vtkFloatingPointType colorCoefficient2 = 0;
    vtkFloatingPointType specularCoefficient2 = 0;
    vtkFloatingPointType specularPower2 = 0;

    mitk::Material::Pointer myMP2 = mitk::Material::New(rgb2, rgb2, rgb2, colorCoefficient2,
      specularCoefficient2, specularPower2, opacity2);
    vtkFloatingPointType lineWidth2 = 1;
    myMP2->SetLineWidth(lineWidth2);
    mitk::Material::RepresentationType representation2 = mitk::Material::Wireframe;
    myMP2->SetRepresentation(representation2);
    mitk::Material::InterpolationType interpolation2 = mitk::Material::Flat;
    myMP2->SetInterpolation(interpolation2);
    myMP2->SetSpecularColor(color2);
    std::string name2 = "Hans Wurst";
    myMP2->SetName(name2);
    MITK_TEST_CONDITION(*myMP == *myMP2, "testing equality Operator")
  }
  {
    mitk::Material::Color color;
    color.Set(0, 0, 0);
    vtkFloatingPointType opacity = 1.0f;
    vtkFloatingPointType rgb = 0;
    vtkFloatingPointType colorCoefficient = 0;
    vtkFloatingPointType specularCoefficient = 0;
    vtkFloatingPointType specularPower = 0;

    myMP = mitk::Material::New(rgb, rgb, rgb, colorCoefficient,
      specularCoefficient, specularPower, opacity);
    vtkFloatingPointType lineWidth = 1;
    myMP->SetLineWidth(lineWidth);
    mitk::Material::RepresentationType representation = mitk::Material::Wireframe;
    myMP->SetRepresentation(representation);mitk::Material::InterpolationType interpolation = mitk::Material::Flat;
    myMP->SetInterpolation(interpolation);
    myMP->SetSpecularColor(color);
    std::string name = "Hans Wurst";
    myMP->SetName(name);


    mitk::Material::Color color2;
    color2.Set(0, 0, 0);
    vtkFloatingPointType opacity2 = 1.0f;
    vtkFloatingPointType rgb2 = 1;
    vtkFloatingPointType colorCoefficient2 = 0;
    vtkFloatingPointType specularCoefficient2 = 0;
    vtkFloatingPointType specularPower2 = 0;

    mitk::Material::Pointer myMP2 = mitk::Material::New(rgb2, rgb2, rgb2, colorCoefficient2,
      specularCoefficient2, specularPower2, opacity2);
    vtkFloatingPointType lineWidth2 = 1;
    myMP2->SetLineWidth(lineWidth2);
    mitk::Material::RepresentationType representation2 = mitk::Material::Wireframe;
    myMP2->SetRepresentation(representation2);
    mitk::Material::InterpolationType interpolation2 = mitk::Material::Flat;
    myMP2->SetInterpolation(interpolation2);
    myMP2->SetSpecularColor(color2);
    std::string name2 = "Hans Wurst";
    myMP2->SetName(name2);
    MITK_TEST_CONDITION(!(*myMP == *myMP2), "testing equality Operator")
  }
 

}

void testAssignable()
{
  mitk::Material::Pointer materialProp = mitk::Material::New();
  MITK_TEST_CONDITION(myMP->Assignable(*materialProp),"testing Assignable with Material" )
} 

void testOperatorAssign()
{
  mitk::Material::Pointer myMP2 = mitk::Material::New();
  *myMP2 = *myMP;
  MITK_TEST_CONDITION(*myMP == *myMP2, "Testing Assignment Operator")
}
};





int
mitkMaterialTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("Material")
      
      MaterialTest materialTest;

      materialTest.testConstructor();
      materialTest.testConstructorWithColorOpacity();
      materialTest.testConstructorWithRedGreenBlueOpacity();
      materialTest.testConstructorRedGreenBlueColorCoefficientSpecularCoefficientSpecularPowerOpacity();
      materialTest.testConstructorColorColorCoefficientSpecularCoefficientSpecularPowerOpacity();
      materialTest.testConstructorPropertyRedGreenBlueOpacityAndName();
      materialTest.testAssignable();
      materialTest.testOperatorAssign();
      materialTest.testSetColor();
      materialTest.testSetColorCoefficient();
      materialTest.testSetSpecularColor();
      materialTest.testSetSpecularCoefficient();
      materialTest.testSetSpecularPower();
      materialTest.testSetOpacity();
      materialTest.testSetInterpolation();
      materialTest.testSetRepresentation();
      materialTest.testSetLineWidth();
      materialTest.testInitialize();
      materialTest.testOperatorequality();
   
      // first test: did this work?
      // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
      // it makes no sense to continue without an object.

      // write your own tests here and use the macros from mitkTestingMacros.h !!!
      // do not write to std::cout and do not return from this function yourself!

      // always end with this!
MITK_TEST_END    ()  
}


