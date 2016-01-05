#ifndef ICEBOT_DEFINITIONS
#define ICEBOT_DEFINITIONS

//*****************************************************************************//
//
//	Author: Alperen Degirmenci
//
//	Date: 11/29/2015
//
//	COPYRIGHT:		COPYRIGHT HARVARD BIOROBOTICS LABORATORY - 2015
//
//*****************************************************************************//
//
//	DESCRIPTION:   icebot_definitions.h
//
//	Header file containing definitions of constants and structures required.
//
//*****************************************************************************//

enum LOG_TYPES
{
    LOG_INFO, // regular messages
    LOG_WARNING, // warnings
    LOG_ERROR, // errors
    LOG_FATAL // fatal errors
};

// **********************
// ********  EM  ********
// **********************

enum EM_ERROR_CODES
{
    EM_SUCCESS = 0,     //
    EM_FAIL,
    EM_BELOW_MIN_SAMPLE_RATE,
    EM_ABOVE_MAX_SAMPLE_RATE,
    EM_CANT_MUTATE_WHILE_RUNNING,
    EM_FREQ_SET_FAILURE,
    EM_SET_DATA_FORMAT_TYPE_FAILURE
};

enum EM_EVENT_IDS
{
	EM_INITIALIZE_BEGIN = 0,
	EM_INITIALIZE_FAILED,
	EM_INITIALIZED,
	EM_SENSORS_DETECTED,
	EM_TRANSMITTER_SET,
	EM_FREQ_DETECTED,
    EM_FREQ_SET,
    EM_FREQ_SET_FAILED,
	EM_ACQUISITION_STARTED,
	EM_START_ACQUISITION_FAILED,
	EM_ACQUIRE_FAILED,
	EM_ACQUISITION_STOPPED,
	EM_STOP_ACQUISITION_FAILED,
	EM_DISCONNECT_FAILED,
    EM_DISCONNECTED,
	EM_EPOCH_SET,
	EM_EPOCH_SET_FAILED,
	EM_SET_DATA_FORMAT_TYPE,
	EM_SET_DATA_FORMAT_TYPE_FAILED
};

static const int EM_DEFAULT_SAMPLE_RATE = 150;
static const int EM_MIN_SAMPLE_RATE = 20;
static const int EM_MAX_SAMPLE_RATE = 250;

// **********************
// ******** EPOS ********
// **********************

enum EPOS_ERROR_CODES
{
    EPOS_SUCCESS = 0,
    EPOS_FAIL,
    EPOS_FAILED_TO_CONNECT
};

enum EPOS_EVENT_IDS
{
    EPOS_INITIALIZE_BEGIN = 0,
    EPOS_INITIALIZE_FAILED,
    EPOS_INITIALIZED,
    EPOS_SERVO_LOOP_STARTED,
    EPOS_SERVO_LOOP_STOPPED,
    EPOS_SERVO_TO_POS_FAILED,
    EPOS_EPOCH_SET,
    EPOS_EPOCH_SET_FAILED,
    EPOS_DISABLE_MOTOR_FAILED,
    EPOS_DISABLE_MOTOR_SUCCESS,
    EPOS_DEVICE_CANT_CONNECT,
    EPOS_UPDATE_QC_FAILED,
    EPOS_HALT_FAILED,
    EPOS_DISCONNECTED,
    EPOS_DISCONNECT_FAILED
};

enum EPOS_DATA_IDS
{
    EPOS_COMMANDED = 0,
    EPOS_READ
};

enum EPOS_MOTOR_STATUS
{
    EPOS_MOTOR_DISABLED = 0,
    EPOS_MOTOR_ENABLED,
    EPOS_MOTOR_FAULT,
    EPOS_MOTOR_CANT_CONNECT,
    EPOS_INVALID_MOTOR_ID
};

// enum EPOS_MOTOR_IDS
// {
//     TRANS_MOTOR_ID = 1,
//     PITCH_MOTOR_ID,
//     YAW_MOTOR_ID,
//     ROLL_MOTOR_ID
// };

static const int EPOS_NUM_MOTORS = 4;

static const int TRANS_MOTOR_ID = 1;
static const int PITCH_MOTOR_ID = 2;
static const int YAW_MOTOR_ID = 3;
static const int ROLL_MOTOR_ID = 4;

static const int EPOS_MOTOR_IDS[EPOS_NUM_MOTORS] = {TRANS_MOTOR_ID, PITCH_MOTOR_ID, YAW_MOTOR_ID, ROLL_MOTOR_ID};

static const int TRANS_AXIS_ID = 0;
static const int PITCH_AXIS_ID = 1;
static const int YAW_AXIS_ID = 2;
static const int ROLL_AXIS_ID = 3;

static const int EPOS_AXIS_IDS[EPOS_NUM_MOTORS] = {TRANS_AXIS_ID, PITCH_AXIS_ID, YAW_AXIS_ID, ROLL_AXIS_ID};

static const int EPOS_VELOCITY = 5000;
static const int EPOS_ACCEL = 8000;
static const int EPOS_DECEL = 8000;

static const long EPOS_TRANS_MIN = -320000;
static const long EPOS_TRANS_MAX = 320000;
static const long EPOS_PITCH_MIN = -73000;
static const long EPOS_PITCH_MAX = 73000;
static const long EPOS_YAW_MIN = -73000;
static const long EPOS_YAW_MAX = 73000;
static const long EPOS_ROLL_MIN = -1381000;
static const long EPOS_ROLL_MAX = 1381000;

