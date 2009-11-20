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

#ifndef QMITKTRANSFERFUNCTIONCANVAS_H_INCLUDED
#define QMITKTRANSFERFUNCTIONCANVAS_H_INCLUDED

#include <mitkHistogramGenerator.h>
#include <mitkRenderingManager.h>

#include <QWidget>
#include <QLineEdit>

#include <vtkPiecewiseFunction.h>

#include <mitkCommon.h>
#include <mitkImage.h>
#include "mitkImageCast.h"
#include <mitkWeakPointer.h>


namespace mitk {

class QMITKEXT_EXPORT SimpleHistogram
{
  public:
  
  SimpleHistogram()
  {
    valid=false;
    histogram=0;
  }
  
  ~SimpleHistogram()
  {
    if(histogram)
      delete histogram;
  }
  
  typedef itk::Image<short, 3>          CTImage;
  typedef itk::ImageRegionIterator< CTImage  > CTIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< CTImage  > CTIteratorIndexType;
  
  typedef itk::Image<unsigned char, 3> BinImage;
  typedef itk::ImageRegionIterator< BinImage > BinIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< BinImage > BinIteratorIndexType;

  typedef unsigned long CountType;

  protected:

  CountType *histogram;

  bool valid;
  
  int first;
  int last;
  int min;
  int max;
  CountType highest;
  double invLogHighest;
  
  public:
  
  int GetMin()
  {
    if(!valid)
      return 0;
      
    return min;
  }
  
  int GetMax()
  {
    if(!valid)
      return 0;
      
    return max;
  }
  
  void ComputeFromImage( Image::Pointer source );
  float GetRelativeBin( float start, float end );
  
};

class QMITKEXT_EXPORT SimpleHistogramCache
{
  public:
  
    static const int maxCacheSize = 64;

  protected:

    class Element
    {
      public:
        mitk::WeakPointer<mitk::Image> image;
        itk::TimeStamp m_LastUpdateTime;
        SimpleHistogram histogram;
    };

    typedef std::list<Element*> CacheContainer;
    
    CacheContainer cache;
    
  public:
  
    SimpleHistogramCache()
    {
      
    }
    
    ~SimpleHistogramCache()
    {
      TrimCache(true);
    }

    SimpleHistogram *operator[](mitk::Image::Pointer sp_Image)
    {
      mitk::Image *p_Image = sp_Image.GetPointer();

      if(!p_Image)
      {
        LOG_WARN << "SimpleHistogramCache::operator[] with null image called";
        return 0;
      }
      
      Element *elementToUpdate = 0;
      
      bool first = true;
      
      for(CacheContainer::iterator iter = cache.begin(); iter != cache.end(); iter++)
      {
        Element *e = *iter;
        mitk::Image *p_tmp = e->image.GetPointer();

        if(p_tmp == p_Image)
        {
          if(!first)
          {
            cache.erase(iter);
            cache.push_front(e);
          }
          if( p_Image->GetMTime() > e->m_LastUpdateTime.GetMTime())
            goto recomputeElement;
          
          LOG_INFO << "using a cached histogram";
          
          return &e->histogram;
        }
          
        first = false;
      }

      elementToUpdate = new Element();
      elementToUpdate->image = p_Image;
      cache.push_front(elementToUpdate);
      TrimCache();

      recomputeElement:

      LOG_INFO << "computing a new histogram";

      elementToUpdate->histogram.ComputeFromImage(p_Image);
      elementToUpdate->m_LastUpdateTime.Modified();
      return &elementToUpdate->histogram;
    }
    
  protected:
 
    void TrimCache(bool full=false)
    {
      int targetSize = full?0:maxCacheSize;
      
      while(cache.size()>targetSize)
      {
        delete cache.back();
        cache.pop_back();
      }
            
    }
    
};



}


class QMITKEXT_EXPORT QmitkTransferFunctionCanvas : public QWidget
{

  Q_OBJECT

public:

