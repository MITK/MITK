#include <assert.h>
#include <math.h>
#include "ipFuncP.h"

/*!
The algorithm implements the shape-based interpolation technique.
Further detailed information can be found in: 

G.T. Herman, J. Zheng, C.A. Bucholtz: "Shape-based interpolation"
IEEE Computer Graphics & Applications, pp. 69-79,May 1992
*/

#define MAX 2048

extern float truncf (float x);
static ipPicDescriptor* setup (ipPicDescriptor* pic_old, const ipFloat4_t threshold);
static ipInt2_t distance (const ipInt2_t* const old_distance, const ipInt2_t* mask_dist, const ipInt2_t* mask_offset, const ipUInt4_t mask_elements);

#define INTERPOLATE(TYPE, RESULT, PIC0, PIC1)							\
{												\
    ipInt4_t x, y;										\
    ipUInt4_t i; 	                       /* loop counters */				\
    ipFloat4_t weight[] = {1.0f-ratio, ratio}; /* weights of the interpolants */ 		\
    ipInt2_t mask_width = 1;                   /* mask width from center to border */		\
    ipInt2_t mask_x[] = {0, -1, +1,  0, -1};   /* relativ position in x-axis of mask */		\
    ipInt2_t mask_y[] = {0,  0, -1, -1, -1};   /* relativ position in y-axis of mask */		\
    /* ipInt2_t mask_dist[] = {0, 3, 4, 3, 4}; */ 						\
    ipInt2_t mask_dist[] = {0, 10, 14, 10, 14};/* distance to central element */		\
    ipInt2_t mask_offset [5];                  /* relative start position in given image */	\
    ipUInt4_t mask_elements = 5;               /* elements in distance mask */			\
    ipInt2_t first_x, first_y;                 /* first pixel for distance calculation		\
						  in enlarged image version */			\
    ipInt2_t last_x, last_y;                   /* last pixel for distance calculation		\
						  in enlarged image version */			\
    ipInt2_t *pixel[2];                        /* pointer to the current pixels */		\
    TYPE* pixel_out; 										\
												\
    /* apply the mask in both directions */							\
    for (i=0; i< mask_elements; i++) {								\
        mask_offset [i] = mask_x[i] + mask_y[i]*(PIC0)->n[0];					\
    }												\
    first_x = mask_width;									\
    first_y = mask_width;									\
    last_x = (PIC0)->n[0] - mask_width-1;							\
    last_y = (PIC0)->n[1] - mask_width-1;							\
    /* top-left to bottom-right, borders are neglected */					\
    for (y = first_y; y <= last_y; y++) {							\
        pixel [0] = (ipInt2_t *) (PIC0)->data + (first_x + y * (PIC0)->n [0]); 			\
        pixel [1] = (ipInt2_t *) (PIC1)->data + (first_x + y * (PIC1)->n [0]); 			\
        for (x = first_x; x <= last_x; x++) {							\
	    *(pixel [0])++ = distance (pixel [0], mask_dist, mask_offset, mask_elements);	\
	    *(pixel [1])++ = distance (pixel [1], mask_dist, mask_offset, mask_elements);	\
        }											\
    }												\
    /* bottom-right to top-left, borders are neglected */					\
    for (i=0; i< mask_elements; i++) {								\
        mask_offset [i] = -mask_offset [i];							\
    }												\
    pixel_out = (TYPE *) result->data + _ipPicElements(result) - 1;				\
    for (y = last_y; y >= first_y; y--) {							\
	    pixel [0] = (ipInt2_t *) (PIC0)->data + (last_x + y * (PIC0)->n [0]);		\
	    pixel [1] = (ipInt2_t *) (PIC1)->data + (last_x + y * (PIC1)->n [0]);		\
        for (x = last_x; x >= first_x; x--) {							\
	    *(pixel [0]) = distance (pixel [0], mask_dist, mask_offset, mask_elements);		\
	    *(pixel [1]) = distance (pixel [1], mask_dist, mask_offset, mask_elements);		\
            *pixel_out-- = (weight [0] * *(pixel [0]) + weight [1] * *(pixel[1]) > 0 ? 1 : 0);	\
	    pixel[0]--;										\
	    pixel[1]--;										\
        }											\
    }												\
}

/*!
\brief Computes an intermediate image by linear shape-based interpolation.
@param pic1,pic2 the images which enclose the intermediate image
@param threshold the threshold which extracts the shape in the images 
@param ratio the ratio the ratio specifies the weights for interpolation, i.e. the intermediate
image equals to pic1 or pic2 if the ratio is zero and one, respectively 
@returns the intermediate image
*/
ipPicDescriptor*
ipFuncShapeInterpolation (ipPicDescriptor* pic1, ipPicDescriptor* pic2, const ipFloat4_t threshold, const ipFloat4_t ratio, ipPicDescriptor* result)
{
    ipPicDescriptor *pic[2];                   /* pointer to image data */

    /* prepare the images */
    result = ipPicCopyHeader(pic1, NULL);
    if (result == NULL) {
	return NULL;
    }
    result->data = malloc (_ipPicSize (result)); 
    if (result->data == NULL) {
	_ipFuncSetErrno (mitkIpFuncMALLOC_ERROR);
	ipPicFree (result);	
	return NULL;
    }
    pic[0] = setup (pic1, threshold); 
    pic[1] = setup (pic2, threshold); 

    mitkIpPicFORALL_2(INTERPOLATE, result, pic[0], pic[1]);

    ipPicFree(pic [0]);
    ipPicFree(pic [1]);
    return result;
}

#define COPY(TYPE, SRC, DST, THRESH)					\
{									\
	TYPE* src;							\
	ipInt2_t* dst;							\
	src = (TYPE *) (SRC)->data;					\
	dst = (ipInt2_t *) (DST)->data + (1 + (DST)->n[0]);		\
	for (y = 0; y < (SRC)->n[1]; y++) {				\
		for (x = 0; x < (SRC)->n[0]; x++) {			\
			*dst++ = (*src++ > (THRESH) ? MAX : -MAX);	\
		}							\
		dst += 2;						\
	}								\
}

static ipPicDescriptor*
setup (ipPicDescriptor* pic_old, const ipFloat4_t threshold)
{
	ipPicDescriptor* pic;
	ipInt2_t* dst;
	ipUInt4_t x, y;

	/* Allocate new image for distance transform */

	pic = ipPicCopyHeader (pic_old, NULL);
	pic->type = ipPicInt;
	pic->bpe = 16;
	pic->n[0] += 2;
	pic->n[1] += 2;
	pic->data = malloc (_ipPicSize (pic));

	/* Set the frame to -1 */

	dst = (ipInt2_t *) pic->data;
	for (x = 0; x < pic->n[0]; x++) {
		*dst++ = -MAX;
	}	
	dst = (ipInt2_t *) pic->data + _ipPicElements (pic) - pic->n[0];
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

	mitkIpPicFORALL_2(COPY, pic_old, pic, threshold);
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
				if ((dst[0] < dst[-1]) || (dst[0] < dst[-pic->n[0]])) {
					*dst = -5;
				} else if ((dst[0] > dst[-1]) || (dst[0] > dst[-pic->n[0]])) {
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
