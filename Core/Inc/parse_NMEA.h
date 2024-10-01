/**
 * @file parse_NMEA.h
 * @brief NMEA sentence parsing and analysis functions.
 * @author Sonal Tamrakar, Prudhvi Kondapalli
 * @date 12/16/2023
 */

#ifndef INC_PARSE_NMEA_H_
#define INC_PARSE_NMEA_H_

/**
 * User defined Macros
 */
#define GMT  (500)

/**
 * @brief Structure to store parsed GGA (Global Positioning System Fix Data) information.
 */
typedef struct {
    float latitude;   /**< Latitude information. */
    char NS;           /**< North/South indicator. */
    float longitude;  /**< Longitude information. */
    char EW;           /**< East/West indicator. */
    int hour;          /**< Hour information. */
    int min;           /**< Minute information. */
    int sec;           /**< Second information. */
    int fixbit_gga;    /**< Fix status indicator. */
    float altitude;    /**< Altitude information. */
    char unit;         /**< Unit of altitude measurement. */
    int numofsat;      /**< Number of satellites used in the fix. */
} GGASTRUCT;

/**
 * @brief Structure to store parsed RMC (Recommended Minimum Navigation Information) information.
 */
typedef struct {
    int Day;           /**< Day information. */
    int Mon;           /**< Month information. */
    int Yr;            /**< Year information. */
    float speed;       /**< Speed information. */
    float course;      /**< Course information. */
    int fixbit_rmc;    /**< Fix status indicator. */
} RMCSTRUCT;

/**
 * @brief Structure to store combined GPS data with both GGA and RMC information.
 */
typedef struct {
    GGASTRUCT GGA;     /**< GGA information. */
    RMCSTRUCT RMC;     /**< RMC information. */
} GPSSTRUCT;


/**
 * User defined functions
 */
void GGA_analysis(char *input_buffer, GGASTRUCT *gga_input);

void RMC_analysis(char *input_buffer, RMCSTRUCT *rmc_input);

int gps_fix_check(char *input_buffer);

int valid_data_check(char *input_buffer);

int speed_data_check(char *input_buffer);

#endif /* INC_PARSE_NMEA_H_ */
