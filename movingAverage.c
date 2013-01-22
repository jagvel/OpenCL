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
