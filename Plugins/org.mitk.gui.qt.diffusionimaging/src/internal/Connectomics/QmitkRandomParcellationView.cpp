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

// Qmitk
#include "QmitkRandomParcellationView.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qt
#include <QMessageBox>

// Includes for image casting between ITK and MITK
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkITKImageImport.h"

//To convert integer to string
#include <stdlib.h>
#include <sstream>

//Itk Iterators
#include <itkNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>

// mitk
#include <mitkRegionVoxelCounter.h>
#include <mitkRandomParcellationGenerator.h>


//Child
const std::string QmitkRandomParcellationView::PROPERTYNAME              = "name";
const std::string QmitkRandomParcellationView::PROPERTYOPACITY           = "opacity";
const std::string QmitkRandomParcellationView::m_NAMEFORRANDOMVOXELIMAGE = "Random Voxel Image";

const std::string QmitkRandomParcellationView::VIEW_ID = "org.mitk.views.randomparcellationview";


void QmitkRandomParcellationView::SetFocus()
{
  m_Controls.buttonSelectRandomNodes->setFocus();
}

void QmitkRandomParcellationView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.buttonSelectRandomNodes, SIGNAL(clicked()), this, SLOT(SelectRandomNodes()) );
  connect( m_Controls.checkBoxMerging, SIGNAL(stateChanged(int)),this, SLOT(OnMergingCheckboxChanged(int)));
  connect( m_Controls.radioButtonNumberParcels, SIGNAL(clicked()), this, SLOT(OnRadioButtonNumberParcelsChanged(int)));
  connect( m_Controls.radioButtonSmallestParcel, SIGNAL(clicked()), this, SLOT(OnRadioButtonSmallestParcelChanged(int)));
}

void QmitkRandomParcellationView::OnMergingCheckboxChanged(int state)
{
  if (m_Controls.checkBoxMerging->isChecked())
  {
    //Make the frame visible
    m_Controls.frameMerging->setEnabled(true);
    m_Controls.frameNumberParcels->setEnabled(true);
    m_Controls.frameSmallestParcel->setEnabled(true);
  }
  else
  {
    m_Controls.frameMerging->setEnabled(false);
  }
}

void QmitkRandomParcellationView::OnRadioButtonNumberParcelsChanged(int state)
{
  if (m_Controls.radioButtonNumberParcels->isChecked())
  {
    m_Controls.frameNumberParcels->setEnabled(true);
    m_Controls.radioButtonSmallestParcel->setChecked(false);
  }
  else
  {
    m_Controls.frameNumberParcels->setEnabled(false);
  }
}

void QmitkRandomParcellationView::OnRadioButtonSmallestParcelChanged(int state)
{
  if (m_Controls.radioButtonSmallestParcel->isChecked())
  {
    m_Controls.frameSmallestParcel->setEnabled(true);
    m_Controls.radioButtonNumberParcels->setChecked(false);
  }
  else
  {
    m_Controls.frameSmallestParcel->setEnabled(false);
  }
}

void QmitkRandomParcellationView::OnSelectionChanged(std::vector<mitk::DataNode*> nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls.labelWarning->setVisible( false );
      m_Controls.buttonSelectRandomNodes->setEnabled( true );
      mitk::Image* imageDisplay = dynamic_cast<mitk::Image*>(node->GetData());
      AccessFixedDimensionByItk(imageDisplay, ShowNumberOfVoxels, 3);
      return;
    }
  }

  m_Controls.labelWarning->setVisible( true );
  m_Controls.buttonSelectRandomNodes->setEnabled( false );
}

template <typename TPixel, unsigned int VImageDimension>
void QmitkRandomParcellationView::ShowNumberOfVoxels( itk::Image<TPixel, VImageDimension> * inputImage)
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;

  mitk::RegionVoxelCounter<TPixel, VImageDimension> counter;
  counter.SetRegion(inputImage->GetLargestPossibleRegion());
  counter.SetImage(inputImage);
  int numberVoxelsDisplay = counter.VoxelWithValue(1);
  std::string stringDisplay = "Number of voxels with value 1: ";
  std::ostringstream convertDisplay;
  convertDisplay << numberVoxelsDisplay;
  std::string stringNumberVoxels = convertDisplay.str();
  stringDisplay += stringNumberVoxels;
  QString textNumberVoxelsDisplay = QString::fromStdString(stringDisplay);
  m_Controls.labelNumberVoxels->setText(textNumberVoxelsDisplay);
}


