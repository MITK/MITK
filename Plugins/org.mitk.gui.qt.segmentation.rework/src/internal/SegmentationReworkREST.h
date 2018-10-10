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

#ifndef SegmentationReworkREST_h
#define SegmentationReworkREST_h

#include <mitkRestServer.h>

class SegmentationReworkREST : public mitk::RESTServer
{
  Q_OBJECT

public:
  struct DicomDTO
  {
    std::string segSeriesUIDA;
    std::string segSeriesUIDB;
    std::string imageSeriesUID;
    std::string studyUID;
    std::string segInstanceUIDA;
    std::string segInstanceUIDB;
    std::string srSeriesUID;
    std::vector<double> simScoreArray;
    int minSliceStart;
    std::string groundTruth;
  };

  SegmentationReworkREST();
  SegmentationReworkREST(utility::string_t url);
  ~SegmentationReworkREST();

  void HandlePut(MitkRequest message);
  void HandleGet(MitkRequest message);

  void SetPutCallback(std::function<void(DicomDTO &message)> callback)
  { 
	  m_PutCallback = callback;
  }

  void SetGetImageSegCallback(std::function<void(DicomDTO &message)> callback) 
  {
	  m_GetImageSegCallback = callback;
  }

  void SetGetEvalCallback(std::function<void(DicomDTO &message)> callback)
  {
    m_GetEvalCallback = callback;
  }

signals:
  void InvokeUpdateChartWidget();

private:
  std::function<void(DicomDTO &message)> m_PutCallback;
  std::function<void(DicomDTO &message)> m_GetImageSegCallback;
  std::function<void(DicomDTO &message)> m_GetEvalCallback;
};

#endif // SegmentationReworkREST_h
