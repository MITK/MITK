/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMxNDynamicLayout.h>

#include <QmitkMxNMultiWidget.h>

#include <mitkBaseGeometry.h>
#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <mitkNodePredicateDataType.h>
#include <mitkPointSet.h>
#include <mitkPropertyNameHelper.h>
#include <mitkSurface.h>

#include <vtkVolumeMapper.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <optional>
#include <set>

namespace
{
  using NodeList = std::vector<mitk::DataNode::Pointer>;
  using Json = nlohmann::json;

  constexpr unsigned int ModalityGroup = 0x0008;
  constexpr unsigned int ModalityElement = 0x0060;
  constexpr unsigned int AcquisitionDateGroup = 0x0008;
  constexpr unsigned int AcquisitionDateElement = 0x0022;
  constexpr unsigned int FrameOfReferenceGroup = 0x0020;
  constexpr unsigned int FrameOfReferenceElement = 0x0052;

  bool HasTrueProperty(const mitk::DataNode& node, const char* key)
  {
    bool value = false;
    return node.GetBoolProperty(key, value) && value;
  }

  bool IsMultiLabelSegmentation(const mitk::DataNode& node)
  {
    // MitkQtWidgets does not depend on MitkMultilabel, hence the class name.
    return mitk::NodePredicateDataType::New("MultiLabelSegmentation")->CheckNode(&node);
  }

  bool IsThreeDRepresentation(const mitk::DataNode& node)
  {
    return IsMultiLabelSegmentation(node) || nullptr != dynamic_cast<const mitk::Surface*>(node.GetData());
  }

  std::optional<std::string> DicomTag(const mitk::DataNode& node, unsigned int group, unsigned int element)
  {
    const auto* data = node.GetData();
    std::string value;
    if (nullptr == data || !mitk::GetDICOMPropertyValue(group, element, data->GetPropertyList(), value))
    {
      return std::nullopt;
    }
    return value;
  }

  NodeList OverlayDerivations(const mitk::DataNode& image, const mitk::DataStorage& dataStorage)
  {
    NodeList overlays;
    // Held: a range-for over the dereferenced temporary iterates a freed set.
    const auto derivations = dataStorage.GetDerivations(&image, nullptr, true);
    for (const auto& derivation : *derivations)
    {
      if (QmitkMxNDynamicLayouts::IsOfDataClass(*derivation, QmitkMxNSlotDataClass::Overlay))
      {
        overlays.push_back(derivation);
      }
    }
    return overlays;
  }

  int NodeLayer(const mitk::DataNode& node)
  {
    int layer = 0;
    node.GetIntProperty("layer", layer);
    return layer;
  }

  /** An image and its overlays, bottom to top: the image, then its overlay
   *  derivations by ascending node-global layer. */
  NodeList DatasetNodes(const mitk::DataNode::Pointer& image, const mitk::DataStorage& dataStorage)
  {
    auto overlays = OverlayDerivations(*image, dataStorage);
    std::stable_sort(overlays.begin(), overlays.end(),
                     [](const auto& a, const auto& b) { return NodeLayer(*a) < NodeLayer(*b); });
    NodeList nodes{ image };
    nodes.insert(nodes.end(), overlays.begin(), overlays.end());
    return nodes;
  }

  /** Identical geometry at time step 0, or one shared Frame of Reference UID
   *  on all. A mismatch only means "not aligned". */
  bool AreAligned(const NodeList& images)
  {
    if (images.empty())
    {
      return false;
    }

    const auto geometryOf = [](const mitk::DataNode::Pointer& image) -> const mitk::BaseGeometry* {
      return nullptr != image->GetData() ? image->GetData()->GetGeometry(0) : nullptr;
    };
    bool sameGeometry = true;
    const auto* first = geometryOf(images.front());
    for (const auto& image : images)
    {
      const auto* geometry = geometryOf(image);
      if (nullptr == first || nullptr == geometry || !mitk::Equal(*first, *geometry, mitk::eps, false))
      {
        sameGeometry = false;
        break;
      }
    }
    if (sameGeometry)
    {
      return true;
    }

    const auto frameOfReference = DicomTag(*images.front(), FrameOfReferenceGroup, FrameOfReferenceElement);
    if (!frameOfReference.has_value())
    {
      return false;
    }
    return std::all_of(images.begin(), images.end(), [&frameOfReference](const auto& image) {
      return DicomTag(*image, FrameOfReferenceGroup, FrameOfReferenceElement) == frameOfReference;
    });
  }

