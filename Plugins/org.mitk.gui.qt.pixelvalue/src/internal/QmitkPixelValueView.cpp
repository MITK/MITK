/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPixelValueView.h"
#include <QmitkIconTheme.h>

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkImage.h>
#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkLabelSetImage.h>
#include <mitkLookupTableProperty.h>
#include <mitkRenderingModeProperty.h>
#include <mitkTransferFunctionProperty.h>

#include <itkRGBAPixel.h>
#include <itkRGBPixel.h>

#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

#include <QClipboard>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <QTreeWidgetItem>

#include <ui_QmitkPixelValueView.h>

#include <algorithm>
#include <optional>
#include <regex>

namespace
{
  void CopyCoordsToClipboard(const QLineEdit* x, const QLineEdit* y, const QLineEdit* z)
  {
    auto text = QStringList() << x->text() << y->text();

    if (const auto zText = z->text(); !zText.isEmpty())
      text << zText;

    QApplication::clipboard()->setText(text.join(", "));
  }

  std::vector<const mitk::DataNode*> GetVisibleNodesAt(const mitk::DataStorage* dataStorage, const mitk::Point3D& position, mitk::TimePointType timePoint)
  {
    const auto predicate = mitk::NodePredicateOr::New(
      mitk::TNodePredicateDataType<mitk::Image>::New(),
      mitk::TNodePredicateDataType<mitk::MultiLabelSegmentation>::New());

    const auto candidates = dataStorage->GetSubset(predicate);
    std::vector<std::pair<int, const mitk::DataNode*>> layeredNodes;

    for (const auto& node : *candidates)
    {
      bool isHelperObject = false;
      node->GetBoolProperty("helper object", isHelperObject);

      if (isHelperObject || !node->IsVisible(nullptr))
        continue;

      int layer = 0;

      if (!node->GetIntProperty("layer", layer))
        continue;

      const auto* timeGeometry = node->GetData()->GetTimeGeometry();

      if (nullptr == timeGeometry || !timeGeometry->IsValidTimePoint(timePoint))
        continue;

      if (!timeGeometry->GetGeometryForTimePoint(timePoint)->IsInside(position))
        continue;

      layeredNodes.emplace_back(layer, node);
    }

    // Stable, so that among equal layers the first node wins, like in mitk::FindTopmostVisibleNode().
    std::stable_sort(layeredNodes.begin(), layeredNodes.end(), [](const auto& lhs, const auto& rhs) {
      return lhs.first > rhs.first;
    });

    std::vector<const mitk::DataNode*> nodes;
    nodes.reserve(layeredNodes.size());

    for (const auto& [layer, node] : layeredNodes)
      nodes.push_back(node);

    return nodes;
  }

  bool IsInteger(const mitk::PixelType& pixelType)
  {
    const auto componentType = pixelType.GetComponentType();
    return componentType != itk::IOComponentEnum::FLOAT && componentType != itk::IOComponentEnum::DOUBLE;
  }

  QString FormatValue(mitk::ScalarType value, bool isInteger)
  {
    if (isInteger)
      return QString::number(static_cast<qlonglong>(value));

    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream.precision(2);

    if (std::fabs(value) > 1000000 || std::fabs(value) < 0.01)
    {
      stream << std::scientific;
    }
    else
    {
      stream << std::fixed;
    }

    stream << value;

    return QString::fromStdString(stream.str());
  }

