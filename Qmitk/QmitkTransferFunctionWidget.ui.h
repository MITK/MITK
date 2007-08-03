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

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you want to add, delete, or rename functions or slots, use
 ** Qt Designer to update this file, preserving your code.
 **
 ** You should not define a constructor or destructor in this file.
 ** Instead, write your code in functions called init() and destroy().
 ** These will automatically be called by the form's constructor and
 ** destructor.
 *****************************************************************************/
#include "mitkTransferFunctionProperty.h"
#include <mitkDataTreeNode.h>
#include <mitkRenderingManager.h>
#include <qstring.h>

/**
 * \class QmitkTransferFunctionWidget
 * \brief A custom widget for editing transfer functions for volume rendering. 
 * \ingroup Widgets
 */ 
void QmitkTransferFunctionWidget::SetDataTreeNode( mitk::DataTreeNode* node)
{
  if (node) {
    if (mitk::TransferFunctionProperty::Pointer tfp = dynamic_cast<mitk::TransferFunctionProperty*>(node->GetProperty("TransferFunction").GetPointer()))
    {
      if (tfp)
      {
        mitk::TransferFunction* tf = dynamic_cast<mitk::TransferFunction*>(tfp->GetValue().GetPointer());
        if (tf)
        {
          tf->InitializeByMitkImage(dynamic_cast<mitk::Image*>(node->GetData()));
          //std::cout << "TF access" << std::endl;
          m_ScalarOpacityFunctionCanvas->SetPiecewiseFunction(tf->GetScalarOpacityFunction());
          m_ScalarOpacityFunctionCanvas->SetHistogram(tf->GetHistogram());
          m_ColorTransferFunctionCanvas->SetColorTransferFunction(tf->GetColorTransferFunction());
          m_GradientOpacityCanvas ->SetPiecewiseFunctionGO(tf->GetGradientOpacityFunction());
          m_GradientOpacityCanvas->SetHistogram(tf->GetHistogram());
          UpdateMinMaxLabels();
        }
      }
    }
    else 
    {
      mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
      if (mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData())) 
      {
        tf->InitializeByMitkImage(image);
        m_ScalarOpacityFunctionCanvas->SetPiecewiseFunction(tf->GetScalarOpacityFunction());
        m_ScalarOpacityFunctionCanvas->SetHistogram(tf->GetHistogram());
        m_GradientOpacityCanvas->SetPiecewiseFunctionGO(tf->GetGradientOpacityFunction());
        m_GradientOpacityCanvas->SetHistogram(tf->GetHistogram());
        m_ColorTransferFunctionCanvas->SetColorTransferFunction(tf->GetColorTransferFunction());
        UpdateMinMaxLabels();
        node->SetProperty("TransferFunction", new mitk::TransferFunctionProperty(tf.GetPointer()));
      } 
      else 
      {
        std::cerr << "QmitkTransferFunctionWidget::SetDataTreeNode called with non-image node" << std::endl;
      }
    }
    m_ScalarOpacityFunctionCanvas->setEnabled(true);
    m_ScalarOpacityFunctionCanvas->SetQLineEdits(m_XEdit, m_YEdit);
    m_GradientOpacityCanvas->setEnabled(true);
    m_GradientOpacityCanvas->SetQLineEdits(m_XEdit, m_YEdit);
    m_ColorTransferFunctionCanvas->setEnabled(true);
    m_ColorTransferFunctionCanvas->SetQLineEdits(m_XEdit, m_YEdit);
    m_ScalarOpacityFunctionCanvas->update();
    m_GradientOpacityCanvas->update();
    m_ColorTransferFunctionCanvas->update();
  }
  else {
    // called with null-node
    //
    m_ScalarOpacityFunctionCanvas->setEnabled(false);
    m_GradientOpacityCanvas->setEnabled(false);
    m_ColorTransferFunctionCanvas->setEnabled(false);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

void QmitkTransferFunctionWidget::UpdateMinMaxLabels()
{
  if (m_ScalarOpacityFunctionCanvas)
  {
    m_MinLabel->setText(QString::number(m_ScalarOpacityFunctionCanvas->GetMin()));
    m_MinLabel_2->setText(QString::number(m_ScalarOpacityFunctionCanvas->GetMin()));
    m_MaxLabel->setText(QString::number(m_ScalarOpacityFunctionCanvas->GetMax()));
    m_MaxLabel_2->setText(QString::number(m_ScalarOpacityFunctionCanvas->GetMax()));
  }
    if (m_GradientOpacityCanvas)
  {
    m_MinLabelGO->setText(QString::number(m_GradientOpacityCanvas->GetMin()));
    m_MaxLabelGO->setText(QString::number(m_GradientOpacityCanvas->GetMax()));
  }
}

int number_c; //Anzahl Color-Points
int number_s; //Anzahl Scalar-Points

double color_pointsX[50];
double color_pointsR[50];
double color_pointsG[50];
double color_pointsB[50];

double scalar_pointsX[200];
double scalar_pointsY[200];

void QmitkTransferFunctionWidget::ChooseTF(int choice){

 if(choice==1){ //bone + soft-tissue
   ResetTF();

   number_s=4;
   scalar_pointsX[0]=18;
   scalar_pointsY[0]=0;
   scalar_pointsX[1]=20;
   scalar_pointsY[1]=0.3;
   scalar_pointsX[2]=70;
   scalar_pointsY[2]=0.3;
   scalar_pointsX[3]=72;
   scalar_pointsY[3]=0;
 

   AddPoints();
 }

 if(choice==2){ //soft-tissue
   ResetTF();

   number_s=5;
   scalar_pointsX[0]=18;
   scalar_pointsY[0]=0;
   scalar_pointsX[1]=20;
   scalar_pointsY[1]=1;
   scalar_pointsX[2]=70;
   scalar_pointsY[2]=1;
   scalar_pointsX[3]=72;
   scalar_pointsY[3]=0;
   scalar_pointsX[4]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[4]=0;

   AddPoints();
 }

if(choice==3){ //bones
 ResetTF();

   number_s=4;
   scalar_pointsX[0]=170;
   scalar_pointsY[0]=0;
   scalar_pointsX[1]=175;
   scalar_pointsY[1]=1;
   scalar_pointsX[2]=1455;
   scalar_pointsY[2]=0;
   scalar_pointsX[3]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[3]=0;



   AddPoints();


 }

if(choice==101){ //Preset 1
   ResetTF();

   number_s=2;
    if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[0] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.20)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[0] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.20;
   }
   scalar_pointsY[0]=0;
   scalar_pointsX[1]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[1]=1;

   AddPoints();


 }