  /** Columns and rows for 'n' equal windows: the shape whose aspect is
   *  closest to the editor's, with a penalty per empty place; never a whole
   *  empty row. On a tie, fewer columns. */
  std::pair<int, int> GridShape(int n, double aspect)
  {
    int bestColumns = 1;
    int bestRows = n;
    double bestScore = std::numeric_limits<double>::max();
    for (int columns = 1; columns <= n; ++columns)
    {
      const int rows = (n + columns - 1) / columns;
      const int empty = rows * columns - n;
      if (empty >= columns)
      {
        continue;
      }
      const double score = std::abs(std::log(aspect * rows / columns)) + 0.35 * empty;
      if (score < bestScore)
      {
        bestScore = score;
        bestColumns = columns;
        bestRows = rows;
      }
    }
    return { bestColumns, bestRows };
  }

  /** Builds the document tree; windows are numbered in the order they are
   *  made, which callers keep equal to the tree's pre-order. */
  class DocumentBuilder
  {
  public:
    explicit DocumentBuilder(const QString& widgetName)
      : m_WidgetName(widgetName.toStdString())
    {
    }

    QString NextId() const
    {
      return QString::fromStdString(m_WidgetName + "__widget" + std::to_string(m_WindowCount));
    }

    Json Window(const char* direction, Json links, const QString& name = QString(), bool threeD = false)
    {
      Json window = { { "type", "window" },
                      { "id", this->NextId().toStdString() },
                      { "view_direction", direction },
                      { "links", std::move(links) } };
      if (!name.isEmpty())
      {
        window["name"] = name.toStdString();
      }
      if (threeD)
      {
        window["render_mode"] = "3d";
      }
      ++m_WindowCount;
      return window;
    }

    static Json Split(const char* orientation, Json children)
    {
      return { { "type", "split" }, { "orientation", orientation }, { "children", std::move(children) } };
    }

    void SelectionGroup(const std::string& id, const std::string& name)
    {
      Json entry = { { "select_all", false } };
      if (!name.empty())
      {
        entry["name"] = name;
      }
      m_Groups[id] = std::move(entry);
    }

    void NavigationGroup(const std::string& id, const std::string& name)
    {
      m_Groups[id] = { { "name", name } };
    }

    Json Document(const QString& name, Json root) const
    {
      return { { "version", "3.0" }, { "name", name.toStdString() }, { "groups", m_Groups }, { "root", std::move(root) } };
    }

  private:
    std::string m_WidgetName;
    int m_WindowCount = 0;
    Json m_Groups = Json::object();
  };

  std::string DatasetGroupId(std::size_t index)
  {
    // Zero-padded, so that the groups' allocation order, and with it their
    // hue order, is the dataset order.
    return (index < 10 ? "ds0" : "ds") + std::to_string(index);
  }

  const NodeList& SlotNodes(const QmitkMxNSlotBindings& bindings, QmitkMxNSlot slot)
  {
    static const NodeList none;
    const auto it = bindings.find(slot);
    return it != bindings.end() ? it->second : none;
  }

  void ValidateBindings(const QmitkMxNTemplateDescription& description, const QmitkMxNSlotBindings& bindings)
  {
    for (const auto& [slot, nodes] : bindings)
    {
      const auto described = std::find_if(description.slotDescriptions.begin(), description.slotDescriptions.end(),
                                          [slot = slot](const auto& s) { return s.slot == slot; });
      if (described == description.slotDescriptions.end())
      {
        mitkThrow() << "Dynamic layout '" << description.name.toStdString()
                    << "': a binding names a slot the template does not have.";
      }
    }
    for (const auto& slot : description.slotDescriptions)
    {
      const auto& nodes = SlotNodes(bindings, slot.slot);
      const int count = static_cast<int>(nodes.size());
      if (count < slot.min || count > slot.max)
      {
        mitkThrow() << "Dynamic layout '" << description.name.toStdString() << "': slot '"
                    << slot.role.toStdString() << "' holds " << count << " nodes, it takes " << slot.min
                    << (slot.min == slot.max ? "" : " to " + std::to_string(slot.max)) << ".";
      }
      for (const auto& node : nodes)
      {
        if (node.IsNull() || !QmitkMxNDynamicLayouts::IsOfDataClass(*node, slot.dataClass))
        {
          mitkThrow() << "Dynamic layout '" << description.name.toStdString() << "': a node in slot '"
                      << slot.role.toStdString() << "' is not of the slot's data class.";
        }
      }
    }
  }

