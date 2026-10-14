/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRegionGrowingTool.h>
#include <mitkBaseRenderer.h>
#include <mitkContourModel.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageVtkMapper2D.h>
#include <mitkLevelWindowProperty.h>
#include <mitkPointSet.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRenderingManager.h>
#include <mitkToolManager.h>

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

// ITK
#include <mitkITKImageImport.h>
#include <mitkImageAccessByItk.h>
#include <itkBinaryMedianImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkMedianImageFilter.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>
#include <tuple>
#include <utility>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, RegionGrowingTool, "Region growing tool");
}

namespace
{
  /** \brief Radius of the square around the seed the first tissue statistics are taken from, 2 yields 5x5 pixels. */
  constexpr int CoreRadius = 2;

  /** \brief A pixel deviating from the tissue mean by more than this many standard deviations is dissimilar. */
  constexpr double DissimilarDeviation = 3.0;

  /** \brief The tissue standard deviation may grow during growth up to this factor of the one of the core. */
  constexpr double MaxSpreadGrowth = 2.0;

  /** \brief Screen distance in pixels the mouse has to move with the button held to start a drag. */
  constexpr double DragDeadzone = 5.0;

  /** \brief Fraction of its opacity the segmentation keeps while cutting. The active label's outline is unaffected. */
  constexpr float CutOpacityFactor = 0.2f;

  /** \brief Diameter in screen pixels of the marker of the last seed while cutting. */
  constexpr float SeedMarkerSize = 8.0f;

  using BinaryImageType = itk::Image<mitk::Tool::DefaultSegmentationDataType, 2>;

  /** \brief Gray values of a slice, x running fastest, with its geometry in index space. */
  struct SliceSamples
  {
    std::vector<double> values;
    BinaryImageType::SizeType size;
    BinaryImageType::SpacingType spacing;
  };

  /** \brief Calls visit(j, axis) for every 4-neighbor j of pixel i, adjacent to it along axis. Pixels are numbered
   * with x running fastest. */
  template <typename TVisit>
  void ForEachNeighbor(std::size_t i, const BinaryImageType::SizeType& size, const TVisit& visit)
  {
    const std::size_t width = size[0];
    const std::size_t height = size[1];
    const auto x = i % width;
    const auto y = i / width;

    if (x > 0)
      visit(i - 1, 0u);
    if (x + 1 < width)
      visit(i + 1, 0u);
    if (y > 0)
      visit(i - width, 1u);
    if (y + 1 < height)
      visit(i + width, 1u);
  }

  /** \brief Samples the slice after a 3x3 median filter, as noise widens the gray value spread of every tissue and
   * with it the risk of leaking into neighbors. */
  template <typename TPixel, unsigned int VImageDimension>
  void SampleDenoisedSlice(const itk::Image<TPixel, VImageDimension>* image, SliceSamples* samples)
  {
    using ImageType = itk::Image<TPixel, VImageDimension>;

    auto filter = itk::MedianImageFilter<ImageType, ImageType>::New();
    filter->SetInput(image);
    filter->SetRadius(1);
    filter->Update();
    const auto* denoised = filter->GetOutput();
    const auto region = denoised->GetLargestPossibleRegion();

    samples->values.clear();
    samples->values.reserve(region.GetNumberOfPixels());
    for (itk::ImageRegionConstIterator<ImageType> it(denoised, region); !it.IsAtEnd(); ++it)
    {
      // Infinity makes a pixel impossible to grow into, while NaN would break the ordering of the core statistics.
      const auto value = static_cast<double>(it.Get());
      samples->values.push_back(std::isfinite(value) ? value : std::numeric_limits<double>::infinity());
    }

    for (unsigned int i = 0; i < 2; ++i)
    {
      samples->size[i] = region.GetSize(i);
      samples->spacing[i] = denoised->GetSpacing()[i];
    }
  }

  /** \brief Median and spread of the values. The spread is the median absolute deviation, scaled to equal the
   * standard deviation for normally distributed values, so that a few outliers like edge pixels barely affect it. */
  std::pair<double, double> ComputeRobustStatistics(std::vector<double> values)
  {
    const auto middle = values.begin() + values.size() / 2;
    std::nth_element(values.begin(), middle, values.end());
    const auto median = *middle;

    for (auto& value : values)
    {
      value = std::abs(value - median);
    }

    std::nth_element(values.begin(), middle, values.end());
    return { median, 1.4826 * *middle };
  }

