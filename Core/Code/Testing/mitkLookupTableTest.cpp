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

#include <mitkLookupTable.h>

#include <mitkTestFixture.h>
#include "mitkTestingMacros.h"
#include <mitkNumericTypes.h>

#include <iostream>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

class mitkLookupTableTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLookupTableTestSuite);
  MITK_TEST(TestCreateLookupTable);
  MITK_TEST(TestSetVtkLookupTable);
  MITK_TEST(TestSetOpacity);
  MITK_TEST(TestCreateColorTransferFunction);
  MITK_TEST(TestCreateOpacityTransferFunction);
  MITK_TEST(TestCreateGradientTransferFunction);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::LookupTable::Pointer m_LookupTable;

public:

  void TestCreateLookupTable()
  {
    // let's create an object of our class
    mitk::LookupTable::Pointer myLookupTable = mitk::LookupTable::New();
    // first test: did this work?
    // it makes no sense to continue without an object.
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation", myLookupTable.IsNotNull());
  }

  void TestSetVtkLookupTable ()
  {
    mitk::LookupTable::Pointer myLookupTable = mitk::LookupTable::New();

    // create a vtkLookupTable and add two values
    vtkLookupTable *lut = vtkLookupTable::New();
    lut->SetTableValue(0, 0.5, 0.5, 0.5, 1.0);
    lut->SetTableValue(1, 0.5, 0.5, 0.5, 0.5);
    lut->Build();

    myLookupTable->SetVtkLookupTable(lut);

    // check if the same lookuptable is returned
    vtkLookupTable *lut2 = myLookupTable->GetVtkLookupTable();

    CPPUNIT_ASSERT_MESSAGE("Input and output table are not equal",lut == lut2);

    lut->Delete();
  }

  void TestSetOpacity()
  {
    mitk::LookupTable::Pointer myLookupTable = mitk::LookupTable::New();

    // create a vtkLookupTable and add two values
    vtkLookupTable *lut = vtkLookupTable::New();
    lut->SetRange(0, 200);
    lut->SetAlphaRange(0.0, 1.0);
    lut->Build();

    myLookupTable->SetVtkLookupTable(lut);

    myLookupTable->ChangeOpacityForAll(0.7f);

    for (int i = 0; i < lut->GetNumberOfTableValues(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("Opacity not set for all", mitk::Equal(0.7, lut->GetOpacity(i), 0.01, true));
    }

    vtkIdType tableIndex = 10;
    myLookupTable->ChangeOpacity(tableIndex, 1.0);
    double rgba[4];
    lut->GetTableValue(tableIndex, rgba);
    CPPUNIT_ASSERT_MESSAGE("Opacity not set for value", mitk::Equal(1.0, rgba[3], 0.01, true));
    lut->Delete();
  }

  void TestCreateColorTransferFunction ()
  {
    mitk::LookupTable::Pointer myLookupTable = mitk::LookupTable::New();

    // create a vtkLookupTable and add two values
    vtkLookupTable *lut = vtkLookupTable::New();
    lut->SetRange(0, 255);
    lut->Build();

    myLookupTable->SetVtkLookupTable(lut);
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = myLookupTable->CreateColorTransferFunction();

    CPPUNIT_ASSERT(colorTransferFunction != 0);

    vtkIdType numberOfTableEntries = lut->GetNumberOfTableValues();
    double rgbaTable[4];
    double rgbaFunction[4];
    for (int i = 0; i < numberOfTableEntries; ++i)
    {
      lut->GetIndexedColor(i, rgbaTable);
      colorTransferFunction->GetIndexedColor(i, rgbaFunction);
      CPPUNIT_ASSERT_MESSAGE("Wrong color of transfer function",
                             mitk::Equal(rgbaTable[0], rgbaFunction[0], 0.000001, true) &&
                             mitk::Equal(rgbaTable[1], rgbaFunction[1], 0.000001, true) &&
                             mitk::Equal(rgbaTable[2], rgbaFunction[2], 0.000001, true)
                             );
    }
    lut->Delete();
  }

  void TestCreateOpacityTransferFunction ()
  {
    mitk::LookupTable::Pointer myLookupTable = mitk::LookupTable::New();

    // create a vtkLookupTable and add two values
    vtkLookupTable *lut = vtkLookupTable::New();
    lut->SetAlphaRange(0, 1.0);
    lut->Build();

    myLookupTable->SetVtkLookupTable(lut);
    vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction = myLookupTable->CreateOpacityTransferFunction();
    CPPUNIT_ASSERT(opacityTransferFunction != 0);

    int funcSize = opacityTransferFunction->GetSize();
    double *table = new double[funcSize];
    double rgba[4];
    opacityTransferFunction->GetTable(0, 1, funcSize, table);
    for (int i = 0; i < funcSize; ++i)
    {
      lut->GetIndexedColor(i, rgba);
      CPPUNIT_ASSERT_MESSAGE("Wrong opacity of transfer function",
                             mitk::Equal(table[i], rgba[3], 0.000001, true)
                             );
    }
    lut->Delete();
    delete [] table;
  }

  void TestCreateGradientTransferFunction ()
  {
    mitk::LookupTable::Pointer myLookupTable = mitk::LookupTable::New();

    // create a vtkLookupTable and add two values
    vtkLookupTable *lut = vtkLookupTable::New();
    lut->SetAlphaRange(0, 0.73);
    lut->Build();


    myLookupTable->SetVtkLookupTable(lut);
    vtkSmartPointer<vtkPiecewiseFunction> gradientTransferFunction = myLookupTable->CreateGradientTransferFunction();
    CPPUNIT_ASSERT(gradientTransferFunction != 0);

    int funcSize = gradientTransferFunction->GetSize();
    double *table = new double[funcSize];
    double rgba[4];
    gradientTransferFunction->GetTable(0, 1, funcSize, table);
    for (int i = 0; i < funcSize; ++i)
    {
      lut->GetIndexedColor(i, rgba);
      CPPUNIT_ASSERT_MESSAGE("Wrong opacity of transfer function",
                             mitk::Equal(table[i], rgba[3], 0.000001, true)
                             );
    }
    lut->Delete();
    delete [] table;
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkLookupTable)

