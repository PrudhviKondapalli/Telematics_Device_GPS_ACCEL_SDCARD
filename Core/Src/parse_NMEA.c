/**
 * @file NMEA.c
 * @brief NMEA sentence parsing and analysis functions.
 * @author Sonal Tamrakar, Prudhvi Kondapalli
 * @date 12/16/2023
 * @credit Thanks to ControllersTech for the informative
 * tutorial on UART using STM32 family. ControllersTech gave us a
 * good understanding on how to parse NMEA sentences and use 
 * them for processing. Link below
 * @leveraged code We utilized ControllersTech's method
 * to convert latitude + longitude buffers to their
 * decimal representations. This is evident in the functions
 * GGA_analysis() and RMC_analysis().
 * @link https://www.youtube.com/watch?v=tq_RoaPLahk&ab_channel=ControllersTech
 */

/**
 * Default Libraries allowed to be used
 */
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

/**
 * User-defined libraries
 */
#include "fatfs.h"
#include "fatfs_sd.h"
#include "systick.h"
#include <parse_NMEA.h>

/**
 * User defined Macros
 */
#define FIX_POS		(6)
#define VALID_POS	(2)
#define SPEED_POS	(7)

/**
 * File system and file variables
 */
FATFS fs2; // file system
FIL fil2; //file
UINT br2, bw2; //file read/write count

FATFS fs3;
FIL fil3;
UINT br3, bw3;

/**
 * User defined variables
 */
int index1 = 0;

/**
 * @brief Checks the GPS fix status in the NMEA sentence.
 * @param input_buffer: NMEA sentence buffer.
 * @return The index of the GPS fix status in the buffer or -1 if not found.
 */
int gps_fix_check(char *input_buffer) {
  // This function checks if the input buffer contains a valid GPS fix.
  // The buffer is expected to be a comma-separated string containing GPS data.

  int comma_count = 0; // Initialize a counter for encountered commas.

  // Loop through the first 60 characters of the buffer, searching for the expected number of commas.
  for (int i = 0; i < 60; i++) {
    if (input_buffer[i] == ',') {
      comma_count++; // Increment the counter for each comma found.
      if (comma_count == FIX_POS) { // Check if the expected number of commas is reached.
        // FIX_POS is a predefined constant specifying the number of commas
        // before the start of the GPS fix data (e.g., 6 for latitude,
        // longitude, altitude, etc.).
        return i + 1; // If found, return the index of the next character,
        // signifying the start of the fix data.
      }
    }
  }
  // No valid fix found within the first 60 characters or the expected comma count wasn't reached.
  return -1; // Return -1 to indicate failure.
}

/**
 * @brief Parses and analyzes GGA NMEA sentence.
 * @param input_buffer: NMEA sentence buffer.
 * @param gga: Pointer to the GGASTRUCT structure to store the parsed data.
 */