  /**
   * \brief Geodesic distance of every pixel from the seed.
   *
   * Stepping onto a pixel costs the step length times 1 + z^2, with z the deviation of its gray value from the mean
   * of the tissue grown so far in standard deviations. The mean and standard deviation start from the seed core and
   * are updated from every pixel the growth reaches, unless that pixel is dissimilar, so foreign tissue a leak has
   * paid its way into cannot drag them along. The standard deviation stays between one and MaxSpreadGrowth times the
   * one of the core, which in turn is at least a small fraction of the gray value range of the image.
   */
  std::vector<double> ComputeGeodesicDistances(const SliceSamples& slice, std::size_t seed, double valueRange)
  {
    const auto& values = slice.values;
    const std::size_t width = slice.size[0];
    const std::size_t height = slice.size[1];
    const std::size_t numberOfPixels = values.size();

    // Tissue statistics of the core around the seed. The seed itself is finite, so the core is never empty.
    std::vector<double> core;
    const auto seedX = static_cast<int>(seed % width);
    const auto seedY = static_cast<int>(seed / width);
    for (int y = std::max(0, seedY - CoreRadius); y <= std::min(static_cast<int>(height) - 1, seedY + CoreRadius); ++y)
    {
      for (int x = std::max(0, seedX - CoreRadius); x <= std::min(static_cast<int>(width) - 1, seedX + CoreRadius); ++x)
      {
        if (std::isfinite(values[x + y * width]))
        {
          core.push_back(values[x + y * width]);
        }
      }
    }

    const auto [coreMean, robustCoreSpread] = ComputeRobustStatistics(std::move(core));

    // A perfectly homogeneous core would make every other gray value infinitely dissimilar. The range is taken from
    // the image, as the slice also covers the background of a world plane reaching beyond the image. An infinite
    // pixel value in the image would make every gray value similar.
    const double rangeSpread = std::isfinite(valueRange) ? 1e-3 * valueRange : 0.0;
    const double coreSpread = std::max(robustCoreSpread, std::max(rangeSpread, 1e-12));

    std::size_t count = 0;
    double mean = coreMean;
    double sumOfSquares = 0.0;

    const auto tissueMean = [&]() { return count < 2 ? coreMean : mean; };
    const auto tissueSpread = [&]() {
      return count < 2 ? coreSpread
                       : std::clamp(std::sqrt(sumOfSquares / count), coreSpread, MaxSpreadGrowth * coreSpread);
    };

    std::vector<double> distances(numberOfPixels, std::numeric_limits<double>::infinity());
    std::vector<bool> settled(numberOfPixels, false);

    using QueueEntry = std::pair<double, std::size_t>;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<>> queue;
    distances[seed] = 0.0;
    queue.emplace(0.0, seed);

    while (!queue.empty())
    {
      const auto [distance, i] = queue.top();
      queue.pop();

      if (settled[i])
      {
        continue;
      }
      settled[i] = true;

      if (std::abs(values[i] - tissueMean()) <= DissimilarDeviation * tissueSpread())
      {
        ++count;
        const double delta = values[i] - mean;
        mean += delta / count;
        sumOfSquares += delta * (values[i] - mean);
      }

      const double currentMean = tissueMean();
      const double currentSpread = tissueSpread();

      ForEachNeighbor(i, slice.size, [&](std::size_t j, unsigned int axis) {
        if (settled[j])
        {
          return;
        }

        const double z = (values[j] - currentMean) / currentSpread;
        const double neighborDistance = distance + slice.spacing[axis] * (1.0 + z * z);
        if (neighborDistance < distances[j])
        {
          distances[j] = neighborDistance;
          queue.emplace(neighborDistance, j);
        }
      });
    }

    return distances;
  }

  /** \brief Every pixel is replaced by the majority of its 5x5 neighborhood. */
  BinaryImageType::Pointer SmoothByMajorityVote(const BinaryImageType* region)
  {
    auto filter = itk::BinaryMedianImageFilter<BinaryImageType, BinaryImageType>::New();
    filter->SetInput(region);
    filter->SetRadius(BinaryImageType::SizeType::Filled(2));
    filter->SetForegroundValue(1);
    filter->SetBackgroundValue(0);
    filter->Update();
    return filter->GetOutput();
  }

  /** \brief 4-connected components of a binary image, with their count. */
  std::pair<BinaryImageType::Pointer, std::size_t> LabelComponents(const BinaryImageType* image)
  {
    auto labeler = itk::ConnectedComponentImageFilter<BinaryImageType, BinaryImageType>::New();
    labeler->SetInput(image);
    labeler->Update();
    return { labeler->GetOutput(), labeler->GetObjectCount() };
  }

  /** \brief The component of region that contains the seed, or an empty image if the seed is not part of region. */
  BinaryImageType::Pointer SelectSeedComponent(const BinaryImageType* region, const itk::Index<2>& seedIndex)
  {
    auto components = LabelComponents(region).first;
    const auto seedComponent = components->GetPixel(seedIndex);

    // Component 0 is the background, so a seed outside the region selects nothing.
    auto* buffer = components->GetBufferPointer();
    for (std::size_t i = 0; i < components->GetLargestPossibleRegion().GetNumberOfPixels(); ++i)
    {
      buffer[i] = 0 != seedComponent && seedComponent == buffer[i] ? 1 : 0;
    }

    return components;
  }

  itk::Index<2> RoundIndex(const itk::ContinuousIndex<double, 2>& index)
  {
    return { { std::lround(index[0]), std::lround(index[1]) } };
  }

  /**
   * \brief Marks every pixel the segment between two continuous indices passes through.
   *
   * Consecutive marked pixels share an edge, so the marked line separates the 4-connected components it crosses.
   * Pixels outside mask are skipped.
   */
  void RasterizeSegment(BinaryImageType* mask, const itk::ContinuousIndex<double, 2>& from,
                        const itk::ContinuousIndex<double, 2>& to)
  {
    const auto bounds = mask->GetLargestPossibleRegion();

    // Pixel i covers [i - 0.5, i + 0.5), so shift by half a pixel to walk a grid with cells [i, i + 1).
    const double startX = from[0] + 0.5;
    const double startY = from[1] + 0.5;
    const double deltaX = to[0] - from[0];
    const double deltaY = to[1] - from[1];

    itk::Index<2> index = { { static_cast<itk::IndexValueType>(std::floor(startX)),
                              static_cast<itk::IndexValueType>(std::floor(startY)) } };
    const itk::Index<2> end = { { static_cast<itk::IndexValueType>(std::floor(to[0] + 0.5)),
                                  static_cast<itk::IndexValueType>(std::floor(to[1] + 0.5)) } };

    const int stepX = deltaX > 0 ? 1 : -1;
    const int stepY = deltaY > 0 ? 1 : -1;
    constexpr double Never = std::numeric_limits<double>::infinity();

    // Parameter along the segment at which the next vertical / horizontal cell border is crossed
    double nextX = 0.0 == deltaX ? Never : (stepX > 0 ? index[0] + 1 - startX : startX - index[0]) / std::abs(deltaX);
    double nextY = 0.0 == deltaY ? Never : (stepY > 0 ? index[1] + 1 - startY : startY - index[1]) / std::abs(deltaY);
    const double stepParameterX = 0.0 == deltaX ? Never : 1.0 / std::abs(deltaX);
    const double stepParameterY = 0.0 == deltaY ? Never : 1.0 / std::abs(deltaY);

    const auto numberOfSteps = std::abs(end[0] - index[0]) + std::abs(end[1] - index[1]);
    for (itk::IndexValueType step = 0;; ++step)
    {
      if (bounds.IsInside(index))
      {
        mask->SetPixel(index, 1);
      }

      if (step == numberOfSteps)
      {
        break;
      }

      if (nextX < nextY)
      {
        nextX += stepParameterX;
        index[0] += stepX;
      }
      else
      {
        nextY += stepParameterY;
        index[1] += stepY;
      }
    }
  }

