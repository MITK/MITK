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

#include "vtkMitkThickSlicesFilter.h"

#include "vtkDataArray.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <math.h>
#include <vtksys/ios/sstream>

vtkStandardNewMacro(vtkMitkThickSlicesFilter);

//----------------------------------------------------------------------------
// Construct an instance of vtkMitkThickSlicesFilter filter.
vtkMitkThickSlicesFilter::vtkMitkThickSlicesFilter()
{
  this->HandleBoundaries = 1;
  this->Dimensionality = 2;

  this->m_CurrentMode = MIP;

  // by default process active point scalars
  this->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,
                               vtkDataSetAttributes::SCALARS);
}

//----------------------------------------------------------------------------
void vtkMitkThickSlicesFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "HandleBoundaries: " << this->HandleBoundaries << "\n";
  os << indent << "Dimensionality: " << this->Dimensionality << "\n";
}

//----------------------------------------------------------------------------
int vtkMitkThickSlicesFilter::RequestInformation(vtkInformation*,
                                         vtkInformationVector** inputVector,
                                         vtkInformationVector* outputVector)
{
  // Get input and output pipeline information.
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

  // Get the input whole extent.
  int extent[6];
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent);

  // Reduce 3D to 2D output
  extent[4] = extent[5] = 0;

  // Store the new whole extent for the output.
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);

/*
  // Set the number of point data componets to the number of
  // components in the gradient vector.
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_DOUBLE,
                                              this->Dimensionality);
  */
  return 1;
}

//----------------------------------------------------------------------------
// This method computes the input extent necessary to generate the output.
int vtkMitkThickSlicesFilter::RequestUpdateExtent(vtkInformation*,
                                          vtkInformationVector** inputVector,
                                          vtkInformationVector* outputVector)
{
  // Get input and output pipeline information.
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

  // Get the input whole extent.
  int wholeExtent[6];
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExtent);

  // Get the requested update extent from the output.
  int inUExt[6];
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), inUExt);

  /*inUExt[4] -= 5;
  inUExt[5] += 5;

  if (inUExt[4] < wholeExtent[4]) */inUExt[4] = wholeExtent[4];
  /*if (inUExt[5] > wholeExtent[5]) */inUExt[5] = wholeExtent[5];

  // Store the update extent needed from the intput.
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), inUExt, 6);

  return 1;
}

