/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* Copyright (c) 2002 Berliner Elektronenspeicherringgesellschaft fuer
*     Synchrotronstrahlung.
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

/*
 *  Author: Ralph Lange (BESSY)
 *
 *  Modification History
 *  2009/03/31 Larry Hoff (BNL)
 *     Added field separators
 *
 */
#ifndef INCLtool_libh
#define INCLtool_libh

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define epicsAlarmGLOBAL
#include <alarm.h>
#undef epicsAlarmGLOBAL
#include <epicsTime.h>
#include <epicsString.h>
#include <cadef.h>

#define TIMETEXTLEN 28          /* Length of timestamp text buffer */

/* Convert status and severity to strings */
#define stat_to_str(stat)                                       \
        ((stat) >= 0 && (stat) <= (signed)lastEpicsAlarmCond) ? \
        epicsAlarmConditionStrings[stat] : "??"

#define sevr_to_str(stat)                                       \
        ((stat) >= 0 && (stat) <= (signed)lastEpicsAlarmSev) ?  \
        epicsAlarmSeverityStrings[stat] : "??"

#define stat_to_str_unsigned(stat)              \
        ((stat) <= lastEpicsAlarmCond) ?        \
        epicsAlarmConditionStrings[stat] : "??"

#define sevr_to_str_unsigned(stat)              \
        ((stat) <= lastEpicsAlarmSev) ?         \
        epicsAlarmSeverityStrings[stat] : "??"

/* The different versions are necessary because stat and sevr are
 * defined unsigned in CA's DBR_STSACK structure and signed in all the
 * others. Some compilers generate warnings if you check an unsigned
 * being >=0 */
static epicsTimeStamp tsStart, tsFirst, tsPreviousC, tsPreviousS;

int enumAsNr = 0;        /* Used for -n option (get DBF_ENUM as number) */

#define DEFAULT_CA_PRIORITY 0  /* Default CA priority */
#define DEFAULT_TIMEOUT 1.0     /* Default CA timeout */

#ifndef _WIN32
#  define LINE_BUFFER(stream) setvbuf(stream, NULL, _IOLBF, BUFSIZ)
#else
/* Windows doesn't support line mode, turn buffering off completely */
#  define LINE_BUFFER(stream) setvbuf(stream, NULL, _IONBF, 0)
#endif


/* Type of timestamp */
typedef enum { absolute, relative, incremental, incrementalByChan } TimeT;

/* Output formats for integer data types */
typedef enum { dec, bin, oct, hex } IntFormatT;

/* Structure representing one PV (= channel) */
typedef struct
  {
    char* name;
    chid  channelid;
    long  dbfType;
    long  dbrType;
    unsigned long nElems;       // True length of data in value
    unsigned long reqElems;     // Requested length of data
    int status;
    void* value;
    epicsTimeStamp tsPreviousC;
    epicsTimeStamp tsPreviousS;
    char firstStampPrinted;
    char onceConnected;
    double ywght;
    std::function<void(const double &ysum)> Update;
  }
pv;


static void sprint_long (char *ret, dbr_long_t val, IntFormatT outType)
{
  if (outType == bin && val != 0)
    {
      /* sprintf doesn't do binary; this code doesn't handle 0 */
      int i, skip = -1;

      for (i = 31; i >= 0; i--)
        {
          int bit = (val >> i) & 1;

          if (skip < 0 && bit)
            {
              skip = 31 - i;          /* skip leading 0's */
              ret[i+1] = '\0';
            }
          if (skip >= 0)
            {
              ret[31-i-skip] = '0' + bit;
            }
        }
    }
  else
    {
      const char *fmt[4] =
        { /* Order must match the enum IntFormatT */
          "%d"   /* dec */,
          "0"    /* bin and val is 0 */,
          "0o%o" /* oct */,
          "0x%X" /* hex */
        };

      /* dbr_long_t is actually an int on all supported platforms */
      sprintf(ret, fmt[outType], (int) val);
    }
}


/*+**************************************************************************
 *
 * Function:	dbr2str
 *
 * Description:	Print (convert) additional information contained in dbr_...
 *
 * Arg(s) In:	value  -  Pointer to dbr_... structure
 *              type   -  Numeric dbr type
 *
 * Return(s):	Pointer to static output string
 *
 **************************************************************************-*/