  /**
   * \brief The pixels of the label that a cut line separates and that are to be removed, or null if it separates none.
   *
   * A line separates nothing unless both its ends lie outside the label. Of each part of the label the line splits,
   * the piece containing protectedIndex is kept, or the largest piece without one. Line pixels on the label go with the
   * nearest pieces, so a kept piece does not lose its edge to the line. labelParts are the components of labelMask as
   * computed by LabelComponents().
   */
  BinaryImageType::Pointer ComputeCutAway(const BinaryImageType* labelMask, const BinaryImageType* labelParts,
                                          std::size_t numberOfParts, const BinaryImageType* lineMask,
                                          const itk::Index<2>& start, const itk::Index<2>& end,
                                          const itk::Index<2>* protectedIndex)
  {
    const auto bounds = labelMask->GetLargestPossibleRegion();
    for (const auto& lineEnd : { start, end })
    {
      if (bounds.IsInside(lineEnd) && 0 != labelMask->GetPixel(lineEnd))
      {
        return nullptr;
      }
    }

    const std::size_t numberOfPixels = bounds.GetNumberOfPixels();
    const auto* label = labelMask->GetBufferPointer();
    const auto* line = lineMask->GetBufferPointer();

    auto pieceMask = BinaryImageType::New();
    pieceMask->CopyInformation(labelMask);
    pieceMask->SetRegions(bounds);
    pieceMask->Allocate();
    auto* pieceBuffer = pieceMask->GetBufferPointer();
    for (std::size_t i = 0; i < numberOfPixels; ++i)
    {
      pieceBuffer[i] = (0 != label[i] && 0 == line[i]) ? 1 : 0;
    }

    const auto [pieces, numberOfPieces] = LabelComponents(pieceMask);
    const auto* part = labelParts->GetBufferPointer();
    const auto* piece = pieces->GetBufferPointer();

    std::vector<std::size_t> pieceSize(numberOfPieces + 1, 0);
    std::vector<std::size_t> partOfPiece(numberOfPieces + 1, 0);
    std::vector<bool> isCrossed(numberOfParts + 1, false);
    for (std::size_t i = 0; i < numberOfPixels; ++i)
    {
      if (0 != piece[i])
      {
        ++pieceSize[piece[i]];
        partOfPiece[piece[i]] = part[i];
      }
      if (0 != line[i] && 0 != part[i])
      {
        isCrossed[part[i]] = true;
      }
    }

    std::vector<std::vector<std::size_t>> piecesOfPart(numberOfParts + 1);
    for (std::size_t p = 1; p <= numberOfPieces; ++p)
    {
      if (isCrossed[partOfPiece[p]])
      {
        piecesOfPart[partOfPiece[p]].push_back(p);
      }
    }

    const std::size_t protectedPiece =
      nullptr != protectedIndex && bounds.IsInside(*protectedIndex) ? pieces->GetPixel(*protectedIndex) : 0;

    std::vector<bool> isRemoved(numberOfPieces + 1, false);
    bool removesAnything = false;
    for (const auto& partPieces : piecesOfPart)
    {
      if (partPieces.size() < 2)
      {
        continue;
      }

      auto kept = *std::max_element(partPieces.begin(), partPieces.end(),
                                    [&](std::size_t a, std::size_t b) { return pieceSize[a] < pieceSize[b]; });
      if (std::find(partPieces.begin(), partPieces.end(), protectedPiece) != partPieces.end())
      {
        kept = protectedPiece;
      }

      for (const auto p : partPieces)
      {
        if (p != kept)
        {
          isRemoved[p] = true;
          removesAnything = true;
        }
      }
    }

    if (!removesAnything)
    {
      return nullptr;
    }

    enum class Side : unsigned char
    {
      Unassigned,
      Kept,
      Removed
    };

    std::vector<Side> side(numberOfPixels, Side::Unassigned);
    std::vector<std::size_t> unassignedLinePixels;
    for (std::size_t i = 0; i < numberOfPixels; ++i)
    {
      if (0 != piece[i])
      {
        side[i] = isRemoved[piece[i]] ? Side::Removed : Side::Kept;
      }
      else if (0 != line[i] && 0 != label[i])
      {
        unassignedLinePixels.push_back(i);
      }
    }

    // A line drawn several pixels thick is assigned layer by layer, each layer at once, so that a line pixel
    // bordering both sides is kept.
    std::vector<std::pair<std::size_t, Side>> assignments;
    do
    {
      assignments.clear();
      std::erase_if(unassignedLinePixels, [&](std::size_t i) {
        bool bordersKept = false;
        bool bordersRemoved = false;
        ForEachNeighbor(i, bounds.GetSize(), [&](std::size_t j, unsigned int) {
          bordersKept |= Side::Kept == side[j];
          bordersRemoved |= Side::Removed == side[j];
        });

        if (!bordersKept && !bordersRemoved)
        {
          return false;
        }

        assignments.emplace_back(i, bordersKept ? Side::Kept : Side::Removed);
        return true;
      });

      for (const auto& [i, assignedSide] : assignments)
      {
        side[i] = assignedSide;
      }
    } while (!assignments.empty());

    auto cutAway = BinaryImageType::New();
    cutAway->CopyInformation(labelMask);
    cutAway->SetRegions(bounds);
    cutAway->Allocate(true);
    auto* cutAwayBuffer = cutAway->GetBufferPointer();

    for (std::size_t i = 0; i < numberOfPixels; ++i)
    {
      if (Side::Removed == side[i])
      {
        cutAwayBuffer[i] = 1;
      }
    }

    return cutAway;
  }

