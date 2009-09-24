/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Thu, 28 May 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef mitkVolumeVisualizationImagePreprocessor_h_include
#define mitkVolumeVisualizationImagePreprocessor_h_include


#include "mitkCommon.h"


#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkTransferFunctionProperty.h"

#include <itkMedianImageFilter.h>
#include "mitkMaskImageFilter.h"
#include "mitkAutoCropImageFilter.h"
#include "mitkSurface.h"

#include <vtkContourFilter.h>
#include <vtkImageChangeInformation.h>
#include <vtkMatrix4x4.h>
#include <vtkLinearTransform.h>
#include <vtkDataObject.h>

#include <itkDiscreteGaussianImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkThresholdLabelerImageFilter.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryBallStructuringElement.h>

#include <vtkSmoothPolyDataFilter.h>

#include <vtkDecimatePro.h>
#include <vtkQuadricDecimation.h>

namespace mitk
{

class MITKEXT_CORE_EXPORT VolumeVisualizationImagePreprocessor : public itk::Object
{
public:

  mitkClassMacro(VolumeVisualizationImagePreprocessor, itk::Object);

  itkNewMacro(Self);  


  mitk::Image::Pointer Process( mitk::Image::Pointer originalCT,
                                mitk::Image::Pointer originalLiverMask );


  double GetEstimatedThreshold() const
  {
    return m_EstimatedThreshold;
  }

  double GetMinThreshold() const
  {
    return m_MinThreshold;
  }

  double GetMaxThreshold() const
  {
    return m_MaxThreshold;
  }
                                
  double GetLastUsedThreshold() const
  {
    return m_LastUsedTreshold;
  }
                                
  mitk::TransferFunction::Pointer GetInitialTransferFunction( );
  void UpdateTransferFunction( mitk::TransferFunction::Pointer tf , int treshold );


protected:

  typedef itk::Image<short, 3>          CTImage;
  typedef itk::ImageRegionIterator< CTImage  > CTIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< CTImage  > CTIteratorIndexType;
  
  typedef itk::Image<unsigned char, 3 > BinImage;
  typedef itk::ImageRegionIterator< BinImage > BinIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< BinImage > BinIteratorIndexType;

  typedef itk::Image<unsigned short, 3 > LabelImage;
  typedef itk::ImageRegionIterator< LabelImage > LabelIteratorType;


  VolumeVisualizationImagePreprocessor();

  ~VolumeVisualizationImagePreprocessor();


  CTImage::Pointer Composite( CTImage::Pointer work,
    BinImage::Pointer mask,
    BinImage::Pointer dilated,
    BinImage::Pointer eroded );


  void DetermineBoundingBox( BinImage::Pointer mask );

  CTImage::Pointer Crop(CTImage::Pointer src );
  BinImage::Pointer Crop(BinImage::Pointer src );




  /**the original mask is decrease by 1 Voxel*/
  BinImage::Pointer Dilate(BinImage::Pointer src);
  BinImage::Pointer Erode(BinImage::Pointer src);
  CTImage::Pointer Gaussian(CTImage::Pointer src);
  LabelImage::Pointer ConnectComponents(BinImage::Pointer src);
  LabelImage::Pointer RelabelComponents(LabelImage::Pointer src);
  BinImage::Pointer Threshold(CTImage::Pointer src, int threshold);




  // grayvalue of voxel out of liver 
  double m_OutOfLiverValue;

  // grayvalue liver surface will be set to
  double m_surfaceValue;

  // average of all grayvalues located on the liver surface 
  double m_realSurfaceValue;

  double m_realInLiverValue;
  
  //estimated treshold value 
  double m_EstimatedThreshold;
  
  double m_GreatestStructureThreshold;


  //minimum treshold value 
  double m_MinThreshold;

  //maximum treshold value 
  double m_MaxThreshold;
  
  int m_MinX;
  int m_MinY;
  int m_MinZ;

  int m_MaxX;
  int m_MaxY;
  int m_MaxZ;

  int m_LastUsedTreshold;
  
  int histogramm[65536];
  int total;

  
  int GetHistogrammValueFromTop( double part );
  int GetHistogrammValueFromBottom( double part );
  

};


}

#endif // #define mitkVolumeVisualizationImagePreprocessor_h_include