  /** Mirrors the color mapping of mitk::ImageVtkMapper2D::ApplyRenderingMode() for a non-binary scalar value. */
  std::optional<QColor> GetDisplayedColor(const mitk::DataNode* node, mitk::ScalarType value)
  {
    int renderingMode = mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR;

    if (auto* renderingModeProperty = dynamic_cast<mitk::RenderingModeProperty*>(node->GetProperty("Image Rendering.Mode")); nullptr != renderingModeProperty)
      renderingMode = renderingModeProperty->GetRenderingMode();

    double rgba[4] = { 0.0, 0.0, 0.0, 1.0 };

    switch (renderingMode)
    {
      case mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR:
      case mitk::RenderingModeProperty::LOOKUPTABLE_COLOR:
      {
        const auto* lookupTableProperty = dynamic_cast<const mitk::LookupTableProperty*>(node->GetProperty("LookupTable"));

        if (nullptr == lookupTableProperty)
          return std::nullopt;

        // The renderer sets the level window range on the shared table, so work on a copy.
        auto lookupTable = vtkSmartPointer<vtkLookupTable>::New();
        lookupTable->DeepCopy(lookupTableProperty->GetValue()->GetVtkLookupTable());

        if (mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR == renderingMode)
        {
          mitk::LevelWindow levelWindow;

          if (!node->GetLevelWindow(levelWindow))
            return std::nullopt;

          lookupTable->SetRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
        }

        const auto* mappedColor = lookupTable->MapValue(value);

        for (int i = 0; i < 4; ++i)
          rgba[i] = mappedColor[i] / 255.0;

        break;
      }

      case mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR:
      case mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR:
      {
        const auto* transferFunctionProperty = dynamic_cast<const mitk::TransferFunctionProperty*>(node->GetProperty("Image Rendering.Transfer Function"));

        if (nullptr == transferFunctionProperty)
          return std::nullopt;

        const auto transferFunction = transferFunctionProperty->GetValue();
        transferFunction->GetColorTransferFunction()->GetColor(value, rgba);

        if (auto* opacityFunction = transferFunction->GetScalarOpacityFunction(); nullptr != opacityFunction)
          rgba[3] = opacityFunction->GetValue(value);

        break;
      }

      default:
        return std::nullopt;
    }

    float nodeColor[3] = { 1.0f, 1.0f, 1.0f };
    node->GetColor(nodeColor);

    return QColor::fromRgbF(
      static_cast<float>(rgba[0] * nodeColor[0]),
      static_cast<float>(rgba[1] * nodeColor[1]),
      static_cast<float>(rgba[2] * nodeColor[2]),
      static_cast<float>(rgba[3]));
  }

  QIcon CreateSwatchIcon(const QColor& color)
  {
    constexpr int size = 16;
    constexpr int cellSize = 4;

    // A dark background resembles the typical render window, so blended colors look as they do on screen.
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::black);

    QPainter painter(&pixmap);

    for (int y = 0; y < size; y += cellSize)
    {
      for (int x = 0; x < size; x += cellSize)
      {
        if ((x / cellSize + y / cellSize) % 2 != 0)
          painter.fillRect(x, y, cellSize, cellSize, QColor(64, 64, 64));
      }
    }

    painter.fillRect(pixmap.rect(), color);
    painter.setPen(Qt::gray);
    painter.drawRect(0, 0, size - 1, size - 1);
    painter.end();