  mitk::DataNode::Pointer CreatePreviewNode(const std::string& name)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    node->SetBoolProperty("binary", true);
    node->SetBoolProperty("helper object", true);
    node->SetBoolProperty("includeInBoundingBox", false);
    node->SetBoolProperty("fixedLayer", true);
    node->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(0, 1)));
    node->SetColor(0.0, 1.0, 0.0);

    for (const auto& renderWindowAndRenderer : mitk::BaseRenderer::GetAll3DRenderWindows())
    {
      node->SetVisibility(false, renderWindowAndRenderer.second);
    }

    return node;
  }

  /** \brief Makes data with a single time step valid at every time point, like a single time step image. */
  void SetUnboundedTimeGeometry(mitk::BaseData* data)
  {
    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(data->GetGeometry(), 1);
    data->SetTimeGeometry(timeGeometry);
  }

  /** \brief Shows the node in renderer only. */
  void ShowOnlyIn(mitk::DataNode* node, const mitk::BaseRenderer* renderer)
  {
    for (const auto& rendererName : node->GetPropertyListNames())
    {
      node->GetPropertyList(rendererName)->DeleteProperty("visible");
    }

    node->SetVisibility(false);
    node->SetVisibility(true, renderer);
  }

}

mitk::RegionGrowingTool::RegionGrowingTool()
  : SegTool2D("RegionGrowingTool")
{
  this->RequiresScalarReferenceSliceOn();
}

mitk::RegionGrowingTool::~RegionGrowingTool()
{
}

void mitk::RegionGrowingTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION("Move", OnMouseMoved);
  CONNECT_FUNCTION("Release", OnMouseReleased);
  CONNECT_FUNCTION("ArmCut", OnCutArmed);
  CONNECT_FUNCTION("DisarmCut", OnCutDisarmed);
  CONNECT_FUNCTION("StartCut", OnCutStarted);
  CONNECT_FUNCTION("MoveCut", OnCutMoved);
  CONNECT_FUNCTION("FinishCut", OnCutFinished);
  CONNECT_FUNCTION("Cancel", OnCancel);
}

us::ModuleResource mitk::RegionGrowingTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("RegionGrowing.svg");
  return resource;
}

us::ModuleResource mitk::RegionGrowingTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("RegionGrowing_Cursor.svg");
  return resource;
}

const char *mitk::RegionGrowingTool::GetName() const
{
  return "Region Growing";
}

mitk::Label::AlgorithmType mitk::RegionGrowingTool::GetAlgorithmType() const
{
  return mitk::Label::AlgorithmType::SEMIAUTOMATIC;
}

void mitk::RegionGrowingTool::Activated()
{
  Superclass::Activated();

  m_PreviewNode = CreatePreviewNode("RegionGrowing_Preview");
  m_PreviewNode->SetBoolProperty("outline binary", false);
  m_PreviewNode->SetOpacity(0.2f);

  m_PreviewOutlineNode = CreatePreviewNode("RegionGrowing_PreviewOutline");
  m_PreviewOutlineNode->SetBoolProperty("outline binary", true);
  m_PreviewOutlineNode->SetFloatProperty("outline width", 2.0f);
  m_PreviewOutlineNode->SetBoolProperty("outline binary shadow", true);
  m_PreviewOutlineNode->SetProperty("outline binary shadow color", ColorProperty::New(0.0f, 0.0f, 0.0f));
  m_PreviewOutlineNode->SetOpacity(1.0f);

  m_CutAwayNode = CreatePreviewNode("RegionGrowing_CutAway");
  m_CutAwayNode->SetBoolProperty("outline binary", false);
  m_CutAwayNode->SetColor(1.0, 0.0, 0.0);
  m_CutAwayNode->SetOpacity(0.5f);

  m_CutLineNode = DataNode::New();
  m_CutLineNode->SetName("RegionGrowing_CutLine");
  m_CutLineNode->SetBoolProperty("helper object", true);
  m_CutLineNode->SetBoolProperty("includeInBoundingBox", false);
  m_CutLineNode->SetBoolProperty("contour.project-onto-plane", false);
  m_CutLineNode->SetFloatProperty("contour.width", 2.0f);
  m_CutLineNode->SetIntProperty("layer", std::numeric_limits<short>::max());
  m_CutLineNode->SetBoolProperty("fixedLayer", true);
  m_CutLineNode->SetColor(1.0f, 0.0f, 0.0f);

  m_SeedMarkerNode = DataNode::New();
  m_SeedMarkerNode->SetName("RegionGrowing_SeedMarker");
  m_SeedMarkerNode->SetBoolProperty("helper object", true);
  m_SeedMarkerNode->SetBoolProperty("includeInBoundingBox", false);
  m_SeedMarkerNode->SetProperty("Pointset.2D.shape", PointSetShapeProperty::New(PointSetShapeProperty::CIRCLE));
  m_SeedMarkerNode->SetIntProperty("Pointset.2D.resolution", 32);
  m_SeedMarkerNode->SetFloatProperty("point 2D size", SeedMarkerSize);
  m_SeedMarkerNode->SetBoolProperty("Pointset.2D.fixed size on screen", true);
  m_SeedMarkerNode->SetFloatProperty("Pointset.2D.distance to plane", 0.5f);
  m_SeedMarkerNode->SetBoolProperty("Pointset.2D.fill shape", true);
  m_SeedMarkerNode->SetIntProperty("layer", std::numeric_limits<short>::max());
  m_SeedMarkerNode->SetBoolProperty("fixedLayer", true);

  auto dataStorage = this->GetToolManager()->GetDataStorage();
  for (auto* node : { m_PreviewNode.GetPointer(), m_PreviewOutlineNode.GetPointer(), m_CutAwayNode.GetPointer(),
                      m_CutLineNode.GetPointer(), m_SeedMarkerNode.GetPointer() })
  {
    dataStorage->Add(node);
  }
}

