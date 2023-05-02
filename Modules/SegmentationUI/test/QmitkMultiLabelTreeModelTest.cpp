/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QApplication>
#include <QColor>
#include "QmitkMultiLabelTreeModel.h"

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class QmitkMultiLabelTreeModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMultiLabelTreeModelTestSuite);
  MITK_TEST(NullTest);
  MITK_TEST(GetterSetterTest);
  MITK_TEST(AddingLabelTest);
  MITK_TEST(AddingLayerTest);
  MITK_TEST(RemovingLabelTest);
  MITK_TEST(RemovingLayerTest);
  MITK_TEST(ModifyLabelNameTest);
  MITK_TEST(ModifyLabelTest);

  CPPUNIT_TEST_SUITE_END();

  mitk::LabelSetImage::Pointer m_Segmentation;

  QCoreApplication* m_TestApp;

public:

  mitk::LabelSetImage::Pointer GenerateSegmentation()
  {
    // Create a new labelset image
    auto seg = mitk::LabelSetImage::New();
    mitk::Image::Pointer regularImage = mitk::Image::New();
    unsigned int dimensions[3] = { 5, 5, 5 };
    regularImage->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);
    seg->Initialize(regularImage);
    return seg;
  }

  QColor GetQColor(const mitk::Label* label)
  {
    return QColor(label->GetColor().GetRed() * 255, label->GetColor().GetGreen() * 255, label->GetColor().GetBlue() * 255);
  }

  mitk::Label::Pointer CreateLabel(const std::string& name, mitk::Label::PixelType value)
  {
    auto label = mitk::Label::New();
    label->SetName(name);
    label->SetValue(value);
    label->SetColor(mitk::Color(value / 255.));
    return label;
  }

  /** Populate a seg with a following setup (in brackets the order of addition).
  * - Group 1 (1)
  *  - Label A
  *   - Instance 1 (1)
  *   - Instance 5 (2)
  *   - Instance 10 (8)
  *  - Label B
  *   - Instance 4 (3)
  *  - Label D
  *   - Instance 2 (7)
  * - Group 2 (4)
  * - Group 3 (5)
  *  - Label B
  *   - Instance 9 (6)
  */
  void PopulateSegmentation(mitk::LabelSetImage* seg)
  {
    seg->SetActiveLayer(0);
    seg->GetActiveLabelSet()->AddLabel(CreateLabel("A", 1));
    seg->GetActiveLabelSet()->AddLabel(CreateLabel("A", 5));
    seg->GetActiveLabelSet()->AddLabel(CreateLabel("B", 4));
    seg->AddLayer();
    seg->AddLayer();
    seg->SetActiveLayer(2);
    seg->GetActiveLabelSet()->AddLabel(CreateLabel("B", 9));
    seg->SetActiveLayer(0);
    seg->GetActiveLabelSet()->AddLabel(CreateLabel("D", 2));
    seg->GetActiveLabelSet()->AddLabel(CreateLabel("A", 10));
  }

  void setUp() override
  {
    m_Segmentation = GenerateSegmentation();

    PopulateSegmentation(m_Segmentation);

    int argc = 0;
    char** argv = nullptr;
    m_TestApp = new QCoreApplication(argc, argv);
  }

  void tearDown() override
  {
    delete m_TestApp;
  }

  QModelIndex GetIndex(const QmitkMultiLabelTreeModel& model, const std::vector<int>& rows, int column = 0) const
  {
    QModelIndex testIndex;

    int i = 0;
    for (auto row : rows)
    {
      if (static_cast<std::vector<int>::size_type>(i) + 1 < rows.size())
      {
        testIndex = model.index(row, 0, testIndex);
      }
      else
      {
        testIndex = model.index(row, column, testIndex);
      }
      i++;
    }

    return testIndex;
  }

  bool CheckModelItem(const QmitkMultiLabelTreeModel& model, const std::vector<int>& rows, const QVariant& reference, int column, const mitk::Label* /*label = nullptr*/) const
  {
    QModelIndex testIndex = GetIndex(model, rows, column);

    auto value = model.data(testIndex);

    bool test = value == reference;

    if (!test) std::cerr << std::endl <<"    Model item error. Expected: '" << reference.toString().toStdString() << "'; actual: '" << value.toString().toStdString()  <<"'";
    return test;
  }

  bool CheckModelRow(const QmitkMultiLabelTreeModel& model, const std::vector<int>& rows, const std::vector<QVariant> references) const
  {
    int column = 0;
    bool test = true;
    for (const auto& ref : references)
    {
      test = test & CheckModelItem(model, rows, ref, column, nullptr);
      column++;
    }
    return test;
  }

  void CheckModelGroup0Default(const QmitkMultiLabelTreeModel& model)
  {
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0 }, { QString("A (3 instances)"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(GetIndex(model, { 0,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,0 }, { QString("A [1]"), QVariant(true), QVariant(QColor(1,1,1)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,1 }, { QString("A [5]"), QVariant(true), QVariant(QColor(5,5,5)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,2 }, { QString("A [10]"), QVariant(true), QVariant(QColor(10,10,10)), QVariant(true) }));

    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1 }, { QString("B"), QVariant(true), QVariant(QColor(4,4,4)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,2 }, { QString("D"), QVariant(true), QVariant(QColor(2,2,2)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,2 })));
  }

  void CheckModelGroup1Default(const QmitkMultiLabelTreeModel& model)
  {
    CPPUNIT_ASSERT(CheckModelRow(model, { 1 }, { QString("Group 1"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 1 })));
  }

  void CheckModelGroup2Default(const QmitkMultiLabelTreeModel& model)
  {
    CPPUNIT_ASSERT(CheckModelRow(model, { 2 }, { QString("Group 2"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(1, model.rowCount(GetIndex(model, { 2 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 2,0 }, { QString("B"), QVariant(true), QVariant(QColor(9,9,9)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 2,0 })));
  }

  void CheckModelDefault(const QmitkMultiLabelTreeModel& model)
  {
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CheckModelGroup0Default(model);
    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);
  }

  void NullTest()
  {
    QmitkMultiLabelTreeModel model(nullptr);

    CPPUNIT_ASSERT(nullptr == model.GetSegmentation());
  }

  void GetterSetterTest()
  {
    QmitkMultiLabelTreeModel model(nullptr);
    model.SetSegmentation(m_Segmentation);

    CheckModelDefault(model);

    model.SetSegmentation(nullptr);
    CPPUNIT_ASSERT(nullptr == model.GetSegmentation());
    CPPUNIT_ASSERT(false == model.hasChildren(QModelIndex()));
  }

  void AddingLabelTest()
  {
    QmitkMultiLabelTreeModel model(nullptr);
    model.SetSegmentation(m_Segmentation);

    //Add label instance (not visible) to labelwith multiple instances (at the end)
    m_Segmentation->SetActiveLayer(0);
    auto newLabel = CreateLabel("A", 100);
    newLabel->SetVisible(false);
    m_Segmentation->GetActiveLabelSet()->AddLabel(newLabel);

    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0 }, { QString("A (4 instances)"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(4, model.rowCount(GetIndex(model, { 0,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,0 }, { QString("A [1]"), QVariant(true), QVariant(QColor(1,1,1)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,1 }, { QString("A [5]"), QVariant(true), QVariant(QColor(5,5,5)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,2 }, { QString("A [10]"), QVariant(true), QVariant(QColor(10,10,10)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,3 }, { QString("A [100]"), QVariant(true), QVariant(QColor(100,100,100)), QVariant(false) }));

    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1 }, { QString("B"), QVariant(true), QVariant(QColor(4,4,4)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,2 }, { QString("D"), QVariant(true), QVariant(QColor(2,2,2)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,2 })));

    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);

    //Add label instance (not locked) to label with multiple instances (in between)
    m_Segmentation->SetActiveLayer(0);
    newLabel = CreateLabel("A", 7);
    newLabel->SetLocked(false);
    m_Segmentation->GetActiveLabelSet()->AddLabel(newLabel);

    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0 }, { QString("A (5 instances)"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(5, model.rowCount(GetIndex(model, { 0,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,0 }, { QString("A [1]"), QVariant(true), QVariant(QColor(1,1,1)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,1 }, { QString("A [5]"), QVariant(true), QVariant(QColor(5,5,5)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,2 }, { QString("A [7]"), QVariant(false), QVariant(QColor(7,7,7)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,3 }, { QString("A [10]"), QVariant(true), QVariant(QColor(10,10,10)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,4 }, { QString("A [100]"), QVariant(true), QVariant(QColor(100,100,100)), QVariant(false) }));

    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1 }, { QString("B"), QVariant(true), QVariant(QColor(4,4,4)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,2 }, { QString("D"), QVariant(true), QVariant(QColor(2,2,2)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,2 })));

    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);

    //reset everything
    m_Segmentation = GenerateSegmentation();
    PopulateSegmentation(m_Segmentation);
    model.SetSegmentation(m_Segmentation);

    //Add label instance to an empty group
    m_Segmentation->SetActiveLayer(1);
    newLabel = CreateLabel("A", 3);
    m_Segmentation->GetActiveLabelSet()->AddLabel(newLabel);

    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CheckModelGroup0Default(model);
    CPPUNIT_ASSERT(CheckModelRow(model, { 1 }, { QString("Group 1"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(1, model.rowCount(GetIndex(model, { 1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 1,0 }, { QString("A"), QVariant(true), QVariant(QColor(3,3,3)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 1,0 })));

    CheckModelGroup2Default(model);
  }

  void AddingLayerTest()
  {
    QmitkMultiLabelTreeModel model(nullptr);
    model.SetSegmentation(m_Segmentation);

    m_Segmentation->AddLayer();

    CPPUNIT_ASSERT_EQUAL(4, model.rowCount(QModelIndex()));
    CheckModelGroup0Default(model);
    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);
    CPPUNIT_ASSERT(CheckModelRow(model, { 3 }, { QString("Group 3"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 3 })));
  }

  void RemovingLabelTest()
  {
    QmitkMultiLabelTreeModel model(nullptr);
    model.SetSegmentation(m_Segmentation);
    //remove label instance from label with multiple instances (middel)
    m_Segmentation->RemoveLabel(5);
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0 }, { QString("A (2 instances)"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(GetIndex(model, { 0,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,0 }, { QString("A [1]"), QVariant(true), QVariant(QColor(1,1,1)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,1 }, { QString("A [10]"), QVariant(true), QVariant(QColor(10,10,10)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1 }, { QString("B"), QVariant(true), QVariant(QColor(4,4,4)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,2 }, { QString("D"), QVariant(true), QVariant(QColor(2,2,2)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,2 })));
    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);

    //remove label instance from label with multiple instances (first)
    m_Segmentation->RemoveLabel(1);
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0 }, { QString("A"), QVariant(true), QVariant(QColor(10,10,10)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1 }, { QString("B"), QVariant(true), QVariant(QColor(4,4,4)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,2 }, { QString("D"), QVariant(true), QVariant(QColor(2,2,2)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,2 })));
    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);


    //reset everything
    m_Segmentation = GenerateSegmentation();
    PopulateSegmentation(m_Segmentation);
    model.SetSegmentation(m_Segmentation);

    //remove label instance from label with multiple instances (at the end)
    m_Segmentation->RemoveLabel(10);
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0 }, { QString("A (2 instances)"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(GetIndex(model, { 0,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,0 }, { QString("A [1]"), QVariant(true), QVariant(QColor(1,1,1)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,1 }, { QString("A [5]"), QVariant(true), QVariant(QColor(5,5,5)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1 }, { QString("B"), QVariant(true), QVariant(QColor(4,4,4)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,2 }, { QString("D"), QVariant(true), QVariant(QColor(2,2,2)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,2 })));
    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);

    //reset everything
    m_Segmentation = GenerateSegmentation();
    PopulateSegmentation(m_Segmentation);
    model.SetSegmentation(m_Segmentation);

    //remove label instance from label with only one instance
    m_Segmentation->RemoveLabel(9);
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CheckModelGroup0Default(model);
    CheckModelGroup1Default(model);
    CPPUNIT_ASSERT(CheckModelRow(model, { 2 }, { QString("Group 2"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 2 })));
  }

  void RemovingLayerTest()
  {
    QmitkMultiLabelTreeModel model(nullptr);
    model.SetSegmentation(m_Segmentation);
    //remove group in the middle
    m_Segmentation->SetActiveLayer(1);
    m_Segmentation->RemoveLayer();
    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(QModelIndex()));
    CheckModelGroup0Default(model);
    CPPUNIT_ASSERT(CheckModelRow(model, { 1 }, { QString("Group 1"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(1, model.rowCount(GetIndex(model, { 1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 1,0 }, { QString("B"), QVariant(true), QVariant(QColor(9,9,9)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 1,0 })));

    //remove groups in the end
    m_Segmentation->SetActiveLayer(1);
    m_Segmentation->RemoveLayer();
    CPPUNIT_ASSERT_EQUAL(1, model.rowCount(QModelIndex()));
    CheckModelGroup0Default(model);

    //remove all groups
    m_Segmentation->SetActiveLayer(0);
    m_Segmentation->RemoveLayer();
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(QModelIndex()));

    //reset everything
    m_Segmentation = GenerateSegmentation();
    PopulateSegmentation(m_Segmentation);
    model.SetSegmentation(m_Segmentation);

    //remove first group
    m_Segmentation->SetActiveLayer(0);
    m_Segmentation->RemoveLayer();
    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(QModelIndex()));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 1 }, { QString("Group 1"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(1, model.rowCount(GetIndex(model, { 1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 1,0 }, { QString("B"), QVariant(true), QVariant(QColor(9,9,9)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 1,0 })));
  }


  void ModifyLabelNameTest()
  {
    QmitkMultiLabelTreeModel model(nullptr);
    model.SetSegmentation(m_Segmentation);
    //move from multiple instance to new label in the middle
    auto label = m_Segmentation->GetLabel(5,0);
    label->SetName("C");

    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(4, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0 }, { QString("A (2 instances)"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(GetIndex(model, { 0,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,0 }, { QString("A [1]"), QVariant(true), QVariant(QColor(1,1,1)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0,1 }, { QString("A [10]"), QVariant(true), QVariant(QColor(10,10,10)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1 }, { QString("B"), QVariant(true), QVariant(QColor(4,4,4)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,2 }, { QString("C"), QVariant(true), QVariant(QColor(5,5,5)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,2 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,3 }, { QString("D"), QVariant(true), QVariant(QColor(2,2,2)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,3 })));
    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);

    //move from multiple instance to new label at the end
    label = m_Segmentation->GetLabel(10, 0);
    label->SetName("E");

    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(5, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0 }, { QString("A"), QVariant(true), QVariant(QColor(1,1,1)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1 }, { QString("B"), QVariant(true), QVariant(QColor(4,4,4)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,2 }, { QString("C"), QVariant(true), QVariant(QColor(5,5,5)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,2 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,3 }, { QString("D"), QVariant(true), QVariant(QColor(2,2,2)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,3 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,4 }, { QString("E"), QVariant(true), QVariant(QColor(10,10,10)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,4 })));
    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);

    //move last instance to new label
    label = m_Segmentation->GetLabel(10, 0);
    label->SetName("F");

    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(5, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0 }, { QString("A"), QVariant(true), QVariant(QColor(1,1,1)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1 }, { QString("B"), QVariant(true), QVariant(QColor(4,4,4)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,2 }, { QString("C"), QVariant(true), QVariant(QColor(5,5,5)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,2 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,3 }, { QString("D"), QVariant(true), QVariant(QColor(2,2,2)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,3 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,4 }, { QString("F"), QVariant(true), QVariant(QColor(10,10,10)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,4 })));
    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);

    //move last instance to an existing label
    label->SetName("B");

    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0 }, { QString("Group 0"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(4, model.rowCount(GetIndex(model, { 0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,0 }, { QString("A"), QVariant(true), QVariant(QColor(1,1,1)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1 }, { QString("B (2 instances)"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(GetIndex(model, { 0,1 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1,0 }, { QString("B [4]"), QVariant(true), QVariant(QColor(4,4,4)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,1,1 }, { QString("B [10]"), QVariant(true), QVariant(QColor(10,10,10)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,2 }, { QString("C"), QVariant(true), QVariant(QColor(5,5,5)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,2 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 0,3 }, { QString("D"), QVariant(true), QVariant(QColor(2,2,2)), QVariant(true) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 0,3 })));
    CheckModelGroup1Default(model);
    CheckModelGroup2Default(model);
  }

  void ModifyLabelTest()
  {
    QmitkMultiLabelTreeModel model(nullptr);
    model.SetSegmentation(m_Segmentation);

    auto label = m_Segmentation->GetLabel(9, 2);

    //check single instance modifications
    label->SetVisible(false);
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CheckModelGroup0Default(model);
    CheckModelGroup1Default(model);
    CPPUNIT_ASSERT(CheckModelRow(model, { 2 }, { QString("Group 2"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(1, model.rowCount(GetIndex(model, { 2 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 2,0 }, { QString("B"), QVariant(true), QVariant(QColor(9,9,9)), QVariant(false) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 2,0 })));

    label->SetLocked(false);
    label->SetColor(mitk::Color(22 / 255.));
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CheckModelGroup0Default(model);
    CheckModelGroup1Default(model);
    CPPUNIT_ASSERT(CheckModelRow(model, { 2 }, { QString("Group 2"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(1, model.rowCount(GetIndex(model, { 2 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 2,0 }, { QString("B"), QVariant(false), QVariant(QColor(22,22,22)), QVariant(false) }));
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(GetIndex(model, { 2,0 })));

    //check instance modifications with multi instance label
    m_Segmentation->SetActiveLayer(2);
    m_Segmentation->GetActiveLabelSet()->AddLabel(CreateLabel("B", 33));
    label->SetVisible(true);
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(QModelIndex()));
    CheckModelGroup0Default(model);
    CheckModelGroup1Default(model);
    CPPUNIT_ASSERT(CheckModelRow(model, { 2 }, { QString("Group 2"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(1, model.rowCount(GetIndex(model, { 2 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 2,0 }, { QString("B (2 instances)"), QVariant(), QVariant(), QVariant() }));
    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(GetIndex(model, { 2,0 })));
    CPPUNIT_ASSERT(CheckModelRow(model, { 2,0,0 }, { QString("B [9]"), QVariant(false), QVariant(QColor(22,22,22)), QVariant(true) }));
    CPPUNIT_ASSERT(CheckModelRow(model, { 2,0,1 }, { QString("B [33]"), QVariant(true), QVariant(QColor(33,33,33)), QVariant(true) }));
  }

};

MITK_TEST_SUITE_REGISTRATION(QmitkMultiLabelTreeModel)
