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
    m_SizeThreshold(0.2),
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
    itk::Vector<double> spacing;
    itk::Point<double> origin;
    itk::Matrix<double, 3, 3> direction;
    ImageRegion<3> imageRegion;
    if (!m_MaskImage.IsNull())
    {
        spacing = m_MaskImage->GetSpacing();
        imageRegion = m_MaskImage->GetLargestPossibleRegion();
        origin = m_MaskImage->GetOrigin();
        direction = m_MaskImage->GetDirection();
    }
    else
    {
        spacing = geometry->GetSpacing();
        origin = geometry->GetOrigin();
        mitk::BaseGeometry::BoundsArrayType bounds = geometry->GetBounds();
        origin[0] += bounds.GetElement(0);
        origin[1] += bounds.GetElement(2);
        origin[2] += bounds.GetElement(4);

        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
                direction[j][i] = geometry->GetMatrixColumn(i)[j];
        imageRegion.SetSize(0, geometry->GetExtent(0));
        imageRegion.SetSize(1, geometry->GetExtent(1));
        imageRegion.SetSize(2, geometry->GetExtent(2));


        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( spacing );
        m_MaskImage->SetOrigin( origin );
        m_MaskImage->SetDirection( direction );
        m_MaskImage->SetRegions( imageRegion );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);
    }
    OutputImageType::RegionType::SizeType outImageSize = imageRegion.GetSize();
    m_OutImageSpacing = m_MaskImage->GetSpacing();
    m_ClusteredDirectionsContainer = ContainerType::New();

    // initialize num directions image
    m_NumDirectionsImage = ItkUcharImgType::New();
    m_NumDirectionsImage->SetSpacing( spacing );
    m_NumDirectionsImage->SetOrigin( origin );
    m_NumDirectionsImage->SetDirection( direction );
    m_NumDirectionsImage->SetRegions( imageRegion );
    m_NumDirectionsImage->Allocate();
    m_NumDirectionsImage->FillBuffer(0);

    // initialize direction images
    m_DirectionImageContainer = DirectionImageContainerType::New();

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
    m_FiberBundle->ResampleSpline(minSpacing/10);

    // iterate over all fibers
    vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();
    int numFibers = m_FiberBundle->GetNumFibers();
    m_DirectionsContainer = ContainerType::New();

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
        OutputImageType::IndexType index = dirIt.GetIndex();
        int idx = index[0]+(index[1]+index[2]*outImageSize[1])*outImageSize[0];

        if (!m_DirectionsContainer->IndexExists(idx))
        {
            ++dirIt;
            continue;
        }
        DirectionContainerType::Pointer dirCont = m_DirectionsContainer->GetElement(idx);
        if (dirCont.IsNull() || dirCont->empty())
        {
            ++dirIt;
            continue;
        }

        std::vector< double > lengths; lengths.resize(dirCont->size(), 1);  // all peaks have size 1
        DirectionContainerType::Pointer directions;
        if (m_MaxNumDirections>0)
        {
            directions = FastClustering(dirCont, lengths);
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
            center[0] = index[0];
            center[1] = index[1];
            center[2] = index[2];
            itk::Point<double> worldCenter;
            m_MaskImage->TransformContinuousIndexToPhysicalPoint( center, worldCenter );
            DirectionType dir = directions->at(i);

            if (dir.magnitude()<m_SizeThreshold)
                continue;
            count++;

//            if (m_NormalizeVectors)
//                dir.normalize();

            if (m_CreateDirectionImages && i<10)
            {
                if (i==m_DirectionImageContainer->size())
                {
                    ItkDirectionImageType::Pointer directionImage = ItkDirectionImageType::New();
                    directionImage->SetSpacing( spacing );
                    directionImage->SetOrigin( origin );
                    directionImage->SetDirection( direction );
                    directionImage->SetRegions( imageRegion );
                    directionImage->Allocate();
                    Vector< float, 3 > nullVec; nullVec.Fill(0.0);
                    directionImage->FillBuffer(nullVec);
                    m_DirectionImageContainer->InsertElement(i, directionImage);
                }

                // set direction image pixel
                ItkDirectionImageType::Pointer directionImage = m_DirectionImageContainer->GetElement(i);
                Vector< float, 3 > pixel;
                pixel.SetElement(0, dir[0]);
                pixel.SetElement(1, dir[1]);
                pixel.SetElement(2, dir[2]);
                directionImage->SetPixel(index, pixel);
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
    m_OutputFiberBundle = mitk::FiberBundleX::New(directionsPolyData);
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
        if (!m_NormalizeVectors && max>0)
            for (unsigned int i=0; i<outDirs->size(); i++)
                outDirs->SetElement(i, outDirs->at(i)*newLengths.at(i)/max);
        return outDirs;
    }
    else
        return FastClustering(outDirs, newLengths);
}


//template< class PixelType >
//std::vector< DirectionType > TractsToVectorImageFilter< PixelType >::Clustering(std::vector< DirectionType >& inDirs)
//{
//    std::vector< DirectionType > outDirs;
//    if (inDirs.empty())
//        return outDirs;
//    DirectionType oldMean, currentMean, workingMean;