/* Definitions for sprintf format strings and matching argument lists */

#define FMT_TIME                                \
    "    Timestamp:        %s"

#define ARGS_TIME(T)                            \
    timeText

#define FMT_STS                                 \
    "    Status:           %s\n"                \
    "    Severity:         %s"

#define ARGS_STS(T)                             \
    stat_to_str(((struct T *)value)->status),   \
    sevr_to_str(((struct T *)value)->severity)

#define ARGS_STS_UNSIGNED(T)                    \
    stat_to_str_unsigned(((struct T *)value)->status),  \
    sevr_to_str_unsigned(((struct T *)value)->severity)

#define FMT_ACK                                 \
    "    Ack transient?:   %s\n"                \
    "    Ack severity:     %s"

#define ARGS_ACK(T)                             \
    ((struct T *)value)->ackt ? "YES" : "NO",   \
    sevr_to_str_unsigned(((struct T *)value)->acks)

#define FMT_UNITS                               \
    "    Units:            %s"

#define ARGS_UNITS(T)                           \
    ((struct T *)value)->units

#define FMT_PREC                                \
    "    Precision:        %d"

#define ARGS_PREC(T)                            \
    ((struct T *)value)->precision

#define FMT_GR(FMT)                             \
    "    Lo disp limit:    " #FMT "\n"          \
    "    Hi disp limit:    " #FMT "\n"          \
    "    Lo alarm limit:   " #FMT "\n"          \
    "    Lo warn limit:    " #FMT "\n"          \
    "    Hi warn limit:    " #FMT "\n"          \
    "    Hi alarm limit:   " #FMT

#define ARGS_GR(T,F)                            \
    (F)((struct T *)value)->lower_disp_limit,   \
    (F)((struct T *)value)->upper_disp_limit,   \
    (F)((struct T *)value)->lower_alarm_limit,  \
    (F)((struct T *)value)->lower_warning_limit, \
    (F)((struct T *)value)->upper_warning_limit, \
    (F)((struct T *)value)->upper_alarm_limit

#define FMT_CTRL(FMT)                           \
    "    Lo ctrl limit:    " #FMT "\n"          \
    "    Hi ctrl limit:    " #FMT

#define ARGS_CTRL(T,F)                          \
    (F)((struct T *)value)->lower_ctrl_limit,   \
    (F)((struct T *)value)->upper_ctrl_limit


/* Definitions for the actual sprintf calls */

#define PRN_DBR_STS(T)                          \
    sprintf(str,                                \
            FMT_STS,                            \
            ARGS_STS(T))

#define PRN_DBR_TIME(T)                                         \
    epicsTimeToStrftime(timeText, TIMETEXTLEN, "%Y-%m-%d %H:%M:%S.%06f",   \
                        &(((struct T *)value)->stamp));         \
    sprintf(str,                                                \
            FMT_TIME "\n" FMT_STS,                              \
            ARGS_TIME(T), ARGS_STS(T))

#define PRN_DBR_GR(T,F,FMT)                             \
    sprintf(str,                                        \
            FMT_STS "\n" FMT_UNITS "\n" FMT_GR(FMT),    \
            ARGS_STS(T), ARGS_UNITS(T), ARGS_GR(T,F))

#define PRN_DBR_GR_PREC(T,F,FMT)                                        \
    sprintf(str,                                                        \
            FMT_STS "\n" FMT_UNITS "\n" FMT_PREC "\n" FMT_GR(FMT),      \
            ARGS_STS(T), ARGS_UNITS(T), ARGS_PREC(T), ARGS_GR(T,F))

#define PRN_DBR_CTRL(T,F,FMT)                                                   \
    sprintf(str,                                                                \
            FMT_STS "\n" FMT_UNITS "\n" FMT_GR(FMT) "\n" FMT_CTRL(FMT),         \
            ARGS_STS(T), ARGS_UNITS(T), ARGS_GR(T,F),    ARGS_CTRL(T,F))

