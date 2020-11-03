/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef STEP6_H
#define STEP6_H

//#include <qmainwindow.h>
#include <QWidget>
#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkStandaloneDataStorage.h>

#include <itkImage.h>

#ifndef DOXYGEN_IGNORE

class QLineEdit;

class Step6 : public QWidget
{
  Q_OBJECT
public:
  Step6(int argc, char *argv[], QWidget *parent = nullptr);
  ~Step6() override {}
  virtual void Initialize();

  virtual int GetThresholdMin();
  virtual int GetThresholdMax();

protected:
  void Load(int argc, char *argv[]);
  virtual void SetupWidgets();

  template <typename TPixel, unsigned int VImageDimension>
  friend void RegionGrowing(itk::Image<TPixel, VImageDimension> *itkImage, Step6 *step6);

  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_FirstImage;
  mitk::PointSet::Pointer m_Seeds;

  mitk::Image::Pointer m_ResultImage;
  mitk::DataNode::Pointer m_ResultNode;

  QLineEdit *m_LineEditThresholdMin;
  QLineEdit *m_LineEditThresholdMax;

protected slots:
  virtual void StartRegionGrowing();
};
#endif // DOXYGEN_IGNORE

#endif // STEP6_H

/**
\example Step6.h
*/
