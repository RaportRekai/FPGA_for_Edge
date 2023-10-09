// Including Required Libraries
#include<math.h>
#include<iostream>
#include<ap_int.h>
#include "yolov2_parameters.h"
#include "image.h"

using namespace std;

// Defining Custom Data type
typedef ap_int<10> accuracy;
typedef ap_int<9> bbxy;
typedef ap_int<3> int3;
typedef ap_int<1> int1;

// Defining Global Variables
int yolo_max = 120, yolo_filters = 30, max_size = 12288;
int score, bbx1, bby1, bbx2, bby2, oresult;
int leaves_count, result, cflag=0;
float yolo_process[] = {0}, yolo_out[] = {0};

void zero_padding(int pad, int height, int width, int channel) {
    /*
    Function Name: Zero Padding
    Input        : pad     - padding size
                   height  - image height
                   width   - image width
                   channel - image channel
    Description  : This function add zeros to the end of the input sequence so that the total number of samples is equal to the next higher power of two.
    */
    for(int i=0; i<(height + 2*pad); i++) {
		#pragma hls pipeline II=2 style=flp
        if(i>=pad && i< height + pad){
            for(int j=0; j<((width + 2 * pad)*channel); j++) {
                if (j>=pad*channel && j < (width +pad)*channel) {
                    imageData[i*(width+2*pad)*channel+j] = imageData[(i-pad)*width*channel+j-pad*channel];
                }
            }
        }
    }
}

void convolution(int k_width, int k_height, int filters, int arr_width, int arr_height, int arr_channels, int stride, float layer[12188]){
    /*
    Function Name: convolution
    Input        : k_width      - kernel width
                   k_height     - kernel height
                   filters      - no of filters
                   arr_width    - image width
                   arr_height   - image height
                   arr_channels - no of channels in image
                   stride       - stride size
    Description  : This function will complete the convolution for the
                   given input parameters.
    */
    int e = 0;
    // Perform Convolution
	for(int i=0; i<((arr_height - (k_width-1)+1)/stride); i++) {
#pragma hls pipeline II=2 style=flp
        // Variable Initialization
        int s_cnt = 0, k_count = 0, k_ind_cnt = 0, init = 0, l = 0;
        int row_cnt = 0, n_arr = 0, kind = 0, next_arr = 0, j = 0;
        int nex_arr[1000]={0};
        while(row_cnt <= arr_height) {
            if(s_cnt == (arr_width - (k_width - 1) + 1)/stride) {
                row_cnt = row_cnt + 1;
                if(row_cnt%3 == 0) kind = 0;
                if(row_cnt%3 == 1) kind = 3;
                if(row_cnt%3 == 2) kind = 6;
                if(row_cnt == 3) break;
                s_cnt = 0;
            }
            next_arr = next_arr + layer[(k_ind_cnt+kind)*filters*arr_channels+ k_count+j*filters]* imageData[(i*stride+ row_cnt)*arr_width*arr_channels +(stride*s_cnt*arr_channels)+j+k_ind_cnt*arr_channels];
            j = j + 1;
            if(j==arr_channels && k_ind_cnt<3) {
                j = 0; k_ind_cnt = k_ind_cnt + 1;
            }
            if(k_ind_cnt == 3){
                nex_arr[l] = next_arr;
                l = l + 1;
                next_arr = 0; k_ind_cnt = 0;
                k_count = k_count + 1;
            }
            if(k_count == filters) {
                s_cnt = s_cnt + 1; j = 0; k_count = 0;
            }
        }
        for(int m=0; m<s_cnt*filters; m++) {
            for(int t=0; t<k_height; t++) {
                n_arr = n_arr + nex_arr[m+s_cnt*filters*t];
            }
            yolo_process[e] = n_arr;
            e = e + 1; n_arr = 0;
        }
    }
}

