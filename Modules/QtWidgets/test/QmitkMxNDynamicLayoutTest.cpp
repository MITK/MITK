/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNDynamicLayout.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkBaseRenderer.h>
#include <mitkImageGenerator.h>
#include <mitkInteractionConst.h>
#include <mitkLabelSetImage.h>
#include <mitkLevelWindow.h>
#include <mitkLookupTableProperty.h>
#include <mitkPointSet.h>
#include <mitkPropertyNameHelper.h>
#include <mitkRenderingManager.h>
#include <mitkRotationOperation.h>
#include <mitkSliceNavigationController.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStepper.h>
#include <mitkStringProperty.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkTimeNavigationController.h>
#include <mitkTransferFunctionProperty.h>

#include <nlohmann/json.hpp>

#include <usModule.h>
#include <usModuleRegistry.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeMapper.h>

#include <QCoreApplication>

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

/**
 * Dynamic layouts: the generators' rules and documents, and what applying a
 * generated layout does to the display and the data - plus the per-renderer
 * cleanup that keeps a reused window id from inheriting a removed window's
 * appearance.
 */
class QmitkMxNDynamicLayoutTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNDynamicLayoutTestSuite);

  MITK_TEST(DataClasses_ImagesSegmentationsAndNeither);
  MITK_TEST(R1_Prefill_TakesTheSelectionInItsOrderPerDataClass);
  MITK_TEST(R2_DatasetIsTheImageWithItsSegmentationDerivations);
  MITK_TEST(R3_SharedBase_FromTheSelectionOrTheDataTree);
  MITK_TEST(R4_Aligned_ByIdenticalGeometry);
  MITK_TEST(T1_Aligned_ByFrameOfReference_PresentAndAbsent);
  MITK_TEST(R5_ThreeD_FollowsThreeDRepresentations);
  MITK_TEST(R6_GridShape_ReproducesTheTable);
  MITK_TEST(T2_FusionRoles_ByModality_PresentAndAbsent);
  MITK_TEST(T3_FusionLook_ByTheFusedModality_PresentAndAbsent);
  MITK_TEST(T4_DatasetOrder_ByAcquisitionDate_PresentAndAbsent);
  MITK_TEST(Documents_EveryCombinationLoadsWithExactlyItsGroups);
  MITK_TEST(DatasetsByPlanes_Design);
  MITK_TEST(OneViewPerDataset_Design);
  MITK_TEST(SharedBase_Design);
  MITK_TEST(FusionPair_Design);
  MITK_TEST(Generate_ThrowsForEachContractViolation);
  MITK_TEST(Apply_BindsEveryGroupToExactlyItsNodes);
  MITK_TEST(Apply_WritesRendererStateOnTheNamedWindowsOnly);
  MITK_TEST(Apply_FitsTheViewsAndKeepsTheTimeStep);
  MITK_TEST(Apply_ObliqueDataset_ShownInItsAcquisitionPlanes);
  MITK_TEST(Apply_FrameOfReferenceAlignedDatasets_FitTheirJointExtent);
  MITK_TEST(Apply_MixedTimeData_GlobalTimeCoversAll);
  MITK_TEST(Apply_WhileHidden_GlobalTimeStillCoversAllOnceShown);
  MITK_TEST(Apply_ObjectFormSelectionLinks_Accepted);
  MITK_TEST(Apply_ValidationFailuresChangeNothing);
  MITK_TEST(Cleanup_ReusedIdInheritsNoRendererState);
  MITK_TEST(Cleanup_UnlinkKeepsTheCellsOtherRendererState);
  MITK_TEST(Cleanup_PresetAfterADynamicLayoutShowsTheGlobalOpacity);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

