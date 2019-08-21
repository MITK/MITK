#include "mitkShowSegmentationAsElasticNetSurface.h"
#include <itkImageRegionIteratorWithIndex.h>

#include <vtkPolyDataNormals.h>

namespace mitk
{

ShowSegmentationAsElasticNetSurface::ShowSegmentationAsElasticNetSurface()
{
  m_ProgressAccumulator = itk::ProgressAccumulator::New();
}

void ShowSegmentationAsElasticNetSurface::GenerateData()
{
  createSurfaceCubes();
  createNodes();
  linkNodes();

  Vector3D spacing = m_Input->GetSpacing();
  double length = spacing.GetNorm() * (1 - m_FilterArgs.elasticNetRelaxation);
  for (int i = 0; i < m_FilterArgs.elasticNetIterations; i++) {
    m_LastMaxRelaxation = 0.;
    relaxNodes();
    if (m_FilterArgs.elasticNetRelaxation > 0. && m_LastMaxRelaxation < length) {
      break;
    }
  }

  m_Output = triangulateNodes();
}

short ShowSegmentationAsElasticNetSurface::getPixel(int x, int y, int z, InputImageType::RegionType region)
{
  InputImageType::IndexType index;
  index[0] = x;
  index[1] = y;
  index[2] = z;

  if (!region.IsInside(index)) {
    return -1;
  }

  return m_Input->GetPixel(index);
}

void ShowSegmentationAsElasticNetSurface::createSurfaceCubes()
{
  InputImageType::RegionType region = m_Input->GetLargestPossibleRegion();
  InputImageType::SizeType dim = region.GetSize();
  m_SurfaceCubes = std::make_unique<SurfaceCubeData>(dim[0] + 1, dim[1] + 1, dim[2] + 1);

  char neighbours[8];
  for (int x = 0; x <= dim[0]; x++) {
    for (int y = 0; y <= dim[1]; y++) {
      for (int z = 0; z <= dim[2]; z++) {
        neighbours[0] = getPixel(x - 1, y - 1, z - 1, region);
        neighbours[1] = getPixel(x - 1, y    , z - 1, region);
        neighbours[2] = getPixel(x,     y - 1, z - 1, region);
        neighbours[3] = getPixel(x,     y,     z - 1, region);
        neighbours[4] = getPixel(x - 1, y - 1, z,     region);
        neighbours[5] = getPixel(x - 1, y    , z,     region);
        neighbours[6] = getPixel(x,     y - 1, z,     region);
        neighbours[7] = getPixel(x,     y,     z,     region);

        bool hasZeroes       = false;
        bool hasOutside      = false;
        bool hasSegmentation = false;

        for (int i = 0; i < 8; i++) {
          hasZeroes       |= neighbours[i] == 0;
          hasOutside      |= neighbours[i] < 0;
          hasSegmentation |= neighbours[i] > 0;
        }
        if (hasSegmentation && (hasZeroes || hasOutside)) {
          m_SurfaceCubes->set(x, y, z, std::make_shared<SurfaceCube>());
        }
      }
    }
  }
}

void ShowSegmentationAsElasticNetSurface::createNodes()
{
  m_SurfaceNodes.clear();

  Vector3D offset = m_Input->GetSpacing();
  offset[0] /= 2.;
  offset[1] /= 2.;
  offset[2] /= 2.;

  for (int x = 0; x < m_SurfaceCubes->getXSize(); x++) {
    for (int y = 0; y < m_SurfaceCubes->getYSize(); y++) {
      for (int z = 0; z < m_SurfaceCubes->getZSize(); z++) {
        std::shared_ptr<SurfaceCube> cube = m_SurfaceCubes->get(x, y, z);
        if (cube == nullptr) {
          continue;
        }
        std::shared_ptr<SurfaceNode> node = std::make_shared<SurfaceNode>();
        node->parent = cube;
        cube->child = node;

        InputImageType::IndexType index;
        index[0] = x;
        index[1] = y;
        index[2] = z;
        Point3D point;
        m_Input->TransformIndexToPhysicalPoint(index, point);
        point -= offset;
        node->pos = point;

        node->index = m_SurfaceNodes.size();
        m_SurfaceNodes.push_back(node);
      }
    }
  }
}

bool ShowSegmentationAsElasticNetSurface::isEdge(InputImageType::RegionType region, int indX, int indY, int indZ, int alongAxis)
{
  bool edge = false;

  short segmentationCount = 0;

  short pixels[4];
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      int x = indX;
      int y = indY;
      int z = indZ;

      if (alongAxis == 0) {
        y += -1 * i;
        z += -1 * j;
      } else if (alongAxis == 1) {
        x += -1 * i;
        z += -1 * j;
      } else {
        x += -1 * i;
        y += -1 * j;
      }

      short pixel = getPixel(x, y, z, region);
      segmentationCount += pixel > 0;
      pixels[i*2+j] = pixel;
    }
  }

  edge = segmentationCount > 0 && segmentationCount < 4;

  return edge;
}

