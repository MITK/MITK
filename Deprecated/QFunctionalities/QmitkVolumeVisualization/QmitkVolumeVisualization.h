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

#if !defined(QMITK_VOLUMEVISUALIZATION_H__INCLUDED)
#define QMITK_VOLUMEVISUALIZATION_H__INCLUDED

#include <QmitkFunctionality.h>
#include <mitkDataTreeFilter.h>

class QmitkStdMultiWidget;
class QmitkVolumeVisualizationControls;


/*@brief A volume rendering functionality.
 * The functionality supplies a volume defined by a transfer function.  
 * The transfer function defines an RGBA value for each Voxel. 
 * The result is a 2D projection of a 3D sampled data set.
 *
 * \sa QmitkFunctionality
 * \ingroup Functionalities
 **/
class QmitkVolumeVisualization : public QmitkFunctionality
{  
  Q_OBJECT

  public:  
    /*!  
      \brief default constructor  
      */  
    QmitkVolumeVisualization(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

    /*!  
      \brief default destructor  
      */  
    virtual ~QmitkVolumeVisualization();

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
    void ImageSelected(const mitk::DataTreeFilter::Item* item);
    void EnableRendering(bool state);

    void GetChoice(int number);
    void GetCStyle(int number);
    void GetPreset(int number);
    void SetShading(bool state, int lod);
    void ImmediateUpdate(bool state);
    void EnableClippingPlane(bool state);
    void SetShadingValues(float ambient, float diffuse, float specular, float specpower);

    /* !!! ************************/
    virtual void OnChangeTransferFunctionMode( int mode );

  protected:  
    /*!  
     * default main widget containing 4 windows showing 3   
     * orthogonal slices of the volume and a 3d render window  
     */  
    QmitkStdMultiWidget * m_MultiWidget;

    /*!  
     * controls containing sliders for scrolling through the slices  
     */  
    QmitkVolumeVisualizationControls * m_Controls;

    bool image_ok;

};
#endif // !defined(QMITK_VOLUMEVISUALIZATION_H__INCLUDED)