#define PRN_DBR_CTRL_PREC(T,F,FMT)                                                      \
    sprintf(str,                                                                        \
            FMT_STS "\n" FMT_UNITS "\n" FMT_PREC "\n" FMT_GR(FMT) "\n" FMT_CTRL(FMT),   \
            ARGS_STS(T), ARGS_UNITS(T), ARGS_PREC(T), ARGS_GR(T,F),    ARGS_CTRL(T,F))

#define PRN_DBR_STSACK(T)                       \
    sprintf(str,                                \
            FMT_STS "\n" FMT_ACK,               \
            ARGS_STS_UNSIGNED(T), ARGS_ACK(T))

#define PRN_DBR_X_ENUM(T)                               \
    n = ((struct T *)value)->no_str;                    \
    PRN_DBR_STS(T);                                     \
    sprintf(str+strlen(str),                            \
                "\n    Enums:            (%2d)", n);    \
    for (i=0; i<n; i++)                                 \
        sprintf(str+strlen(str),                        \
                "\n                      [%2d] %s", i,  \
                ((struct T *)value)->strs[i]);


/* Make a good guess how long the dbr_... stuff might get as worst case */
#define DBR_PRINT_BUFFER_SIZE                                           \
      50                        /* timestamp */                         \
    + 2 * 30                    /* status / Severity */                 \
    + 2 * 30                    /* acks / ackt */                       \
    + 20 + MAX_UNITS_SIZE       /* units */                             \
    + 30                        /* precision */                         \
    + 6 * 45                    /* graphic limits */                    \
    + 2 * 45                    /* control limits */                    \
    + 30 + (MAX_ENUM_STATES * (20 + MAX_ENUM_STRING_SIZE)) /* enums */  \
    + 50                        /* just to be sure */


/*+**************************************************************************
 *
 * Function:	print_time_val_sts
 *
 * Description:	Print (to stdout) one wide output line
 *              (name, timestamp, value, status, severity)
 *
 * Arg(s) In:	pv      -  Pointer to pv structure
 *              nElems  -  Number of elements (array)
 *
 **************************************************************************-*/

