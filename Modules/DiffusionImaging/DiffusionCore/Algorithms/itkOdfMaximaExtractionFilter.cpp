
#ifndef __itkOdfMaximaExtractionFilter_cpp
#define __itkOdfMaximaExtractionFilter_cpp


#include "itkOdfMaximaExtractionFilter.h"
#include <itkImageRegionIterator.h>
#include <itkContinuousIndex.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

#include <boost/progress.hpp>
#include <boost/math/special_functions.hpp>
#include <vnl/vnl_det.h>
#include <vnl/vnl_trace.h>

using namespace boost::math;
using namespace std;

namespace itk {

template< class TOdfPixelType >
OdfMaximaExtractionFilter< TOdfPixelType >::OdfMaximaExtractionFilter():
    m_NormalizationMethod(MAX_VEC_NORM),
    m_PeakThreshold(0.2),
    m_MaxNumPeaks(10),
    m_ShCoeffImage(NULL),
    m_OutputFiberBundle(NULL),
    m_NumDirectionsImage(NULL),
    m_DirectionImageContainer(NULL)
{

}

// solve ax? + bx? + cx + d = 0 using cardanos method
template< class TOdfPixelType >
bool OdfMaximaExtractionFilter<TOdfPixelType>::ReconstructQballImage()
{
    if (m_ShCoeffImage.IsNotNull())
    {
        cout << "Using preset coefficient image\n";
        return true;
    }

    cout << "Starting qball reconstruction\n";
    try {
        QballReconstructionFilterType::Pointer filter = QballReconstructionFilterType::New();
        filter->SetGradientImage( m_DiffusionGradients, m_DiffusionImage );
        filter->SetBValue(m_Bvalue);
        filter->SetLambda(0.006);
        filter->SetNormalizationMethod(QballReconstructionFilterType::QBAR_SOLID_ANGLE);
        filter->Update();
        m_ShCoeffImage = filter->GetCoefficientImage();
        if (m_ShCoeffImage.IsNull())
            return false;
        return true;
    }
    catch (...)
    {
        return false;
    }
}

// solve ax³ + bx² + cx + d = 0 using cardanos method
template< class TOdfPixelType >
std::vector<double> OdfMaximaExtractionFilter< TOdfPixelType >
::SolveCubic(const double& a, const double& b, const double& c, const double& d)
{
    double A, B, p, q, r, D, offset, ee, tmp, root;
    vector<double> roots;
    double inv3 = 1.0/3.0;

    if (a!=0) // solve ax³ + bx² + cx + d = 0
    {
        p = b/a; q = c/a; r = d/a; // x³ + px² + qx + r = 0
        A = q-p*p*inv3;
        B = (2.0*p*p*p-9.0*p*q+27.0*r)/27.0;
        A = A*inv3;
        B = B*0.5;
        D = B*B+A*A*A;
        offset = p*inv3;

        if (D>0.0) // one real root
        {
            ee = sqrt(D);
            tmp = -B+ee;  root  = cbrt(tmp);
            tmp = -B-ee;  root += cbrt(tmp);
            root -= offset; roots.push_back(root);
        }
        else if (D<0.0) // three real roots
        {
            ee = sqrt(-D);
            double tmp2 = -B;
            double angle =  2.0*inv3*atan(ee/(sqrt(tmp2*tmp2+ee*ee)+tmp2));
            double sqrt3 = sqrt(3.0);
            tmp = cos(angle);
            tmp2 = sin(angle);
            ee = sqrt(-A);
            root = 2*ee*tmp-offset;             roots.push_back(root);
            root = -ee*(tmp+sqrt3*tmp2)-offset; roots.push_back(root);
            root = -ee*(tmp-sqrt3*tmp2)-offset; roots.push_back(root);
        }
        else // one or two real roots
        {
            tmp=-B;
            tmp=cbrt(tmp);
            root=2*tmp-offset;      roots.push_back(root);
            if (A!=0 || B!=0)
                root=-tmp-offset;   roots.push_back(root);
        }
    }
    else if (b!=0) // solve bx² + cx + d = 0
    {
        D = c*c-4*b*d;
        if (D>0)
        {
            tmp = sqrt(D);
            root = (-c+tmp)/(2.0*b); roots.push_back(root);
            root = (-c-tmp)/(2.0*b); roots.push_back(root);
        }
        else if (D==0)
            root = -c/(2.0*b); roots.push_back(root);
    }
    else if (c!=0) // solve cx + d = 0
        root = -d/c; roots.push_back(root);

    return roots;
}

template< class TOdfPixelType >
double OdfMaximaExtractionFilter< TOdfPixelType >
::ODF_dtheta(const double& sn, const double& cs, const double& A, const double& B, const double& C, const double& D, const double& E, const double& F, const double& G, const double& H)
{
    double dtheta=(G-7*E)*sn*sn + (7*F-35*D-H)*sn*cs + (H+C-F-3*A-5*D)*sn + (0.5*E+B+0.5*G)*cs -0.5*G+3.5*E;
    return dtheta;
}

template< class TOdfPixelType >
double OdfMaximaExtractionFilter< TOdfPixelType >
::ODF_dtheta2(const double& sn, const double& cs, const double& A, const double& B, const double& C, const double& D, const double& E, const double& F, const double& G, const double& H)
{
    double dtheta2=4*(G-7*E)*sn*cs + 2*(7*F-35*D-H)*(2*cs*cs-1) + 2*(H+C-F-3*A-5*D)*cs -(E+2*B+G)*sn;
    return dtheta2;
}

template< class TOdfPixelType >
double OdfMaximaExtractionFilter< TOdfPixelType >
::ODF_dphi2(const double& sn, const double& cs, const double& A, const double& B, const double& C, const double& D, const double& E, const double& F, const double& G, const double& H)
{
    double dphi2=35*D*((1+cs)*(1+cs)/4)+(3*A-30*D)*(1+cs)/2.0+3*D-A + 0.5*(7*E*(1+cs)/2.0-3*E+B)*sn + (7*F*(1+cs)/2+C-F)*(1-cs)/2.0 + G*sn*(1-cs)/4.0 + H*((1-cs)*(1-cs)/4);
    return dphi2;
}

template< class TOdfPixelType >
void OdfMaximaExtractionFilter< TOdfPixelType >
::FindCandidatePeaks(const CoefficientPixelType& SHcoeff)
{
    const double thr = 0.03;             // threshold on the derivative of the ODF with respect to theta
    const double phi_step = 0.005;       // step size for 1D exhaustive search on phi
    bool highRes;                        // when close to maxima increase resolution
    double mag, Y, Yp, sn, cs;
    double phi, dPhi;
    double A, B, C, D, E, F, G, H, Bp, Cp, Ep, Fp, Gp, Hp, Bs, Cs, Es, Fs, Gs, Hs;
    CoefficientPixelType a, ap;
    a = SHcoeff; ap = SHcoeff;

    m_CandidatePeaks.clear();   // clear peaks of last voxel

    for (int adaptiveStepwidth=0; adaptiveStepwidth<=1; adaptiveStepwidth++)
    {
    phi=0;
    while (phi<(2*M_PI)) // phi exhaustive search 0..pi
    {
        // calculate 4th order SH representtaion of ODF and according derivative
        for (int l=0; l<=4; l=l+2)
        {
            for (int m=-l; m<=l; m++)
            {
                int j=l*(l+1)/2+m;
                if (m<0)
                {
                    mag = sqrt(((2*l+1)/(2*M_PI))*factorial<double>(l+m)/factorial<double>(l-m));
                    Y = mag*cos(m*phi);
                    Yp = -m*mag*sin(m*phi);
                }
                else if (m==0)
                {
                    Y = sqrt((2*l+1)/(4*M_PI));
                    Yp = 0;
                }
                else
                {
                    mag = pow(-1.0,m)*sqrt(((2*l+1)/(2*M_PI))*factorial<double>(l-m)/factorial<double>(l+m));
                    Y = mag*sin(m*phi);
                    Yp = m*mag*cos(m*phi);
                }
                a[j] = SHcoeff[j]*Y;
                ap[j] = SHcoeff[j]*Yp;
            }
        }

        // ODF
        A=0.5*a[3]; B=-3*(a[2]+a[4]); C=3*(a[1]+a[5]); D=0.125*a[10]; E=-2.5*(a[9]+a[11]);
        F=7.5*(a[8]+a[12]); G=-105*(a[7]+a[13]); H=105*(a[6]+a[14]);

        // phi derivative
        Bp=-3*(ap[2]+ap[4]); Cp=3*(ap[1]+ap[5]); Ep=-2.5*(ap[9]+ap[11]);
        Fp=7.5*(ap[8]+ap[12]); Gp=-105*(ap[7]+ap[13]); Hp=105*(ap[6]+ap[14]);

        // 2phi derivative
        Bs=-B;    Cs=-4*C;  Es=-E;
        Fs=-4*F;  Gs=-9*G;  Hs=-16*H;

        // solve cubic for tan(theta)
        std::vector<double> tanTheta = SolveCubic(Hp+Cp-Fp, Gp+Bp-3*Ep, 6*Fp+Cp, Bp+4*Ep);

        highRes = false;
        dPhi = phi_step;

        //for each real cubic solution for tan(theta)
        for (int n=0; n<tanTheta.size(); n++)
        {
            double tmp = atan(tanTheta[n]); // arcus tangens of root (theta -pi/2..pi/2)
            double theta = floor(tmp/M_PI); // project theta to 0..pi ...
            theta = tmp - theta*M_PI;       // ... as the modulo of the division atan(tth[n])/M_PI

            sn = sin(2*theta); cs = cos(2*theta);
            tmp = ODF_dtheta(sn, cs, A, B, C, D, E, F, G, H);

            if (fabs(tmp) < thr) // second condition for maximum is true (theta derivative < eps)
            {
                //Compute the Hessian
                vnl_matrix_fixed< double, 2, 2 > hessian;
                hessian(0,0) = ODF_dtheta2(sn, cs, A, B, C, D, E, F, G, H);
                hessian(0,1) = ODF_dtheta(sn, cs, 0, Bp, Cp, 0, Ep, Fp, Gp, Hp);
                hessian(1,0) = hessian(0,1);
                hessian(1,1) = ODF_dphi2(sn, cs, 0, Bs, Cs, 0, Es, Fs, Gs, Hs);

                double det = vnl_det(hessian);  // determinant
                double tr = vnl_trace(hessian); // trace

                highRes = true; // we are close to a maximum, so turn on high resolution 1D exhaustive search
                if (det>=0 && tr<=0) // check if we really have a local maximum
                {
                    vnl_vector_fixed< double, 2 > peak;
                    peak[0] = theta;
                    peak[1] = phi;
                    m_CandidatePeaks.push_back(peak);
                }
            }

            if (adaptiveStepwidth) // calculate adaptive step width
            {
                double t2=tanTheta[n]*tanTheta[n];  double t3=t2*tanTheta[n]; double t4=t3*tanTheta[n];
                double const_step=phi_step*(1+t2)/sqrt(t2+t4+pow((((Hs+Cs-Fs)*t3+(Gs+Bs-3*Es)*t2+(6*Fs+Cs)*tanTheta[n]+(Bs+4*Es))/(3*(Hp+Cp-Fp)*t2+2*(Gp+Bp-3*Ep)*tanTheta[n]+(6*Fp+Cp))),2.0));
                if (const_step<dPhi)
                    dPhi=const_step;
            }
        }

        // update phi
        if (highRes)
            phi=phi+dPhi*0.5;
        else
            phi=phi+dPhi;
    }
    }
}

template< class TOdfPixelType >
std::vector< vnl_vector_fixed< double, 3 > > OdfMaximaExtractionFilter< TOdfPixelType >
::ClusterPeaks(const CoefficientPixelType& shCoeff)
{
    const double distThres = 0.4;
    int npeaks = 0, nMin = 0;
    double dMin, dPos, dNeg, d;
    Vector3D u;
    vector< Vector3D > v;

    // initialize container for vector clusters
    std::vector < std::vector< Vector3D > > clusters;
    clusters.resize(m_CandidatePeaks.size());

    for (int i=0; i<m_CandidatePeaks.size(); i++)
    {
        // calculate cartesian representation of peak
        u[0] = sin(m_CandidatePeaks[i](0))*cos(m_CandidatePeaks[i](1));
        u[1] = sin(m_CandidatePeaks[i](0))*sin(m_CandidatePeaks[i](1));
        u[2] = cos(m_CandidatePeaks[i](0));

        dMin = itk::NumericTraits<double>::max();
        for (int n=0; n<npeaks; n++) //for each other maximum v already visited
        {
            // euclidean distance from u/-u to other clusters
            dPos = vnl_vector_ssd(v[n],u);
            dNeg = vnl_vector_ssd(v[n],-u);
            d = std::min(dPos,dNeg);

            if ( d<dMin )
            {
                dMin = d; // adjust minimum
                nMin = n; // store its index
                if ( dNeg<dPos ) // flip u if neccesary
                    u=-u;
            }
        }
        if ( dMin<distThres ) // if u is very close to any other maximum v
        {
            clusters[nMin].push_back(u);  //store it with all other vectors that are close to v vector (with index nMin)
        }
        else // otherwise store u as output peak
        {
            v.push_back(u);
            npeaks++;
        }
    }

    // calculate mean vector of each cluster
    for (int i=0; i<m_CandidatePeaks.size(); i++)
        if ( !clusters[i].empty() )
        {
            v[i].fill(0.0);
            for (int vc=0; vc<clusters[i].size(); vc++)
                v[i]=v[i]+clusters[i][vc];
            v[i].normalize();
        }


    if (npeaks!=0)
    {
        // check the ODF amplitudes at each candidate peak
        vnl_matrix< double > shBasis, sphCoords;

        Cart2Sph(v, sphCoords);                 // convert candidate peaks to spherical angles
        shBasis = CalcShBasis(sphCoords, 4);  // evaluate spherical harmonics at each peak
        vnl_vector<double> odfVals(npeaks);
        odfVals.fill(0.0);
        double maxVal = itk::NumericTraits<double>::NonpositiveMin();
        int maxPos;
        for (int i=0; i<npeaks; i++) //compute the ODF value at each peak
        {
            for (int j=0; j<15; j++)
                odfVals(i) += shCoeff[j]*shBasis(i,j);

            if ( odfVals(i)>maxVal )
            {
                maxVal = odfVals(i);
                maxPos = i;
            }
        }
        v.clear();
        vector< double > restVals;
        for (int i=0; i<npeaks; i++) // keep only peaks with high enough amplitude and convert back to cartesian coordinates
            if ( odfVals(i)>=m_PeakThreshold*maxVal )
            {
                u[0] = odfVals(i)*cos(sphCoords(i,1))*sin(sphCoords(i,0));
                u[1] = odfVals(i)*sin(sphCoords(i,1))*sin(sphCoords(i,0));
                u[2] = odfVals(i)*cos(sphCoords(i,0));
                restVals.push_back(odfVals(i));
                v.push_back(u);
            }
        npeaks = v.size();

        if (npeaks>m_MaxNumPeaks) // if still too many peaks, keep only the m_MaxNumPeaks with maximum value
        {
            vector< Vector3D > v2;
            for (int i=0; i<m_MaxNumPeaks; i++)
            {
                maxVal = itk::NumericTraits<double>::NonpositiveMin();  //Get the maximum ODF peak value and the corresponding peak index
                for (int i=0; i<npeaks; i++)
                    if ( restVals[i]>maxVal )
                    {
                        maxVal = restVals[i];
                        maxPos = i;
                    }

                v2.push_back(v[maxPos]);
                restVals[maxPos] = 0;               //zero that entry in order to find the next maximum
            }
            return v2;
        }
    }
    return v;
}

// convert cartesian to spherical coordinates
template< class TOdfPixelType >
void OdfMaximaExtractionFilter< TOdfPixelType >
::Cart2Sph(const std::vector< Vector3D >& dir, vnl_matrix<double>& sphCoords)
{
    sphCoords.set_size(dir.size(), 2);

    for (int i=0; i<dir.size(); i++)
    {
        double mag = dir[i].magnitude();

        if( mag<mitk::eps )
        {
            sphCoords(i,0) = M_PI/2; // theta
            sphCoords(i,1) = M_PI/2; // phi
        }
        else
        {
            sphCoords(i,0) = acos(dir[i](2)/mag); // theta
            sphCoords(i,1) = atan2(dir[i](1), dir[i](0)); // phi
        }
    }
}

// generate spherical harmonic values of the desired order for each input direction
template< class TOdfPixelType >
vnl_matrix<double> OdfMaximaExtractionFilter< TOdfPixelType >
::CalcShBasis(vnl_matrix<double>& sphCoords, const int& shOrder)
{
    int R = (shOrder+1)*(shOrder+2)/2;
    int M = sphCoords.rows();
    int j, m; double mag, plm;
    vnl_matrix<double> shBasis;
    shBasis.set_size(M,R);

    for (int p=0; p<M; p++)
    {
        j=0;
        for (int l=0; l<=shOrder; l=l+2)
            for (m=-l; m<=l; m++)
            {
                plm = legendre_p<double>(l,abs(m),cos(sphCoords(p,0)));
                mag = sqrt((double)(2*l+1)/(4.0*M_PI)*factorial<double>(l-abs(m))/factorial<double>(l+abs(m)))*plm;

                if (m<0)
                    shBasis(p,j) = sqrt(2.0)*mag*cos(fabs((double)m)*sphCoords(p,1));
                else if (m==0)
                    shBasis(p,j) = mag;
                else
                    shBasis(p,j) = pow(-1.0, m)*sqrt(2.0)*mag*sin(m*sphCoords(p,1));
                j++;
            }
    }
    return shBasis;
}

template< class TOdfPixelType >
void OdfMaximaExtractionFilter< TOdfPixelType >
::GenerateData()
{
    if (!ReconstructQballImage())
        return;

    std::cout << "Starting maxima extraction\n";

    switch (m_NormalizationMethod)
    {
    case NO_NORM:
        std::cout << "NO_NORM\n";
        break;
    case SINGLE_VEC_NORM:
        std::cout << "SINGLE_VEC_NORM\n";
        break;
    case MAX_VEC_NORM:
        std::cout << "MAX_VEC_NORM\n";
        break;
    }

    typedef ImageRegionConstIterator< CoefficientImageType > InputIteratorType;


    InputIteratorType git(m_ShCoeffImage, m_ShCoeffImage->GetLargestPossibleRegion() );

    mitk::Vector3D spacing = m_ShCoeffImage->GetSpacing();
    double minSpacing = spacing[0];
    if (spacing[1]<minSpacing)
        minSpacing = spacing[1];
    if (spacing[2]<minSpacing)
        minSpacing = spacing[2];

    mitk::Point3D origin = m_ShCoeffImage->GetOrigin();
    itk::Matrix<double, 3, 3> direction = m_ShCoeffImage->GetDirection();
    ImageRegion<3> imageRegion = m_ShCoeffImage->GetLargestPossibleRegion();

    // initialize num directions image
    m_NumDirectionsImage = ItkUcharImgType::New();
    m_NumDirectionsImage->SetSpacing( spacing );
    m_NumDirectionsImage->SetOrigin( origin );
    m_NumDirectionsImage->SetDirection( direction );
    m_NumDirectionsImage->SetRegions( imageRegion );
    m_NumDirectionsImage->Allocate();
    m_NumDirectionsImage->FillBuffer(0);

    vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints>    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

    m_DirectionImageContainer = ItkDirectionImageContainer::New();
    for (int i=0; i<m_MaxNumPeaks; i++)
    {
        itk::Vector< float, 3 > nullVec; nullVec.Fill(0.0);
        ItkDirectionImage::Pointer img = ItkDirectionImage::New();
        img->SetSpacing( spacing );
        img->SetOrigin( origin );
        img->SetDirection( direction );
        img->SetRegions( imageRegion );
        img->Allocate();
        img->FillBuffer(nullVec);
        m_DirectionImageContainer->InsertElement(m_DirectionImageContainer->Size(), img);
    }

    if (m_MaskImage.IsNull())
    {
        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( spacing );
        m_MaskImage->SetOrigin( origin );
        m_MaskImage->SetDirection( direction );
        m_MaskImage->SetRegions( imageRegion );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);
    }

