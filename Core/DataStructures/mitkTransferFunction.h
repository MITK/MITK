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


#ifndef MITK_TRANSFER_FUNCTION_H_HEADER_INCLUDED
#define MITK_TRANSFER_FUNCTION_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <algorithm>
#include <set>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <mitkImage.h>
#include <mitkHistogramGenerator.h>
namespace mitk
{

  //##
  //##Documentation
  //## @brief
  //##
  //##
  class TransferFunction : public itk::Object 
  {
    public:
      TransferFunction(int min, int max) : m_Min(min), m_Max(max) , m_ColorTransferFunction(vtkColorTransferFunction::New()), m_ScalarOpacityFunction(vtkPiecewiseFunction::New()), m_Valid(false), m_Histogram(NULL)  {
        this->m_ScalarOpacityFunction->Initialize();
        // this->m_ColorTransferFunction->Initialize();
        
        // m_VtkElements.push_back();
      }
      TransferFunction() : m_Min(0), m_Max(255) , m_ColorTransferFunction(vtkColorTransferFunction::New()), m_ScalarOpacityFunction(vtkPiecewiseFunction::New()), m_Valid(false), m_Histogram(NULL)  {
        this->m_ScalarOpacityFunction->Initialize();
        // this->m_ColorTransferFunction->Initialize();
      }
      mitkClassMacro(TransferFunction, itk::DataObject);
      itkNewMacro(Self);
      itkSetMacro(Min,int);
      itkSetMacro(Max,int);
      itkGetMacro(Min,int);
      itkGetMacro(Max,int);
      itkGetMacro(ScalarOpacityFunction,vtkPiecewiseFunction*); 
      itkGetMacro(ColorTransferFunction,vtkColorTransferFunction*); 
      itkGetMacro(Valid,bool);
      itkGetConstObjectMacro(Histogram,HistogramGenerator::HistogramType);
      void UpdateVtkFunctions();
      void InitializeByMitkImage(const mitk::Image* image);
      class RGBO {
        public:
	RGBO(int red, int green, int blue, float opacity) : m_Red(red),m_Green(green),m_Blue(blue),m_Opacity(opacity) {}
        RGBO() : m_Red(0),m_Green(0),m_Blue(0),m_Opacity(0.0) {}  
        int m_Red,m_Green,m_Blue;
        float m_Opacity;
        RGBO operator+(const RGBO &other) {
          float opacitySum = other.m_Opacity + m_Opacity;
          RGBO result;
          if (opacitySum <= 0.0) {
            result.m_Opacity = 0.0f;
          } else {
            result.m_Opacity = opacitySum;
            result.m_Red = (int)((m_Red * m_Opacity + other.m_Red * other.m_Opacity) / opacitySum);
            result.m_Green = (int)((m_Green * m_Opacity + other.m_Green * other.m_Opacity) / opacitySum);
            result.m_Blue = (int)((m_Blue * m_Opacity + other.m_Blue * other.m_Opacity) / opacitySum);
          }
          return result;
        }
      };  
      class Element;
      typedef std::set<Element*> ElementSetType; 	
      static void FillValues(std::vector<RGBO> &values, const ElementSetType &elements);
      
      ElementSetType m_Elements;
      ElementSetType& GetElements() { return m_Elements; }
      
      class Handle {

        public:
          typedef std::pair<float,float> PosType;
          Element* m_Element;
          PosType m_Pos;
          PosType m_Min, m_Max;
          Handle(Element* element, float x,float y) : m_Element(element), m_Pos(x,y),m_Min(0.0f,0.0f),m_Max(1.0f,1.0f) {/*  m_Element->m_Handles.insert(this); */}
          ~Handle() {/*m_Handles.erase(this);*/}
          void Set(float x,float y) { 
            m_Pos = std::make_pair(x,y);   
            ClipPos();
          } 
          PosType& GetPos() { return m_Pos; }
          bool operator<(Handle h) {
            return m_Pos.first < h.m_Pos.first;
          }
          void ClipPos(void) {
            m_Pos.first = m_Pos.first > m_Min.first ? m_Pos.first : m_Min.first;
            m_Pos.first = m_Pos.first < m_Max.first ? m_Pos.first : m_Max.first;

            m_Pos.second = m_Pos.second > m_Min.second ? m_Pos.second : m_Min.second;
            m_Pos.second = m_Pos.second < m_Max.second ? m_Pos.second : m_Max.second;
          }

      };
      class Element {
        public:
          /** triggers recalculation
           */
          void UpdateVtkFunctions();
          