void batch_normalization(int arr_channels, int arr_height, int arr_width, float layer[12188]) {
	/*
	Function Name: Batch Normalization
	Input        : arr_channels - no of channels
				   height       - height value
				   width        - width value
	Description  : All the values with different scaling is normalized
				   to 0 to 1.
	*/
    int  i = 0;
    for(int k=0; k<(arr_height*arr_width); k++) {
		#pragma hls pipeline II=2 style=flp
    	if(i == 0) {
            for(int j=0; j<arr_channels; j++) {
                yolo_process[arr_channels*k+j] = yolo_process[arr_channels*k+j] - yolo_process[2*arr_channels+j];
            }
            i = i+1;
        }
        if(i == 1) {
            for(int j=0; j<arr_channels; j++) {
            	if(yolo_process[3*arr_channels+j] < 0) yolo_process[3*arr_channels+j] = yolo_process[3*arr_channels+j] * -1;
            	yolo_process[arr_channels*k+j] = yolo_process[arr_channels*k+j]/sqrt(yolo_process[3*arr_channels+j]+1);
            }
            i = i+1;
        }
        if(i == 2) {
            for(int j=0; j<arr_channels; j++) {
            	yolo_process[arr_channels*k+j] = yolo_process[arr_channels*k+j]*yolo_process[j];
            }
            i = i + 1;
        }
        if(i == 3) {
            for(int j=0; j<arr_channels; j++) {
            	yolo_process[arr_channels*k+j] = yolo_process[arr_channels*k+j]+yolo_process[1*arr_channels+j];
            }
            i = 0;
        }
    }
}

void leaky_relu(float alpha) {
	/*
	Function Name : leaky_relu
	Input         : alpha - computation parameter
	Description   : activation function has a small slope for negative values instead of a flat slope.
	*/
    int length = (sizeof(yolo_process)/sizeof(float));
    for(int i=0; i<length; i++) {
        if(yolo_process[i]<0){
        	yolo_process[i] = alpha*yolo_process[i];
        }
    }
}

void max_pooling(int k_width,int k_height,int filters,int arr_width,int arr_height, int arr_channels, int stride) {
	/*
	Function Name: Max Pooling
	Input        : k_width      - Kernel width
				   k_height     - Kernel Height
				   filters      - No of Filters
				   arr_width    - array width
				   arr_height   - array height
				   arr_channels - array channels
				   stride       - strides
	Description  :
	*/
	int n_arr = 0, max = -266, h = 0, l = 0;
    for(int i=0; i<((arr_height-k_width)/stride+1); i++) {
		#pragma hls pipeline II=2 style=flp
    	float nex_arr[] = {0};
        int k_ind_cnt = 0, k_count = 0, kind = 0, row_cnt = 0;
        int s_cnt = 0, next_arr = 0, n_arr = 0;
        while(row_cnt <= arr_height) {
            if(s_cnt == ((arr_height-k_width)/stride+1)) {
                row_cnt = row_cnt + 1;
                if(row_cnt == k_height) break;
                s_cnt = 0;
            }
            if(max < yolo_process[(i*stride+ row_cnt)*arr_channels*arr_width + (stride*s_cnt*arr_channels) + k_ind_cnt*arr_channels + k_count]) {
                max = yolo_process[(i*stride+ row_cnt)*arr_channels*arr_width + (stride*s_cnt*arr_channels) + k_ind_cnt*arr_channels + k_count];
            }
            if(k_ind_cnt<k_height) k_ind_cnt = k_ind_cnt + 1;
            if(k_ind_cnt == k_height) {
                nex_arr[l] = max;
                l = l + 1;
                k_ind_cnt = 0; max = -266;
            }
            if(k_count == filters) {
                s_cnt = s_cnt + 1; k_count = 0;
            }
        }
        for(int m=0; m<(s_cnt*filters); m++) {
            for(int t=0; t<k_height; t++) {
                if(n_arr < nex_arr[m+s_cnt*filters*t]) {
                    n_arr = nex_arr[m+s_cnt*filters*t];
                }
                yolo_process[h] = n_arr;
                n_arr = -266; h = h + 1;
            }
        }
    }
}

 void yolov2algorithm(){
	/*
		Function Name: yolov2algorithm
	    return       : output flag
	    Description  : Function to perform yolo operations
	                   it calls appropriate functions to do
	                   convolution for the given input images.
		*/
    zero_padding(1, 64, 64, 3);
	convolution(3, 3, 16, 64, 64, 3, 1, imageData);
	batch_normalization(16, 64, 64, yolo_weights);
	leaky_relu(0.1);
	max_pooling(2, 2, 16, 32, 32, 3, 2);
	int n = 0;
	for(int i=0; i<max_size; i++) {
		if((yolo_process[i] >= -yolo_max/(yolo_filters*100)) && (yolo_process[i] <= yolo_max/(yolo_filters*100)) && yolo_process[i] != 0 && i<yolo_max) {
			yolo_process[i] = yoloout_weights[n] - abs(yolo_process[i]);
			yolo_out[n] = yolo_process[i];
			n++;
		}
	}
}

