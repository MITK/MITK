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
/*=========================================================================
   2
   3 Program:   Tensor ToolKit - TTK
   4 Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkMergeDiffusionImagesFilter.txx $
   5 Language:  C++
   6 Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
   7 Version:   $Revision: 68 $
   8
   9 Copyright (c) INRIA 2010. All rights reserved.
  10 See LICENSE.txt for details.
  11
  12 This software is distributed WITHOUT ANY WARRANTY; without even
  13 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  14 PURPOSE.  See the above copyright notices for more information.
  15
  16 =========================================================================*/
#ifndef _itk_MergeDiffusionImagesFilter_txx_
#define _itk_MergeDiffusionImagesFilter_txx_
#endif

#include "itkMergeDiffusionImagesFilter.h"
#include "itkTensorToL2NormImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <boost/progress.hpp>

namespace itk
{

template <class TScalarType>
MergeDiffusionImagesFilter<TScalarType>::MergeDiffusionImagesFilter()
{

}

template <class TScalarType>
MergeDiffusionImagesFilter<TScalarType>::~MergeDiffusionImagesFilter()
{

}

template <class TScalarType>
void MergeDiffusionImagesFilter<TScalarType>
::SetImageVolumes(DwiImageContainerType cont)
{
    m_ImageVolumes=cont;
}

template <class TScalarType>
void MergeDiffusionImagesFilter<TScalarType>
::SetGradientLists(GradientListContainerType cont)
{
    m_GradientLists=cont;
}

template <class TScalarType>
void MergeDiffusionImagesFilter<TScalarType>
::SetBValues(std::vector< double > bvals)
{
    m_BValues=bvals;
}

template <class TScalarType>
MergeDiffusionImagesFilter<TScalarType>::GradientListType::Pointer MergeDiffusionImagesFilter<TScalarType>
::GetOutputGradients()
{
    return m_OutputGradients;
}

template <class TScalarType>
double MergeDiffusionImagesFilter<TScalarType>
::GetB_Value()
{
    return m_BValue;
}

template <class TScalarType>
void
MergeDiffusionImagesFilter<TScalarType>
::GenerateData ()
{

    if( m_ImageVolumes.size()<2 )
      throw itk::ExceptionObject (__FILE__,__LINE__,"Error: cannot combine less than two DWIs.");

    if( m_GradientLists.size()!=m_ImageVolumes.size() || m_ImageVolumes.size()!=m_BValues.size() || m_BValues.size()!=m_GradientLists.size() )
      throw itk::ExceptionObject (__FILE__,__LINE__,"Error: need same number of b-values, image volumes and gradient containers.");

    typename DwiImageType::Pointer img = m_ImageVolumes.at(0);

    m_NumGradients = 0;
    for (int i=0; i<m_GradientLists.size(); i++)
    {
        m_NumGradients += m_GradientLists.at(i)->Size();
        typename DwiImageType::Pointer tmp = m_ImageVolumes.at(i);
        if ( img->GetLargestPossibleRegion()!=tmp->GetLargestPossibleRegion() )
            throw itk::ExceptionObject (__FILE__,__LINE__,"Error: images are not of same size.");
    }

    m_BValue = m_BValues.at(0);
    m_OutputGradients = GradientListType::New();

    typename DwiImageType::Pointer outImage = DwiImageType::New();
    outImage->SetSpacing( img->GetSpacing() );   // Set the image spacing
    outImage->SetOrigin( img->GetOrigin() );     // Set the image origin
    outImage->SetDirection( img->GetDirection() );  // Set the image direction
    outImage->SetLargestPossibleRegion( img->GetLargestPossibleRegion());
    outImage->SetBufferedRegion( img->GetLargestPossibleRegion() );
    outImage->SetRequestedRegion( img->GetLargestPossibleRegion() );
    outImage->SetVectorLength(m_NumGradients);
    outImage->Allocate();

    this->SetNumberOfRequiredOutputs(1);
    this->SetNthOutput (0, outImage);

    typedef ImageRegionIterator<DwiImageType>               IteratorOutputType;
    IteratorOutputType      itOut (this->GetOutput(0), this->GetOutput(0)->GetLargestPossibleRegion());

    MITK_INFO << "MergeDiffusionImagesFilter: merging images";
    GradientType zeroG; zeroG.fill(0.0);
    boost::progress_display disp(this->GetOutput(0)->GetLargestPossibleRegion().GetNumberOfPixels());
    while(!itOut.IsAtEnd())
    {
        ++disp;
        DwiPixelType out;
        out.SetSize(m_NumGradients);
        out.Fill(0);

        int c=0;
        for (int i=0; i<m_GradientLists.size(); i++)
        {
            double bValue = m_BValues.at(i);
            GradientListType::Pointer gradients = m_GradientLists.at(i);
            typename DwiImageType::Pointer img = m_ImageVolumes.at(i);

            for (int j=0; j<gradients->Size(); j++)
            {
                GradientType g = gradients->GetElement(j);

                if (g.magnitude()>0.0001)
                {
                    g.normalize();
                    g *= std::sqrt(bValue/m_BValue);
                }
                else
                    g = zeroG;

                m_OutputGradients->InsertElement(c, g);
                out[c] = static_cast<TScalarType>(img->GetPixel(itOut.GetIndex())[j]);
                c++;
            }
        }

        itOut.Set(out);
        ++itOut;
    }
}

} // end of namespace
