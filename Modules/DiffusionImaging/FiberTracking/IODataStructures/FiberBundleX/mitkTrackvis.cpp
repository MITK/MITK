#include <mitkTrackvis.h>
#include <vtkTransformPolyDataFilter.h>

TrackVisFiberReader::TrackVisFiberReader()  { m_Filename = ""; m_FilePointer = NULL; }

TrackVisFiberReader::~TrackVisFiberReader() { if (m_FilePointer) fclose( m_FilePointer ); }


// Create a TrackVis file and store standard metadata. The file is ready to append fibers.
// ---------------------------------------------------------------------------------------
short TrackVisFiberReader::create(string filename , mitk::FiberBundleX *fib)
{
    // prepare the header
    for(int i=0; i<3 ;i++)
    {
        if (fib->GetReferenceImage().IsNotNull())
        {
            m_Header.dim[i]            = fib->GetReferenceImage()->GetDimension(i);
            m_Header.voxel_size[i]     = fib->GetReferenceImage()->GetGeometry()->GetSpacing().GetElement(i);
            m_Header.origin[i]         = fib->GetReferenceImage()->GetGeometry()->GetOrigin().GetElement(i);
        }
        else
        {
            m_Header.dim[i]            = 1;
            m_Header.voxel_size[i]     = 1;
            m_Header.origin[i]         = 0;
        }
    }
    m_Header.n_scalars = 0;
    m_Header.n_properties = 0;
    sprintf(m_Header.voxel_order,"LPS");
    sprintf(m_Header.pad2,"LPS");
    m_Header.image_orientation_patient[0] = 1.0;
    m_Header.image_orientation_patient[1] = 0.0;
    m_Header.image_orientation_patient[2] = 0.0;
    m_Header.image_orientation_patient[3] = 0.0;
    m_Header.image_orientation_patient[4] = 1.0;
    m_Header.image_orientation_patient[5] = 0.0;
    m_Header.pad1[0] = 0;
    m_Header.pad1[1] = 0;
    m_Header.invert_x = 0;
    m_Header.invert_y = 0;
    m_Header.invert_z = 0;
    m_Header.swap_xy = 0;
    m_Header.swap_yz = 0;
    m_Header.swap_zx = 0;
    m_Header.n_count = 0;
    m_Header.version = 1;
    m_Header.hdr_size = 1000;

    // write the header to the file
    m_FilePointer = fopen(filename.c_str(),"w+b");
    if (m_FilePointer == NULL)
    {
        printf("[ERROR] Unable to create file '%s'\n",filename.c_str());
        return 0;
    }
    sprintf(m_Header.id_string,"TRACK");
    if (fwrite((char*)&m_Header, 1, 1000, m_FilePointer) != 1000)
        MITK_ERROR << "TrackVis::create : Error occurding during writing fiber.";

    this->m_Filename = filename;

    return 1;
}


// Open an existing TrackVis file and read metadata information.
// The file pointer is positiond at the beginning of fibers data
// -------------------------------------------------------------
short TrackVisFiberReader::open( string filename )
{
    m_FilePointer = fopen(filename.c_str(),"r+b");
    if (m_FilePointer == NULL)
    {
        printf("[ERROR] Unable to open file '%s'\n",filename.c_str());
        return 0;
    }
    this->m_Filename = filename;

    return fread((char*)(&m_Header), 1, 1000, m_FilePointer);
}



// Append a fiber to the file
// --------------------------
short TrackVisFiberReader::append(mitk::FiberBundleX *fib)
{
    vtkPolyData* poly = fib->GetFiberPolyData();
    for (int i=0; i<fib->GetNumFibers(); i++)
    {
        vtkCell* cell = poly->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        unsigned int     numSaved, pos = 0;
        //float* tmp = new float[3*maxSteps];
        std::vector< float > tmp;
        tmp.reserve(3*numPoints);

        numSaved = numPoints;
        for(unsigned int i=0; i<numSaved ;i++)
        {
            double* p = points->GetPoint(i);

            tmp[pos++] = p[0];
            tmp[pos++] = p[1];
            tmp[pos++] = p[2];
        }

        // write the coordinates to the file
        if ( fwrite((char*)&numSaved, 1, 4, m_FilePointer) != 4 )
        {
            printf( "[ERROR] Problems saving the fiber!\n" );
            return 1;
        }
        if ( fwrite((char*)&(tmp.front()), 1, 4*pos, m_FilePointer) != 4*pos )
        {
            printf( "[ERROR] Problems saving the fiber!\n" );
            return 1;
        }
    }

    return 0;
}

