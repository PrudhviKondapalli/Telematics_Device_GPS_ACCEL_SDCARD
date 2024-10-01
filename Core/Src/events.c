/**
 * @file events.c
 * @brief Handles event analysis and file operations for storing data.
 * @author Sonal Tamrakar, Prudhvi Kondapalli
 * @date 12/16/2023
 * @credit Thanks to Nizar Mohideen for step through explanation of how
 * to use the FATFS within STM32CubeIDE to interface with the SD-Card 
 * module.
 * @link https://www.youtube.com/watch?v=aqSNz26Cuio&ab_channel=MicroPetabyNizarMohideen
 */

/**
 * Default Libraries allowed to be used
 */
#include <stdint.h>
#include <stdio.h>

/**
 * User-defined libraries
 */
#include "events.h"
#include "fatfs.h"
#include "fatfs_sd.h"
#include "string.h"

/**
 * File system and file variables
 */
FATFS fs1; // file system
FIL fil1; //file
UINT br1, bw1; //file read/write count

/**
 * User defined Macros
 */
#define DATA_VALS		(50)
#define X_RAW_IDLE		(-40)
#define X_RAW_LRANGE	(60)
#define X_HIGH			(-20)
#define X_LOW			(-70)
#define Y_HIGH			(-100)
#define Y_LOW			(-140)
#define Y_RAW_IDLE		(-120)
#define Z_RAW_IDLE		(2090)

/**
 * User defined variables
 */
char char_buf_avg0[20];
char char_buf_avg1[20];
char char_buf_avg2[20];
int16_t buffer_average[3] = {0};
int16_t buffer_range[3] = {0};
uint16_t lane_change = 0;
uint16_t irregular_accel = 0;
uint16_t rash_driving = 0;
int16_t buffer_min[3] = {0};
int16_t buffer_max[3] = {0};

/**
 * @brief Analyzes the input buffers for x, y, and z axes, calculates averages and checks for events.
 * @param xbuffer: Buffer for x-axis data
 * @param ybuffer: Buffer for y-axis data
 * @param zbuffer: Buffer for z-axis data
 */
void event_analysis(int16_t xbuffer[], int16_t ybuffer[], int16_t zbuffer[])
{
    buf_analysis(xbuffer, 0);
    buf_analysis(ybuffer, 1);
    buf_analysis(zbuffer, 2);

    // Convert buffer averages to ASCII strings
    goToAscii(buffer_average[0], char_buf_avg0);
    goToAscii(buffer_average[1], char_buf_avg1);
    goToAscii(buffer_average[2], char_buf_avg2);

    // Mount the file system
    f_mount(&fs1, "", 0);

    // Open or create the data file
    f_open(&fil1, "Blackbox_Data_Average.txt", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);

    // Move file pointer to the end of the file
    f_lseek(&fil1, f_size(&fil1));

    // Write average values to the file
    f_puts("The average value over the past 5 seconds for x axis: ", &fil1);
    f_write(&fil1, char_buf_avg0, sizeof(char_buf_avg0), &bw1);
    f_puts("\n", &fil1);

    f_puts("The average value over the past 5 seconds for y axis: ", &fil1);
    f_write(&fil1, char_buf_avg1, sizeof(char_buf_avg1), &bw1);
    f_puts("\n", &fil1);

    f_puts("The average value over the past 5 seconds for z axis: ", &fil1);
    f_write(&fil1, char_buf_avg2, sizeof(char_buf_avg2), &bw1);
    f_puts("\n", &fil1);

    // Close the file
    f_close(&fil1);

    // Check for specific events based on threshold values
    if (buffer_average[0] > X_HIGH || buffer_average[0] < X_LOW)
    {
        lane_change++;
    }

    if (buffer_average[1] > Y_HIGH || buffer_average[1] < Y_LOW)
    {
        irregular_accel++;
    }

    if ((buffer_average[0] > X_HIGH || buffer_average[0] < X_LOW) &&
        (buffer_average[1] > Y_HIGH || buffer_average[1] < Y_LOW))
    {
        rash_driving++;
    }
}

/**
 * @brief Calculates statistics for the input buffer (minimum, maximum, range, average).
 * @param input_buffer: Input data buffer
 * @param index: Index indicating the axis (0 for x, 1 for y, 2 for z)
 */
void buf_analysis(int16_t input_buffer[], int index)
{
    int32_t sum = 0;
    int16_t minimum_final = input_buffer[0];
    int16_t maximum_final = input_buffer[0];

    // Iterate through the buffer to find minimum, maximum, and calculate the sum
    for (int i = 0; i < DATA_VALS; i++)
    {
        minimum_final = (minimum_final < input_buffer[i]) ? minimum_final : input_buffer[i];
        maximum_final = (maximum_final > input_buffer[i]) ? maximum_final : input_buffer[i];
        sum += input_buffer[i];
    }

    // Update the statistics in the corresponding arrays
    buffer_min[index] = minimum_final;
    buffer_max[index] = maximum_final;
    buffer_range[index] = maximum_final - minimum_final;
    buffer_average[index] = sum / DATA_VALS;
}


/**
 * @brief Converts a 16-bit integer to an ASCII string.
 * @param number: Input 16-bit integer
 * @param resBuf: Output buffer for the ASCII string
 */
void goToAscii(int16_t number, char *resBuf)
{
    int i = 0;
    int qNegative = 0;

    // Handle negative numbers
    if (number < 0)
    {
        qNegative = 1;
        number = -number;
    }

    // Convert the number to ASCII
    do
    {
        int inDigit = number % 10;
        resBuf[i++] = inDigit + '0';
        number = number / 10;
    } while (number > 0);

    // Add the negative sign if necessary
    if (qNegative == 1)
    {
        resBuf[i++] = '-';
    }

    // Null-terminate the string
    resBuf[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end)
    {
        char temp = resBuf[start];
        resBuf[start] = resBuf[end];
        resBuf[end] = temp;
        start++;
        end--;
    }
}





