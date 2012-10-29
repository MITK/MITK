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

#ifndef ITK_ConnectomicsNetworkToConnectivityMatrixImageFilter_H
#define ITK_ConnectomicsNetworkToConnectivityMatrixImageFilter_H

// ITK includes
#include <itkImageSource.h>
#include <itkImage.h>

// MITK includes
#include "mitkConnectomicsNetwork.h"

#include "MitkDiffusionImagingExports.h"

namespace itk
{

  class MitkDiffusionImaging_EXPORT ConnectomicsNetworkToConnectivityMatrixImageFilter : public ImageSource< itk::Image< unsigned short, 2 > >
  {

  public:
    typedef ConnectomicsNetworkToConnectivityMatrixImageFilter Self;
    typedef ProcessObject Superclass;
    typedef SmartPointer< Self > Pointer;
    typedef SmartPointer< const Self > ConstPointer;

    typedef itk::Image< unsigned short, 2 > OutputImageType;
    typedef OutputImageType::PixelType OutPixelType;

    typedef mitk::ConnectomicsNetwork  InputType;

    itkNewMacro(Self)
      itkTypeMacro( ConnectomicsNetworkToConnectivityMatrixImageFilter, ImageSource )

      /** Get/Set m_BinaryConnectivity **/
      itkSetMacro( BinaryConnectivity, bool)
      itkGetMacro( BinaryConnectivity, bool)

      /** Get/Set m_RescaleConnectivity **/
      itkSetMacro( RescaleConnectivity, bool)
      itkGetMacro( RescaleConnectivity, bool)

      itkSetMacro( InputNetwork, InputType::Pointer)

      void GenerateData();

  protected:

    ConnectomicsNetworkToConnectivityMatrixImageFilter();
    virtual ~ConnectomicsNetworkToConnectivityMatrixImageFilter();

    /** Controls whether the connectivity matrix is binary */
    bool m_BinaryConnectivity;
    /** Controls whether the connectivity matrix entries are rescaled to lie between 0 and 255*/
    bool m_RescaleConnectivity;

    InputType::Pointer m_InputNetwork;

  };

}

//#ifndef ITK_MANUAL_INSTANTIATION
//#include "itkConnectomicsNetworkToConnectivityMatrixImageFilter.cpp"
//#endif


#endif /* ITK_ConnectomicsNetworkToConnectivityMatrixImageFilter_H */
