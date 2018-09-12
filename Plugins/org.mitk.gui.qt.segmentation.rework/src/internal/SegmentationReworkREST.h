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

  struct DicomDTO {
    std::string seriesUID;
    std::string studyUID;
    std::string instanceUID;
    std::vector<double> simScoreArray;
    int minSliceStart;
  };

  SegmentationReworkREST();
  SegmentationReworkREST(utility::string_t url);
  ~SegmentationReworkREST();

  void HandlePut(MitkRequest message);
  void SetPutCallback(std::function<void(DicomDTO& message)> callback);

signals:
  void InvokeUpdateChartWidget();

private:
  std::function<void(DicomDTO& message)> m_PutCallback;
};

#endif // SegmentationReworkREST_h