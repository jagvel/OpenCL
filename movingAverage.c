 Moving average of floats implemented in standard C


void moving_average_float(float *values,        
    float *average, 
    int length, 
    int width)  
{       
    int i, j;   
    float add_value;    
         
    /* Insert zeros to 0th ~ (width-2)th elements */   
    for (i=0; i < width-1; i++) {    
        average[i] = 0.0f;
    }   
         
    /* Compute average of (width-1) ~ (length-1) elements */   
    for (i=width-1; i < length; i++) {   
        add_value = 0.0f;
        for (j=0; j < width; j++) {
         
        }
        average[i] = add_value / (float)width;
    }   
}       

001
002
003
004
005
006
007
008
009
010
011
012
013
014
015
016
017
018
019
020
021
022
023
024
025
026
027
028
029
030
031
032
033
034
035
036
037
038
#include <stdio.h>        
#include <stdlib.h>       
         
/* Read Stock data */      
int stock_array1[] = {      
    #include "stock_array1.txt" 
};      
         
/* Define width for the moving average */      
#define WINDOW_SIZE (13)        
         
int main(int argc, char *argv[])        
{       
         
    float *result;  
         
    int data_num = sizeof(stock_array1) / sizeof(stock_array1[0]);  
    int window_num = (int)WINDOW_SIZE;  
         
    int i;  
         
    /* Allocate space for the result */
    result = (float *)malloc(data_num*sizeof(float));   
         
    /* Call the moving average function */ 
    moving_average(stock_array1,    
        result,
        data_num,
        window_num);
         
    /* Print result */ 
    for (i=0; i < data_num; i++) {   
        printf("result[%d] = %f\n", i, result[i]);
    }   
         
    /* Deallocate memory */
    free(result);   
}       

// Moving average kernel (moving_average.cl)

__kernel void moving_average(__global int *values,      
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


List 5.26: Host code to execute the moving_average() kernel
001
002
003
004
005
006
007
008
009
010
011
012
013
014
015
016
017
018
019
020
021
022
023
024
025
026
027
028
029
030
031
032
033
034
035
036
037
038
039
040
041
042
043
044
045
046
047
048
049
050
051
052
053
054
055
056
057
058
059
060
061
062
063
064
065
066
067
068
069
070
071
072
073
074
075
076
077
078
079
080
081
082
083
084
085
086
087
088
089
090
091
092
093
094
095
096
097
098
099
100
101
102
103
104
105
106
107
108
109
110
111
112
113
114
115
116
117
118
119
120
#include <stdlib.h>       
#ifdef __APPLE__        
#include <OpenCL/opencl.h>        
#else       
#include <CL/cl.h>        
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
         
    /* Get Platform */ 
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);    
         
    /* Get Device */   
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id,    
        &ret_num_devices);
         
    /* Create Context */   
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);   
         
    /* Create Command Queue */ 
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);  
         
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

List 5.29: Vector-ized moving average kernel (moving_average_vec4.cl)
001
002
003
004
005
006
007
008
009
010
011
012
013
014
015
016
017
018
019
020
021
022
023
024
025
026
027
028
029
030
031
__kernel void moving_average_vec4(__global int4 *values,        
    __global float4 *average,   
    int length, 
    int width)  
{       
    int i;  
    int4 add_value; /* A vector to hold 4 components */
         
    /* Compute sum for the first "width" elements for 4 stocks */  
    add_value = (int4)0;    
    for (i=0; i < width; i++) {  
        add_value += values[i];
    }   
    average[width-1] = convert_float4(add_value);   
         
    /* Compute sum for the (width)th ～ (length-1)th elements for 4 stocks */   
    for (i=width; i < length; i++) { 
        add_value = add_value - values[i-width] + values[i];
        average[i] = convert_float4(add_value);
    }   
         
    /* Insert zeros to 0th ～ (width-2)th element for 4 stocks*/
    for (i=0; i < width-1; i++) {    
        average[i] = (float4)(0.0f);
    }   
         
    /* Compute average of (width-1) ~ (length-1) elements for 4 stocks */  
    for (i=width-1; i < length; i++) {   
        average[i] /= (float4)width;
    }   
}       