void mitk::RegionGrowingTool::Deactivated()
{
  this->DiscardPreview();
  this->DiscardCut();
  this->DisarmCut();
  m_LastSeedPlane = nullptr;

  auto dataStorage = this->GetToolManager()->GetDataStorage();
  for (auto* node : { m_PreviewNode.GetPointer(), m_PreviewOutlineNode.GetPointer(), m_CutAwayNode.GetPointer(),
                      m_CutLineNode.GetPointer(), m_SeedMarkerNode.GetPointer() })
  {
    if (dataStorage->Exists(node))
    {
      dataStorage->Remove(node);
    }
  }
  m_PreviewNode = nullptr;
  m_PreviewOutlineNode = nullptr;
  m_CutAwayNode = nullptr;
  m_CutLineNode = nullptr;
  m_SeedMarkerNode = nullptr;

  Superclass::Deactivated();
}

void mitk::RegionGrowingTool::OnTimePointChanged()
{
  this->DiscardPreview();
  this->DiscardCut();

  // The last seed belongs to one time step only.
  if (m_IsCutArmed)
  {
    if (const auto* renderer = BaseRenderer::GetByName(m_ArmedRendererName); nullptr != renderer)
    {
      this->ArmCut(renderer);
    }
    else
    {
      this->DisarmCut();
    }
  }
}

void mitk::RegionGrowingTool::OnMousePressed(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (nullptr == positionEvent)
  {
    return;
  }

  this->DiscardPreview();
  this->DiscardCut();
  this->UpdatePreviewLayers();

  m_PressPositionOnScreen = positionEvent->GetPointerPositionOnScreen();
  m_IsDragging = false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  // ReferenceSlice is from the underlying image, WorkingSlice from the active segmentation (can be empty)
  auto referenceSlice = this->GetAffectedReferenceSlice(positionEvent);
  auto workingSlice = this->GetAffectedWorkingSlice(positionEvent);

  if (referenceSlice.IsNull() || workingSlice.IsNull())
  {
    // can't do anything without a working slice (i.e. a possibly empty segmentation)
    return;
  }

  // Determine if the user clicked inside or outside of the working slice (i.e. the whole volume)
  const auto* workingSliceGeometry = workingSlice->GetGeometry();
  itk::Index<3> seedIndex;
  workingSliceGeometry->WorldToIndex(positionEvent->GetPositionInWorld(), seedIndex);

  if (!workingSliceGeometry->IsIndexInside(seedIndex))
  {
    MITK_DEBUG << "OnMousePressed: point " << positionEvent->GetPositionInWorld() << " (index coordinates "
               << seedIndex << ") is not inside working slice";
    return;
  }

  SliceSamples slice;
  AccessFixedDimensionByItk_1(referenceSlice, SampleDenoisedSlice, 2, &slice);

  // Growth runs on the reference slice, but is seeded with the working slice index, as the result is written there.
  m_SeedIndex[0] = seedIndex[0];
  m_SeedIndex[1] = seedIndex[1];
  if (static_cast<itk::SizeValueType>(m_SeedIndex[0]) >= slice.size[0] ||
      static_cast<itk::SizeValueType>(m_SeedIndex[1]) >= slice.size[1])
  {
    return;
  }

  const auto* referenceImage = this->GetReferenceData();
  int displayedComponent = 0;
  this->GetReferenceDataNode()->GetIntProperty("Image.Displayed Component", displayedComponent);
  const auto component = static_cast<unsigned int>(displayedComponent);
  const auto referenceTimeStep =
    static_cast<int>(referenceImage->GetTimeGeometry()->TimePointToTimeStep(positionEvent->GetSender()->GetTime()));
  auto* statistics = referenceImage->GetStatistics();
  const double valueRange = statistics->GetScalarValueMax(referenceTimeStep, component) -
                            statistics->GetScalarValueMin(referenceTimeStep, component);

  const auto seed = static_cast<std::size_t>(m_SeedIndex[0] + m_SeedIndex[1] * slice.size[0]);
  if (std::isinf(slice.values[seed]))
  {
    return;
  }

  m_Distances = ComputeGeodesicDistances(slice, seed, valueRange);
  m_SliceSize = slice.size;
  m_SliceSpacing = slice.spacing;
  m_SeedInWorld = positionEvent->GetPositionInWorld();

  m_WorkingSlice = workingSlice;
  m_WorkingPlane = positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone();
  m_TimeStep = this->GetWorkingData()->GetTimeGeometry()->TimePointToTimeStep(positionEvent->GetSender()->GetTime());
}

