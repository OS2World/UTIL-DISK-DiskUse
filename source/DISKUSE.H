/* Application defines                                                        */

#define szClientClass       "WC_CLIENT"
#define szLegendClass       "WC_LEGEND"
#define szDirlistClass      "WC_DIRLIST"
#define szAllClass          "WC_ALL"
#define szNotebookClass     "NB_CLIENT"

#define idFrameWindow       100
#define idPopup             101

#define DB_RAISED           0x0400
#define DB_DEPRESSED        0x0800

#define DIRECTORYSIZE       255
#define FILESIZE            12

/* Defines controls (ID_)                                                     */

#define ID_TIMER            220
#define ID_LISTBOX          200
#define IDD_REFRESH         210
#define ID_DIRLIST          211
#define ID_LEGEND           212
#define ID_NBWINDOW         213
#define ID_NBSETTINGS       214
#define ID_ALL              215
#define IDD_STOPPRINT       216

/* Defines menu option identifiers (IDM_)                                     */

#define IDM_DISK            3000
#define IDM_GRAPH           3001
#define IDM_REFRESH         3002
#define IDM_SETTINGS        3003
#define IDM_DRIVE           3004
#define IDM_AUTOREF         3005
#define IDM_VIEW            3010
#define IDM_LEGEND          3011
#define IDM_DIRLIST         3012
#define IDM_ALL             3013
#define IDM_FONT            3014
#define IDM_PRINT           3015
#define IDM_MOVE            3201
#define IDM_DELETE          3202
#define IDM_COPY            3203
#define IDM_MAKEROOT        3204
#define IDM_ROOT            3205
#define IDM_HELP            4000
#define IDM_USINGHELP       4001
#define IDM_HELPPRODINFO    4002

#define MENU_DRIVE          500
#define IDM_DRIVE_A         MENU_DRIVE+1
#define IDM_DRIVE_B         MENU_DRIVE+2
#define IDM_DRIVE_C         MENU_DRIVE+3
#define IDM_DRIVE_D         MENU_DRIVE+4
#define IDM_DRIVE_E         MENU_DRIVE+5
#define IDM_DRIVE_F         MENU_DRIVE+6
#define IDM_DRIVE_G         MENU_DRIVE+7
#define IDM_DRIVE_H         MENU_DRIVE+8
#define IDM_DRIVE_I         MENU_DRIVE+9
#define IDM_DRIVE_J         MENU_DRIVE+10
#define IDM_DRIVE_K         MENU_DRIVE+11
#define IDM_DRIVE_L         MENU_DRIVE+12
#define IDM_DRIVE_M         MENU_DRIVE+13
#define IDM_DRIVE_N         MENU_DRIVE+14
#define IDM_DRIVE_O         MENU_DRIVE+15
#define IDM_DRIVE_P         MENU_DRIVE+16
#define IDM_DRIVE_Q         MENU_DRIVE+17
#define IDM_DRIVE_R         MENU_DRIVE+18
#define IDM_DRIVE_S         MENU_DRIVE+19
#define IDM_DRIVE_T         MENU_DRIVE+20
#define IDM_DRIVE_U         MENU_DRIVE+21
#define IDM_DRIVE_V         MENU_DRIVE+22
#define IDM_DRIVE_W         MENU_DRIVE+23
#define IDM_DRIVE_X         MENU_DRIVE+24
#define IDM_DRIVE_Y         MENU_DRIVE+25
#define IDM_DRIVE_Z         MENU_DRIVE+26

/* Defines string identifiers (IDS_)                                          */

