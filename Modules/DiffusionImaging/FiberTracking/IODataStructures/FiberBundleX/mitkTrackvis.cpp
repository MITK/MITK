#include <mitkTrackvis.h>

TrackVis::TrackVis()  { filename = ""; fp = NULL; maxSteps = 20000; }

TrackVis::~TrackVis() { if (fp) fclose( fp ); }


// Create a TrackVis file and store standard metadata. The file is ready to append fibers.
// ---------------------------------------------------------------------------------------
short TrackVis::create(string filename , itk::Size<3> size)
{
    // prepare the header
    for(int i=0; i<3 ;i++)
    {
        hdr.dim[i]            = size.GetElement(i);
        hdr.voxel_size[i]     = 1;
        hdr.origin[i]         = 0;
    }
    hdr.n_scalars = 0;
    hdr.n_properties = 0;
    sprintf(hdr.voxel_order,"RAS");
    sprintf(hdr.pad2,"LPS");
    hdr.image_orientation_patient[0] = 1.0;
    hdr.image_orientation_patient[1] = 0.0;
    hdr.image_orientation_patient[2] = 0.0;
    hdr.image_orientation_patient[3] = 0.0;
    hdr.image_orientation_patient[4] = 1.0;
    hdr.image_orientation_patient[5] = 0.0;
    hdr.pad1[0] = 0;
    hdr.pad1[1] = 0;
    hdr.invert_x = 0;
    hdr.invert_y = 0;
    hdr.invert_z = 0;
    hdr.swap_xy = 0;
    hdr.swap_yz = 0;
    hdr.swap_zx = 0;
    hdr.n_count = 0;
    hdr.version = 1;
    hdr.hdr_size = 1000;

    // write the header to the file
    fp = fopen(filename.c_str(),"w+b");
    if (fp == NULL)
    {
        printf("[ERROR] Unable to create file '%s'\n",filename.c_str());
        return 0;
    }
    sprintf(hdr.id_string,"TRACK");
    if (fwrite((char*)&hdr, 1, 1000, fp) != 1000)
      MITK_ERROR << "TrackVis::create : Error occurding during writing fiber.";

    this->filename = filename;

    return 1;
}


// Open an existing TrackVis file and read metadata information.
// The file pointer is positiond at the beginning of fibers data
// -------------------------------------------------------------
short TrackVis::open( string filename )
{
    fp = fopen(filename.c_str(),"r+b");
    if (fp == NULL)
    {
        printf("[ERROR] Unable to open file '%s'\n",filename.c_str());
        return 0;
    }
    this->filename = filename;

    return fread((char*)(&hdr), 1, 1000, fp);
}



// Append a fiber to the file
// --------------------------
short TrackVis::append(mitk::FiberBundleX *fib)
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

        if ( numPoints > maxSteps )
        {
            printf( "[ERROR] Trying to write a fiber too long!\n" );
            return 0;
        }

        numSaved = numPoints;
        for(unsigned int i=0; i<numSaved ;i++)
        {
            double* p = points->GetPoint(i);

            tmp[pos++] = -p[0];
            tmp[pos++] = -p[1];
            tmp[pos++] = p[2];
        }

        // write the coordinates to the file
        if ( fwrite((char*)&numSaved, 1, 4, fp) != 4 )
        {
            printf( "[ERROR] Problems saving the fiber!\n" );
            return 1;
        }
        if ( fwrite((char*)&(tmp.front()), 1, 4*pos, fp) != 4*pos )
        {
            printf( "[ERROR] Problems saving the fiber!\n" );
            return 1;
        }
    }

    return 0;
}



//// Read one fiber from the file
//// ----------------------------
short TrackVis::read( mitk::FiberBundleX* fib )
{
    int numPoints;
    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    while (fread((char*)&numPoints, 1, 4, fp)==4)
    {
        if ( numPoints >= maxSteps || numPoints <= 0 )
        {
            printf( "[ERROR] Trying to read a fiber with %d points!\n", numPoints );
            return -1;
        }
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();

        float tmp[3];
        for(int i=0; i<numPoints; i++)
        {
            if (fread((char*)tmp, 1, 12, fp) == 0)
              MITK_ERROR << "TrackVis::read: Error during read.";

            tmp[0] = -tmp[0];
            tmp[1] = -tmp[1];
            vtkIdType id = vtkNewPoints->InsertNextPoint(tmp);
            container->GetPointIds()->InsertNextId(id);
        }
        vtkNewCells->InsertNextCell(container);
    }

    vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    fiberPolyData->SetPoints(vtkNewPoints);
    fiberPolyData->SetLines(vtkNewCells);
    fib->SetFiberPolyData(fiberPolyData);

    return numPoints;
}



// Update the field in the header to the new FIBER TOTAL.
// ------------------------------------------------------
void TrackVis::updateTotal( int totFibers )
{
    fseek(fp, 1000-12, SEEK_SET);
    if (fwrite((char*)&totFibers, 1, 4, fp) != 4)
      MITK_ERROR << "[ERROR] Problems saving the fiber!";
}


void TrackVis::writeHdr()
{
    fseek(fp, 0, SEEK_SET);
    if (fwrite((char*)&hdr, 1, 1000, fp) != 1000)
      MITK_ERROR << "[ERROR] Problems saving the fiber!";
}


// Close the TrackVis file, but keep the metadata in the header.
// -------------------------------------------------------------
void TrackVis::close()
{
    fclose(fp);
    fp = NULL;
}