void ShowSegmentationAsElasticNetSurface::linkNodes()
{
  InputImageType::RegionType region = m_Input->GetLargestPossibleRegion();

  for (const auto& node : m_SurfaceNodes) {
    InputImageType::IndexType index = node->parent->pos;
    // Left
    if (index[0] > 0) {
      std::shared_ptr<SurfaceCube> cube = m_SurfaceCubes->get(index[0] - 1, index[1], index[2]);
      if (cube != nullptr) {
        if (isEdge(region, index[0] - 1, index[1], index[2], 0)) {
          node->neighbours[0] = cube->child;
        }
      }
    }
    // Right
    if (index[0] < m_SurfaceCubes->getXSize() - 1) {
      std::shared_ptr<SurfaceCube> cube = m_SurfaceCubes->get(index[0] + 1, index[1], index[2]);
      if (cube != nullptr) {
        if (isEdge(region, index[0], index[1], index[2], 0)) {
          node->neighbours[1] = cube->child;
        }
      }
    }
    // Bottom
    if (index[1] > 0) {
      std::shared_ptr<SurfaceCube> cube = m_SurfaceCubes->get(index[0], index[1] - 1, index[2]);
      if (cube != nullptr) {
        if (isEdge(region, index[0], index[1] - 1, index[2], 1)) {
          node->neighbours[2] = cube->child;
        }
      }
    }
    // Top
    if (index[1] < m_SurfaceCubes->getYSize() - 1) {
      std::shared_ptr<SurfaceCube> cube = m_SurfaceCubes->get(index[0], index[1] + 1, index[2]);
      if (cube != nullptr) {
        if (isEdge(region, index[0], index[1], index[2], 1)) {
          node->neighbours[3] = cube->child;
        }
      }
    }
    // Back
    if (index[2] > 0) {
      std::shared_ptr<SurfaceCube> cube = m_SurfaceCubes->get(index[0], index[1], index[2] - 1);
      if (cube != nullptr) {
        if (isEdge(region, index[0], index[1], index[2] - 1, 2)) {
          node->neighbours[4] = cube->child;
        }
      }
    }
    // Front
    if (index[2] < m_SurfaceCubes->getZSize() - 1) {
      std::shared_ptr<SurfaceCube> cube = m_SurfaceCubes->get(index[0], index[1], index[2] + 1);
      if (cube != nullptr) {
        if (isEdge(region, index[0], index[1], index[2], 2)) {
          node->neighbours[5] = cube->child;
        }
      }
    }
  }
}

Point3D ShowSegmentationAsElasticNetSurface::rayExit(Point3D origin, Vector3D direction, Point3D min, Point3D max)
{
  Vector3D tmax;
  Vector3D div;
  div[0] = 1. / direction[0];
  div[1] = 1. / direction[1];
  div[2] = 1. / direction[2];
  Point3D aabb[2];
  aabb[0] = min;
  aabb[1] = max;

  tmax[0] = (aabb[div[0] > 0][0] - origin[0]) * div[0];
  tmax[1] = (aabb[div[1] > 0][1] - origin[1]) * div[1];
  tmax[2] = (aabb[div[2] > 0][2] - origin[2]) * div[2];

  return origin + std::min(std::min(tmax[0], tmax[1]), tmax[2]) * direction;
}

Point3D ShowSegmentationAsElasticNetSurface::offsetPoint(Point3D point, Vector3D direction, Point3D min, Point3D max)
{
  Point3D result = point;
  result += direction;

  bool insideCube = true;
  for (int i = 0; i < 3; i++) {
    if (result[i] < min[i] || result[i] > max[i]) {
      insideCube = false;
      break;
    }
  }

  if (insideCube) {
    m_LastMaxRelaxation = std::max(m_LastMaxRelaxation, direction.GetNorm());
    return result;
  }

  // Calculate intersection with cube
  Vector3D normDir = direction;
  normDir.Normalize();
  result = rayExit(point, normDir, min, max);

  m_LastMaxRelaxation = std::max(m_LastMaxRelaxation, (result - point).GetNorm());
  return result;
}

