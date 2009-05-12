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

#if !defined(QMITK_VOLUMETRY_H__INCLUDED)
#define QMITK_VOLUMETRY_H__INCLUDED

#include "QmitkFunctionality.h"
#include <mitkOperationActor.h>
#include <mitkProperties.h>
class QmitkStdMultiWidget;
class QmitkVolumetryControls;

namespace mitk { 
  class DisplayInteractor; 
}
/*!
\brief Volume calculator/voxel counter using of a simple threshold.
\ingroup Functionalities
*/
class QmitkVolumetry : public QmitkFunctionality
{  
  Q_OBJECT
  public:  
  /*!  
  \brief default constructor  
  */  
  QmitkVolumetry(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkVolumetry();

  /*!  
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.  
  */  
  virtual QWidget * CreateControlWidget(QWidget *parent);

  /*!  
  \brief method for creating the applications main widget  
  */  
  virtual QWidget * CreateMainWidget(QWidget * parent);

  /*!  
  \brief method for creating the connections of main and control widget  
  */  
  virtual void CreateConnections();

  /*!  
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  */  
  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated();
  virtual void Deactivated();

public slots:
  void InitializeStandardViews(mitk::BaseData::Pointer data);
protected slots:  
  void TreeChanged();

protected:  
  /*!  
  * default main widget containing 4 windows showing 3   
  * orthogonal slices of the volume and a 3d render window  
  */  
  QmitkStdMultiWidget * m_MultiWidget;

  /*!  
  * controls containing sliders for scrolling through the slices  
  */  
  QmitkVolumetryControls * m_Controls;

};
#endif // !defined(QMITK_VOLUMETRY_H__INCLUDED)