if(choice==102){ //Preset 2
   ResetTF();
    
   number_s=3;
   scalar_pointsX[0]=m_ScalarOpacityFunctionCanvas->GetFunctionMin();
   scalar_pointsY[0]=0;
   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[1] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[1] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50;
   }
   scalar_pointsY[1]=1;
   scalar_pointsX[2]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[2]=0;
 

   AddPoints();
 }

if(choice==103){ //Preset 3
   ResetTF();
    
   number_s=3;
   scalar_pointsX[0]=m_ScalarOpacityFunctionCanvas->GetFunctionMin();
   scalar_pointsY[0]=0.5;
   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[1] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[1] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50;
   }
   scalar_pointsY[1]=0.5;
   scalar_pointsX[2]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[2]=0;
 

   AddPoints();
 }

if(choice==104){ //Preset 4
   ResetTF();
    
   number_s=5;
   scalar_pointsX[0]=m_ScalarOpacityFunctionCanvas->GetFunctionMin();
   scalar_pointsY[0]=0;
   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[1] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.25)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[1] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.25;
   }
   scalar_pointsY[1]=0.5;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[2] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[2] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50;
   }
   scalar_pointsY[2]=0;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[3] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.75)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[3] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.75;
   }
   scalar_pointsY[3]=1;

   scalar_pointsX[4]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[4]=0;
 

   AddPoints();
 }

