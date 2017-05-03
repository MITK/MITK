#include "itkTractsToVectorImageFilter.h"

// VTK
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>

// ITK
#include <itkTimeProbe.h>
#include <itkImageRegionIterator.h>

// misc
#define _USE_MATH_DEFINES
#include <math.h>
#include <boost/progress.hpp>


namespace itk{

static bool CompareVectorLengths(const vnl_vector_fixed< double, 3 >& v1, const vnl_vector_fixed< double, 3 >& v2)
{
    return (v1.magnitude()>v2.magnitude());
}

template< class PixelType >
TractsToVectorImageFilter< PixelType >::TractsToVectorImageFilter():
    m_AngularThreshold(0.7),
    m_Epsilon(0.999),
    m_MaskImage(NULL),
    m_NormalizeVectors(false),
    m_UseWorkingCopy(true),
    m_MaxNumDirections(3),
    m_SizeThreshold(0.3),
    m_NumDirectionsImage(NULL),
    m_CreateDirectionImages(true)
{
    this->SetNumberOfRequiredOutputs(1);
}


template< class PixelType >
TractsToVectorImageFilter< PixelType >::~TractsToVectorImageFilter()
{
}


template< class PixelType >
vnl_vector_fixed<double, 3> TractsToVectorImageFilter< PixelType >::GetVnlVector(double point[])
{
    vnl_vector_fixed<double, 3> vnlVector;
    vnlVector[0] = point[0];
    vnlVector[1] = point[1];
    vnlVector[2] = point[2];
    return vnlVector;
}


template< class PixelType >
itk::Point<double, 3> TractsToVectorImageFilter< PixelType >::GetItkPoint(double point[])
{
    itk::Point<double, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
}

template< class PixelType >
void TractsToVectorImageFilter< PixelType >::GenerateData()
{
    mitk::BaseGeometry::Pointer geometry = m_FiberBundle->GetGeometry();

    // calculate new image parameters
    itk::Vector<double> spacing3;
    itk::Point<double> origin3;
    itk::Matrix<double, 3, 3> direction3;
    ImageRegion<3> imageRegion3;
    if (!m_MaskImage.IsNull())
    {
        spacing3 = m_MaskImage->GetSpacing();
        imageRegion3 = m_MaskImage->GetLargestPossibleRegion();
        origin3 = m_MaskImage->GetOrigin();
        direction3 = m_MaskImage->GetDirection();
    }
    else
    {
        spacing3 = geometry->GetSpacing();
        origin3 = geometry->GetOrigin();
        mitk::BaseGeometry::BoundsArrayType bounds = geometry->GetBounds();
        origin3[0] += bounds.GetElement(0);
        origin3[1] += bounds.GetElement(2);
        origin3[2] += bounds.GetElement(4);

        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
                direction3[j][i] = geometry->GetMatrixColumn(i)[j];
        imageRegion3.SetSize(0, geometry->GetExtent(0)+1);
        imageRegion3.SetSize(1, geometry->GetExtent(1)+1);
        imageRegion3.SetSize(2, geometry->GetExtent(2)+1);


        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( spacing3 );
        m_MaskImage->SetOrigin( origin3 );
        m_MaskImage->SetDirection( direction3 );
        m_MaskImage->SetRegions( imageRegion3 );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);
    }
    OutputImageType::RegionType::SizeType outImageSize = imageRegion3.GetSize();
    m_OutImageSpacing = m_MaskImage->GetSpacing();
    m_ClusteredDirectionsContainer = ContainerType::New();

    // initialize num directions image
    m_NumDirectionsImage = ItkUcharImgType::New();
    m_NumDirectionsImage->SetSpacing( spacing3 );
    m_NumDirectionsImage->SetOrigin( origin3 );
    m_NumDirectionsImage->SetDirection( direction3 );
    m_NumDirectionsImage->SetRegions( imageRegion3 );
    m_NumDirectionsImage->Allocate();
    m_NumDirectionsImage->FillBuffer(0);

    itk::Vector<double, 4> spacing4;
    itk::Point<float, 4> origin4;
    itk::Matrix<double, 4, 4> direction4;
    itk::ImageRegion<4> imageRegion4;

    spacing4[0] = spacing3[0]; spacing4[1] = spacing3[1]; spacing4[2] = spacing3[2]; spacing4[3] = 1;
    origin4[0] = origin3[0]; origin4[1] = origin3[1]; origin4[2] = origin3[2]; origin3[3] = 0;
    for (int r=0; r<3; r++)
      for (int c=0; c<3; c++)
        direction4[r][c] = direction3[r][c];
    direction4[3][3] = 1;
    imageRegion4.SetSize(0, imageRegion3.GetSize()[0]);
    imageRegion4.SetSize(1, imageRegion3.GetSize()[1]);
    imageRegion4.SetSize(2, imageRegion3.GetSize()[2]);
    imageRegion4.SetSize(3, m_MaxNumDirections*3);