  std::string NodeName(const mitk::DataNode::Pointer& node)
  {
    return node->GetName();
  }

  QmitkMxNDynamicLayout DatasetsByPlanes(const QmitkMxNSlotBindings& bindings,
                                         const QmitkMxNDatasetsByPlanesOptions& options,
                                         const QString& widgetName,
                                         const mitk::DataStorage& dataStorage)
  {
    const auto& datasets = SlotNodes(bindings, QmitkMxNSlot::Datasets);
    QmitkMxNDynamicLayout layout;
    DocumentBuilder builder(widgetName);
    if (options.aligned)
    {
      builder.NavigationGroup("nav", "Navigation");
      builder.NavigationGroup("ax", "Axial windows");
      builder.NavigationGroup("sag", "Sagittal windows");
      builder.NavigationGroup("cor", "Coronal windows");
    }

    const std::pair<const char*, const char*> planes[] = { { "axial", "ax" }, { "sagittal", "sag" },
                                                           { "coronal", "cor" } };
    Json datasetSplits = Json::array();
    for (std::size_t i = 0; i < datasets.size(); ++i)
    {
      const auto group = DatasetGroupId(i);
      builder.SelectionGroup(group, NodeName(datasets[i]));
      layout.groupNodes[group] = DatasetNodes(datasets[i], dataStorage);

      Json windows = Json::array();
      for (const auto& [direction, plane] : planes)
      {
        // Every member of the dataset's windowing and LUT group shows the
        // same image, so the group has something to act on.
        Json links = { { "selection", group },
                       { "crosshair", options.aligned ? "nav" : group },
                       { "windowing", group },
                       { "lut", group } };
        if (options.aligned)
        {
          links["slice"] = plane;
          links["zoom"] = plane;
          links["pan"] = plane;
        }
        // Aligned: one fit per plane group, from its first window. Otherwise
        // every window is a component of its own and fits itself.
        if (!options.aligned || 0 == i)
        {
          layout.fits.push_back({ builder.NextId(), options.aligned });
        }
        windows.push_back(builder.Window(direction, std::move(links)));
      }
      if (options.threeDColumn)
      {
        layout.fits.push_back({ builder.NextId(), false });
        windows.push_back(builder.Window("coronal", { { "selection", group } }, QString(), true));
      }
      datasetSplits.push_back(DocumentBuilder::Split(options.datasetsAsColumns ? "vertical" : "horizontal",
                                                     std::move(windows)));
    }

    layout.document = builder.Document(QmitkMxNDynamicLayouts::Template(QmitkMxNDynamicTemplate::DatasetsByPlanes).name,
                                       DocumentBuilder::Split(options.datasetsAsColumns ? "horizontal" : "vertical",
                                                              std::move(datasetSplits)));
    return layout;
  }

