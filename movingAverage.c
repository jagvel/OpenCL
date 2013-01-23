// Moving average of floats implemented in standard C
// movingavg.c host program
#include <stdlib.h>       
#ifdef __APPLE__        
#include <OpenCL/opencl.h>        
#else       
#include "cl.h"        
#endif      
#include <stdio.h>        
         
/* Read Stock data */      
int stock_array1[] = {      
    #include "stock_array1.txt" 
};      
         
/* Define width for the moving average */      
#define WINDOW_SIZE (13)        
         
#define MAX_SOURCE_SIZE (0x100000)      
         
int main(void)      
{       
    cl_platform_id platform_id = NULL;  
    cl_uint ret_num_platforms;  
    cl_device_id device_id = NULL;  
    cl_uint ret_num_devices;    
    cl_context context = NULL;  
    cl_command_queue command_queue = NULL;  
    cl_mem memobj_in = NULL;    
    cl_mem memobj_out = NULL;   
    cl_program program = NULL;  
    cl_kernel kernel = NULL;    
    size_t kernel_code_size;    
    char *kernel_src_str;   
    float *result;  
    cl_int ret; 
    FILE *fp;   
         
    int data_num = sizeof(stock_array1) / sizeof(stock_array1[0]);  
    int window_num = (int)WINDOW_SIZE;  
    int i;  
         
    /* Allocate space to read in kernel code */
    kernel_src_str = (char *)malloc(MAX_SOURCE_SIZE);   
         
    /* Allocate space for the result on the host side */   
    result = (float *)malloc(data_num*sizeof(float));   
         
    printf("starting/n");
    /* Get Platform */ 
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);    
         
    /* Get Device */   
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id,    
        &ret_num_devices);
/*   status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
        if (numDevices == 0) //no GPU available.
        {
                cout << "No GPU device available."<<endl;
                cout << "Choose CPU as default device."<<endl;
                status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices);
                devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));

                status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices, NULL);
        }
  */  
     
    /* Create Context */   
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);   
         
    /* Create Command Queue */ 
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);  
        
    printf("after create command queue/n"); 
    /* Read Kernel Code */ 
    fp = fopen("moving_average.cl", "r");   
    kernel_code_size = fread(kernel_src_str, 1, MAX_SOURCE_SIZE, fp);   
    fclose(fp); 
         
    /* Create Program Object */
    program = clCreateProgramWithSource(context, 1, (const char **)&kernel_src_str, 
        (const size_t *)&kernel_code_size, &ret);
    /* Compile kernel */   
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL); 
         
    /* Create Kernel */
    kernel = clCreateKernel(program, "moving_average", &ret);   
         
    /* Create buffer for the input data on the device */   
    memobj_in = clCreateBuffer(context, CL_MEM_READ_WRITE,  
        data_num * sizeof(int), NULL, &ret);
         
    /* Create buffer for the result on the device */   
    memobj_out = clCreateBuffer(context, CL_MEM_READ_WRITE, 
    data_num * sizeof(float), NULL, &ret);  
         
    /* Copy input data to the global memory on the device*/
    ret = clEnqueueWriteBuffer(command_queue, memobj_in, CL_TRUE, 0,    
        data_num * sizeof(int),
        stock_array1, 0, NULL, NULL);
         
    /* Set kernel arguments */ 
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj_in);    
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&memobj_out);   
    ret = clSetKernelArg(kernel, 2, sizeof(int), (void *)&data_num);    
    ret = clSetKernelArg(kernel, 3, sizeof(int), (void *)&window_num);  
         
    /* Execute the kernel */   
    ret = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);  
         
    /* Copy result from device to host */  
    ret = clEnqueueReadBuffer(command_queue, memobj_out, CL_TRUE, 0,    
        data_num * sizeof(float),
        result, 0, NULL, NULL);
         
         
    /* OpenCL Object Finalization */   
    ret = clReleaseKernel(kernel);  
    ret = clReleaseProgram(program);    
    ret = clReleaseMemObject(memobj_in);    
    ret = clReleaseMemObject(memobj_out);   
    ret = clReleaseCommandQueue(command_queue); 
    ret = clReleaseContext(context);    
         
    /* Display Results */  
    for (i=0; i < data_num; i++) {   
        printf("result[%d] = %f\n", i, result[i]);
    }   
         
    /* Deallocate memory on the host */
    free(result);   
    free(kernel_src_str);   
         
    return 0;   
}       


// Kernel program
// moving_average.cl

_kernel void moving_average(__global int *values,      
    __global float *average,    
    int length, 
    int width)  
{       
    int i;  
    int add_value;  
         
    /* Compute sum for the first "width" elements */   
    add_value = 0;  
    for (i=0; i < width; i++) {  
        add_value += values[i];
    }   
    average[width-1] = (float)add_value;    
         
    /* Compute sum for the (width)th ～ (length-1)th elements */
    for (i=width; i < length; i++) { 
        add_value = add_value - values[i-width] + values[i];
        average[i] = (float)(add_value);
    }   
         
    /* Insert zeros to 0th ~ (width-2)th elements */   
    for (i=0; i < width-1; i++) {    
        average[i] = 0.0f;
    }   
         
    /* Compute average of (width-1) ~ (length-1) elements */   
    for (i=width-1; i < length; i++) {   
        average[i] /= (float)width;
    }   
}       
