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
#include <itkImageDuplicator.h>
#include "MitkDiffusionCoreExports.h"

namespace mitk
{

/**
  * \brief this class encapsulates diffusion volumes (vectorimages not
  * yet supported by mitkImage)
  */
template< class TPixelType >
class MITK_EXPORT DiffusionImage : public Image
{

public:
  typedef typename itk::VectorImage<TPixelType, 3>                    ImageType;
  typedef vnl_vector_fixed< double, 3 >                               GradientDirectionType;
  typedef itk::VectorContainer< unsigned int,GradientDirectionType >  GradientDirectionContainerType;
  typedef GradientDirectionContainerType::Pointer                     GradientDirectionContainerTypePointer;
  typedef vnl_matrix_fixed< double, 3, 3 >                            MeasurementFrameType;
  typedef std::vector< unsigned int >                                 IndicesVector;
  typedef itk::VectorImage< TPixelType, 3 >                           ItkDwiType;
  typedef itk::ImageDuplicator< ItkDwiType >                          DwiDuplicatorType;

  /**
   * \brief The BValueMap contains seperated IndicesVectors for each b value (index for GradientDirectionContainer)
   * key   := b value
   * value := indicesVector
  */
  typedef std::map< unsigned int , IndicesVector >                    BValueMap;

  mitkClassMacro( DiffusionImage, Image )
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  mitkCloneMacro(Self)

  /**
  * \brief Return the itkVectorImage as pointer
  */
  typename ImageType::Pointer GetVectorImage();

  /**
  * \brief Return the itkVectorImage as const pointer
  */
  const typename ImageType::Pointer GetVectorImage() const;

  /**
  * \brief Set the itkVectorImage
  *
  * \param SmartPointer ofitk::VectorImage
  */
  void SetVectorImage(typename ImageType::Pointer image );

  /**
  * \brief Initialize the mitkImage (base-class) using the first b-zero value of the itkVectorImage
  *
  * \warning First set itkVectorImage, GradientDirectionContainer and the b value of the image.
  * Has to be called.
  */
  void InitializeFromVectorImage();

  /**
  * \brief Return the directions as GradientDirectionContainer const pointer
  *
  * \warning MeasurementFrame already applied
  */
  GradientDirectionContainerType::Pointer GetDirections() const;

  /**
  * \brief Return the directions as GradientDirectionContainer const pointer
  *
  * \warning no MeasurmentFrame applied
  */
  GradientDirectionContainerType::Pointer GetDirectionsWithoutMeasurementFrame() const;

  /**
  * \brief Set the original and current GradientDirectionContainer
  * \param GradientdirectionContainer as pointer
  * \warning Replace the original (m_OriginalDirections) and current (m_Directions) GradientDirectionContainer.
  * For the current directions the MeasurmentFrame is applied. Remove existing GradientDirectionContainer observer
  * of m_Directions and add a new observer to it (observer update the m_B_ValueMap if modifications occours on the container)
  */
  void SetDirections( GradientDirectionContainerType::Pointer directions );

  /**
  * \brief Set the original and current GradientDirectionContainer
  * \param std::vector<itk::Vector<double,3> >
  * \warning Replace the original (m_OriginalDirections) and current (m_Directions) GradientDirectionContainer.
  * For the current directions the MeasurmentFrame is applied.
  * Remove existing GradientDirectionContainer observer of m_Directions and add a new observer to it
  * (observer update the m_B_ValueMap if modifications occours on the container).
  * Calls the overloaded GradientDirectionContainer pointer version of the method.
  */
  void SetDirections( const std::vector<itk::Vector<double,3> > & directions );

  /**
  * \brief Return a copy of the MeasurmentFrame (m_MeasurementFrame)
  */
  MeasurementFrameType GetMeasurementFrame() const;

  /**
  * \brief Set the MeasurementFrame
  *
  * \param Const reference of MeasurementFrameType
  */
  void SetMeasurementFrame( const MeasurementFrameType & mFrame );

  /**
  * \brief Return true if gradients of with diffrent b-values exist
  */
  bool GetNumberOfBValues() const;

  /**
  * \brief Return a BValueMap (key: b value, Value: IndicesVector of the GradientDirectionContainer)
  */
  const BValueMap & GetBValueMap() const;

  /**
  * \brief Return the reference b value
  * GradientsLength * referenceBValue encoding the true b value of the corresponding GradientDirection
  */
  float GetReferenceBValue() const;

  /**
  * \brief Set the reference b value s
  */
  void SetReferenceBValue( float val );

  GradientDirectionContainerType::Pointer CalcAveragedDirectionSet(double precision, GradientDirectionContainerType::Pointer directions);
  void AverageRedundantGradients(double precision);
  void SetDisplayIndexForRendering(int displayIndex);

protected:
  DiffusionImage();
  DiffusionImage(const DiffusionImage<TPixelType> &);
  virtual ~DiffusionImage();

  // Helper Methods
  bool AreAlike(GradientDirectionType g1, GradientDirectionType g2, double precision);
  float GetB_Value(unsigned int i);

  /**
  * \brief Apply the previouse set MeasurmentFrame to all gradients in the GradientsDirectionContainer (m_Directions)
  *
  * \warning first set the MeasurmentFrame
  */
  void ApplyMeasurementFrame();

  /**
  * \brief Update the BValueMap (m_B_ValueMap) using the current gradient directions (m_Directions)
  *
  * \warning Have to be called after each manipulation on the GradientDirectionContainer
  * !especially after manipulation of the m_Directions (GetDirections()) container via pointer access!
  */
  void UpdateBValueMap();

  typename ImageType::Pointer               m_VectorImage;
  float                                     m_B_Value;
  MeasurementFrameType                      m_MeasurementFrame;
  GradientDirectionContainerType::Pointer   m_OriginalDirections;
  GradientDirectionContainerType::Pointer   m_Directions;
  int                                       m_DisplayIndex;
  BValueMap                                 m_B_ValueMap;

};

/**
* @brief Equal A function comparing two images for beeing equal in meta- and imagedata
*
* @ingroup MITKTestingAPI
*
* Following aspects are tested for equality:
*  - mitk image equal test
*  - GradientDirectionContainer
*  - MeasurementFrame
*  - reference BValue
*  - BValueMap
*  - itkVectorImage
*
* @param rightHandSide An image to be compared
* @param leftHandSide An image to be compared
* @param eps Tolarence for comparison. You can use mitk::eps in most cases.
* @param verbose Flag indicating if the user wants detailed console output or not.
* @return true, if all subsequent comparisons are true, false otherwise
*/

template<class TPixelType>
inline bool Equal(const mitk::DiffusionImage<TPixelType> &leftHandSide, const mitk::DiffusionImage<TPixelType> &rightHandSide, ScalarType eps, bool verbose );


} // namespace mitk

#include "mitkDiffusionImage.txx"

#endif /* __mitkDiffusionImage__h */