#define IDS_TITLEBAR        1350
#define IDS_HELPTITLE       1352
#define IDS_HELPNAME        1353
#define IDS_EXITPROMPT      1356
#define IDS_TITLEEXIT       1351
#define IDS_REFRESH         1355
#define IDS_TITLEREF        1354
#define IDS_OTHER           1357
#define IDS_FREE            1358
#define IDS_LEGEND          1359
#define IDS_DIRLIST         1360
#define IDS_BYTES           1361
#define IDS_KBYTES          1362
#define IDS_MBYTES          1363
#define IDS_FREE_L          1364
#define IDS_TOTAL_L         1365
#define IDS_LISTHDRLEFT     1366
#define IDS_PATIENT         1367
#define IDS_WARNING         1368
#define IDS_THRESFREE       1369
#define IDS_THRESSWAP       1370
#define IDS_COPYTITLE       1371
#define IDS_COPYERROR       1372
#define IDS_MOVETITLE       1373
#define IDS_MOVEERROR       1374
#define IDS_ROOTTITLE       1376
#define IDS_ROOTERROR       1377
#define IDS_MKRTTITLE       1378
#define IDS_MKRTERROR       1379
#define IDS_DIRTITLE        1380
#define IDS_DIRERROR        1381
#define IDS_SELECT          1382
#define IDS_ININAME         1383
#define IDS_DELETETITLE     1384
#define IDS_DELETEWARN      1385
#define IDS_DELETEERROR1    1386
#define IDS_DELETEERROR2    1387
#define IDS_SETTINGS        1389
#define IDS_OPTIONS         1390
#define IDS_GRAPH           1391
#define IDS_LISTHDRRIGHT    1392
#define IDS_ALL             1393
#define IDS_TOTAL           1394
#define IDS_DRIVE           1395
#define IDS_PRINTERROR      1396
#define IDS_ERROR_PRINT_1   1397
#define IDS_ERROR_PRINT_2   1398
#define IDS_ERROR_PRINT_3   1399
#define IDS_ERROR_PRINT_4   1400
#define IDS_PRINT_STATUS    1401
#define IDS_PRINTTEXT       1402
#define IDS_STOPPRINT       1403
#define IDS_FONTTITLE       1404
#define IDS_FONTTEXT        1405
#define IDS_DIRECTORY       1406
#define IDS_DIRSIZE         1407
#define IDS_PERCENT         1408
#define IDS_FILES           1409
#define IDS_ERROR           1410
#define IDS_ERROR_MEMORY    1411
#define IDS_CUMSIZE         1412
#define IDS_PRINTLIST       1413
#define IDS_PRINTLEGEND     1414
#define IDS_PRINTCHART      1415
#define IDS_INSTRUCT        1416
#define IDS_VOLUME          1417

/* Defines help identifiers (IDH_)                                            */

#define IDH_TABLE_MAIN          9999

#define IDH_SUBTABLE_MAIN       9100
#define IDH_MAIN                9110
#define IDH_MENU_DISK           9120
#define IDH_MENU_GRAPH          9121
#define IDH_MENU_SETTINGS       9124
#define IDH_MENU_REFRESH        9122
#define IDH_MENU_AUTOREF        9125
#define IDH_MENU_DRIVE          9126
#define IDH_MENU_VIEW           9130
#define IDH_MENU_LEGEND         9131
#define IDH_MENU_DIRLIST        9132
#define IDH_MENU_ALL            9133
#define IDH_MENU_FONT           9134
#define IDH_FONT_DLG            9135
#define IDH_MENU_PRINT          9136

#define IDH_POPUP_MOVE          9140
#define IDH_POPUP_DELETE        9141
#define IDH_POPUP_COPY          9142
#define IDH_POPUP_MAKEROOT      9143
#define IDH_POPUP_ROOT          9144

#define IDH_REFRESH             9310

#define IDH_MENU_HELP           9180
#define IDH_MENU_HELPINDEX      9182
#define IDH_MENU_HELPGENERAL    9183
#define IDH_MENU_HELPUSING      9184
#define IDH_MENU_HELPKEYS       9185
#define IDH_MENU_HELPPRODINFO   9186
#define IDH_HELPKEYS            9187

