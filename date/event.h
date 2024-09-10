/****************************************************************************
 * event.h: DATE event data format
 ***************************************************************************/

#define EVENT_MAJOR_VERSION_NUMBER 3
#define EVENT_MINOR_VERSION_NUMBER 14

/* ========== Data types ========= */
/* These data types can be overridden via compile-time symbols */
#ifndef long32
#define long32 int
#endif
#ifndef long64
#define long64 long long
#endif

/* ========== Definitions for the event header ========== */

/* ------------------------- Header base size ------------------------- */
/* This value must be updated for each change in the eventHeaderStruct  */
/* This has not been made automatic as we want to keep this value under */
/* strict control...                                                    */
#define EVENT_HEAD_BASE_SIZE 80

/* ---------- Event size ---------- */
typedef unsigned long32 eventSizeType;

/* ----------- Magic signature and its byte-swapped version ----------- */
typedef unsigned long32 eventMagicType;
#define EVENT_MAGIC_NUMBER ((eventMagicType)0xDA1E5AFE)
#define EVENT_MAGIC_NUMBER_SWAPPED ((eventMagicType)0xFE5A1EDA)

/* ---------- Header size ---------- */
typedef unsigned long32 eventHeadSizeType;

/* ---------- Unique version identifier ---------- */
#define EVENT_CURRENT_VERSION                                                                      \
  (((EVENT_MAJOR_VERSION_NUMBER << 16) & 0xffff0000) | (EVENT_MINOR_VERSION_NUMBER & 0x0000ffff))
typedef unsigned long32 eventVersionType;

/* ---------- Event type ---------- */
typedef unsigned long32 eventTypeType;
#define START_OF_RUN ((eventTypeType)1)
#define END_OF_RUN ((eventTypeType)2)
#define START_OF_RUN_FILES ((eventTypeType)3)
#define END_OF_RUN_FILES ((eventTypeType)4)
#define START_OF_BURST ((eventTypeType)5)
#define END_OF_BURST ((eventTypeType)6)
#define PHYSICS_EVENT ((eventTypeType)7)
#define CALIBRATION_EVENT ((eventTypeType)8)
#define EVENT_FORMAT_ERROR ((eventTypeType)9)
#define START_OF_DATA ((eventTypeType)10)
#define END_OF_DATA ((eventTypeType)11)
#define SYSTEM_SOFTWARE_TRIGGER_EVENT ((eventTypeType)12)
#define DETECTOR_SOFTWARE_TRIGGER_EVENT ((eventTypeType)13)
#define SYNC_EVENT ((eventTypeType)14)
#define EVENT_TYPE_MIN 1
#define EVENT_TYPE_MAX 14
enum eventTypeEnum {
  startOfRun = START_OF_RUN,
  endOfRun = END_OF_RUN,
  startOfRunFiles = START_OF_RUN_FILES,
  endOfRunFiles = END_OF_RUN_FILES,
  startOfBurst = START_OF_BURST,
  endOfBurst = END_OF_BURST,
  physicsEvent = PHYSICS_EVENT,
  calibrationEvent = CALIBRATION_EVENT,
  formatError = EVENT_FORMAT_ERROR,
  startOfData = START_OF_DATA,
  endOfData = END_OF_DATA,
  systemSoftwareTriggerEvent = SYSTEM_SOFTWARE_TRIGGER_EVENT,
  detectorSoftwareTriggerEvent = DETECTOR_SOFTWARE_TRIGGER_EVENT,
  syncEvent = SYNC_EVENT
};

/* ---------- Run number ---------- */
typedef unsigned long32 eventRunNbType;

/* ---------- The eventId field ---------- */
#define EVENT_ID_BYTES 8
#define EVENT_ID_WORDS ((EVENT_ID_BYTES) >> 2)
typedef unsigned long32 eventIdType[EVENT_ID_WORDS];

/* System attributes assignment */
#define SUPER_EVENT 0x00000010
#define ORIGINAL_EVENT 0x00000200

/* ---------- LDC and GDC identifier ---------- */
typedef unsigned long32 eventHostIdType;
typedef eventHostIdType eventLdcIdType;
typedef eventHostIdType eventGdcIdType;
#define LDC_VOID ((eventHostIdType) - 1) /* Unloaded ID            */
#define GDC_VOID ((eventHostIdType) - 1) /* Unloaded ID            */

/* ---------- Seconds timestamp ----------

   The following definition is in common for 32 and 64 bit machines.
   In both architectures, the field must be loaded into a time_t
   variable before being used. Failure to do so may cause undefined
   results up to the early termination of the process.

   The recommended procedure to use this field is the following:

   #include <time.h>

   time_t t;

   t = eventHeaderStruct.eventTimestampSec;
   cTime( &t ); (or whatever else can be done with a time_t)

   Please note that the available timestamp will wrap sometime
   around Jan 18, 19:14:07, 2038...
*/
typedef unsigned long32 eventTimestampSecType;
typedef unsigned long32 eventTimestampUsecType;

#define EVENT_TRIGGER_PATTERN_BYTES 16
#define EVENT_TRIGGER_PATTERN_WORDS ((EVENT_TRIGGER_PATTERN_BYTES) >> 2)
typedef unsigned long32 eventTriggerPatternType[EVENT_TRIGGER_PATTERN_WORDS];

#define EVENT_DETECTOR_PATTERN_BYTES 4
#define EVENT_DETECTOR_PATTERN_WORDS (EVENT_DETECTOR_PATTERN_BYTES >> 2)
typedef unsigned long32 eventDetectorPatternType[EVENT_DETECTOR_PATTERN_WORDS];

#define ALL_ATTRIBUTE_WORDS 3
typedef unsigned long32 eventTypeAttributeType[ALL_ATTRIBUTE_WORDS];

/* ---------- The event header structure ---------- */
struct eventHeaderStruct {
  eventSizeType eventSize;
  eventMagicType eventMagic;
  eventHeadSizeType eventHeadSize;
  eventVersionType eventVersion;
  eventTypeType eventType;
  eventRunNbType eventRunNb;
  eventIdType eventId;
  eventTriggerPatternType eventTriggerPattern;
  eventDetectorPatternType eventDetectorPattern;
  eventTypeAttributeType eventTypeAttribute;
  eventLdcIdType eventLdcId;
  eventGdcIdType eventGdcId;
  eventTimestampSecType eventTimestampSec;
  eventTimestampUsecType eventTimestampUsec;
};

/* ========== Definitions for the equipment header ========== */
typedef long32 equipmentSizeType;
typedef long32 equipmentTypeType;
typedef long32 equipmentIdType;
typedef eventTypeAttributeType equipmentTypeAttributeType;
typedef long32 equipmentBasicElementSizeType;

struct equipmentHeaderStruct {
  equipmentSizeType equipmentSize;
  equipmentTypeType equipmentType;
  equipmentIdType equipmentId;
  equipmentTypeAttributeType equipmentTypeAttribute;
  equipmentBasicElementSizeType equipmentBasicElementSize;
};