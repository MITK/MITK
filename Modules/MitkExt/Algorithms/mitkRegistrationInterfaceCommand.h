namespace mitk { 

  template <class TRegistration, class TPixel>  
  class RegistrationInterfaceCommand : public itk::Command 
  {
	
    public:
      typedef RegistrationInterfaceCommand   Self;
      typedef itk::Command                   Superclass;
      typedef itk::SmartPointer<Self>        Pointer;
      itkNewMacro( Self ); 
    protected:
    RegistrationInterfaceCommand() 
    {
      m_UseMask = false;
    }
  	
    public:
    //typedef TRegistration                              RegistrationType;


    typedef itk::MultiResolutionImageRegistrationMethod< itk::Image<float, 3>
                                                       , itk::Image<float, 3> > RegistrationType;


    typedef RegistrationType *                          RegistrationPointer;    
    typedef itk::SingleValuedNonLinearOptimizer         OptimizerType;
    typedef OptimizerType *                             OptimizerPointer;
    typedef itk::ImageMaskSpatialObject< 3 >            MaskType;
       

    mitk::RigidRegistrationObserver::Pointer observer; 
    bool m_UseMask;  
    std::vector<std::string> m_Presets;
    MaskType::Pointer m_BrainMask;
  	
		void Execute(itk::Object * object, const itk::EventObject & event)
		{
  	  if( !(itk::IterationEvent().CheckEvent( &event )) )
			{
			  return;
			}
  	
      RegistrationPointer registration = dynamic_cast<RegistrationPointer>( object );
  	
      
      /*OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >( 
												     registration->GetOptimizer() );*/

      
      std::cout << "-------------------------------------" << std::endl;
      std::cout << "MultiResolution Level : "
                << registration->GetCurrentLevel()  << std::endl << std::endl;
      

      if ( registration->GetCurrentLevel() == 0 )
      {
        // Nothing needs to be changed in the first step
      }
      else
      {
        // Load presets and make a new optimizer if that succeeds
        mitk::RigidRegistrationTestPreset *preset = new mitk::RigidRegistrationTestPreset();

        if( preset->LoadPreset() )
        {          
        
          mitk::OptimizerParameters::Pointer optimizerParameters = mitk::OptimizerParameters::New();
          itk::Array<double> optimizerValues = preset->getOptimizerValues(m_Presets[ registration->GetCurrentLevel() ]);

          optimizerParameters->SetOptimizer(optimizerValues[0]);
          optimizerParameters->SetMaximize(optimizerValues[1]); //should be when used with maximize mutual information for example

          if(optimizerValues[0] == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
          {        
            optimizerParameters->SetLearningRateGradientDescent(optimizerValues[2]);
            optimizerParameters->SetNumberOfIterationsGradientDescent(optimizerValues[3]);
          }

          if(optimizerValues[0] == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
          {
           cout << "use regularstepgradientdescent" << endl;
           optimizerParameters->SetGradientMagnitudeToleranceRegularStepGradientDescent(optimizerValues[2]);
           optimizerParameters->SetMinimumStepLengthRegularStepGradientDescent(optimizerValues[3]);
           optimizerParameters->SetMaximumStepLengthRegularStepGradientDescent(optimizerValues[4]);
           optimizerParameters->SetRelaxationFactorRegularStepGradientDescent(optimizerValues[5]);
           optimizerParameters->SetNumberOfIterationsRegularStepGradientDescent(optimizerValues[6]);       
          }

          
          // Typedef for the OptimizerFactory and initialisation of the optimizer using m_OptimizerParameters
          typename OptimizerFactory::Pointer optFac = OptimizerFactory::New();
          optFac->SetOptimizerParameters(optimizerParameters);
          optFac->SetNumberOfTransformParameters(registration->GetTransform()->GetNumberOfParameters());
          typename OptimizerType::Pointer optimizer = optFac->GetOptimizer();

          if(observer.IsNotNull())
          {
            optimizer->AddObserver(itk::AnyEvent(), observer);
          }

         
          itk::Array<double> transformValues = preset->getTransformValues(m_Presets[ registration->GetCurrentLevel() ]);
          
          itk::Array<double> scales;
          if(transformValues[0] == mitk::TransformParameters::AFFINETRANSFORM) scales.SetSize(12);
          mitk::TransformParameters::Pointer transformParameters = mitk::TransformParameters::New();
          transformParameters->SetTransform(transformValues[0]);          

          for(unsigned int i = 0; i < scales.size(); i++)
          {
            scales[i] = transformValues[i+2];
            std::cout << "scale " << i << ": " << scales[i] << std::endl;
          }

          transformParameters->SetScales(scales);      
          //transformParameters->SetTransformInitializerOn(false);         
         
          
          // Use Scales      
          if(transformValues[1] == 1)
          {
            transformParameters->SetUseOptimizerScales(true);              
          } 

          if (transformParameters->GetUseOptimizerScales())
          {
            itk::Array<double> optimizerScales = transformParameters->GetScales();
            typename OptimizerType::ScalesType scales( registration->GetTransform()->GetNumberOfParameters() );
            for (unsigned int i = 0; i < scales.Size(); i++)
            {
              scales[i] = optimizerScales[i];
            }
            optimizer->SetScales( scales );           
          }
          
          
          registration->SetOptimizer(optimizer);
      

        }      

       
      }
         
      registration->Print(std::cout,0);
      std::cout << std::endl;
      std::cout << "METRIC" << std::endl;
      registration->GetMetric()->Print(std::cout,0);
      std::cout << std::endl;
      std::cout << "OPTIMIZER" << std::endl;
      registration->GetOptimizer()->Print(std::cout,0);
      std::cout << std::endl;
      std::cout << "TRANSFORM" << std::endl;
      registration->GetTransform()->Print(std::cout,0);
    }
	
    void Execute(const itk::Object * , const itk::EventObject & )
      { return; }

  };

}