#define PRN_TIME_VAL_STS(TYPE,TYPE_ENUM)                                \
    printAbs = !pv->firstStampPrinted;                                  \
                                                                        \
    ptsNewS = &((struct TYPE *)value)->stamp;                           \
    ptsNewC = &tsNow;                                                   \
                                                                        \
    switch (tsType) {                                                   \
    case relative:                                                      \
        ptsRefC = &tsStart;                                             \
        ptsRefS = &tsFirst;                                             \
        break;                                                          \
    case incremental:                                                   \
        ptsRefC = &tsPreviousC;                                         \
        ptsRefS = &tsPreviousS;                                         \
        break;                                                          \
    case incrementalByChan:                                             \
        ptsRefC = &pv->tsPreviousC;                                     \
        ptsRefS = &pv->tsPreviousS;                                     \
        break;                                                          \
    default :                                                           \
        printAbs = 1;                                                   \
    }                                                                   \
                                                                        \
    if (printAbs) {                                                     \
        if (tsSrcServer) {                                              \
            epicsTimeToStrftime(timeText, TIMETEXTLEN, "%Y-%m-%d %H:%M:%S.%06f", ptsNewS); \
            printf("%s", timeText);                                     \
        }                                                               \
        if (tsSrcClient) {                                              \
            epicsTimeToStrftime(timeText, TIMETEXTLEN, "%Y-%m-%d %H:%M:%S.%06f", ptsNewC); \
            printf("(%s)", timeText);                                   \
        }                                                               \
        pv->firstStampPrinted = 1;                                      \
    } else {                                                            \
        if (tsSrcServer) {                                              \
            printf("              %+12.6f", epicsTimeDiffInSeconds(ptsNewS, ptsRefS) ); \
        }                                                               \
        if (tsSrcClient) {                                              \
            printf("              (%+12.6f)", epicsTimeDiffInSeconds(ptsNewC, ptsRefC) ); \
        }                                                               \
    }                                                                   \
                                                                        \
    if (tsType == incrementalByChan) {                                  \
        pv->tsPreviousC = *ptsNewC;                                     \
        pv->tsPreviousS = *ptsNewS;                                     \
    }                                                                   \
                                                                        \
    tsPreviousC = *ptsNewC;                                             \
    tsPreviousS = *ptsNewS;                                             \
                                                                        \
    if (charArrAsStr && dbr_type_is_CHAR(TYPE_ENUM) && (reqElems || pv->nElems > 1)) { \
        dbr_char_t *s = (dbr_char_t*) dbr_value_ptr(pv->value, pv->dbrType); \
        size_t len = strlen((char*)s);                                  \
        unsigned long elems = reqElems && (reqElems < pv->nElems) ? reqElems : pv->nElems; \
        int dlen;                                                       \
        char *d;                                                        \
        if (len < elems) elems = len;                                   \
        dlen = epicsStrnEscapedFromRawSize((char*)s, elems);            \
        d = (char*)calloc(dlen+1, sizeof(char));                        \
        if(d) {                                                         \
            epicsStrnEscapedFromRaw(d, dlen+1, (char*)s, elems);        \
            printf("%c%s", fieldSeparator, d);                          \
            free(d);                                                    \
        } else {                                                        \
            printf("Failed to allocate for print_time_val_sts\n");      \
        }                                                               \
    } else {                                                            \
        if (reqElems || pv->nElems > 1) printf("%c%lu", fieldSeparator, pv->nElems); \
        for (i=0; i<(int)pv->nElems; ++i) {                             \
            printf("%c%s", fieldSeparator, val2str(value, TYPE_ENUM, i)); \
        }                                                               \
    }                                                                   \
                             /* Print Status, Severity - if not NO_ALARM */ \
    if ( ((struct TYPE *)value)->status || ((struct TYPE *)value)->severity ) \
    {                                                                   \
        printf("%c%s%c%s\n",                                            \
               fieldSeparator,                                          \
               stat_to_str(((struct TYPE *)value)->status),             \
               fieldSeparator,                                          \
               sevr_to_str(((struct TYPE *)value)->severity));          \
    } else {                                                            \
        printf("%c%c\n",                                                \
               fieldSeparator, fieldSeparator);                         \
    }


class EventHandler
  {
  public:
    EventHandler()
    {
      tsInitS = 0;              /* Flag: Server timestamps init'd */
      tsInitC = 0;              /* Flag: Client timestamps init'd */

      tsType = absolute;        /* Timestamp type flag (-t option) */
      tsSrcServer = 1;          /* Timestamp source flag (-t option) */
      tsSrcClient = 0;          /* Timestamp source flag (-t option) */
      outTypeI = dec;           /* For -0.. output format option */
      outTypeF = dec;           /* For -l.. output format option */

      fieldSeparator = ' ';     /* OFS default is whitespace */

      charArrAsStr = 0;    /* used for -S option - treat char array as (long) string */
      caTimeout = 1.0;  /* wait time default (see -w option) */
      caPriority = DEFAULT_CA_PRIORITY;  /* CA Priority */
    };

    int tsInitS;              /* Flag: Server timestamps init'd */
    int tsInitC;              /* Flag: Client timestamps init'd */

    TimeT tsType;        /* Timestamp type flag (-t option) */
    int tsSrcServer;     /* Timestamp source flag (-t option) */
    int tsSrcClient;     /* Timestamp source flag (-t option) */
    IntFormatT outTypeI; /* Flag used for -0.. output format option */
    IntFormatT outTypeF; /* Flag used for -l.. output format option */
    int charArrAsStr;    /* used for -S option - treat char array as (long) string */
    double caTimeout;    /* Wait time default (see -w option) */
    char fieldSeparator; /* Output field separator */
    capri caPriority;    /* CA priority */

    int create_pvs ( pv *pvs, int nPvs, caCh *pCB );
    int connect_pvs (pv* pvs, int nPvs);
    static void event_handler(evargs args);
    static void pvget_connection_handler(struct connection_handler_args args);
    static void pvput_connection_handler(struct connection_handler_args args);

    char *val2str (const void *v, unsigned type, int index);
    char *dbr2str (const void *value, unsigned type);
    void print_time_val_sts(pv *pv, unsigned long reqElems);
  };