void mitk::RegionGrowingTool::OnMouseMoved(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (nullptr == positionEvent || m_WorkingPlane.IsNull() || positionEvent->GetSender() != m_LastEventSender)
  {
    return;
  }

  if (!m_IsDragging &&
      positionEvent->GetPointerPositionOnScreen().EuclideanDistanceTo(m_PressPositionOnScreen) < DragDeadzone)
  {
    return;
  }

  itk::Index<3> cursorIndex;
  m_WorkingSlice->GetGeometry()->WorldToIndex(positionEvent->GetPositionInWorld(), cursorIndex);

  // A cursor outside the slice holds the edge at the slice border closest to it.
  const auto clamp = [](itk::IndexValueType value, itk::SizeValueType size) {
    return static_cast<std::size_t>(std::clamp<itk::IndexValueType>(value, 0, static_cast<itk::IndexValueType>(size) - 1));
  };
  const auto x = clamp(cursorIndex[0], m_SliceSize[0]);
  const auto y = clamp(cursorIndex[1], m_SliceSize[1]);

  // A single dissimilar pixel under the cursor is far more distant than its neighbors, so the edge would jump.
  double maxDistance = std::numeric_limits<double>::infinity();
  for (auto ny = y > 0 ? y - 1 : y; ny <= std::min<std::size_t>(y + 1, m_SliceSize[1] - 1); ++ny)
  {
    for (auto nx = x > 0 ? x - 1 : x; nx <= std::min<std::size_t>(x + 1, m_SliceSize[0] - 1); ++nx)
    {
      maxDistance = std::min(maxDistance, m_Distances[nx + ny * m_SliceSize[0]]);
    }
  }

  // Pixels the growth cannot reach would turn the region into the whole slice, so the edge stays where it was.
  if (std::isinf(maxDistance))
  {
    return;
  }

  const bool isDragStart = !m_IsDragging;
  m_IsDragging = true;

  if (isDragStart || maxDistance != m_MaxDistance)
  {
    m_MaxDistance = maxDistance;
    this->UpdatePreview();
  }
}

void mitk::RegionGrowingTool::OnMouseReleased(StateMachineAction*, InteractionEvent*)
{
  if (m_IsDragging && this->WriteBackPreview())
  {
    auto* workingSeg = this->GetWorkingData();
    m_LastSeed = m_SeedInWorld;
    m_LastSeedPlane = m_WorkingPlane;
    m_LastSeedTimeStep = m_TimeStep;
    m_LastSeedSegmentation = workingSeg;
    m_LastSeedLabelValue = workingSeg->GetActiveLabel()->GetValue();
  }

  this->DiscardPreview();
}

void mitk::RegionGrowingTool::OnCutStarted(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (nullptr == positionEvent)
  {
    return;
  }

  this->DiscardPreview();
  this->DiscardCut();
  this->UpdatePreviewLayers();

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  // Without a key event before, e.g. as the render window had no keyboard focus, the cut is not prepared yet.
  this->ArmCut(m_LastEventSender);

  auto* workingSeg = this->GetWorkingData();
  const auto* activeLabel = nullptr != workingSeg ? workingSeg->GetActiveLabel() : nullptr;
  if (nullptr == activeLabel)
  {
    return;
  }

  m_CutLabelValue = activeLabel->GetValue();
  m_CutPlane = positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone();
  m_CutTimeStep = workingSeg->GetTimeGeometry()->TimePointToTimeStep(positionEvent->GetSender()->GetTime());

  const auto extractedSlice = SegTool2D::GetAffectedImageSliceAs2DImage(
    m_CutPlane, workingSeg->GetGroupImage(workingSeg->GetGroupIndexOfLabel(m_CutLabelValue)), m_CutTimeStep);
  if (extractedSlice.IsNull())
  {
    m_CutPlane = nullptr;
    return;
  }
  m_CutSlice = extractedSlice->Clone();

  const auto* geometry = m_CutSlice->GetGeometry();
  BinaryImageType::SizeType size = { { m_CutSlice->GetDimension(0), m_CutSlice->GetDimension(1) } };
  BinaryImageType::SpacingType spacing;
  spacing[0] = geometry->GetSpacing()[0];
  spacing[1] = geometry->GetSpacing()[1];

  m_CutLabelMask = BinaryImageType::New();
  m_CutLabelMask->SetRegions(size);
  m_CutLabelMask->SetSpacing(spacing);
  m_CutLabelMask->Allocate();
  {
    ImagePixelReadAccessor<DefaultSegmentationDataType, 2> accessor(m_CutSlice);
    const auto* labels = accessor.GetData();
    auto* mask = m_CutLabelMask->GetBufferPointer();
    for (std::size_t i = 0; i < m_CutLabelMask->GetLargestPossibleRegion().GetNumberOfPixels(); ++i)
    {
      mask[i] = labels[i] == m_CutLabelValue ? 1 : 0;
    }
  }

  m_CutLineMask = BinaryImageType::New();
  m_CutLineMask->SetRegions(size);
  m_CutLineMask->SetSpacing(spacing);
  m_CutLineMask->Allocate(true);

  Point3D index;
  geometry->WorldToIndex(positionEvent->GetPositionInWorld(), index);
  m_CutLastIndex[0] = index[0];
  m_CutLastIndex[1] = index[1];
  m_CutStartIndex = RoundIndex(m_CutLastIndex);
  RasterizeSegment(m_CutLineMask, m_CutLastIndex, m_CutLastIndex);
  std::tie(m_CutLabelParts, m_CutNumberOfLabelParts) = LabelComponents(m_CutLabelMask);

  auto line = ContourModel::New();
  SetUnboundedTimeGeometry(line);
  line->AddVertex(positionEvent->GetPositionInWorld());
  m_CutLineNode->SetData(line);
  ShowOnlyIn(m_CutLineNode, m_LastEventSender);

  m_HasProtectedIndex = this->IsLastSeedApplicable(positionEvent->GetSender());
  if (m_HasProtectedIndex)
  {
    geometry->WorldToIndex(m_LastSeed, index);
    m_ProtectedIndex = { { std::lround(index[0]), std::lround(index[1]) } };
  }

  this->UpdateCut();
}

void mitk::RegionGrowingTool::OnCutMoved(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (nullptr == positionEvent || m_CutSlice.IsNull() || positionEvent->GetSender() != m_LastEventSender)
  {
    return;
  }

  Point3D index;
  m_CutSlice->GetGeometry()->WorldToIndex(positionEvent->GetPositionInWorld(), index);
  itk::ContinuousIndex<double, 2> cursorIndex;
  cursorIndex[0] = index[0];
  cursorIndex[1] = index[1];

  RasterizeSegment(m_CutLineMask, m_CutLastIndex, cursorIndex);
  m_CutLastIndex = cursorIndex;

  if (auto* line = dynamic_cast<ContourModel*>(m_CutLineNode->GetData()))
  {
    line->AddVertex(positionEvent->GetPositionInWorld());
  }

  this->UpdateCut();
}