List 5.30: Host code to run the vector-ized moving average kernel
001
002
003
004
005
006
007
008
009
010
011
012
013
014
015
016
017
018
019
020
021
022
023
024
025
026
027
028
029
030
031
032
033
034
035
036
037
038
039
040
041
042
043
044
045
046
047
048
049
050
051
052
053
054
055
056
057
058
059
060
061
062
063
064
065
066
067
068
069
070
071
072
073
074
075
076
077
078
079
080
081
082
083
084
085
086
087
088
089
090
091
092
093
094
095
096
097
098
099
100
101
102
103
104
105
106
107
108
109
110
111
112
113
114
115
116
117
118
119
120
121
122
123
124
125
126
127
128
129
130
#include <stdlib.h>           
#ifdef __APPLE__            
#include <OpenCL/opencl.h>            
#else           
#include <CL/cl.h>            
#endif          
#include <stdio.h>            
             
#define NAME_NUM (4) /* Number of Stocks */         
#define DATA_NUM (21) /* Number of data to process for each stock*/         
             
/* Read Stock data */          
int stock_array_4[NAME_NUM*DATA_NUM]= {         
    #include "stock_array_4.txt"        
};          
             
/* Moving average width */         
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
             
    int window_num = (int)WINDOW_SIZE;      
    int point_num = NAME_NUM * DATA_NUM;        
    int data_num = (int)DATA_NUM;       
    int name_num = (int)NAME_NUM;       
    int i, j;       
             
    /* Allocate space to read in kernel code */    
    kernel_src_str = (char *)malloc(MAX_SOURCE_SIZE);       
             
    /* Allocate space for the result on the host side */       
    result = (float *)malloc(point_num*sizeof(float));      
             
    /* Get Platform */     
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);        
             
    /* Get Device */       
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id,        
        &ret_num_devices);  
             
    /* Create Context */       
    context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);      
             
    /* Create command queue */     
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);      
             
    /* Read kernel source code */      
    fp = fopen("moving_average_vec4.cl", "r");      
    kernel_code_size = fread(kernel_src_str, 1, MAX_SOURCE_SIZE, fp);       
    fclose(fp);     
             
    /* Create Program Object */    
    program = clCreateProgramWithSource(context, 1, (const char **)&kernel_src_str,     
        (const size_t *)&kernel_code_size, &ret);   
             
    /* Compile kernel */       
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);     
             
    /* Create kernel */    
    kernel = clCreateKernel(program, "moving_average_vec4", &ret);      
             
    /* Create buffer for the input data on the device */       
    memobj_in = clCreateBuffer(context, CL_MEM_READ_WRITE,      
        point_num * sizeof(int), NULL, &ret);   
             
             
    /* Create buffer for the result on the device */       
    memobj_out = clCreateBuffer(context, CL_MEM_READ_WRITE,     
        point_num * sizeof(float), NULL, &ret); 
             
    /* Copy input data to the global memory on the device*/    
    ret = clEnqueueWriteBuffer(command_queue, memobj_in, CL_TRUE, 0,        
        point_num * sizeof(int),    
        stock_array_4, 0, NULL, NULL);  
             
    /* Set Kernel Arguments */     
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj_in);        
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&memobj_out);       
    ret = clSetKernelArg(kernel, 2, sizeof(int), (void *)&data_num);        
    ret = clSetKernelArg(kernel, 3, sizeof(int), (void *)&window_num);      
             
    /* Execute kernel */       
    ret = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);      
             
    /* Copy result from device to host */      
    ret = clEnqueueReadBuffer(command_queue, memobj_out, CL_TRUE, 0,        
        point_num * sizeof(float),  
        result, 0, NULL, NULL); 
             
    /* OpenCL Object Finalization */       
    ret = clReleaseKernel(kernel);      
    ret = clReleaseProgram(program);        
    ret = clReleaseMemObject(memobj_in);        
    ret = clReleaseMemObject(memobj_out);       
    ret = clReleaseCommandQueue(command_queue);     
    ret = clReleaseContext(context);        
             
    /* Print results */    
    for (i=0; i < data_num; i++) {       
        printf("result[%d]:", i);   
        for (j=0; j < name_num; j++) {   
            printf("%f, ", result[i*NAME_NUM+j]);
        }   
        printf("\n");   
    }       
             
    /* Deallocate memory on the host */    
    free(result);       
    free(kernel_src_str);       
             
    return 0;       
}           