static const int EPOS_MOTOR_LIMITS[EPOS_NUM_MOTORS][2] =
                                           { {EPOS_TRANS_MIN, EPOS_TRANS_MAX},
                                             {EPOS_PITCH_MIN, EPOS_PITCH_MAX},
                                             {EPOS_YAW_MIN, EPOS_YAW_MAX},
                                             {EPOS_ROLL_MIN, EPOS_ROLL_MAX}   };

// ***********************
// ******  LabJack  ******
// ***********************

// enum LABJACK_ERROR_CODES ---- use LJ_ERROR codes instead from LabJackUD.h

enum LABJACK_EVENT_IDS
{
    LABJACK_CONNECT_BEGIN = 0,
    LABJACK_CONNECT_FAILED,
    LABJACK_CONNECTED,
    LABJACK_INITIALIZE_BEGIN,
    LABJACK_INITIALIZE_FAILED,
    LABJACK_INITIALIZED,
    LABJACK_LOOP_STARTED,
    LABJACK_LOOP_STOPPED,
    LABJACK_EPOCH_SET,
    LABJACK_EPOCH_SET_FAILED,
    LABJACK_DISCONNECTED,
    LABJACK_DISCONNECT_FAILED
};

// ***********************
// ******  FrmGrab  ******
// ***********************

enum FRMGRAB_ERROR_CODES
{
    FRMGRAB_SUCCESS = 0,     //
    FRMGRAB_FAIL,
    FRMGRAB_CANT_MUTATE_WHILE_RUNNING
};

enum FRMGRAB_EVENT_IDS
{
    FRMGRAB_CONNECT_BEGIN = 0,
    FRMGRAB_CONNECT_FAILED,
    FRMGRAB_CONNECTED,
    FRMGRAB_INITIALIZE_BEGIN,
    FRMGRAB_INITIALIZE_FAILED,
    FRMGRAB_INITIALIZED,
    FRMGRAB_LOOP_STARTED,
    FRMGRAB_LOOP_STOPPED,
    FRMGRAB_LIVE_FEED_STARTED,
    FRMGRAB_LIVE_FEED_STOPPED,
    FRMGRAB_EPOCH_SET,
    FRMGRAB_EPOCH_SET_FAILED,
    FRMGRAB_DISCONNECTED,
    FRMGRAB_DISCONNECT_FAILED
};

static const int FRMGRAB_IM_WIDTH = 1920; // 640;
static const int FRMGRAB_IM_HEIGHT = 1080; // 480;
static const int FRMGRAB_FPS = 60;

// ***********************
// ***** DATA LOGGER *****
// ***********************

static const int DATALOG_NUM_FILES = 7;
static const unsigned short DATALOG_EM_ID = 0;
static const unsigned short DATALOG_ECG_ID = 1;
static const unsigned short DATALOG_EPOS_ID = 2;
static const unsigned short DATALOG_FrmGrab_ID = 3;
static const unsigned short DATALOG_Log_ID = 4;
static const unsigned short DATALOG_Error_ID = 5;
static const unsigned short DATALOG_Note_ID = 6;

static const unsigned short DATALOG_FILE_IDS[DATALOG_NUM_FILES] =
                                                  {DATALOG_EM_ID,
                                                   DATALOG_ECG_ID,
                                                   DATALOG_EPOS_ID,
                                                   DATALOG_FrmGrab_ID,
                                                   DATALOG_Log_ID,
                                                   DATALOG_Error_ID,
                                                   DATALOG_Note_ID};

enum DATALOG_EVENT_IDS
{
    DATALOG_INITIALIZE_BEGIN = 0,
    DATALOG_INITIALIZE_FAILED,
    DATALOG_INITIALIZED,
    DATALOG_FILE_OPENED,
    DATALOG_FILE_CLOSED,
    DATALOG_FILE_ERROR,
    DATALOG_FILE_DATA_LOGGED,
    DATALOG_EM_FILE_OPENED,
    DATALOG_EM_FILE_CLOSED,
    DATALOG_EM_FILE_DATA_LOGGED,
    DATALOG_ECG_FILE_OPENED,
    DATALOG_ECG_FILE_CLOSED,
    DATALOG_ECG_FILE_DATA_LOGGED,
    DATALOG_EPOS_FILE_OPENED,
    DATALOG_EPOS_FILE_CLOSED,
    DATALOG_EPOS_FILE_DATA_LOGGED,
    DATALOG_FRMGRAB_FILE_OPENED,
    DATALOG_FRMGRAB_FILE_CLOSED,
    DATALOG_FRMGRAB_FILE_DATA_LOGGED,
    DATALOG_LOG_FILE_OPENED,
    DATALOG_LOG_FILE_CLOSED,
    DATALOG_LOG_FILE_DATA_LOGGED,
    DATALOG_ERROR_FILE_OPENED,
    DATALOG_ERROR_FILE_CLOSED,
    DATALOG_ERROR_FILE_DATA_LOGGED,
    DATALOG_NOTE_FILE_OPENED,
    DATALOG_NOTE_FILE_CLOSED,
    DATALOG_NOTE_FILE_DATA_LOGGED,
    DATALOG_LOGGING_STARTED,
    DATALOG_LOGGING_STOPPED,
    DATALOG_CLOSED
};

#endif // ICEBOT_DEFINITIONS