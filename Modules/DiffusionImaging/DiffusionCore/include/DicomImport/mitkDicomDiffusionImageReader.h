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

#ifndef __mitkDicomDiffusionImageReader_h
#define __mitkDicomDiffusionImageReader_h

#include "itkImageSource.h"
#include "itkVectorImage.h"
#include "mitkDiffusionImageHeaderInformation.h"
#include "itkDataObject.h"
#include "mitkCommon.h"

namespace mitk
{

/** \brief Reads in the image data stored as DICOM series. The series is specified by a HeaderContainer
 */

template <class TPixelType, const int TDimension>
class DicomDiffusionImageReader : public itk::ImageSource< itk::VectorImage< TPixelType, TDimension > >
{
public:

  typedef itk::VectorImage< TPixelType, TDimension >   OutputImageType;
  typedef itk::Image<TPixelType, TDimension >         InputImageType;
  typedef itk::ImageSource< OutputImageType > ImageSourceType;

  mitkClassMacro( DicomDiffusionImageReader, ImageSourceType );
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)


  /** The size of the output image. */
  typedef typename OutputImageType::SizeType  SizeType;

  /** The region of the output image. */
  typedef typename OutputImageType::RegionType  ImageRegionType;

  typedef  std::vector<DiffusionImageHeaderInformation::Pointer> HeaderContainer;

  /** Set the vector of strings that contains the file names. Files
   * are processed in sequential order. */
  void SetHeaders (const HeaderContainer &headers)
    {
    if ( m_Headers != headers)
      {
      m_Headers = headers;
      this->Modified();
      }
    }
  const HeaderContainer & GetHeaders() const
    {
    return m_Headers;
    }

  /** Prepare the allocation of the output image during the first back
   * propagation of the pipeline. */
  virtual void GenerateOutputInformation(void);

  /** Give the reader a chance to indicate that it will produce more
   * output than it was requested to produce. ImageSeriesReader cannot
   * currently read a portion of an image (since the ImageIO objects
   * cannot read a portion of an image), so the ImageSeriesReader must
   * enlarge the RequestedRegion to the size of the image on disk. */
  virtual void EnlargeOutputRequestedRegion(itk::DataObject *output);

protected:
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /** Does the real work. */
  virtual void GenerateData();

  /** A list of filenames to be processed. */
  HeaderContainer m_Headers;

  /** The number of independent variables in the images that comprise
   *  the series. */
  int m_NumberOfDimensionsInImage;


private:
  void operator=(const Self&); //purposely not implemented
};

} //namespace MITK

#include "mitkDicomDiffusionImageReader.cpp"

#endif // __mitkDicomDiffusionImageReader_h
