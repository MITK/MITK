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

TractsToVectorImageFilter::TractsToVectorImageFilter():
    m_AngularThreshold(0.7),
    m_MaskImage(NULL),
    m_NumDirectionsImage(NULL),
    m_NormalizeVectors(false),
    m_Epsilon(0.999),
    m_UseWorkingCopy(true),
    m_MaxNumDirections(3),
    m_FiberSampling(5),
    m_UseTrilinearInterpolation(true),
    m_Thres(0.5)
{
    this->SetNumberOfRequiredOutputs(1);
}


TractsToVectorImageFilter::~TractsToVectorImageFilter()
{
}


vnl_vector_fixed<double, 3> TractsToVectorImageFilter::GetVnlVector(double point[3])
{
    vnl_vector_fixed<double, 3> vnlVector;
    vnlVector[0] = point[0];
    vnlVector[1] = point[1];
    vnlVector[2] = point[2];
    return vnlVector;
}


itk::Point<float, 3> TractsToVectorImageFilter::GetItkPoint(double point[3])
{
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
}

void TractsToVectorImageFilter::GenerateData()
{
    mitk::Geometry3D::Pointer geometry = m_FiberBundle->GetGeometry();

    // calculate new image parameters
    mitk::Vector3D spacing;
    mitk::Point3D origin;
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
        mitk::Geometry3D::BoundsArrayType bounds = geometry->GetBounds();
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

    // initialize crossings image
    m_CrossingsImage = ItkUcharImgType::New();
    m_CrossingsImage->SetSpacing( spacing );
    m_CrossingsImage->SetOrigin( origin );
    m_CrossingsImage->SetDirection( direction );
    m_CrossingsImage->SetRegions( imageRegion );
    m_CrossingsImage->Allocate();
    m_CrossingsImage->FillBuffer(0);

    // initialize num directions image
    m_NumDirectionsImage = ItkUcharImgType::New();
    m_NumDirectionsImage->SetSpacing( spacing );
    m_NumDirectionsImage->SetOrigin( origin );
    m_NumDirectionsImage->SetDirection( direction );
    m_NumDirectionsImage->SetRegions( imageRegion );
    m_NumDirectionsImage->Allocate();
    m_NumDirectionsImage->FillBuffer(0);

    // resample fiber bundle
    float minSpacing = 1;
    if(m_OutImageSpacing[0]<m_OutImageSpacing[1] && m_OutImageSpacing[0]<m_OutImageSpacing[2])
        minSpacing = m_OutImageSpacing[0];
    else if (m_OutImageSpacing[1] < m_OutImageSpacing[2])
        minSpacing = m_OutImageSpacing[1];
    else
        minSpacing = m_OutImageSpacing[2];

    if (m_UseWorkingCopy)
        m_FiberBundle = m_FiberBundle->GetDeepCopy();

    if (m_FiberSampling>0)
        m_FiberBundle->DoFiberSmoothing(ceil(10.0*m_FiberSampling/minSpacing));

    // iterate over all fibers
    vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();
    vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
    vLines->InitTraversal();
    int numFibers = m_FiberBundle->GetNumFibers();
    itk::TimeProbe clock;
    m_DirectionsContainer = ContainerType::New();

    MITK_INFO << "Generating fODFs (trilinear interpolation)";
    boost::progress_display disp(numFibers);
    for( int i=0; i<numFibers; i++ )
    {
        ++disp;
        clock.Start();
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );
        if (numPoints<2)
            continue;

        itk::Index<3> index; index.Fill(0);
        itk::ContinuousIndex<float, 3> contIndex;
        vnl_vector_fixed<double, 3> dir, wDir;
        itk::Point<float, 3> worldPos;
        vnl_vector<double> v;
        for( int j=0; j<numPoints-1; j++)
        {
            double* temp = fiberPolyData->GetPoint(points[j]);
            worldPos = GetItkPoint(temp);
            v = GetVnlVector(temp);

            dir = GetVnlVector(fiberPolyData->GetPoint(points[j+1]))-v;
            dir.normalize();

            m_MaskImage->TransformPhysicalPointToIndex(worldPos, index);
            m_MaskImage->TransformPhysicalPointToContinuousIndex(worldPos, contIndex);

            if (m_MaskImage->GetPixel(index)==0)
                continue;

            if (!m_UseTrilinearInterpolation)
            {
                if (index[0] < 0 || index[0] >= outImageSize[0])
                    continue;
                if (index[1] < 0 || index[1] >= outImageSize[1])
                    continue;
                if (index[2] < 0 || index[2] >= outImageSize[2])
                    continue;

                int idx = index[0] + outImageSize[0]*(index[1] + outImageSize[1]*index[2]);
                DirectionContainerType::Pointer dirCont = DirectionContainerType::New();
                if (m_DirectionsContainer->IndexExists(idx))
                {
                    dirCont = m_DirectionsContainer->GetElement(idx);
                    if (dirCont.IsNull())
                    {
                        dirCont = DirectionContainerType::New();
                        dirCont->InsertElement(0, dir);
                        m_DirectionsContainer->InsertElement(idx, dirCont);
                    }
                    else
                        dirCont->InsertElement(dirCont->Size(), dir);
                }
                else
                {
                    dirCont->InsertElement(0, dir);
                    m_DirectionsContainer->InsertElement(idx, dirCont);
                }

                continue;
            }

            float frac_x = contIndex[0] - index[0];
            float frac_y = contIndex[1] - index[1];
            float frac_z = contIndex[2] - index[2];

            if (frac_x<0)
            {
                index[0] -= 1;
                frac_x += 1;
            }
            if (frac_y<0)
            {
                index[1] -= 1;
                frac_y += 1;
            }
            if (frac_z<0)
            {
                index[2] -= 1;
                frac_z += 1;
            }

            frac_x = 1-frac_x;
            frac_y = 1-frac_y;
            frac_z = 1-frac_z;

            // int coordinates inside image?
            if (index[0] < 0 || index[0] >= outImageSize[0]-1)
                continue;
            if (index[1] < 0 || index[1] >= outImageSize[1]-1)
                continue;
            if (index[2] < 0 || index[2] >= outImageSize[2]-1)
                continue;

            DirectionContainerType::Pointer dirCont;
            int idx;
            wDir = dir;
            float weight = (  frac_x)*(  frac_y)*(  frac_z);
            if (weight>m_Thres)
            {
                wDir *= weight;
                idx = index[0]   + outImageSize[0]*(index[1]  + outImageSize[1]*index[2]  );
                dirCont = DirectionContainerType::New();
                if (m_DirectionsContainer->IndexExists(idx))
                {
                    dirCont = m_DirectionsContainer->GetElement(idx);
                    if (dirCont.IsNull())
                    {
                        dirCont = DirectionContainerType::New();
                        dirCont->InsertElement(0, wDir);
                        m_DirectionsContainer->InsertElement(idx, dirCont);
                    }
                    else
                        dirCont->InsertElement(dirCont->Size(), wDir);
                }
                else
                {
                    dirCont->InsertElement(0, wDir);
                    m_DirectionsContainer->InsertElement(idx, dirCont);
                }
            }

            wDir = dir;
            weight = (  frac_x)*(1-frac_y)*(  frac_z);
            if (weight>m_Thres)
            {
                wDir *= weight;
                idx = index[0]   + outImageSize[0]*(index[1]+1+ outImageSize[1]*index[2]  );
                dirCont = DirectionContainerType::New();
                if (m_DirectionsContainer->IndexExists(idx))
                {
                    dirCont = m_DirectionsContainer->GetElement(idx);
                    if (dirCont.IsNull())
                    {
                        dirCont = DirectionContainerType::New();
                        dirCont->InsertElement(0, wDir);
                        m_DirectionsContainer->InsertElement(idx, dirCont);
                    }
                    else
                        dirCont->InsertElement(dirCont->Size(), wDir);
                }
                else
                {
                    dirCont->InsertElement(0, wDir);
                    m_DirectionsContainer->InsertElement(idx, dirCont);
                }
            }

            wDir = dir;
            weight = (  frac_x)*(  frac_y)*(1-frac_z);
            if (weight>m_Thres)
            {
                wDir *= weight;
                idx = index[0]   + outImageSize[0]*(index[1]  + outImageSize[1]*index[2]+outImageSize[1]);
                dirCont = DirectionContainerType::New();
                if (m_DirectionsContainer->IndexExists(idx))
                {
                    dirCont = m_DirectionsContainer->GetElement(idx);
                    if (dirCont.IsNull())
                    {
                        dirCont = DirectionContainerType::New();
                        dirCont->InsertElement(0, wDir);
                        m_DirectionsContainer->InsertElement(idx, dirCont);
                    }
                    else
                        dirCont->InsertElement(dirCont->Size(), wDir);
                }
                else
                {
                    dirCont->InsertElement(0, wDir);
                    m_DirectionsContainer->InsertElement(idx, dirCont);
                }
            }

            wDir = dir;
            weight = (  frac_x)*(1-frac_y)*(1-frac_z);
            if (weight>m_Thres)
            {
                wDir *= weight;
                idx = index[0]   + outImageSize[0]*(index[1]+1+ outImageSize[1]*index[2]+outImageSize[1]);
                dirCont = DirectionContainerType::New();
                if (m_DirectionsContainer->IndexExists(idx))
                {
                    dirCont = m_DirectionsContainer->GetElement(idx);
                    if (dirCont.IsNull())
                    {
                        dirCont = DirectionContainerType::New();
                        dirCont->InsertElement(0, wDir);
                        m_DirectionsContainer->InsertElement(idx, dirCont);
                    }
                    else
                        dirCont->InsertElement(dirCont->Size(), wDir);
                }
                else
                {
                    dirCont->InsertElement(0, wDir);
                    m_DirectionsContainer->InsertElement(idx, dirCont);
                }
            }

            wDir = dir;
            weight = (1-frac_x)*(  frac_y)*(  frac_z);
            if (weight>m_Thres)
            {
                wDir *= weight;
                idx = index[0]+1 + outImageSize[0]*(index[1]  + outImageSize[1]*index[2]  );
                dirCont = DirectionContainerType::New();
                if (m_DirectionsContainer->IndexExists(idx))
                {
                    dirCont = m_DirectionsContainer->GetElement(idx);
                    if (dirCont.IsNull())
                    {
                        dirCont = DirectionContainerType::New();
                        dirCont->InsertElement(0, wDir);
                        m_DirectionsContainer->InsertElement(idx, dirCont);
                    }
                    else
                        dirCont->InsertElement(dirCont->Size(), wDir);
                }
                else
                {
                    dirCont->InsertElement(0, wDir);
                    m_DirectionsContainer->InsertElement(idx, dirCont);
                }
            }

            wDir = dir;
            weight = (1-frac_x)*(  frac_y)*(1-frac_z);
            if (weight>m_Thres)
            {
                wDir *= weight;
                idx = index[0]+1 + outImageSize[0]*(index[1]  + outImageSize[1]*index[2]+outImageSize[1]);
                dirCont = DirectionContainerType::New();
                if (m_DirectionsContainer->IndexExists(idx))
                {
                    dirCont = m_DirectionsContainer->GetElement(idx);
                    if (dirCont.IsNull())
                    {
                        dirCont = DirectionContainerType::New();
                        dirCont->InsertElement(0, wDir);
                        m_DirectionsContainer->InsertElement(idx, dirCont);
                    }
                    else
                        dirCont->InsertElement(dirCont->Size(), wDir);
                }
                else
                {
                    dirCont->InsertElement(0, wDir);
                    m_DirectionsContainer->InsertElement(idx, dirCont);
                }
            }

            wDir = dir;
            weight = (1-frac_x)*(1-frac_y)*(  frac_z);
            if (weight>m_Thres)
            {
                wDir *= weight;
                idx = index[0]+1 + outImageSize[0]*(index[1]+1+ outImageSize[1]*index[2]  );
                dirCont = DirectionContainerType::New();
                if (m_DirectionsContainer->IndexExists(idx))
                {
                    dirCont = m_DirectionsContainer->GetElement(idx);
                    if (dirCont.IsNull())
                    {
                        dirCont = DirectionContainerType::New();
                        dirCont->InsertElement(0, wDir);
                        m_DirectionsContainer->InsertElement(idx, dirCont);
                    }
                    else
                        dirCont->InsertElement(dirCont->Size(), wDir);
                }
                else
                {
                    dirCont->InsertElement(0, wDir);
                    m_DirectionsContainer->InsertElement(idx, dirCont);
                }
            }

            wDir = dir;
            weight = (1-frac_x)*(1-frac_y)*(1-frac_z);
            if (weight>m_Thres)
            {
                wDir *= weight;
                idx = index[0]+1 + outImageSize[0]*(index[1]+1+ outImageSize[1]*index[2]+outImageSize[1]);
                dirCont = DirectionContainerType::New();
                if (m_DirectionsContainer->IndexExists(idx))
                {
                    dirCont = m_DirectionsContainer->GetElement(idx);
                    if (dirCont.IsNull())
                    {
                        dirCont = DirectionContainerType::New();
                        dirCont->InsertElement(0, wDir);
                        m_DirectionsContainer->InsertElement(idx, dirCont);
                    }
                    else
                        dirCont->InsertElement(dirCont->Size(), wDir);
                }
                else
                {
                    dirCont->InsertElement(0, wDir);
                    m_DirectionsContainer->InsertElement(idx, dirCont);
                }
            }
        }
        clock.Stop();
    }

    vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints>    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

    itk::ImageRegionIterator<ItkUcharImgType> dirIt(m_NumDirectionsImage, m_NumDirectionsImage->GetLargestPossibleRegion());
    itk::ImageRegionIterator<ItkUcharImgType> crossIt(m_CrossingsImage, m_CrossingsImage->GetLargestPossibleRegion());

    m_DirectionImageContainer = DirectionImageContainerType::New();
    int maxNumDirections = 0;

    MITK_INFO << "Clustering directions";
    boost::progress_display disp2(outImageSize[0]*outImageSize[1]*outImageSize[2]);
    for(crossIt.GoToBegin(); !crossIt.IsAtEnd(); ++crossIt)
    {
        ++disp2;
        OutputImageType::IndexType index = crossIt.GetIndex();
        int idx = index[0]+(index[1]+index[2]*outImageSize[1])*outImageSize[0];

        if (!m_DirectionsContainer->IndexExists(idx))
        {
            ++dirIt;
            continue;
        }
        DirectionContainerType::Pointer dirCont = m_DirectionsContainer->GetElement(idx);
        if (dirCont.IsNull() || index[0] < 0 || index[0] >= outImageSize[0] || index[1] < 0 || index[1] >= outImageSize[1] || index[2] < 0 || index[2] >= outImageSize[2])
        {
            ++dirIt;
            continue;
        }

        std::vector< DirectionType > directions;

        for (int i=0; i<dirCont->Size(); i++)
            if (dirCont->ElementAt(i).magnitude()>0.0001)
                directions.push_back(dirCont->ElementAt(i));

        if (!directions.empty())
            directions = FastClustering(directions);

        std::sort( directions.begin(), directions.end(), CompareVectorLengths );

        if ( directions.size() > maxNumDirections )
        {
            for (int i=maxNumDirections; i<std::min((int)directions.size(), m_MaxNumDirections); i++)
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
            maxNumDirections = std::min((int)directions.size(), m_MaxNumDirections);
        }

        int numDir = directions.size();
        if (numDir>m_MaxNumDirections)
            numDir = m_MaxNumDirections;

        for (int i=0; i<numDir; i++)
        {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            itk::ContinuousIndex<float, 3> center;
            center[0] = index[0];
            center[1] = index[1];
            center[2] = index[2];
            itk::Point<float> worldCenter;
            m_MaskImage->TransformContinuousIndexToPhysicalPoint( center, worldCenter );

            DirectionType dir = m_MaskImage->GetDirection()*directions.at(i);

            // set direction image pixel
            ItkDirectionImageType::Pointer directionImage = m_DirectionImageContainer->GetElement(i);
            Vector< float, 3 > pixel;
            pixel.SetElement(0, dir[0]);
            pixel.SetElement(1, dir[1]);
            pixel.SetElement(2, dir[2]);
            directionImage->SetPixel(index, pixel);

            // add direction to vector field (with spacing compensation)
            itk::Point<float> worldStart;
            worldStart[0] = worldCenter[0]-dir[0]/2*minSpacing;
            worldStart[1] = worldCenter[1]-dir[1]/2*minSpacing;
            worldStart[2] = worldCenter[2]-dir[2]/2*minSpacing;
            vtkIdType id = m_VtkPoints->InsertNextPoint(worldStart.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
            itk::Point<float> worldEnd;
            worldEnd[0] = worldCenter[0]+dir[0]/2*minSpacing;
            worldEnd[1] = worldCenter[1]+dir[1]/2*minSpacing;
            worldEnd[2] = worldCenter[2]+dir[2]/2*minSpacing;
            id = m_VtkPoints->InsertNextPoint(worldEnd.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
            m_VtkCellArray->InsertNextCell(container);
        }
        dirIt.Set(numDir);
        ++dirIt;
    }

    vtkSmartPointer<vtkPolyData> directionsPolyData = vtkSmartPointer<vtkPolyData>::New();
    directionsPolyData->SetPoints(m_VtkPoints);
    directionsPolyData->SetLines(m_VtkCellArray);
    m_OutputFiberBundle = mitk::FiberBundleX::New(directionsPolyData);

    MITK_INFO << "mean time per fiber: " << clock.GetMean() << "s";
    MITK_INFO << "absolute time: " << clock.GetTotal()/60 << "min";
}


std::vector< vnl_vector_fixed< double, 3 > > TractsToVectorImageFilter::FastClustering(std::vector< vnl_vector_fixed< double, 3 > >& inDirs)
{
    std::vector< vnl_vector_fixed< double, 3 > > outDirs;
    if (inDirs.empty())
        return outDirs;
    vnl_vector_fixed< double, 3 > oldMean, currentMean, workingMean;

    std::vector< vnl_vector_fixed< double, 3 > > normalizedDirs;
    std::vector< int > touched;
    for (int i=0; i<inDirs.size(); i++)
    {
        normalizedDirs.push_back(inDirs[i]);
        normalizedDirs.back().normalize();
    }

    // initialize
    float max = 0.0;
    touched.resize(inDirs.size(), 0);
    bool free = true;
    currentMean = inDirs[0];  // initialize first seed
    while (free)
    {
        oldMean.fill(0.0);

        // start mean-shift clustering
        float angle = 0.0;
        int counter = 0;
        while ((currentMean-oldMean).magnitude()>0.0001)
        {
            counter = 0;
            oldMean = currentMean;
            workingMean = oldMean;
            workingMean.normalize();
            currentMean.fill(0.0);
            for (int i=0; i<normalizedDirs.size(); i++)
            {
                angle = dot_product(workingMean, normalizedDirs[i]);
                if (angle>=m_AngularThreshold)
                {
                    currentMean += inDirs[i];
                    touched[i] = 1;
                    counter++;
                }
                else if (-angle>=m_AngularThreshold)
                {
                    currentMean -= inDirs[i];
                    touched[i] = 1;
                    counter++;
                }
            }
        }

        // found stable mean
        if (counter>0)
        {
            currentMean /= counter;
            float mag = currentMean.magnitude();

            if (mag>0)
            {
                if (mag>max)
                    max = mag;

                outDirs.push_back(currentMean);
            }
        }

        // find next unused seed
        free = false;
        for (int i=0; i<touched.size(); i++)
            if (touched[i]==0)
            {
                currentMean = inDirs[i];
                free = true;
            }
    }

    if (m_NormalizeVectors)
        for (int i=0; i<outDirs.size(); i++)
            outDirs[i].normalize();
    else if (max>0)
        for (int i=0; i<outDirs.size(); i++)
            outDirs[i] /= max;

    if (inDirs.size()==outDirs.size())
        return outDirs;
    else
        return FastClustering(outDirs);
}


std::vector< vnl_vector_fixed< double, 3 > > TractsToVectorImageFilter::Clustering(std::vector< vnl_vector_fixed< double, 3 > >& inDirs)
{
    std::vector< vnl_vector_fixed< double, 3 > > outDirs;
    if (inDirs.empty())
        return outDirs;
    vnl_vector_fixed< double, 3 > oldMean, currentMean, workingMean;

    std::vector< vnl_vector_fixed< double, 3 > > normalizedDirs;
    std::vector< int > touched;
    for (int i=0; i<inDirs.size(); i++)
    {
        normalizedDirs.push_back(inDirs[i]);
        normalizedDirs.back().normalize();
    }

    // initialize
    float max = 0.0;
    touched.resize(inDirs.size(), 0);
    for (int j=0; j<inDirs.size(); j++)
    {
        currentMean = inDirs[j];
        oldMean.fill(0.0);

        // start mean-shift clustering
        float angle = 0.0;
        int counter = 0;
        while ((currentMean-oldMean).magnitude()>0.0001)
        {
            counter = 0;
            oldMean = currentMean;
            workingMean = oldMean;
            workingMean.normalize();
            currentMean.fill(0.0);
            for (int i=0; i<normalizedDirs.size(); i++)
            {
                angle = dot_product(workingMean, normalizedDirs[i]);
                if (angle>=m_AngularThreshold)
                {
                    currentMean += inDirs[i];
                    counter++;
                }
                else if (-angle>=m_AngularThreshold)
                {
                    currentMean -= inDirs[i];
                    counter++;
                }
            }
        }

        // found stable mean
        if (counter>0)
        {
            bool add = true;
            vnl_vector_fixed< double, 3 > normMean = currentMean;
            normMean.normalize();
            for (int i=0; i<outDirs.size(); i++)
            {
                vnl_vector_fixed< double, 3 > dir = outDirs[i];
                dir.normalize();
                if ((normMean-dir).magnitude()<=0.0001)
                {
                    add = false;
                    break;
                }
            }

            currentMean /= counter;
            if (add)
            {
                float mag = currentMean.magnitude();
                if (mag>0)
                {
                    if (mag>max)
                        max = mag;

                    outDirs.push_back(currentMean);
                }
            }
        }
    }

    if (m_NormalizeVectors)
        for (int i=0; i<outDirs.size(); i++)
            outDirs[i].normalize();
    else if (max>0)
        for (int i=0; i<outDirs.size(); i++)
            outDirs[i] /= max;

    if (inDirs.size()==outDirs.size())
        return outDirs;
    else
        return FastClustering(outDirs);
}


TractsToVectorImageFilter::DirectionContainerType::Pointer TractsToVectorImageFilter::MeanShiftClustering(DirectionContainerType::Pointer dirCont)
{
    DirectionContainerType::Pointer container = DirectionContainerType::New();

    float max = 0;
    for (DirectionContainerType::ConstIterator it = dirCont->Begin(); it!=dirCont->End(); ++it)
    {
        vnl_vector_fixed<double, 3> mean = ClusterStep(dirCont, it.Value());

        if (mean.is_zero())
            continue;
        bool addMean = true;

        for (DirectionContainerType::ConstIterator it2 = container->Begin(); it2!=container->End(); ++it2)
        {
            vnl_vector_fixed<double, 3> dir = it2.Value();
            float angle = fabs(dot_product(mean, dir)/(mean.magnitude()*dir.magnitude()));
            if (angle>=m_Epsilon)
            {
                addMean = false;
                break;
            }
        }

        if (addMean)
        {
            if (m_NormalizeVectors)
                mean.normalize();
            else if (mean.magnitude()>max)
                max = mean.magnitude();
            container->InsertElement(container->Size(), mean);
        }
    }

    // max normalize voxel directions
    if (max>0 && !m_NormalizeVectors)
        for (int i=0; i<container->Size(); i++)
            container->ElementAt(i) /= max;

    if (container->Size()<dirCont->Size())
        return MeanShiftClustering(container);
    else
        return container;
}


vnl_vector_fixed<double, 3> TractsToVectorImageFilter::ClusterStep(DirectionContainerType::Pointer dirCont, vnl_vector_fixed<double, 3> currentMean)
{
    vnl_vector_fixed<double, 3> newMean; newMean.fill(0);

    for (DirectionContainerType::ConstIterator it = dirCont->Begin(); it!=dirCont->End(); ++it)
    {
        vnl_vector_fixed<double, 3> dir = it.Value();
        float angle = dot_product(currentMean, dir)/(currentMean.magnitude()*dir.magnitude());
        if (angle>=m_AngularThreshold)
            newMean += dir;
        else if (-angle>=m_AngularThreshold)
            newMean -= dir;
    }

    if (fabs(dot_product(currentMean, newMean)/(currentMean.magnitude()*newMean.magnitude()))>=m_Epsilon || newMean.is_zero())
        return newMean;
    else
        return ClusterStep(dirCont, newMean);
}
}