void GGA_analysis(char *input_buffer, GGASTRUCT *gga)
{
    // Initialize variables and buffers for extracted data.
    int index1 = 0;
    char time_buffer[12] = {0}; // Holds extracted time data.
    char latitude_buffer[12] = {0}; // Holds extracted latitude data.
    char longitude_buffer[12] = {0}; // Holds extracted longitude data.
    char satellite_buffer[12] = {0}; // Holds extracted number of satellites.
    char altitude_buffer[12] = {0}; // Holds extracted altitude data.

    // Check if the input buffer contains a valid GPS fix.
    index1 = gps_fix_check(input_buffer);
    if (input_buffer[index1] == '0') {
        // Fixbit indicates no valid fix, set GGA data accordingly and return.
        gga->fixbit_gga = 0;
        return;
    } else {
        // Valid fix found, set fixbit and reset index.
        gga->fixbit_gga = 1;
        index1 = 0;
    }

    // Extract time data from the sentence, starting at index 8 (after "$GPGGA,").
    index1 = 8;
    int i = 0;
    for (; input_buffer[index1] != ','; time_buffer[i++] = input_buffer[index1++]);

    // Convert time data to hours, minutes, and seconds with GMT adjustment.
    gga->hour = ((time_buffer[0] - '0') * 10 + (time_buffer[1] - '0')) + (GMT/100) - 12;
    gga->min = ((time_buffer[2] - '0') * 10 + (time_buffer[3] - '0')) + (GMT % 100);
    gga->sec = (time_buffer[4] - '0') * 10 + (time_buffer[5] - '0');

    // Extract latitude data (next field after comma).
    index1++;
    i = 0;
    for (; input_buffer[index1] != ','; latitude_buffer[i++] = input_buffer[index1++]);

    // Check for minimum buffer size to ensure proper conversion.
    if (strlen(latitude_buffer) < 6) {
        // Insufficient data, return error code.
        return 2;
    }

    // Convert latitude buffer to a float value with decimal precision.
    int num = atoi(latitude_buffer); // Convert string to integer (ignoring decimals).
    int j = 0;
    while (latitude_buffer[j] != '.') j++; // Find decimal point position.
    j++; // Skip decimal point.
    int declen = (strlen(latitude_buffer)) - j; // Calculate decimal part length.
    int dec = atoi((char *)latitude_buffer + j); // Convert decimal part to integer.
    float lat = (num / 100.0) + (dec / pow(10, (declen + 2))); // Combine integer and decimal parts.

    // Store extracted latitude data in the GGA structure.
    gga->latitude = lat;

    // Extract North/South indicator (next character after comma) and store.
    index1++;
    int store_index_NS = index1;
    gga->NS = input_buffer[index1];

    // Extract longitude data (similar process as latitude).
    index1++; // Skip comma after N/S indicator.
    index1++; // Reach the first number in the longitude.
    i = 0;
    for (; input_buffer[index1] != ','; longitude_buffer[i++] = input_buffer[index1++]);

    num = atoi(longitude_buffer); // Convert string to integer.
    j = 0;
    while (longitude_buffer[j] != '.') j++; // Find decimal point position.
    j++; // Skip decimal point.
    declen = (strlen(longitude_buffer)) - j; // Calculate decimal part length.
    dec = atoi((char *)longitude_buffer + j); // Convert decimal part to integer.
    lat = (num / 100.0) + (dec / pow(10, (declen + 2))); // Combine integer and decimal parts.

    // Store extracted longitude data in the GGA structure.
    gga->longitude = lat;

    // Extract East/West indicator (next character after comma) and store.
    index1++;
    int store_index_EW = index1;
    gga->EW = input_buffer[index1];

    /**************************************************/
    // Skip position fix and related commas.
    index1++;   // ',' after E/W
    index1++;   // Position fix
    index1++;   // ',' after position fix;

    // Number of satellites
    index1++;  // Reach the first number in the satellites
    i = 0;
    for (; input_buffer[index1] != ','; satellite_buffer[i++] = input_buffer[index1++]);

    // Convert and store the number of satellites in the GGA structure.
    gga->numofsat = atoi(satellite_buffer);

    /***************** Skip HDOP  *********************/
    index1++;
    while (input_buffer[index1] != ',') index1++;

    // Extract altitude data (next field after HDOP).
    index1++;
    i = 0;
    for (; input_buffer[index1] != ','; altitude_buffer[i++] = input_buffer[index1++]);

    // Convert altitude buffer to a float value with decimal precision.
    num = atoi(altitude_buffer);
    j = 0;
    while (altitude_buffer[j] != '.') j++;
    j++;
    declen = (strlen(altitude_buffer)) - j;
    dec = atoi((char *)altitude_buffer + j);
    lat = (num) + (dec / pow(10, (declen)));

    // Store extracted altitude data in the GGA structure.
    gga->altitude = lat;

    // Extract altitude unit (next character after altitude data) and store.
    index1++;
    gga->unit = input_buffer[index1];

    // Mount the file system, open or create the file, and set file pointer to the end
    f_mount(&fs2, "", 0);
    f_open(&fil2, "GGA_DATA.txt", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
    f_lseek(&fil2, f_size(&fil2));

    // Write timestamp data to the file
    f_puts("Timestamp: ", &fil2);
    f_write(&fil2, time_buffer, sizeof(time_buffer), &bw2);
    f_puts("\n", &fil2);

    // Write latitude data to the file
    f_puts("Latitude: ", &fil2);
    f_write(&fil2, latitude_buffer, sizeof(latitude_buffer), &bw2);
    f_write(&fil2, input_buffer[store_index_NS], sizeof(char), &bw2);
    f_puts("\n", &fil2);

    // Write longitude data to the file
    f_puts("Longitude: ", &fil2);
    f_write(&fil2, longitude_buffer, sizeof(longitude_buffer), &bw2);
    f_write(&fil2, input_buffer[store_index_EW], sizeof(char), &bw2);
    f_puts("\n", &fil2);

    // Write number of satellites data to the file
    f_puts("Number of satellites: ", &fil2);
    f_write(&fil2, satellite_buffer, sizeof(satellite_buffer), &bw2);
    f_puts("\n", &fil2);

    // Write altitude data to the file
    f_puts("Altitude: ", &fil2);
    f_write(&fil2, altitude_buffer, sizeof(altitude_buffer), &bw2);
    f_puts("\n", &fil2);

    // Close the file
    f_close(&fil2);
}


/**
 * @brief Checks the validity status in the NMEA sentence.
 * @param input_buffer: NMEA sentence buffer.
 * @return The index of the validity status in the buffer or -1 if not found.
 */
int valid_data_check(char *input_buffer)
{
	int comma_count = 0;
	for(int i = 0; i < 60; i++)
	{
		if(input_buffer[i] == ',')
		{
			comma_count++;
			if(comma_count == VALID_POS)
			return i+1;
		}
	}
	return -1;
}

/**
 * @brief Checks the speed data in the NMEA sentence.
 * @param input_buffer: NMEA sentence buffer.
 * @return The index of the speed data in the buffer or -1 if not found.
 */
int speed_data_check(char *input_buffer)
{
	int comma_count = 0;
		for(int i = 0; i < 60; i++)
		{
			if(input_buffer[i] == ',')
			{
				comma_count++;
				if(comma_count == SPEED_POS)
				return i+1;
			}
		}
		return -1;
}

/**
 * @brief Parses and analyzes RMC NMEA sentence.
 * @param input_buffer: NMEA sentence buffer.
 * @param rmc: Pointer to the RMCSTRUCT structure to store the parsed data.
 */
void RMC_analysis(char *input_buffer, RMCSTRUCT *rmc)
{
    // Initialize variables
    index1 = 0;
    char speed_buffer[12] = {0};
    char course_buffer[12] = {0};
    char date_buffer[12] = {0};

    int i = 0;

    // Check the validity status in the NMEA sentence
    index1 = valid_data_check(input_buffer);
    if (input_buffer[index1] == 'A')
    {
        rmc->fixbit_rmc = 1;  // Valid data, set fixbit_rmc to 1
    }
    else
    {
        rmc->fixbit_rmc = 0;  // Invalid data, set fixbit_rmc to 0
        return;
    }

    // Reset index1 to 0 and locate the speed data
    index1 = 0;
    index1 = speed_data_check(input_buffer);

    // Parse and store speed data
    i = 0;
    for (; input_buffer[index1] != ','; speed_buffer[i++] = input_buffer[index1++]);

    if (strlen(speed_buffer) > 0)
    {
        // Convert and store speed data as a floating-point number
        int16_t num = (atoi(speed_buffer));
        int j = 0;
        while (speed_buffer[j] != '.')
            j++;
        j++;
        int declen = (strlen(speed_buffer)) - j;
        int dec = atoi((char *)speed_buffer + j);
        float lat = num + (dec / pow(10, (declen)));
        rmc->speed = lat;
    }
    else
    {
        rmc->speed = 0;
    }

    // Move to the next field (course data)
    index1++;
    i = 0;
    for (; input_buffer[index1] != ','; course_buffer[i++] = input_buffer[index1++]);

    if (strlen(course_buffer) > 0)
    {
        // Convert and store course data as a floating-point number
        int16_t num = (atoi(course_buffer));
        int j = 0;
        while (course_buffer[j] != '.')
            j++;
        j++;
        int declen = (strlen(course_buffer)) - j;
        int dec = atoi((char *)course_buffer + j);
        float lat = num + (dec / pow(10, (declen)));
        rmc->course = lat;
    }
    else
    {
        rmc->course = 0;
    }

    // Move to the next field (date data)
    index1++;
    i = 0;
    for (; input_buffer[index1] != ','; date_buffer[i++] = input_buffer[index1++]);

    // Parse and store date components
    rmc->Day = (date_buffer[0] - '0') * 10 + (date_buffer[1] - '0');
    rmc->Mon = (date_buffer[2] - '0') * 10 + (date_buffer[3] - '0');
    rmc->Yr = (date_buffer[4] - '0') * 10 + (date_buffer[5] - '0');

    // Mount the file system, open or create the file, and set file pointer to the end
    f_mount(&fs3, "", 0);
    f_open(&fil3, "RMC_DATA.txt", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
    f_lseek(&fil3, f_size(&fil3));

    // Write speed data to the file
    f_puts("Speed: ", &fil3);
    f_write(&fil3, speed_buffer, sizeof(speed_buffer), &bw3);
    f_puts("\n", &fil3);

    // Write course data to the file
    f_puts("Course: ", &fil3);
    f_write(&fil3, course_buffer, sizeof(course_buffer), &bw3);
    f_puts("\n", &fil3);

    // Write date data to the file
    f_puts("Date: ", &fil3);
    f_write(&fil3, date_buffer, sizeof(date_buffer), &bw3);
    f_puts("\n", &fil3);

    // Close the file
    f_close(&fil3);
}


