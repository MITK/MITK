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


#ifndef DICOMPERSPECTIVE_H_
#define DICOMPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

#include <QObject>

/**
 * \brief Perspective class used as DICOM perspective within the custom viewer plug-in.
 *
 * This is the perspective class that is used as DICOM import perspective within the custom
 * viewer plug-in. The DicomPerspective holds a single instance of DicomView. The editor area
 * is made invisible, and the DicomView instance is integrated as a non-closeable fixated
 * standalone view.
 */
class DicomPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  DicomPerspective();

  /**
   * An instance of DicomView is initially being laid out.
   */
  void CreateInitialLayout(berry::IPageLayout::Pointer layout);

};

#endif /* DICOMPERSPECTIVE_H_ */