void yolo_eval(float *box_xy, float *box_wh, float *box_confidence, float  *box_class_probs, float *boxes,float *box_class_scores, float threshold, int &count) {
    /*
    Function Name: yolo_eval
    Input        :
                   -- Yolo Outputs --
                   box_xy           - bounding box x and y position
                   box_wh           - bounding box widht and height values
                   box_confidence   - confidence of the bounding boxes
                   box_class_prob   - detected class probability
                   --               --
                   boxes            - maximum predicted box_whes
                   box_class_scores - bounding box class scores
                   threshold        - threshold for scores and bounding boxes
                   count            - counter

    Description  : This function converts yolo outputs (multiple bounding boxes) to best
                   predicted bounding box along with their scores, box co-ordinates and
                   classes.
    */
    // Input Image size
    int raw_image_h = 64, raw_image_w = 64;
    int i=0,index;
    // Convert boxes to be ready for filtering functions
    // convert boxes box_xy and box_wh to corner coordinates
    for(int ind=0; ind<20;ind++) {
        index = 2*ind+1;
        boxes[i] =box_xy[index] - (box_wh[index]/2.0);
        box_class_scores[ind] = box_confidence[ind] * box_class_probs[ind];
        i++;
        index = 2*ind;
        boxes[i] = box_xy[index] - (box_wh[index]/2.0);
        i++;
        index = 2*ind+1;
        boxes[i] = box_xy[index] + (box_wh[index]/2.0);
        i++;
        index = 2*ind;
        boxes[i] = box_xy[index] + (box_wh[index]/2.0);
        i++;
    }
    //threshold value checked
    for(int i=0;i<20;i++) {
        if(box_class_scores[i] <= threshold) {
            for(int j=0;j<4;j++) {
                boxes[i*4+j] = 0;
            }
            box_class_scores[i] = 0;
        }
        else {
            count++;
            for(int j=0;j<4;j++) {
                if(j==0||j==2) boxes[i*4+j] = boxes[i*4+j]*raw_image_h;
                else boxes[i*4+j] = boxes[i*4+j]*raw_image_w;
            }
        }
    }
}