    itk::ImageRegionIterator<ItkUcharImgType> dirIt(m_NumDirectionsImage, m_NumDirectionsImage->GetLargestPossibleRegion());
    itk::ImageRegionIterator<ItkUcharImgType> maskIt(m_MaskImage, m_MaskImage->GetLargestPossibleRegion());

    int maxProgress = m_MaskImage->GetLargestPossibleRegion().GetSize()[0]*m_MaskImage->GetLargestPossibleRegion().GetSize()[1]*m_MaskImage->GetLargestPossibleRegion().GetSize()[2];

    boost::progress_display disp(maxProgress);

    git.GoToBegin();
    while( !git.IsAtEnd() )
    {
        ++disp;
        if (maskIt.Value()<=0)
        {
            ++git;
            ++dirIt;
            ++maskIt;
            continue;
        }

        CoefficientPixelType c = git.Get();
        FindCandidatePeaks(c);
        std::vector< Vector3D > directions = ClusterPeaks(c);

        typename CoefficientImageType::IndexType index = git.GetIndex();

        float max = 0.0;
        for (int i=0; i<directions.size(); i++)
            if (directions.at(i).magnitude()>max)
                max = directions.at(i).magnitude();
        if (max<0.0001)
            max = 1.0;

        for (int i=0; i<directions.size(); i++)
        {
            ItkDirectionImage::Pointer img = m_DirectionImageContainer->GetElement(i);
            itk::Vector< float, 3 > pixel;
            vnl_vector<double> dir = directions.at(i);

            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            itk::ContinuousIndex<double, 3> center;
            center[0] = index[0];
            center[1] = index[1];
            center[2] = index[2];
            itk::Point<double> worldCenter;
            m_ShCoeffImage->TransformContinuousIndexToPhysicalPoint( center, worldCenter );

            switch (m_NormalizationMethod)
            {
            case NO_NORM:
                break;
            case SINGLE_VEC_NORM:
                dir.normalize();
                break;
            case MAX_VEC_NORM:
                dir /= max;
                break;
            }

            dir = m_MaskImage->GetDirection()*dir;
            pixel.SetElement(0, dir[0]);
            pixel.SetElement(1, dir[1]);
            pixel.SetElement(2, dir[2]);
            img->SetPixel(index, pixel);

            itk::Point<double> worldStart;
            worldStart[0] = worldCenter[0]-dir[0]/2 * minSpacing;
            worldStart[1] = worldCenter[1]-dir[1]/2 * minSpacing;
            worldStart[2] = worldCenter[2]-dir[2]/2 * minSpacing;
            vtkIdType id = m_VtkPoints->InsertNextPoint(worldStart.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
            itk::Point<double> worldEnd;
            worldEnd[0] = worldCenter[0]+dir[0]/2 * minSpacing;
            worldEnd[1] = worldCenter[1]+dir[1]/2 * minSpacing;
            worldEnd[2] = worldCenter[2]+dir[2]/2 * minSpacing;
            id = m_VtkPoints->InsertNextPoint(worldEnd.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
            m_VtkCellArray->InsertNextCell(container);
        }

        dirIt.Set(directions.size());

        ++git;
        ++dirIt;
        ++maskIt;
    }

    vtkSmartPointer<vtkPolyData> directionsPolyData = vtkSmartPointer<vtkPolyData>::New();
    directionsPolyData->SetPoints(m_VtkPoints);
    directionsPolyData->SetLines(m_VtkCellArray);
    m_OutputFiberBundle = mitk::FiberBundleX::New(directionsPolyData);
    std::cout << "Maxima extraction finished\n";
}
}

#endif // __itkOdfMaximaExtractionFilter_cpp