    return QIcon(pixmap);
  }

  void AddRow(QTreeWidget* treeWidget, const mitk::DataNode* node, const QString& name, const QString& value, std::optional<QColor> color)
  {
    auto* item = new QTreeWidgetItem(treeWidget, { QString(), name, value });

    if (!color.has_value())
      return;

    // Like the renderer, treat a missing opacity property as fully opaque.
    float opacity = 1.0f;
    node->GetOpacity(opacity, nullptr);
    color->setAlphaF(color->alphaF() * opacity);

    item->setIcon(0, CreateSwatchIcon(color.value()));
  }

  mitk::ScalarType ReadPixelComponent(const mitk::Image* image, const mitk::ImageDataItem* volume, const itk::Index<3>& index, int component)
  {
    mitk::ScalarType value = 0.0;

    mitkPixelTypeMultiplex5(
      mitk::FastSinglePixelAccess,
      image->GetChannelDescriptor().GetPixelType(),
      mitk::Image::ConstPointer(image),
      volume,
      index,
      value,
      component);

    return value;
  }

  struct CompositePixel
  {
    QString text;
    std::optional<QColor> color;
  };

  /** Reads an RGB(A) pixel of one time step, e.g. "(255, 128, 0)". Only 8-bit components are supported. */
  CompositePixel ReadCompositePixel(const mitk::Image* image, const mitk::ImageDataItem* volume, const itk::Index<3>& index)
  {
    const auto pixelType = image->GetChannelDescriptor().GetPixelType();

    // Composite pixels cannot be read component-wise like vector pixels; MITK supports them with 8 bits only.
    if (itk::IOComponentEnum::UCHAR != pixelType.GetComponentType())
      return { QStringLiteral("Unsupported RGB type"), std::nullopt };

    const bool hasAlpha = itk::IOPixelEnum::RGBA == pixelType.GetPixelType();
    QColor color;

    if (hasAlpha)
    {
      mitk::ImagePixelReadAccessor<itk::RGBAPixel<unsigned char>, 3> accessor(image, volume, mitk::ImageAccessorBase::IgnoreLock);
      const auto& pixel = accessor.GetPixelByIndex(index);
      color.setRgb(pixel.GetRed(), pixel.GetGreen(), pixel.GetBlue(), pixel.GetAlpha());
    }
    else
    {
      mitk::ImagePixelReadAccessor<itk::RGBPixel<unsigned char>, 3> accessor(image, volume, mitk::ImageAccessorBase::IgnoreLock);
      const auto& pixel = accessor.GetPixelByIndex(index);
      color.setRgb(pixel.GetRed(), pixel.GetGreen(), pixel.GetBlue());
    }

    QStringList components = { QString::number(color.red()), QString::number(color.green()), QString::number(color.blue()) };

    if (hasAlpha)
      components << QString::number(color.alpha());

    return { QStringLiteral("(%1)").arg(components.join(", ")), color };
  }

  /** Like mitk::ImageVtkMapper2D, honors "Image.Displayed Component" only for images with more than one component.
   *  \return The displayed component, or nothing if the image has a single component or the property is not set.
   *          Callers then read component 0.
   *  \throw mitk::Exception if the property names a component the image does not have.
   */
  std::optional<int> GetDisplayedComponent(const mitk::DataNode* node, const mitk::Image* image)
  {
    const auto numberOfComponents = image->GetChannelDescriptor().GetPixelType().GetNumberOfComponents();
    int component = 0;

    if (numberOfComponents <= 1 || !node->GetIntProperty("Image.Displayed Component", component))
      return std::nullopt;

    if (component < 0 || static_cast<std::size_t>(component) >= numberOfComponents)
      mitkThrow() << "Displayed component " << component << " is out of range [0, " << numberOfComponents - 1 << "].";

    return component;
  }
}

const std::string QmitkPixelValueView::VIEW_ID = "org.mitk.views.pixelvalue";

QmitkPixelValueView::QmitkPixelValueView(QObject*)
  : m_Ui(std::make_unique<Ui::QmitkPixelValueView>())
{
}

QmitkPixelValueView::~QmitkPixelValueView()
{
}

