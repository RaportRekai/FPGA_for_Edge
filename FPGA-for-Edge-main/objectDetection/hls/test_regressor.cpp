//#include <iostream>
#include<cmath>
#include<iostream>
#include<ap_int.h>

using namespace std;

// Defining Custom Data type Variables
typedef ap_uint<10> accuracy;
typedef ap_uint<9> bbxy;
typedef ap_uint<3> int3;
typedef ap_uint<1> int1;

int yolo_eval(float *box_xy, float *box_wh, float *box_confidence, float  *box_class_probs, float *boxes,float *box_class_scores, float threshold, int &count) {
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
#pragma HLS loop_flatten off
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
#pragma HLS loop_flatten off
        if(box_class_scores[i] <= threshold)  {
            for(int j=0;j<4;j++) {
#pragma HLS loop_flatten off
                boxes[i*4+j] = 0;
            }
            box_class_scores[i] = 0;
        }
        else {     
            count++;
            for(int j=0;j<4;j++) {
#pragma HLS loop_flatten off
                if(j==0||j==2) boxes[i*4+j] = boxes[i*4+j]*raw_image_h;
                else boxes[i*4+j] = boxes[i*4+j]*raw_image_w;
            }
        }
    }
//    flag = 1;
    return 1;
}

int non_max_suppresion(float *scores,float *boxes,int count,int max_boxes = 10, float iou_threshold = 0.3) {

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
#pragma HLS loop_flatten off
    max_idx = i;
    for(int j=i+1;j<20;j++)
    {
#pragma HLS loop_flatten off
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
#pragma HLS loop_flatten off
        if (mask[i] != 0)
            continue;
        else{
        for(int j=i+1;j<20;j++)
         {
#pragma HLS loop_flatten off
            if (mask[j]!=0) continue;
            else
            {
            float xi1,xi2,yi1,yi2;
            float inter_height,inter_width,inter_area,box1_area,box2_area,iou,union_area;
            for(int k=0;k<4;k++)
            {
#pragma HLS loop_flatten off
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


            if(iou >= iou_threshold)
            {
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
    int m=0;
    return 2;
}

void regressor(int1 start, int1 &compute, int3 &leaves, accuracy &scores_out, bbxy &out_bbx1, bbxy &out_bby1, bbxy &out_bbx2, bbxy &out_bby2) {
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE ap_none port=compute bundle=predictions
#pragma HLS INTERFACE ap_none port=leaves bundle=predictions
#pragma HLS INTERFACE ap_none port=scores_out bundle=predictions
#pragma HLS INTERFACE ap_none port=out_bbx1 bundle=predictions
#pragma HLS INTERFACE ap_none port=out_bby1 bundle=predictions
#pragma HLS INTERFACE ap_none port=out_bbx2 bundle=predictions
#pragma HLS INTERFACE ap_none port=out_bby2 bundle=predictions
	// Main function to test
	if(start == 1) {
	bbxy bbx1, bby1, bbx2, bby2;
	accuracy score;
	int3 leaves, flag;
	int1 result;
    float box_xy[20*2], box_wh[20*2], box_confidence[20],box_class_probs[20];
    int count = 0;
    float boxes[40*2],box_class_scores[20];
    float conv_dims = 2.0;
    float anchors_tensor[10] = {0.57273,  0.677385, 1.87446,  2.06253 , 3.33843,  5.47434 , 7.88282,  3.52778 , 9.77052,  9.16828};
    //input your 1D array features down here
    float feats[] = {-4.6243405, 0.84960103, -0.2599549, -0.77497494, -4.164954, 2.3468022, 2.4310565, 2.8735251, -0.5037936, -0.09149927, 1.0385386, -0.5798238, -1.142833, 0.06454898, -0.24900562, -0.3886128, -4.5418477, -0.27149326, 0.55217594, 2.228736, 2.6266255, -0.8682554, -6.0100565, 0.19091323, -1.3343002, 0.14348663, -0.92876816, -0.97621584, -4.5341725, 0.13001765, -5.419123, 0.44518122, -0.773263, -0.9777708, -3.4528039, 1.2587534, -2.213027, 2.429602, -0.38500854, -0.08032792, 2.426098, 0.3019848, 0.29330906, 0.5500988, 0.038952194, -0.67729616, -4.9695363, -1.4718397, -0.2917492, 1.3352497, 2.0338428, -0.45763257, -5.7043858, -0.20904753, -0.14086093, -0.27976346, -0.22631451, -0.59984565, -3.97438, 0.5724709, -1.4119892, 0.7616302, -0.67075264, -0.5854198, -3.582089, 2.5332067, 2.876822, -2.526292, -0.48034027, -0.106113866, 1.3045089, -0.9448797, -1.6333854, 0.5676803, -0.21839744, -0.5562814, -5.6190615, -0.3912425, -1.2463518, 1.760736, 1.1366075, -1.3002201, -4.862821, -0.5019809, -1.4602835, -0.69102657, -0.82349014, -2.1480203, -4.707937, -0.18863772, -5.2811117, 0.3111872, -0.877627, -1.1157461, -3.4915752, 0.5666436, -2.3862824, -3.4880993, -0.40272686, -0.17332782, 2.575323, 0.29113597, -0.27096143, 1.5025591, 0.031412654, -0.17334807, -4.985726, -0.09580569, -0.22290805, 0.16424714, 1.3588431, -0.9521086, -5.1858745, -0.6644707, -0.96648854, -1.201414, -0.077686734, -1.8301326, -4.46064, 0.7828548};

    for(int i=0;i<20;i++) {
#pragma HLS loop_flatten off
    	box_xy[2*i] = 1/(1+exp(-1*feats[6*i+0]));
        box_xy[2*i+1]= 1/(1+exp(-1*feats[6*i+1]));
        box_wh[2*i] = exp(feats[6*i+2]);
        box_wh[2*i+1] = exp(feats[6*i+3]);
        box_confidence[i]  = 1/(1+exp(-1*feats[6*i+4]));

        box_class_probs[i] = 1;
    }

    float conv_index[8] = { 0.,  0.,  1., 0., 0., 1., 1., 1.} ;
    for(int i=0;i<4;i++)
     {
#pragma HLS loop_flatten off
        for(int j=0;j<5;j++)
        {
            box_xy[i*10+j*2] = (box_xy[i*10+j*2] + conv_index[i*2])/conv_dims;
            box_xy[i*10+j*2+1] = (box_xy[i*10+j*2+1] + conv_index[i*2+1])/conv_dims;
        }
    }
    for(int i=0;i<4;i++)
    {
#pragma HLS loop_flatten off
        for(int j=0;j<5;j++)
        {
          box_wh[i*10+j*2] = box_wh[i*10+j*2] * anchors_tensor[j*2]/ conv_dims;
          box_wh[i*10+j*2+1] = box_wh[i*10+j*2+1] * anchors_tensor[j*2+1]/ conv_dims;
        }
    }
    flag = yolo_eval(box_xy,box_wh,box_confidence,box_class_probs,boxes,box_class_scores,0.7,count);
    flag = non_max_suppresion(box_class_scores,boxes,count,10,0.8);
    int1 m = 0;
    if(flag == 2) {
    	for(int i=0;i<20;i++) {
#pragma HLS loop_flatten off
			if(box_class_scores[i]!=0) {
				m++;
				score = int(box_class_scores[i]*1000);
				bbx1 = boxes[4*i];
				bby1 = boxes[4*i+1];
				bbx2 = boxes[4*i+2];
				bby2 = boxes[4*i+3];
			}
		}
    }
    if(flag == 2) {
		compute = m;
		leaves = m;
		scores_out = score;
		out_bbx1 = bbx1;
		out_bby1 = bby1;
		out_bbx2 = bbx2;
		out_bby2 = bby2;
        cout<<"No of Leaves Detected : "<<leaves<<"\n";
        cout<<"Accuracy : "<<scores_out<<"\n";
        cout<<"Boundary Boxes : "<<out_bbx1<<", "<<out_bby1<<", "<<out_bbx2<<", "<<out_bby2<<"\n";
		}
	}
}
