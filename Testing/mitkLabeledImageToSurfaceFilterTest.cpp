#include "mitkLabeledImageToSurfaceFilter.h"
#include <itksys/SystemTools.hxx>
#include "mitkDataTreeNodeFactory.h"
#include "mitkReferenceCountWatcher.h"

#include <cmath>

bool equals(const mitk::ScalarType& val1, const mitk::ScalarType& val2, mitk::ScalarType epsilon = mitk::eps )
{
  return ( std::fabs(val1 - val2) <= epsilon );
}

int mitkLabeledImageToSurfaceFilterTest(int argc, char* argv[])
{
  if(argc<2)
  {
    std::cout<<"no path to testing specified [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  
  std::string fileIn = argv[1];
  std::cout<<"Eingabe Datei: "<<fileIn<<std::endl;
  mitk::Image::Pointer image = NULL;
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
  try
  {
    std::cout << "Loading file: ";
    factory->SetFileName( fileIn.c_str() );
    factory->Update();

    if(factory->GetNumberOfOutputs()<1)
    {
      std::cout<<"file could not be loaded [FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    mitk::DataTreeNode::Pointer node = factory->GetOutput( 0 );
    image = dynamic_cast<mitk::Image*>(node->GetData());
    if(image.IsNull())
    {
      std::cout<<"file not an image - test will not be applied [PASSED]"<<std::endl;
      std::cout<<"[TEST DONE]"<<std::endl;
      return EXIT_SUCCESS;
    }
  }
  catch ( itk::ExceptionObject & ex )
  {
    std::cout << "Exception: " << ex << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }


  std::cout << "Testing instantiation: " ;
  mitk::LabeledImageToSurfaceFilter::Pointer filter = mitk::LabeledImageToSurfaceFilter::New();
  if (filter.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  }

  std::cout << "Create surface with default settings: ";
  filter->SetInput(image);
  filter->Update();

  if ( filter->GetNumberOfOutputs() != 1 )
  {
     std::cout<<"Wrong number of outputs, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput() == NULL )
  {
     std::cout<<"Output is NULL, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput()->GetVtkPolyData() == NULL )
  {
     std::cout<<"PolyData of surface is NULL, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  
  std::cout << "Testing index to label conversion: ";
  if ( filter->GetLabelForNthOutput( 0 ) != 257 )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }  
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  
  std::cout << "Testing volume for label calculation: ";
  if ( ! equals ( filter->GetVolumeForLabel( 257 ) , 14.328 ) )
  {
     std::cout<<filter->GetVolumeForLabel( 257 )<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }  
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
    
  std::cout << "Testing volume for index calculation: ";
  if ( ! equals ( filter->GetVolumeForNthOutput( 0 ) , 14.328 ) )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }  
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  
 
  std::cout << "Create surface using optimised settings: ";
  filter->GenerateAllLabelsOn();
  filter->SetGaussianStandardDeviation( 1.5 ); 
  filter->SetSmooth(true); // smooth wireframe
  filter->SetDecimate( mitk::ImageToSurfaceFilter::DecimatePro );
  filter->SetTargetReduction( 0.8 );
  if( filter->GetNumberOfOutputs() != 1 )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  
  std::cout << "Create surface for label 257: ";
  filter->GenerateAllLabelsOff();
  filter->SetLabel(257);
  filter->Update();
  if ( filter->GetNumberOfOutputs() != 1 )
  {
     std::cout<<"Wrong number of outputs, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput() == NULL )
  {
     std::cout<<"Output is NULL, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput()->GetVtkPolyData() == NULL )
  {
     std::cout<<"PolyData of surface is NULL, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  
  std::cout << "Testing volume for label calculation: ";
  if ( ! equals ( filter->GetVolumeForLabel( 257 ) , 14.328 ) )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }  
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
    
  std::cout << "Testing volume for index calculation: ";
  if ( ! equals ( filter->GetVolumeForNthOutput( 0 ) , 14.328 ) )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }  
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  
  std::cout << "Create surface for multiple labels: ";
  filter->GenerateAllLabelsOn();
  filter->SetBackgroundLabel(32000);
  filter->Update();
  if ( filter->GetNumberOfOutputs() != 2 )
  {
     std::cout<<"Wrong number of outputs, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput(0) == NULL )
  {
     std::cout<<"Output 0 is NULL, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput(0)->GetVtkPolyData() == NULL )
  {
     std::cout<<"PolyData of output 0 is NULL, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput(1) == NULL )
  {
     std::cout<<"Output 1 is NULL, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput(1)->GetVtkPolyData() == NULL )
  {
     std::cout<<"PolyData of output 1 is NULL, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  
  std::cout << "Testing volume for label calculation: ";
  if ( ! equals ( filter->GetVolumeForLabel( 257 ) , 14.328 ) )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( ! equals ( filter->GetVolumeForLabel( 0 ), 12.672 ) )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
    
  std::cout << "Testing volume for index calculation: ";
  if ( ! equals( filter->GetVolumeForNthOutput( 1 ), 14.328 ) )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }  
  else if ( ! equals ( filter->GetVolumeForNthOutput( 0 ), 12.672 ) )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }

  mitk::ReferenceCountWatcher::Pointer outputSurface1Watcher = new mitk::ReferenceCountWatcher(filter->GetOutput(1), "outputSurface1");
  mitk::ReferenceCountWatcher::Pointer filterWatcher = new mitk::ReferenceCountWatcher(filter, "filter");
  
  std::cout << "Create surface for background (label 0): " << std::flush;
  filter->GenerateAllLabelsOff();
  filter->SetLabel(0);
  filter->SetBackgroundLabel(257);
  //mitk::Surface::Pointer surface = filter->GetOutput(1);
  //std::cout<< surface->GetReferenceCount() << std::endl;
  filter->Update();
  //surface = NULL;
  
  if ( filter->GetNumberOfOutputs() != 1 )
  {
     std::cout<<"Wrong number of outputs, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput() == NULL )
  {
     std::cout<<"Output is NULL, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput()->GetVtkPolyData() == NULL )
  {
     std::cout<<"PolyData of surface is NULL, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  std::cout << "Testing reference count correctness of old output 1: " << std::flush;
  if ( outputSurface1Watcher->GetReferenceCount() != 0 )
  {
     std::cout<<"outputSurface1Watcher->GetReferenceCount()=="
       << outputSurface1Watcher->GetReferenceCount()
       << "!=0, [FAILED]" << std::endl;
     return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  std::cout << "Testing reference count correctness of filter: " << std::flush;
  if ( filterWatcher->GetReferenceCount() != 2 )
  {
     std::cout<<"filterWatcher->GetReferenceCount()=="
       << outputSurface1Watcher->GetReferenceCount()
       << "!=2, [FAILED]" << std::endl;
     return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing index to label conversion: ";
  if ( filter->GetLabelForNthOutput( 0 ) != 0 )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }  
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  
  std::cout << "Testing volume for label calculation: ";
  if ( ! equals ( filter->GetVolumeForLabel( filter->GetLabel() ) , 12.672 ) )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }  
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
    
  std::cout << "Testing volume for index calculation: ";
  if ( ! equals( filter->GetVolumeForNthOutput( 0 ), 12.672 ) )
  {
     std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }  
  else 
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  
  std::cout << "Create surface for invalid label: ";
  filter->GenerateAllLabelsOff();
  filter->SetLabel(1);
  filter->Update();
  if ( filter->GetNumberOfOutputs() != 1 )
  {
     std::cout<<"Number of outputs != 1, [FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else if ( filter->GetOutput()->GetVtkPolyData()->GetNumberOfPoints() != 0 )
  {
    std::cout<<"PolyData is not empty ("<<filter->GetOutput()->GetVtkPolyData()->GetNumberOfPoints()<<"), [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else
  {
     std::cout<<"[PASSED]"<<std::endl;
  }
  
  

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
  
}