void QmitkPixelValueView::CreateQtPartControl(QWidget* parent)
{
  m_Ui->setupUi(parent);

  auto clipboardIcon = QmitkIconTheme::GetIcon(QLatin1String(":/Qmitk/clipboard.svg"));
  m_Ui->copyIndexCoordButton->setIcon(clipboardIcon);
  m_Ui->copyWorldCoordButton->setIcon(clipboardIcon);

  connect(m_Ui->copyIndexCoordButton, &QToolButton::clicked, [this]() {
    CopyCoordsToClipboard(m_Ui->indexXLineEdit, m_Ui->indexYLineEdit, m_Ui->indexZLineEdit);
  });

  connect(m_Ui->copyWorldCoordButton, &QToolButton::clicked, [this]() {
    CopyCoordsToClipboard(m_Ui->worldXLineEdit, m_Ui->worldYLineEdit, m_Ui->worldZLineEdit);
  });

  this->m_SliceNavigationListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceNavigationListener, &QmitkSliceNavigationListener::SelectedPositionChanged, this, &QmitkPixelValueView::OnSelectedPositionChanged);
  connect(&m_SliceNavigationListener, &QmitkSliceNavigationListener::SelectedTimePointChanged, this, &QmitkPixelValueView::OnSelectedTimePointChanged);

  connect(m_Ui->allValuesGroupBox, &QGroupBox::toggled, this, [this](bool expanded) {
    if (expanded)
      this->Update();
  });

  this->Update();
}

void QmitkPixelValueView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartActivated(renderWindowPart);
}

void QmitkPixelValueView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkPixelValueView::RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartInputChanged(renderWindowPart);
}

void QmitkPixelValueView::OnSelectedPositionChanged(const mitk::Point3D&)
{
  this->Update();
}

void QmitkPixelValueView::OnSelectedTimePointChanged(const mitk::TimePointType&)
{
  this->Update();
}

void QmitkPixelValueView::NodeAdded(const mitk::DataNode*)
{
  this->RequestUpdate();
}

void QmitkPixelValueView::NodeChanged(const mitk::DataNode*)
{
  this->RequestUpdate();
}

void QmitkPixelValueView::NodeRemoved(const mitk::DataNode*)
{
  this->RequestUpdate();
}

void QmitkPixelValueView::RequestUpdate()
{
  if (m_IsUpdateRequested)
    return;

  m_IsUpdateRequested = true;

  // Deferred, because DataStorage announces a removal before the node is removed, and
  // because Update() replaces the data observers, one of which may be the caller.
  QTimer::singleShot(0, this, [this]() {
    m_IsUpdateRequested = false;
    this->Update();
  });
}

void QmitkPixelValueView::Update()
{
  const auto position = m_SliceNavigationListener.GetCurrentSelectedPosition();
  const auto timePoint = m_SliceNavigationListener.GetCurrentSelectedTimePoint();

  this->UpdateAllPixelValues(position, timePoint);

  auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto isSegmentation = mitk::NodePredicateDataType::New("MultiLabelSegmentation");

  auto predicate = mitk::NodePredicateAnd::New();
  predicate->AddPredicate(mitk::NodePredicateNot::New(isSegmentation));
  predicate->AddPredicate(isImage);

  auto nodes = GetDataStorage()->GetSubset(predicate);

  if (nodes.IsNull())
  {
    this->Clear();
    return;
  }

  auto node = mitk::FindTopmostVisibleNode(nodes, position, timePoint, nullptr);

  if (node.IsNull())
  {
    this->Clear();
    return;
  }

  // The node whose image is read: a binary image shows the value of its parent image.
  auto imageNode = node;

  bool isBinary = false;
  node->GetBoolProperty("binary", isBinary);

  if (isBinary)
  {
    auto parentNodes = GetDataStorage()->GetSources(node, predicate, true);

    if (!parentNodes->empty())
    {
      if (auto topParentNode = FindTopmostVisibleNode(parentNodes, position, timePoint, nullptr); topParentNode.IsNotNull())
        imageNode = topParentNode;
    }
  }

  const mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(imageNode->GetData());

  if (image.IsNotNull())
  {
    m_Ui->imageNameLineEdit->setText(QString::fromStdString(imageNode->GetName()));

    const auto* timeGeometry = image->GetTimeGeometry();
    const auto timeStep = timeGeometry->TimePointToTimeStep(timePoint);
    const auto geometry = timeGeometry->GetGeometryForTimeStep(timeStep);

    itk::Index<3> index;
    geometry->WorldToIndex(position, index);

    if (!geometry->IsIndexInside(index))
    {
      this->Clear();
      return;
    }

    auto pixelType = image->GetChannelDescriptor().GetPixelType().GetPixelType();

    try
    {
      if (pixelType == itk::IOPixelEnum::RGB || pixelType == itk::IOPixelEnum::RGBA)
      {
        m_Ui->pixelValueLineEdit->setText(ReadCompositePixel(image, image->GetVolumeData(timeStep), index).text);
      }
      else if (pixelType == itk::IOPixelEnum::DIFFUSIONTENSOR3D || pixelType == itk::IOPixelEnum::SYMMETRICSECONDRANKTENSOR)
      {
        m_Ui->pixelValueLineEdit->setText(QStringLiteral("See ODF Details view."));
      }
      else
      {
        mitk::ScalarType pixelValue = 0.0;

        mitkPixelTypeMultiplex5(
          mitk::FastSinglePixelAccess,
          image->GetChannelDescriptor().GetPixelType(),
          image,
          image->GetVolumeData(timeStep),
          index,
          pixelValue,
          GetDisplayedComponent(imageNode, image).value_or(0));

        m_Ui->pixelValueLineEdit->setText(FormatValue(pixelValue, IsInteger(image->GetChannelDescriptor().GetPixelType())));
      }
    }
    catch (const std::exception& e)
    {
      this->ReportReadError(imageNode, e);
      m_Ui->pixelValueLineEdit->setText(QStringLiteral("Error"));
    }

    this->UpdateCoords(image, index, position);
  }
  else
  {
    this->Clear();
  }
}

