#ifndef MITKPLANARFIGUREMASKGENERATOR
#define MITKPLANARFIGUREMASKGENERATOR

#include <MitkImageStatisticsExports.h>
#include <itkImage.h>
#include <itkVTKImageExport.h>
#include <itkVTKImageImport.h>
#include <mitkImage.h>
#include <mitkMaskGenerator.h>
#include <mitkPlanarFigure.h>
#include <vtkImageExport.h>
#include <vtkImageImport.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
   * \class PlanarFigureMaskGenerator
   * \brief Derived from MaskGenerator. This class is used to convert a mitk::PlanarFigure into a binary image mask
   */
  class MITKIMAGESTATISTICS_EXPORT PlanarFigureMaskGenerator : public MaskGenerator
  {
  public:
    /** Standard Self typedef */
    typedef PlanarFigureMaskGenerator Self;
    typedef MaskGenerator Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

      /** Runtime information support. */
      itkTypeMacro(PlanarFigureMaskGenerator, MaskGenerator)

      /**
       * @brief GetMask Computes and returns the mask
       * @return mitk::Image::Pointer of the generated mask
       */
      mitk::Image::Pointer GetMask() override;

    void SetPlanarFigure(mitk::PlanarFigure::Pointer planarFigure);

    mitk::Image::ConstPointer GetReferenceImage() override;

    /**
     * @brief SetTimeStep is used to set the time step for which the mask is to be generated
     * @param timeStep
     */
    void SetTimeStep(unsigned int timeStep) override;

    itkGetConstMacro(PlanarFigureAxis, unsigned int);
    itkGetConstMacro(PlanarFigureSlice, unsigned int);

  protected:
    PlanarFigureMaskGenerator()
      : Superclass(),
        m_ReferenceImage(nullptr),
        m_PlanarFigureAxis(0),
        m_InternalMaskUpdateTime(0),
        m_PlanarFigureSlice(0)
    {
      m_InternalMask = mitk::Image::New();
    }

  private:
    void CalculateMask();

    template <typename TPixel, unsigned int VImageDimension>
    void InternalCalculateMaskFromPlanarFigure(const itk::Image<TPixel, VImageDimension> *image, unsigned int axis);

    template <typename TPixel, unsigned int VImageDimension>
    void InternalCalculateMaskFromOpenPlanarFigure(const itk::Image<TPixel, VImageDimension> *image, unsigned int axis);

    mitk::Image::ConstPointer extract2DImageSlice(unsigned int axis, unsigned int slice);

    bool GetPrincipalAxis(const BaseGeometry *geometry, Vector3D vector, unsigned int &axis);

    /** Connection from ITK to VTK */
    template <typename ITK_Exporter, typename VTK_Importer>
    void ConnectPipelines(ITK_Exporter exporter, vtkSmartPointer<VTK_Importer> importer)
    {
      importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());

      importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
      importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
      importer->SetSpacingCallback(exporter->GetSpacingCallback());
      importer->SetOriginCallback(exporter->GetOriginCallback());
      importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());

      importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());

      importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
      importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
      importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
      importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
      importer->SetCallbackUserData(exporter->GetCallbackUserData());
    }

    /** Connection from VTK to ITK */
    template <typename VTK_Exporter, typename ITK_Importer>
    void ConnectPipelines(vtkSmartPointer<VTK_Exporter> exporter, ITK_Importer importer)
    {
      importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());

      importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
      importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
      importer->SetSpacingCallback(exporter->GetSpacingCallback());
      importer->SetOriginCallback(exporter->GetOriginCallback());
      importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());

      importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());

      importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
      importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
      importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
      importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
      importer->SetCallbackUserData(exporter->GetCallbackUserData());
    }

    bool IsUpdateRequired() const;

    mitk::PlanarFigure::Pointer m_PlanarFigure;
    itk::Image<unsigned short, 2>::Pointer m_InternalITKImageMask2D;
    mitk::Image::ConstPointer m_InternalTimeSliceImage;
    mitk::Image::ConstPointer m_ReferenceImage;
    unsigned int m_PlanarFigureAxis;
    unsigned long m_InternalMaskUpdateTime;
    unsigned int m_PlanarFigureSlice;
  };
} // namespace mitk

#endif // MITKPLANARFIGUREMASKGENERATOR
