/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#include <assert.h>
#include <math.h>
#include <ipFunc/mitkIpFunc.h>
#include "ipSegmentationP.h"

/*!
The algorithm implements the shape-based interpolation technique.
Further detailed information can be found in: 

G.T. Herman, J. Zheng, C.A. Bucholtz: "Shape-based interpolation"
IEEE Computer Graphics & Applications, pp. 69-79,May 1992
*/

#define MAX 2048

extern float truncf (float x);
static mitkIpPicDescriptor* setup (mitkIpPicDescriptor* pic_old);
static ipInt2_t distance (const ipInt2_t* const old_distance, const ipInt2_t* mask_dist, const ipInt2_t* mask_offset, const ipUInt4_t mask_elements);

// DON'T ever touch this code again - rather rewrite it! Then use ITK or some other library!

// Works ONLY with 2D images.
mitkIpPicDescriptor*
ipMITKSegmentationPadBy1Pixel (mitkIpPicDescriptor* pic_in)
{
  //prepare the images
  mitkIpPicDescriptor* pic_out;
  ipMITKSegmentationTYPE* out_pixel_pointer;
  ipMITKSegmentationTYPE* in_pixel_pointer;
  unsigned int x, y;  

  pic_out = mitkIpPicCopyHeader(pic_in, NULL);
  if (pic_out == NULL) 
  {
    ipMITKSegmentationError (ipMITKSegmentationPIC_NULL);
  }

 if ((pic_out->type != ipMITKSegmentationTYPE_ID) || (pic_out->bpe != ipMITKSegmentationBPE)) 
  {
    ipMITKSegmentationError (ipMITKSegmentationWRONG_TYPE);
  }

  // pad by 1 pixel
  pic_out->n[0] += 4;
  pic_out->n[1] += 4;
   
  // allocate image data
  pic_out->data = malloc ( pic_out->n[0] * pic_out->n[1] * ((pic_out->bpe)/8) );
  // copy pixel data
  out_pixel_pointer = (ipMITKSegmentationTYPE*) (pic_out->data);
  in_pixel_pointer = (ipMITKSegmentationTYPE*) (pic_in->data);
  
  for (y = 0; y < pic_out->n[1]; ++y)
    for (x = 0; x < pic_out->n[0]; ++x)
    {
      if ( x < 2  || y < 2 || (x > pic_out->n[0]-3) || (y > pic_out->n[1]-3) ) // set border pixels to 0
      {
        *out_pixel_pointer = 0;
      }
      else
      {
        *out_pixel_pointer = *in_pixel_pointer;
        ++in_pixel_pointer;
      }
      ++out_pixel_pointer;
    }
    
  return pic_out;
}

// Works ONLY with 2D images.
mitkIpPicDescriptor*
ipMITKSegmentationShrinkBy1Pixel (mitkIpPicDescriptor* pic_in )
{
  mitkIpPicDescriptor* pic_out;
  ipMITKSegmentationTYPE* out_pixel_pointer;
  ipMITKSegmentationTYPE* in_pixel_pointer;
  unsigned int x, y;  

  //prepare the images
  pic_out = mitkIpPicCopyHeader(pic_in, NULL);
  if (pic_out == NULL) {
      ipMITKSegmentationError (ipMITKSegmentationPIC_NULL);
  }

  if ((pic_out->type != ipMITKSegmentationTYPE_ID) || (pic_out->bpe != ipMITKSegmentationBPE)) 
  {
    ipMITKSegmentationError (ipMITKSegmentationWRONG_TYPE);
  }

  // pad by 1 pixel
  pic_out->n[0] -= 4;
  pic_out->n[1] -= 4;
    
  // allocate image data
  pic_out->data = malloc ( pic_out->n[0] * pic_out->n[1] * (pic_out->bpe/8) );

  // copy pixel data
  out_pixel_pointer = pic_out->data;
  in_pixel_pointer = pic_in->data;
  
  for (y = 0; y < pic_in->n[1]; ++y)
    for (x = 0; x < pic_in->n[0]; ++x)
    {
      if ( x < 2  || y < 2 || (x > pic_in->n[0]-3) || (y > pic_in->n[1]-3) ) // ignore border pixels
      {
      }
      else
      {
        *out_pixel_pointer = *in_pixel_pointer;
        ++out_pixel_pointer;
      }
      ++in_pixel_pointer;
    }
    
  return pic_out;
}