if(choice==105){ //Preset 5
   ResetTF();
    
   number_s=3;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[0] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[0] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50;
   }
   scalar_pointsY[0]=0;

    if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[1] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.75)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[1] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.75;
   }
   scalar_pointsY[1]=1;

   scalar_pointsX[2]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[2]=0;


   AddPoints();
 }

if(choice==106){ //Preset 6
   ResetTF();
    
   number_s=2;
   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[0] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[0] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50;
   }
   scalar_pointsY[0]=0;
   scalar_pointsX[1]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[1]=1;
 

   AddPoints();
 }

if(choice==107){ //Preset 7
   ResetTF();
    
   number_s=11;
   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[0] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.005)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[0] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.005;
   }
   scalar_pointsY[0]=0;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[1] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.05)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[1] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.05;
   }
   scalar_pointsY[1]=0.85;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[2] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.1)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[2] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.1;
   }
   scalar_pointsY[2]=0.90;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[3] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.15)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[3] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.15;
   }
   scalar_pointsY[3]=0.95;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[4] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.2)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[4] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.2;
   }
   scalar_pointsY[4]=0.975;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[5] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.25)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[5] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.25;
   }
   scalar_pointsY[5]=1;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[6] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.3)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[6] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.3;
   }
   scalar_pointsY[6]=0.975;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[7] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.35)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[7] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.35;
   }
   scalar_pointsY[7]=0.95;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[8] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.4)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[8] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.4;
   }
   scalar_pointsY[8]=0.9;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[9] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.45)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[9] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.45;
   }
   scalar_pointsY[9]=0.85;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[9] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.495)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[9] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.495;
   }
   scalar_pointsY[9]=0;

   scalar_pointsX[10]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[10]=0;
 

   AddPoints();
 }
if(choice==108){ //Preset 8
   ResetTF();
    
   number_s=4;
   scalar_pointsX[0]=m_ScalarOpacityFunctionCanvas->GetFunctionMin();
   scalar_pointsY[0]=0;
   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[1] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[1] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50;
   }
   scalar_pointsY[1]=1;
    if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[2] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin())+0.01;
   }
   else{
  scalar_pointsX[2] =((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+0.01;
   }
   scalar_pointsY[2]=0;
   scalar_pointsX[3]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[3]=0;
 

   AddPoints();
 }
if(choice==109){ //Preset 9
   ResetTF();
    
   number_s=4;
   scalar_pointsX[0]=m_ScalarOpacityFunctionCanvas->GetFunctionMin();
   scalar_pointsY[0]=0;
   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[1] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[1] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50;
   }
   scalar_pointsY[1]=0;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[2] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin())+0.01;
   }
   else{
  scalar_pointsX[2] =((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.50)+0.01;
   }
   scalar_pointsY[2]=1;

   scalar_pointsX[3]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[3]=0;
 

   AddPoints();
 }

if(choice==110){ //Preset 10
   ResetTF();
    
   number_s=6;
   scalar_pointsX[0]=m_ScalarOpacityFunctionCanvas->GetFunctionMin();
   scalar_pointsY[0]=0;
   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[1] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.40)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin())-0.01;
   }
   else{
  scalar_pointsX[1] =((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.40)-0.01;
   }
   scalar_pointsY[1]=0;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[2] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.40)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[2] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.40;
   }
   scalar_pointsY[2]=1;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[3] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.60)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin());
   }
   else{
  scalar_pointsX[3] =(m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.60;
   }
   scalar_pointsY[3]=1;

   if((m_ScalarOpacityFunctionCanvas->GetFunctionMin())<0){
     scalar_pointsX[4] = ((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.60)+(m_ScalarOpacityFunctionCanvas->GetFunctionMin())+0.01;
   }
   else{
  scalar_pointsX[4] =((m_ScalarOpacityFunctionCanvas->GetFunctionRange())*0.60)+0.01;
   }
   scalar_pointsY[4]=0;

   scalar_pointsX[5]=m_ScalarOpacityFunctionCanvas->GetFunctionMax();
   scalar_pointsY[5]=0;
 

   AddPoints();
 }

 
 if(choice==999){ //Reset
   ResetTF();
   ResetCS();
   m_GradientOpacityCanvas->ResetGO();
 }
}

