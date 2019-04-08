#include <MitkSegmentationExports.h>

#include <itkAddImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkMacro.h>
#include <itkMultiplyImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>

#include "mitkSurfaceUtils.h"
#include <mitkImageAccessByItk.h>
#include <mitkSurface.h>

namespace mitk
{

class MITKSEGMENTATION_EXPORT ShowSegmentationAsAgtkSurface : public itk::ProcessObject
{
public:
  typedef ShowSegmentationAsAgtkSurface Self;
  typedef itk::ProcessObject            Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  itkNewMacro(Self);
  itkTypeMacro(ShowSegmentationAsAgtkSurface, itk::ProcessObject);

  static const unsigned int DIM = 3;
  typedef itk::Image<unsigned char, 3> InputImageType;
  typedef itk::Image<float, 3> FloatImage;

  enum class SurfaceSmoothingType
  {
    None,
    Laplacian,
    WindowedSync,
  };

  enum class SurfaceDecimationType
  {
    None,
    DecimatePro,
    QuadricDecimation,
  };

  struct SurfaceComputingParameters
  {
    bool isBlurImage = false;
    float blurSigma = .3f;
    float levelValue = 0.f;
    SurfaceSmoothingType smoothingType = SurfaceSmoothingType::WindowedSync;
    int smoothingIterations = 15;
    float smoothingRelaxation = .1f;
    SurfaceDecimationType decimationType = SurfaceDecimationType::DecimatePro;
    float reduction = .25f;
    bool isResampling = true;
    mitk::Vector3D spacing = 1.f;
    bool isFillHoles = true;
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

  void SetProgress(float progress)
  {
    m_CurrentProgress = progress;
  }

  float GetProgress()
  {
    return m_CurrentProgress;
  }

  float GetProgressWeight()
  {
    return m_ProgressWeight;
  }

protected:
  ShowSegmentationAsAgtkSurface();
  virtual ~ShowSegmentationAsAgtkSurface() {};

  SurfaceCreator::SurfaceCreationArgs m_SurfaceArgs;
  SurfaceComputingParameters m_FilterArgs;

  InputImageType::Pointer m_Input;
  vtkSmartPointer<vtkPolyData> m_Output;

  bool PreProcessing();
  void ComputeSurface();
  void PostProcessing();

  virtual void GenerateData() ITK_OVERRIDE;

private:
  ShowSegmentationAsAgtkSurface(const Self&);
  void operator=(const Self&);

  typename FloatImage::Pointer m_FloatTmpImage;

  float m_CurrentProgress; // Used only for vtk filters
  float m_ProgressWeight;
  itk::ProgressAccumulator::Pointer m_ProgressAccumulator;

  static void vtkOnProgress(vtkObject* caller, long unsigned int vtkNotUsed(eventId),
      void* vtkNotUsed(clientData), void* vtkNotUsed(callData));

  static void vtkOnEnd(vtkObject* caller, long unsigned int vtkNotUsed(eventId),
      void* clientData, void* vtkNotUsed(callData));

  vtkSmartPointer<vtkCallbackCommand> m_VtkProgressCallback;
  vtkSmartPointer<vtkCallbackCommand> m_VtkEndCallback;
};

}

