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

#include "mitkTumorInvasionClassification.h"

#include "mitkCollectionDilatation.h"
#include "mitkCollectionGrayOpening.h"
#include "mitkDataCollectionImageIterator.h"
#include "mitkDataCollectionUtilities.h"
#include "mitkIOUtil.h"
#include "mitkCLUtil.h"
#include <fstream>
// To initialize random number generator
#include <time.h>

static void EnsureDataImageInCollection(mitk::DataCollection::Pointer collection, std::string origin, std::string target)
{
  typedef itk::Image<double, 3> FeatureImage;

  if (collection->HasElement(origin))
  {
    mitk::Image::Pointer originImage = dynamic_cast<mitk::Image*>(collection->GetMitkImage(origin).GetPointer());
    FeatureImage::Pointer itkOriginImage = FeatureImage::New();
    mitk::CastToItkImage(originImage,itkOriginImage);

    if (!collection->HasElement(target) && itkOriginImage.IsNotNull())
    {
      FeatureImage::Pointer image = FeatureImage::New();
      image->SetRegions(itkOriginImage->GetLargestPossibleRegion());
      image->SetSpacing(itkOriginImage->GetSpacing());
      image->SetOrigin(itkOriginImage->GetOrigin());
      image->SetDirection(itkOriginImage->GetDirection());
      image->Allocate();

      collection->AddData(dynamic_cast<itk::DataObject*>(image.GetPointer()),target,"");
    }
  }
  for (std::size_t i = 0; i < collection->Size();++i)
  {
    mitk::DataCollection* newCol = dynamic_cast<mitk::DataCollection*>(collection->GetData(i).GetPointer());
    if (newCol != 0)
    {
      EnsureDataImageInCollection(newCol, origin, target);
    }
  }
}


mitk::TumorInvasionClassification::TumorInvasionClassification()
  : m_TargetID("TARGET"), m_TumorID("GTV"), m_MaskID("BRAINMASK"), m_ResultID("RESULT"), m_Randomize(false), m_WeightSamples(false)
{
}