  /** The datasets in display order: by acquisition date when asked, stable,
   *  so datasets of one day keep the selection order; undated ones last. */
  NodeList OrderedDatasets(const NodeList& datasets, QmitkMxNDatasetOrder order)
  {
    NodeList ordered = datasets;
    if (QmitkMxNDatasetOrder::AcquisitionDate == order)
    {
      std::stable_sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) {
        const auto dateA = DicomTag(*a, AcquisitionDateGroup, AcquisitionDateElement);
        const auto dateB = DicomTag(*b, AcquisitionDateGroup, AcquisitionDateElement);
        if (dateA.has_value() != dateB.has_value())
        {
          return dateA.has_value();
        }
        return dateA.has_value() && *dateA < *dateB;
      });
    }
    return ordered;
  }

  QmitkMxNDynamicLayout OneViewPerDataset(const QmitkMxNSlotBindings& bindings,
                                          const QmitkMxNOneViewPerDatasetOptions& options,
                                          const QString& widgetName,
                                          double editorAspectRatio,
                                          const mitk::DataStorage& dataStorage)
  {
    const auto datasets = OrderedDatasets(SlotNodes(bindings, QmitkMxNSlot::Datasets), options.order);
    const bool threeD = QmitkMxNDatasetView::ThreeD == options.view;
    if (threeD && !std::all_of(datasets.begin(), datasets.end(), [&dataStorage](const auto& dataset) {
          return QmitkMxNDynamicLayouts::HasThreeDRepresentation(*dataset, dataStorage);
        }))
    {
      mitkThrow() << "Dynamic layout 'One view per dataset': the 3D view needs a 3D representation "
                  << "(a segmentation or a surface) for every dataset.";
    }

    QmitkMxNDynamicLayout layout;
    DocumentBuilder builder(widgetName);
    if (threeD)
    {
      builder.NavigationGroup("spin", "Turn together");
    }
    else if (options.aligned)
    {
      builder.NavigationGroup("nav", "Navigation");
    }

    const char* direction = "axial";
    switch (options.view)
    {
      case QmitkMxNDatasetView::Sagittal: direction = "sagittal"; break;
      case QmitkMxNDatasetView::Coronal:  direction = "coronal"; break;
      case QmitkMxNDatasetView::ThreeD:   direction = "free"; break;
      default: break;
    }

    const int n = static_cast<int>(datasets.size());
    const auto [columns, rows] = GridShape(n, editorAspectRatio);
    Json rowSplits = Json::array();
    std::size_t index = 0;
    for (int row = 0; row < rows; ++row)
    {
      const int count = row < rows - 1 ? columns : n - columns * (rows - 1);
      Json windows = Json::array();
      for (int column = 0; column < count; ++column, ++index)
      {
        const auto group = DatasetGroupId(index);
        builder.SelectionGroup(group, NodeName(datasets[index]));
        layout.groupNodes[group] = DatasetNodes(datasets[index], dataStorage);

        Json links = { { "selection", group } };
        if (threeD)
        {
          links["orientation"] = "spin";
          links["zoom"] = "spin";
          // Panned together, windows that do not share a world would
          // push each other's subject out of view.
          if (options.aligned)
          {
            links["pan"] = "spin";
          }
        }
        else if (options.aligned)
        {
          links["pan"] = "nav";
          links["zoom"] = "nav";
          links["slice"] = "nav";
          links["crosshair"] = "nav";
        }
        // Aligned datasets share one world: one fit from the first window.
        // Fitting a component to the first one's bounds would send the other
        // subjects of unaligned datasets out of view, so each fits itself.
        if (!options.aligned || 0 == index)
        {
          layout.fits.push_back({ builder.NextId(), options.aligned });
        }
        windows.push_back(builder.Window(direction, std::move(links), QString(), threeD));
      }
      rowSplits.push_back(DocumentBuilder::Split("horizontal", std::move(windows)));
    }

    layout.document = builder.Document(QmitkMxNDynamicLayouts::Template(QmitkMxNDynamicTemplate::OneViewPerDataset).name,
                                       DocumentBuilder::Split("vertical", std::move(rowSplits)));
    return layout;
  }

  QmitkMxNDynamicLayout SharedBase(const QmitkMxNSlotBindings& bindings,
                                   const QmitkMxNSharedBaseOptions& options,
                                   const QString& widgetName)
  {
    const auto& base = SlotNodes(bindings, QmitkMxNSlot::BaseImage).front();
    const auto& overlays = SlotNodes(bindings, QmitkMxNSlot::Overlays);
    const QString allOverlaysName = QStringLiteral("All overlays");

    QmitkMxNDynamicLayout layout;
    DocumentBuilder builder(widgetName);
    builder.NavigationGroup("nav", "Navigation");
    builder.NavigationGroup("look", "Base look");
    if (options.threeDRow)
    {
      builder.NavigationGroup("spin", "Turn together");
    }

    // The columns: one per overlay, then the one with all of them.
    std::vector<std::pair<std::string, QString>> columns;
    for (std::size_t i = 0; i < overlays.size(); ++i)
    {
      const auto group = "ov" + std::to_string(i);
      builder.SelectionGroup(group, NodeName(overlays[i]));
      layout.groupNodes[group] = { base, overlays[i] };
      columns.emplace_back(group, QString());
    }
    if (options.allOverlaysWindow)
    {
      builder.SelectionGroup("all", allOverlaysName.toStdString());
      NodeList all{ base };
      all.insert(all.end(), overlays.begin(), overlays.end());
      layout.groupNodes["all"] = all;
      columns.emplace_back("all", allOverlaysName);
    }

    Json twoD = Json::array();
    for (const auto& [group, name] : columns)
    {
      // Every window shows the same base image, which the look group acts on.
      if (options.allOverlaysWindow ? "all" == group : twoD.empty())
      {
        layout.fits.push_back({ builder.NextId(), true });
      }
      twoD.push_back(builder.Window("axial",
                                    { { "selection", group }, { "crosshair", "nav" }, { "slice", "nav" },
                                      { "zoom", "nav" }, { "pan", "nav" }, { "windowing", "look" },
                                      { "lut", "look" } },
                                    name));
    }
    Json root = DocumentBuilder::Split("horizontal", std::move(twoD));

    if (options.threeDRow)
    {
      Json threeD = Json::array();
      for (const auto& [group, name] : columns)
      {
        if (threeD.empty())
        {
          layout.fits.push_back({ builder.NextId(), true });
        }
        threeD.push_back(builder.Window("free",
                                        { { "selection", group }, { "orientation", "spin" }, { "zoom", "spin" },
                                          { "pan", "spin" } },
                                        name, true));
      }
      root = DocumentBuilder::Split("vertical", Json::array({ std::move(root),
                                                              DocumentBuilder::Split("horizontal", std::move(threeD)) }));
    }

    layout.document = builder.Document(QmitkMxNDynamicLayouts::Template(QmitkMxNDynamicTemplate::SharedBase).name,
                                       std::move(root));
    return layout;
  }

  mitk::LookupTable::LookupTableType LookupTableTypeOf(QmitkMxNFusionColormap colormap)
  {
    switch (colormap)
    {
      case QmitkMxNFusionColormap::HotIron:             return mitk::LookupTable::HOT_IRON;
      case QmitkMxNFusionColormap::JetTransparentFloor: return mitk::LookupTable::JET_TRANSPARENT;
      default:                                          return mitk::LookupTable::PET_COLOR;
    }
  }

  float OpacityOf(QmitkMxNFusionOpacity opacity)
  {
    switch (opacity)
    {
      case QmitkMxNFusionOpacity::Percent30: return 0.3f;
      case QmitkMxNFusionOpacity::Percent70: return 0.7f;
      default:                               return 0.5f;
    }
  }

  QmitkMxNDynamicLayout FusionPair(const QmitkMxNSlotBindings& bindings,
                                   const QmitkMxNFusionPairOptions& options,
                                   const QString& widgetName)
  {
    const auto& anatomy = SlotNodes(bindings, QmitkMxNSlot::Anatomy).front();
    const auto& fused = SlotNodes(bindings, QmitkMxNSlot::FusedImage).front();
    // The fused column shows the PET in gray beside its colored fusion; any
    // other colormap is left to the fused image's own colormap there.
    const bool grayFusedColumn = QmitkMxNFusionColormap::PetColour == options.colormap;

    QmitkMxNDynamicLayout layout;
    DocumentBuilder builder(widgetName);
    builder.SelectionGroup("anat", NodeName(anatomy));
    builder.SelectionGroup("fused", NodeName(fused));
    builder.SelectionGroup("fusion", "Fusion");
    builder.NavigationGroup("nav", "Navigation");
    builder.NavigationGroup("ax", "Axial windows");
    builder.NavigationGroup("cor", "Coronal windows");
    builder.NavigationGroup("fusionlook", "Fusion look");
    if (grayFusedColumn)
    {
      builder.NavigationGroup("fusedlook", "Fused image look");
    }
    layout.groupNodes["anat"] = { anatomy };
    layout.groupNodes["fused"] = { fused };
    layout.groupNodes["fusion"] = { anatomy, fused };

    // No windowing link anywhere: both images keep their node-global
    // level/window, so the anatomy under the fusion looks as it does alone.
    struct Row
    {
      const char* direction;
      const char* plane;
      bool assignsLookupTables;  // a table reaches every member of its group
    };
    Json rows = Json::array();
    for (const auto& [direction, plane, assignsLookupTables] : { Row{ "axial", "ax", true }, Row{ "coronal", "cor", false } })
    {
      const auto navigation = [plane = plane](const char* selection) {
        return Json{ { "selection", selection }, { "crosshair", "nav" }, { "slice", plane },
                     { "zoom", plane }, { "pan", plane } };
      };
      Json windows = Json::array();
      windows.push_back(builder.Window(direction, navigation("anat"), QStringLiteral("Anatomy")));

      auto fusedLinks = navigation("fused");
      if (grayFusedColumn)
      {
        fusedLinks["lut"] = "fusedlook";
        if (assignsLookupTables)
        {
          layout.lookupTables.push_back({ builder.NextId(), fused, mitk::LookupTable::INVERSE_GRAYSCALE });
        }
      }
      windows.push_back(builder.Window(direction, std::move(fusedLinks), QStringLiteral("Fused image")));

      auto fusionLinks = navigation("fusion");
      fusionLinks["lut"] = "fusionlook";
      const auto fusionId = builder.NextId();
      if (assignsLookupTables)
      {
        layout.lookupTables.push_back({ fusionId, fused, LookupTableTypeOf(options.colormap) });
      }
      layout.opacities.push_back({ fusionId, fused, OpacityOf(options.opacity) });
      // Only the fusion window sees both images, so it fits its row.
      layout.fits.push_back({ fusionId, true });
      windows.push_back(builder.Window(direction, std::move(fusionLinks), QStringLiteral("Fusion")));

      rows.push_back(DocumentBuilder::Split("horizontal", std::move(windows)));
    }

    Json root = DocumentBuilder::Split("vertical", std::move(rows));
    if (options.mipWindow)
    {
      root["size"] = 3;
      const auto mipId = builder.NextId();
      auto mip = builder.Window("coronal", { { "selection", "fused" } }, QStringLiteral("MIP"), true);
      mip["size"] = 1;
      layout.volumeRenderings.push_back({ mipId, fused, vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND });
      layout.fits.push_back({ mipId, false });
      root = DocumentBuilder::Split("horizontal", Json::array({ std::move(root), std::move(mip) }));
    }

    layout.document = builder.Document(QmitkMxNDynamicLayouts::Template(QmitkMxNDynamicTemplate::FusionPair).name,
                                       std::move(root));
    return layout;
  }
}