void QmitkPixelValueView::ReportReadError(const mitk::DataNode* node, const std::exception& e)
{
  // Warn once per node and error, as reads are repeated on every refresh.
  if (m_ReportedReadErrors.insert(node->GetName() + ": " + e.what()).second)
    MITK_WARN << "Cannot read the pixel value of \"" << node->GetName() << "\": " << e.what();
}

void QmitkPixelValueView::UpdateAllPixelValues(const mitk::Point3D& position, mitk::TimePointType timePoint)
{
  m_DataObservers.clear();

  // Nothing to show while collapsed; expanding the group box triggers a rebuild.
  if (m_Ui->allValuesGroupBox->collapsed())
    return;

  auto* scrollBar = m_Ui->allValuesTreeWidget->verticalScrollBar();
  const auto scrollPosition = scrollBar->value();

  m_Ui->allValuesTreeWidget->clear();

  for (const auto* node : GetVisibleNodesAt(this->GetDataStorage(), position, timePoint))
  {
    const auto requestUpdate = [this](const itk::EventObject&) {
      this->RequestUpdate();
    };

    m_DataObservers.emplace_back(node->GetData(), itk::ModifiedEvent(), requestUpdate);

    const auto* segmentation = dynamic_cast<const mitk::MultiLabelSegmentation*>(node->GetData());

    // Undo and redo of a slice edit modify only the group image, not the segmentation.
    if (nullptr != segmentation)
    {
      for (mitk::MultiLabelSegmentation::GroupIndexType i = 0; i < segmentation->GetNumberOfGroups(); ++i)
        m_DataObservers.emplace_back(segmentation->GetGroupImage(i), itk::ModifiedEvent(), requestUpdate);
    }

    try
    {
      if (nullptr != segmentation)
      {
        this->AddSegmentationRows(node, position, timePoint);
      }
      else
      {
        this->AddImageRow(node, position, timePoint);
      }
    }
    catch (const std::exception& e)
    {
      this->ReportReadError(node, e);
      AddRow(m_Ui->allValuesTreeWidget, node, QString::fromStdString(node->GetName()), QStringLiteral("Error"), std::nullopt);
    }
  }

  m_Ui->allValuesTreeWidget->resizeColumnToContents(0);
  m_Ui->allValuesTreeWidget->resizeColumnToContents(1);
  scrollBar->setValue(scrollPosition);
}

