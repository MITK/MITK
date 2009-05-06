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

#if !defined(QMITK_ISOSURFACE_H__INCLUDED)
#define QMITK_ISOSURFACE_H__INCLUDED

#include <QmitkFunctionality.h>
#include <mitkColorSequenceRainbow.h>
#include <mitkDataTreeFilter.h>

class QmitkStdMultiWidget;
class QmitkIsoSurfaceControls;

/*!
  \brief IsoSurface

  One needs to reimplement the methods CreateControlWidget(..), CreateMainWidget(..)
  and CreateAction(..) from QmitkFunctionality.

  \todo review use of iterators and DataTreeCombobox
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

    protected slots:

    virtual void TreeChanged();

    /*
     * just an example slot for the example TreeNodeSelector widget
     */
    void ImageSelected(const mitk::DataTreeFilter::Item* item);

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
     *  variable to count Surfaces and give it to name in DataTree
     */
    int m_SurfaceCounter;

    mitk::ColorSequenceRainbow m_RainbowColor;
};

#endif // !defined(QMITK_ISOSURFACE_H__INCLUDED)