//----------------------------------------------------------------------------
// This execute method handles boundaries.
// it handles boundaries. Pixels are just replicated to get values
// out of extent.
template <class T>
void vtkMitkThickSlicesFilterExecute(vtkMitkThickSlicesFilter *self,
                             vtkImageData *inData, T *inPtr,
                             vtkImageData *outData, T *outPtr,
                             int outExt[6], int /*id*/)
{
  int idxX, idxY;
  int maxX, maxY;
  vtkIdType inIncX, inIncY, inIncZ;
  vtkIdType outIncX, outIncY, outIncZ;
  //int axesNum;
  int *inExt = inData->GetExtent();
  int *wholeExtent;
  vtkIdType *inIncs;
  //int useYMin, useYMax, useXMin, useXMax;

  // find the region to loop over
  maxX = outExt[1] - outExt[0];
  maxY = outExt[3] - outExt[2];

//  maxZ = outExt[5] - outExt[4];

  // Get the dimensionality of the gradient.
  //axesNum = self->GetDimensionality();

  // Get increments to march through data
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
                   /*
  // The data spacing is important for computing the gradient.
  // central differences (2 * ratio).
  // Negative because below we have (min - max) for dx ...
  inData->GetSpacing(r);
  r[0] = -0.5 / r[0];
  r[1] = -0.5 / r[1];
  r[2] = -0.5 / r[2];
                     */
  // get some other info we need
  inIncs = inData->GetIncrements();
  wholeExtent = inData->GetExtent();

  // Move the pointer to the correct starting position.
  inPtr += (outExt[0]-inExt[0])*inIncs[0] +
           (outExt[2]-inExt[2])*inIncs[1] +
           (outExt[4]-inExt[4])*inIncs[2];

  // Loop through ouput pixels

  int _minZ = /*-5 + outExt[4];  if( _minZ  < wholeExtent[4]) _minZ=*/wholeExtent[4];
  int _maxZ = /* 5 + outExt[4];  if( _maxZ  > wholeExtent[5]) _maxZ=*/wholeExtent[5];

  if(_maxZ<_minZ)
    return;

  double invNum = 1.0 / (_maxZ-_minZ+1) ;


  switch(self->GetThickSliceMode())
  {
    default:
    case vtkMitkThickSlicesFilter::MIP:
      {
        //MIP
        for (idxY = 0; idxY <= maxY; idxY++)
        {
          //useYMin = ((idxY + outExt[2]) <= wholeExtent[2]) ? 0 : -inIncs[1];
          //useYMax = ((idxY + outExt[2]) >= wholeExtent[3]) ? 0 : inIncs[1];
          for (idxX = 0; idxX <= maxX; idxX++)
          {
            //useXMin = ((idxX + outExt[0]) <= wholeExtent[0]) ? 0 : -inIncs[0];
            //useXMax = ((idxX + outExt[0]) >= wholeExtent[1]) ? 0 : inIncs[0];

            T mip = inPtr[_minZ*inIncs[2]];

            for(int z = _minZ+1; z<= _maxZ;z++)
            {
              T value = inPtr[z*inIncs[2]];
              if(value > mip)
                mip=value;
            }

            // do X axis
            *outPtr = mip;
            outPtr++;
            inPtr++;
          }
          outPtr += outIncY;
          inPtr += inIncY;
        }
      }
      break;

    case vtkMitkThickSlicesFilter::SUM:
      {
        //MIP
        for (idxY = 0; idxY <= maxY; idxY++)
        {
          //useYMin = ((idxY + outExt[2]) <= wholeExtent[2]) ? 0 : -inIncs[1];
          //useYMax = ((idxY + outExt[2]) >= wholeExtent[3]) ? 0 : inIncs[1];
          for (idxX = 0; idxX <= maxX; idxX++)
          {
            //useXMin = ((idxX + outExt[0]) <= wholeExtent[0]) ? 0 : -inIncs[0];
            //useXMax = ((idxX + outExt[0]) >= wholeExtent[1]) ? 0 : inIncs[0];

            double sum = 0;

            for(int z = _minZ; z<= _maxZ;z++)
            {
              T value = inPtr[z*inIncs[2]];
              sum += value;
            }

            // do X axis
            *outPtr = static_cast<T>(invNum*sum);
            outPtr++;
            inPtr++;
          }
          outPtr += outIncY;
          inPtr += inIncY;
        }
      }
      break;

  case vtkMitkThickSlicesFilter::WEIGHTED:
    {
      const int size = _maxZ-_minZ;
      std::vector<double> weights(size);
      double mean = 0.5 * double(_minZ + _maxZ);
      double sigma_sq = double(size) / 6.0;
      sigma_sq *= sigma_sq;
      double sum = 0;
      int i=0;
      for(int z = _minZ+1; z<= _maxZ;z++)
      {
        double val = exp(-(((double)z-mean)/sigma_sq));
        weights[i++] = val;
        sum += val;
      }
      for(i=0; i<size; i++)
      {
        weights[i] /= sum;
      }

      for (idxY = 0; idxY <= maxY; idxY++)
      {
        //useYMin = ((idxY + outExt[2]) <= wholeExtent[2]) ? 0 : -inIncs[1];
        //useYMax = ((idxY + outExt[2]) >= wholeExtent[3]) ? 0 : inIncs[1];
        for (idxX = 0; idxX <= maxX; idxX++)
        {
          //useXMin = ((idxX + outExt[0]) <= wholeExtent[0]) ? 0 : -inIncs[0];
          //useXMax = ((idxX + outExt[0]) >= wholeExtent[1]) ? 0 : inIncs[0];

          T mip = inPtr[_minZ*inIncs[2]];
          i=0;
          double mymip = 0;
          for(int z = _minZ+1; z<= _maxZ;z++)
          {
            double value = inPtr[z*inIncs[2]];
            mymip+=value*weights[i++];
          }
          mip = static_cast<T>(mymip);
          // do X axis
          *outPtr = mip;
          outPtr++;
          inPtr++;
        }
        outPtr += outIncY;
        inPtr += inIncY;
      }
    }
    break;

  case vtkMitkThickSlicesFilter::MINIP:
    {
      for (idxY = 0; idxY <= maxY; idxY++)
      {
        for (idxX = 0; idxX <= maxX; idxX++)
        {
          T mip = inPtr[_minZ*inIncs[2]];

          for(int z = _minZ+1; z<= _maxZ;z++)
          {
            T value = inPtr[z*inIncs[2]];
            if(value < mip)
              mip=value;
          }

          // do X axis
          *outPtr = mip;
          outPtr++;
          inPtr++;
        }
        outPtr += outIncY;
        inPtr += inIncY;
      }
    }
    break;

  case vtkMitkThickSlicesFilter::MEAN:
    {
      const int size = _maxZ-_minZ;

      //MEAN
      for (idxY = 0; idxY <= maxY; idxY++)
      {
        for (idxX = 0; idxX <= maxX; idxX++)
        {
          T sum = 0;
          for(int z = _minZ; z <= _maxZ;z++)
          {
            T value = inPtr[z*inIncs[2]];
            sum += value;
          }

          T mip = sum/size;

          // do X axis
          *outPtr = mip;
          outPtr++;
          inPtr++;
        }
        outPtr += outIncY;
        inPtr += inIncY;
      }
    }
    break;


  }

}

