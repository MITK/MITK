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


#ifndef __mitkDiffusionImage__h
#define __mitkDiffusionImage__h

#include "mitkImage.h"
#include "itkVectorImage.h"
#include "itkVectorImageToImageAdaptor.h"
#include <iomanip>
#include <itkCommand.h>

namespace mitk
{

/**
  * \brief this class encapsulates diffusion volumes (vectorimages not
  * yet supported by mitkImage)
  */
template<class TPixelType>
class DiffusionImage : public Image
{

public:
  typedef TPixelType PixelType;
  typedef typename itk::VectorImage<TPixelType, 3>
  ImageType;
  typedef vnl_vector_fixed< double, 3 >       GradientDirectionType;
  typedef itk::VectorContainer< unsigned int,
  GradientDirectionType >                   GradientDirectionContainerType;
  typedef itk::VectorImageToImageAdaptor< TPixelType, 3 >
  AdaptorType;
  typedef vnl_matrix_fixed< double, 3, 3 >      MeasurementFrameType;

  // BValue Map
  // key   := b-Value
  // value := indicesVector (containing corresponding gradient directions for a b-Value-Shell
  typedef std::vector< unsigned int > IndicesVector;
  typedef std::map< double , IndicesVector >  BValueMap;

  mitkClassMacro( DiffusionImage, Image )
  itkNewMacro(Self)


  void AverageRedundantGradients(double precision);

  GradientDirectionContainerType::Pointer CalcAveragedDirectionSet(double precision, GradientDirectionContainerType::Pointer directions);

  void CorrectDKFZBrokenGradientScheme(double precision);

  typename ImageType::Pointer GetVectorImage() { return m_VectorImage; }
  void SetVectorImage(typename ImageType::Pointer image ) { this->m_VectorImage = image; }

  void InitializeFromVectorImage();
  void SetDisplayIndexForRendering(int displayIndex);

  GradientDirectionContainerType::Pointer GetDirectionsWithoutMeasurementFrame() { return m_OriginalDirections; }
  GradientDirectionContainerType::Pointer GetDirections() { return m_Directions; }

  void SetDirections( GradientDirectionContainerType::Pointer directions )
  {
      this->m_OriginalDirections = directions;
      ApplyMeasurementFrame();
  }
  void SetDirections(const std::vector<itk::Vector<double,3> > directions);

  MeasurementFrameType GetMeasurementFrame()  { return m_MeasurementFrame; }
  void SetMeasurementFrame( MeasurementFrameType mFrame )  { this->m_MeasurementFrame = mFrame; this->ApplyMeasurementFrame(); }

  bool AreAlike(GradientDirectionType g1, GradientDirectionType g2, double precision);
  int GetNumDirections();
  int GetNumB0();

  float GetB_Value(int i);
  bool IsMultiBval();
  void UpdateBValueList();

  IndicesVector GetB0Indices();

  itkGetMacro(B_Value, float)
  itkSetMacro(B_Value, float)

  BValueMap GetB_ValueMap(){ return m_B_ValueMap; }

  void AddDirectionsContainerObserver();
  void RemoveDirectionsContainerObserver();

protected:
  DiffusionImage();
  virtual ~DiffusionImage();

  void ApplyMeasurementFrame();

  typename ImageType::Pointer               m_VectorImage;
  GradientDirectionContainerType::Pointer   m_Directions;
  GradientDirectionContainerType::Pointer   m_OriginalDirections;
  float                                     m_B_Value;
  typename AdaptorType::Pointer             m_VectorImageAdaptor;
  int                                       m_DisplayIndex;
  MeasurementFrameType                      m_MeasurementFrame;
  BValueMap                                 m_B_ValueMap;



  unsigned long    m_DirectionsObserverTag;
};

} // namespace mitk

#include "mitkDiffusionImage.txx"

#endif /* __mitkDiffusionImage__h */