          typedef std::vector<Handle*> HandleSetType;
          HandleSetType m_Handles;
          virtual float GetValueAt(float x) const = 0 ;
          virtual RGBO GetRGBOAt(float x) {
             return RGBO(m_Red,m_Green,m_Blue,GetValueAt(x));
          }
          Element() : m_Red(127),m_Green(127),m_Blue(127){
          
          }
          virtual ~Element() {
            for (HandleSetType::iterator it = m_Handles.begin() ; it != m_Handles.end(); it++ ) {
              delete *it;
            }
          }
          virtual void Set(Handle* handle, float x, float y) {
            handle->Set(x,y);
            UpdateConstraints();
          }
          virtual void UpdateConstraints() {};
          int m_Red,m_Green,m_Blue;
        protected:

      };
      class SimpleElement : public Element {
        public:
          SimpleElement(float x, float y) : m_Window(.1) {
            m_Handles.push_back(new Handle(this,x,y));
          }
          virtual void SetX(float x) {m_Handles[0]->GetPos().first = x; m_Handles[0]->ClipPos();}
          virtual void SetY(float y) { m_Handles[0]->GetPos().second = y; m_Handles[0]->ClipPos();}
          virtual float GetValueAt(float x) const {
            float normX =  x - m_Handles[0]->m_Pos.first;
            float normR =  m_Window / 2;
            float normL =  - m_Window / 2;
            if (normX < 2*normL) return 0;
            if (normX > 2*normR) return 0;
            if (normX<0) {
              return m_Handles[0]->m_Pos.second * (cos(normX / normL * 3.1415*.5) * .5 + .5);
            } else {
              return m_Handles[0]->m_Pos.second * (cos(normX / normR * 3.1415*.5) * .5 + .5);
            }
          }  

          float m_Window; 
      };

      class TriElement : public Element {
        public:	
          TriElement(float x,float y) {
            m_Handles.push_back(new Handle(this,x,y));
            m_Handles.push_back(new Handle(this,x+.1,y/2));
            m_Handles.push_back(new Handle(this,x-.1,y/2));
          };
          virtual float GetValueAt(float x) const {
            float normX =  x - GetTopHandle()->m_Pos.first;
            float normR =  GetRightHandle()->m_Pos.first - GetTopHandle()->m_Pos.first;
            float normL =  GetLeftHandle()->m_Pos.first - GetTopHandle()->m_Pos.first;
            if (normX < 2*normL) return 0;
            if (normX > 2*normR) return 0;
            if (normX<0) {
              return GetTopHandle()->m_Pos.second * (cos(normX / normL * 3.1415*.5) * .5 + .5);
            } else {
              return GetTopHandle()->m_Pos.second * (cos(normX / normR * 3.1415*.5) * .5 + .5);
            }
          } 
          Handle* GetTopHandle() const { return m_Handles[0];}
          Handle* GetRightHandle() const { return m_Handles[1];}
          Handle* GetLeftHandle() const { return m_Handles[2];}
          virtual void UpdateConstraints() {
            // reconsider whether we really need this 
            GetTopHandle()->m_Min.first = GetLeftHandle()->m_Pos.first; 
            GetTopHandle()->m_Max.first = GetRightHandle()->m_Pos.first; 
            GetTopHandle()->m_Min.second = vnl_math_max(GetLeftHandle()->m_Pos.second,GetRightHandle()->m_Pos.second);
            GetLeftHandle()->m_Max.second = .5 * GetTopHandle()->m_Pos.second;
            GetLeftHandle()->m_Min.second = .5 * GetTopHandle()->m_Pos.second;
            GetRightHandle()->m_Max.second = .5 * GetTopHandle()->m_Pos.second;
            GetRightHandle()->m_Min.second = .5 * GetTopHandle()->m_Pos.second;
            for (HandleSetType::iterator it = m_Handles.begin(); it != m_Handles.end() ; it++) {
              (*it)->ClipPos();
            }

            //GetLeftHandle()->m_Max.second = GetRightHandle()->m_Max.second = GetTopHandle()->m_Pos.second;
          };

      };
    protected:
          int m_Min;
          int m_Max;
          std::vector<Handle*> m_VtkElements;
      
      vtkColorTransferFunction* m_ColorTransferFunction;
      vtkPiecewiseFunction* m_ScalarOpacityFunction;
      bool m_Valid;
      mitk::HistogramGenerator::HistogramType::ConstPointer m_Histogram; 
  };
}
#endif 
// MITK_TRANSFER_FUNCTION_H_HEADER_INCLUDED
