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

#ifndef __mitkDicomDiffusionImageReader_cpp
#define __mitkDicomDiffusionImageReader_cpp

#include "mitkDicomDiffusionImageReader.h"

#include "itkImageSeriesReader.h"


namespace mitk
{

template <class TPixelType, const int TDimension>
void DicomDiffusionImageReader<TPixelType, TDimension>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}


template <class TPixelType, const int TDimension>
void DicomDiffusionImageReader<TPixelType, TDimension>
::GenerateOutputInformation(void)
{
  typename OutputImageType::Pointer output = this->GetOutput();
  typedef itk::ImageSeriesReader<InputImageType> ReaderType;

  // Read the first (or last) volume and use its size.
  if (m_Headers.size() > 0)
  {
    typename ReaderType::Pointer reader = ReaderType::New();

    try
    {
      // Read the image
      reader->SetFileNames (m_Headers[0]->m_DicomFilenames);
      reader->UpdateOutputInformation();

      output->SetSpacing( reader->GetOutput()->GetSpacing() );   // Set the image spacing
      output->SetOrigin( reader->GetOutput()->GetOrigin() );     // Set the image origin
      output->SetDirection( reader->GetOutput()->GetDirection() );  // Set the image direction
      output->SetLargestPossibleRegion( reader->GetOutput()->GetLargestPossibleRegion() );
      output->SetVectorLength( m_Headers.size() );
    }
    catch (itk::ExceptionObject &e)
    {
      throw e;
    }
  }
  else
  {
    itkExceptionMacro(<< "At least one filename is required." );
  }
}


template <class TPixelType, const int TDimension>
  void
    DicomDiffusionImageReader<TPixelType, TDimension>
    ::EnlargeOutputRequestedRegion(itk::DataObject *output)
  {
    typename OutputImageType::Pointer out = dynamic_cast<OutputImageType*>(output);
    out->SetRequestedRegion( out->GetLargestPossibleRegion() );
}


template <class TPixelType, const int TDimension>
void DicomDiffusionImageReader<TPixelType, TDimension>
::GenerateData()
{
  typedef itk::ImageSeriesReader<InputImageType> ReaderType;

  typename OutputImageType::Pointer output = this->GetOutput();

  typedef typename OutputImageType::RegionType   RegionType;
  RegionType requestedRegion = output->GetRequestedRegion();

  // Each file must have the same size.
  SizeType validSize = requestedRegion.GetSize();

  int numberOfVolumes = static_cast<int>(m_Headers.size());

  // Allocate the output buffer
  output->SetBufferedRegion( requestedRegion );
  output->Allocate();

  itk::ProgressReporter progress(this, 0,
                            m_Headers.size(),
                            m_Headers.size());

  itk::ImageRegionIterator<OutputImageType> ot (output, requestedRegion );

  typename OutputImageType::PixelType vec;

  for (int i = 0; i < numberOfVolumes; i ++)
    {

    MITK_INFO << "Loading volume " << i+1 << "/" << numberOfVolumes;
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileNames(m_Headers[i]->m_DicomFilenames);
    reader->UpdateLargestPossibleRegion();

    if (reader->GetOutput()->GetRequestedRegion().GetSize() != validSize)
      {
      itkExceptionMacro(<< "Size mismatch!");
      }

    itk::ImageRegionConstIterator<InputImageType> it (reader->GetOutput(),
                                               reader->GetOutput()->GetLargestPossibleRegion());

    while (!it.IsAtEnd())
      {
        vec = ot.Get();
        vec.SetElement(i, it.Get());
        ot.Set(vec);
        ++it;
        ++ot;
      }
    ot.GoToBegin();
    progress.CompletedPixel();
    }
}

} //namespace MITK

#endif