void non_max_suppresion(float *scores,float *boxes,int count,int max_boxes = 10, float iou_threshold = 0.3) {

    /*
    Function Name: non_max_suppresion
    Input        : scores        - scores array
                   boxes         - bounding boxes array
                   count         - count value
                   max_boxes     - maximum no of bounding boxes
                   iou_threshold - threshold for intersection over union
    Description  : This function validates all the input bounding boxes and outputs the
                   more appropriate bounding box for the input parameters.
    */
    int l,max_idx;
    float temp;
    l = count;
    for(int i=0;i<20;i++) {
		max_idx = i;
		for(int j=i+1;j<20;j++) {
			if (scores[max_idx] < scores[j]) {
				max_idx = j;
			}
		}
		temp = scores[max_idx];
		scores[max_idx] = scores[i];
		scores[i] = temp;

		temp = boxes[max_idx*4];
		boxes[max_idx*4] = boxes[i*4];
		boxes[i*4] = temp;

		temp = boxes[max_idx*4+1];
		boxes[max_idx*4+1] = boxes[i*4+1];
		boxes[i*4+1] = temp;

		temp = boxes[max_idx*4+2];
		boxes[max_idx*4+2] = boxes[i*4+2];
		boxes[i*4+2] = temp;

		temp = boxes[max_idx*4+3];
		boxes[max_idx*4+3] = boxes[i*4+3];
		boxes[i*4+3] = temp;
    }

    int mask[20];
    for(int i =0;i<20;i++) mask[i] = 0;
    for(int i=0;i<20;i++) {
        if (mask[i] != 0) continue;
        else {
			for(int j=i+1;j<20;j++) {
				if (mask[j]!=0) continue;
				else {
					float xi1,xi2,yi1,yi2;
					float inter_height,inter_width,inter_area,box1_area,box2_area,iou,union_area;
					for(int k=0;k<4;k++) {
						if(k==0)
						 {
							if(boxes[i*4+k]<boxes[j*4+k]) xi1 = boxes[i*4+k];
							else xi1 = boxes[j*4+k];
						}
						else if(k==1) {
							if(boxes[i*4+k]<boxes[j*4+k]) yi1 = boxes[i*4+k];
							else yi1 = boxes[j*4+k];
						}
						else if(k==2) {
							if(boxes[i*4+k]>boxes[j*4+k]) xi2 = boxes[j*4+k];
							else xi2 = boxes[i*4+k];
						}
						else {
							if(boxes[i*4+k]>boxes[j*4+k]) yi2 = boxes[j*4+k];
							else yi2 = boxes[i*4+k];
						}
					}
					if(yi2 - yi1>  0) inter_width = yi2 - yi1;
					else inter_width = 0;
					if(xi2 - xi1 > 0) inter_height = xi2 - xi1;
					else inter_height = 0;
					inter_area = inter_height*inter_width;
					box1_area = (boxes[i*4+2] - boxes[i*4+0])*(boxes[i*4+3] - boxes[i*4+1]);
					box2_area = (boxes[j*4+2] - boxes[j*4+0])*(boxes[j*4+3] - boxes[j*4+1]);
					union_area = box1_area + box2_area - inter_area;
					iou = inter_area/(1.0*union_area);
					if(iou >= iou_threshold) {
						mask[j]=1;
						boxes[j*4] = 0;
						boxes[j*4+1] = 0;
						boxes[j*4+2] = 0;
						boxes[j*4+3] = 0;
						scores[j] = 0;
					}
				}
			}
        }
    }
}