static unsigned long reqElems = 0;
static unsigned long eventMask = DBE_VALUE | DBE_ALARM;
static int floatAsString = 0;
static int nConn = 0;

/*+**************************************************************************
 *
 * Function:	val2str
 *
 * Description:	Print (convert) value to a string
 *
 * Arg(s) In:	v      -  Pointer to dbr_... structure
 *              type   -  Numeric dbr type
 *              index  -  Index of element to print (for arrays)
 *
 * Return(s):	Pointer to static output string
 *
 **************************************************************************-*/

char *EventHandler::val2str (const void *v, unsigned type, int index)
{
#define STR 500
  static char str[STR];
  char ch;
  void *val_ptr;
  unsigned base_type;
  dbr_long_t val_long;

  if (!dbr_type_is_valid(type))
    {
      strcpy (str, "*** invalid type");
      return str;
    }
  strcpy (str, "!!!");

  base_type = type % (LAST_TYPE+1);

  if (type == DBR_STSACK_STRING || type == DBR_CLASS_NAME)
    base_type = DBR_STRING;

  val_ptr = dbr_value_ptr(v, type);

  switch (base_type)
    {
    case DBR_STRING:
      epicsStrnEscapedFromRaw(str, STR, ((dbr_string_t*) val_ptr)[index], strlen(((dbr_string_t*) val_ptr)[index]));
      break;
    case DBR_FLOAT:
      if (outTypeF == dec)
        {
          sprintf(str, "%g", ((dbr_float_t*) val_ptr)[index]);
        }
      else
        {
          if (((dbr_float_t*) val_ptr)[index] > 0.0)
            val_long = ((dbr_float_t*) val_ptr)[index] + 0.5;
          else
            val_long = ((dbr_float_t*) val_ptr)[index] - 0.5;
          sprint_long(str, val_long, outTypeF);
        }
      break;
    case DBR_DOUBLE:
      if (outTypeF == dec)
        {
          sprintf(str, "%g", ((dbr_double_t*) val_ptr)[index]);
        }
      else
        {
          if (((dbr_double_t*) val_ptr)[index] > 0.0)
            val_long = ((dbr_double_t*) val_ptr)[index] + 0.5;
          else
            val_long = ((dbr_double_t*) val_ptr)[index] - 0.5;
          sprint_long(str, val_long, outTypeF);
        }
      break;
    case DBR_CHAR:
      ch = ((dbr_char_t*) val_ptr)[index];
      sprintf(str, "%d", ch);
      break;
    case DBR_INT:
      sprint_long(str, ((dbr_int_t*) val_ptr)[index], outTypeI);
      break;
    case DBR_LONG:
      sprint_long(str, ((dbr_long_t*) val_ptr)[index], outTypeI);
      break;
    case DBR_ENUM:
      {
        dbr_enum_t *val = (dbr_enum_t *)val_ptr;
        if (dbr_type_is_GR(type) && !enumAsNr)
          {
            if (val[index] >= MAX_ENUM_STATES)
              sprintf(str, "Illegal Value (%d)", val[index]);
            else if (val[index] >= ((struct dbr_gr_enum *)v)->no_str)
              sprintf(str, "Enum Index Overflow (%d)", val[index]);
            else
              sprintf(str, "%s", ((struct dbr_gr_enum *)v)->strs[val[index]]);
          }
        else if (dbr_type_is_CTRL(type) && !enumAsNr)
          {
            if (val[index] >= MAX_ENUM_STATES)
              sprintf(str, "Illegal Value (%d)", val[index]);
            else if (val[index] >= ((struct dbr_ctrl_enum *)v)->no_str)
              sprintf(str, "Enum Index Overflow (%d)", val[index]);
            else
              sprintf(str, "%s", ((struct dbr_ctrl_enum *)v)->strs[val[index]]);
          }
        else
          sprintf(str, "%d", val[index]);
      }
    }
  return str;
}


