t
emplate <typename PixelType, unsigned int Dimension> void _calculateItkPipelineFunction(mitk::Image* result, mitk::Image* mitksource, itk::ImageSource< mitk::ImageToItk<PixelType, Dimension> >* itkpipelinefunction)
{
  typedef mitk::ImageToItk<PixelType, Dimension> ImageToItkType;
  typedef itk::Image<type, dimension> ImageType;                                  \
    typedef itk::ImageSource< ImageType > ImageFilterType;
  
  ImageToItkType::Pointer myImageToItkFilter = ImageToItkType::New();
    myImageToItkFilter->SetInput(mitksource);
    
    ImageFilterType::Pointer itkpipeline = itkpipelinefunction(myImageToItkFilter->GetOutput()).GetPointer();
    itkpipeline->Update();
    //	itkpipeline->SetInput(myItkImage);
    //	itkpipeline->Update();
    
    result->Initialize(itkpipeline->GetOutput());
    result->SetVolume(itkpipeline->GetOutput()->GetBufferPointer());
}

#define _calculateItkPipelineFunction(result, mitksource, itkpipelinefunction, type, dimension)    \
  if ( typeId == typeid(type) )                                                                  \
{                                                                                              \
    typedef mitk::ImageToItk<type, dimension> ImageToItkType;                                  \
    typedef itk::Image<type, dimension> ImageType;                                  \
    typedef itk::ImageSource<ImageType> ImageFilterType;                                \
    \
    ImageToItkType::Pointer myImageToItkFilter = ImageToItkType::New();                                \
    myImageToItkFilter ->SetInput(mitksource);                                                  \
    \
    ImageFilterType::Pointer itkpipeline =                                                     \
    itkpipelinefunction(myImageToItkFilter->GetOutput()).GetPointer();                             \
    itkpipeline->Update();                                                                 \
    \
    result->InitializeByItk(itkpipeline->GetOutput());                                              \
    result->SetVolume(itkpipeline->GetOutput()->GetBufferPointer());                           \
}                                                              
//		_calculateItkPipelineFunction(result, mitkimage, itkpipeline<type, dimension>::New());     

#define ItkFunctionMultiplexer(result, mitkSourceImage, itkfunction)                                     \
{                                                                                                  \
  const std::type_info& typeId=*mitkSourceImage->GetPixelType().GetTypeId();                                     \
  switch (mitkSourceImage->GetDimension())                                                             \
{                                                                                                  \
    case 2:                                                                                      \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, double, 2) else            \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, float, 2) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, long, 2) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned long, 2) else     \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, int, 2) else               \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned int, 2) else      \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, short, 2) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned short, 2) else    \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, char, 2) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned char,  2)         \
      break;                                                                                         \
    case 3:                                                                                          \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, double, 3) else            \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, float, 3) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, long, 3) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned long, 3) else     \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, int, 3) else               \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned int, 3) else      \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, short, 3) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned short, 3) else    \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, char, 3) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned char,  3)         \
      break;                                                                                         \
    case 4:                                                                                          \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, double, 4) else            \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, float, 4) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, long, 4) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned long, 4) else     \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, int, 4) else               \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned int, 4) else      \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, short, 4) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned short, 4) else    \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, char, 4) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned char,  4)         \
      break;                                                                                         \
    default: break;                                                                                  \
}                                                                                                    \
}