void mitk::TumorInvasionClassification::SelectTrainingSamples(mitk::DataCollection *collection, unsigned int mode)
{
  srand (time(NULL));
  MITK_INFO << "LearnProgressionFeatures: Selecting training voxels.";
  switch (mode) {
  case 0:
  {
    MITK_INFO << " Selection Mode " << mode << " use all tumor voxels, healthy: 50% vicinity / 50% far away";

    CollectionDilation::DilateBinaryByName(collection,m_TargetID,2,0,"EXCLUDE");
    CollectionDilation::ErodeBinaryByName(collection,m_TargetID,1,0,"ERODE");
    CollectionDilation::DilateBinaryByName(collection,m_TargetID,m_TargetDil2D,m_TargetDil3D,"TRAIN");
    DataCollectionImageIterator<unsigned char,3> gtvIter(collection, m_TumorID);
    DataCollectionImageIterator<unsigned char,3> brainMaskIter(collection, m_MaskID);
    DataCollectionImageIterator<unsigned char,3> targetIter(collection, m_TargetID);
    DataCollectionImageIterator<unsigned char,3> targetDil(collection, m_TargetID+"TRAIN");

    // Count Healthy/ Tumor voxels
    // i.o. to decide how many of each are taken
    unsigned int totalTumor = 0;
    unsigned int totalHealthyClose = 0;
    unsigned int totalHealthy= 0;

    while (!brainMaskIter.IsAtEnd())
    {
      if (brainMaskIter.GetVoxel() != 0)
      {
        if (targetIter.GetVoxel() == 1 && gtvIter.GetVoxel() == 0 )
          ++totalTumor;

        if (targetIter.GetVoxel() == 0 && targetDil.GetVoxel()==1 && gtvIter.GetVoxel() == 0)
          ++totalHealthyClose;

        if (targetIter.GetVoxel() == 0  && gtvIter.GetVoxel() == 0 && targetDil.GetVoxel()==0)
          ++totalHealthy; // healthy but not close

      }
      ++brainMaskIter;
      ++targetIter;
      ++targetDil;
      ++gtvIter;
    }
    brainMaskIter.ToBegin();
    targetIter.ToBegin();
    targetDil.ToBegin();
    gtvIter.ToBegin();
    // Total of healthy samples that is to be collected
    unsigned int targetHealthy = totalTumor * m_ClassRatio;
    // Determines which portion of the healthy samples is drawn from the immediate vicinity of the newly grown tumor
    ScalarType ratioClose = .5;

    // Compute probabilities thresholds for choosing a close healthy voxel / any healthy voxel
    ScalarType thHealthyClose =  std::min(1.0 ,(targetHealthy*ratioClose)/totalHealthyClose);
    ScalarType thHealthyAny =  std::min(1.0 ,(targetHealthy*(1.0-ratioClose))/totalHealthy);
    // Some stats
    {
      MITK_INFO << "Total # Tumor Voxels " << totalTumor;
      MITK_INFO << "Total # Healthy Voxels" << totalHealthy;
      MITK_INFO << "Target Ratio " << m_ClassRatio;
      MITK_INFO << "Ratio for healthy close: " << thHealthyClose;
      MITK_INFO << "Ratio for healthy any other: " << thHealthyAny;
    }

    // DEBUG count occurances to compare
    unsigned int tumor = 0;
    unsigned int healthy = 0;
    while (!brainMaskIter.IsAtEnd())
    {
      if (brainMaskIter.GetVoxel() != 0)
      {
        if (targetIter.GetVoxel() == 1)
        { // choose tumor voxels for training
          if (gtvIter.GetVoxel() == 0) // tumor always
          {
            targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
            ++tumor;
          }
          else
            targetIter.SetVoxel(0);
        }
        else
        { // choose healty tissue voxels for training
          ScalarType rndVal =  (float)rand()/(float)(RAND_MAX) ; //(0..1)
          if (gtvIter.GetVoxel() == 0 &&  ((targetDil.GetVoxel()==1 && rndVal <= thHealthyClose)))
          {
            targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
            ++healthy;
          }
          else if (((targetDil.GetVoxel()==0 &&  rndVal <= thHealthyAny)))
          {
            targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
            ++healthy;
          }
          else
            targetIter.SetVoxel(0);
        }
      }
      else
        targetIter.SetVoxel(0);

      ++brainMaskIter;
      ++targetIter;
      ++gtvIter;
      ++targetDil;
    }
    MITK_INFO << "Training with Samples #Tumor " << tumor << " / healthy # "<< healthy;
  }
    break;
  case 2:
  {
    MITK_INFO << " Selection Mode " << mode << " Weighted with ratio";

    EnsureDataImageInCollection(collection, m_TumorID, "WEIGHTS");

    CollectionDilation::DilateBinaryByName(collection,m_TargetID,1,0,"EXCLUDE");
    CollectionDilation::ErodeBinaryByName(collection,m_TargetID,1,0,"ERODE");
    CollectionDilation::DilateBinaryByName(collection,m_TargetID+"EXCLUDE",m_TargetDil2D,m_TargetDil3D,"TRAIN");
    DataCollectionImageIterator<unsigned char,3> gtvIter(collection, m_TumorID);
    DataCollectionImageIterator<unsigned char,3> brainMaskIter(collection, m_MaskID);
    DataCollectionImageIterator<unsigned char,3> targetIter(collection, m_TargetID);
    DataCollectionImageIterator<unsigned char,3> targetDil(collection, m_TargetID+"EXCLUDETRAIN");
    DataCollectionImageIterator<unsigned char,3> excludeTumorIter(collection, m_TargetID+"ERODE");
    DataCollectionImageIterator<unsigned char,3> excludeHealthyIter(collection, m_TargetID+"EXCLUDE");

    DataCollectionImageIterator<double,3> weightsIter(collection, "WEIGHTS");

    // Count Healthy/ Tumor voxels
    // i.o. to decide how many of each are taken
    double totalTumor = 0;
    double totalHealthyClose = 0;
    double totalHealthyNonClose= 0;
    double totalHealthy= 0;

    while (!brainMaskIter.IsAtEnd())
    {
      if (brainMaskIter.GetVoxel() != 0)
      {
        if (targetIter.GetVoxel() == 1 && gtvIter.GetVoxel() == 0  && excludeTumorIter.GetVoxel() == 1)
          ++totalTumor;

        if (excludeHealthyIter.GetVoxel() == 0 && targetDil.GetVoxel()==1 && gtvIter.GetVoxel() == 0)
          ++totalHealthyClose;

        if (excludeHealthyIter.GetVoxel() == 0  && gtvIter.GetVoxel() == 0 && targetDil.GetVoxel()==0)
          ++totalHealthyNonClose; // healthy but not close

        if (excludeHealthyIter.GetVoxel() == 0  && gtvIter.GetVoxel() == 0 && targetIter.GetVoxel()==0)
          ++totalHealthy; // healthy
      }
      ++brainMaskIter;
      ++targetIter;
      ++targetDil;
      ++gtvIter;
      ++excludeHealthyIter;
      ++excludeTumorIter;
    }
    brainMaskIter.ToBegin();
    targetIter.ToBegin();
    targetDil.ToBegin();
    gtvIter.ToBegin();
    excludeHealthyIter.ToBegin();
    excludeTumorIter.ToBegin();


    // Compute probabilities thresholds for choosing a close healthy voxel / any healthy voxel
    ScalarType wHealthyClose =  10000.0 / totalHealthyClose;
    ScalarType wHealthyAny = 5000.0 / totalHealthyNonClose;
    ScalarType wTumor = (m_ClassRatio * (10000.0+5000.0)) / totalTumor ;

    // DEBUG count occurances to compare
    double potentialClose = 0;
    double selectedClose = 0;
    double tumor = 0;
    double healthy = 0;
    while (!brainMaskIter.IsAtEnd())
    {
      weightsIter.SetVoxel(0);
      if (brainMaskIter.GetVoxel() != 0)
      {
        if (targetIter.GetVoxel() == 1)
        { // choose tumor voxels for training
          if (gtvIter.GetVoxel() == 0 && excludeTumorIter.GetVoxel() == 1) // tumor always
          {
            targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
            weightsIter.SetVoxel(wTumor);
            ++tumor;
          }
          else if (gtvIter.GetVoxel() == 0 )
          {
            weightsIter.SetVoxel(0); //.1);
            targetIter.SetVoxel(0); //2);
          }
        }
        else
        { // choose healty tissue voxels for training
          if (gtvIter.GetVoxel() == 0 &&  ((excludeHealthyIter.GetVoxel() == 0 && targetDil.GetVoxel()==1 )))
          {
              targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
              weightsIter.SetVoxel(wHealthyClose);
              healthy+=wHealthyClose;
          }
          else if (((targetDil.GetVoxel()==0 && excludeHealthyIter.GetVoxel() == 0 )))
          {
              targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
              weightsIter.SetVoxel(wHealthyAny);
              healthy+=wHealthyAny;
          }
          else if ((gtvIter.GetVoxel() == 0 &&  excludeHealthyIter.GetVoxel() == 1) )
          {
            targetIter.SetVoxel( 0);//brainMaskIter.GetVoxel()+targetIter.GetVoxel());
            weightsIter.SetVoxel(0);//.1);
          }
          else
          {
            targetIter.SetVoxel(0);
            weightsIter.SetVoxel(0);
          }
        }
      }
      else
      {
        targetIter.SetVoxel(0);
        weightsIter.SetVoxel(0);
      }

      if (gtvIter.GetVoxel() != 0)
        targetIter.SetVoxel(0);

      ++brainMaskIter;
      ++targetIter;
      ++gtvIter;
      ++targetDil;
      ++excludeHealthyIter;
      ++excludeTumorIter;
      ++weightsIter;
    }
    MITK_INFO << "Training with Samples #Tumor " << tumor << " / healthy # "<< healthy;
    MITK_INFO << "Potential Close" << potentialClose;
    MITK_INFO << "Selected Close" << selectedClose;
  }
    break;
  default:
  {
    MITK_INFO << " Selection Mode " << mode << " Exclude voxels in border regions, healthy: 50% vicinity / 50% far away";

    //weights
    ScalarType tumorWeight = 1;
    //ScalarType unsureRegion = .25;
    ScalarType healthyTissue = 1;

    EnsureDataImageInCollection(collection, m_TumorID, "WEIGHTS");

    CollectionDilation::DilateBinaryByName(collection,m_TargetID,1,0,"EXCLUDE");
    CollectionDilation::ErodeBinaryByName(collection,m_TargetID,1,0,"ERODE");
    CollectionDilation::DilateBinaryByName(collection,m_TargetID+"EXCLUDE",m_TargetDil2D,m_TargetDil3D,"TRAIN");
    DataCollectionImageIterator<unsigned char,3> gtvIter(collection, m_TumorID);
    DataCollectionImageIterator<unsigned char,3> brainMaskIter(collection, m_MaskID);
    DataCollectionImageIterator<unsigned char,3> targetIter(collection, m_TargetID);
    DataCollectionImageIterator<unsigned char,3> targetDil(collection, m_TargetID+"EXCLUDETRAIN");
    DataCollectionImageIterator<unsigned char,3> excludeTumorIter(collection, m_TargetID+"ERODE");
    DataCollectionImageIterator<unsigned char,3> excludeHealthyIter(collection, m_TargetID+"EXCLUDE");

    DataCollectionImageIterator<double,3> weightsIter(collection, "WEIGHTS");

    // Count Healthy/ Tumor voxels
    // i.o. to decide how many of each are taken
    double totalTumor = 0;
    double totalHealthyClose = 0;
    double totalHealthyNonClose= 0;
    double totalHealthy= 0;

    while (!brainMaskIter.IsAtEnd())
    {
      if (brainMaskIter.GetVoxel() != 0)
      {
        if (targetIter.GetVoxel() == 1 && gtvIter.GetVoxel() == 0  && excludeTumorIter.GetVoxel() == 1)
          ++totalTumor;

        if (excludeHealthyIter.GetVoxel() == 0 && targetDil.GetVoxel()==1 && gtvIter.GetVoxel() == 0)
          ++totalHealthyClose;

        if (excludeHealthyIter.GetVoxel() == 0  && gtvIter.GetVoxel() == 0 && targetDil.GetVoxel()==0)
          ++totalHealthyNonClose; // healthy but not close

        if (excludeHealthyIter.GetVoxel() == 0  && gtvIter.GetVoxel() == 0 && targetIter.GetVoxel()==0)
          ++totalHealthy; // healthy
      }
      ++brainMaskIter;
      ++targetIter;
      ++targetDil;
      ++gtvIter;
      ++excludeHealthyIter;
      ++excludeTumorIter;
    }
    brainMaskIter.ToBegin();
    targetIter.ToBegin();
    targetDil.ToBegin();
    gtvIter.ToBegin();
    excludeHealthyIter.ToBegin();
    excludeTumorIter.ToBegin();

    // Total of healthy samples that is to be collected
    unsigned int targetHealthy = (tumorWeight/healthyTissue) * totalTumor * m_ClassRatio;
    // Determines which portion of the healthy samples is drawn from the immediate vicinity of the newly grown tumor
    ScalarType ratioClose = .5;

    // Compute probabilities thresholds for choosing a close healthy voxel / any healthy voxel
    ScalarType thHealthyClose =  std::min(1.0 ,((double)targetHealthy*ratioClose)/totalHealthyClose);
    ScalarType thHealthyAny =  std::min(1.0 ,((double)targetHealthy*(1.0-ratioClose))/totalHealthyNonClose);
    // Some stats
    {
      MITK_INFO << "Total Tumor " << totalTumor;
      MITK_INFO << "Total healthy " << totalHealthyNonClose;
      MITK_INFO << "Total healthy close " << totalHealthyClose;
      MITK_INFO << "Total healthy non-close " << totalHealthyNonClose;
      MITK_INFO << "Target Ratio " << m_ClassRatio;

      MITK_INFO << "Target Healthy " << targetHealthy;
      MITK_INFO << "Probabilty close " << thHealthyClose;
      MITK_INFO << "Probabilty any other " << thHealthyAny;
    }

    // DEBUG count occurances to compare
    double potentialClose = 0;
    double selectedClose = 0;
    double tumor = 0;
    double healthy = 0;
    while (!brainMaskIter.IsAtEnd())
    {
      weightsIter.SetVoxel(0);
      if (brainMaskIter.GetVoxel() != 0)
      {
        if (targetIter.GetVoxel() == 1)
        { // choose tumor voxels for training
          if (gtvIter.GetVoxel() == 0 && excludeTumorIter.GetVoxel() == 1) // tumor always
          {
            targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
            weightsIter.SetVoxel(tumorWeight);
            ++tumor;
          }
          else if (gtvIter.GetVoxel() == 0 )
          {
            weightsIter.SetVoxel(0); //.1);
            targetIter.SetVoxel(0); //2);
          }
        }
        else
        { // choose healty tissue voxels for training
          if (gtvIter.GetVoxel() == 0 &&  ((excludeHealthyIter.GetVoxel() == 0 && targetDil.GetVoxel()==1 )))
          {
            if (!m_WeightSamples && ((float)rand()/(float)(RAND_MAX)  < thHealthyClose))
            {
              targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
              weightsIter.SetVoxel(1);
              ++healthy;
            }
            else
            {
              targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
              weightsIter.SetVoxel(thHealthyClose);
              healthy+=thHealthyClose;
            }
          }
          else if (((targetDil.GetVoxel()==0 && excludeHealthyIter.GetVoxel() == 0 )))
          {
            if (!m_WeightSamples && ( (float)rand()/(float)(RAND_MAX)  < thHealthyAny))
            {
              targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
              weightsIter.SetVoxel(1);
              ++healthy;
            }
            else
            {
              targetIter.SetVoxel(brainMaskIter.GetVoxel()+targetIter.GetVoxel());
              weightsIter.SetVoxel(thHealthyAny);
              healthy+=thHealthyAny;
            }
          }
          else if ((gtvIter.GetVoxel() == 0 &&  excludeHealthyIter.GetVoxel() == 1) )
          {
            targetIter.SetVoxel( 0);//brainMaskIter.GetVoxel()+targetIter.GetVoxel());
            weightsIter.SetVoxel(0);//.1);
          }
          else
          {
            targetIter.SetVoxel(0);
            weightsIter.SetVoxel(0);
          }

          if (gtvIter.GetVoxel() == 0 &&  ((excludeHealthyIter.GetVoxel() == 0 && targetDil.GetVoxel()==1 )))
          {
            potentialClose++;

            if ((float)rand()/(float)(RAND_MAX)  < thHealthyClose)
              selectedClose++;
          }
        }
      }
      else
      {
        targetIter.SetVoxel(0);
        weightsIter.SetVoxel(0);
      }

      if (gtvIter.GetVoxel() != 0)
        targetIter.SetVoxel(0);

      ++brainMaskIter;
      ++targetIter;
      ++gtvIter;
      ++targetDil;
      ++excludeHealthyIter;
      ++excludeTumorIter;
      ++weightsIter;
    }
    MITK_INFO << "Training with Samples #Tumor " << tumor << " / healthy # "<< healthy;
    MITK_INFO << "Potential Close" << potentialClose;
    MITK_INFO << "Selected Close" << selectedClose;
  }
    break;
  }

}