int vtkMitkThickSlicesFilter::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  if (!this->Superclass::RequestData(request, inputVector, outputVector))
    {
    return 0;
    }
  vtkImageData* output = vtkImageData::GetData(outputVector);
  vtkDataArray* outArray = output->GetPointData()->GetScalars();
  vtksys_ios::ostringstream newname;
  newname << (outArray->GetName()?outArray->GetName():"")
    << "Gradient";
  outArray->SetName(newname.str().c_str());
  // Why not pass the original array?
  if (this->GetInputArrayToProcess(0, inputVector))
    {
    output->GetPointData()->AddArray(
        this->GetInputArrayToProcess(0, inputVector));
    }
  return 1;
}

//----------------------------------------------------------------------------
// This method contains a switch statement that calls the correct
// templated function for the input data type.  This method does handle
// boundary conditions.
void vtkMitkThickSlicesFilter::ThreadedRequestData(vtkInformation*,
                                           vtkInformationVector** inputVector,
                                           vtkInformationVector*,
                                           vtkImageData*** inData,
                                           vtkImageData** outData,
                                           int outExt[6],
                                           int threadId)
{
  // Get the input and output data objects.
  vtkImageData* input = inData[0][0];
  vtkImageData* output = outData[0];

  // The ouptut scalar type must be double to store proper gradients.
/*
  if(output->GetScalarType() != VTK_DOUBLE)
    {
    vtkErrorMacro("Execute: output ScalarType is "
                  << output->GetScalarType() << "but must be double.");
    return;
    }
  */
  vtkDataArray* inputArray = this->GetInputArrayToProcess(0, inputVector);
  if (!inputArray)
    {
    vtkErrorMacro("No input array was found. Cannot execute");
    return;
    }

  // Gradient makes sense only with one input component.  This is not
  // a Jacobian filter.
  if(inputArray->GetNumberOfComponents() != 1)
    {
    vtkErrorMacro(
      "Execute: input has more than one component. "
      "The input to gradient should be a single component image. "
      "Think about it. If you insist on using a color image then "
      "run it though RGBToHSV then ExtractComponents to get the V "
      "components. That's probably what you want anyhow.");
    return;
    }

  void* inPtr = inputArray->GetVoidPointer(0);
  void* outPtr = output->GetScalarPointerForExtent(outExt);

  switch(inputArray->GetDataType())
    {
    vtkTemplateMacro(
      vtkMitkThickSlicesFilterExecute(this, input, static_cast<VTK_TT*>(inPtr), output, static_cast<VTK_TT*>(outPtr), outExt, threadId)
      );
    default:
      vtkErrorMacro("Execute: Unknown ScalarType " << input->GetScalarType());
      return;
    }
}
