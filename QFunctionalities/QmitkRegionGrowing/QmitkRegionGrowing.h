/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2006-06-23 15:16:01 +0200 (Fri, 23 Jun 2006) $
Version:   $Revision: 7202 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QmitkRegionGrowing_H__INCLUDED)
#define QmitkRegionGrowing_H__INCLUDED

#include "QmitkFunctionality.h"
#include "mitkPointSet.h"

class QmitkStdMultiWidget;
class QmitkRegionGrowingControls;

/*!
\brief QmitkRegionGrowing 

One needs to reimplement the methods CreateControlWidget(..), CreateMainWidget(..) 
and CreateAction(..) from QmitkFunctionality. 

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkRegionGrowing : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  /*!  
  \brief default constructor  
  */  
  QmitkRegionGrowing(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkRegionGrowing();

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

protected slots:  
  void TreeChanged();
  /*
   * just an example slot for the example TreeNodeSelector widget
   */
  void ImageSelected(mitk::DataTreeIteratorClone imageIt);

protected:  
  /*!  
  * default main widget containing 4 windows showing 3   
  * orthogonal slices of the volume and a 3d render window  
  */  
  QmitkStdMultiWidget * m_MultiWidget;

  /*!  
  * controls containing sliders for scrolling through the slices  
  */  
  QmitkRegionGrowingControls * m_Controls;

  // we'll access these members from different member functions
  mitk::DataTreeNode::Pointer m_PointSetNode;
  mitk::PointSet::Pointer m_PointSet;

};
#endif // !defined(QmitkRegionGrowing_H__INCLUDED)