List 5.35: Host code for task parallel processing of 2 moving averages
001
002
003
004
005
006
007
008
009
010
011
012
013
014
015
016
017
018
019
020
021
022
023
024
025
026
027
028
029
030
031
032
033
034
035
036
037
038
039
040
041
042
043
044
045
046
047
048
049
050
051
052
053
054
055
056
057
058
059
060
061
062
063
064
065
066
067
068
069
070
071
072
073
074
075
076
077
078
079
080
081
082
083
084
085
086
087
088
089
090
091
092
093
094
095
096
097
098
099
100
101
102
103
104
105
106
107
108
109
110
111
112
113
114
115
116
117
118
119
120
121
122
123
124
125
126
127
128
129
130
131
132
133
134
135
136
137
138
139
140
141
142
143
144
145
146
147
148
149
150
151
152
153
154
155
156
157
158
159
160
161
#include <stdlib.h>           
#ifdef __APPLE__            
#include <OpenCL/opencl.h>            
#else           
#include <CL/cl.h>            
#endif          
#include <stdio.h>            
             
#define NAME_NUM (4) /* Number of stocks */         
#define DATA_NUM (100) /* Number of data to process for each stock */           
             
/* Read Stock data */          
int stock_array_4[NAME_NUM*DATA_NUM]= {         
    #include "stock_array_4.txt"        
};          
             