    m_DirectionImage = ItkDirectionImageType::New();
    m_DirectionImage->SetSpacing( spacing4 );
    m_DirectionImage->SetOrigin( origin4 );
    m_DirectionImage->SetDirection( direction4 );
    m_DirectionImage->SetRegions( imageRegion4 );
    m_DirectionImage->Allocate();
    m_DirectionImage->FillBuffer(0.0);

    // resample fiber bundle
    double minSpacing = 1;
    if(m_OutImageSpacing[0]<m_OutImageSpacing[1] && m_OutImageSpacing[0]<m_OutImageSpacing[2])
        minSpacing = m_OutImageSpacing[0];
    else if (m_OutImageSpacing[1] < m_OutImageSpacing[2])
        minSpacing = m_OutImageSpacing[1];
    else
        minSpacing = m_OutImageSpacing[2];

    if (m_UseWorkingCopy)
        m_FiberBundle = m_FiberBundle->GetDeepCopy();

    // resample fiber bundle for sufficient voxel coverage
    m_FiberBundle->ResampleLinear(minSpacing/10);

    // iterate over all fibers
    vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();
    int numFibers = m_FiberBundle->GetNumFibers();
    m_DirectionsContainer = ContainerType::New();

    VectorContainer< unsigned int, std::vector< double > >::Pointer peakLengths = VectorContainer< unsigned int, std::vector< double > >::New();

    MITK_INFO << "Generating directions from tractogram";
    boost::progress_display disp(numFibers);
    for( int i=0; i<numFibers; i++ )
    {
        ++disp;
        vtkCell* cell = fiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();
        if (numPoints<2)
            continue;

        vnl_vector_fixed<double, 3> dir;
        itk::Point<double, 3> worldPos;
        vnl_vector<double> v;


        float fiberWeight = m_FiberBundle->GetFiberWeight(i);

        for( int j=0; j<numPoints-1; j++)
        {
            // get current position along fiber in world coordinates
            double* temp = points->GetPoint(j);
            worldPos = GetItkPoint(temp);
            itk::Index<3> index;
            m_MaskImage->TransformPhysicalPointToIndex(worldPos, index);
            if (!m_MaskImage->GetLargestPossibleRegion().IsInside(index) || m_MaskImage->GetPixel(index)==0)
                continue;

            // get fiber tangent direction at this position
            v = GetVnlVector(temp);
            dir = GetVnlVector(points->GetPoint(j+1))-v;
            if (dir.is_zero())
                continue;
            dir.normalize();

            // add direction to container
            unsigned int idx = index[0] + outImageSize[0]*(index[1] + outImageSize[1]*index[2]);
            DirectionContainerType::Pointer dirCont;
            if (m_DirectionsContainer->IndexExists(idx))
            {
                peakLengths->ElementAt(idx).push_back(fiberWeight);

                dirCont = m_DirectionsContainer->GetElement(idx);
                if (dirCont.IsNull())
                {
                    dirCont = DirectionContainerType::New();
                    dirCont->push_back(dir);
                    m_DirectionsContainer->InsertElement(idx, dirCont);
                }
                else
                    dirCont->push_back(dir);
            }
            else
            {
                dirCont = DirectionContainerType::New();
                dirCont->push_back(dir);
                m_DirectionsContainer->InsertElement(idx, dirCont);

                std::vector< double > lengths; lengths.push_back(fiberWeight);
                peakLengths->InsertElement(idx, lengths);
            }
        }
    }

    vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints>    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

    itk::ImageRegionIterator<ItkUcharImgType> dirIt(m_NumDirectionsImage, m_NumDirectionsImage->GetLargestPossibleRegion());