void mitk::TumorInvasionClassification::PrepareResponseSamples(mitk::DataCollection *collection)
{
  srand (time(NULL));
  MITK_INFO << "PrepareResponseSamples: Selecting training voxels.";

  EnsureDataImageInCollection(collection, m_TumorID, "WEIGHTS");

  CollectionDilation::DilateBinaryByName(collection,m_TargetID,10,4,"TRAIN");
  DataCollectionImageIterator<unsigned char,3> gtvIter(collection, m_TumorID);
  DataCollectionImageIterator<unsigned char,3> brainMaskIter(collection, m_MaskID);
  DataCollectionImageIterator<unsigned char,3> targetIter(collection, m_TargetID);
  DataCollectionImageIterator<unsigned char,3> dilIter(collection, m_TargetID+"TRAIN");


  // Count Non-Involved/Responsive/Rezidiv Voxels

  double nonInvolved = 0;
  double responsive = 0;
  double rezidiv= 0;

  while (!brainMaskIter.IsAtEnd())
  {
    if (brainMaskIter.GetVoxel() != 0 && dilIter.GetVoxel() !=0)
    {
      if (targetIter.GetVoxel() == 0 && gtvIter.GetVoxel() == 0 )
      {
        targetIter.SetVoxel(1);
        ++nonInvolved;
      }

      if (targetIter.GetVoxel() == 0 && gtvIter.GetVoxel() == 1)
      {
        targetIter.SetVoxel(3);
        ++responsive;
      }

      if (targetIter.GetVoxel() == 1  && gtvIter.GetVoxel() == 0 )
      {
        targetIter.SetVoxel(2);
        ++rezidiv;
      }
    }
    else
      targetIter.SetVoxel(0);

    ++brainMaskIter;
    ++targetIter;
    ++gtvIter;
    ++dilIter;
  }
  brainMaskIter.ToBegin();
  targetIter.ToBegin();
  gtvIter.ToBegin();
  dilIter.ToBegin();

  // weight for groups

  double wNonInvolved = 10000.0 / nonInvolved;
  double wResponsive = 10000.0 / responsive;
  double wRezidiv = 10000.0 / rezidiv;


  std::cout << "Weights are " << wNonInvolved << "/ " << wResponsive << " / " << wRezidiv << std::endl;

  // Assign weight for each voxel
  DataCollectionImageIterator<double,3> weightsIter(collection, "WEIGHTS");

  while (!brainMaskIter.IsAtEnd())
  {
    if (brainMaskIter.GetVoxel() != 0 && dilIter.GetVoxel() !=0)
    {
      if (targetIter.GetVoxel() == 1)
        weightsIter.SetVoxel(wNonInvolved);

      if (targetIter.GetVoxel() == 2)
        weightsIter.SetVoxel(wRezidiv);

      if (targetIter.GetVoxel() == 3)
        weightsIter.SetVoxel(wResponsive);

    }

    ++dilIter;
    ++brainMaskIter;
    ++targetIter;
    ++weightsIter;
  }

}