void QmitkTransferFunctionWidget::AddPoints()
{


 for(int i=0; i<number_s; i++){
 m_ScalarOpacityFunctionCanvas->AddFunctionPoint(scalar_pointsX[i],scalar_pointsY[i]);
 std::cout<<"Added scalar point at("<<scalar_pointsX[i]<<"/"<<scalar_pointsY[i]<<")\n";
 }

 m_ScalarOpacityFunctionCanvas->update();
 mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}


void QmitkTransferFunctionWidget::ResetTF()
{
   
   /*
   for(int i=0; i<number_c;i++){
    m_ColorTransferFunctionCanvas->RemoveFunctionPoint(color_pointsX[i]);
    std::cout<<"Removed color point at "<<color_pointsX[i]<<"\n";
  }

   for(int i=0; i<number_s;i++){
    m_ScalarOpacityFunctionCanvas->RemoveFunctionPoint(scalar_pointsX[i]);
    std::cout<<"Removed scalar point at ("<<scalar_pointsX[i]<<"/"<<scalar_pointsY[i]<<")\n";
  }
  */
  m_ScalarOpacityFunctionCanvas->RemoveAllFunctionPoints();
  m_ScalarOpacityFunctionCanvas->update();
  //m_GradientOpacityCanvas->ResetGO();
  m_GradientOpacityCanvas->update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

/*** Color ***/
void QmitkTransferFunctionWidget::ChooseCS(int cstyle){

  if(cstyle == 1){//bone - natural
   ResetCS();
   number_c=2;
   color_pointsX[0]=m_ColorTransferFunctionCanvas->GetFunctionMin();
   color_pointsR[0]=0.4;
   color_pointsG[0]=0.37;
   color_pointsB[0]=0.29;

   color_pointsX[1]=m_ColorTransferFunctionCanvas->GetFunctionMax();
   color_pointsR[1]=0.97;
   color_pointsG[1]=1;
   color_pointsB[1]=0.6;
   AddCS();
  }

  if(cstyle == 2){//bone - high contrast
   ResetCS();
   number_c=4;
   color_pointsX[0]=140;
   color_pointsR[0]=0;
   color_pointsG[0]=1;
   color_pointsB[0]=0;

   color_pointsX[1]=300;
   color_pointsR[1]=1;
   color_pointsG[1]=0;
   color_pointsB[1]=0;

   color_pointsX[2]=500;
   color_pointsR[2]=0;
   color_pointsG[2]=0;
   color_pointsB[2]=1;

   color_pointsX[3]=700;
   color_pointsR[3]=1;
   color_pointsG[3]=1;
   color_pointsB[3]=0;
   AddCS();
  }

  if(cstyle == 3){//soft tissue - natural
   ResetCS();
   number_c=4;
   color_pointsX[0]=18;
   color_pointsR[0]=0.93;
   color_pointsG[0]=0.5;
   color_pointsB[0]=0.4;
   color_pointsX[1]=45;
   color_pointsR[1]=0.79;
   color_pointsG[1]=0.6;
   color_pointsB[1]=0.52;
   color_pointsX[2]=70;
   color_pointsR[2]=0.82;
   color_pointsG[2]=0.57;
   color_pointsB[2]=0.53;
   color_pointsX[3]=m_ColorTransferFunctionCanvas->GetFunctionMax();
   color_pointsR[3]=0.82;
   color_pointsG[3]=0.57;
   color_pointsB[3]=0.53;
   AddCS();
  }

  if(cstyle == 4){//muscles-bones
   ResetCS();

   double step = (m_ColorTransferFunctionCanvas->GetFunctionRange())/256.0;

   number_c=9;
   color_pointsX[0]=m_ColorTransferFunctionCanvas->GetFunctionMin();
   color_pointsR[0]=0;
   color_pointsG[0]=0;
   color_pointsB[0]=0;
   std::cout<<"Point 1 x= "<<color_pointsX[0]<<"\n";

   color_pointsX[1]=(m_ColorTransferFunctionCanvas->GetFunctionMin())+10*step;
   color_pointsR[1]=26/255.0;
   color_pointsG[1]=2/255.0;
   color_pointsB[1]=2/255.0;

   color_pointsX[2]=(m_ColorTransferFunctionCanvas->GetFunctionMin())+20*step;
   color_pointsR[2]=53/255.0;
   color_pointsG[2]=4/255.0;
   color_pointsB[2]=5/255.0;

   color_pointsX[3]=(m_ColorTransferFunctionCanvas->GetFunctionMin())+40*step;
   color_pointsR[3]=107/255.0;
   color_pointsG[3]=9/255.0;
   color_pointsB[3]=11/255.0;

   color_pointsX[4]=(m_ColorTransferFunctionCanvas->GetFunctionMin())+60*step;
   color_pointsR[4]=161/255.0;
   color_pointsG[4]=13/255.0;
   color_pointsB[4]=17/255.0;

   color_pointsX[5]=(m_ColorTransferFunctionCanvas->GetFunctionMin())+100*step;
   color_pointsR[5]=255/255.0;
   color_pointsG[5]=36/255.0;
   color_pointsB[5]=26/255.0;

   color_pointsX[6]=(m_ColorTransferFunctionCanvas->GetFunctionMin())+150*step;
   color_pointsR[6]=255/255.0;
   color_pointsG[6]=180/255.0;
   color_pointsB[6]=11/255.0;

   color_pointsX[7]=(m_ColorTransferFunctionCanvas->GetFunctionMin())+200*step;
   color_pointsR[7]=255/255.0;
   color_pointsG[7]=240/255.0;
   color_pointsB[7]=38/255.0;

   color_pointsX[8]=(m_ColorTransferFunctionCanvas->GetFunctionMax());
   color_pointsR[8]=255/255.0;
   color_pointsG[8]=254/255.0;
   color_pointsB[8]=251/255.0;

   AddCS();
  }

}


void QmitkTransferFunctionWidget::ResetCS()
{
 m_ColorTransferFunctionCanvas->RemoveAllFunctionPoints();
 m_ColorTransferFunctionCanvas->update();
 mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}


void QmitkTransferFunctionWidget::AddCS()
{
 for(int i=0; i<number_c;i++){
   m_ColorTransferFunctionCanvas->AddRGB(color_pointsX[i],color_pointsR[i],color_pointsG[i],color_pointsB[i]);
   std::cout<<"Added color point at "<<color_pointsX[i]<< " RGB: "<<color_pointsR[i]<<color_pointsG[i]<<color_pointsB[i]<<"\n";
 }
 m_ColorTransferFunctionCanvas->update();
 mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTransferFunctionWidget::ImmediateUpdate(bool state)
{
    m_ScalarOpacityFunctionCanvas->SetImmediateUpdate(state);
    m_ColorTransferFunctionCanvas->SetImmediateUpdate(state);
    m_GradientOpacityCanvas->SetImmediateUpdate(state);	    
}

void QmitkTransferFunctionWidget::SetXValue()
{
  m_ScalarOpacityFunctionCanvas->SetX(m_XEdit->text().toFloat());
  m_ColorTransferFunctionCanvas->SetX(m_XEdit->text().toFloat());
  m_GradientOpacityCanvas->SetX(m_XEdit->text().toFloat());
}


void QmitkTransferFunctionWidget::SetYValue()
{
  m_ScalarOpacityFunctionCanvas->SetY(m_YEdit->text().toFloat());
  m_GradientOpacityCanvas->SetY(m_YEdit->text().toFloat());
}