    MITK_INFO << "Clustering directions";
    boost::progress_display disp2(outImageSize[0]*outImageSize[1]*outImageSize[2]);
    while(!dirIt.IsAtEnd())
    {
        ++disp2;
        OutputImageType::IndexType idx3 = dirIt.GetIndex();
        int idx_lin = idx3[0]+(idx3[1]+idx3[2]*outImageSize[1])*outImageSize[0];

        itk::Index<4> idx4; idx4[0] = idx3[0]; idx4[1] = idx3[1]; idx4[2] = idx3[2];

        if (!m_DirectionsContainer->IndexExists(idx_lin))
        {
            ++dirIt;
            continue;
        }
        DirectionContainerType::Pointer dirCont = m_DirectionsContainer->GetElement(idx_lin);
        if (dirCont.IsNull() || dirCont->empty())
        {
            ++dirIt;
            continue;
        }

//        std::vector< double > lengths; lengths.resize(dirCont->size(), 1);  // all peaks have size 1
        DirectionContainerType::Pointer directions;
        if (m_MaxNumDirections>0)
        {
            directions = FastClustering(dirCont, peakLengths->GetElement(idx_lin));
            std::sort( directions->begin(), directions->end(), CompareVectorLengths );
        }
        else
            directions = dirCont;

        unsigned int numDir = directions->size();
        if (m_MaxNumDirections>0 && numDir>m_MaxNumDirections)
            numDir = m_MaxNumDirections;

        int count = 0;
        for (unsigned int i=0; i<numDir; i++)
        {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            itk::ContinuousIndex<double, 3> center;
            center[0] = idx3[0];
            center[1] = idx3[1];
            center[2] = idx3[2];
            itk::Point<double> worldCenter;
            m_MaskImage->TransformContinuousIndexToPhysicalPoint( center, worldCenter );
            DirectionType dir = directions->at(i);

            if (dir.magnitude()<m_SizeThreshold)
                continue;
            if (m_NormalizeVectors)
                dir.normalize();
            count++;

            for (unsigned int j = 0; j<3; j++)
            {
              idx4[3] = i*3 + j;
              m_DirectionImage->SetPixel(idx4, dir[j]);
            }

            // add direction to vector field (with spacing compensation)
            itk::Point<double> worldStart;
            worldStart[0] = worldCenter[0]-dir[0]/2*minSpacing;
            worldStart[1] = worldCenter[1]-dir[1]/2*minSpacing;
            worldStart[2] = worldCenter[2]-dir[2]/2*minSpacing;
            vtkIdType id = m_VtkPoints->InsertNextPoint(worldStart.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
            itk::Point<double> worldEnd;
            worldEnd[0] = worldCenter[0]+dir[0]/2*minSpacing;
            worldEnd[1] = worldCenter[1]+dir[1]/2*minSpacing;
            worldEnd[2] = worldCenter[2]+dir[2]/2*minSpacing;
            id = m_VtkPoints->InsertNextPoint(worldEnd.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
            m_VtkCellArray->InsertNextCell(container);
        }
        dirIt.Set(count);
        ++dirIt;
    }

    vtkSmartPointer<vtkPolyData> directionsPolyData = vtkSmartPointer<vtkPolyData>::New();
    directionsPolyData->SetPoints(m_VtkPoints);
    directionsPolyData->SetLines(m_VtkCellArray);
    m_OutputFiberBundle = mitk::FiberBundle::New(directionsPolyData);
}


template< class PixelType >
TractsToVectorImageFilter< PixelType >::DirectionContainerType::Pointer TractsToVectorImageFilter< PixelType >::FastClustering(DirectionContainerType::Pointer inDirs, std::vector< double > lengths)
{
    DirectionContainerType::Pointer outDirs = DirectionContainerType::New();
    if (inDirs->size()<2)
        return inDirs;

    DirectionType oldMean, currentMean;
    std::vector< int > touched;

    // initialize
    touched.resize(inDirs->size(), 0);
    bool free = true;
    currentMean = inDirs->at(0);  // initialize first seed
    currentMean.normalize();
    double length = lengths.at(0);
    touched[0] = 1;
    std::vector< double > newLengths;
    bool meanChanged = false;

    double max = 0;
    while (free)
    {
        oldMean.fill(0.0);

        // start mean-shift clustering
        double angle = 0;

        while (fabs(dot_product(currentMean, oldMean))<0.99)
        {
            oldMean = currentMean;
            currentMean.fill(0.0);
            for (unsigned int i=0; i<inDirs->size(); i++)
            {
                angle = dot_product(oldMean, inDirs->at(i));
                if (angle>=m_AngularThreshold)
                {
                    currentMean += inDirs->at(i);
                    if (meanChanged)
                        length += lengths.at(i);
                    touched[i] = 1;
                    meanChanged = true;
                }
                else if (-angle>=m_AngularThreshold)
                {
                    currentMean -= inDirs->at(i);
                    if (meanChanged)
                        length += lengths.at(i);
                    touched[i] = 1;
                    meanChanged = true;
                }
            }
            if(!meanChanged)
                currentMean = oldMean;
            else
                currentMean.normalize();
        }

        // found stable mean
        outDirs->push_back(currentMean);
        newLengths.push_back(length);
        if (length>max)
            max = length;

        // find next unused seed
        free = false;
        for (unsigned int i=0; i<touched.size(); i++)
            if (touched[i]==0)
            {
                currentMean = inDirs->at(i);
                free = true;
                meanChanged = false;
                length = lengths.at(i);
                touched[i] = 1;
                break;
            }
    }

    if (inDirs->size()==outDirs->size())
    {
        if (max>0)
            for (unsigned int i=0; i<outDirs->size(); i++)
                outDirs->SetElement(i, outDirs->at(i)*newLengths.at(i)/max);
        return outDirs;
    }
    else
        return FastClustering(outDirs, newLengths);
}

}