void mitk::RegionGrowingTool::OnCutFinished(StateMachineAction*, InteractionEvent*)
{
  if (m_CutAway.IsNotNull())
  {
    {
      ImagePixelReadAccessor<DefaultSegmentationDataType, 2> cutAwayAccessor(m_CutAway);
      ImagePixelWriteAccessor<DefaultSegmentationDataType, 2> accessor(m_CutSlice);
      auto* labels = accessor.GetData();
      const auto* cutAway = cutAwayAccessor.GetData();
      const auto numberOfPixels = static_cast<std::size_t>(m_CutAway->GetDimension(0)) * m_CutAway->GetDimension(1);
      for (std::size_t i = 0; i < numberOfPixels; ++i)
      {
        if (0 != cutAway[i])
        {
          labels[i] = MultiLabelSegmentation::UNLABELED_VALUE;
        }
      }
    }

    this->WriteBackSegmentationResult(m_CutPlane, m_CutSlice, m_CutTimeStep);
  }

  this->DiscardCut();
}

void mitk::RegionGrowingTool::OnCancel(StateMachineAction*, InteractionEvent*)
{
  this->DiscardPreview();
  this->DiscardCut();
}

void mitk::RegionGrowingTool::UpdateCut()
{
  auto cutAway = ComputeCutAway(m_CutLabelMask, m_CutLabelParts, m_CutNumberOfLabelParts, m_CutLineMask,
                                m_CutStartIndex, RoundIndex(m_CutLastIndex),
                                m_HasProtectedIndex ? &m_ProtectedIndex : nullptr);

  if (cutAway.IsNotNull())
  {
    m_CutAway = mitk::GrabItkImageMemory(cutAway);
    m_CutAway->SetGeometry(m_CutSlice->GetGeometry());
  }
  else
  {
    m_CutAway = nullptr;
  }

  m_CutAwayNode->SetData(m_CutAway);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(RenderingManager::REQUEST_UPDATE_2DWINDOWS);
}