const std::vector<QmitkMxNTemplateDescription>& QmitkMxNDynamicLayouts::Templates()
{
  using DataClass = QmitkMxNSlotDataClass;
  static const std::vector<QmitkMxNTemplateDescription> templates = {
    { QmitkMxNDynamicTemplate::DatasetsByPlanes, QStringLiteral("Datasets x planes"),
      { { QmitkMxNSlot::Datasets, QStringLiteral("Datasets"), DataClass::Image, 1, 4 } }, false },
    { QmitkMxNDynamicTemplate::OneViewPerDataset, QStringLiteral("One view per dataset"),
      { { QmitkMxNSlot::Datasets, QStringLiteral("Datasets"), DataClass::Image, 2, 16 } }, false },
    { QmitkMxNDynamicTemplate::SharedBase, QStringLiteral("Shared base, varying overlay"),
      { { QmitkMxNSlot::BaseImage, QStringLiteral("Base image"), DataClass::Image, 1, 1 },
        { QmitkMxNSlot::Overlays, QStringLiteral("Overlays"), DataClass::Overlay, 1, 4 } }, false },
    { QmitkMxNDynamicTemplate::FusionPair, QStringLiteral("Fusion pair"),
      { { QmitkMxNSlot::Anatomy, QStringLiteral("Anatomy"), DataClass::Image, 1, 1 },
        { QmitkMxNSlot::FusedImage, QStringLiteral("Fused image"), DataClass::Image, 1, 1 } }, true },
  };
  return templates;
}

