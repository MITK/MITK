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


#ifndef RandomParcellationView_h
#define RandomParcellationView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkRandomParcellationViewControls.h"

#include <vector>

#include <itkImage.h>

//To use pair
#include <utility>


/**
\brief QmitkRandomParcellationView for GUI Interactions
\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkRandomParcellationView : public QmitkFunctionality
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;
  virtual void CreateQtPartControl(QWidget *parent);

  /** \brief Interaction with GUI and mitkRandomParcellationGenerator to get random parcels.*/
  template <typename TPixel, unsigned int VImageDimension>
  void GetRandomParcels( itk::Image<TPixel, VImageDimension>* );

  /** \brief Method to set the default data storage.*/
  virtual void SetDataStorage(mitk::DataStorage* dataStorage)
  {
    m_DataStorage = dataStorage;
  }

  protected slots:

    /** \brief Called when the user clicks the GUI button*/
    void SelectRandomNodes();

    /** \brief Called when the user checks the GUI checkbox*/
    void OnMergingCheckboxChanged(int state);

    /** \brief Called when the user ckecks the GUI radio button*/
    void OnRadioButtonNumberParcelsChanged(int state);

    /** \brief Called when the user ckecks the GUI radio button*/
    void OnRadioButtonSmallestParcelChanged(int state);

protected:

  QmitkStdMultiWidget* m_MultiWidget;

  mitk::DataStorage* m_DataStorage;

  //to save an itk image
  mitk::DataNode::Pointer m_InputImageNode;

  /** \brief Shows the number of voxels with value one on the GUI*/
  template <typename TPixel, unsigned int VImageDimension>
  void ShowNumberOfVoxels( itk::Image<TPixel, VImageDimension> * inputImage);

  /** \brief Shows the number of nodes on the GUI (may have changed after merging)*/
  void ShowNumberOfNodes(int numberNodes);

  virtual void SetFocus();

  /** \brief called by QmitkFunctionality when DataManager's selection has changed*/
  virtual void OnSelectionChanged(std::vector<mitk::DataNode*> nodes);

  Ui::QmitkRandomParcellationViewControls m_Controls;

  static const std::string PROPERTYNAME;
  static const std::string PROPERTYOPACITY;
  static const std::string m_NAMEFORRANDOMVOXELIMAGE;

};

#endif // RandomParcellationView_h