void QmitkPixelValueView::AddImageRow(const mitk::DataNode* node, const mitk::Point3D& position, mitk::TimePointType timePoint)
{
  const auto* image = dynamic_cast<const mitk::Image*>(node->GetData());

  if (nullptr == image)
    return;

  const auto* timeGeometry = image->GetTimeGeometry();
  const auto timeStep = timeGeometry->TimePointToTimeStep(timePoint);
  const auto geometry = timeGeometry->GetGeometryForTimeStep(timeStep);

  itk::Index<3> index;
  geometry->WorldToIndex(position, index);

  if (!geometry->IsIndexInside(index))
    return;

  const auto volume = image->GetVolumeData(static_cast<int>(timeStep));

  if (volume.IsNull())
    return;

  const auto name = QString::fromStdString(node->GetName());
  const auto pixelType = image->GetChannelDescriptor().GetPixelType();
  const bool isInteger = IsInteger(pixelType);

  switch (pixelType.GetPixelType())
  {
    case itk::IOPixelEnum::RGB:
    case itk::IOPixelEnum::RGBA:
    {
      const auto [text, color] = ReadCompositePixel(image, volume, index);
      AddRow(m_Ui->allValuesTreeWidget, node, name, text, color);
      return;
    }

    case itk::IOPixelEnum::DIFFUSIONTENSOR3D:
    case itk::IOPixelEnum::SYMMETRICSECONDRANKTENSOR:
      AddRow(m_Ui->allValuesTreeWidget, node, name, QStringLiteral("See ODF Details view."), std::nullopt);
      return;

    default:
      break;
  }

  const auto displayedComponent = GetDisplayedComponent(node, image);
  const auto value = ReadPixelComponent(image, volume, index, displayedComponent.value_or(0));

  bool isBinary = false;
  node->GetBoolProperty("binary", isBinary);

  if (isBinary)
  {
    if (0 == value)
      return;

    bool isSelected = false;
    node->GetBoolProperty("selected", isSelected);

    bool isHovering = false;
    node->GetBoolProperty("binaryimage.ishovering", isHovering);

    const char* colorKey = isSelected ? "binaryimage.selectedcolor"
                         : isHovering ? "binaryimage.hoveringcolor"
                                      : "color";

    float nodeColor[3] = { 1.0f, 1.0f, 1.0f };

    if (!node->GetColor(nodeColor, nullptr, colorKey))
      node->GetColor(nodeColor);

    const auto color = QColor::fromRgbF(nodeColor[0], nodeColor[1], nodeColor[2]);

    AddRow(m_Ui->allValuesTreeWidget, node, name, FormatValue(value, isInteger), color);
    return;
  }

  auto text = FormatValue(value, isInteger);

  if (displayedComponent.has_value())
    text += QStringLiteral(" (component %1)").arg(displayedComponent.value());

  AddRow(m_Ui->allValuesTreeWidget, node, name, text, GetDisplayedColor(node, value));
}

void QmitkPixelValueView::AddSegmentationRows(const mitk::DataNode* node, const mitk::Point3D& position, mitk::TimePointType timePoint)
{
  const auto* segmentation = dynamic_cast<const mitk::MultiLabelSegmentation*>(node->GetData());

  if (nullptr == segmentation)
    return;

  const auto timeStep = segmentation->GetTimeGeometry()->TimePointToTimeStep(timePoint);

  // The default group argument selects only group 0, so request all groups explicitly.
  auto labelValues = segmentation->GetLabelValuesByCoordinates(position, timeStep, std::nullopt);

  // Groups with a higher index are drawn on top.
  std::reverse(labelValues.begin(), labelValues.end());

  const auto nodeName = QString::fromStdString(node->GetName());

  // Like LabelSetImageVtkMapper2D, which scales all label opacities by this Segmentation view preference.
  const auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  const auto opacityFactor = preferences->GetFloat("opacity factor", 1.0f);

  for (const auto labelValue : labelValues)
  {
    const auto label = segmentation->GetLabel(labelValue);

    if (label.IsNull() || !label->GetVisible())
      continue;

    const auto& color = label->GetColor();
    const auto name = QStringLiteral("%1: %2").arg(nodeName, QString::fromStdString(label->GetName()));

    AddRow(m_Ui->allValuesTreeWidget, node, name, QString::number(labelValue),
      QColor::fromRgbF(color.GetRed(), color.GetGreen(), color.GetBlue(), label->GetOpacity() * opacityFactor));
  }
}