const QmitkMxNTemplateDescription& QmitkMxNDynamicLayouts::Template(QmitkMxNDynamicTemplate templateId)
{
  const auto& templates = Templates();
  return *std::find_if(templates.begin(), templates.end(),
                       [templateId](const auto& description) { return description.id == templateId; });
}

bool QmitkMxNDynamicLayouts::IsOfDataClass(const mitk::DataNode& node, QmitkMxNSlotDataClass dataClass)
{
  if (nullptr == node.GetData() || HasTrueProperty(node, "helper object") || HasTrueProperty(node, "hidden object"))
  {
    return false;
  }
  const bool isImage = nullptr != dynamic_cast<const mitk::Image*>(node.GetData());
  const bool binary = HasTrueProperty(node, "binary");
  if (QmitkMxNSlotDataClass::Image == dataClass)
  {
    return isImage && !binary;
  }
  return IsMultiLabelSegmentation(node) || nullptr != dynamic_cast<const mitk::Surface*>(node.GetData())
         || nullptr != dynamic_cast<const mitk::PointSet*>(node.GetData()) || (isImage && binary);
}

bool QmitkMxNDynamicLayouts::HasThreeDRepresentation(const mitk::DataNode& image, const mitk::DataStorage& dataStorage)
{
  const auto overlays = OverlayDerivations(image, dataStorage);
  return std::any_of(overlays.begin(), overlays.end(), [](const auto& overlay) { return IsThreeDRepresentation(*overlay); });
}