#define IDH_SUBTABLE_NOTEBOOK   9400
#define IDH_SETTINGS            9410
#define IDH_SETTINGS_DIRLIST    9411
#define IDH_SETTINGS_LEGEND     9412
#define IDH_SETTINGS_ALL        9418
#define IDH_SETTINGS_EXIT       9413
#define IDH_SETTINGS_REFRESH    9415
#define IDH_SETTINGS_UNITS      9416
#define IDH_SETTINGS_DEFAULT    9414
#define IDH_SETTINGS_TREESHOW   9417
#define IDH_GRAPH               9210
#define IDH_GRAPH_TYPE          9213
#define IDH_GRAPH_LEVEL         9214
#define IDH_GRAPH_DISPLAY       9217
#define IDH_GRAPH_SWAPPER       9219
#define IDH_GRAPH_MAXPIES       9215
#define IDH_GRAPH_DEFAULT       9216
#define IDH_AUTOREF             9710
#define IDH_AUTOREF_INTERVAL    9711
#define IDH_AUTOREF_THRESFREE   9712
#define IDH_AUTOREF_THRESSWAP   9713
#define IDH_AUTOREF_WARNFREE    9714
#define IDH_AUTOREF_WARNSWAP    9715
#define IDH_AUTOREF_NONE        9716

#define IDH_SUBTABLE_REFRESH    9300

#define IDH_DIRLIST             9510
#define IDH_DIRLIST_DIRSIZE     9520
#define IDH_LEGEND              9530
#define IDH_ALL                 9540

#define IDH_SUBTABLE_DIRECTORY  9600
#define IDH_DIRECTORY           9610

#define IDH_SUBTABLE_LEGEND     9650
#define IDH_SUBTABLE_DIRLIST    9651
#define IDH_SUBTABLE_ALL        9652

#define IDH_SUBTABLE_PRINT      9800
#define IDH_PRINT               9810
#define IDH_SUBTABLE_PRINTSTAT  9820
#define IDH_PRINTSTAT           9830
#define IDH_SUBTABLE_STOPPRINT  9840
#define IDH_STOPPRINT           9850

#define TID_LOGOTIMER           TID_USERMAX - 1
#define STACKSIZE               65536
#define UM_LEGEND               WM_USER + 1
#define UM_PAINT                WM_USER + 2
#define UM_WARNING              WM_USER + 3
#define UM_CLOSE                WM_USER + 4
#define UM_DIRLIST              WM_USER + 5
#define UM_SELECT               WM_USER + 6
#define UM_CREATENB             WM_USER + 7
#define UM_AUTOREF              WM_USER + 8
#define UM_LISTBOX              WM_USER + 9
#define UM_REFDIR               WM_USER + 10
#define UM_CREATE               WM_USER + 11
#define UM_ALL                  WM_USER + 12
#define UM_FONT                 WM_USER + 13
#define UM_PRINT                WM_USER + 14
#define UM_STOPPRINT            WM_USER + 15
#define UM_PRINTSTATUS          WM_USER + 16

#define szAppName     "DISKUSE"
#define szGraphDef    "Customize Graph"
#define szSettingsDef "Settings Defaults"
#define szAutorefDef  "Automated Refresh"
#define szWinPos      "Window Position"
#define szLegendPos   "Legend Position"
#define szDirlistPos  "Directory List Position"
#define szAllPos      "All Drives Position"
#define szSetWinPos   "Notebook Window Position"

#define OFF   0
#define ON    1
#define NO    0
#define YES   1
#define LEFT  0
#define RIGHT 1

#define GRAPH_MAX_PIES     50
#define GRAPH_TYPE_TOTAL   0
#define GRAPH_TYPE_USED    1
#define GRAPH_LEVEL_ALL    0
#define GRAPH_LEVEL_1ST    1
#define UNITS_BYTES        0
#define UNITS_KBYTES       1
#define UNITS_MEGS         2
#define GRAPH_SHADE_1      PATSYM_DENSE5
#define GRAPH_SHADE_2      PATSYM_DENSE3
#define GRAPH_SHADE_3      PATSYM_DIAG1
#define GRAPH_SHADE_4      PATSYM_DIAG3
#define GRAPH_SHADE_OTHER  PATSYM_DENSE6
#define GRAPH_SHADE_SWAP   PATSYM_DENSE4

