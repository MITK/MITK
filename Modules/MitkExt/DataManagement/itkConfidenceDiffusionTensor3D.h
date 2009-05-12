/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkConfidenceDiffusionTensor3D_h
#define __itkConfidenceDiffusionTensor3D_h

// Undefine an eventual DiffusionTensor3D macro
#ifdef ConfidenceDiffusionTensor3D
#undef ConfidenceDiffusionTensor3D
#endif

#include "itkDiffusionTensor3D.h"

namespace itk {

  template < typename TComponent >
  class ConfidenceDiffusionTensor3D: public DiffusionTensor3D<TComponent>
  {
    
  public:

    /** Standard class typedefs. */
    typedef ConfidenceDiffusionTensor3D  Self;
    typedef DiffusionTensor3D<TComponent> Superclass;

    /** Propagating some typedef from the superclass */
    typedef typename Superclass::ValueType             ValueType;
    typedef typename Superclass::ComponentType         ComponentType;
#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (__GNUC__ == 3 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ == 3)
    typedef ComponentType ComponentArrayType[6];
#else
    typedef typename Superclass::ComponentArrayType    ComponentArrayType;
#endif
    typedef typename Superclass::AccumulateValueType   AccumulateValueType;
    typedef typename Superclass::RealValueType         RealValueType;

    typedef typename Superclass::EigenValuesArrayType    EigenValuesArrayType;
    typedef typename Superclass::EigenVectorsMatrixType  EigenVectorsMatrixType;

    /** Default Constructor. */
    ConfidenceDiffusionTensor3D();

    /** Constructor with initialization. */
    ConfidenceDiffusionTensor3D(const Self& r);
    ConfidenceDiffusionTensor3D(const Superclass& r);
    ConfidenceDiffusionTensor3D(const ComponentType& r);
    ConfidenceDiffusionTensor3D(const ComponentArrayType r);

    /** Pass-through assignment operator for the Array base class. */
    Self& operator= (const Self& r);
    Self& operator= (const Superclass & r);
    Self& operator= (const ComponentType& r);
    Self& operator= (const ComponentArrayType r);
    
    TComponent GetConfidence()
    {return m_Confidence;}

  private:

    TComponent m_Confidence;

  };

} //end namespace 

// Define instantiation macro for this template.
#define ITK_TEMPLATE_ConfidenceDiffusionTensor3D(_, EXPORT, x, y) namespace itk { \
  _(1(class EXPORT ConfidenceDiffusionTensor3D< ITK_TEMPLATE_1 x >)) \
  namespace Templates { typedef ConfidenceDiffusionTensor3D< ITK_TEMPLATE_1 x > \
  ConfidenceDiffusionTensor3D##y; } \
}

#if ITK_TEMPLATE_EXPLICIT
# include "Templates/itkConfidenceDiffusionTensor3D+-.h"
#endif

#if ITK_TEMPLATE_TXX
# include "itkConfidenceDiffusionTensor3D.cpp"
#endif


#endif /*__itkConfidenceDiffusionTensor3D_h */


