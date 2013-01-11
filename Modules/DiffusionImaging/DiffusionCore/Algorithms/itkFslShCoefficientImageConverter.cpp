
#ifndef __itkFslShCoefficientImageConverter_cpp
#define __itkFslShCoefficientImageConverter_cpp

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkFslShCoefficientImageConverter.h"
#include <itkImageRegionIterator.h>
#include <boost/math/special_functions.hpp>

using namespace boost::math;

namespace itk {

template< class PixelType, int ShOrder >
FslShCoefficientImageConverter< PixelType, ShOrder >::FslShCoefficientImageConverter()
{
    m_ShBasis.set_size(QBALL_ODFSIZE, (ShOrder+1)*(ShOrder+2)/2);
    CalcShBasis();
}

template< class PixelType, int ShOrder >
void FslShCoefficientImageConverter< PixelType, ShOrder >
::GenerateData()
{
    if (m_InputImage.IsNull())
        return;

    Vector<float, 4> spacing4 = m_InputImage->GetSpacing();
    Point<float, 4> origin4 = m_InputImage->GetOrigin();
    Matrix<double, 4, 4> direction4 = m_InputImage->GetDirection();
    ImageRegion<4> imageRegion4 = m_InputImage->GetLargestPossibleRegion();

    Vector<float, 3> spacing3;
    Point<float, 3> origin3;
    Matrix<double, 3, 3> direction3;
    ImageRegion<3> imageRegion3;

    spacing3[0] = spacing4[0]; spacing3[1] = spacing4[1]; spacing3[2] = spacing4[2];
    origin3[0] = origin4[0]; origin3[1] = origin4[1]; origin3[2] = origin4[2];
    for (int r=0; r<3; r++)
        for (int c=0; c<3; c++)
            direction3[r][c] = direction4[r][c];
    imageRegion3.SetSize(0, imageRegion4.GetSize()[0]);
    imageRegion3.SetSize(1, imageRegion4.GetSize()[1]);
    imageRegion3.SetSize(2, imageRegion4.GetSize()[2]);

    m_QballImage = QballImageType::New();
    m_QballImage->SetSpacing( spacing3 );
    m_QballImage->SetOrigin( origin3 );
    m_QballImage->SetDirection( direction3 );
    m_QballImage->SetRegions( imageRegion3 );
    m_QballImage->Allocate();
    Vector< PixelType, QBALL_ODFSIZE > nullVec1; nullVec1.Fill(0.0);
    m_QballImage->FillBuffer(nullVec1);

    m_CoefficientImage = CoefficientImageType::New();
    m_CoefficientImage->SetSpacing( spacing3 );
    m_CoefficientImage->SetOrigin( origin3 );
    m_CoefficientImage->SetDirection( direction3 );
    m_CoefficientImage->SetRegions( imageRegion3 );
    m_CoefficientImage->Allocate();
    Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder > nullVec2; nullVec2.Fill(0.0);
    m_CoefficientImage->FillBuffer(nullVec2);

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    int x = imageRegion4.GetSize(0);
    int y = imageRegion4.GetSize(1);
    int z = imageRegion4.GetSize(2);
    int numCoeffs = imageRegion4.GetSize(3);

    for (int a=0; a<x; a++)
        for (int b=0; b<y; b++)
            for (int c=0; c<z; c++)
            {
                vnl_matrix<double> coeffs((ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder,1);

                typename InputImageType::IndexType index;
                index.SetElement(0,a);
                index.SetElement(1,b);
                index.SetElement(2,c);
                typename CoefficientImageType::PixelType pix;
                for (int d=0; d<numCoeffs; d++)
                {
                    index.SetElement(3,d);
                    pix[d] = m_InputImage->GetPixel(index);
                    coeffs[d][0] = pix[d];
                }
                typename CoefficientImageType::IndexType index2;
                index2.SetElement(0,a);
                index2.SetElement(1,b);
                index2.SetElement(2,c);
                m_CoefficientImage->SetPixel(index2, pix);

                vnl_matrix<double> odf = m_ShBasis*coeffs;
                typename QballImageType::PixelType pix2;
                for (int d=0; d<QBALL_ODFSIZE; d++)
                    pix2[d] = odf(d,0)*M_PI*4/QBALL_ODFSIZE;
                m_QballImage->SetPixel(index2,pix2);
            }

}

// generate spherical harmonic values of the desired order for each input direction
template< class PixelType, int ShOrder >
void FslShCoefficientImageConverter< PixelType, ShOrder >
::CalcShBasis()
{
    vnl_matrix_fixed<double, 2, QBALL_ODFSIZE> sphCoords = GetSphericalOdfDirections();
    int j, m; double mag, plm;

    for (int p=0; p<QBALL_ODFSIZE; p++)
    {
        j=0;
        for (int l=0; l<=ShOrder; l=l+2)
            for (m=-l; m<=l; m++)
            {
                plm = legendre_p<double>(l,abs(m),cos(sphCoords(0,p)));
                mag = sqrt((double)(2*l+1)/(4.0*M_PI)*factorial<double>(l-abs(m))/factorial<double>(l+abs(m)))*plm;

                if (m<0)
                    m_ShBasis(p,j) = sqrt(2.0)*mag*cos(-m*sphCoords(1,p));
                else if (m==0)
                    m_ShBasis(p,j) = mag;
                else
                    m_ShBasis(p,j) = pow(-1.0, m)*sqrt(2.0)*mag*sin(m*sphCoords(1,p));
                j++;
            }
    }
}

// convert cartesian to spherical coordinates
template< class PixelType, int ShOrder >
vnl_matrix_fixed<double, 2, QBALL_ODFSIZE> FslShCoefficientImageConverter< PixelType, ShOrder >
::GetSphericalOdfDirections()
{
    itk::OrientationDistributionFunction< PixelType, QBALL_ODFSIZE > odf;
    vnl_matrix_fixed<double, 3, QBALL_ODFSIZE>* dir = odf.GetDirections();
    vnl_matrix_fixed<double, 2, QBALL_ODFSIZE> sphCoords;

    for (int i=0; i<QBALL_ODFSIZE; i++)
    {
        double mag = dir->get_column(i).magnitude();

        if( mag<mitk::eps )
        {
            sphCoords(0,i) = M_PI/2; // theta
            sphCoords(1,i) = M_PI/2; // phi
        }
        else
        {
            sphCoords(0,i) = acos(dir->get(2,i)/mag); // theta
            sphCoords(1,i) = atan2(dir->get(1,i), dir->get(0,i)); // phi
        }
    }
    return sphCoords;
}

}

#endif // __itkFslShCoefficientImageConverter_cpp
