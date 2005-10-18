#include "mitkManualSegmentationToSurfaceFilter.h"


mitk::ManualSegmentationToSurfaceFilter::ManualSegmentationToSurfaceFilter() 
  : m_MedianKernelSizeX(3), m_MedianKernelSizeY(3), m_MedianKernelSizeZ(3), m_StandardDeviation(1.5){};
mitk::ManualSegmentationToSurfaceFilter::~ManualSegmentationToSurfaceFilter(){};


void mitk::ManualSegmentationToSurfaceFilter::GenerateData() 
{
  mitk::Surface *surface = this->GetOutput();
  mitk::Image * image    =  (mitk::Image*)GetInput();
  mitk::Image::RegionType outputRegion = image->GetRequestedRegion();

  int tstart=outputRegion.GetIndex(3);
  int tmax=tstart+outputRegion.GetSize(3); //GetSize()==1 - will aber 0 haben, wenn nicht zeitaufgelöst

  ScalarType thresholdExpanded = this->m_Threshold;
  
  for( int t=tstart; t<tmax; t++ )
  {
    
    vtkImageData *vtkimage = image->GetVtkImageData(t);

    //Inkrement Referenzcounter Counter (hier: RC)
    vtkimage->Register(NULL);

    // Median -->smooth 3D 
    if(m_MedianFilter3D)
    {
      vtkImageMedian3D *median = vtkImageMedian3D::New();
      median->SetInput(vtkimage); //RC++
      vtkimage->Delete();//RC--
      median->SetKernelSize(m_MedianKernelSizeX,m_MedianKernelSizeY,m_MedianKernelSizeZ);//Std: 3x3x3
      median->ReleaseDataFlagOn();
      median->UpdateInformation();
      vtkimage = median->GetOutput(); 
    }

    //Interpolate image spacing 
    if(m_Interpolation)
    {
      vtkImageResample * imageresample = vtkImageResample::New();
      imageresample->SetInput(vtkimage);

      //Set Spacing Manual to 1mm in each direction (Original spacing is lost during image processing)      
      imageresample->SetAxisOutputSpacing(0, 1);
      imageresample->SetAxisOutputSpacing(1, 1);
      imageresample->SetAxisOutputSpacing(2, 1);
      vtkimage=imageresample->GetOutput();
      vtkimage->UpdateInformation();
    }

    if(m_UseStandardDeviation)//gauss
    {
      vtkImageThreshold* vtkimagethreshold = vtkImageThreshold::New();
      vtkimagethreshold->SetInput(vtkimage);//RC++
      vtkimage->Delete();//RC--
      vtkimagethreshold->SetInValue( 100 );
      vtkimagethreshold->SetOutValue( 0 );
      vtkimagethreshold->ThresholdByUpper( this->m_Threshold ); 
      thresholdExpanded = 49;
  
      vtkimagethreshold->SetOutputScalarTypeToUnsignedChar();
      vtkimagethreshold->ReleaseDataFlagOn();
      
      vtkImageGaussianSmooth *gaussian = vtkImageGaussianSmooth::New();
      gaussian->SetInput(vtkimagethreshold->GetOutput()); //RC ++
      vtkimagethreshold->Delete(); //RC--
      gaussian->SetDimensionality(3);
      gaussian->SetRadiusFactor(0.49);
      gaussian->SetStandardDeviation( m_StandardDeviation );
      gaussian->ReleaseDataFlagOn();
      gaussian->UpdateInformation();
      vtkimage = gaussian->GetOutput();
   }

    // Create sureface for t-Slice
    CreateSurface(t, vtkimage, surface, thresholdExpanded);
  }
};


/** 
* Set Kernel for Median3D.
*/
void mitk::ManualSegmentationToSurfaceFilter::SetMedianKernelSize(int x, int y, int z)
{
  m_MedianKernelSizeX = x;
  m_MedianKernelSizeY = y;
  m_MedianKernelSizeZ = z;
}

