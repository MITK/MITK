/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#if !defined(QMITK_ISOSURFACE_H__INCLUDED)
#define QMITK_ISOSURFACE_H__INCLUDED

#include <QmitkFunctionality.h>
#include <mitkColorSequence.h>

class QmitkStdMultiWidget;
class QmitkIsoSurfaceControls;

/*!
  \brief IsoSurface 

  One needs to reimplement the methods CreateControlWidget(..), CreateMainWidget(..) 
  and CreateAction(..) from QmitkFunctionality. 

  \sa QmitkFunctionality
  \ingroup Functionalities
  */
class QmitkIsoSurface : public QmitkFunctionality
{  
  Q_OBJECT

  public:  
    /*!  
      \brief default constructor  
      */  
    QmitkIsoSurface(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

    /*!  
      \brief default destructor  
      */  
    virtual ~QmitkIsoSurface();

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

    /*!  
      \brief method for creating a surface object  
      */ 
    void CreateSurface();

  protected:  
    /*!  
     * default main widget containing 4 windows showing 3   
     * orthogonal slices of the volume and a 3d render window  
     */  
    QmitkStdMultiWidget * m_MultiWidget;

    /*!  
     * controls containing sliders for scrolling through the slices  
     */  
    QmitkIsoSurfaceControls * m_Controls;

    /*!
     * image which is used to create the surface
     */
    mitk::Image* m_MitkImage;

    /*!
     * read thresholdvalue from GUI and convert it to float
     */
    float getThreshold();

    /*!
     * Node that contains the surfaceModel
     */
    //  mitk::DataTreeNode::Pointer m_SurfaceNode;

    /*!
     * iterator on current image
     */
    mitk::DataTreeIteratorClone m_MitkImageIterator;

    /*!
     *  variable to count Surfaces and give it to name in DataTree 
     */
    int m_SurfaceCounter;

    //SurfaceColor
    float m_r;
    float m_g;
    float m_b;

    mitk::Color m_Color;
    mitk::ColorSequence* m_RainbowColor;
};
#endif // !defined(QMITK_ISOSURFACE_H__INCLUDED)