char *EventHandler::dbr2str (const void *value, unsigned type)
{
  static char str[DBR_PRINT_BUFFER_SIZE];
  char timeText[TIMETEXTLEN];
  int n, i;

  switch (type)
    {
    case DBR_STRING:   /* no additional information for basic data types */
    case DBR_INT:
    case DBR_FLOAT:
    case DBR_ENUM:
    case DBR_CHAR:
    case DBR_LONG:
    case DBR_DOUBLE:
      break;

    case DBR_CTRL_STRING:       /* see db_access.h: not implemented */
    case DBR_GR_STRING:         /* see db_access.h: not implemented */
    case DBR_STS_STRING:
      PRN_DBR_STS(dbr_sts_string);
      break;
    case DBR_STS_SHORT:
      PRN_DBR_STS(dbr_sts_short);
      break;
    case DBR_STS_FLOAT:
      PRN_DBR_STS(dbr_sts_float);
      break;
    case DBR_STS_ENUM:
      PRN_DBR_STS(dbr_sts_enum);
      break;
    case DBR_STS_CHAR:
      PRN_DBR_STS(dbr_sts_char);
      break;
    case DBR_STS_LONG:
      PRN_DBR_STS(dbr_sts_long);
      break;
    case DBR_STS_DOUBLE:
      PRN_DBR_STS(dbr_sts_double);
      break;

    case DBR_TIME_STRING:
      PRN_DBR_TIME(dbr_time_string);
      break;
    case DBR_TIME_SHORT:
      PRN_DBR_TIME(dbr_time_short);
      break;
    case DBR_TIME_FLOAT:
      PRN_DBR_TIME(dbr_time_float);
      break;
    case DBR_TIME_ENUM:
      PRN_DBR_TIME(dbr_time_enum);
      break;
    case DBR_TIME_CHAR:
      PRN_DBR_TIME(dbr_time_char);
      break;
    case DBR_TIME_LONG:
      PRN_DBR_TIME(dbr_time_long);
      break;
    case DBR_TIME_DOUBLE:
      PRN_DBR_TIME(dbr_time_double);
      break;

    case DBR_GR_CHAR:
      PRN_DBR_GR(dbr_gr_char, char, %8d);
      break;
    case DBR_GR_INT:
      PRN_DBR_GR(dbr_gr_int,  int,  %8d);
      break;
    case DBR_GR_LONG:
      PRN_DBR_GR(dbr_gr_long, long int, %8ld);
      break;
    case DBR_GR_FLOAT:
      PRN_DBR_GR_PREC(dbr_gr_float,  float, %g);
      break;
    case DBR_GR_DOUBLE:
      PRN_DBR_GR_PREC(dbr_gr_double, double, %g);
      break;
    case DBR_GR_ENUM:
      PRN_DBR_X_ENUM(dbr_gr_enum);
      break;
    case DBR_CTRL_CHAR:
      PRN_DBR_CTRL(dbr_ctrl_char,   char,     %8d);
      break;
    case DBR_CTRL_INT:
      PRN_DBR_CTRL(dbr_ctrl_int,    int,      %8d);
      break;
    case DBR_CTRL_LONG:
      PRN_DBR_CTRL(dbr_ctrl_long,   long int, %8ld);
      break;
    case DBR_CTRL_FLOAT:
      PRN_DBR_CTRL_PREC(dbr_ctrl_float,  float,  %g);
      break;
    case DBR_CTRL_DOUBLE:
      PRN_DBR_CTRL_PREC(dbr_ctrl_double, double, %g);
      break;
    case DBR_CTRL_ENUM:
      PRN_DBR_X_ENUM(dbr_ctrl_enum);
      break;
    case DBR_STSACK_STRING:
      PRN_DBR_STSACK(dbr_stsack_string);
      break;
    default :
      strcpy (str, "can't print data type");
    }
  return str;
}