void regressor() {
	/*
	Function Name: regressor
	return       : output flag
	Description  : This function stores parameters for
	yolov2 regressor and calls appropriate functions
	to complete evaluation and give the final results.
	*/
	int count = 0, conv_dims = 2.0;
	float box_xy[20*2], box_wh[20*2], box_confidence[20], box_class_probs[20], boxes[40*2], box_class_scores[20];
	float anchors_tensor[10] = {0.57273,  0.677385, 1.87446,  2.06253 , 3.33843,  5.47434 , 7.88282,  3.52778 , 9.77052,  9.16828};
	//input your 1D array features down here
	float feats[] = {-4.6243405, 0.84960103, -0.2599549, -0.77497494, -4.164954, 2.3468022, 2.4310565, 2.8735251, -0.5037936, -0.09149927, 1.0385386, -0.5798238, -1.142833, 0.06454898, -0.24900562, -0.3886128, -4.5418477, -0.27149326, 0.55217594, 2.228736, 2.6266255, -0.8682554, -6.0100565, 0.19091323, -1.3343002, 0.14348663, -0.92876816, -0.97621584, -4.5341725, 0.13001765, -5.419123, 0.44518122, -0.773263, -0.9777708, -3.4528039, 1.2587534, -2.213027, 2.429602, -0.38500854, -0.08032792, 2.426098, 0.3019848, 0.29330906, 0.5500988, 0.038952194, -0.67729616, -4.9695363, -1.4718397, -0.2917492, 1.3352497, 2.0338428, -0.45763257, -5.7043858, -0.20904753, -0.14086093, -0.27976346, -0.22631451, -0.59984565, -3.97438, 0.5724709, -1.4119892, 0.7616302, -0.67075264, -0.5854198, -3.582089, 2.5332067, 2.876822, -2.526292, -0.48034027, -0.106113866, 1.3045089, -0.9448797, -1.6333854, 0.5676803, -0.21839744, -0.5562814, -5.6190615, -0.3912425, -1.2463518, 1.760736, 1.1366075, -1.3002201, -4.862821, -0.5019809, -1.4602835, -0.69102657, -0.82349014, -2.1480203, -4.707937, -0.18863772, -5.2811117, 0.3111872, -0.877627, -1.1157461, -3.4915752, 0.5666436, -2.3862824, -3.4880993, -0.40272686, -0.17332782, 2.575323, 0.29113597, -0.27096143, 1.5025591, 0.031412654, -0.17334807, -4.985726, -0.09580569, -0.22290805, 0.16424714, 1.3588431, -0.9521086, -5.1858745, -0.6644707, -0.96648854, -1.201414, -0.077686734, -1.8301326, -4.46064, 0.7828548};
	for(int i=0;i<20;i++) {
		box_xy[2*i] = 1/(1+exp(-1*feats[6*i+0]));
		box_xy[2*i+1]= 1/(1+exp(-1*feats[6*i+1]));
		box_wh[2*i] = exp(feats[6*i+2]);
		box_wh[2*i+1] = exp(feats[6*i+3]);
		box_confidence[i]  = 1/(1+exp(-1*feats[6*i+4]));

		box_class_probs[i] = 1;
	}

    float conv_index[8] = { 0.,  0.,  1., 0., 0., 1., 1., 1.} ;
    for(int i=0;i<4;i++) {
        for(int j=0;j<5;j++)
        {
            box_xy[i*10+j*2] = (box_xy[i*10+j*2] + conv_index[i*2])/conv_dims;
            box_xy[i*10+j*2+1] = (box_xy[i*10+j*2+1] + conv_index[i*2+1])/conv_dims;
        }
    }
    for(int i=0;i<4;i++) {
        for(int j=0;j<5;j++) {
          box_wh[i*10+j*2] = box_wh[i*10+j*2] * anchors_tensor[j*2]/ conv_dims;
          box_wh[i*10+j*2+1] = box_wh[i*10+j*2+1] * anchors_tensor[j*2+1]/ conv_dims;
        }
    }
    yolo_eval(box_xy,box_wh,box_confidence,box_class_probs,boxes,box_class_scores,0.7,count);
    non_max_suppresion(box_class_scores,boxes,count,10,0.8);
    leaves_count = 0, oresult = 0;
    score = 0; bbx1 = 0; bby1 = 0; bbx2 = 0; bby2 = 0; cflag = 0; result = 0;
	for(int i=0;i<20;i++) {
		if(box_class_scores[i]!=0) {
			leaves_count++; oresult = 1;
			score = int(box_class_scores[i]*1000);
			bby1 = boxes[4*i];
			bbx1 = boxes[4*i+1];
			bbx2 = boxes[4*i+2];
			bby2 = boxes[4*i+3];
		}
	}
}

void yolotop(int1 &compute, int3 &leaves, accuracy &scores_out, bbxy &obbx1, bbxy &obby1, bbxy &obbx2, bbxy &obby2) {
	/*
	Function Name: yolotop
	Input        : start - start switch
	Output       :
	compute - start tx computation flag
	leaves  - no of leaves detected flag
	scores_out - accuracy of detected leaves
	out_bb--   - bounding box co-ordinates
	Description : This function will create the module I/Os
	using the HLS Pragmas.
	*/
	// Defining PRAGMAs
	#pragma HLS INTERFACE ap_ctrl_none port=return
	#pragma HLS INTERFACE ap_none port=leaves
	#pragma HLS INTERFACE ap_none port=compute
	#pragma HLS INTERFACE ap_none port=scores_out
	#pragma HLS INTERFACE ap_none port=obbx1
	#pragma HLS INTERFACE ap_none port=obby1
	#pragma HLS INTERFACE ap_none port=obbx2
	#pragma HLS INTERFACE ap_none port=obby2
	yolov2algorithm();
	regressor();
	compute = oresult;
	leaves = leaves_count;
	scores_out = score;
	obbx1 = bbx1;
	obby1 = bby1;
	obbx2 = bbx2;
	obby2 = bby2;
}

// Note: Due to some technical limitations in this software (Vivado HLS 2018.3), this code cannot be simulated :(
// So, all the parts of the code are simulated individually.