void QmitkRandomParcellationView::SelectRandomNodes()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode* node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "No image available", "Please load and select an image before starting image processing.");
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>( data );
    if (image)
    {
      //get numberNodes (number of nodes) with SpinBox
      int numberNodes( 0 );
      numberNodes = m_Controls.spinBoxNumberNodes->value();

      std::stringstream message;
      std::string name;
      message << "Selecting " << numberNodes << " random nodes for ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
      }
      message << ".";
      MITK_INFO << message.str();

      //MITK into ITK image, function GetRandomParcels gets executed, Dimension is 3
      AccessFixedDimensionByItk(image, GetRandomParcels, 3);

    }
  }
}



template <typename TPixel, unsigned int VImageDimension>
void QmitkRandomParcellationView::GetRandomParcels( itk::Image<TPixel, VImageDimension> * inputImage)
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::Image< int, VImageDimension > IntegerImageType;

  //new Image which will contain the parcellation
  typename IntegerImageType::Pointer voxelImage = IntegerImageType::New();

  voxelImage->SetRegions(inputImage->GetLargestPossibleRegion().GetSize());
  voxelImage->SetSpacing(inputImage->GetSpacing());
  voxelImage->SetOrigin(inputImage->GetOrigin());
  voxelImage->Allocate();

  //Voxel iterators
  itk::ImageRegionIterator<ImageType> it_inputImage(inputImage, inputImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<IntegerImageType> it_voxelImage(voxelImage, voxelImage->GetLargestPossibleRegion());

  //Copy the inputImage -> voxelImage
  for (it_inputImage.GoToBegin(), it_voxelImage.GoToBegin(); !it_inputImage.IsAtEnd(); ++it_inputImage, ++it_voxelImage)
  {
    it_voxelImage.Value() = it_inputImage.Value();
  }

  //Neighborhood Iterator
  typedef itk::NeighborhoodIterator< IntegerImageType > NeighborhoodIteratorType;
  typename NeighborhoodIteratorType::RadiusType radius;
  radius.Fill(1);

  //Counter for voxels with value 1
  mitk::RegionVoxelCounter<int, 3> counter;
  counter.SetImage(voxelImage);
  counter.SetRegion(voxelImage->GetLargestPossibleRegion());

  int numberVoxels = counter.VoxelWithValue(1);

  //Get numberNodes with SpinBox
  int numberNodes(0);
  numberNodes = m_Controls.spinBoxNumberNodes->value();

  //WARNINGS
  if (numberNodes > numberVoxels )
  {
    QMessageBox::information( NULL, "Smaller Number of Nodes", "The number of nodes is greater than the number of voxels. Please choose a smaller number!");
    return;
  }

  if (m_Controls.checkBoxMerging->isChecked() && m_Controls.radioButtonNumberParcels->isChecked())
  {
    int numberParcels = m_Controls.spinBoxNumberParcels->value();
    if (numberNodes<numberParcels )
    {
      QMessageBox::information( NULL, "Smaller Number of Parcels", "The number of parcels is greater than the number of nodes. Please choose a smaller number of parcels!");
      return;
    }
  }

  if (m_Controls.checkBoxMerging->isChecked() && m_Controls.radioButtonSmallestParcel->isChecked())
  {
    int sizeSmallestParcel = m_Controls.spinBoxSmallestParcel->value();
    if (sizeSmallestParcel > numberVoxels )
    {
      QMessageBox::information( NULL, "Smaller Size", "The size of the smallest parcel is greater than the number of voxels. Please choose a smaller size!");
      return;
    }
  }

  //Get a RandomParcellationGenerator to generate the parcels
  mitk::RandomParcellationGenerator<int, 3> generator;
  generator.SetImage(voxelImage);
  generator.SetNumberNodes(numberNodes);

  //Chooses random initial voxels, with which we can start the parcellation growth
  generator.GetRandomSeedVoxels();
  //Now we add all the voxels (which share a face with one of the voxels of a region) to a region, particular constraints have to be fulfilled
  generator.ParcelGrowthOverFaces();
  //Because of the constraints or the position of some voxels we now add voxels to the regions which just share an edge or vertex with one region
  generator.FillOverEdgeOrVertex();
  //Isolated voxels also have to be allocated to the regions
  generator.AllocateIsolatedVoxels();

  //Show the sizeOfRegions in the window
  MITK_DEBUG << "Without merging we get the following size of regions: " << endl;
  generator.ShowSizeOfRegions();

  //Check if the Merge-Buttons are chosen
  int newNumberNodes;

  if (m_Controls.checkBoxMerging->isChecked())
  {
    int givenSizeOfSmallestRegion(0);
    int desiredNumberOfParcels(0);
    int givenSizeOfSmallestRegionBeginning = numberVoxels;
    bool mergingWithNumberParcels(false);
    bool mergingWithSmallestParcel(false);
    bool justMergeSmallParcels(false);

    if (m_Controls.radioButtonSmallestParcel->isChecked() )
    {
      mergingWithSmallestParcel = true;
      givenSizeOfSmallestRegionBeginning = m_Controls.spinBoxSmallestParcel->value();
      if( m_Controls.checkBoxJustMergeSmallParcels-> isChecked() )
      {
        //Spinbox: How many voxels should the smallest parcel have?
        givenSizeOfSmallestRegion = m_Controls.spinBoxSmallestParcel->value();
        justMergeSmallParcels = true;
      }
      else
      {
        givenSizeOfSmallestRegion = numberVoxels;
      }
    }
    else if (m_Controls.radioButtonNumberParcels->isChecked())
    {
      givenSizeOfSmallestRegion = numberVoxels;
      desiredNumberOfParcels = m_Controls.spinBoxNumberParcels->value();
      mergingWithNumberParcels = true;
    }

    //Merge parcels according to a costfunction
    generator.SetVariablesForMerging(givenSizeOfSmallestRegion, desiredNumberOfParcels, givenSizeOfSmallestRegionBeginning);
    generator.SetBoolsForMerging(mergingWithNumberParcels, mergingWithSmallestParcel, justMergeSmallParcels);
    newNumberNodes = generator.MergeParcels();

    MITK_DEBUG << "After merging we get the following size of regions:" << endl;
    generator.ShowSizeOfRegions();
  }

  //Show NumberOfNodes on the GUI
  if (m_Controls.checkBoxMerging->isChecked()) //merging was used
  {
    ShowNumberOfNodes(newNumberNodes);
  }
  else
  {
    ShowNumberOfNodes(numberNodes);
  }

  //Change the values of the nodes, such that no gaps exist and the values starts with 1
  generator.SetAppropriateValues();

  //Get a new data node which contains the random parcellation
  mitk::Image::Pointer mitkRandomVoxelImage = mitk::GrabItkImageMemory( voxelImage );
  mitk::DataNode::Pointer newRandomVoxelImageNode = mitk::DataNode::New();
  newRandomVoxelImageNode->SetData(mitkRandomVoxelImage);
  newRandomVoxelImageNode->SetProperty(PROPERTYNAME.c_str(),
    mitk::StringProperty::New(m_NAMEFORRANDOMVOXELIMAGE));
  newRandomVoxelImageNode->SetProperty(PROPERTYOPACITY.c_str(),
    mitk::FloatProperty::New(1.0));
  this->GetDefaultDataStorage()->Add( newRandomVoxelImageNode );

  return;
}

void QmitkRandomParcellationView::ShowNumberOfNodes(int numberNodes)
{
  std::string string = "Number of nodes: ";
  std::string stringNumberNodes;
  std::ostringstream convert;
  convert << numberNodes;
  stringNumberNodes = convert.str();
  string += stringNumberNodes;
  QString textNumberNodes = QString::fromStdString(string);
  m_Controls.labelNumberNodes->setText(textNumberNodes);
}