void QmitkPixelValueView::UpdateCoords(const mitk::Image* image, const itk::Index<3>& index, const mitk::Point3D& position)
{
  const auto dimension = image->GetDimension();

  this->UpdateIndexCoord(index, dimension);
  this->UpdateWorldCoord(position, dimension);

  const auto geometry = image->GetTimeGeometry();

  this->UpdateTimeStep(geometry);
}

void QmitkPixelValueView::UpdateIndexCoord(const itk::Index<3>& index, unsigned int dimension)
{
  m_Ui->indexXLineEdit->setText(QString::number(index[0]));
  m_Ui->indexYLineEdit->setText(QString::number(index[1]));

  if (dimension > 2)
  {
    m_Ui->indexZLineEdit->setText(QString::number(index[2]));
  }
  else
  {
    m_Ui->indexZLineEdit->clear();
  }

  m_Ui->copyIndexCoordButton->setEnabled(true);
}

void QmitkPixelValueView::UpdateWorldCoord(const mitk::Point3D& position, unsigned int dimension)
{
  m_Ui->worldXLineEdit->setText(QString::number(position[0], 'f', 2));
  m_Ui->worldYLineEdit->setText(QString::number(position[1], 'f', 2));

  if (dimension > 2)
  {
    m_Ui->worldZLineEdit->setText(QString::number(position[2], 'f', 2));
  }
  else
  {
    m_Ui->worldZLineEdit->clear();
  }

  m_Ui->copyWorldCoordButton->setEnabled(true);
}

void QmitkPixelValueView::UpdateTimeStep(const mitk::TimeGeometry* geometry)
{
  const auto* renderingManager = mitk::RenderingManager::GetInstance();
  const auto* timeNavController = renderingManager->GetTimeNavigationController();
  const auto timePoint = timeNavController->GetSelectedTimePoint();

  if (geometry->CountTimeSteps() <= 1 || !geometry->IsValidTimePoint(timePoint))
  {
    this->ShowTimeStep(false);
    return;
  }

  const auto timeStep = geometry->TimePointToTimeStep(timePoint);
  m_Ui->timeStepLineEdit->setText(QString::number(timeStep));

  this->ShowTimeStep(true);
}

void QmitkPixelValueView::ShowTimeStep(bool show)
{
  m_Ui->timeStepLabel->setVisible(show);
  m_Ui->timeStepLineEdit->setVisible(show);
}

void QmitkPixelValueView::Clear()
{
  m_Ui->imageNameLineEdit->clear();
  m_Ui->pixelValueLineEdit->clear();

  this->ClearCoords();
}

void QmitkPixelValueView::ClearCoords()
{
  m_Ui->indexXLineEdit->clear();
  m_Ui->indexYLineEdit->clear();
  m_Ui->indexZLineEdit->clear();

  m_Ui->copyIndexCoordButton->setEnabled(false);

  m_Ui->worldXLineEdit->clear();
  m_Ui->worldYLineEdit->clear();
  m_Ui->worldZLineEdit->clear();

  m_Ui->copyWorldCoordButton->setEnabled(false);

  m_Ui->timeStepLineEdit->clear();

  this->ShowTimeStep(false);
}

void QmitkPixelValueView::SetFocus()
{
  m_Ui->pixelValueLineEdit->setFocus();
}