void ShowSegmentationAsElasticNetSurface::relaxNodes()
{
  Vector3D spacing = m_Input->GetSpacing();

  Vector3D offset = m_Input->GetSpacing();
  offset[0] /= 2.;
  offset[1] /= 2.;
  offset[2] /= 2.;

  // Calculate relaxation force for each node
  for (const auto& node : m_SurfaceNodes) {
    Point3D targetPos;
    targetPos.Fill(0.);
    short neighboursCount = 0;
    for (std::shared_ptr<SurfaceNode> neighbour : node->neighbours) {
      if (neighbour != nullptr) {
        targetPos[0] += neighbour->pos[0];
        targetPos[1] += neighbour->pos[1];
        targetPos[2] += neighbour->pos[2];
        neighboursCount++;
      }
    }
    if (neighboursCount == 0) {
      continue;
    }
    targetPos[0] /= neighboursCount;
    targetPos[1] /= neighboursCount;
    targetPos[2] /= neighboursCount;
    Vector3D force = targetPos - node->pos;
    node->relaxationForce = force;
  }

  // Apply relaxation force
  for (const auto& node : m_SurfaceNodes) {
    // Clamp inside surface cube
    Point3D cubeCenter;
    m_Input->TransformIndexToPhysicalPoint(node->parent->pos, cubeCenter);
    cubeCenter -= offset;
    Point3D min, max;
    for (int i = 0; i < 3; i++) {
      min[i] = cubeCenter[i] - abs(spacing[i]);
      max[i] = cubeCenter[i] + abs(spacing[i]);
    }

    node->pos = offsetPoint(node->pos, node->relaxationForce, min, max);
  }
}

void ShowSegmentationAsElasticNetSurface::checkAndCreateTriangle(vtkSmartPointer<vtkCellArray> triangles, std::shared_ptr<SurfaceNode> node, int neighbourA, int neighbourB)
{
  InputImageType::RegionType region = m_Input->GetLargestPossibleRegion();
  // Check nodes exists
  if (node->neighbours[neighbourA] == nullptr || node->neighbours[neighbourB] == nullptr) {
    return;
  }

  // Check triangle in quad neighbourhood
  if (node->neighbours[neighbourA]->neighbours[neighbourB] == nullptr ||
      node->neighbours[neighbourB]->neighbours[neighbourA] == nullptr) {
    return;
  }

  // Check triangle on surface
  std::shared_ptr<SurfaceNode> nodes[4];
  nodes[0] = node;
  nodes[1] = node->neighbours[neighbourA];
  nodes[2] = node->neighbours[neighbourB];
  nodes[3] = node->neighbours[neighbourA]->neighbours[neighbourB];

  int mainAxis = 0;
  for (int i = 1; i < 3; i++) {
    if (nodes[0]->parent->pos[i] == nodes[1]->parent->pos[i] &&
        nodes[0]->parent->pos[i] == nodes[2]->parent->pos[i] &&
        nodes[0]->parent->pos[i] == nodes[3]->parent->pos[i]) {
      mainAxis = i;
      break;
    }
  }

  InputImageType::IndexType maxIndex;
  for (int i = 0; i < 3; i++) {
    maxIndex[i] = nodes[0]->parent->pos[i];
    for (int j = 1; j < 4; j++) {
      maxIndex[i] = std::max(maxIndex[i], nodes[j]->parent->pos[i]);
    }
  }

  InputImageType::IndexType targetIndex;
  targetIndex[0] = maxIndex[0] - 1;
  targetIndex[1] = maxIndex[1] - 1;
  targetIndex[2] = maxIndex[2] - 1;
  short pixel0 = getPixel(targetIndex[0], targetIndex[1], targetIndex[2], region);
  targetIndex[mainAxis] += 1;
  short pixel1 = getPixel(targetIndex[0], targetIndex[1], targetIndex[2], region);

  if (pixel0 == pixel1) {
    return;
  }

  // Generate triangle
  vtkIdType triangle[3];
  triangle[0] = node->index;
  triangle[1] = node->neighbours[neighbourA]->index;
  triangle[2] = node->neighbours[neighbourB]->index;
  triangles->InsertNextCell(3, triangle);
}

vtkSmartPointer<vtkPolyData> ShowSegmentationAsElasticNetSurface::triangulateNodes()
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();

  for (const auto& node : m_SurfaceNodes) {
    // Insert node point
    points->InsertNextPoint(node->pos[0], node->pos[1], node->pos[2]);

    // Create neighbours triangles
    // Only consider 6 triangles out of 12 to not create overlapping triangles on next node
    // Center -> Left -> Back
    checkAndCreateTriangle(triangles, node, 0, 4);
    // Center -> Right -> Front
    checkAndCreateTriangle(triangles, node, 1, 5);
    // Center -> Top -> Back
    checkAndCreateTriangle(triangles, node, 3, 4);
    // Center -> Bottom -> Front
    checkAndCreateTriangle(triangles, node, 2, 5);
    // Center -> Right -> Top
    checkAndCreateTriangle(triangles, node, 1, 3);
    // Center -> Left -> Bottom
    checkAndCreateTriangle(triangles, node, 0, 2);
  }

  vtkSmartPointer<vtkPolyData> data = vtkSmartPointer<vtkPolyData>::New();
  data->SetPoints(points);
  data->SetPolys(triangles);

  vtkSmartPointer<vtkPolyDataNormals> normalsFilter = vtkSmartPointer<vtkPolyDataNormals>::New();
  normalsFilter->SetInputData(data);
  normalsFilter->SetFeatureAngle(60.0);
  normalsFilter->Update();

  data = normalsFilter->GetOutput();

  return data;
}
}

