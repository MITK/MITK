template <typename PixelType, unsigned int Dimension> void _calculateItkPipelineFunction(mitk::Image* result, mitk::Image* mitksource, itk::ImageSource< mitk::ImageToItk<PixelType, Dimension> >* itkpipelinefunction)
{
	typedef mitk::ImageToItk<PixelType, Dimension> ImageToItkType;
	typedef itk::ImageSource< ImageToItkType > ImageFilterType;

	ImageToItkType::Pointer myItkImage = ImageToItkType::New();
		myItkImage->SetMitkImage(mitksource);
	
	ImageFilterType::Pointer itkpipeline = itkpipelinefunction(myItkImage.GetPointer()).GetPointer();
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
		typedef itk::ImageSource< ImageToItkType > ImageFilterType;                                \
                                                                                                   \
		ImageToItkType::Pointer myItkImage = ImageToItkType::New();                                \
			myItkImage->SetMitkImage(mitksource);                                                  \
                                                                                                   \
		ImageFilterType::Pointer itkpipeline =                                                     \
            itkpipelinefunction(myItkImage.GetPointer()).GetPointer();                             \
			itkpipeline->Update();                                                                 \
                                                                                                   \
		result->InitializeByItk(itkpipeline->GetOutput());                                              \
		result->SetVolume(itkpipeline->GetOutput()->GetBufferPointer());                           \
	}                                                              
//		_calculateItkPipelineFunction(result, mitkimage, itkpipeline<type, dimension>::New());     
   
#define ItkFunctionMultiplexer(result, mitkimage, itkfunction)                                     \
{                                                                                                  \
	const type_info& typeId=*mitkimage->GetType().GetTypeId();                                     \
	switch (mitkimage->GetDimension())                                                             \
{                                                                                                  \
	  case 2:                                                                                      \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, double, 2) else            \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, float, 2) else             \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, long, 2) else              \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned long, 2) else     \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, int, 2) else               \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned int, 2) else      \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, short, 2) else             \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned short, 2) else    \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, char, 2) else              \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned char,  2) else    \
		  break;                                                                                   \
	  case 3:                                                                                      \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, double, 3) else            \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, float, 3) else             \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, long, 3) else              \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned long, 3) else     \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, int, 3) else               \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned int, 3) else      \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, short, 3) else             \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned short, 3) else    \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, char, 3) else              \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned char,  3) else    \
		  break;                                                                                   \
	  case 4:                                                                                      \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, double, 4) else            \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, float, 4) else             \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, long, 4) else              \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned long, 4) else     \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, int, 4) else               \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned int, 4) else      \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, short, 4) else             \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned short, 4) else    \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, char, 4) else              \
		  _calculateItkPipelineFunction(result, mitkimage, itkfunction, unsigned char,  4) else    \
		  break;                                                                                   \
	  default: break;                                                                              \
}                                                                                                  \
}