void mitk::TumorInvasionClassification::LearnProgressionFeatures(mitk::DataCollection *collection, std::vector<std::string> modalitiesList,size_t forestSize , size_t treeDepth )
{
  auto trainDataX = mitk::DCUtilities::DC3dDToMatrixXd(collection, modalitiesList, m_TargetID);
  auto trainDataY = mitk::DCUtilities::DC3dDToMatrixXi(collection, m_TargetID, m_TargetID);

  if (treeDepth != 0)
    m_Forest.SetMaximumTreeDepth(treeDepth);

  m_Forest.SetTreeCount(forestSize);

  if (m_WeightSamples)
  {
    auto trainDataW = mitk::DCUtilities::DC3dDToMatrixXd(collection, "WEIGHTS", m_TargetID);
    m_Forest.SetPointWiseWeight(trainDataW);
    m_Forest.UsePointWiseWeight(true);
  }


  MITK_INFO << "Start Training:";
  try
  {

    m_Forest.Train(trainDataX,trainDataY);
  }
  catch(std::exception& e)
  {
    MITK_INFO << "Exception while training forest: " << e.what();
  }
  std::cout << "Training finished" << std::endl;
}


void mitk::TumorInvasionClassification::PredictInvasion(mitk::DataCollection *collection, std::vector<std::string> modalitiesList)
{
  if (collection != NULL)
  {
    MITK_INFO << "Evaluating Forest";

    auto testDataX = mitk::DCUtilities::DC3dDToMatrixXd(collection,modalitiesList, m_MaskID);
    auto testDataNewY = m_Forest.Predict(testDataX);
    mitk::DCUtilities::MatrixToDC3d(testDataNewY, collection, m_ResultID, m_MaskID);

    Eigen::MatrixXd Probs = m_Forest.GetPointWiseProbabilities();


    Eigen::MatrixXd prob0 = Probs.col(0);
    Eigen::MatrixXd prob1 = Probs.col(1);

    mitk::DCUtilities::MatrixToDC3d(prob0, collection,"prob0", m_MaskID);
    mitk::DCUtilities::MatrixToDC3d(prob1, collection,"prob1", m_MaskID);
    if (Probs.cols() >= 3)
    {
      Eigen::MatrixXd prob2 = Probs.col(2);
      mitk::DCUtilities::MatrixToDC3d(prob2, collection, "prob2", m_MaskID);
    }
  }
  else
    MITK_ERROR << "TumorInvasionClassification::PredictInvasion - provided collection is NULL.";
}