public:
  void setUp() override
  {
    EnsureQApplication();
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_DataStorage = nullptr;
  }

  static void Pump()
  {
    for (int i = 0; i < 20; ++i)
    {
      QCoreApplication::processEvents();
    }
  }

  mitk::DataNode::Pointer AddImage(const std::string& name, unsigned int size = 16)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    node->SetData(mitk::ImageGenerator::GenerateGradientImage<short>(size, size, size, 1.0f, 1.0f, 1.0f));
    node->SetIntProperty("layer", 0);
    m_DataStorage->Add(node);
    return node;
  }

  mitk::BaseRenderer* Renderer(const QString& windowId) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(windowId);
    CPPUNIT_ASSERT_MESSAGE("Window " + windowId.toStdString() + " exists", nullptr != cell);
    return mitk::BaseRenderer::GetInstance(cell->GetRenderWindow()->GetVtkRenderWindow());
  }

  /** Whether 'node' carries 'key' for the renderer named 'windowId', without
   *  creating that renderer's property list. */
  static bool HasRendererProperty(const mitk::DataNode& node, const QString& windowId, const std::string& key)
  {
    const auto context = windowId.toStdString();
    const auto contexts = node.GetPropertyListNames();
    if (std::find(contexts.begin(), contexts.end(), context) == contexts.end())
    {
      return false;
    }
    return nullptr != node.GetPropertyList(context)->GetProperty(key);
  }

  mitk::DataNode::Pointer AddDerived(const mitk::DataNode::Pointer& parent, mitk::BaseData* data,
                                     const std::string& name, int layer)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    node->SetData(data);
    node->SetIntProperty("layer", layer);
    m_DataStorage->Add(node, parent);
    return node;
  }

  mitk::DataNode::Pointer AddMask(const mitk::DataNode::Pointer& parent, const std::string& name, int layer)
  {
    auto node = this->AddDerived(parent, mitk::ImageGenerator::GenerateGradientImage<unsigned char>(16, 16, 16), name,
                                 layer);
    node->SetBoolProperty("binary", true);
    return node;
  }

  mitk::DataNode::Pointer AddSegmentation(const mitk::DataNode::Pointer& parent, const std::string& name, int layer)
  {
    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(dynamic_cast<mitk::Image*>(parent->GetData()));
    return this->AddDerived(parent, segmentation, name, layer);
  }

  static void SetTag(const mitk::DataNode::Pointer& node, unsigned int group, unsigned int element,
                     const std::string& value)
  {
    node->GetData()->SetProperty(mitk::GeneratePropertyNameForDICOMTag(group, element).c_str(),
                                 mitk::StringProperty::New(value));
  }

  static void SetModality(const mitk::DataNode::Pointer& node, const std::string& modality)
  {
    SetTag(node, 0x0008, 0x0060, modality);
  }

  QmitkMxNDynamicLayout Generate(const QmitkMxNSlotBindings& bindings, const QmitkMxNDynamicOptions& options,
                                 double aspect = 1.6) const
  {
    return QmitkMxNDynamicLayouts::Generate(bindings, options, QStringLiteral("mxn"), aspect, *m_DataStorage);
  }

  /** The window leaves of a document, in pre-order. */
  static std::vector<nlohmann::json> Windows(const nlohmann::json& document)
  {
    std::vector<nlohmann::json> windows;
    std::vector<const nlohmann::json*> stack{ &document.at("root") };
    std::function<void(const nlohmann::json&)> walk = [&windows, &walk](const nlohmann::json& node) {
      if ("window" == node.at("type").get<std::string>())
      {
        windows.push_back(node);
        return;
      }
      for (const auto& child : node.at("children"))
      {
        walk(child);
      }
    };
    walk(document.at("root"));
    return windows;
  }

  static std::set<std::string> DeclaredGroups(const nlohmann::json& document)
  {
    std::set<std::string> groups;
    for (auto it = document.at("groups").begin(); it != document.at("groups").end(); ++it)
    {
      groups.insert(it.key());
    }
    return groups;
  }

  std::set<std::string> EditorGroups() const
  {
    std::set<std::string> groups;
    for (const auto& info : m_Editor->GetSyncGroupInfos())
    {
      groups.insert(info.id);
    }
    return groups;
  }

  static std::string Link(const nlohmann::json& window, const char* key)
  {
    const auto& links = window.at("links");
    if (!links.contains(key))
    {
      return std::string();
    }
    const auto& value = links.at(key);
    return value.is_string() ? value.get<std::string>() : value.at("target").get<std::string>();
  }

  static std::string Name(const nlohmann::json& window)
  {
    return window.value("name", std::string());
  }

  static bool IsThreeD(const nlohmann::json& window)
  {
    return "3d" == window.value("render_mode", std::string());
  }

  // --- Data classes and rules --------------------------------------------------

  void DataClasses_ImagesSegmentationsAndNeither()
  {
    using QmitkMxNDynamicLayouts::IsOfDataClass;
    const auto image = this->AddImage("image");
    const auto mask = this->AddMask(image, "mask", 1);
    const auto segmentation = this->AddSegmentation(image, "segmentation", 1);
    const auto surface = this->AddDerived(image, mitk::Surface::New(), "surface", 1);
    const auto points = this->AddDerived(image, mitk::PointSet::New(), "points", 1);
    auto helper = this->AddImage("helper");
    helper->SetBoolProperty("helper object", true);
    auto hidden = this->AddImage("hidden");
    hidden->SetBoolProperty("hidden object", true);

    CPPUNIT_ASSERT(IsOfDataClass(*image, QmitkMxNSlotDataClass::Image));
    CPPUNIT_ASSERT(!IsOfDataClass(*image, QmitkMxNSlotDataClass::Segmentation));
    for (const auto& candidate : { mask, segmentation, surface, points })
    {
      CPPUNIT_ASSERT_MESSAGE(candidate->GetName() + " is a segmentation",
                             IsOfDataClass(*candidate, QmitkMxNSlotDataClass::Segmentation));
      CPPUNIT_ASSERT_MESSAGE(candidate->GetName() + " is no image", !IsOfDataClass(*candidate, QmitkMxNSlotDataClass::Image));
    }
    for (const auto& neither : { helper, hidden })
    {
      CPPUNIT_ASSERT(!IsOfDataClass(*neither, QmitkMxNSlotDataClass::Image));
      CPPUNIT_ASSERT(!IsOfDataClass(*neither, QmitkMxNSlotDataClass::Segmentation));
    }
  }

  void R1_Prefill_TakesTheSelectionInItsOrderPerDataClass()
  {
    const auto a = this->AddImage("a");
    const auto b = this->AddImage("b");
    const auto c = this->AddImage("c");
    const auto d = this->AddImage("d");
    const auto e = this->AddImage("e");
    const auto mask = this->AddMask(a, "mask", 1);

    const auto datasets =
      QmitkMxNDynamicLayouts::Prefill(QmitkMxNDynamicTemplate::DatasetsByPlanes, { mask, c, a, b, d, e }, *m_DataStorage);
    CPPUNIT_ASSERT_MESSAGE("A multi slot takes up to its maximum, in selection order, of its class only",
                           (std::vector<mitk::DataNode::Pointer>{ c, a, b, d }) == datasets.at(QmitkMxNSlot::Datasets));

    const auto fusion =
      QmitkMxNDynamicLayouts::Prefill(QmitkMxNDynamicTemplate::FusionPair, { mask, c, a }, *m_DataStorage);
    CPPUNIT_ASSERT_MESSAGE("A single slot takes the first match",
                           (std::vector<mitk::DataNode::Pointer>{ c }) == fusion.at(QmitkMxNSlot::Anatomy));
    CPPUNIT_ASSERT_MESSAGE("...the next one the first match no earlier slot took",
                           (std::vector<mitk::DataNode::Pointer>{ a }) == fusion.at(QmitkMxNSlot::FusedImage));
  }

  void R2_DatasetIsTheImageWithItsSegmentationDerivations()
  {
    const auto image = this->AddImage("image");
    const auto mask = this->AddMask(image, "mask", 3);
    const auto surface = this->AddDerived(image, mitk::Surface::New(), "surface", 1);
    const auto derived = this->AddDerived(image, mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 16),
                                          "derived image", 2);

    const auto layout = this->Generate({ { QmitkMxNSlot::Datasets, { image, derived } } },
                                       QmitkMxNDatasetsByPlanesOptions{});
    CPPUNIT_ASSERT_MESSAGE("The image, then its segmentations by layer; the derived image is not one",
                           (std::vector<mitk::DataNode::Pointer>{ image, surface, mask }) == layout.groupNodes.at("ds00"));
    CPPUNIT_ASSERT_MESSAGE("A derived image that is not binary is a dataset of its own",
                           (std::vector<mitk::DataNode::Pointer>{ derived }) == layout.groupNodes.at("ds01"));
  }

  void R3_SharedBase_FromTheSelectionOrTheDataTree()
  {
    const auto base = this->AddImage("base");
    const auto first = this->AddMask(base, "first", 1);
    const auto second = this->AddMask(base, "second", 2);
    this->AddImage("other");

    auto bindings = QmitkMxNDynamicLayouts::Prefill(QmitkMxNDynamicTemplate::SharedBase, { second, base }, *m_DataStorage);
    CPPUNIT_ASSERT_MESSAGE("The selected image is the base",
                           (std::vector<mitk::DataNode::Pointer>{ base }) == bindings.at(QmitkMxNSlot::BaseImage));
    CPPUNIT_ASSERT_MESSAGE("...and the selected segmentations its segmentations",
                           (std::vector<mitk::DataNode::Pointer>{ second }) == bindings.at(QmitkMxNSlot::Segmentations));

    bindings = QmitkMxNDynamicLayouts::Prefill(QmitkMxNDynamicTemplate::SharedBase, { first, second }, *m_DataStorage);
    CPPUNIT_ASSERT_MESSAGE("Without a selected image, the source the segmentations share is the base",
                           (std::vector<mitk::DataNode::Pointer>{ base }) == bindings.at(QmitkMxNSlot::BaseImage));

    bindings = QmitkMxNDynamicLayouts::Prefill(QmitkMxNDynamicTemplate::SharedBase, { base }, *m_DataStorage);
    CPPUNIT_ASSERT_MESSAGE("Without selected segmentations, the base's segmentations are taken",
                           (std::vector<mitk::DataNode::Pointer>{ first, second }) == bindings.at(QmitkMxNSlot::Segmentations));
  }

  static QmitkMxNDatasetsByPlanesOptions DatasetsByPlanesDefaults(const QmitkMxNSlotBindings& bindings,
                                                                  const mitk::DataStorage& dataStorage)
  {
    return std::get<QmitkMxNDatasetsByPlanesOptions>(
      QmitkMxNDynamicLayouts::DefaultOptions(QmitkMxNDynamicTemplate::DatasetsByPlanes, bindings, dataStorage));
  }

  void R4_Aligned_ByIdenticalGeometry()
  {
    const auto a = this->AddImage("a");
    const auto b = this->AddImage("b");
    const auto c = this->AddImage("c", 24);
    CPPUNIT_ASSERT_MESSAGE("Identical geometries are aligned",
                           DatasetsByPlanesDefaults({ { QmitkMxNSlot::Datasets, { a, b } } }, *m_DataStorage).aligned);
    CPPUNIT_ASSERT_MESSAGE("Different geometries are not",
                           !DatasetsByPlanesDefaults({ { QmitkMxNSlot::Datasets, { a, c } } }, *m_DataStorage).aligned);
  }

  void T1_Aligned_ByFrameOfReference_PresentAndAbsent()
  {
    const auto a = this->AddImage("a");
    const auto c = this->AddImage("c", 24);
    const QmitkMxNSlotBindings bindings{ { QmitkMxNSlot::Datasets, { a, c } } };
    CPPUNIT_ASSERT_MESSAGE("Absent: different geometries are not aligned",
                           !DatasetsByPlanesDefaults(bindings, *m_DataStorage).aligned);

    SetTag(a, 0x0020, 0x0052, "1.2.840.1");
    SetTag(c, 0x0020, 0x0052, "1.2.840.1");
    CPPUNIT_ASSERT_MESSAGE("Present and equal: aligned", DatasetsByPlanesDefaults(bindings, *m_DataStorage).aligned);

    SetTag(c, 0x0020, 0x0052, "1.2.840.2");
    CPPUNIT_ASSERT_MESSAGE("Present and different: not aligned",
                           !DatasetsByPlanesDefaults(bindings, *m_DataStorage).aligned);
  }

  void R5_ThreeD_FollowsThreeDRepresentations()
  {
    const auto withSurface = this->AddImage("with surface");
    this->AddDerived(withSurface, mitk::Surface::New(), "surface", 1);
    const auto plain = this->AddImage("plain");
    const auto mask = this->AddMask(plain, "mask", 1);

    CPPUNIT_ASSERT_MESSAGE("A 3D column when any dataset has a 3D representation",
                           DatasetsByPlanesDefaults({ { QmitkMxNSlot::Datasets, { withSurface, plain } } }, *m_DataStorage)
                             .threeDColumn);
    CPPUNIT_ASSERT_MESSAGE("None without one; a binary mask is none",
                           !DatasetsByPlanesDefaults({ { QmitkMxNSlot::Datasets, { plain } } }, *m_DataStorage).threeDColumn);

    QmitkMxNOneViewPerDatasetOptions threeD;
    threeD.view = QmitkMxNDatasetView::ThreeD;
    CPPUNIT_ASSERT_THROW_MESSAGE("The 3D view needs a 3D representation for every dataset",
                                 this->Generate({ { QmitkMxNSlot::Datasets, { withSurface, plain } } }, threeD),
                                 mitk::Exception);
    this->AddSegmentation(plain, "segmentation", 2);
    CPPUNIT_ASSERT_NO_THROW(this->Generate({ { QmitkMxNSlot::Datasets, { withSurface, plain } } }, threeD));

    const auto sharedBase = [this](const mitk::DataNode::Pointer& base, const mitk::DataNode::Pointer& segmentation) {
      return std::get<QmitkMxNSharedBaseOptions>(QmitkMxNDynamicLayouts::DefaultOptions(
        QmitkMxNDynamicTemplate::SharedBase,
        { { QmitkMxNSlot::BaseImage, { base } }, { QmitkMxNSlot::Segmentations, { segmentation } } }, *m_DataStorage));
    };
    CPPUNIT_ASSERT_MESSAGE("No 3D row for a binary segmentation", !sharedBase(plain, mask).threeDRow);
    const auto segmentation = this->AddSegmentation(plain, "second segmentation", 3);
    CPPUNIT_ASSERT_MESSAGE("A 3D row for a segmentation", sharedBase(plain, segmentation).threeDRow);
  }

  void R6_GridShape_ReproducesTheTable()
  {
    const std::pair<std::size_t, std::size_t> expected[] = { { 2, 1 }, { 3, 1 }, { 2, 2 }, { 3, 2 }, { 3, 2 },
                                                             { 4, 2 }, { 4, 2 }, { 3, 3 }, { 5, 2 }, { 4, 3 },
                                                             { 4, 3 }, { 5, 3 }, { 5, 3 }, { 5, 3 }, { 4, 4 } };
    std::vector<mitk::DataNode::Pointer> images;
    for (int i = 0; i < 16; ++i)
    {
      images.push_back(this->AddImage("image " + std::to_string(i), 4));
    }
    for (std::size_t n = 2; n <= 16; ++n)
    {
      const auto layout = this->Generate(
        { { QmitkMxNSlot::Datasets, std::vector<mitk::DataNode::Pointer>(images.begin(), images.begin() + n) } },
        QmitkMxNOneViewPerDatasetOptions{});
      const auto& rows = layout.document.at("root").at("children");
      const std::string where = " for " + std::to_string(n) + " datasets";
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Columns" + where, expected[n - 2].first, rows.at(0).at("children").size());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Rows" + where, expected[n - 2].second, rows.size());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Windows" + where, n, Windows(layout.document).size());
    }
  }

  void T2_FusionRoles_ByModality_PresentAndAbsent()
  {
    const auto pt = this->AddImage("pt");
    const auto ct = this->AddImage("ct");

    auto bindings = QmitkMxNDynamicLayouts::Prefill(QmitkMxNDynamicTemplate::FusionPair, { pt, ct }, *m_DataStorage);
    CPPUNIT_ASSERT_MESSAGE("Absent: the selection order decides",
                           pt == bindings.at(QmitkMxNSlot::Anatomy).front()
                             && ct == bindings.at(QmitkMxNSlot::FusedImage).front());

    SetModality(pt, "PT");
    SetModality(ct, "CT");
    bindings = QmitkMxNDynamicLayouts::Prefill(QmitkMxNDynamicTemplate::FusionPair, { pt, ct }, *m_DataStorage);
    CPPUNIT_ASSERT_MESSAGE("Present: the CT is the anatomy, the PT the fused image",
                           ct == bindings.at(QmitkMxNSlot::Anatomy).front()
                             && pt == bindings.at(QmitkMxNSlot::FusedImage).front());
  }

  void T3_FusionLook_ByTheFusedModality_PresentAndAbsent()
  {
    const auto anatomy = this->AddImage("anatomy");
    const auto fused = this->AddImage("fused");
    const QmitkMxNSlotBindings bindings{ { QmitkMxNSlot::Anatomy, { anatomy } }, { QmitkMxNSlot::FusedImage, { fused } } };
    const auto defaults = [this, &bindings]() {
      return std::get<QmitkMxNFusionPairOptions>(
        QmitkMxNDynamicLayouts::DefaultOptions(QmitkMxNDynamicTemplate::FusionPair, bindings, *m_DataStorage));
    };

    auto options = defaults();
    CPPUNIT_ASSERT_MESSAGE("Absent: the PET defaults",
                           QmitkMxNFusionColormap::PetColour == options.colormap && options.mipWindow);
    SetModality(fused, "PT");
    options = defaults();
    CPPUNIT_ASSERT_MESSAGE("A PT: the PET defaults", QmitkMxNFusionColormap::PetColour == options.colormap && options.mipWindow);
    SetModality(fused, "MR");
    options = defaults();
    CPPUNIT_ASSERT_MESSAGE("Known and not PT: hot iron and no MIP window",
                           QmitkMxNFusionColormap::HotIron == options.colormap && !options.mipWindow);
  }

  void T4_DatasetOrder_ByAcquisitionDate_PresentAndAbsent()
  {
    const auto a = this->AddImage("a");
    const auto b = this->AddImage("b");
    const auto c = this->AddImage("c");
    const QmitkMxNSlotBindings bindings{ { QmitkMxNSlot::Datasets, { a, b, c } } };
    const auto defaults = [this, &bindings]() {
      return std::get<QmitkMxNOneViewPerDatasetOptions>(
        QmitkMxNDynamicLayouts::DefaultOptions(QmitkMxNDynamicTemplate::OneViewPerDataset, bindings, *m_DataStorage));
    };

    CPPUNIT_ASSERT_MESSAGE("Absent: the selection order", QmitkMxNDatasetOrder::Selection == defaults().order);
    SetTag(a, 0x0008, 0x0022, "20240301");
    SetTag(b, 0x0008, 0x0022, "20230101");
    CPPUNIT_ASSERT_MESSAGE("Missing on one dataset: the selection order", QmitkMxNDatasetOrder::Selection == defaults().order);
    SetTag(c, 0x0008, 0x0022, "20240301");
    const auto options = defaults();
    CPPUNIT_ASSERT_MESSAGE("Present on every dataset: by date", QmitkMxNDatasetOrder::AcquisitionDate == options.order);

    const auto layout = this->Generate(bindings, options);
    CPPUNIT_ASSERT_MESSAGE("The earliest first; same-day datasets keep the selection order",
                           b == layout.groupNodes.at("ds00").front() && a == layout.groupNodes.at("ds01").front()
                             && c == layout.groupNodes.at("ds02").front());
  }

  // --- Documents ---------------------------------------------------------------

  void LoadAndCheck(const QmitkMxNDynamicLayout& layout, std::size_t windowCount, const std::string& where)
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The window count" + where, windowCount, Windows(layout.document).size());
    try
    {
      m_Editor->ApplyLayout(layout.document);
    }
    catch (const mitk::Exception& e)
    {
      CPPUNIT_FAIL("The document loads" + where + ": " + e.GetDescription());
    }
    CPPUNIT_ASSERT_MESSAGE("Exactly the declared groups, no empty cards" + where,
                           DeclaredGroups(layout.document) == this->EditorGroups());
    for (const auto& window : Windows(layout.document))
    {
      const auto id = QString::fromStdString(window.at("id").get<std::string>());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The window's name" + where, Name(window),
                                   m_Editor->GetRenderWindowWidget(id)->GetDisplayName().toStdString());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The window's data group" + where, Link(window, "selection"),
                                   m_Editor->GetCellSelectionGroup(id));
    }
  }

  void Documents_EveryCombinationLoadsWithExactlyItsGroups()
  {
    const auto a = this->AddImage("a");
    const auto b = this->AddImage("b");
    const auto segmentationA = this->AddSegmentation(a, "segmentation a", 1);
    const auto maskA = this->AddMask(a, "mask a", 2);
    this->AddSegmentation(b, "segmentation b", 1);
    const QmitkMxNSlotBindings datasets{ { QmitkMxNSlot::Datasets, { a, b } } };

    for (const bool aligned : { false, true })
      for (const bool threeD : { false, true })
        for (const bool columns : { false, true })
        {
          const std::string where = " (datasets x planes, aligned " + std::to_string(aligned) + ", 3D "
                                    + std::to_string(threeD) + ", columns " + std::to_string(columns) + ")";
          this->LoadAndCheck(this->Generate(datasets, QmitkMxNDatasetsByPlanesOptions{ aligned, threeD, columns }),
                             2 * (threeD ? 4 : 3), where);
        }

    for (const bool aligned : { false, true })
      for (const auto view : { QmitkMxNDatasetView::Axial, QmitkMxNDatasetView::Sagittal,
                               QmitkMxNDatasetView::Coronal, QmitkMxNDatasetView::ThreeD })
        for (const auto order : { QmitkMxNDatasetOrder::Selection, QmitkMxNDatasetOrder::AcquisitionDate })
        {
          const std::string where = " (one view per dataset, aligned " + std::to_string(aligned) + ", view "
                                    + std::to_string(static_cast<int>(view)) + ")";
          this->LoadAndCheck(this->Generate(datasets, QmitkMxNOneViewPerDatasetOptions{ aligned, view, order }), 2,
                             where);
        }

    const QmitkMxNSlotBindings shared{ { QmitkMxNSlot::BaseImage, { a } },
                                       { QmitkMxNSlot::Segmentations, { segmentationA, maskA } } };
    for (const bool threeDRow : { false, true })
      for (const bool all : { false, true })
      {
        const std::string where = " (shared base, 3D row " + std::to_string(threeDRow) + ", all "
                                  + std::to_string(all) + ")";
        this->LoadAndCheck(this->Generate(shared, QmitkMxNSharedBaseOptions{ threeDRow, all }),
                           (all ? 3 : 2) * (threeDRow ? 2 : 1), where);
      }

    const QmitkMxNSlotBindings fusion{ { QmitkMxNSlot::Anatomy, { a } }, { QmitkMxNSlot::FusedImage, { b } } };
    for (const auto colormap : { QmitkMxNFusionColormap::PetColour, QmitkMxNFusionColormap::HotIron,
                                 QmitkMxNFusionColormap::JetTransparentFloor })
      for (const bool mip : { false, true })
        for (const auto opacity : { QmitkMxNFusionOpacity::Percent30, QmitkMxNFusionOpacity::Percent50,
                                    QmitkMxNFusionOpacity::Percent70 })
        {
          const std::string where = " (fusion pair, colormap " + std::to_string(static_cast<int>(colormap)) + ", MIP "
                                    + std::to_string(mip) + ")";
          this->LoadAndCheck(this->Generate(fusion, QmitkMxNFusionPairOptions{ colormap, mip, opacity }), mip ? 7 : 6,
                             where);
        }
  }

  void DatasetsByPlanes_Design()
  {
    const auto a = this->AddImage("a");
    const auto b = this->AddImage("b");
    const QmitkMxNSlotBindings datasets{ { QmitkMxNSlot::Datasets, { a, b } } };

    auto layout = this->Generate(datasets, QmitkMxNDatasetsByPlanesOptions{ true, true, false });
    auto windows = Windows(layout.document);
    CPPUNIT_ASSERT_EQUAL(std::string("vertical"), layout.document.at("root").at("orientation").get<std::string>());
    const char* planes[] = { "ax", "sag", "cor" };
    for (std::size_t i = 0; i < 3; ++i)
    {
      const auto& window = windows[i];
      CPPUNIT_ASSERT_EQUAL(std::string("ds00"), Link(window, "selection"));
      CPPUNIT_ASSERT_EQUAL(std::string("ds00"), Link(window, "windowing"));
      CPPUNIT_ASSERT_EQUAL(std::string("ds00"), Link(window, "lut"));
      CPPUNIT_ASSERT_EQUAL(std::string("nav"), Link(window, "crosshair"));
      CPPUNIT_ASSERT_EQUAL(std::string(planes[i]), Link(window, "slice"));
      CPPUNIT_ASSERT_EQUAL(std::string(planes[i]), Link(window, "zoom"));
      CPPUNIT_ASSERT_EQUAL(std::string(planes[i]), Link(window, "pan"));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Dataset windows are unnamed", std::string(), Name(window));
    }
    CPPUNIT_ASSERT_MESSAGE("The 3D column", IsThreeD(windows[3]) && "ds00" == Link(windows[3], "selection")
                                              && Link(windows[3], "zoom").empty());
    CPPUNIT_ASSERT_EQUAL(std::string("ds01"), Link(windows[4], "selection"));
    CPPUNIT_ASSERT_EQUAL(std::string("nav"), Link(windows[4], "crosshair"));
    std::size_t componentFits = 0;
    for (const auto& fit : layout.fits)
    {
      componentFits += fit.wholeComponent ? 1 : 0;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Aligned: one fit per plane group", std::size_t(3), componentFits);

    layout = this->Generate(datasets, QmitkMxNDatasetsByPlanesOptions{ false, false, true });
    windows = Windows(layout.document);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Datasets as columns", std::string("horizontal"),
                                 layout.document.at("root").at("orientation").get<std::string>());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not aligned: the crosshair stays in the dataset", std::string("ds01"),
                                 Link(windows[3], "crosshair"));
    CPPUNIT_ASSERT_MESSAGE("...and nothing else is shared", Link(windows[3], "slice").empty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Every window fits itself", windows.size(), layout.fits.size());
  }

  void OneViewPerDataset_Design()
  {
    const auto a = this->AddImage("a");
    const auto b = this->AddImage("b");
    const auto c = this->AddImage("c");
    for (const auto& image : { a, b, c })
    {
      this->AddDerived(image, mitk::Surface::New(), image->GetName() + " surface", 1);
    }
    const QmitkMxNSlotBindings datasets{ { QmitkMxNSlot::Datasets, { a, b, c } } };

    auto layout = this->Generate(datasets, QmitkMxNOneViewPerDatasetOptions{ true, QmitkMxNDatasetView::Coronal,
                                                                             QmitkMxNDatasetOrder::Selection });
    for (const auto& window : Windows(layout.document))
    {
      CPPUNIT_ASSERT_EQUAL(std::string("coronal"), window.at("view_direction").get<std::string>());
      for (const auto* key : { "pan", "zoom", "slice", "crosshair" })
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(std::string("Aligned: ") + key + " on one group", std::string("nav"),
                                     Link(window, key));
      }
      CPPUNIT_ASSERT_MESSAGE("No look shared across datasets", Link(window, "windowing").empty() && Link(window, "lut").empty());
    }
    CPPUNIT_ASSERT_MESSAGE("One component fit", 1 == layout.fits.size() && layout.fits.front().wholeComponent);

    layout = this->Generate(datasets, QmitkMxNOneViewPerDatasetOptions{ false, QmitkMxNDatasetView::ThreeD,
                                                                        QmitkMxNDatasetOrder::Selection });
    for (const auto& window : Windows(layout.document))
    {
      CPPUNIT_ASSERT_MESSAGE("A free 3D window", IsThreeD(window) && "free" == window.at("view_direction").get<std::string>());
      CPPUNIT_ASSERT_EQUAL(std::string("spin"), Link(window, "orientation"));
      CPPUNIT_ASSERT_EQUAL(std::string("spin"), Link(window, "zoom"));
      CPPUNIT_ASSERT_MESSAGE("Not aligned: no pan link", Link(window, "pan").empty());
    }
    CPPUNIT_ASSERT_MESSAGE("Not aligned: every window fits itself",
                           3 == layout.fits.size()
                             && std::none_of(layout.fits.begin(), layout.fits.end(),
                                             [](const auto& fit) { return fit.wholeComponent; }));
  }

  void SharedBase_Design()
  {
    const auto base = this->AddImage("base");
    const auto first = this->AddMask(base, "first", 1);
    const auto second = this->AddSegmentation(base, "second", 2);
    const auto layout = this->Generate({ { QmitkMxNSlot::BaseImage, { base } }, { QmitkMxNSlot::Segmentations, { first, second } } },
                                       QmitkMxNSharedBaseOptions{ true, true });
    const auto windows = Windows(layout.document);
    CPPUNIT_ASSERT_EQUAL(std::size_t(6), windows.size());
    const char* groups[] = { "seg0", "seg1", "all" };
    for (std::size_t i = 0; i < 3; ++i)
    {
      CPPUNIT_ASSERT_EQUAL(std::string(groups[i]), Link(windows[i], "selection"));
      CPPUNIT_ASSERT_EQUAL(std::string("nav"), Link(windows[i], "crosshair"));
      CPPUNIT_ASSERT_EQUAL(std::string("look"), Link(windows[i], "windowing"));
      CPPUNIT_ASSERT_EQUAL(std::string("look"), Link(windows[i], "lut"));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The 3D row in the same columns", std::string(groups[i]),
                                   Link(windows[i + 3], "selection"));
      CPPUNIT_ASSERT_EQUAL(std::string("spin"), Link(windows[i + 3], "orientation"));
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The all-segmentations window comes last", std::string("All segmentations"), Name(windows[2]));
    CPPUNIT_ASSERT_EQUAL(std::string("All segmentations"), Name(windows[5]));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Segmentation windows are unnamed", std::string(), Name(windows[0]));
    CPPUNIT_ASSERT_MESSAGE("A column shows the base and its segmentation",
                           (std::vector<mitk::DataNode::Pointer>{ base, second }) == layout.groupNodes.at("seg1"));
    CPPUNIT_ASSERT_MESSAGE("The all-segmentations group shows them all",
                           (std::vector<mitk::DataNode::Pointer>{ base, first, second }) == layout.groupNodes.at("all"));
    CPPUNIT_ASSERT_MESSAGE("Fitted from the all-segmentations window and the first 3D window",
                           2 == layout.fits.size() && windows[2].at("id").get<std::string>() == layout.fits[0].windowId.toStdString()
                             && windows[3].at("id").get<std::string>() == layout.fits[1].windowId.toStdString());
  }

  void FusionPair_Design()
  {
    const auto anatomy = this->AddImage("anatomy");
    const auto fused = this->AddImage("fused");
    const QmitkMxNSlotBindings bindings{ { QmitkMxNSlot::Anatomy, { anatomy } }, { QmitkMxNSlot::FusedImage, { fused } } };

    auto layout = this->Generate(bindings, QmitkMxNFusionPairOptions{});
    auto windows = Windows(layout.document);
    CPPUNIT_ASSERT_EQUAL(std::size_t(7), windows.size());
    const char* names[] = { "Anatomy", "Fused image", "Fusion", "Anatomy", "Fused image", "Fusion", "MIP" };
    for (std::size_t i = 0; i < windows.size(); ++i)
    {
      CPPUNIT_ASSERT_EQUAL(std::string(names[i]), Name(windows[i]));
      CPPUNIT_ASSERT_MESSAGE("No windowing link anywhere", Link(windows[i], "windowing").empty());
    }
    CPPUNIT_ASSERT_EQUAL(std::string("ax"), Link(windows[0], "slice"));
    CPPUNIT_ASSERT_EQUAL(std::string("cor"), Link(windows[3], "slice"));
    CPPUNIT_ASSERT_EQUAL(std::string("fusedlook"), Link(windows[1], "lut"));
    CPPUNIT_ASSERT_EQUAL(std::string("fusionlook"), Link(windows[5], "lut"));
    CPPUNIT_ASSERT_MESSAGE("The MIP window", IsThreeD(windows[6]) && "fused" == Link(windows[6], "selection"));
    CPPUNIT_ASSERT_MESSAGE("The fusion shows the anatomy under the fused image",
                           (std::vector<mitk::DataNode::Pointer>{ anatomy, fused }) == layout.groupNodes.at("fusion"));

    std::map<std::string, mitk::LookupTable::LookupTableType> tables;
    for (const auto& table : layout.lookupTables)
    {
      CPPUNIT_ASSERT(fused == table.node);
      tables[table.windowId.toStdString()] = table.type;
    }
    CPPUNIT_ASSERT_MESSAGE("PET colour: gray in the fused column, the colormap in the fusion",
                           2 == tables.size()
                             && mitk::LookupTable::INVERSE_GRAYSCALE == tables.at(windows[1].at("id").get<std::string>())
                             && mitk::LookupTable::PET_COLOR == tables.at(windows[2].at("id").get<std::string>()));
    CPPUNIT_ASSERT_MESSAGE("The opacity in the two fusion windows",
                           2 == layout.opacities.size() && std::abs(layout.opacities[0].opacity - 0.5f) < 1e-6f);
    CPPUNIT_ASSERT_MESSAGE("A maximum intensity projection in the MIP window",
                           1 == layout.volumeRenderings.size()
                             && QmitkMxNVolumeBlendMode::MaximumIntensity == layout.volumeRenderings[0].blendMode
                             && layout.volumeRenderings[0].transferFunction.IsNotNull());

    layout = this->Generate(bindings, QmitkMxNFusionPairOptions{ QmitkMxNFusionColormap::HotIron, false,
                                                                 QmitkMxNFusionOpacity::Percent70 });
    windows = Windows(layout.document);
    CPPUNIT_ASSERT_EQUAL(std::size_t(6), windows.size());
    CPPUNIT_ASSERT_MESSAGE("Another colormap leaves the fused column alone",
                           Link(windows[1], "lut").empty() && 0 == DeclaredGroups(layout.document).count("fusedlook"));
    CPPUNIT_ASSERT_MESSAGE("...and colors the fusion",
                           1 == layout.lookupTables.size() && mitk::LookupTable::HOT_IRON == layout.lookupTables[0].type);
    CPPUNIT_ASSERT_MESSAGE("No MIP, no volume rendering", layout.volumeRenderings.empty());
  }

  void Generate_ThrowsForEachContractViolation()
  {
    const auto a = this->AddImage("a");
    const auto b = this->AddImage("b");
    const auto mask = this->AddMask(a, "mask", 1);
    const QmitkMxNDatasetsByPlanesOptions planes;

    CPPUNIT_ASSERT_THROW_MESSAGE("A missing slot", this->Generate({}, planes), mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("Fewer than the minimum",
                                 this->Generate({ { QmitkMxNSlot::Datasets, { a } } }, QmitkMxNOneViewPerDatasetOptions{}),
                                 mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("More than the maximum",
                                 this->Generate({ { QmitkMxNSlot::Datasets,
                                                    { a, b, this->AddImage("c"), this->AddImage("d"), this->AddImage("e") } } },
                                                planes),
                                 mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("One node twice in a slot",
                                 this->Generate({ { QmitkMxNSlot::Datasets, { a, a } } }, planes), mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("One node in two slots",
                                 this->Generate({ { QmitkMxNSlot::Anatomy, { a } }, { QmitkMxNSlot::FusedImage, { a } } },
                                                QmitkMxNFusionPairOptions{}),
                                 mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("A node of another data class",
                                 this->Generate({ { QmitkMxNSlot::Datasets, { mask } } }, planes), mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("A slot the template does not have",
                                 this->Generate({ { QmitkMxNSlot::Datasets, { a } }, { QmitkMxNSlot::Segmentations, { mask } } },
                                                planes),
                                 mitk::Exception);
    QmitkMxNOneViewPerDatasetOptions threeD;
    threeD.view = QmitkMxNDatasetView::ThreeD;
    CPPUNIT_ASSERT_THROW_MESSAGE("A 3D view without 3D representations",
                                 this->Generate({ { QmitkMxNSlot::Datasets, { a, b } } }, threeD), mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("An invalid widget name",
                                 QmitkMxNDynamicLayouts::Generate({ { QmitkMxNSlot::Datasets, { a } } }, planes,
                                                                  QStringLiteral("not_valid"), 1.6, *m_DataStorage),
                                 mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("A non-positive aspect ratio",
                                 this->Generate({ { QmitkMxNSlot::Datasets, { a } } }, planes, 0.0), mitk::Exception);
  }

  // --- Applying ----------------------------------------------------------------

  void ShowEditor()
  {
    m_Editor->resize(1200, 800);
    m_Editor->show();
    Pump();
  }

  std::vector<QString> WindowIds() const
  {
    std::vector<QString> ids;
    for (const auto& descriptor : m_Editor->ListWindowDescriptors())
    {
      ids.push_back(descriptor.id);
    }
    return ids;
  }

  void Apply_BindsEveryGroupToExactlyItsNodes()
  {
    const auto base = this->AddImage("base");
    const auto first = this->AddMask(base, "first", 5);
    const auto second = this->AddMask(base, "second", 2);
    const auto other = this->AddImage("other");
    this->ShowEditor();
    const auto layout = this->Generate({ { QmitkMxNSlot::BaseImage, { base } }, { QmitkMxNSlot::Segmentations, { first, second } } },
                                       QmitkMxNSharedBaseOptions{ false, true });
    m_Editor->ApplyDynamicLayout(layout);
    Pump();

    for (const auto& descriptor : m_Editor->ListWindowDescriptors())
    {
      auto* renderer = this->Renderer(descriptor.id);
      const auto& shown = layout.groupNodes.at(descriptor.selectionGroup.toStdString());
      for (const auto& node : { base, first, second, other })
      {
        const bool expected = std::find(shown.begin(), shown.end(), node) != shown.end();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(node->GetName() + " in " + descriptor.id.toStdString(), expected,
                                     node->IsVisible(renderer));
      }
      int previous = std::numeric_limits<int>::min();
      for (const auto& node : shown)
      {
        int layer = 0;
        node->GetIntProperty("layer", layer, renderer);
        CPPUNIT_ASSERT_MESSAGE("Drawn in the given order in " + descriptor.id.toStdString(), layer > previous);
        previous = layer;
      }
    }
  }

  void Apply_WritesRendererStateOnTheNamedWindowsOnly()
  {
    const auto anatomy = this->AddImage("anatomy");
    const auto fused = this->AddImage("fused");
    fused->SetOpacity(1.0f);
    fused->SetLevelWindow(mitk::LevelWindow(100.0, 200.0));
    this->ShowEditor();
    const auto layout = this->Generate({ { QmitkMxNSlot::Anatomy, { anatomy } }, { QmitkMxNSlot::FusedImage, { fused } } },
                                       QmitkMxNFusionPairOptions{});
    m_Editor->ApplyDynamicLayout(layout);
    Pump();

    const auto windows = Windows(layout.document);
    const auto id = [&windows](std::size_t i) { return QString::fromStdString(windows[i].at("id").get<std::string>()); };
    const std::map<std::size_t, mitk::LookupTable::LookupTableType> tables = {
      { 1, mitk::LookupTable::INVERSE_GRAYSCALE }, { 4, mitk::LookupTable::INVERSE_GRAYSCALE },
      { 2, mitk::LookupTable::PET_COLOR },         { 5, mitk::LookupTable::PET_COLOR } };
    for (std::size_t i = 0; i < windows.size(); ++i)
    {
      const std::string where = " in window " + std::to_string(i);
      const bool fusion = 2 == i || 5 == i;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("An own opacity only in the fusion windows" + where, fusion,
                                   HasRendererProperty(*fused, id(i), "opacity"));
      if (fusion)
      {
        float opacity = 0.0f;
        fused->GetOpacity(opacity, this->Renderer(id(i)));
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The chosen opacity" + where, 0.5, opacity, 1e-6);
      }
      const auto table = tables.find(i);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("An own lookup table only in the look groups" + where, table != tables.end(),
                                   HasRendererProperty(*fused, id(i), "LookupTable"));
      if (table != tables.end())
      {
        auto* property = dynamic_cast<mitk::LookupTableProperty*>(
          fused->GetPropertyList(id(i).toStdString())->GetProperty("LookupTable"));
        CPPUNIT_ASSERT_MESSAGE("The chosen table" + where,
                               nullptr != property && table->second == property->GetLookupTable()->GetActiveType());
      }
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Volume rendering only in the MIP window" + where, 6 == i,
                                   HasRendererProperty(*fused, id(i), "volumerendering"));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A transfer function only in the MIP window" + where, 6 == i,
                                   HasRendererProperty(*fused, id(i), "TransferFunction"));
    }

    // Without a transfer function the volume mapper renders nothing; the MIP
    // window gets a white ramp over the fused image's level window.
    auto* mipRenderer = this->Renderer(id(6));
    int blendMode = -1;
    CPPUNIT_ASSERT_MESSAGE("A maximum intensity projection",
                           fused->GetIntProperty("volumerendering.blendmode", blendMode, mipRenderer)
                             && vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND == blendMode);
    auto* transferFunction = dynamic_cast<mitk::TransferFunctionProperty*>(fused->GetProperty("TransferFunction", mipRenderer));
    CPPUNIT_ASSERT_MESSAGE("The MIP window has a transfer function", nullptr != transferFunction);
    auto* opacity = transferFunction->GetValue()->GetScalarOpacityFunction();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Transparent at the lower window bound", 0.0, opacity->GetValue(0.0), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Half opaque at the level", 0.5, opacity->GetValue(100.0), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Opaque at the upper window bound", 1.0, opacity->GetValue(200.0), 1e-6);
    double color[3] = { 0.0, 0.0, 0.0 };
    transferFunction->GetValue()->GetColorTransferFunction()->GetColor(100.0, color);
    CPPUNIT_ASSERT_MESSAGE("White", color[0] > 1.0 - 1e-6 && color[1] > 1.0 - 1e-6 && color[2] > 1.0 - 1e-6);
    CPPUNIT_ASSERT_MESSAGE("No node-global transfer function", nullptr == fused->GetProperty("TransferFunction"));

    float globalOpacity = 0.0f;
    fused->GetOpacity(globalOpacity, nullptr);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The node-global opacity is unchanged", 1.0, globalOpacity, 1e-6);
    bool globalVolumeRendering = false;
    CPPUNIT_ASSERT_MESSAGE("No node-global volume rendering",
                           !fused->GetBoolProperty("volumerendering", globalVolumeRendering) || !globalVolumeRendering);
  }

  void Apply_FitsTheViewsAndKeepsTheTimeStep()
  {
    auto a = mitk::DataNode::New();
    a->SetName("a");
    a->SetData(mitk::ImageGenerator::GenerateRandomImage<unsigned short>(16, 16, 8, 3));
    m_DataStorage->Add(a);
    auto b = mitk::DataNode::New();
    b->SetName("b");
    b->SetData(mitk::ImageGenerator::GenerateRandomImage<unsigned short>(32, 24, 12, 3));
    m_DataStorage->Add(b);
    this->ShowEditor();
    auto* renderingManager = mitk::RenderingManager::GetInstance();
    renderingManager->InitializeViews(a->GetData()->GetTimeGeometry());
    renderingManager->GetTimeNavigationController()->GetStepper()->SetPos(1);

    const auto layout = this->Generate({ { QmitkMxNSlot::Datasets, { a, b } } }, QmitkMxNDatasetsByPlanesOptions{});
    m_Editor->ApplyDynamicLayout(layout);
    Pump();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The time step is kept", 1u,
                                 renderingManager->GetTimeNavigationController()->GetStepper()->GetPos());
    // Each window fits its own dataset, not the union of both: its slices
    // are those of its dataset along its normal, and its plane covers it.
    const std::map<std::string, int> normalAxis = { { "axial", 2 }, { "sagittal", 0 }, { "coronal", 1 } };
    for (const auto& descriptor : m_Editor->ListWindowDescriptors())
    {
      const std::string where = " of " + descriptor.id.toStdString();
      const auto* image = dynamic_cast<const mitk::Image*>(
        layout.groupNodes.at(descriptor.selectionGroup.toStdString()).front()->GetData());
      const int normal = normalAxis.at(descriptor.viewDirection.toStdString());
      auto* renderer = this->Renderer(descriptor.id);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The slices" + where, image->GetDimension(normal),
                                   renderer->GetSliceNavigationController()->GetStepper()->GetSteps());

      const auto data = image->GetTimeGeometry()->GetBoundsInWorld();
      const auto view = renderer->GetWorldTimeGeometry()->GetBoundsInWorld();
      for (int axis = 0; axis < 3; ++axis)
      {
        if (axis != normal)
        {
          CPPUNIT_ASSERT_MESSAGE("The plane covers the data" + where,
                                 view[2 * axis] <= data[2 * axis] + 1e-3 && view[2 * axis + 1] >= data[2 * axis + 1] - 1e-3);
        }
      }
    }
  }

  mitk::DataNode::Pointer AddImage(const std::string& name, unsigned int x, unsigned int y, unsigned int z,
                                   unsigned int t = 1)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    node->SetData(mitk::ImageGenerator::GenerateRandomImage<unsigned short>(x, y, z, t));
    m_DataStorage->Add(node);
    return node;
  }

  void Apply_ObliqueDataset_ShownInItsAcquisitionPlanes()
  {
    // An obliquely acquired image is shown in its own planes, as a reinit
    // with the image does, not resliced along the world axes.
    const auto oblique = this->AddImage("oblique", 16, 16, 8);
    auto* image = dynamic_cast<mitk::Image*>(oblique->GetData());
    auto* geometry = image->GetGeometry();
    mitk::Vector3D axis;
    axis[0] = 1.0;
    axis[1] = 0.0;
    axis[2] = 0.0;
    mitk::RotationOperation rotation(mitk::OpROTATE, geometry->GetCenter(), axis, 30.0);
    geometry->ExecuteOperation(&rotation);
    image->GetTimeGeometry()->UpdateBoundingBox();
    this->ShowEditor();

    m_Editor->ApplyDynamicLayout(
      this->Generate({ { QmitkMxNSlot::Datasets, { oblique } } }, QmitkMxNDatasetsByPlanesOptions{}));
    Pump();

    const auto imageNormal = geometry->GetAxisVector(2);
    for (const auto& descriptor : m_Editor->ListWindowDescriptors())
    {
      if ("axial" != descriptor.viewDirection)
      {
        continue;
      }
      auto* renderer = this->Renderer(descriptor.id);
      const auto planeNormal = renderer->GetCurrentWorldPlaneGeometry()->GetNormal();
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The axial plane is the image's own", 1.0,
                                           std::abs(planeNormal * imageNormal)
                                             / (planeNormal.GetNorm() * imageNormal.GetNorm()),
                                           1e-6);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("...with the image's slices", image->GetDimension(2),
                                   renderer->GetSliceNavigationController()->GetStepper()->GetSteps());
    }
  }

  void Apply_FrameOfReferenceAlignedDatasets_FitTheirJointExtent()
  {
    // Series in one Frame of Reference, e.g. a CT and a PET with a longer
    // scan range: aligned, but of different extents. Every window must reach
    // every dataset's slices, not only the first dataset's.
    const auto shorter = this->AddImage("shorter", 16, 16, 8);
    const auto longer = this->AddImage("longer", 16, 16, 24);
    SetTag(shorter, 0x0020, 0x0052, "1.2.3");
    SetTag(longer, 0x0020, 0x0052, "1.2.3");
    this->ShowEditor();

    const QmitkMxNSlotBindings datasets{ { QmitkMxNSlot::Datasets, { shorter, longer } } };
    CPPUNIT_ASSERT_MESSAGE("Fixture: the defaults call them aligned",
                           std::get<QmitkMxNDatasetsByPlanesOptions>(QmitkMxNDynamicLayouts::DefaultOptions(
                             QmitkMxNDynamicTemplate::DatasetsByPlanes, datasets, *m_DataStorage)).aligned);
    const std::pair<std::string, QmitkMxNDynamicOptions> variants[] = {
      { "datasets x planes", QmitkMxNDatasetsByPlanesOptions{ true } },
      { "one view per dataset", QmitkMxNOneViewPerDatasetOptions{ true } } };
    const double halfSlice = 0.5 + 1e-3;
    for (const auto& [name, options] : variants)
    {
      m_Editor->ApplyDynamicLayout(this->Generate(datasets, options));
      Pump();

      for (const auto& descriptor : m_Editor->ListWindowDescriptors())
      {
        const std::string where = " of " + descriptor.id.toStdString() + " (" + name + ")";
        auto* renderer = this->Renderer(descriptor.id);
        if ("axial" == descriptor.viewDirection)
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Every slice of the longer dataset" + where,
                                       dynamic_cast<const mitk::Image*>(longer->GetData())->GetDimension(2),
                                       renderer->GetSliceNavigationController()->GetStepper()->GetSteps());
        }
        const auto view = renderer->GetWorldTimeGeometry()->GetBoundsInWorld();
        for (const auto& dataset : { shorter, longer })
        {
          const auto data = dataset->GetData()->GetTimeGeometry()->GetBoundsInWorld();
          for (int axis = 0; axis < 3; ++axis)
          {
            // Slices laid over a joint extent sit half a slice off a dataset's
            // own grid, as after a global reinit.
            CPPUNIT_ASSERT_MESSAGE("The view covers " + dataset->GetName() + where,
                                   view[2 * axis] <= data[2 * axis] + halfSlice
                                     && view[2 * axis + 1] >= data[2 * axis + 1] - halfSlice);
          }
        }
      }
    }
  }

  void Apply_MixedTimeData_GlobalTimeCoversAll()
  {
    // A dynamic series beside a static one: the time navigation must still
    // step through the dynamic series, whichever window was fitted last.
    const auto dynamic = this->AddImage("dynamic", 16, 16, 8, 3);
    const auto still = this->AddImage("static", 16, 16, 8);
    this->ShowEditor();

    m_Editor->ApplyDynamicLayout(
      this->Generate({ { QmitkMxNSlot::Datasets, { dynamic, still } } }, QmitkMxNDatasetsByPlanesOptions{}));
    Pump();

    auto* timeNavigation = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The time steps of the dynamic series", mitk::TimeStepType(3),
                                 timeNavigation->GetInputWorldTimeGeometry()->CountTimeSteps());
    timeNavigation->GetStepper()->SetPos(2);
    const auto* dynamicTime = dynamic->GetData()->GetTimeGeometry();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The last step reaches the series' last time step", mitk::TimeStepType(2),
                                 dynamicTime->TimePointToTimeStep(timeNavigation->GetSelectedTimePoint()));
  }

  void Apply_WhileHidden_GlobalTimeStillCoversAllOnceShown()
  {
    // Applied to a hidden display, the windows are fitted to their size once
    // it is shown; that repair of the 3D windows must not narrow the time
    // range the layout set to one window's data.
    const auto dynamic = this->AddImage("dynamic", 16, 16, 8, 3);
    const auto still = this->AddImage("static", 16, 16, 8);
    m_Editor->ApplyDynamicLayout(this->Generate({ { QmitkMxNSlot::Datasets, { dynamic, still } } },
                                                QmitkMxNDatasetsByPlanesOptions{ false, true, false }));
    auto* timeNavigation = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fixture: applied, the time covers the dynamic series", mitk::TimeStepType(3),
                                 timeNavigation->GetInputWorldTimeGeometry()->CountTimeSteps());

    this->ShowEditor();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shown, it still does", mitk::TimeStepType(3),
                                 timeNavigation->GetInputWorldTimeGeometry()->CountTimeSteps());
  }

  /** Every window's selection link 's' rewritten as '{"target": s}'. */
  static void UseObjectFormSelectionLinks(nlohmann::json& node)
  {
    if ("window" == node.at("type").get<std::string>())
    {
      auto& links = node.at("links");
      links["selection"] = nlohmann::json{ { "target", links.at("selection") } };
      return;
    }
    for (auto& child : node.at("children"))
    {
      UseObjectFormSelectionLinks(child);
    }
  }

  void Apply_ObjectFormSelectionLinks_Accepted()
  {
    // The layout format accepts a link as a group name or as an object
    // naming it; a hand-built dynamic layout may use either.
    const auto anatomy = this->AddImage("anatomy");
    const auto fused = this->AddImage("fused");
    auto layout = this->Generate({ { QmitkMxNSlot::Anatomy, { anatomy } }, { QmitkMxNSlot::FusedImage, { fused } } },
                                 QmitkMxNFusionPairOptions{});
    UseObjectFormSelectionLinks(layout.document.at("root"));

    CPPUNIT_ASSERT_NO_THROW(m_Editor->ApplyDynamicLayout(layout));
    std::set<std::string> bound;
    for (const auto& descriptor : m_Editor->ListWindowDescriptors())
    {
      bound.insert(descriptor.selectionGroup.toStdString());
    }
    CPPUNIT_ASSERT_MESSAGE("Every window shows its group", (std::set<std::string>{ "anat", "fused", "fusion" }) == bound);
  }

  void Apply_ValidationFailuresChangeNothing()
  {
    const auto anatomy = this->AddImage("anatomy");
    const auto fused = this->AddImage("fused");
    m_Editor->SetLayout(1, 3);
    const auto before = this->WindowIds();
    const auto valid = this->Generate({ { QmitkMxNSlot::Anatomy, { anatomy } }, { QmitkMxNSlot::FusedImage, { fused } } },
                                      QmitkMxNFusionPairOptions{});
    const auto anatomyWindow = QString::fromStdString(Windows(valid.document)[0].at("id").get<std::string>());

    std::vector<std::pair<std::string, QmitkMxNDynamicLayout>> broken;
    auto layout = valid;
    layout.groupNodes["nowhere"] = { anatomy };
    broken.emplace_back("a group the document does not declare", layout);
    layout = valid;
    layout.lookupTables.push_back({ anatomyWindow, fused, mitk::LookupTable::GRAYSCALE });
    broken.emplace_back("a lookup table for a window without a LUT link", layout);
    layout = valid;
    layout.volumeRenderings.push_back(
      { anatomyWindow, fused, QmitkMxNVolumeBlendMode::MaximumIntensity, mitk::TransferFunction::New() });
    broken.emplace_back("a volume rendering for a 2D window", layout);
    layout = valid;
    layout.volumeRenderings.front().transferFunction = nullptr;
    broken.emplace_back("a volume rendering without a transfer function", layout);
    layout = valid;
    layout.fits.push_back({ QStringLiteral("mxn__widget99"), true });
    broken.emplace_back("a fit of a window the document does not have", layout);
    layout = valid;
    auto stranger = mitk::DataNode::New();
    stranger->SetData(mitk::ImageGenerator::GenerateGradientImage<short>(4, 4, 4));
    layout.groupNodes["anat"] = { stranger };
    broken.emplace_back("a node outside the data storage", layout);
    // The first window of the document is the anatomy window.
    const auto anatomyLinks = [](QmitkMxNDynamicLayout& l) -> nlohmann::json& {
      auto* node = &l.document.at("root");
      while ("window" != node->at("type").get<std::string>())
      {
        node = &node->at("children").at(0);
      }
      return node->at("links");
    };
    layout = valid;
    anatomyLinks(layout)["selection"] = 5;
    broken.emplace_back("a selection link that is a number", layout);
    layout = valid;
    anatomyLinks(layout)["selection"] = nlohmann::json{ { "target", 5 } };
    broken.emplace_back("a selection link whose target is a number", layout);
    layout = valid;
    anatomyLinks(layout) = nlohmann::json::array();
    broken.emplace_back("links that are not an object", layout);

    for (const auto& [what, layout] : broken)
    {
      CPPUNIT_ASSERT_THROW_MESSAGE(what + " is rejected", m_Editor->ApplyDynamicLayout(layout), mitk::Exception);
      CPPUNIT_ASSERT_MESSAGE("...before anything changes: " + what, before == this->WindowIds());
    }
  }

  static nlohmann::json OneWindowDocument()
  {
    return nlohmann::json::parse(R"json({
      "version": "3.0",
      "groups": { "main": { "select_all": true } },
      "root": { "type": "split", "orientation": "horizontal", "children": [
        { "type": "window", "id": "mxn__widget0", "view_direction": "axial", "links": { "selection": "main" } }
      ] }
    })json");
  }

  // --- Per-renderer cleanup ----------------------------------------------------

  void Cleanup_ReusedIdInheritsNoRendererState()
  {
    // A dynamic layout writes opacity and volume rendering for its windows'
    // renderers, and an applied lookup table writes the rendering mode and
    // the interpolation. A later layout that reuses the window id must start
    // from the node-global appearance.
    const auto image = this->AddImage("image");
    m_Editor->SetLayout(1, 2);
    const auto windowId = QStringLiteral("mxn__widget0");
    auto* renderer = this->Renderer(windowId);
    image->SetOpacity(0.3f, renderer);
    image->SetBoolProperty("volumerendering", true, renderer);
    image->SetIntProperty("volumerendering.blendmode", 1, renderer);
    image->SetIntProperty("Image Rendering.Mode", 1, renderer);
    image->SetBoolProperty("reslice interpolation", true, renderer);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the cell has its own opacity", HasRendererProperty(*image, windowId, "opacity"));

    m_Editor->ApplyLayout(OneWindowDocument());
    Pump();

    CPPUNIT_ASSERT_MESSAGE("Precondition: the id is in use again", nullptr != m_Editor->GetRenderWindowWidget(windowId));
    for (const auto* key : { "opacity", "volumerendering", "volumerendering.blendmode", "Image Rendering.Mode",
                             "reslice interpolation" })
    {
      CPPUNIT_ASSERT_MESSAGE(std::string("The reused id inherits no '") + key + "'",
                             !HasRendererProperty(*image, windowId, key));
    }
  }

  void Cleanup_UnlinkKeepsTheCellsOtherRendererState()
  {
    // Unlinking leaves the cell alive; only the unlinked dimension's key goes.
    const auto image = this->AddImage("image");
    m_Editor->SetLayout(1, 2);
    const auto windowId = QStringLiteral("mxn__widget0");
    auto* renderer = this->Renderer(windowId);
    image->SetOpacity(0.3f, renderer);
    m_Editor->SetLevelWindow(windowId, image, mitk::LevelWindow(100.0, 50.0));
    CPPUNIT_ASSERT_MESSAGE("Fixture: the cell has its own level/window",
                           HasRendererProperty(*image, windowId, "levelwindow"));

    m_Editor->ClearSyncLink(windowId, QmitkMxNSyncDimension::Windowing);

    CPPUNIT_ASSERT_MESSAGE("Unlinking removes the level/window",
                           !HasRendererProperty(*image, windowId, "levelwindow"));
    CPPUNIT_ASSERT_MESSAGE("...and keeps the opacity", HasRendererProperty(*image, windowId, "opacity"));
  }

  void Cleanup_PresetAfterADynamicLayoutShowsTheGlobalOpacity()
  {
    const auto anatomy = this->AddImage("anatomy");
    const auto fused = this->AddImage("fused");
    this->ShowEditor();
    m_Editor->ApplyDynamicLayout(this->Generate(
      { { QmitkMxNSlot::Anatomy, { anatomy } }, { QmitkMxNSlot::FusedImage, { fused } } }, QmitkMxNFusionPairOptions{}));

    const auto resource = us::ModuleRegistry::GetModule("MitkQtWidgets")->GetResource("mxnLayout_01_mprAnd3d.json");
    CPPUNIT_ASSERT(resource.IsValid());
    us::ModuleResourceStream stream(resource);
    m_Editor->ApplyLayout(nlohmann::json::parse(stream));
    Pump();

    for (const auto& id : this->WindowIds())
    {
      CPPUNIT_ASSERT_MESSAGE("No window keeps the fusion's opacity: " + id.toStdString(),
                             !HasRendererProperty(*fused, id, "opacity"));
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNDynamicLayout)
