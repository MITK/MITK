/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <cstdlib>
#include <ctime>

#include <iostream>
#include <fstream>

#include <itksys/SystemTools.hxx>
#include "mitkPixelType.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileReader.h"
#include <mitkIOUtil.h>

template< class D, class T >
mitk::TeemDiffusionTensor3DReconstructionImageFilter<D,T>
::TeemDiffusionTensor3DReconstructionImageFilter():
m_EstimateErrorImage(false),m_Sigma(-19191919),
m_EstimationMethod(TeemTensorEstimationMethodsLLS),
m_NumIterations(1),m_ConfidenceThreshold(-19191919.0),
m_ConfidenceFuzzyness(0.0),m_MinPlausibleValue(1.0)
{
}

template< class D, class T >
mitk::TeemDiffusionTensor3DReconstructionImageFilter<D,T>
::~TeemDiffusionTensor3DReconstructionImageFilter()
{
}

void file_replace(std::string filename, std::string what, std::string with)
{
  ofstream myfile2;

  std::locale C("C");
  std::locale originalLocale2 = myfile2.getloc();
  myfile2.imbue(C);

  char filename2[512];
  sprintf(filename2, "%s2",filename.c_str());
  myfile2.open (filename2);

  std::string line;
  ifstream myfile (filename.c_str());

  std::locale originalLocale = myfile.getloc();
  myfile.imbue(C);

  if (myfile.is_open())
  {
    while (! myfile.eof() )
    {
      getline (myfile,line);
      itksys::SystemTools::ReplaceString(line,what.c_str(),with.c_str());
      myfile2 << line << std::endl;
    }
    myfile.close();
  }
  myfile2.close();
  itksys::SystemTools::RemoveFile(filename.c_str());
  rename(filename2,filename.c_str());

  myfile.imbue( originalLocale );
  myfile2.imbue( originalLocale2 );
}

// do the work
template< class D, class T >
void
mitk::TeemDiffusionTensor3DReconstructionImageFilter<D,T>
::Update()
{

  // save input image to nrrd file in temp-folder
  char filename[512];
  srand((unsigned)time(0));
  int random_integer = rand();
  sprintf( filename, "dwi_%d.nhdr",random_integer);

  try
  {
    mitk::IOUtil::Save(m_Input, filename);
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e << std::endl;
  }

  file_replace(filename,"vector","list");

  // build up correct command from input params
  char command[4096];
  sprintf( command, "tend estim -i %s -B kvp -o tensors_%d.nhdr -knownB0 true",
    filename, random_integer);

  //m_DiffusionImages;
  if(m_EstimateErrorImage)
  {
    sprintf( command, "%s -ee error_image_%d.nhdr", command, random_integer);
  }

  if(m_Sigma != -19191919)
  {
    sprintf( command, "%s -sigma %f", command, m_Sigma);
  }

  switch(m_EstimationMethod)
  {
  case TeemTensorEstimationMethodsLLS:
    sprintf( command, "%s -est lls", command);
    break;
  case TeemTensorEstimationMethodsMLE:
    sprintf( command, "%s -est mle", command);
    break;
  case TeemTensorEstimationMethodsNLS:
    sprintf( command, "%s -est nls", command);
    break;
  case TeemTensorEstimationMethodsWLS:
    sprintf( command, "%s -est wls", command);
    break;
  }

  sprintf( command, "%s -wlsi %d", command, m_NumIterations);

  if(m_ConfidenceThreshold != -19191919.0)
  {
    sprintf( command, "%s -t %f", command, m_ConfidenceThreshold);
  }

  sprintf( command, "%s -soft %f", command, m_ConfidenceFuzzyness);
  sprintf( command, "%s -mv %f", command, m_MinPlausibleValue);

  // call tend estim command
  std::cout << "Calling <" << command << ">" << std::endl;
  int success = system(command);
  if(!success)
  {
    MITK_ERROR << "system command could not be called!";
  }

  remove(filename);
  sprintf( filename, "dwi_%d.raw", random_integer);
  remove(filename);

  // change kind from tensor to vector
  sprintf( filename, "tensors_%d.nhdr", random_integer);
  file_replace(filename,"3D-masked-symmetric-matrix","vector");

  // read result as mitk::Image and provide it in m_Output
  typedef itk::ImageFileReader<VectorImageType> FileReaderType;
  typename FileReaderType::Pointer reader = FileReaderType::New();
  reader->SetFileName(filename);
  reader->Update();
  typename VectorImageType::Pointer vecImage = reader->GetOutput();

  remove(filename);
  sprintf( filename, "tensors_%d.raw", random_integer);
  remove(filename);

  typename ItkTensorImageType::Pointer itkTensorImage = ItkTensorImageType::New();
  itkTensorImage->SetSpacing( vecImage->GetSpacing() );   // Set the image spacing
  itkTensorImage->SetOrigin( vecImage->GetOrigin() );     // Set the image origin
  itkTensorImage->SetDirection( vecImage->GetDirection() );  // Set the image direction
  itkTensorImage->SetLargestPossibleRegion( vecImage->GetLargestPossibleRegion() );
  itkTensorImage->SetBufferedRegion( vecImage->GetLargestPossibleRegion() );
  itkTensorImage->SetRequestedRegion( vecImage->GetLargestPossibleRegion() );
  itkTensorImage->Allocate();

  itk::ImageRegionIterator<VectorImageType> it(vecImage,
    vecImage->GetLargestPossibleRegion());

  itk::ImageRegionIterator<ItkTensorImageType> it2(itkTensorImage,
    itkTensorImage->GetLargestPossibleRegion());
  it2 = it2.Begin();

  //#pragma omp parallel private (it)
  {
    for(it=it.Begin();!it.IsAtEnd(); ++it, ++it2)
    {
      //#pragma omp single nowait
      {
        VectorType vec = it.Get();
        TensorType tensor;
        for(int i=1;i<7;i++)
          tensor[i-1] = vec[i] * vec[0];
        it2.Set( tensor );
      }
    } // end for
  } // end ompparallel

  m_OutputItk = mitk::TensorImage::New();
  m_OutputItk->InitializeByItk(itkTensorImage.GetPointer());
  m_OutputItk->SetVolume( itkTensorImage->GetBufferPointer() );

  // in case: read resulting error-image and provide it in m_ErrorImage
  if(m_EstimateErrorImage)
  {
    // open error image here
  }

}