/* Moving average width */         
#define WINDOW_SIZE_13 (13)         
#define WINDOW_SIZE_26 (26)         
             
             
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
    cl_mem memobj_out13 = NULL;     
    cl_mem memobj_out26 = NULL;     
    cl_program program = NULL;      
    cl_kernel kernel13 = NULL;      
    cl_kernel kernel26 = NULL;      
    cl_event event13, event26;      
    size_t kernel_code_size;        
    char *kernel_src_str;       
    float *result13;        
    float *result26;        
    cl_int ret;     
    FILE *fp;       
             
    int window_num_13 = (int)WINDOW_SIZE_13;        
    int window_num_26 = (int)WINDOW_SIZE_26;        
    int point_num = NAME_NUM * DATA_NUM;        
    int data_num = (int)DATA_NUM;       
    int name_num = (int)NAME_NUM;       
             
    int i, j;       
             
    /* Allocate space to read in kernel code */    
    kernel_src_str = (char *)malloc(MAX_SOURCE_SIZE);       
             
    /* Allocate space for the result on the host side */       
    result13 = (float *)malloc(point_num*sizeof(float)); /* average over 13 weeks */       
    result26 = (float *)malloc(point_num*sizeof(float)); /* average over 26 weeks */       
             
    /* Get Platform */     
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);        
             
    /* Get Device */       
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id,        
        &ret_num_devices);  
             
    /* Create Context */       
    context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);      
             
    /* Create Command Queue */     
    command_queue = clCreateCommandQueue(context, device_id,        
        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ret);  
             
    /* Read kernel source code */      
    fp = fopen("moving_average_vec4.cl", "r");      
    kernel_code_size = fread(kernel_src_str, 1, MAX_SOURCE_SIZE, fp);       
    fclose(fp);     
             
    /* Create Program Object */    
    program = clCreateProgramWithSource(context, 1, (const char **)&kernel_src_str,     
        (const size_t *)&kernel_code_size, &ret);   
             
    /* Compile kernel */       
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);     
             
    /* Create kernel */    
    kernel13 = clCreateKernel(program, "moving_average_vec4", &ret); /* 13 weeks */    
    kernel26 = clCreateKernel(program, "moving_average_vec4", &ret); /* 26 weeks */    
             
    /* Create buffer for the input data on the device */       
    memobj_in = clCreateBuffer(context, CL_MEM_READ_WRITE,      
        point_num * sizeof(int), NULL, &ret);   
             
    /* Create buffer for the result on the device */       
    memobj_out13 = clCreateBuffer(context, CL_MEM_READ_WRITE,       
        point_num * sizeof(float), NULL, &ret); /* 13 weeks */ 
    memobj_out26 = clCreateBuffer(context, CL_MEM_READ_WRITE,       
        point_num * sizeof(float), NULL, &ret); /* 26 weeks */ 
             
    /* Copy input data to the global memory on the device*/    
    ret = clEnqueueWriteBuffer(command_queue, memobj_in, CL_TRUE, 0,        
        point_num * sizeof(int),    
        stock_array_4, 0, NULL, NULL);  
             
    /* Set Kernel Arguments (13 weeks) */      
    ret = clSetKernelArg(kernel13, 0, sizeof(cl_mem), (void *)&memobj_in);      
    ret = clSetKernelArg(kernel13, 1, sizeof(cl_mem), (void *)&memobj_out13);       
    ret = clSetKernelArg(kernel13, 2, sizeof(int), (void *)&data_num);      
    ret = clSetKernelArg(kernel13, 3, sizeof(int), (void *)&window_num_13);     
             
    /* Submit task to compute the moving average over 13 weeks */      
    ret = clEnqueueTask(command_queue, kernel13, 0, NULL, &event13);        
             
    /* Set Kernel Arguments (26 weeks) */      
    ret = clSetKernelArg(kernel26, 0, sizeof(cl_mem), (void *)&memobj_in);      
    ret = clSetKernelArg(kernel26, 1, sizeof(cl_mem), (void *)&memobj_out26);       
    ret = clSetKernelArg(kernel26, 2, sizeof(int), (void *)&data_num);      
    ret = clSetKernelArg(kernel26, 3, sizeof(int), (void *)&window_num_26);     
             
    /* Submit task to compute the moving average over 26 weeks */      
    ret = clEnqueueTask(command_queue, kernel26, 0, NULL, &event26);        
             
    /* Copy result for the 13 weeks moving average from device to host */      
    ret = clEnqueueReadBuffer(command_queue, memobj_out13, CL_TRUE, 0,      
        point_num * sizeof(float),  
        result13, 1, &event13, NULL);   
             
    /* Copy result for the 26 weeks moving average from device to host */      
    ret = clEnqueueReadBuffer(command_queue, memobj_out26, CL_TRUE, 0,      
        point_num * sizeof(float),  
        result26, 1, &event26, NULL);   
             
    /* OpenCL Object Finalization */       
    ret = clReleaseKernel(kernel13);        
    ret = clReleaseKernel(kernel26);        
    ret = clReleaseProgram(program);        
    ret = clReleaseMemObject(memobj_in);        
    ret = clReleaseMemObject(memobj_out13);     
    ret = clReleaseMemObject(memobj_out26);     
    ret = clReleaseCommandQueue(command_queue);     
    ret = clReleaseContext(context);        
             
    /* Display results */      
    for (i=window_num_26-1; i < data_num; i++) {     
        printf("result[%d]:", i );  
        for (j=0; j < name_num; j++ ) {  
            /* Display whether the 13 week average is greater */
            printf( "[%d] ", (result13[i*NAME_NUM+j] > result26[i*NAME_NUM+j]) );
        }   
        printf("\n");   
    }       
             
    /* Deallocate memory on the host */    
    free(result13);     
    free(result26);     
    free(kernel_src_str);       
             
    return 0;       
}           

List 5.34: Host code for calling the kernel in List 5.33
001
002
003
004
005
006
007
008
009
010
011
012
013
014
015
016
017
018
019
020
021
022
023
024
025
026
027
028
029
030
031
032
033
034
035
036
037
038
039
040
041
042
043
044
045
046
047
048
049
050
051
052
053
054
055
056
057
058
059
060
061
062
063
064
065
066
067
068
069
070
071
072
073
074
075
076
077
078
079
080
081
082
083
084
085
086
087
088
089
090
091
092
093
094
095
096
097
098
099
100
101
102
103
104
105
106
107
108
109
110
111
112
113
114
115
116
117
118
119
120
121
122
123
124
125
126
127
128
129
130
131
132
133
134
135
136
137
138
139
140
141
142
143
#include <stdlib.h>           
#ifdef __APPLE__            
#include <OpenCL/opencl.h>            
#else           
#include <CL/cl.h>            
#endif          
#include <stdio.h>            
             