//    std::vector< DirectionType > normalizedDirs;
//    std::vector< int > touched;
//    for (std::size_t i=0; i<inDirs.size(); i++)
//    {
//        normalizedDirs.push_back(inDirs[i]);
//        normalizedDirs.back().normalize();
//    }

//    // initialize
//    double max = 0.0;
//    touched.resize(inDirs.size(), 0);
//    for (std::size_t j=0; j<inDirs.size(); j++)
//    {
//        currentMean = inDirs[j];
//        oldMean.fill(0.0);

//        // start mean-shift clustering
//        double angle = 0.0;
//        int counter = 0;
//        while ((currentMean-oldMean).magnitude()>0.0001)
//        {
//            counter = 0;
//            oldMean = currentMean;
//            workingMean = oldMean;
//            workingMean.normalize();
//            currentMean.fill(0.0);
//            for (std::size_t i=0; i<normalizedDirs.size(); i++)
//            {
//                angle = dot_product(workingMean, normalizedDirs[i]);
//                if (angle>=m_AngularThreshold)
//                {
//                    currentMean += inDirs[i];
//                    counter++;
//                }
//                else if (-angle>=m_AngularThreshold)
//                {
//                    currentMean -= inDirs[i];
//                    counter++;
//                }
//            }
//        }

//        // found stable mean
//        if (counter>0)
//        {
//            bool add = true;
//            DirectionType normMean = currentMean;
//            normMean.normalize();
//            for (std::size_t i=0; i<outDirs.size(); i++)
//            {
//                DirectionType dir = outDirs[i];
//                dir.normalize();
//                if ((normMean-dir).magnitude()<=0.0001)
//                {
//                    add = false;
//                    break;
//                }
//            }

//            currentMean /= counter;
//            if (add)
//            {
//                double mag = currentMean.magnitude();
//                if (mag>0)
//                {
//                    if (mag>max)
//                        max = mag;

//                    outDirs.push_back(currentMean);
//                }
//            }
//        }
//    }

//    if (m_NormalizeVectors)
//        for (std::size_t i=0; i<outDirs.size(); i++)
//            outDirs[i].normalize();
//    else if (max>0)
//        for (std::size_t i=0; i<outDirs.size(); i++)
//            outDirs[i] /= max;

//    if (inDirs.size()==outDirs.size())
//        return outDirs;
//    else
//        return FastClustering(outDirs);
//}


//template< class PixelType >
//TractsToVectorImageFilter< PixelType >::DirectionContainerType::Pointer TractsToVectorImageFilter< PixelType >::MeanShiftClustering(DirectionContainerType::Pointer dirCont)
//{
//    DirectionContainerType::Pointer container = DirectionContainerType::New();

//    double max = 0;
//    for (DirectionContainerType::ConstIterator it = dirCont->Begin(); it!=dirCont->End(); ++it)
//    {
//        vnl_vector_fixed<double, 3> mean = ClusterStep(dirCont, it.Value());

//        if (mean.is_zero())
//            continue;
//        bool addMean = true;

//        for (DirectionContainerType::ConstIterator it2 = container->Begin(); it2!=container->End(); ++it2)
//        {
//            vnl_vector_fixed<double, 3> dir = it2.Value();
//            double angle = fabs(dot_product(mean, dir)/(mean.magnitude()*dir.magnitude()));
//            if (angle>=m_Epsilon)
//            {
//                addMean = false;
//                break;
//            }
//        }

//        if (addMean)
//        {
//            if (m_NormalizeVectors)
//                mean.normalize();
//            else if (mean.magnitude()>max)
//                max = mean.magnitude();
//            container->InsertElement(container->Size(), mean);
//        }
//    }

//    // max normalize voxel directions
//    if (max>0 && !m_NormalizeVectors)
//        for (std::size_t i=0; i<container->Size(); i++)
//            container->ElementAt(i) /= max;

//    if (container->Size()<dirCont->Size())
//        return MeanShiftClustering(container);
//    else
//        return container;
//}


//template< class PixelType >
//vnl_vector_fixed<double, 3> TractsToVectorImageFilter< PixelType >::ClusterStep(DirectionContainerType::Pointer dirCont, vnl_vector_fixed<double, 3> currentMean)
//{
//    vnl_vector_fixed<double, 3> newMean; newMean.fill(0);

//    for (DirectionContainerType::ConstIterator it = dirCont->Begin(); it!=dirCont->End(); ++it)
//    {
//        vnl_vector_fixed<double, 3> dir = it.Value();
//        double angle = dot_product(currentMean, dir)/(currentMean.magnitude()*dir.magnitude());
//        if (angle>=m_AngularThreshold)
//            newMean += dir;
//        else if (-angle>=m_AngularThreshold)
//            newMean -= dir;
//    }

//    if (fabs(dot_product(currentMean, newMean)/(currentMean.magnitude()*newMean.magnitude()))>=m_Epsilon || newMean.is_zero())
//        return newMean;
//    else
//        return ClusterStep(dirCont, newMean);
//}
}