#define GPIFULLARC_MULT_LIMIT 255
#define M_PI                  3.1415927

#define ENABLE_EXCEPTION    0
#define DISABLE_EXCEPTION   2
#define ENABLE_HARDERROR    1
#define DISABLE_HARDERROR   0
#define DISABLE_ERRORPOPUPS DISABLE_EXCEPTION | DISABLE_HARDERROR
#define ENABLE_ERRORPOPUPS  ENABLE_EXCEPTION  | ENABLE_HARDERROR

void topolar( double x, double y, double *r, double *t );

/* Data structures                                                            */

typedef struct
{
  HAB    hab;
  HWND   hwnd;
  LONG   cyChar;
  LONG   cxChar;
  LONG   cyDesc;
  LONG   cxClient;
  LONG   cyClient;
  POINTL ptctr;
  FIXED  radius;
} CHARTINFO;
typedef CHARTINFO * PCHARTINFO;

typedef struct
{
  HAB  hab;
  HWND hwnd;
  HWND hwndVscroll;
  HWND hwndHscroll;
  LONG cxChar;
  LONG cxCaps;
  LONG cyChar;
  LONG cyDesc;
  LONG cyHeight;
  LONG cxClient;
  LONG cyClient;
  LONG cxTextTotal;
  LONG cyLineTotal;
  LONG cyLowHeight;
  LONG sHscrollMax;
  LONG sHscrollPos;
  LONG sVscrollMax;
  LONG sVscrollPos;
} LEGENDINFO;
typedef LEGENDINFO * PLEGENDINFO;

typedef struct
{
  BOOL   boola;
  BOOL   boolb;
  BOOL   boolc;
  BOOL   boold;
  BOOL   boole;
  BOOL   boolf;
  USHORT usa;
  USHORT usb;
  USHORT usc;
  USHORT usd;
  USHORT use;
  USHORT usf;
  ULONG  ula;
  ULONG  ulb;
  ULONG  ulc;
  ULONG  uld;
  ULONG  ule;
  ULONG  ulf;
  CHAR   sza[40];
  CHAR   szb[40];
} PROFILEINFO;
typedef PROFILEINFO * PPROFILEINFO;


typedef struct
{
  MINIRECORDCORE miniRecordCore;
  PSZ            pszDirectory;
  PSZ            pszSize;
  PSZ            pszPercent;
  PSZ            pszCumSize;
  PSZ            pszCumPercent;
  ULONG          ulFiles;
  PDIRINFO       pDirInfo;
} CUSTOBJ;
typedef CUSTOBJ * PCUSTOBJ;

typedef struct
{
  CHAR  searchdir[DIRECTORYSIZE];
  CHAR  directory[DIRECTORYSIZE];
  CHAR  targetdir[DIRECTORYSIZE];
  CHAR  szDirectory[20];
  CHAR  szDirSize[20];
  CHAR  szCumSize[20];
  CHAR  szPercent[2];
  CHAR  szFiles[20];
  CHAR  szFree[10];
  CHAR  szTotal[10];
  CHAR  szPatient[80];
  CHAR  szInstruct[80];
} GLOBSTRINGS;
typedef GLOBSTRINGS * PGLOBSTRINGS;

/* Function prototypes                                                        */

MRESULT EXPENTRY wpClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpClientWndProc, system)
MRESULT EXPENTRY wpLegendWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpLegendWndProc, system)
MRESULT EXPENTRY wpAllWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpAllWndProc, system)
MRESULT EXPENTRY wpDirlistWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpDirlistWndProc, system)
MRESULT EXPENTRY wpNBWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpNBWndProc, system)

