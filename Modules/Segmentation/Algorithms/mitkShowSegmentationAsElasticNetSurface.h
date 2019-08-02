#include <MitkSegmentationExports.h>

#include <mitkSurface.h>

#include "mitkSurfaceUtils.h"

namespace mitk
{

class MITKSEGMENTATION_EXPORT ShowSegmentationAsElasticNetSurface : public itk::ProcessObject
{
public:
  typedef ShowSegmentationAsElasticNetSurface Self;
  typedef itk::ProcessObject                  Superclass;
  typedef itk::SmartPointer<Self>             Pointer;
  typedef itk::SmartPointer<const Self>       ConstPointer;

  itkNewMacro(Self);
  itkTypeMacro(ShowSegmentationAsElasticNetSurface, itk::ProcessObject);

  static const unsigned int DIM = 3;
  typedef itk::Image<unsigned char, 3> InputImageType;

  enum class BridgeTipType
  {
    Flat,
    Pointy,
    Bridge,
  };

  struct SurfaceComputingParameters
  {
    int elasticNetIterations = 10;
    BridgeTipType bridgeTip = BridgeTipType::Pointy; // Doesn't work at the moment
  };

  virtual void Update() override { this->UpdateOutputData(nullptr); }

  void setArgs(SurfaceCreator::SurfaceCreationArgs creationArgs, SurfaceComputingParameters filterArgs)
  {
    m_SurfaceArgs = creationArgs;
    m_FilterArgs = filterArgs;
  }

  itkGetObjectMacro(Output, vtkPolyData);

  void SetInput(InputImageType::Pointer input)
  {
    m_Input = input;
  }

protected:
  ShowSegmentationAsElasticNetSurface();
  virtual ~ShowSegmentationAsElasticNetSurface() {};

  SurfaceCreator::SurfaceCreationArgs m_SurfaceArgs;
  SurfaceComputingParameters m_FilterArgs;

  InputImageType::Pointer m_Input;
  vtkSmartPointer<vtkPolyData> m_Output;

  virtual void GenerateData() override;

private:
  ShowSegmentationAsElasticNetSurface(const Self&);
  void operator=(const Self&);

  itk::ProgressAccumulator::Pointer m_ProgressAccumulator;

  void createSurfaceCubes();
  void createNodes();
  void linkNodes();
  void relaxNodes();
  vtkSmartPointer<vtkPolyData> triangulateNodes();

  struct SurfaceNode;
  struct SurfaceCube {
    InputImageType::IndexType pos;
    std::shared_ptr<SurfaceNode> child;
  };

  struct SurfaceNode {
    std::shared_ptr<SurfaceCube> parent;
    Point3D pos; // World position
    std::shared_ptr<SurfaceNode> neighbours[6]; // Order: Left, Right, Bottom, Top, Back, Front
    Vector3D relaxationForce;
    int index;
  };

  struct SurfaceCubeData {
    public:
      SurfaceCubeData(int x, int y, int z)
      {
        xSize = x;
        ySize = y;
        zSize = z;
        data.resize(x * y * z);
      }

      void set(int x, int y, int z, std::shared_ptr<SurfaceCube> cube)
      {
        data[x * ySize * zSize + y * zSize + z] = cube;
        InputImageType::IndexType pos;
        pos[0] = x; pos[1] = y; pos[2] = z;
        cube->pos = pos;
      }
      std::shared_ptr<SurfaceCube> get(int x, int y, int z)
      {
        return data[x * ySize * zSize + y * zSize + z];
      }

      int getXSize() { return xSize; }
      int getYSize() { return ySize; }
      int getZSize() { return zSize; }
    private:
      int xSize, ySize, zSize;
      std::vector<std::shared_ptr<SurfaceCube>> data;
  };

  std::unique_ptr<SurfaceCubeData> m_SurfaceCubes;
  std::vector<std::shared_ptr<SurfaceNode>> m_SurfaceNodes;

  short getPixel(int x, int y, int z, InputImageType::RegionType region);
  void checkAndCreateTriangle(vtkSmartPointer<vtkCellArray> triangles, std::shared_ptr<SurfaceNode> node, int neighbourA, int neighbourB);
};

}

