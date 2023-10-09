# FPGA for Edge

## Introduction
<p align="justify">
    FPGAs are steadily entering into the race of compute-intensive work specifically in the field of Machine Learning in the name of hardware accelerators. Nowadays, FPGAs are preferred because of the hardware reconfigurability, power efficiency and low cost compared to CPUs and GPUs.
</p>

<p align="justify"> The project aims to understand and exploit the potential of FPGAs in the application of agriculture. The work will focus on training ML models which can be deployed on FPGA, and video processing to use the ML models for indoor agricultural applications. </p>

<p align="justify">
    In this project, we have developed an YOLOv2 model to detect tomato leaves using FPGA and their performance parameters are being measured. A comparison study between FPGA, CPU & GPUs is also added to this project.
</p>

**Hardware** - Zedboard, Nexys Video Board

**Software** - Vivado HLx Edition 2018.3 & Paperspace

## Implementation
### Approach
* Develop object detection models using Paperspace.
* Compare the performance of the model to choose the ML model for deployment on the FPGA.
* Understanding HLS systems & Xilinx FPGA.
* Implementing Edge detection algorithms on FPGA.
* Implementing Yolov2-Tiny on FPGA
* Performance Analysis using python tools.

### Hardware Accelerator
<p align = "justify">
Yolo algorithm has high learning capabilities, accuracy, speed, and a well-defined regressor. So, this project uses yolo algorithm to detect tomato leaves. </p>

<p align = "justfiy">
At First, a YOLOv2-tiny architecture is implemented on the FPGA using HLS. The entire Yolo-v2 algorithm has been re-written from scratch in C++ for implementation. Also, a custom regressor is developed to extract the bounding box and score of the predicted classes.</p>

<div style="text" align="center">
    <img src="https://github.com/hari-vickey/FPGA-for-Edge/blob/main/documentation/images/yolov2-architecture.png" />
</div>

<p align = "justify">
The output of the algorithm is extracted and fed to the UART module to establish serial communication between the FPGA and the Computer. Using python program the serial data is read and the outputs are visualized using opencv. </p>

<div style="text" align="center">
    <img src="https://github.com/hari-vickey/FPGA-for-Edge/blob/main/documentation/images/architecturetop.png" />
</div>

The output of the yolo-v2 accelerator is shown below. The detailed explaination video can be found using this drive link.

### Results and Conclusion
<p align = "justify">
The project laid focus on the comparison between eight hardware, comprising four CPUs. two GPUs and two FPGAs. Latency and Power consumption are the two domains under comparison.</p>

<p align = "justify">
Among the CPUs, the i7-9750CPU had the least latency of 148ms in implementing YoloV2-tiny while  that of INTEL XENON 2.3GHz was recorded to be the highest (326ms). Both the ZedBoard and Nexys Video latency was recorded to be about 210ms with Nexys Video Board having better performance than Of all devices, GPUs are found to have the least latency, with NVIDIA RTX A4000 running at 124ms latency and NVIDIA Tesla at 84ms.</p>

<div style="text" align="center">
    <img src="https://github.com/hari-vickey/FPGA-for-Edge/blob/main/documentation/images/hardware-performance.png" />
</div>

<p align = "justify">
It is evident that ML model implementation works best on FPGAs and GPUs. The choice between FPGA and GPU is application specific. FPGAs are best for low-power implementation and standalone applications. GPUs for low latency models. A more efficient code can alter the speed and resources required to implement the model. Traditional hardware like CPUs has turned out to be poor choices for model implementation. Based on the resource cost and simple architecture the propsed model can be implemented on low-tier FPGAs.</p>

## Repository Structure

documentation - Final Reports, and presentation files are added.
1. images - Files related to describing this project are added.
2. poster - poster files give a short description of this project.
3. latex  - Files used to create reports and presentations are added as tex files.

objectDetection - Main project files to recreate the project using the Vivado software.
1. vivado - project and source files
2. HLS    - CPP source files for IP Export

<p align="justify">
scripts - This folder has the necessary python codes to test the developed model in FPGA and to analyze the performance in CPU & GPU.
</p>

1. models - YOLOv2 model used in this project
2. notebooks - Notebooks used to Train and develop custom yolo-v2 model
3. output - To store the output files from the scripts
4. test_images - images used for testing