mitkIpPicDescriptor*
ipMITKSegmentationInterpolate (mitkIpPicDescriptor* pPic1, mitkIpPicDescriptor* pPic2, const ipFloat4_t ratio)
{
    mitkIpPicDescriptor *pic_out, *pic[2];         /* pointer to image data */
    ipUInt4_t frame [_mitkIpPicNDIM];              /* pointer for definition of frame-size */
    ipInt4_t x, y;
    ipUInt4_t i;                               /* loop counters */
    ipFloat4_t weight[] = {1.0f-ratio, ratio}; /* weights of the interpolants */ 
    ipInt2_t mask_width = 1;                   /* mask width from center to border       */
    ipInt2_t mask_x[] = {0, -1, +1,  0, -1};   /* relativ position in x-axis of mask */
    ipInt2_t mask_y[] = {0,  0, -1, -1, -1};   /* relativ position in y-axis of mask */
    ipInt2_t mask_dist[] = {0, 10, 14, 10, 14};/* distance to central element */
    ipInt2_t mask_offset [5];                  /* relative start position in given image */
    ipUInt4_t mask_elements = 5;               /* elements in distance mask */
    ipInt2_t first_x, first_y;                 /* first pixel for distance calculation in enlarged image version */
    ipInt2_t last_x, last_y;                   /* last pixel for distance calculation in enlarged image version */
    ipInt2_t *pixel[2];                        /* pointer to the current pixels */
    ipMITKSegmentationTYPE* pixel_out;
	mitkIpPicDescriptor* returnImage;

    mitkIpPicDescriptor* pic1 = ipMITKSegmentationPadBy1Pixel( pPic1 );
    mitkIpPicDescriptor* pic2 = ipMITKSegmentationPadBy1Pixel( pPic2 );

    /* prepare the images */
    pic_out = mitkIpPicCopyHeader(pic1, NULL);
    if (pic_out == NULL) {
        ipMITKSegmentationError (ipMITKSegmentationPIC_NULL);
    }
    if ((pic_out->type != ipMITKSegmentationTYPE_ID) || (pic_out->bpe != ipMITKSegmentationBPE)) {
  ipMITKSegmentationError (ipMITKSegmentationWRONG_TYPE);
    }
    pic_out->data = malloc (_mitkIpPicSize (pic_out)); 
    for (i = 0; i < pic1->dim; i++) {
        frame [i] = 1;
    }  
    pic[0] = setup (pic1); 
    pic[1] = setup (pic2); 

    /* apply the mask in both directions */
    for (i=0; i< mask_elements; i++) {
        mask_offset [i] = mask_x[i] + mask_y[i]*pic[0]->n[0];
    }
    first_x = mask_width;
    first_y = mask_width;
    last_x = pic[0]->n[0] - mask_width-1;
    last_y = pic[0]->n[1] - mask_width-1;
    /* top-left to bottom-right, borders are neglected */
    for (y = first_y; y <= last_y; y++) {
        pixel [0] = (ipInt2_t *) pic [0]->data + (first_x + y * pic [0]->n [0]); 
        pixel [1] = (ipInt2_t *) pic [1]->data + (first_x + y * pic [1]->n [0]); 
        for (x = first_x; x <= last_x; x++) {
      *(pixel [0])++ = distance (pixel [0], mask_dist, mask_offset, mask_elements);
      *(pixel [1])++ = distance (pixel [1], mask_dist, mask_offset, mask_elements);
        }
    }
    /* bottom-right to top-left, borders are neglected */
    for (i=0; i< mask_elements; i++) {
        mask_offset [i] = -mask_offset [i];
    }
    pixel_out = (ipMITKSegmentationTYPE *) pic_out->data + _mitkIpPicElements(pic_out) - 1;
    for (y = last_y; y >= first_y; y--) {
      pixel [0] = (ipInt2_t *) pic [0]->data + (last_x + y * pic [0]->n [0]);
      pixel [1] = (ipInt2_t *) pic [1]->data + (last_x + y * pic [1]->n [0]);
        for (x = last_x; x >= first_x; x--) {
      *(pixel [0]) = distance (pixel [0], mask_dist, mask_offset, mask_elements);
      *(pixel [1]) = distance (pixel [1], mask_dist, mask_offset, mask_elements);
            *pixel_out-- = (weight [0] * *(pixel [0]) + weight [1] * *(pixel[1]) > 0 ? 1 : 0);
      pixel[0]--;
      pixel[1]--;
        }
    }
    mitkIpPicFree(pic [0]);
    mitkIpPicFree(pic [1]);
    
    mitkIpPicFree(pic1);
    mitkIpPicFree(pic2);

    returnImage = ipMITKSegmentationShrinkBy1Pixel( pic_out );

    mitkIpPicFree(pic_out);

    return returnImage;
}

