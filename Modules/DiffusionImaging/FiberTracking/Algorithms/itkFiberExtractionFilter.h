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
#ifndef itkFiberExtractionFilter_h
#define itkFiberExtractionFilter_h

// MITK
#include <mitkFiberBundle.h>

// ITK
#include <itkProcessObject.h>

namespace itk{

/**
* \brief Extract streamlines from tractograms using binary images */

template< class PixelType >
class FiberExtractionFilter : public ProcessObject
{
public:

  enum MODE {
    OVERLAP,
    ENDPOINTS
  };

  enum INPUT {
    SCALAR_MAP,  ///< In this case, positive means roi image > threshold
    LABEL_MAP    ///< In this case, positive means roi image in labels vector
  };

  typedef FiberExtractionFilter Self;
  typedef ProcessObject                   Superclass;
  typedef SmartPointer< Self >            Pointer;
  typedef SmartPointer< const Self >      ConstPointer;
  typedef itk::Image< PixelType , 3>      ItkInputImgType;


  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( FiberExtractionFilter, ProcessObject )

  virtual void Update() override{
    this->GenerateData();
  }

  itkSetMacro( InputFiberBundle, mitk::FiberBundle::Pointer )
  itkSetMacro( Mode, MODE )
  itkSetMacro( InputType, INPUT )
  itkSetMacro( BothEnds, bool )
  itkSetMacro( OverlapFraction, float )
  itkSetMacro( DontResampleFibers, bool )
  itkSetMacro( NoNegatives, bool )
  itkSetMacro( NoPositives, bool )
  itkSetMacro( Interpolate, bool )
  itkSetMacro( Threshold, float )

  void SetRoiImages(const std::vector< ItkInputImgType* > &rois);
  void SetLabels(const std::vector<unsigned short> &Labels);

  std::vector<mitk::FiberBundle::Pointer> GetPositives() const;
  std::vector<mitk::FiberBundle::Pointer> GetNegatives() const;

protected:

  void GenerateData() override;

  FiberExtractionFilter();
  virtual ~FiberExtractionFilter();

  mitk::FiberBundle::Pointer CreateFib(std::vector< long >& ids);
  void ExtractOverlap(mitk::FiberBundle::Pointer fib);
  void ExtractEndpoints(mitk::FiberBundle::Pointer fib);
  bool IsPositive(const itk::Point<float, 3>& itkP, itk::Image<PixelType, 3>* image);

  mitk::FiberBundle::Pointer                  m_InputFiberBundle;
  std::vector< mitk::FiberBundle::Pointer >   m_Positives;
  std::vector< mitk::FiberBundle::Pointer >   m_Negatives;
  std::vector< ItkInputImgType* >             m_RoiImages;
  bool                                        m_DontResampleFibers;
  MODE                                        m_Mode;
  INPUT                                       m_InputType;
  bool                                        m_BothEnds;
  float                                       m_OverlapFraction;
  bool                                        m_NoNegatives;
  bool                                        m_NoPositives;
  bool                                        m_Interpolate;
  float                                       m_Threshold;
  std::vector< unsigned short >               m_Labels;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFiberExtractionFilter.cpp"
#endif

#endif
