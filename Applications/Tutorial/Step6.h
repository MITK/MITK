/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef STEP6_H
#define STEP6_H

//#include <qmainwindow.h>
#include <QWidget>
#include <mitkStandaloneDataStorage.h>
#include <mitkImage.h>
#include <mitkPointSet.h>

#include <itkImage.h>

#ifndef DOXYGEN_IGNORE

class QLineEdit;

class Step6 : public QWidget
{
  Q_OBJECT
public:
  Step6( int argc, char* argv[], QWidget *parent=0 );
  ~Step6() {};

  virtual void Initialize();

  virtual int GetThresholdMin();
  virtual int GetThresholdMax();

protected:
  void Load(int argc, char* argv[]);
  virtual void SetupWidgets();

  template < typename TPixel, unsigned int VImageDimension >
    friend void RegionGrowing( itk::Image<TPixel, VImageDimension>* itkImage, Step6* step6);

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