void mitk::RegionGrowingTool::DiscardCut()
{
  if (m_CutSlice.IsNull() && m_CutPlane.IsNull())
  {
    return;
  }

  m_CutSlice = nullptr;
  m_CutPlane = nullptr;
  m_CutLabelMask = nullptr;
  m_CutLabelParts = nullptr;
  m_CutLineMask = nullptr;
  m_CutAway = nullptr;

  for (auto* node : { m_CutAwayNode.GetPointer(), m_CutLineNode.GetPointer() })
  {
    if (nullptr != node)
    {
      node->SetData(nullptr);
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(RenderingManager::REQUEST_UPDATE_2DWINDOWS);
}

void mitk::RegionGrowingTool::OnCutArmed(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const auto* renderer = nullptr != interactionEvent ? interactionEvent->GetSender() : nullptr;
  const std::string rendererName = nullptr != renderer ? renderer->GetName() : "";

  // Moving the mouse with Ctrl held arms again and again, so only a change of the render window counts.
  if (!m_IsCutArmed || rendererName != m_ArmedRendererName)
  {
    this->ArmCut(renderer);
  }
}

void mitk::RegionGrowingTool::OnCutDisarmed(StateMachineAction*, InteractionEvent*)
{
  this->DisarmCut();
}

void mitk::RegionGrowingTool::ArmCut(const BaseRenderer* renderer)
{
  if (!m_IsCutArmed)
  {
    m_IsCutArmed = true;

    auto module = us::GetModuleContext()->GetModule();
    this->PushCursor(module->GetResource("Cut_Cursor.svg"));
  }

  const std::string rendererName = nullptr != renderer ? renderer->GetName() : "";
  if (rendererName != m_ArmedRendererName)
  {
    this->RestoreSegmentationOpacity();
    m_ArmedRendererName = rendererName;
    this->DimSegmentation();
  }

  const auto* workingSeg = this->GetWorkingData();
  const auto* activeLabel = nullptr != workingSeg ? workingSeg->GetActiveLabel() : nullptr;

  if (nullptr != activeLabel && this->IsLastSeedApplicable(renderer))
  {
    auto seedMarker = PointSet::New();
    SetUnboundedTimeGeometry(seedMarker);
    seedMarker->InsertPoint(m_LastSeed);
    m_SeedMarkerNode->SetData(seedMarker);
    m_SeedMarkerNode->SetColor(activeLabel->GetColor());
    ShowOnlyIn(m_SeedMarkerNode, renderer);
  }
  else
  {
    m_SeedMarkerNode->SetData(nullptr);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(RenderingManager::REQUEST_UPDATE_2DWINDOWS);
}

void mitk::RegionGrowingTool::DisarmCut()
{
  if (!m_IsCutArmed)
  {
    return;
  }

  m_IsCutArmed = false;

  this->PopCursor();

  this->RestoreSegmentationOpacity();
  m_ArmedRendererName.clear();

  if (m_SeedMarkerNode.IsNotNull())
  {
    m_SeedMarkerNode->SetData(nullptr);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(RenderingManager::REQUEST_UPDATE_2DWINDOWS);
}

void mitk::RegionGrowingTool::DimSegmentation()
{
  m_DimmedNode = !m_ArmedRendererName.empty() ? this->GetToolManager()->GetWorkingData(0) : nullptr;
  if (m_DimmedNode.IsNull())
  {
    return;
  }

  m_DimmedNode->GetPropertyList(m_ArmedRendererName)
    ->SetFloatProperty(LabelSetImageVtkMapper2D::PROPERTY_NAME_2D_OPACITY_SCALE(), CutOpacityFactor);
}

void mitk::RegionGrowingTool::RestoreSegmentationOpacity()
{
  if (m_DimmedNode.IsNull())
  {
    return;
  }

  m_DimmedNode->GetPropertyList(m_ArmedRendererName)
    ->DeleteProperty(LabelSetImageVtkMapper2D::PROPERTY_NAME_2D_OPACITY_SCALE());
  m_DimmedNode = nullptr;
}

bool mitk::RegionGrowingTool::IsLastSeedApplicable(const BaseRenderer* renderer) const
{
  if (m_LastSeedPlane.IsNull() || nullptr == renderer || BaseRenderer::Standard2D != renderer->GetMapperID())
  {
    return false;
  }

  const auto* workingSeg = this->GetWorkingData();
  const auto* activeLabel = nullptr != workingSeg ? workingSeg->GetActiveLabel() : nullptr;
  const auto* plane = renderer->GetCurrentWorldPlaneGeometry();
  if (nullptr == activeLabel || nullptr == plane || m_LastSeedSegmentation != workingSeg ||
      activeLabel->GetValue() != m_LastSeedLabelValue ||
      workingSeg->GetTimeGeometry()->TimePointToTimeStep(renderer->GetTime()) != m_LastSeedTimeStep)
  {
    return false;
  }

  // The seed lies on the plane it was grown in. Slices through the segmentation are at least a voxel apart.
  const auto spacing = workingSeg->GetGeometry(m_LastSeedTimeStep)->GetSpacing();
  const auto minSpacing = std::min({ spacing[0], spacing[1], spacing[2] });
  return plane->IsParallel(m_LastSeedPlane) && plane->DistanceFromPlane(m_LastSeed) <= 0.5 * minSpacing;
}

void mitk::RegionGrowingTool::UpdatePreviewLayers()
{
  // The image mapper turns the layer into a depth within the clipping range, so the previews are put right above the
  // data they are drawn against rather than at a fixed top layer.
  int baseLayer = 0;
  for (const auto* node : { this->GetToolManager()->GetWorkingData(0), this->GetReferenceDataNode() })
  {
    int layer = 0;
    if (nullptr != node && node->GetIntProperty("layer", layer))
    {
      baseLayer = std::max(baseLayer, layer);
    }
  }

  m_PreviewNode->SetIntProperty("layer", baseLayer + 1);
  m_CutAwayNode->SetIntProperty("layer", baseLayer + 1);
  m_PreviewOutlineNode->SetIntProperty("layer", baseLayer + 2);
}

void mitk::RegionGrowingTool::UpdatePreview()
{
  auto region = BinaryImageType::New();
  region->SetRegions(m_SliceSize);
  region->SetSpacing(m_SliceSpacing);
  region->Allocate();

  auto* buffer = region->GetBufferPointer();
  for (std::size_t i = 0; i < m_Distances.size(); ++i)
  {
    buffer[i] = m_Distances[i] <= m_MaxDistance ? 1 : 0;
  }

  // The majority vote removes small regions entirely. Rather show the unsmoothed region than nothing.
  auto smoothed = SmoothByMajorityVote(region);
  if (0 == smoothed->GetPixel(m_SeedIndex))
  {
    smoothed = region;
  }

  auto preview = SelectSeedComponent(smoothed, m_SeedIndex);

  auto previewSlice = mitk::GrabItkImageMemory(preview);
  previewSlice->SetGeometry(m_WorkingSlice->GetGeometry());

  m_PreviewNode->SetData(previewSlice);
  m_PreviewOutlineNode->SetData(previewSlice);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(RenderingManager::REQUEST_UPDATE_2DWINDOWS);
}

bool mitk::RegionGrowingTool::WriteBackPreview()
{
  auto* workingSeg = this->GetWorkingData();
  const auto* activeLabel = nullptr != workingSeg ? workingSeg->GetActiveLabel() : nullptr;
  const auto* previewSlice = nullptr != m_PreviewNode ? dynamic_cast<const Image*>(m_PreviewNode->GetData()) : nullptr;

  if (m_WorkingPlane.IsNull() || nullptr == activeLabel || nullptr == previewSlice)
  {
    return false;
  }

  const auto activeLabelValue = activeLabel->GetValue();
  const auto groupIndex = workingSeg->GetGroupIndexOfLabel(activeLabelValue);

  const auto extractedSlice =
    SegTool2D::GetAffectedImageSliceAs2DImage(m_WorkingPlane, workingSeg->GetGroupImage(groupIndex), m_TimeStep);
  if (extractedSlice.IsNull())
  {
    return false;
  }
  auto slice = extractedSlice->Clone();

  TransferLabelContentAtTimeStep(previewSlice, slice,
    workingSeg->GetConstLabelsByValue(workingSeg->GetLabelValuesByGroup(groupIndex)), 0,
    MultiLabelSegmentation::UNLABELED_VALUE, MultiLabelSegmentation::UNLABELED_VALUE, false,
    { {1, activeLabelValue} }, MultiLabelSegmentation::MergeStyle::Merge);

  this->WriteBackSegmentationResult(m_WorkingPlane, slice, m_TimeStep);
  return true;
}

void mitk::RegionGrowingTool::DiscardPreview()
{
  m_IsDragging = false;

  if (m_WorkingPlane.IsNull())
  {
    return;
  }

  m_WorkingSlice = nullptr;
  m_WorkingPlane = nullptr;
  m_Distances.clear();
  m_Distances.shrink_to_fit();

  for (auto* node : { m_PreviewNode.GetPointer(), m_PreviewOutlineNode.GetPointer() })
  {
    if (nullptr != node)
    {
      node->SetData(nullptr);
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(RenderingManager::REQUEST_UPDATE_2DWINDOWS);
}
