/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkAbstractNodeSelectionWidget.h>
#include <QmitkMxNDynamicLayoutDialog.h>
#include <QmitkMxNLayoutEditorWidget.h>
#include <QmitkMxNMultiWidget.h>

#include <mitkImageGenerator.h>
#include <mitkPropertyNameHelper.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStringProperty.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
#include <QToolButton>

#include <memory>
#include <optional>

/**
 * The dynamic layout dialog: its slots filled from the selection it opens
 * with, Apply held back while a slot is short, options that follow the data
 * until the user sets them, "Swap roles", and the layout editor reopening it
 * on the template chosen last.
 */
class QmitkMxNDynamicLayoutDialogTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNDynamicLayoutDialogTestSuite);

  MITK_TEST(Prefill_FillsTheSlotsFromTheSelection);
  MITK_TEST(Apply_DisabledWhileASlotIsShort_NamingIt);
  MITK_TEST(Apply_GeneratesTheLayoutAndAccepts);
  MITK_TEST(Options_FollowTheSlotsUntilChanged);
  MITK_TEST(SwapRoles_ExchangesTheSlotsAndMovesTheDefaults);
  MITK_TEST(Reopen_StartsOnTheTemplateChosenLast);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;

public:
  void setUp() override
  {
    EnsureQApplication();
    m_DataStorage = mitk::StandaloneDataStorage::New();
  }

  void tearDown() override
  {
    m_DataStorage = nullptr;
  }

  mitk::DataNode::Pointer AddImage(const std::string& name)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    node->SetData(mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 16));
    m_DataStorage->Add(node);
    return node;
  }

  void AddSurface(const mitk::DataNode::Pointer& parent)
  {
    auto node = mitk::DataNode::New();
    node->SetName(parent->GetName() + " surface");
    node->SetData(mitk::Surface::New());
    m_DataStorage->Add(node, parent);
  }

  static void SetModality(const mitk::DataNode::Pointer& node, const std::string& modality)
  {
    node->GetData()->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0060).c_str(),
                                 mitk::StringProperty::New(modality));
  }

  std::unique_ptr<QmitkMxNDynamicLayoutDialog> MakeDialog(const QList<mitk::DataNode::Pointer>& selection,
                                                          QmitkMxNDynamicTemplate initialTemplate) const
  {
    return std::make_unique<QmitkMxNDynamicLayoutDialog>(m_DataStorage, selection, QStringLiteral("mxn"), 1.6,
                                                         initialTemplate);
  }

  template <typename T>
  static T* Control(const QWidget& dialog, const char* name)
  {
    auto* control = dialog.findChild<T*>(QString::fromLatin1(name));
    CPPUNIT_ASSERT_MESSAGE(std::string("The dialog has '") + name + "'", nullptr != control);
    return control;
  }

  static QmitkAbstractNodeSelectionWidget* Slot(const QWidget& dialog, int index)
  {
    return Control<QmitkAbstractNodeSelectionWidget>(dialog, ("slot" + std::to_string(index)).c_str());
  }

  void Prefill_FillsTheSlotsFromTheSelection()
  {
    const auto a = this->AddImage("a");
    const auto b = this->AddImage("b");
    auto dialog = this->MakeDialog({ a, b }, QmitkMxNDynamicTemplate::DatasetsByPlanes);
    CPPUNIT_ASSERT_MESSAGE("A multi slot takes the selection",
                           (std::vector<mitk::DataNode::Pointer>{ a, b }) == dialog->GetBindings().at(QmitkMxNSlot::Datasets));

    dialog->SelectTemplate(QmitkMxNDynamicTemplate::FusionPair);
    const auto bindings = dialog->GetBindings();
    CPPUNIT_ASSERT_MESSAGE("Switching the template fills its slots from the same selection",
                           a == bindings.at(QmitkMxNSlot::Anatomy).front() && b == bindings.at(QmitkMxNSlot::FusedImage).front());
  }

  void Apply_DisabledWhileASlotIsShort_NamingIt()
  {
    const auto a = this->AddImage("a");
    auto dialog = this->MakeDialog({ a }, QmitkMxNDynamicTemplate::FusionPair);
    CPPUNIT_ASSERT_MESSAGE("Apply waits for the fused image", !Control<QPushButton>(*dialog, "apply")->isEnabled());
    CPPUNIT_ASSERT_EQUAL(std::string("Select the fused image"),
                         Control<QLabel>(*dialog, "message")->text().toStdString());

    dialog->SelectTemplate(QmitkMxNDynamicTemplate::OneViewPerDataset);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A multi slot names its minimum", std::string("Select at least 2 datasets"),
                                 Control<QLabel>(*dialog, "message")->text().toStdString());

    Slot(*dialog, 0)->SetCurrentSelection({ a, this->AddImage("b") });
    CPPUNIT_ASSERT_MESSAGE("Enough data: Apply is enabled", Control<QPushButton>(*dialog, "apply")->isEnabled());
    CPPUNIT_ASSERT_MESSAGE("...and nothing is asked for", Control<QLabel>(*dialog, "message")->text().isEmpty());
  }

  void Apply_GeneratesTheLayoutAndAccepts()
  {
    const auto a = this->AddImage("a");
    auto dialog = this->MakeDialog({ a }, QmitkMxNDynamicTemplate::DatasetsByPlanes);
    Control<QPushButton>(*dialog, "apply")->click();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Apply accepts", static_cast<int>(QDialog::Accepted), dialog->result());
    CPPUNIT_ASSERT_MESSAGE("...with the generated layout",
                           dialog->GetLayout().document.is_object() && 1 == dialog->GetLayout().groupNodes.size());
  }

  void Options_FollowTheSlotsUntilChanged()
  {
    const auto withSurface = this->AddImage("with surface");
    this->AddSurface(withSurface);
    const auto plain = this->AddImage("plain");
    auto dialog = this->MakeDialog({ withSurface }, QmitkMxNDynamicTemplate::DatasetsByPlanes);
    auto* threeD = Control<QCheckBox>(*dialog, "threeDColumn");
    CPPUNIT_ASSERT_MESSAGE("A 3D representation turns the 3D column on", threeD->isChecked());

    Slot(*dialog, 0)->SetCurrentSelection({ plain });
    CPPUNIT_ASSERT_MESSAGE("Without one, the option follows the data off", !threeD->isChecked());

    Slot(*dialog, 0)->SetCurrentSelection({ withSurface });
    CPPUNIT_ASSERT_MESSAGE("...and on again", threeD->isChecked());
    threeD->setChecked(false);
    Slot(*dialog, 0)->SetCurrentSelection({ plain });
    Slot(*dialog, 0)->SetCurrentSelection({ withSurface });
    CPPUNIT_ASSERT_MESSAGE("Once the user has set it, it stays", !threeD->isChecked());
    CPPUNIT_ASSERT_MESSAGE("The options report it", !std::get<QmitkMxNDatasetsByPlanesOptions>(dialog->GetOptions()).threeDColumn);
  }

  void SwapRoles_ExchangesTheSlotsAndMovesTheDefaults()
  {
    const auto pt = this->AddImage("pt");
    const auto mr = this->AddImage("mr");
    SetModality(pt, "PT");
    SetModality(mr, "MR");
    auto dialog = this->MakeDialog({ mr, pt }, QmitkMxNDynamicTemplate::FusionPair);
    auto* colormap = Control<QComboBox>(*dialog, "colormap");
    auto* mip = Control<QCheckBox>(*dialog, "mipWindow");
    CPPUNIT_ASSERT_MESSAGE("Fixture: a PT fused image gets the PET defaults",
                           static_cast<int>(QmitkMxNFusionColormap::PetColour) == colormap->currentData().toInt()
                             && mip->isChecked());

    Control<QPushButton>(*dialog, "swapRoles")->click();
    const auto bindings = dialog->GetBindings();
    CPPUNIT_ASSERT_MESSAGE("Swap roles exchanges the two slots",
                           pt == bindings.at(QmitkMxNSlot::Anatomy).front() && mr == bindings.at(QmitkMxNSlot::FusedImage).front());
    CPPUNIT_ASSERT_MESSAGE("...and the defaults follow the new fused image",
                           static_cast<int>(QmitkMxNFusionColormap::HotIron) == colormap->currentData().toInt()
                             && !mip->isChecked());
  }

  void Reopen_StartsOnTheTemplateChosenLast()
  {
    this->AddImage("a");
    QmitkMxNMultiWidget editor;
    editor.SetDataStorage(m_DataStorage);
    editor.InitializeMultiWidget();
    QmitkMxNLayoutEditorWidget layoutEditor;
    layoutEditor.SetMultiWidget(&editor);
    layoutEditor.SetDataStorage(m_DataStorage);
    QToolButton* editGrid = nullptr;
    for (auto* button : layoutEditor.findChildren<QToolButton*>())
    {
      if (button->text() == QStringLiteral("Edit grid..."))
      {
        editGrid = button;
      }
    }
    CPPUNIT_ASSERT(nullptr != editGrid);

    // Through the grid picker into the dialog; 'answer' is what to do there.
    const auto open = [editGrid](const std::function<void(QmitkMxNDynamicLayoutDialog&)>& answer) {
      std::optional<QmitkMxNDynamicTemplate> opensOn;
      QPointer<QDialog> gridDialog;
      QTimer poll;
      poll.setInterval(5);
      QObject::connect(&poll, &QTimer::timeout, [&]()
      {
        auto* modal = QApplication::activeModalWidget();
        if (auto* dialog = qobject_cast<QmitkMxNDynamicLayoutDialog*>(modal))
        {
          if (!opensOn.has_value())
          {
            opensOn = dialog->GetTemplate();
            answer(*dialog);
          }
          dialog->reject();
        }
        else if (auto* picker = qobject_cast<QDialog*>(modal); nullptr != picker && gridDialog.isNull())
        {
          gridDialog = picker;
          picker->findChild<QPushButton*>(QStringLiteral("dynamicLayoutButton"))->click();
        }
      });
      poll.start();
      editGrid->click();
      poll.stop();
      return opensOn;
    };

    const auto first = open([](QmitkMxNDynamicLayoutDialog& dialog) {
      Control<QRadioButton>(dialog, "template3")->click();
    });
    CPPUNIT_ASSERT_MESSAGE("The dialog opened", first.has_value());
    const auto second = open([](QmitkMxNDynamicLayoutDialog&) {});
    CPPUNIT_ASSERT_MESSAGE("Reopened on the template chosen last, even after a cancel",
                           second.has_value() && QmitkMxNDynamicTemplate::FusionPair == *second);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNDynamicLayoutDialog)