void EventHandler::print_time_val_sts(pv* pv, unsigned long reqElems)
{
  char timeText[2*TIMETEXTLEN+2];
  int i, printAbs;
  void* value = pv->value;
  epicsTimeStamp *ptsRefC, *ptsRefS;  /* Reference timestamps (client, server) */
  epicsTimeStamp *ptsNewC, *ptsNewS;  /* Update timestamps (client, server) */
  epicsTimeStamp tsNow;

  epicsTimeGetCurrent(&tsNow);
  epicsTimeToStrftime(timeText, TIMETEXTLEN, "%Y-%m-%d %H:%M:%S.%06f", &tsNow);

  if (!tsInitS)
    {
      tsFirst = tsNow;
      tsInitS = 1;
    }

  if (pv->nElems <= 1 && fieldSeparator == ' ')
    printf("%-30s", pv->name);
  else
    printf("%s", pv->name);
  printf("%c", fieldSeparator);
  if (!pv->onceConnected)
    printf("*** Not connected (PV not found)\n");
  else if (pv->status == ECA_DISCONN)
    printf("%s *** disconnected\n", timeText);
  else if (pv->status == ECA_NORDACCESS)
    printf("%s *** no read access\n", timeText);
  else if (pv->status != ECA_NORMAL)
    printf("%s *** CA error %s\n", timeText, ca_message(pv->status));
  else if (pv->value == 0)
    printf("%s *** no data available (timeout)\n", timeText);
  else
    switch (pv->dbrType)
      {
      case DBR_TIME_STRING:
        PRN_TIME_VAL_STS(dbr_time_string, DBR_TIME_STRING);
        break;
      case DBR_TIME_SHORT:
        PRN_TIME_VAL_STS(dbr_time_short, DBR_TIME_SHORT);
        break;
      case DBR_TIME_FLOAT:
        PRN_TIME_VAL_STS(dbr_time_float, DBR_TIME_FLOAT);
        break;
      case DBR_TIME_ENUM:
        PRN_TIME_VAL_STS(dbr_time_enum, DBR_TIME_ENUM);
        break;
      case DBR_TIME_CHAR:
        PRN_TIME_VAL_STS(dbr_time_char, DBR_TIME_CHAR);
        break;
      case DBR_TIME_LONG:
        PRN_TIME_VAL_STS(dbr_time_long, DBR_TIME_LONG);
        break;
      case DBR_TIME_DOUBLE:
        PRN_TIME_VAL_STS(dbr_time_double, DBR_TIME_DOUBLE);
        break;
      default:
        printf("can't print data type\n");
      }
}


/*+**************************************************************************
 *
 * Function:	create_pvs
 *
 * Description:	Creates an arbitrary number of PVs
 *
 * Arg(s) In:	pvs   -  Pointer to an array of pv structures
 *              nPvs  -  Number of elements in the pvs array
 *              pCB   -  Connection state change callback
 *
 * Arg(s) Out:	none
 *
 * Return(s):	Error code:
 *                  0  -  All PVs created
 *                  1  -  Some PV(s) not created
 *
 **************************************************************************-*/

int EventHandler::create_pvs (pv* pvs, int nPvs, caCh *pCB)
{
  int n;
  int result;
  int returncode = 0;

  if (!tsInitC)                /* Initialize start timestamp */
    {
      epicsTimeGetCurrent(&tsStart);
      tsInitC = 1;
    }
  /* Issue channel connections */
  for (n = 0; n < nPvs; n++)
    {
      result = ca_create_channel (pvs[n].name,
                                  pCB,
                                  &pvs[n],
                                  caPriority,
                                  &pvs[n].channelid);
      if (result != ECA_NORMAL)
        {
          fprintf(stderr, "CA error %s occurred while trying "
                  "to create channel '%s'.\n", ca_message(result), pvs[n].name);
          pvs[n].status = result;
          returncode = 1;
        }
    }

  return returncode;
}


/*+**************************************************************************
 *
 * Function:    connect_pvs
 *
 * Description: Connects an arbitrary number of PVs
 *
 * Arg(s) In:   pvs   -  Pointer to an array of pv structures
 *              nPvs  -  Number of elements in the pvs array
 *
 * Arg(s) Out:  none
 *
 * Return(s):   Error code:
 *                  0  -  All PVs connected
 *                  1  -  Some PV(s) not connected
 *
 **************************************************************************-*/

