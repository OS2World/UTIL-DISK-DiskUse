/* History: 26Aug94 - Added new sz*[x] members in DIRINFO structure           */
/* Structure definitions                                                      */

#define VOLUME_LENGTH  12

typedef struct
{                                       /* linked list of valid drives        */
  SHORT drivenumber;                    /* drive number (ie 3 for C, 4=D)     */
  CHAR  driveletter[3];                 /* drive letter as a string           */
  CHAR  volume[VOLUME_LENGTH+1];        /* volume label                       */
  ULONG totalsize;                      /* total size of drive                */
  ULONG free;                           /* total free space                   */
  BOOL  ignore;                         /* ignore drive, no drive info found  */
  PVOID next;                           /* next item in linked list           */
} DRIVES;
typedef DRIVES * PDRIVES;

PDRIVES drives;

typedef struct
{
  ULONG  id;                           /* drive identifier                    */
  ULONG  sector;                       /* number of sectors                   */
  ULONG  unit;                         /* number of units                     */
  ULONG  avail;                        /* sectors/units available             */
  USHORT bytesnum;                     /*                                     */
} DRIVEINFO;
typedef DRIVEINFO * PDRIVEINFO;

typedef struct
{
  ULONG  VolumeSerialNum;              /* serial number                       */
  BYTE   volumelength;                 /* length of volume label              */
  CHAR   volumelabel[VOLUME_LENGTH+1]; /* volume label                        */
} DRIVEVOLSER;
typedef DRIVEVOLSER * PDRIVEVOLSER;

typedef struct                  /* this is main linked list (double) contains */
{                               /* directory information in its elements      */
  CHAR   dirname[255];          /* directory tree name */
  CHAR   fulldirname[255];      /* full (path) directory name */
  CHAR   directory[255];        /* actual directory name */
  CHAR   listtext[80];          /*                            */
  CHAR   szSize[11];            /* 082694 */
  CHAR   szPercent[6];          /* 082694 */
  CHAR   szCumSize[11];         /* 082694 */
  CHAR   szCumPercent[6];       /* 082694 */
  ULONG  bytes;                 /* size in bytes */
  float  percent1;              /* % of total disk */
  float  percent2;              /* % of used       */
  ULONG  files;                 /* number of files in subdirectory */
  ULONG  cumbytes;              /* cumulative size in bytes */
  float  cumpercent1;
  float  cumpercent2;
  float  angle;
  USHORT level;                 /* level, 0=root */
  PVOID  next;                  /* pointers to previous and next member of    */
  PVOID  prior;                 /* the double link list                       */
  PVOID  nextsort;              /* sorted, all directories                    */
  PVOID  priorsort;
  PVOID  nextsort1st;           /* sorted, 1st level directories only         */
  PVOID  priorsort1st;
} DIRINFO;
typedef DIRINFO * PDIRINFO;

typedef struct                         /* not really important, this is just  */
{                                      /* a working linked list used by one   */
  CHAR  dirname[255];                  /* of the routines, fnGetDirectoryInfo */
  PVOID next;
} SUBDIRINFO;
typedef SUBDIRINFO * PSUBDIRINFO;

typedef struct
{
  CHAR  fulldirname[255];      /* full (path) directory name */
  ULONG bytes;                 /* size in bytes */
  ULONG files;                 /* number of files in subdirectory */
  PVOID next;
  PVOID prior;
} DELDIRINFO;
typedef DELDIRINFO * PDELDIRINFO;

typedef struct
{
  CHAR  dirname[255];
  PVOID next;
} DELSUBDIRINFO;
typedef DELSUBDIRINFO * PDELSUBDIRINFO;

/* Global variables                                                           */

PDIRINFO start;                        /* starting and ending address for     */
PDIRINFO last;                         /* double linked list contain directory*/
PDIRINFO startsort;                    /* information                         */
PDIRINFO lastsort;
PDIRINFO startsort1st;
PDIRINFO lastsort1st;
PDELDIRINFO delstart;
PDELDIRINFO dellast;

ULONG totaldirectories;                /* total number of subdirectories      */
ULONG total1stdirectories;             /* total number of 1st level direct.   */
ULONG chardirectories;
ULONG char1stdirectories;

ULONG allocunits;                      /* allocation units                    */
ULONG totalfiles;                      /* total number of files               */
ULONG totalsize;                       /* total disk size (bytes)             */
ULONG totalfree;                       /* free space (bytes)                  */
ULONG totalused;                       /* total used (bytes)                  */
ULONG swapsize;                        /* SWAPPER.DAT size (bytes)            */
UCHAR Drive[3];                        /* Drive letter (character string)     */
ULONG DriveNumber;                     /* Drive number (C=3, D=4, E=5, etc.)  */

/* Function prototypes                                                        */

USHORT   fnGetDriveInfo( ULONG DriveNumber, ULONG *size, ULONG *free,
                         ULONG *used, ULONG *alloc, ULONG *swapsize,
                         CHAR  *volume );
VOID     fnGetDirectoryInfo( UCHAR *fulldirname, UCHAR *dirname, UCHAR *prefix,
                             ULONG size, ULONG used, ULONG allocunits,
                             USHORT level, ULONG *cumbytes, BOOL lastdir,
                             BOOL swap, ULONG *swapsize );
PDRIVES  fnGetValidDrives( VOID );
PDIRINFO fnPutLinkListSorted( PDIRINFO item, PDIRINFO top );
PDIRINFO fnPutLinkList1stSorted( PDIRINFO item, PDIRINFO top );
VOID     fnPutLinkList( PDIRINFO item );
VOID     fnFreeList( VOID );
APIRET   fnGetDeleteDirectoryInfo( UCHAR *fulldirname );
VOID     fnPutDeleteList( PDELDIRINFO item );
VOID     fnFreeDeleteList( VOID );
APIRET   fnDeleteDirectories( VOID );