QmitkMxNSlotBindings QmitkMxNDynamicLayouts::Prefill(QmitkMxNDynamicTemplate templateId,
                                                     const std::vector<mitk::DataNode::Pointer>& selection,
                                                     const mitk::DataStorage& dataStorage)
{
  NodeList images;
  NodeList overlays;
  for (const auto& node : selection)
  {
    if (node.IsNull())
    {
      continue;
    }
    if (IsOfDataClass(*node, QmitkMxNSlotDataClass::Image))
    {
      images.push_back(node);
    }
    else if (IsOfDataClass(*node, QmitkMxNSlotDataClass::Overlay))
    {
      overlays.push_back(node);
    }
  }

  const auto& description = Template(templateId);
  QmitkMxNSlotBindings bindings;
  const auto take = [&bindings](QmitkMxNSlot slot, const NodeList& nodes, int max) {
    if (!nodes.empty())
    {
      bindings[slot] = NodeList(nodes.begin(), nodes.begin() + std::min<std::size_t>(nodes.size(), max));
    }
  };

  switch (templateId)
  {
    case QmitkMxNDynamicTemplate::DatasetsByPlanes:
    case QmitkMxNDynamicTemplate::OneViewPerDataset:
      take(QmitkMxNSlot::Datasets, images, description.slotDescriptions.front().max);
      break;

    case QmitkMxNDynamicTemplate::SharedBase:
    {
      mitk::DataNode::Pointer base = images.empty() ? nullptr : images.front();
      if (base.IsNull() && !overlays.empty())
      {
        // The one direct source all selected overlays share.
        std::optional<std::set<mitk::DataNode*>> shared;
        for (const auto& overlay : overlays)
        {
          std::set<mitk::DataNode*> sources;
          const auto overlaySources = dataStorage.GetSources(overlay, nullptr, true);
          for (const auto& source : *overlaySources)
          {
            sources.insert(source.GetPointer());
          }
          if (!shared.has_value())
          {
            shared = sources;
            continue;
          }
          std::set<mitk::DataNode*> common;
          std::set_intersection(shared->begin(), shared->end(), sources.begin(), sources.end(),
                                std::inserter(common, common.begin()));
          shared = common;
        }
        if (shared.has_value() && 1 == shared->size() && IsOfDataClass(**shared->begin(), QmitkMxNSlotDataClass::Image))
        {
          base = *shared->begin();
        }
      }
      if (base.IsNotNull())
      {
        bindings[QmitkMxNSlot::BaseImage] = { base };
        if (overlays.empty())
        {
          overlays = OverlayDerivations(*base, dataStorage);
        }
      }
      take(QmitkMxNSlot::Overlays, overlays, description.slotDescriptions.back().max);
      break;
    }

    case QmitkMxNDynamicTemplate::FusionPair:
    {
      NodeList cts;
      NodeList pts;
      for (const auto& image : images)
      {
        const auto modality = DicomTag(*image, ModalityGroup, ModalityElement);
        if (modality == "CT")
        {
          cts.push_back(image);
        }
        else if (modality == "PT")
        {
          pts.push_back(image);
        }
      }
      if (1 == cts.size() && 1 == pts.size())
      {
        bindings[QmitkMxNSlot::Anatomy] = cts;
        bindings[QmitkMxNSlot::FusedImage] = pts;
      }
      else
      {
        take(QmitkMxNSlot::Anatomy, images, 1);
        if (images.size() > 1)
        {
          bindings[QmitkMxNSlot::FusedImage] = { images[1] };
        }
      }
      break;
    }
  }
  return bindings;
}

