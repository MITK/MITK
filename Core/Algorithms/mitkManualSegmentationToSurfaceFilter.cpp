#include "mitkManualSegmentationToSurfaceFilter.h"
//#include <iostream>

mitk::ManualSegmentationToSurfaceFilter::ManualSegmentationToSurfaceFilter() 
  : m_MedianKernelSizeX(3), m_MedianKernelSizeY(3), m_MedianKernelSizeZ(3), m_StdDeviation(1.5)
{}
mitk::ManualSegmentationToSurfaceFilter::~ManualSegmentationToSurfaceFilter() 
{}


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

    // Median -->smooth
    if(m_UseMedianFilter3D)
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
    if(m_SetInterpolation)
    {
      vtkImageResample * imageresample = vtkImageResample::New();
      imageresample->SetInput(vtkimage);

      //Spacing Manual or Original (Original spacing is lost during image processing - 
      
      imageresample->SetAxisOutputSpacing(0, 1);
      imageresample->SetAxisOutputSpacing(1, 1);
      imageresample->SetAxisOutputSpacing(2, 1); //auf 1 interpoliert
      imageresample->InterpolateOn();//OFF: pixel replication is used 
      
      //int extent[6];
      //vtkimage->GetWholeExtent(extent);
      //double spacing[3];
      //vtkimage->GetSpacing(spacing);
      //cout << "0. spacing vtkimage: [0]" << spacing[0]<<"  ::[1]"<< spacing[1]<<"  ::[2]"<< spacing[2]  <<"\n";
      //cout << "1. dimensions vtkimage: " << extent[1]<<"x"<< extent[3]<<"x"<< extent[5]  <<"\n";
      //cout << "2. Dimensionen: " << imageresample->GetDimensionality()   <<"\n";
      //cout << "3. GetAxisMagnificationFactor: " << imageresample->GetAxisMagnificationFactor (0)<< " x "
      //    << imageresample->GetAxisMagnificationFactor (1) << " x "
      //    << imageresample->GetAxisMagnificationFactor (2) <<"\n";
      //cout << "4. interpolated? "<< imageresample->GetInterpolate()<<endl;

      vtkimage=imageresample->GetOutput();
      vtkimage->UpdateInformation();
      
      //vtkimage->GetWholeExtent(extent);
      //vtkimage->GetSpacing(spacing);
      //cout << "10. dimensions vtkimage: " << extent[1]<<"x"<< extent[3]<<"x"<< extent[5]  <<"\n";
      //cout << "11. spacing vtkimage: [0]" << spacing[0]<<"  ::[1]"<< spacing[1]<<"  ::[2]"<< spacing[2]  <<"\n";
      
    }

    // Median -->smooth 2. mal ;-(
    if(m_UseMedianFilter3D)
    {
      vtkImageMedian3D *median = vtkImageMedian3D::New();
      median->SetInput(vtkimage); //RC++
      vtkimage->Delete();//RC--
      median->SetKernelSize(m_MedianKernelSizeX,m_MedianKernelSizeY,m_MedianKernelSizeZ);//Std: 3x3x3
      median->ReleaseDataFlagOn();
      median->UpdateInformation();
      vtkimage = median->GetOutput(); 
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
      gaussian->SetStandardDeviation( m_StdDeviation );
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

