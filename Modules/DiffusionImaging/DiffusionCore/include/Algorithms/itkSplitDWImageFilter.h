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

#ifndef ITKSPLITDWIMAGEFILTER_H
#define ITKSPLITDWIMAGEFILTER_H

#include "itkImageToImageFilter.h"
#include <itkVectorImage.h>

namespace itk
{

/** \class SplitDWImageFilter
 *
 * \brief Splits a DW-Image passed in as input into a 3D-t image where each volume coresponds to a
 * gradient image ( or the unweighted b0 )
 *
 * Several applications require to get the gradient images as a separate volume, f.e. the registration for
 * head-motion correction. Also a reduction of the DW Image is possible when combined with its counterpart filter,
 * the \sa mitkImageToDiffusionImageSource, which can reinterpret a 3d+t (scalar) image into a diffusion weighted image.
 */

template< class TInputImagePixelType,
          class TOutputImagePixelType >
class SplitDWImageFilter :
    public ImageToImageFilter< VectorImage<TInputImagePixelType, 3>,
    Image< TOutputImagePixelType, 4 > >
{
public:

  typedef SplitDWImageFilter  Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;

  typedef TInputImagePixelType                    InputPixelType;
  typedef TOutputImagePixelType                   OutputPixelType;

  typedef ImageToImageFilter< VectorImage<TInputImagePixelType, 3>,
      Image< TOutputImagePixelType, 4 > >         Superclass;

  /** typedefs from superclass */
  typedef typename Superclass::InputImageType     InputImageType;
  //typedef typename Superclass::OutputImageType    OutputImageType;
  typedef Image< TOutputImagePixelType, 4 >       OutputImageType;
  typedef typename OutputImageType::RegionType    OutputImageRegionType;

  typedef std::vector< unsigned int > IndexListType;
  typedef std::map< unsigned int, IndexListType> BValueMapType;

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(SplitDWImageFilter, ImageToImageFilter);

  /**
   * @brief Set the indices of the images to be extracted.
   *
   *
   */
  void SetExtractIndices( IndexListType list)
  {
    m_IndexList = list;
  }

  /**
   * @brief Extract all images
   *
   * The same as setting SetExtractIndices( ) with [0,1,2,...,N]
   */
  void SetExtractAll()
  {
    m_ExtractAllImages = true;
  }

  /**
   * @brief Selects only the weighted images with b-value above the given b_threshold to be extracted
   *
   * Setting b_threshold to 0 will do the same as \sa SetExtractAll. Please note that some images have no true
   * unweighted images as the minimal b-value is something like 5 so for extracting all.
   *
   * @note It will reorder the images!
   *
   * @param b_threshold the minimal b-value to be extracted
   * @param map the map with b-values to the corresponding image
   *
   * @sa GetIndexList
   */
  void SetExtractAllAboveThreshold( double b_threshold, BValueMapType map);

  /**
   * @brief SetExtractSingleShell
   * @param b_value b-value of the shell to be extracted
   * @param tol the tolerance of the shell choice, i.e. all shells within [ b_value - tol, b_value + tol ] will be extracted
   */
  void SetExtractSingleShell( double b_value, BValueMapType map, double tol );

  /**
   * @brief Returns the index list used for extraction
   *
   * The list is necessary for further processing, especially when a b-value threshold is used ( like in \sa SetExtractAllAboveThreshold )
   *
   * @return The index list used during the extraction
   */
  const IndexListType GetIndexList() const
  {
    return m_IndexList;
  }

protected:

  SplitDWImageFilter();
  virtual ~SplitDWImageFilter(){};

  void GenerateData();

  /** The dimension of the output does not match the dimension of the input
    hence we need to re-implement the CopyInformation method to avoid
    executing the default implementation which tries to copy the input information to the
    output
  */
  virtual void CopyInformation( const DataObject *data);

  /** Override of the ProcessObject::GenerateOutputInformation() because of different
    dimensionality of the input and the output
  */
  virtual void GenerateOutputInformation();

  IndexListType m_IndexList;
  bool m_ExtractAllImages;
};

} //end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSplitDWImageFilter.txx"
#endif

#endif // ITKSPLITDWIMAGEFILTER_H
