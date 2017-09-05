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

#include "mitkFiberBundleDicomWriter.h"
#include <vtkSmartPointer.h>
#include <itksys/SystemTools.hxx>
#include <itkSize.h>
#include <vtkFloatArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <mitkAbstractFileWriter.h>
#include <mitkCustomMimeType.h>
#include "mitkDiffusionIOMimeTypes.h"
#include <boost/date_time.hpp>
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmtract/trctractographyresults.h"

mitk::FiberBundleDicomWriter::FiberBundleDicomWriter()
  : mitk::AbstractFileWriter(mitk::FiberBundle::GetStaticNameOfClass(), mitk::DiffusionIOMimeTypes::FIBERBUNDLE_DICOM_MIMETYPE_NAME(), "DICOM Fiber Bundle Writer")
{
  RegisterService();
}

mitk::FiberBundleDicomWriter::FiberBundleDicomWriter(const mitk::FiberBundleDicomWriter & other)
  :mitk::AbstractFileWriter(other)
{}

mitk::FiberBundleDicomWriter::~FiberBundleDicomWriter()
{}

mitk::FiberBundleDicomWriter * mitk::FiberBundleDicomWriter::Clone() const
{
  return new mitk::FiberBundleDicomWriter(*this);
}

void mitk::FiberBundleDicomWriter::Write()
{
  try
  {
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, nullptr );
    setlocale(LC_ALL, locale.c_str());

    // Instance Number, Label, Description, Creator's Name
    ContentIdentificationMacro id("1", "TRACTOGRAM", "Tractogram processed with MITK Diffusion", "MIC@DKFZ");

    // Manufacturer, model name, serial number, software version(s)
    IODEnhGeneralEquipmentModule::EquipmentInfo equipment("MIC@DKFZ", "dcmtract library", "0815", OFFIS_DCMTK_VERSION_STRING);
    IODReferences refs;

    // We need at least one image reference this Tractography Results object is based on.
    // We provide: Patient ID, Study Instance UID, Series Instance UID, SOP Instance UID, SOP Class UID
    IODImageReference* ref = new IODImageReference("PAT_ID_4711", "1.2.3", "4.5.6", "7.8.9", UID_MRImageStorage);
    refs.add(ref);

    OFString contentDate = boost::gregorian::to_iso_string(boost::gregorian::day_clock::universal_day()).c_str();
    OFString contentTime = boost::posix_time::to_iso_string(boost::posix_time::second_clock::universal_time()).substr(9).c_str();

    TrcTractographyResults *trc = NULL;
    TrcTractographyResults::create(id, contentDate, contentTime, equipment, refs, trc);

    // Create track set
    CodeWithModifiers anatomy("");
    anatomy.set("T-A0095", "SRT", "White matter of brain and spinal cord");

    // Every CodeSequenceMacro has: Code Value, Coding Scheme Designator, Code Meaning
    CodeSequenceMacro diffusionModel("sup181_bb03", "DCM", "Single Tensor");
    CodeSequenceMacro algorithmId("sup181_ee01", "DCM", "Deterministic");
    TrcTrackSet *set = NULL;
    trc->addTrackSet("First and last Track Set", "Mini description", anatomy, diffusionModel, algorithmId, set);

    // Create trackset
    mitk::FiberBundle::ConstPointer fib = dynamic_cast<const mitk::FiberBundle*>(this->GetInput());
    vtkPolyData* poly = fib->GetFiberPolyData();

    Uint16 cieLabColor[3]; // color the whole track with this color; we use some blue
    cieLabColor[0] = 30000; // L
    cieLabColor[1] = 0 ; // a
    cieLabColor[2] = 0 ; // b

    for (int i=0; i<fib->GetNumFibers(); i++)
    {
        vtkCell* cell = poly->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        Float32 pointData[numPoints*3];
        for(int i=0; i<numPoints ;i++)
        {
            double* p = points->GetPoint(i);

            pointData[i*3] = p[0];
            pointData[i*3+1] = p[1];
            pointData[i*3+2] = p[2];
        }

        TrcTrack* track = NULL;
        set->addTrack(pointData, numPoints, cieLabColor, 1 /* numColors */, track);
    }

    // Frame of Reference is required; could be the same as from related MR series
    trc->getFrameOfReference().setFrameOfReferenceUID("10.11.12");

    // Set some optional data
    trc->getPatient().setPatientID("4711");
    trc->getPatient().setPatientName("Doe^John");
    trc->getSeries().setSeriesDescription("This is just a test series with a single Tractography Results object inside");

    // Save file
    trc->saveFile(this->GetOutputLocation().c_str());
    delete trc;

    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "DICOM Fiber bundle written to " << this->GetOutputLocation();
  }
  catch(...)
  {
    throw;
  }
}
