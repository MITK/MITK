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

#include "mitkFiberBundleDicomReader.h"
#include <itkMetaDataObject.h>
#include <vtkPolyData.h>
#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <itksys/SystemTools.hxx>
#include <tinyxml.h>
#include <vtkCleanPolyData.h>
#include <mitkTrackvis.h>
#include <mitkCustomMimeType.h>
#include "mitkDiffusionIOMimeTypes.h"
#include <vtkTransformPolyDataFilter.h>
#include <mitkLexicalCast.h>

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmtract/trctractographyresults.h"
#include "dcmtk/dcmtract/trctrack.h"

mitk::FiberBundleDicomReader::FiberBundleDicomReader()
  : mitk::AbstractFileReader( mitk::DiffusionIOMimeTypes::FIBERBUNDLE_DICOM_MIMETYPE_NAME(), "DICOM Fiber Bundle Reader" )
{
  m_ServiceReg = this->RegisterService();
}

mitk::FiberBundleDicomReader::FiberBundleDicomReader(const FiberBundleDicomReader &other)
  :mitk::AbstractFileReader(other)
{
}

mitk::FiberBundleDicomReader * mitk::FiberBundleDicomReader::Clone() const
{
  return new FiberBundleDicomReader(*this);
}

std::vector<itk::SmartPointer<mitk::BaseData> > mitk::FiberBundleDicomReader::Read()
{

  std::vector<itk::SmartPointer<mitk::BaseData> > output_fibs;
  try
  {
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, nullptr );
    setlocale(LC_ALL, locale.c_str());

    std::string filename = this->GetInputLocation();

    OFCondition result;
    TrcTractographyResults *trc = nullptr;
    result = TrcTractographyResults::loadFile(filename.c_str(), trc);
    if (result.bad())
      mitkThrow() << "Unable to load tractography dicom file: " << result.text();

    OFString val = "-";
    trc->getPatient().getPatientName(val);
    MITK_INFO << "Patient Name: " <<  val; val = "-";
    trc->getStudy().getStudyInstanceUID(val);
    MITK_INFO << "Study       : " << val; val = "-";
    trc->getSeries().getSeriesInstanceUID(val);
    MITK_INFO << "Series      : " << val; val = "-";
    trc->getSOPCommon().getSOPInstanceUID(val);
    MITK_INFO << "Instance    : " << val; val = "-";
    MITK_INFO << "-------------------------------------------------------------------------";
    size_t numTrackSets = trc->getNumberOfTrackSets();
    OFVector<TrcTrackSet*>& sets = trc->getTrackSets();
    for (size_t ts = 0; ts < numTrackSets; ts++)
    {
      size_t numTracks = sets[ts]->getNumberOfTracks();
      MITK_INFO << "  Track Set #" << ts << ": " << numTracks << " Tracks, "
           << sets[ts]->getNumberOfTrackSetStatistics() << " Track Set Statistics, "
           << sets[ts]->getNumberOfTrackStatistics() << " Track Statistics, "
           << sets[ts]->getNumberOfMeasurements() << " Measurements ";

      vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
      vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

      for (size_t t = 0; t < numTracks; t++)
      {
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        TrcTrack* track = sets[ts]->getTracks()[t];
        const Float32* vals = nullptr;
        size_t numPoints = track->getTrackData(vals);

        for (size_t v = 0; v < numPoints; ++v)
        {
          vtkIdType id = vtkNewPoints->InsertNextPoint(vals[v*3],vals[v*3+1],vals[v*3+2]);
          container->GetPointIds()->InsertNextId(id);
        }

        vtkNewCells->InsertNextCell(container);
      }

      vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
      fiberPolyData->SetPoints(vtkNewPoints);
      fiberPolyData->SetLines(vtkNewCells);

//      // transform polydata from RAS (MRtrix) to LPS (MITK)
//      mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
//      vtkSmartPointer< vtkMatrix4x4 > matrix = vtkSmartPointer< vtkMatrix4x4 >::New();
//      matrix->Identity();
//      matrix->SetElement(0,0,-matrix->GetElement(0,0));
//      matrix->SetElement(1,1,-matrix->GetElement(1,1));
//      geometry->SetIndexToWorldTransformByVtkMatrix(matrix);

//      vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
//      transformFilter->SetInputData(fiberPolyData);
//      transformFilter->SetTransform(geometry->GetVtkTransform());
//      transformFilter->Update();

      FiberBundle::Pointer fib = FiberBundle::New(fiberPolyData);

      CodeSequenceMacro* algoCode = sets[ts]->getTrackingAlgorithmIdentification().at(0);
      val = "-"; algoCode->getCodeValue(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.algo_code.value",val.c_str());
      val = "-"; algoCode->getCodeMeaning(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.algo_code.meaning",val.c_str());

      CodeSequenceMacro modelCode = sets[ts]->getDiffusionModelCode();
      val = "-"; modelCode.getCodeValue(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.model_code.value",val.c_str());
      val = "-"; modelCode.getCodeMeaning(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.model_code.meaning",val.c_str());

      CodeWithModifiers anatomy = sets[ts]->getTrackSetAnatomy();
      val = "-"; anatomy.getCodeValue(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.anatomy.value",val.c_str());
      val = "-"; anatomy.getCodeMeaning(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.anatomy.meaning",val.c_str());

      val = "-"; trc->getPatient().getPatientID(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.patient_id",val.c_str());
      val = "-"; trc->getPatient().getPatientName(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.patient_name",val.c_str());
      val = "-"; trc->getStudy().getStudyInstanceUID(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.study_instance_uid",val.c_str());
      val = "-"; trc->getSeries().getSeriesInstanceUID(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.series_instance_uid",val.c_str());
      val = "-"; trc->getSOPCommon().getSOPInstanceUID(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.sop_instance_uid",val.c_str());
      val = "-"; trc->getSOPCommon().getSOPClassUID(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.sop_class_uid",val.c_str());
      val = "-"; trc->getFrameOfReference().getFrameOfReferenceUID(val);
      fib->GetPropertyList()->SetStringProperty("DICOM.frame_of_reference_uid",val.c_str());

      output_fibs.push_back(fib.GetPointer());
      MITK_INFO << "Fiber bundle read";
    }
    delete trc;

    setlocale(LC_ALL, currLocale.c_str());
    return output_fibs;
  }
  catch(...)
  {
    throw;
  }
  return output_fibs;
}