  QmitkTransferFunctionCanvas( QWidget * parent=0, Qt::WindowFlags f = 0 );

  mitk::SimpleHistogram* GetHistogram()
  {
    return m_Histogram;
  }

  void SetHistogram(mitk::SimpleHistogram *histogram)
  {
    m_Histogram = histogram;
  }

  vtkFloatingPointType GetMin()
  {
    return m_Min;
  }

  void SetMin(vtkFloatingPointType min)
  {
    this->m_Min = min;
    SetLower(min);
  }

  vtkFloatingPointType GetMax()
  {
    return m_Max;
  }

  void SetMax(vtkFloatingPointType max)
  {
    this->m_Max = max;
    SetUpper(max);
  }

  vtkFloatingPointType GetLower()
  {
    return m_Lower;
  }

  void SetLower(vtkFloatingPointType lower)
  {
    this->m_Lower = lower;
  }

  vtkFloatingPointType GetUpper()
  {
    return m_Upper;
  }

  void SetUpper(vtkFloatingPointType upper)
  {
    this->m_Upper = upper;
  }

  // itkGetConstObjectMacro(Histogram,mitk::HistogramGenerator::HistogramType);
  // itkSetObjectMacro(Histogram,mitk::HistogramGenerator::HistogramType);
  void mousePressEvent( QMouseEvent* mouseEvent );
  virtual void paintEvent( QPaintEvent* e );
  virtual void DoubleClickOnHandle(int handle) = 0;

  void mouseMoveEvent( QMouseEvent* mouseEvent );
  void mouseReleaseEvent( QMouseEvent* mouseEvent );
  void mouseDoubleClickEvent( QMouseEvent* mouseEvent );
  void PaintHistogram(QPainter &p);

  virtual int GetNearHandle(int x,int y,unsigned int maxSquaredDistance = 32) = 0;
  virtual void AddFunctionPoint(vtkFloatingPointType x,vtkFloatingPointType val) = 0;
  virtual void RemoveFunctionPoint(vtkFloatingPointType x) = 0;
  virtual void MoveFunctionPoint(int index, std::pair<vtkFloatingPointType,vtkFloatingPointType> pos) = 0;
  virtual vtkFloatingPointType GetFunctionX(int index) = 0;
  virtual float GetFunctionY(int index) = 0;
  virtual int GetFunctionSize() = 0;
  int m_GrabbedHandle;

  vtkFloatingPointType m_Lower, m_Upper, m_Min, m_Max;
    
  std::pair<int,int> FunctionToCanvas(std::pair<vtkFloatingPointType,vtkFloatingPointType>);
  std::pair<vtkFloatingPointType,vtkFloatingPointType> CanvasToFunction(std::pair<int,int>);


  mitk::SimpleHistogram *m_Histogram;

  void keyPressEvent ( QKeyEvent * e );

  void SetImmediateUpdate(bool state);

  static std::pair<vtkFloatingPointType,vtkFloatingPointType> ValidateCoord( std::pair<vtkFloatingPointType,vtkFloatingPointType> x )
  {
    if( x.first < -2048 ) x.first = -2048;
    if( x.first >  2048 ) x.first =  2048;
    if( x.second < 0 ) x.second = 0;
    if( x.second > 1 ) x.second = 1;
    return x;
  }

  void SetX(float x)
  {
    if (m_GrabbedHandle != -1)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair(x,GetFunctionY(m_GrabbedHandle))));
      update();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }

  void SetY(float y)
  {
    if (m_GrabbedHandle != -1)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair(GetFunctionX(m_GrabbedHandle),y)));
      update();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }

  void SetQLineEdits(QLineEdit* xEdit, QLineEdit* yEdit)
  {
    m_XEdit = xEdit;
    m_YEdit = yEdit;
    m_LineEditAvailable = true;
  }

protected:
  bool m_ImmediateUpdate;
  float m_Range;

  bool m_LineEditAvailable;
  QLineEdit* m_XEdit;
  QLineEdit* m_YEdit;
};
#endif