void mitk::TumorInvasionClassification::SanitizeResults(mitk::DataCollection *collection, std::string resultID)
{
  CollectionGrayOpening::PerformGrayOpening(collection,resultID,"OPEN");
}


void mitk::TumorInvasionClassification::DescriptionToLogFile(mitk::DataCollection *collection, std::string outputFile)
{
  std::ofstream fileStream;
  fileStream.open(outputFile.c_str(),std::ios_base::app);

  fileStream << "Configuration:" << std::endl;
  for (size_t patient = 0; patient < collection->Size(); ++patient)
  {
    DataCollection* dataPatient = dynamic_cast<DataCollection*>(collection->GetData(patient).GetPointer());
    fileStream << "Name : " << dataPatient->GetName() << " with time steps:"  << std::endl;
    for (size_t timeStep = 0; timeStep < dataPatient->Size(); ++timeStep)
    {
      fileStream << dataPatient->IndexToName(timeStep)<< "-" << std::endl;
      DataCollection* dataTS = dynamic_cast<DataCollection*>(dataPatient->GetData(timeStep).GetPointer());
      for (size_t ts = 0; ts < dataTS->Size(); ++ts)
        fileStream << dataTS->IndexToName(ts)<< "-" << std::endl;
    }
  }
}

void mitk::TumorInvasionClassification::SaveRandomForest(std::string filename)
{
  mitk::IOUtil::Save(&m_Forest, filename);
}

void mitk::TumorInvasionClassification::LoadRandomForest(std::string /*filename*/)
{
  // TBD
  MITK_ERROR << "not yet implemented ...";
}