QmitkMxNDynamicOptions QmitkMxNDynamicLayouts::DefaultOptions(QmitkMxNDynamicTemplate templateId,
                                                              const QmitkMxNSlotBindings& bindings,
                                                              const mitk::DataStorage& dataStorage)
{
  const auto hasThreeD = [&dataStorage](const NodeList& images) {
    return std::any_of(images.begin(), images.end(),
                       [&dataStorage](const auto& image) { return HasThreeDRepresentation(*image, dataStorage); });
  };

  switch (templateId)
  {
    case QmitkMxNDynamicTemplate::DatasetsByPlanes:
    {
      const auto& datasets = SlotNodes(bindings, QmitkMxNSlot::Datasets);
      QmitkMxNDatasetsByPlanesOptions options;
      options.aligned = AreAligned(datasets);
      options.threeDColumn = hasThreeD(datasets);
      return options;
    }
    case QmitkMxNDynamicTemplate::OneViewPerDataset:
    {
      const auto& datasets = SlotNodes(bindings, QmitkMxNSlot::Datasets);
      QmitkMxNOneViewPerDatasetOptions options;
      options.aligned = AreAligned(datasets);
      const bool allDated = !datasets.empty() && std::all_of(datasets.begin(), datasets.end(), [](const auto& dataset) {
        return DicomTag(*dataset, AcquisitionDateGroup, AcquisitionDateElement).has_value();
      });
      options.order = allDated ? QmitkMxNDatasetOrder::AcquisitionDate : QmitkMxNDatasetOrder::Selection;
      return options;
    }
    case QmitkMxNDynamicTemplate::SharedBase:
    {
      const auto& overlays = SlotNodes(bindings, QmitkMxNSlot::Overlays);
      QmitkMxNSharedBaseOptions options;
      options.threeDRow = std::any_of(overlays.begin(), overlays.end(),
                                      [](const auto& overlay) { return IsThreeDRepresentation(*overlay); });
      return options;
    }
    case QmitkMxNDynamicTemplate::FusionPair:
    default:
    {
      QmitkMxNFusionPairOptions options;
      const auto& fused = SlotNodes(bindings, QmitkMxNSlot::FusedImage);
      if (!fused.empty())
      {
        const auto modality = DicomTag(*fused.front(), ModalityGroup, ModalityElement);
        if (modality.has_value() && "PT" != *modality)
        {
          options.colormap = QmitkMxNFusionColormap::HotIron;
          options.mipWindow = false;
        }
      }
      return options;
    }
  }
}

QmitkMxNDynamicTemplate QmitkMxNDynamicLayouts::TemplateOf(const QmitkMxNDynamicOptions& options)
{
  // The alternatives are in the order of the template enumerators.
  return static_cast<QmitkMxNDynamicTemplate>(options.index());
}

QmitkMxNDynamicLayout QmitkMxNDynamicLayouts::Generate(const QmitkMxNSlotBindings& bindings,
                                                       const QmitkMxNDynamicOptions& options,
                                                       const QString& widgetName,
                                                       double editorAspectRatio,
                                                       const mitk::DataStorage& dataStorage)
{
  if (!QmitkMxNIsValidMultiWidgetName(widgetName))
  {
    mitkThrow() << "Dynamic layout: '" << widgetName.toStdString() << "' is not a valid MxN widget name.";
  }
  if (!std::isfinite(editorAspectRatio) || editorAspectRatio <= 0.0)
  {
    mitkThrow() << "Dynamic layout: the editor aspect ratio must be positive, not " << editorAspectRatio << ".";
  }
  ValidateBindings(Template(TemplateOf(options)), bindings);

  switch (TemplateOf(options))
  {
    case QmitkMxNDynamicTemplate::DatasetsByPlanes:
      return DatasetsByPlanes(bindings, std::get<QmitkMxNDatasetsByPlanesOptions>(options), widgetName, dataStorage);
    case QmitkMxNDynamicTemplate::OneViewPerDataset:
      return OneViewPerDataset(bindings, std::get<QmitkMxNOneViewPerDatasetOptions>(options), widgetName,
                               editorAspectRatio, dataStorage);
    case QmitkMxNDynamicTemplate::SharedBase:
      return SharedBase(bindings, std::get<QmitkMxNSharedBaseOptions>(options), widgetName);
    case QmitkMxNDynamicTemplate::FusionPair:
    default:
      return FusionPair(bindings, std::get<QmitkMxNFusionPairOptions>(options), widgetName);
  }
}