int EventHandler::connect_pvs (pv* pvs, int nPvs)
{
  int returncode = create_pvs ( pvs, nPvs, 0);
  if ( returncode == 0 )
    {
      /* Wait for channels to connect */
      int result = ca_pend_io (caTimeout);
      if (result == ECA_TIMEOUT)
        {
          if (nPvs > 1)
            {
              fprintf(stderr, "Channel connect timed out: some PV(s) not found.\n");
            }
          else
            {
              fprintf(stderr, "Channel connect timed out: '%s' not found.\n", pvs[0].name);
            }
          returncode = 1;
        }
    }
  return returncode;
}

void EventHandler::event_handler(evargs args)
{
  const unsigned int pvoffset = 2;

  pv* pv_ = (pv*)args.usr;

  pv_->status = args.status;
  if (args.status == ECA_NORMAL)
    {
      pv_->dbrType = args.type;
      pv_->nElems  = args.count;
      pv_->value   = (void*)args.dbr;

      double ysum = 0.;
      for (unsigned int index=pvoffset; index<pv_->nElems+pvoffset; ++index)
        ysum += ((dbr_double_t*)pv_->value)[index];

      pv_->value = NULL;
      pv_->Update(ysum * pv_->ywght);
    }
}


void EventHandler::pvget_connection_handler(struct connection_handler_args args)
{
  pv *ppv = (pv*)ca_puser(args.chid);
  if (args.op == CA_OP_CONN_UP)
    {
      nConn++;
      if (!ppv->onceConnected)
        {
          ppv->onceConnected = 1;

          /* Get natural type and array count */
          ppv->dbfType = ca_field_type(ppv->channelid);
          ppv->dbrType = dbf_type_to_DBR_TIME(ppv->dbfType);
          if (dbr_type_is_ENUM(ppv->dbrType))
            {
              if (enumAsNr)
                ppv->dbrType = DBR_TIME_INT;
              else
                ppv->dbrType = DBR_TIME_STRING;
            }
          else if (floatAsString && (dbr_type_is_FLOAT(ppv->dbrType) || dbr_type_is_DOUBLE(ppv->dbrType)))
            {
              ppv->dbrType = DBR_TIME_STRING;
            }

          /* Set request count */
          ppv->nElems   = ca_element_count(ppv->channelid);
          ppv->reqElems = reqElems > ppv->nElems ? ppv->nElems : reqElems;
          ppv->status = ca_create_subscription(ppv->dbrType, ppv->reqElems, ppv->channelid, eventMask, event_handler, (void*)ppv, NULL);
        }
    }
  else if ( args.op == CA_OP_CONN_DOWN )
    {
      nConn--;
      ppv->status = ECA_DISCONN;
      //EventHandler::print_time_val_sts(ppv, reqElems);
    }
}


void EventHandler::pvput_connection_handler(struct connection_handler_args args)
{
  pv *ppv = (pv*)ca_puser(args.chid);
  if (args.op == CA_OP_CONN_UP)
    {
      nConn++;
      if (!ppv->onceConnected)
        {
          ppv->onceConnected = 1;

          /* Get natural type and array count */
          ppv->dbfType = ca_field_type(ppv->channelid);
          ppv->dbrType = dbf_type_to_DBR_TIME(ppv->dbfType);
          if (dbr_type_is_ENUM(ppv->dbrType))
            {
              if (enumAsNr)
                ppv->dbrType = DBR_TIME_INT;
              else
                ppv->dbrType = DBR_TIME_STRING;
            }
          else if (floatAsString && (dbr_type_is_FLOAT(ppv->dbrType) || dbr_type_is_DOUBLE(ppv->dbrType)))
            {
              ppv->dbrType = DBR_TIME_STRING;
            }

          /* Set request count */
          ppv->nElems   = ca_element_count(ppv->channelid);
          ppv->reqElems = reqElems > ppv->nElems ? ppv->nElems : reqElems;
          ppv->status = ca_create_subscription(ppv->dbrType, ppv->reqElems, ppv->channelid, eventMask, event_handler, (void*)ppv, NULL);
        }
    }
  else if ( args.op == CA_OP_CONN_DOWN )
    {
      nConn--;
      ppv->status = ECA_DISCONN;
      //EventHandler::print_time_val_sts(ppv, reqElems);
    }
}

#endif /* ifndef INCLtool_libh */