#define NAME_NUM (8) /* Number of stocks */         
#define DATA_NUM (21) /* Number of data to process for each stock */            
             
/* Read Stock data */          
int stock_array_many[NAME_NUM*DATA_NUM]= {          
    #include "stock_array_many.txt"     
};          
             
/* Moving average width */         
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
             
    int window_num = (int)WINDOW_SIZE;      
    int point_num = NAME_NUM * DATA_NUM;        
    int data_num = (int)DATA_NUM;       
    int name_num = (int)NAME_NUM;       
             
    int i,j;        
             
    /* Allocate space to read in kernel code */    
    kernel_src_str = (char *)malloc(MAX_SOURCE_SIZE);       
             
    /* Allocate space for the result on the host side */       
    result = (float *)malloc(point_num*sizeof(float));      
             
    /* Get Platform*/      
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);        
             
    /* Get Device */       
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id,        
        &ret_num_devices);  
             
    /* Create Context */       
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);       
             
    /* Create Command Queue */     
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);      
             
    /* Read kernel source code */      
    fp = fopen("moving_average_vec4_para.cl", "r");     
    kernel_code_size = fread(kernel_src_str, 1, MAX_SOURCE_SIZE, fp);       
    fclose(fp);     
             
    /* Create Program Object */    
    program = clCreateProgramWithSource(context, 1, (const char **)&kernel_src_str,     
    (const size_t *)&kernel_code_size, &ret);       
             
    /* Compile kernel */       
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);     
             
    /* Create kernel */    
    kernel = clCreateKernel(program, "moving_average_vec4_para", &ret);     
             
    /* Create buffer for the input data on the device */       
    memobj_in = clCreateBuffer(context, CL_MEM_READ_WRITE,      
    point_num * sizeof(int), NULL, &ret);       
             
    /* Create buffer for the result on the device */       
    memobj_out = clCreateBuffer(context, CL_MEM_READ_WRITE,     
    point_num * sizeof(float), NULL, &ret);     
             
    /* Copy input data to the global memory on the device*/    
    ret = clEnqueueWriteBuffer(command_queue, memobj_in, CL_TRUE, 0,        
        point_num * sizeof(int),    
        stock_array_many, 0, NULL, NULL);   
             
    /* Set Kernel Arguments */     
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj_in);        
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&memobj_out);       
    ret = clSetKernelArg(kernel, 2, sizeof(int), (void *)&data_num);        
    ret = clSetKernelArg(kernel, 3, sizeof(int), (void *)&name_num);        
    ret = clSetKernelArg(kernel, 4, sizeof(int), (void *)&window_num);      
             
             
    /* Set parameters for data parallel processing (work item) */      
    cl_uint work_dim = 1;       
    size_t global_item_size[3];     
    size_t local_item_size[3];      
             
    global_item_size[0] = 2; /* Global number of work items */     
    local_item_size[0] = 1; /* Number of work items per work group */      
    /* --> global_item_size[0] / local_item_size[0] becomes 2, which indirectly sets the number of workgroups to 2*/    
             
    /* Execute Data Parallel Kernel */     
    ret = clEnqueueNDRangeKernel(command_queue, kernel, work_dim, NULL,     
        global_item_size, local_item_size,  
        0, NULL, NULL); 
             
    /* Copy result from device to host */      
    ret = clEnqueueReadBuffer(command_queue, memobj_out, CL_TRUE, 0,        
        point_num * sizeof(float),  
        result, 0, NULL, NULL); 
             
    /* OpenCL Object Finalization */       
    ret = clReleaseKernel(kernel);      
    ret = clReleaseProgram(program);        
    ret = clReleaseMemObject(memobj_in);        
    ret = clReleaseMemObject(memobj_out);       
    ret = clReleaseCommandQueue(command_queue);     
    ret = clReleaseContext(context);        
             
    /* Deallocate memory on the host */    
    for (i=0; i < data_num; i++) {       
        printf("result[%d]: ", i);  
        for (j=0; j < name_num; j++) {   
            printf("%f, ", result[i*NAME_NUM+j]);
        }   
        printf("\n");   
    }       
             
    /* Deallocate memory on the host */    
    free(result);       
    free(kernel_src_str);       
             
    return 0;       
}           