static mitkIpPicDescriptor*
setup (mitkIpPicDescriptor* pic_old)
{
  mitkIpPicDescriptor* pic;
  ipMITKSegmentationTYPE* src;
  ipInt2_t* dst;
  ipUInt4_t x, y;

  /* Allocate new image for distance transform */

  pic = mitkIpPicCopyHeader (pic_old, NULL);
  pic->type = mitkIpPicInt;
  pic->bpe = 16;
  pic->n[0] += 2;
  pic->n[1] += 2;
  pic->data = malloc (_mitkIpPicSize (pic));

  /* Set the frame to -1 */

  dst = (ipInt2_t *) pic->data;
  for (x = 0; x < pic->n[0]; x++) {
    *dst++ = -MAX;
  }  
  dst = (ipInt2_t *) pic->data + _mitkIpPicElements (pic) - pic->n[0];
  for (x = 0; x < pic->n[0]; x++) {
    *dst++ = -MAX;
  }  
  dst = (ipInt2_t *) pic->data;
  for (y = 0; y < pic->n[1]; y++) {
    *dst = -MAX;
    dst += pic->n[0];
  }
  dst = (ipInt2_t *) pic->data + (pic->n[0] - 1);
  for (y = 0; y < pic->n[1]; y++) {
    *dst = -MAX;
    dst += pic->n[0];
  }

  /* Set the image data to initial values */

  src = (ipMITKSegmentationTYPE *) pic_old->data;
  dst = (ipInt2_t *) pic->data + (1 + pic->n[0]);
  for (y = 0; y < pic_old->n[1]; y++) {
    for (x = 0; x < pic_old->n[0]; x++) {
      *dst++ = (*src++ > 0 ? MAX : -MAX);
    }
    dst += 2;
  }
  dst = (ipInt2_t *) pic->data + (1 + pic->n[0]);
  for (y = 0; y < pic_old->n[1]; y++) {
    for (x = 0; x < pic_old->n[0]; x++) {
      if ((dst[0] < dst[1]) || (dst[0] < dst[pic->n[0]])) {
        *dst = -5;
      } else if ((dst[0] > dst[1]) || (dst[0] > dst[pic->n[0]])) {
        *dst = 5;
      }  
      dst++;
    }
    dst += 2;
  }
  dst -= 2;
  for (y = 0; y < pic_old->n[1]; y++) {
    for (x = 0; x < pic_old->n[0]; x++) {
      dst--;
      if (abs (dst[0]) > 5) {
        if ((dst[0] < dst[-1]) || (dst[0] < dst[-(int)(pic->n[0])])) {
          *dst = -5;
        } else if ((dst[0] > dst[-1]) || (dst[0] > dst[-(int)(pic->n[0])])) {
          *dst = 5;
        }  
      }  
    }
  }
  return pic;
}

static ipInt2_t distance (const ipInt2_t* const old_distance, const ipInt2_t* mask_dist, const ipInt2_t* mask_offset, const ipUInt4_t mask_elements)
{
    ipInt2_t cur_distance, new_distance;
    ipUInt4_t i;

    cur_distance = old_distance [0];
    if (abs (cur_distance) != 5) {
  if (cur_distance > 0) {
    for (i = 0; i < mask_elements; i++) {
      new_distance = *mask_dist + old_distance [*mask_offset];
      if (new_distance < cur_distance) {
        cur_distance = new_distance;
      }
      mask_dist++;
      mask_offset++;
    }
  } else if (cur_distance < 0) {
    for (i = 0; i < mask_elements; i++) {
      new_distance = old_distance [*mask_offset] - *mask_dist;
      if (new_distance > cur_distance) {
        cur_distance = new_distance;
      }
      mask_dist++;
      mask_offset++;
    }
  }
    }
    return cur_distance;
}
