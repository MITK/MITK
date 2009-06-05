/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkImageSeriesReader.h,v $
  Language:  C++
  Date:      $Date: 2007-08-24 13:35:59 $
  Version:   $Revision: 1.14 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkDicomDiffusionVolumesReader_h
#define __mitkDicomDiffusionVolumesReader_h

#include "itkImageSource.h"
#include "itkVectorImage.h"
#include "mitkDiffusionVolumeHeaderInformation.h"
#include "itkDataObject.h"
#include "mitkCommon.h"

//#include "itkImageIOBase.h"
//#include "itkExceptionObject.h"
//#include "itkSize.h"
//#include "itkImageRegion.h"
//#include <vector>
//#include <string>
//#include "itkMetaDataDictionary.h"

namespace mitk
{

/** \brief 
 */

template <class TPixelType, const int TDimension>
class DicomDiffusionVolumesReader : public itk::ImageSource< itk::VectorImage< TPixelType, TDimension > >
{
public:

  typedef itk::VectorImage< TPixelType, TDimension >   OutputImageType;
  typedef itk::Image<TPixelType, TDimension >         InputImageType;
  typedef itk::ImageSource< OutputImageType > ImageSourceType;
  
  mitkClassMacro( DicomDiffusionVolumesReader, ImageSourceType );
  itkNewMacro(Self);


  /** The size of the output image. */
  typedef typename OutputImageType::SizeType  SizeType;

  /** The region of the output image. */
  typedef typename OutputImageType::RegionType  ImageRegionType;

  /** The dictionary type of the output image. */
  //typedef MetaDataDictionary                   DictionaryType;
  //typedef MetaDataDictionary *                 DictionaryRawPointer;
  //typedef std::vector< DictionaryRawPointer >  DictionaryArrayType;
  //typedef const DictionaryArrayType *          DictionaryArrayRawPointer;
  
  typedef  std::vector<DiffusionVolumeHeaderInformation::Pointer> HeaderContainer;

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
  
  /** Get access to the Array of MetaDataDictionaries */
  //DictionaryArrayRawPointer GetMetaDataDictionaryArray() const;
  
protected:
  //DicomDiffusionVolumesReader();
  //~DicomDiffusionVolumesReader();

  void PrintSelf(std::ostream& os, itk::Indent indent) const;
  
  /** Does the real work. */
  virtual void GenerateData();

  /** A list of filenames to be processed. */
  HeaderContainer m_Headers;

  /** The number of independent variables in the images that comprise
   *  the series. */
  int m_NumberOfDimensionsInImage;

  /** Array of MetaDataDictionaries. This allows to hold information from the
   * ImageIO objects after reading every sub image in the series */
  //DictionaryArrayType m_MetaDataDictionaryArray;

private:
  //DicomDiffusionVolumesReader(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

} //namespace MITK

#include "mitkDicomDiffusionVolumesReader.cpp"

#endif // __mitkDicomDiffusionVolumesReader_h
