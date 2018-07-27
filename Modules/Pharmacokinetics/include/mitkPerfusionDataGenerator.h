#ifndef PERFUSIONDATAGENERATOR_H
#define PERFUSIONDATAGENERATOR_H


#include "MitkPharmacokineticsExports.h"
#include "mitkModelBase.h"
#include "mitkModelDataGenerationFunctor.h"
#include "mitkSimpleFunctorPolicy.h"
#include "mitkImage.h"


namespace mitk {

    class MITKPHARMACOKINETICS_EXPORT PerfusionDataGenerator: public ::itk::Object
    {
    public:

        mitkClassMacroItkParent(PerfusionDataGenerator, itk::Object);
        itkFactorylessNewMacro(Self);

        typedef mitk::Image::Pointer ParameterImageType;
        typedef std::vector<std::string> ParameterNamesType;
        typedef unsigned int ParametersIndexType;
        typedef std::vector<ParameterImageType> ParameterVectorType;
        typedef std::map<ParametersIndexType, ParameterImageType> ParameterMapType;


        typedef ModelDataGenerationFunctor FunctorType;
        typedef mitk::Image::Pointer ResultImageType;
        typedef mitk::Image::Pointer MaskType;

        typedef mitk::ModelBase::TimeGridType GridType;

        itkSetObjectMacro(Functor, FunctorType);
        itkGetObjectMacro(Functor, FunctorType);

        void SetParameterInputImage(const ParametersIndexType index, ParameterImageType inputParameterImage);

        ResultImageType GetGeneratedImage();
        void Generate();



    protected:
        PerfusionDataGenerator()
        {};
        ~PerfusionDataGenerator()
        {};

        template <typename TPixel, unsigned int VDim>
        void DoGenerateData(itk::Image<TPixel, VDim>* image);

        template <typename TPixel, unsigned int VDim>
        void DoPrepareMask(itk::Image<TPixel, VDim>* image);

    private:
        ParameterMapType m_ParameterInputMap;
        ParameterVectorType  m_InputParameterImages;

        void SortParameterImages();


        MaskType m_Mask;

        typedef itk::Image<unsigned char, 3> InternalMaskType;
        InternalMaskType::Pointer m_InternalMask;

        ResultImageType m_ResultImage;
          FunctorType::Pointer m_Functor;

    };
}


#endif // MITKPHARMACOKINETICSDATAGENERATOR_H