//// Read one fiber from the file
//// ----------------------------
short TrackVisFiberReader::read( mitk::FiberBundleX* fib )
{
    int numPoints;
    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    while (fread((char*)&numPoints, 1, 4, m_FilePointer)==4)
    {
        if ( numPoints <= 0 )
        {
            printf( "[ERROR] Trying to read a fiber with %d points!\n", numPoints );
            return -1;
        }
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();

        float tmp[3];
        for(int i=0; i<numPoints; i++)
        {
            if (fread((char*)tmp, 1, 12, m_FilePointer) == 0)
                MITK_ERROR << "TrackVis::read: Error during read.";

            //            tmp[0] *= hdr.voxel_size[0];
            //            tmp[1] *= hdr.voxel_size[1];
            //            tmp[2] *= hdr.voxel_size[2];

            //            tmp[0] += hdr.origin[0];
            //            tmp[1] += hdr.origin[1];
            //            tmp[2] += hdr.origin[2];

            vtkIdType id = vtkNewPoints->InsertNextPoint(tmp);
            container->GetPointIds()->InsertNextId(id);
        }
        vtkNewCells->InsertNextCell(container);
    }

    vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    fiberPolyData->SetPoints(vtkNewPoints);
    fiberPolyData->SetLines(vtkNewCells);

    MITK_INFO << "Coordinate convention: " << m_Header.voxel_order;

    mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
    vtkSmartPointer< vtkMatrix4x4 > matrix = vtkSmartPointer< vtkMatrix4x4 >::New();
    matrix->Identity();
    if (IsTransformValid())
    {
        matrix->SetElement(0,0,m_Header.image_orientation_patient[0]);
        matrix->SetElement(0,1,m_Header.image_orientation_patient[1]);
        matrix->SetElement(0,2,m_Header.image_orientation_patient[2]);
        matrix->SetElement(1,1,m_Header.image_orientation_patient[3]);
        matrix->SetElement(1,2,m_Header.image_orientation_patient[4]);
        matrix->SetElement(2,2,m_Header.image_orientation_patient[5]);
    }

    if (m_Header.voxel_order[0]=='R')
        matrix->SetElement(0,0,-matrix->GetElement(0,0));
    if (m_Header.voxel_order[1]=='A')
        matrix->SetElement(1,1,-matrix->GetElement(1,1));
    if (m_Header.voxel_order[2]=='I')
        matrix->SetElement(2,2,-matrix->GetElement(2,2));

    geometry->SetIndexToWorldTransformByVtkMatrix(matrix);

    mitk::Point3D origin;
    origin[0]=m_Header.origin[0];
    origin[1]=m_Header.origin[1];
    origin[2]=m_Header.origin[2];
    geometry->SetOrigin(origin);

    mitk::Vector3D spacing;
    spacing[0]=m_Header.voxel_size[0];
    spacing[1]=m_Header.voxel_size[1];
    spacing[2]=m_Header.voxel_size[2];
    geometry->SetSpacing(spacing);

//    matrix = geometry->GetVtkMatrix();
//    for (int i=0; i<3; i++)
//        MITK_INFO << matrix->GetElement(i,i);

    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputData(fiberPolyData);
    transformFilter->SetTransform(geometry->GetVtkTransform());
    transformFilter->Update();

    fib->SetFiberPolyData(transformFilter->GetOutput());

    return numPoints;
}



// Update the field in the header to the new FIBER TOTAL.
// ------------------------------------------------------
void TrackVisFiberReader::updateTotal( int totFibers )
{
    fseek(m_FilePointer, 1000-12, SEEK_SET);
    if (fwrite((char*)&totFibers, 1, 4, m_FilePointer) != 4)
        MITK_ERROR << "[ERROR] Problems saving the fiber!";
}


void TrackVisFiberReader::writeHdr()
{
    fseek(m_FilePointer, 0, SEEK_SET);
    if (fwrite((char*)&m_Header, 1, 1000, m_FilePointer) != 1000)
        MITK_ERROR << "[ERROR] Problems saving the fiber!";
}


// Close the TrackVis file, but keep the metadata in the header.
// -------------------------------------------------------------
void TrackVisFiberReader::close()
{
    fclose(m_FilePointer);
    m_FilePointer = NULL;
}

bool TrackVisFiberReader::IsTransformValid()
{
    if (fabs(m_Header.image_orientation_patient[0])<=0.001 || fabs(m_Header.image_orientation_patient[3])<=0.001 || fabs(m_Header.image_orientation_patient[5])<=0.001)
        return false;
    return true;
}