MRESULT EXPENTRY wpAboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpAboutDlgProc, system)
MRESULT EXPENTRY wpDirectoryDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpDirectoryProc, system)
MRESULT EXPENTRY wpDetailsDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpDetailsDlgProc, system)
MRESULT EXPENTRY wpPrintDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpPrintDlgProc, system)
MRESULT EXPENTRY wpPrtStatDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpPrtStatDlgProc, system)

BOOL fnCreateNotebook ( HAB hab, HWND hwnd );

VOID fnGraphDefaults  ( VOID );
VOID fnOptionsDefaults( VOID );
VOID fnAutorefDefaults( VOID );

VOID fnToUpperString( char *string );
VOID fnToPolar( float x, float y, float *r, float *t );
VOID fnSubstr( CHAR *source, CHAR *dest, ULONG base, ULONG ofs );

/* Global variables                                                           */

BOOL   showlegend;                     /* legend display, 0=NO, 1=YES         */
BOOL   showdirlist;                    /* directory list display, 0=NO, 1=YES */
BOOL   showall;                        /* all drives summary display, 0/1     */
BOOL   rootdrive;                      /*                                     */
                                       /* CUSTOMIZE GRAPH VARIABLES           */
BOOL   graphtype;                      /* used only (1) or total space (0)    */
BOOL   graphlevel;                     /* 1st level (1) or all directories (0)*/
BOOL   graphpercent;                   /* show (1=YES), don't show (0=NO)     */
BOOL   graphsize;                      /* show (1=YES), don't show (0=NO)     */
USHORT graphmaxpies;                   /* maximum # of pie slices (1-20)      */
BOOL   graphswapsize;                  /* show (1=YES) SWAPPER.DAT separately */
BOOL   graphswapstat;                  /* show SWAPPER.DAT size in status area*/
                                       /* SETTINGS VARIABLES                  */
BOOL   showexit;                       /* exit confirmation, 0=NO, 1=YES      */
BOOL   showrefresh;                    /* refresh confirmation, 0=NO, 1=YES   */
BOOL   initlegend;                     /* show legend initially (1=YES)       */
BOOL   initdirlist;                    /* show directory list initially (1=Y) */
BOOL   initall;                        /* show all drives summary initially   */
USHORT directorytree;                  /* directory tree display (0=LEFT, 1=R)*/
USHORT units;                          /* bytes (0), kbytes (1), MB (2)       */
                                       /* AUTOMATED REFRESH VARIABLES         */
ULONG refreshmin;                      /* refresh interval (minutes)          */
ULONG thresfree;                       /* % threshold of free space           */
ULONG thresswap;                       /* % threshold of SWAPPER.DAT          */
BOOL  alarmfree;                       /* sound alarm if under free space %   */
BOOL  messagefree;                     /* display msg if under free space %   */
BOOL  alarmswap;                       /* sound alarm if over SWAPPER.DAT %   */
BOOL  messageswap;                     /* display msg if over SWAPPER.DAT %   */
CHAR  commandfree[40];                 /* spawned command if under free space */
CHAR  commandswap[40];                 /* spawned command if over SWAPPER.DAT */

CHAR  fontname[FACESIZE];
ULONG fontsize;

ULONG totaldrives;

HEV    hev;                            /* event semaphore handle              */
ULONG  hevcount;                       /* count of event semaphore use        */
HINI   hini;                           /* initialization profile handle       */

HWND  hwndFrame,                       /* main window frame handle            */
      hwndClient,
      hwndLegend,                      /* legend frame window handle          */
      hwndDirlist,                     /* directory list frame window handle  */
      hwndList,                        /* directory list list box handle      */
      hwndHelp,
      hwndAll;
                                       /* globally declared strings           */

BOOL  useProfile;

GLOBSTRINGS globStrings;
