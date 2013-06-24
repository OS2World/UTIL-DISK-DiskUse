/******************************************************************************/
/*                       (c) IBM Canada Ltd. 1993                             */
/*                                                                            */
/* Program:     DISKUSE - PM Graphical Disk Usage                             */
/* Description: Display hard disk usage in a colour pie chart with legend     */
/*              and directory list containing size, percent and number of     */
/*              files. There are four windows: Main (containing pie chart),   */
/*              Legend, Directory List and All Drives Information.            */
/*                                                                            */
/* Author : Enrico Zapanta                                                    */
/* Date   : 07Nov92                                                           */
/* Version: 1.5                                                               */
/*                                                                            */
/* History: 16Dec92  BETA  Beta release (V 0.1)                               */
/*          03Jan93  0.2   Scrollable legend window, display percentages in   */
/*                         pie chart, Settings dialog window, fix timing      */
/*                         problem by using WinSendMsg and creating message   */
/*                         queue in fnListBoxEntry thread                     */
/*          10Jan93  0.3   Add diskette drives (A: and B:), bug fixes and     */
/*                         other suggestions by Victor Bocking (01/05/93).    */
/*          17Jan93  1.0   General availability on OS2TOOLS                   */
/*          19Jan93  1.01  Fix bug reading diskette drive > C:                */
/*          24Jan93  1.1   Improve multi-threading when reading drive and     */
/*                           drawing pie chart/legend.                        */
/*                         Save and restore previous window positions.        */
/*                         Implement automated refresh at customized intervals*/
/*                           with warnings.                                   */
/*                         Changes to Options/Settings dialogs.               */
/*                         Allow display of % or actual size in graph.        */
/*                         Support SWAPPER.DAT size display.                  */
/*          05Feb93  1.11  Fix timer bug, WinStartTimer maximum timer value   */
/*                           is 65545 ms (or about 1 minute). Use             */
/*                           refreshtimecount, remove seconds in refresh int. */
/*          06Feb93  1.12  Add support for warning spawned command.           */
/*          12Feb93  1.13  Fix bug when displaying SWAPPER.DAT size on the    */
/*                           status area only. Put SWAPPER.DAT information    */
/*                           on line above the status area.                   */
/*                         Increase the number of allowable pie slices to 50. */
/*                         Add customizable refresh confirmation prompt in    */
/*                           Settings.                                        */
/*          16Feb93  1.2   Fix bug when displaying SWAPPER.DAT size.          */
/*                         Set windows active when one window is activated.   */
/*                         New Directory List popup menu: Move, Delete, Copy, */
/*                           Make root, Disk root.                            */
/*                         New Directory Details dialog box when double click */
/*                           on pie slice in main window.                     */
/*                         Use DISKUSE.INI as profile initialization file.    */
/*          28Feb93  1.22  Bug fixes for:                                     */
/*                           In WM_TIMER, Add refreshtime = refreshmin;       */
/*                           Pie slice shading, change shade = -15; to        */
/*                             shade -i; in fnDrawChart and fnDrawLegend.     */
/*                           Clean application bitmap and icon.               */
/*          13Mar93  1.23  Add drive letter to the Directory List window.     */
/*                         Minimize/Restore the Directory List and Legend     */
/*                           window when main window minimized/restored.      */
/*                         Option to display directory tree on right.         */
/*                         Bug fixes for:                                     */
/*                           Do not draw pie chart when window is very small. */
/*                           Ensure main window active upon creation.         */
/*          18Mar93  1.24  Bug fixes for:                                     */
/*                           DISKPROC - add FILE_xxxxxx flags to DosFindFirst */
/*                           Use float for threshold limit comparison         */
/*          10May93  1.3   Use Notebook control for all settings dialogs      */
/*                         New command line parameters: /P, /D and /L         */
/*          02Jul93  1.4   Compile using OS/2 ToolKit Version 2.1             */
/*                         Change Select Drive dialog to Drives menu option   */
/*                         New All Drives Summary window, /A option           */
/*                         Use single set of radio buttons for Graph display  */
/*          23Aug94  1.5   Add full print support.                            */
/*                         Add font support (non-proportional fonts only).    */
/*                         Change listbox to a container (details view only). */
/*                         3D bottom status area.                             */
/*          02Sep94  1.51  Add volume label to All Drives Info window.        */
/*                         Double click in Directory List window - Make Root. */
/*                         Refresh all windows when font is changed.          */
/*                         Fix bug: Empty Drive menu at startup - thread      */
/*                           timing problem, fix by creating fnMenuBarUpdate()*/
/*          11Sep94  1.52  Fix bug: Empty Drive menu at startup               */
/*                                                                            */
/******************************************************************************/

#define INCL_WIN
#define INCL_BASE
#define INCL_DOS
#define INCL_GPI

#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "diskproc.h"                  /* include file for disk procedures    */
#include "diskuse.h"                   /* main include file                   */
#include "diskdlg.h"                   /* include file for dialog boxes       */
                                       /* global variables (move to winwords  */
VOID fnMenuBarUpdate( HWND hwnd );
VOID fnDrawChart( HPS hps, PCHARTINFO pci );
VOID fnDrawLegend( HPS hps, PLEGENDINFO pli );
VOID fnDrawSummary( PLEGENDINFO pli );
VOID fnDrawPercent( HPS hps, FIXED angle, FIXED sweep, float percent,
                    ULONG actsize, PPOINTL point, LONG cx, LONG cy );
VOID fnListBoxEntry( VOID );
#pragma linkage (fnListBoxEntry, system)

BOOL fnRefreshDirectory( HWND hwnd );
VOID fnReadDrive( VOID );
#pragma linkage (fnReadDrive, system)

VOID fnInitialize( VOID );

APIRET   fnStartCommand( CHAR *szCommand, CHAR *szMsg );
PDIRINFO fnSelectItem( HAB hab, HWND hwnd );

BOOL fnCreateDetailsCols( HAB hab );

void main( int argc, char *argv[] )    /* main routine, optional argument is  */
                                       /* drive letter                        */
{

 HAB   hab;                            /* anchor block handle                 */
 HMQ   hmq;                            /* message queue handle                */
 HPS   hps;                            /* presentation space handle           */
 TID   tidThread;                      /* thread id - used for multi-thread   */
 QMSG  qmsg;                           /* PM message                          */
 CHAR  szHelpName [13];                /* filename of help file, ie. *.HLP    */
 CHAR  profileDir[256];
 CHAR  szTitle[50], szText[80];
 RECTL rectlDesktop;                   /* rectangle structure                 */
 SWP   swp;                            /* set window position structure       */
 ULONG ulSize;                         /* size of SWP structure               */
 ULONG argLegend, argDirlist, argAll;
 LONG  x, y, cx, cy;                   /* window position coordinates and size*/
 LONG  lLogoDisplay;                   /* number of seconds to display logo   */
 ULONG LogicalDriveMap;                /* needed by DosQueryCurrentDrive      */
 UCHAR DriveLetter;                    /* drive letter (ie. C, D, E, etc.)    */
 PROFILEINFO profinfo;                 /*                                     */
 HELPINIT hiMain;                      /* help initialization structure       */
 BOOL  windowSize, foundDrive;
 ULONG flFrameFlags = FCF_TITLEBAR      | /* frame creation flags             */
                      FCF_SYSMENU       |
                      FCF_SIZEBORDER    |
                      FCF_MINMAX        |
                      FCF_SHELLPOSITION |
                      FCF_TASKLIST      |
                      FCF_MENU          |
                      FCF_ICON          |
                      FCF_ACCELTABLE;

 /* Process arguments - /P, /L, /D and /A                                     */
 foundDrive = NO;
 useProfile = YES;
 argLegend  = 0;
 argDirlist = 0;
 argAll     = 0;
 profileDir[0] = '\0';
 if( argc > 1 )
 {
   for( x=1; x < argc; x++ )
   {
     strcpy( szText, argv[x]);         /* copy argument to work string        */
     fnToUpperString( szText );        /* convert to upper case               */

     if( szText[0] == '/' )
     {
       switch( szText[1] )
       {
         case( 'P' ):
           {
             if( szText[2] == '=' )
               fnSubstr( szText, profileDir, 4, strlen(szText)-3 );
             if( szText[2] == '-' )
               useProfile = NO;
           }
           break;
         case( 'L' ):
           {
             if( szText[2] == '-' )
               argLegend = 2;
             else
             {
               if( szText[2] == '+' || szText[2] == ' ' )
                 argLegend = 1;
             }
           }
           break;
         case( 'D' ):
           {
             if( szText[2] == '-' )
               argDirlist = 2;
             else
             {
               if( szText[2] == '+' || szText[2] == ' ' )
                 argDirlist = 1;
             }
           }
         case( 'A' ):
           {
             if( szText[2] == '-' )
               argAll = 2;
             else
             {
               if( szText[2] == '+' || szText[2] == ' ' )
                 argAll = 1;
             }
           }
           break;
         default:
           break;
       }   /* end switch               */
     }     /* end if szText[0] == '='  */
     else
     {
       if( foundDrive == NO )
       {
         strcpy( Drive, argv[x]);      /* copy argument into variable         */
         DriveLetter = Drive[0];       /* determine drive number based on     */
         DriveNumber = (USHORT)DriveLetter - 64;   /* char type cast to USHORT*/
         if( DriveNumber >= 33 && DriveNumber <= 58 )
         {
           DriveNumber = DriveNumber - 32;
           DriveLetter = (UCHAR)(DriveNumber + 64);
         }
         foundDrive = YES;
       }
     }
   }       /* end if( x=1; i < argc .. */
 }         /* end if arg > 1           */


 /* PM window initialization                                                  */

 hab = WinInitialize (0);              /* initialization procedures, PM and   */
 hmq = WinCreateMsgQueue (hab, 0);     /* application message queue           */

 fnInitialize();                       /* initization of global vars to NULL  */

 /* Get previous session information from the profile and perform and         */
 /* required application customization to restore previous environment        */

 if( useProfile == YES )
 {
   WinLoadString (hab, 0L, IDS_ININAME, 50, szTitle );
   if( profileDir[0] != '\0' )
   {
     strcat( profileDir, "\\" );
     strcat( profileDir, szTitle );
   }
   else
     strcpy( profileDir, szTitle );
   hini = PrfOpenProfile( hab, profileDir );
   if( hini == 0L )
    WinMessageBox( HWND_DESKTOP, 0L, "Error loading initialization profile.",
                   "Profile Error", 1, MB_OK | MB_MOVEABLE );

   ulSize = sizeof (PROFILEINFO );     /* get size of SWP structure           */
   if (PrfQueryProfileData( hini, szAppName, szGraphDef, &profinfo, &ulSize ))
   {                                   /* move contents of SWP structure to   */
     graphtype     = profinfo.boola;   /* appropriate variable                */
     graphlevel    = profinfo.boolb;
     graphpercent  = profinfo.boolc;
     graphsize     = profinfo.boold;
     graphswapsize = profinfo.boole;
     graphswapstat = profinfo.boolf;
     graphmaxpies  = profinfo.usa;
   }
   else                                /* new, then use default option setting*/
     fnGraphDefaults( );

   if( PrfQueryProfileData (hini, szAppName, szSettingsDef, &profinfo, &ulSize))
   {                                   /* move contents of SWP structure to   */
     initdirlist   = profinfo.boola;   /* appropriate variable                */
     initlegend    = profinfo.boolb;
     showexit      = profinfo.boolc;
     showrefresh   = profinfo.boold;
     initall       = profinfo.boole;
     if( initall != NO && initall != YES )
       initall = YES;
     units         = profinfo.usa;
     directorytree = profinfo.usb;
     if( directorytree != LEFT && directorytree != RIGHT )
       directorytree = LEFT;
     fontsize  = profinfo.ula;
     strcpy(fontname, profinfo.sza);
     if(fontsize == 0)
     {
       fontsize = 10;
       strcpy( fontname, "System Monospaced" );
     }
   }
   else                                /* new, then use default option setting*/
   {
     fnOptionsDefaults( );
     fontsize = 10;
     strcpy( fontname, "System Monospaced" );
   }

   if( PrfQueryProfileData (hini, szAppName, szAutorefDef, &profinfo, &ulSize))
   {                                   /* move contents of SWP structure to   */
     refreshmin   = profinfo.ula;      /* appropriate variable                */
     thresfree    = profinfo.ulc;
     thresswap    = profinfo.uld;
     alarmfree    = profinfo.boola;
     messagefree  = profinfo.boolb;
     alarmswap    = profinfo.boolc;
     messageswap  = profinfo.boold;
     strcpy( commandfree, profinfo.sza );
     strcpy( commandswap, profinfo.szb );
   }
   else                                /* new, then use default option setting*/
     fnAutorefDefaults( );
 }
 else
 {
   fnOptionsDefaults( );
   fnGraphDefaults( );
   fnAutorefDefaults( );
 }

 if( argLegend > 0 )
 {
   if( argLegend == 1 )
     initlegend = YES;
   else
     initlegend = NO;
 }
 if( argDirlist > 0 )
 {
   if( argDirlist == 1 )
     initdirlist = YES;
   else
     initdirlist = NO;
 }
 if( argAll > 0 )
 {
   if( argAll == 1 )
     initall = YES;
   else
     initall = NO;
 }

 /* Process argument if any and determine the drive letter and drive number   */
 /* (ie. C = 3, D = 4, etc.                                                   */

 if( foundDrive == NO )
 {
   DosQueryCurrentDisk( &DriveNumber, &LogicalDriveMap ); /* nope, use        */
   DriveLetter = (char)(DriveNumber + 64);  /* current logged on drive then   */
 }
 Drive[0] = DriveLetter;               /* Place drive letter in a string for  */
 Drive[1] = ':';                       /* general use                         */
 Drive[2] = '\0';                      /* don't forget the NULL terminate     */

 /* Read the entire drive information using another thread                    */

 hev = 0;
 DosCreateEventSem( "\\SEM32\\DISKUSE", &hev, 0, TRUE );

 rootdrive = YES;
 strcpy( globStrings.searchdir, Drive );
 strcpy( globStrings.directory, Drive );
 strcat( globStrings.directory, "(ROOT)" );
 DosCreateThread( &tidThread, (PFNTHREAD)fnReadDrive, 0L, 0L, STACKSIZE );

 /* Register all window classes                                               */

 WinRegisterClass (hab, szClientClass, wpClientWndProc,   /* pie chart window */
                   CS_SIZEREDRAW, 0);
 WinRegisterClass (hab, szLegendClass, wpLegendWndProc,   /* legend window    */
                   CS_SIZEREDRAW, 0);
 WinRegisterClass (hab, szDirlistClass, wpDirlistWndProc, /* directory list   */
                   CS_SIZEREDRAW, 0);                     /* window           */
 WinRegisterClass (hab, szAllClass, wpAllWndProc,         /* summary window   */
                   CS_SIZEREDRAW, 0);
 WinRegisterClass (hab, szNotebookClass, wpNBWndProc,     /* Settings NB      */
                   CS_SIZEREDRAW, 0);

 /* Query the system profile for the user-configured value of logo display    */
 /* time. If no time found, set default to indefinite (-1) display time. If   */
 /* non-zero, call dialog box to display the logo 'about' box.                */

 lLogoDisplay = PrfQueryProfileInt( HINI_PROFILE, "PM_ControlPanel",
                                    "LogoDisplayTime", -1L);

 if (lLogoDisplay != 0)
   WinDlgBox (HWND_DESKTOP, 0L, wpAboutDlgProc,
              0L, IDD_ABOUT, (PVOID) &lLogoDisplay);

 /* Standard window creation. Get title bar text in resource string table.    */
 /* Position the window as desired based on Desktop. Finally, show it!        */

 hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0L, &flFrameFlags,
                                 szClientClass, NULL, 0L, 0L,
                                 idFrameWindow, &hwndClient );

 WinLoadString (hab, 0L, IDS_TITLEBAR, 50, szTitle);
 WinSetWindowText (hwndFrame, szTitle);

 windowSize = TRUE;
 ulSize = sizeof (SWP);
 if( PrfQueryProfileData( hini, szAppName, szWinPos, &swp, &ulSize ) )
 {
   x = swp.x;                          /* ok, got it, set coordinates and     */
   y = swp.y;                          /* window size                         */
   cx = swp.cx;
   cy = swp.cy;
   if( x >= 0 || y >= 0 )
     windowSize = FALSE;
 }
 if( windowSize == TRUE )
 {
   WinQueryWindowRect (HWND_DESKTOP, &rectlDesktop);
   x = rectlDesktop.xRight / 5;
   y = 0;
   if( rectlDesktop.yTop * 19 / 20 > rectlDesktop.xRight * 4 / 5 )
   {
     cx = rectlDesktop.xRight * 4 / 5;
     cy = rectlDesktop.xRight * 4 / 5;
   }
   else
   {
     cx = rectlDesktop.yTop * 19 / 20;
     cy = rectlDesktop.yTop * 19 / 20;
   }
 }

 WinSetWindowPos (hwndFrame, HWND_TOP, x, y, cx, cy,
                  SWP_SHOW | SWP_SIZE | SWP_MOVE | SWP_ACTIVATE);
 WinShowWindow( hwndFrame, TRUE );
 WinSetActiveWindow( HWND_DESKTOP, hwndFrame );

 /* Help manager initialization & preparation. Set up the HELPINIT structure  */
 /* Create help instance, if unable, display error message.                   */

 hiMain.cb = sizeof (HELPINIT);        /* Standard size of structure          */
 hiMain.ulReturnCode = 0L;             /* rc - if non-zero after -> error!    */
 hiMain.pszTutorialName = (PSZ)NULL;   /* tutorial program, if none, set NULL */
                                       /* Help table ID in resources          */
 hiMain.phtHelpTable = (PVOID)( 0xffff0000 | IDH_TABLE_MAIN );
 hiMain.hmodHelpTableModule = 0L;      /* Help not in DLL                     */
 hiMain.hmodAccelActionBarModule = 0L; /* No add'l help resources             */
 hiMain.idAccelTable = 0L;             /* No add'l help accelerator           */
 hiMain.idActionBar = 0L;              /* No add'l help menu                  */

 WinLoadString (hab, 0L, IDS_HELPTITLE, 50, szTitle);
 hiMain.pszHelpWindowTitle = szTitle;  /* Help manager window title           */

 hiMain.fShowPanelId = FALSE;          /* Don't show panel numbers            */

 WinLoadString (hab, 0L, IDS_HELPNAME, 13, szHelpName);
 hiMain.pszHelpLibraryName = szHelpName;  /* Name of .HLP file                */

 hwndHelp = WinCreateHelpInstance (hab, &hiMain); /* create help instance     */
 if (!hwndHelp)
   WinMessageBox (HWND_DESKTOP, 0L, szHelpName,
                  "Help Error", 1, MB_OK | MB_MOVEABLE);
 else
 {
   if (hiMain.ulReturnCode)
   {
     WinMessageBox (HWND_DESKTOP, 0L, "Help not available!",
                    "Help Error", 1, MB_OK | MB_MOVEABLE);
     WinDestroyHelpInstance (hwndHelp);
     hwndHelp = 0L;
   }
   else
     WinAssociateHelpInstance (hwndHelp, hwndFrame);
 }

 /* Standard message loop. Get messages from the queue and dispatch them to   */
 /* the approprite windows.                                                   */

 while (WinGetMsg (hab, &qmsg, 0L, 0, 0))
    WinDispatchMsg (hab, &qmsg);

 /* Main loop has terminated. Destroy all windows, help instance and message  */
 /* queue and then terminate the application.                                 */

 if (hwndHelp)
    WinDestroyHelpInstance (hwndHelp);
 WinDestroyWindow (hwndFrame);
 WinDestroyMsgQueue (hmq);
 WinTerminate (hab);

 DosExit (EXIT_PROCESS, 0L);

}

/******************************************************************************/
/*                                                                            */
/*                  W I N D O W       P R O C E D U R E S                     */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* wpClientWndProc: Window procedure to handle pie chart window, menu bar     */
/* processing, help, etc.                                                     */
/*                                                                            */
/* Calls: fnRefreshDirectory                                                  */
/*        fnListBoxEntry                                                      */
/*        fnFreeList                                                          */
/*        fnDrawChart                                                         */
/*        fnToPolar                                                           */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpClientWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{

 MRESULT           mr = (MRESULT) FALSE;
 FONTMETRICS       fm;
 CHAR   szText[80], szTitle[50];
 static HAB        hab;
 static PCHARTINFO pci;
 static ULONG      refreshtimecount;   /* number of time out intervals        */

 switch (msg)
    {

     case WM_CREATE:                   /* Window just got created. Perform    */
                                       /* other window initialization here    */
        WinSendMsg(hwnd, UM_CREATE, 0L, 0L);
        WinSendMsg(hwnd, UM_FONT, 0L, 0L);
        break;

     case UM_CREATE:                   /* Completion of window creation       */
        {                              /* processing                          */
          RECTL    rectl;              /* structure for frame coordinates     */
          LONG     x, y, cx, cy;       /* window position coordinates and size*/
          SWP      swp;
          APIRET   rc;
          ULONG    ulSize;
          BOOL     windowSize;
          HWND     hwndDLClient;
          ULONG flFrameFlags = FCF_TITLEBAR   | /* frame creation flags for   */
                               FCF_SYSMENU    | /* legend and directory list  */
                               FCF_SIZEBORDER | /* window - sizable           */
                               FCF_SHELLPOSITION |
                               FCF_VERTSCROLL |
                               FCF_HORZSCROLL |
                               FCF_TASKLIST;

          WinLoadString (hab, 0L, IDS_DIRECTORY, 20, globStrings.szDirectory );
          WinLoadString (hab, 0L, IDS_DIRSIZE,   20, globStrings.szDirSize );
          WinLoadString (hab, 0L, IDS_CUMSIZE,   20, globStrings.szCumSize );
          WinLoadString (hab, 0L, IDS_PERCENT,   2,  globStrings.szPercent );
          WinLoadString (hab, 0L, IDS_FILES,     20, globStrings.szFiles );

          if( refreshmin > 0 )
          {
            refreshtimecount = refreshmin;
            WinStartTimer( hab, hwnd, ID_TIMER, 60000L );
          }

//        rc = DosWaitEventSem( hev, 1L );
//        if( rc == ERROR_TIMEOUT )
//        {
//          WinEnableMenuItem( WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
//                             FID_MENU), IDM_REFRESH, FALSE );
//          WinEnableMenuItem( WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
//                             FID_MENU), IDM_DRIVE, FALSE );
//        }

          fnMenuBarUpdate( hwnd );

          pci = (PCHARTINFO)malloc(sizeof(CHARTINFO));
          pci->hwnd = hwnd;

          hab = WinQueryAnchorBlock (hwnd);
          pci->hab = hab;

          showlegend  = initlegend;
          showdirlist = initdirlist;
          showall     = initall;


          WinLoadString( hab, 0L, IDS_FREE_L,   10, globStrings.szFree  );
          WinLoadString( hab, 0L, IDS_TOTAL_L,  10, globStrings.szTotal );
          WinLoadString( hab, 0L, IDS_PATIENT,  80, globStrings.szPatient );
          WinLoadString( hab, 0L, IDS_INSTRUCT, 80, globStrings.szInstruct );

          WinQueryWindowRect( HWND_DESKTOP, &rectl );

          /* Create the legend window. Position it as desired. Put or remove  */
          /* a check mark on the pull-down choice. Finally, show it!          */

          hwndLegend = WinCreateStdWindow( HWND_DESKTOP, 0L, &flFrameFlags,
                                           szLegendClass, NULL, 0L, 0L,
                                           ID_LEGEND, NULL );

          if( hwndHelp )
            WinAssociateHelpInstance( hwndHelp, hwndLegend );
          WinLoadString (hab, 0L, IDS_LEGEND, 50, szTitle);
          WinSetWindowText (hwndLegend, szTitle);

          windowSize = TRUE;
          ulSize = sizeof (SWP);
          if( PrfQueryProfileData( hini, szAppName, szLegendPos, &swp, &ulSize ) )
          {
            x = swp.x;                 /* ok, got it, set coordinates and     */
            y = swp.y;                 /* window size                         */
            cx = swp.cx;
            cy = swp.cy;
            if( x >= 0 || y >= 0 )
              windowSize = FALSE;
          }
          if( windowSize == TRUE )
          {
            x = 0;
            y = rectl.yTop / 3;
            cx = rectl.xRight / 4;
            cy = rectl.yTop / 3;
          }

          WinSetWindowPos (hwndLegend, HWND_TOP, x, y, cx, cy,
                           SWP_SHOW | SWP_SIZE | SWP_MOVE | SWP_ACTIVATE);
                                       /* legend window should be displayed   */
                                       /* so show it and also check mark      */
          WinShowWindow( hwndLegend,  showlegend );
          WinSendDlgItemMsg (WinQueryWindow(hwnd,QW_PARENT),
                            (ULONG) FID_MENU,
                            (ULONG) MM_SETITEMATTR,
                            MPFROM2SHORT(IDM_LEGEND,TRUE),
                            MPFROM2SHORT(MIA_CHECKED, showlegend ?
                                         MIA_CHECKED : 0));

          /* Create the Summary window. Position it as desired. Put or remove */
          /* a check mark on the pull-down choice. Finally, show it!          */

          hwndAll = WinCreateStdWindow( HWND_DESKTOP, 0L, &flFrameFlags,
                                        szAllClass, NULL, 0L, 0L,
                                        ID_ALL, NULL );
          if( hwndHelp )
            WinAssociateHelpInstance( hwndHelp, hwndAll );
          WinLoadString (hab, 0L, IDS_ALL, 50, szTitle);
          WinSetWindowText (hwndAll, szTitle);

          windowSize = TRUE;
          ulSize = sizeof (SWP);
          if( PrfQueryProfileData( hini, szAppName, szAllPos, &swp, &ulSize ) )
          {
            x = swp.x;                 /* ok, got it, set coordinates and     */
            y = swp.y;                 /* window size                         */
            cx = swp.cx;
            cy = swp.cy;
            if( x >= 0 || y >= 0 )
              windowSize = FALSE;
          }
          if( windowSize == TRUE )
          {
            x = 0;
            y = 0;
            cx = rectl.xRight / 3;
            cy = rectl.yTop / 3;
          }

          WinSetWindowPos (hwndAll, HWND_TOP, x, y, cx, cy,
                           SWP_SHOW | SWP_SIZE | SWP_MOVE | SWP_ACTIVATE);
                                       /* legend window should be displayed   */
                                       /* so show it and also check mark      */
          WinShowWindow( hwndAll,  showall );
          WinSendDlgItemMsg (WinQueryWindow(hwnd,QW_PARENT),
                            (ULONG) FID_MENU,
                            (ULONG) MM_SETITEMATTR,
                            MPFROM2SHORT(IDM_ALL,TRUE),
                            MPFROM2SHORT(MIA_CHECKED, showall ?
                                         MIA_CHECKED : 0));

          /* Create the directory list window. Position it as desired. Put or */
          /* remove a check mark on the pull-down choice. Finally, show it!   */

          flFrameFlags = FCF_TITLEBAR   |
                         FCF_SYSMENU    |
                         FCF_SIZEBORDER |
                         FCF_SHELLPOSITION |
                         FCF_TASKLIST;
          hwndDirlist = WinCreateStdWindow( HWND_DESKTOP, FS_BORDER, &flFrameFlags,
                                            szDirlistClass, NULL, 0L, 0L,
                                            ID_DIRLIST, NULL );

          if( hwndHelp )
            WinAssociateHelpInstance( hwndHelp, hwndDirlist );
          WinLoadString (hab, 0L, IDS_DIRLIST, 50, szTitle);
          WinSetWindowText (hwndDirlist, szTitle);

          windowSize = TRUE;
          ulSize = sizeof (SWP);
          if( PrfQueryProfileData( hini, szAppName, szDirlistPos,
                          &swp, &ulSize ) )
          {
            x = swp.x;                 /* ok, got it, set coordinates and     */
            y = swp.y;                 /* window size                         */
            cx = swp.cx;
            cy = swp.cy;
            if( x >= 0 || y >= 0 )
              windowSize = FALSE;
          }
          if( windowSize == TRUE )
          {
            x = 0;
            y = 2 * rectl.yTop / 3;
            cx = rectl.xRight * 4 / 5;
            cy = rectl.yTop / 3;
          }

          WinSetWindowPos (hwndDirlist, HWND_TOP, x, y, cx, cy,
                           SWP_SHOW | SWP_SIZE | SWP_MOVE | SWP_ACTIVATE);
                                       /* directory list window should be     */
                                       /* displayed so show it and check mark */
          WinShowWindow( hwndDirlist, showdirlist );
          WinSendDlgItemMsg (WinQueryWindow(hwnd,QW_PARENT),
                            (ULONG) FID_MENU,
                            (ULONG) MM_SETITEMATTR,
                            MPFROM2SHORT(IDM_DIRLIST,TRUE),
                            MPFROM2SHORT(MIA_CHECKED, showdirlist ?
                                         MIA_CHECKED : 0));


       }                               /* end case (UM_CREATE)                */
       break;

     case UM_FONT:
       {
         HPS      hps;                 /* presentation space handle           */
         CHAR szText[80];

         sprintf(szText, "%d.%-s", fontsize, fontname);
         WinSetPresParam(hwnd, PP_FONTNAMESIZE, sizeof(szText), szText);

         hps = WinBeginPaint (hwnd, 0L, 0L);
         GpiQueryFontMetrics( hps, sizeof(FONTMETRICS), &fm );
         pci->cxChar = (LONG)fm.lAveCharWidth;
         pci->cyChar = (LONG)fm.lMaxBaselineExt;
         pci->cyDesc = (LONG)fm.lMaxDescender;
         WinEndPaint( hps );

       }
       break;

     case WM_SIZE:                     /* Window has been resized, re-calc    */
        pci->cxClient = SHORT1FROMMP( mp2 );/* sizing values                  */
        pci->cyClient = SHORT2FROMMP( mp2 );

        /* ptctr.x and ptctr.y define the centre of the pie  */
        pci->ptctr.x = (pci->cxClient) / 2;
        pci->ptctr.y = (pci->cyClient - pci->cyChar ) / 2 + pci->cyChar;
        if( pci->ptctr.x > pci->ptctr.y - pci->cyChar ) /* find radius of pie */
        {
          if( pci->ptctr.y - pci->cyChar - 2*pci->cyChar > GPIFULLARC_MULT_LIMIT )
            pci->radius = MAKEFIXED( GPIFULLARC_MULT_LIMIT, 0 );
          else
            pci->radius = MAKEFIXED(pci->ptctr.y - pci->cyChar - 2*pci->cyChar, 0 );
        }
        else                           /* based on smaller window width       */
        {
          if (pci->ptctr.x - 2*pci->cyChar > GPIFULLARC_MULT_LIMIT )
            pci->radius = MAKEFIXED( GPIFULLARC_MULT_LIMIT, 0 );
          else
            pci->radius = MAKEFIXED(pci->ptctr.x - 2*pci->cyChar, 0 );  /* allow for text    */
        }
        break;                         /* end case (WM_SIZE)                  */

     case WM_PAINT:                    /* Redraw the client window here ...   */
        {
          HPS hps;

          WinSetPointer( HWND_DESKTOP, WinQuerySysPointer( HWND_DESKTOP,
                         SPTR_WAIT, FALSE ) );  /* change to clock pointer    */
          hps = WinBeginPaint (hwnd, 0L, 0L);
          fnDrawChart( hps, pci );
          WinEndPaint (hps);
          WinSetPointer( HWND_DESKTOP, WinQuerySysPointer( HWND_DESKTOP,
                         SPTR_ARROW, FALSE ) );
        }
        break;

     case WM_MINMAXFRAME:
        {
          PSWP pswp;
          pswp = (PSWP)PVOIDFROMMP( mp1 );

          if( pswp->fl & SWP_MINIMIZE )
          {
            WinShowWindow( hwndLegend,  FALSE );
            WinShowWindow( hwndDirlist, FALSE );
          }
          else
          {
            if( pswp->fl & SWP_RESTORE )
            {
              WinShowWindow( hwndLegend,  showlegend );
              WinShowWindow( hwndDirlist, showdirlist );
            }
          }
          mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
        }
        break;

     case WM_TIMER:
        {
          refreshtimecount--;          /* WinStartTimer can only timout to a  */
          if( refreshtimecount == 0 )  /* max of 1 minute, so we must use a   */
          {                            /* counter if > 1 minute. Decrement    */
                                       /* counter until 0 then desired timeout*/
                                       /* has occurred then process           */
             BOOL oldshowrefresh = showrefresh;
             showrefresh = NO;
             rootdrive = YES;
             fnRefreshDirectory( hwnd );
             showrefresh = oldshowrefresh;
             refreshtimecount = refreshmin;
          }
        }
        break;

//   case WM_BUTTON1DOWN:
//   case WM_BUTTON2DOWN:
     case WM_BUTTON1DBLCLK:            /* user double clicked on a pie slice  */
     case WM_BUTTON2DBLCLK:            /* determine location of mouse, then   */
        {                              /* ensure inside the pie circle and    */
          float radius, angle;         /* determine which directory           */
                                       /* must convert to polar coordinates   */
          fnToPolar( (float)(SHORT1FROMMP(mp1) - pci->ptctr.x),
                     (float)(SHORT2FROMMP(mp1) - pci->ptctr.y),
                     &radius, &angle );
          if( pci->radius > MAKEFIXED(radius, 0) )
          {                            /* mouse clicked inside pie circle so  */
            PDIRINFO item;             /* continue processing to determine    */
            USHORT   maxpies, i;       /* which directory                     */
            float    oldangle;

            if( graphlevel == GRAPH_LEVEL_1ST )
              item = startsort1st;
            else
              item = startsort;

            if( graphtype == GRAPH_TYPE_USED )
              maxpies = graphmaxpies - 1;
            else
              maxpies = graphmaxpies - 2;
            if( graphswapsize == YES && swapsize > 0 )
              maxpies = maxpies - 1;
            if( maxpies < 0 )
              maxpies = 0;

            oldangle = 0;
            for( i = 0; i < maxpies; i++ )
            {
              if( angle > oldangle && angle < item->angle )
              {
                WinDlgBox (HWND_DESKTOP, hwnd, wpDetailsDlgProc,
                           0L, IDD_DETAILS, item );
                break;
              }
              oldangle = item->angle;
              if( graphlevel == GRAPH_LEVEL_1ST )
                item = (PDIRINFO)item->nextsort1st;
              else
                item = (PDIRINFO)item->nextsort;
              if( item == NULL )
                i = maxpies;
            }

          }
        }                              /* end case WM_BUTTONxDBLCLK           */
        break;

     case WM_COMMAND:                  /* Application command processing here */
        {
         switch (SHORT1FROMMP (mp1))   /* Message contained in mp1 variable   */
            {
             case IDM_REFRESH:         /* Refresh choice selected. Show       */
                rootdrive = YES;
                fnRefreshDirectory( hwnd ); /* message box and process        */
                break;

             case IDM_DRIVE_A:         /* Valid drives shown in Drives pull-  */
             case IDM_DRIVE_B:         /* down, each drive letter is a choice */
             case IDM_DRIVE_C:         /* Process all together                */
             case IDM_DRIVE_D:
             case IDM_DRIVE_E:
             case IDM_DRIVE_F:
             case IDM_DRIVE_G:
             case IDM_DRIVE_H:
             case IDM_DRIVE_I:
             case IDM_DRIVE_J:
             case IDM_DRIVE_K:
             case IDM_DRIVE_L:
             case IDM_DRIVE_M:
             case IDM_DRIVE_N:
             case IDM_DRIVE_P:
             case IDM_DRIVE_Q:
             case IDM_DRIVE_R:
             case IDM_DRIVE_S:
             case IDM_DRIVE_T:
             case IDM_DRIVE_U:
             case IDM_DRIVE_V:
             case IDM_DRIVE_W:
             case IDM_DRIVE_X:
             case IDM_DRIVE_Y:
             case IDM_DRIVE_Z:
               {
                 ULONG olddrivenumber = DriveNumber;
                 UCHAR olddrive[3];
                 BOOL  oldrootdrive = rootdrive;

//               sprintf(szText, "%d", SHORT1FROMMP(mp1));
//               WinMessageBox (HWND_DESKTOP, hwnd, szText, "Debug",
//                             1, MB_OK | MB_MOVEABLE | MB_INFORMATION);

                 strcpy( olddrive, Drive );
                 DriveNumber = SHORT1FROMMP(mp1) - MENU_DRIVE;
                 Drive[0] = (CHAR)(DriveNumber + 64);

                 if( olddrivenumber != DriveNumber )
                 {
                   rootdrive = YES;
                   if( !fnRefreshDirectory( hwnd ) )
                   {                   /* cancel Refresh message, restore old */
                     rootdrive = oldrootdrive;
                     DriveNumber = olddrivenumber;          /* drive number   */
                     strcpy( Drive, olddrive );
                     strcpy( globStrings.searchdir, Drive );
                     strcpy( globStrings.directory, Drive );
                     strcat( globStrings.directory, "(ROOT)" );
                   }
                   else                /* check new drive letter, uncheck the */
                   {                   /* previously selected one             */
                     WinSendDlgItemMsg (WinQueryWindow(hwnd,QW_PARENT),
                                       (ULONG) FID_MENU,
                                       (ULONG) MM_SETITEMATTR,
                                       MPFROM2SHORT(DriveNumber+MENU_DRIVE,TRUE),
                                       MPFROM2SHORT(MIA_CHECKED,MIA_CHECKED));
                     WinSendDlgItemMsg (WinQueryWindow(hwnd,QW_PARENT),
                                       (ULONG) FID_MENU,
                                       (ULONG) MM_SETITEMATTR,
                                       MPFROM2SHORT(olddrivenumber+MENU_DRIVE,TRUE),
                                       MPFROM2SHORT(MIA_CHECKED,0));
                   }
                 }                     /* end if drive number changed         */
               }                       /* end case IDM_DRIVE_x                */
               break;

             case IDM_SETTINGS:        /* Settings choice selected, display   */
                                       /* Settings notebook                   */
               fnCreateNotebook( hab, hwnd );
               break;

             case IDM_LEGEND:          /* Legend choice selected. Toggle the  */
                {                      /* variable and then put/remove check  */
                  showlegend = !showlegend; /*mark according to bool variable */
                  WinSendDlgItemMsg (WinQueryWindow(hwnd,QW_PARENT),
                             (ULONG) FID_MENU,
                             (ULONG) MM_SETITEMATTR,
                             MPFROM2SHORT(IDM_LEGEND,TRUE),
                             MPFROM2SHORT(MIA_CHECKED, showlegend ?
                                          MIA_CHECKED : 0));
                  if( showlegend == YES ) /* show or hide the legend window   */
                    WinShowWindow( hwndLegend, TRUE );
                  else
                    WinShowWindow( hwndLegend, FALSE );
                }                      /* end case (IDM_LEGEND)               */
                break;

             case IDM_DIRLIST:         /* Directory List selected. Toggle the */
                {                      /* variable and then put/remove check  */
                  showdirlist = !showdirlist; /* mark based on bool variable  */
                  WinSendDlgItemMsg (WinQueryWindow(hwnd,QW_PARENT),
                             (ULONG) FID_MENU,
                             (ULONG) MM_SETITEMATTR,
                             MPFROM2SHORT(IDM_DIRLIST,TRUE),
                             MPFROM2SHORT(MIA_CHECKED, showdirlist ?
                                          MIA_CHECKED : 0));
                  if( showdirlist == YES )/* show or hide the directory list  */
                    WinShowWindow( hwndDirlist, TRUE );
                  else
                    WinShowWindow( hwndDirlist, FALSE );
                }                      /* end case (IDM_DIRLIST)              */
                break;

             case IDM_ALL:             /* Summary choice selected. Toggle the */
                {                      /* variable and then put/remove check  */
                  showall = !showall;       /*mark according to bool variable */
                  WinSendDlgItemMsg (WinQueryWindow(hwnd,QW_PARENT),
                             (ULONG) FID_MENU,
                             (ULONG) MM_SETITEMATTR,
                             MPFROM2SHORT(IDM_ALL,TRUE),
                             MPFROM2SHORT(MIA_CHECKED, showall ?
                                          MIA_CHECKED : 0));
                  if( showall == YES )    /* show or hide the legend window   */
                    WinShowWindow( hwndAll, TRUE );
                  else
                    WinShowWindow( hwndAll, FALSE );
                }                      /* end case (IDM_ALL)                  */
                break;

             case IDM_PRINT:           /* Display Printer dialog box here ... */
                WinDlgBox (HWND_DESKTOP, hwnd, wpPrintDlgProc, 0L,
                           IDD_PRINT, NULL);
                break;                 /* end case IDM_PRINT                  */

             case IDM_FONT:
                {
                  FONTDLG pfdFontdlg;  /* Font dialog info structure          */
                  HWND    hwndFontDlg; /* Font dialog window                  */
                  ULONG   fontSize;

                  memset(&pfdFontdlg, 0, sizeof(FONTDLG));

                  pfdFontdlg.cbSize = sizeof(FONTDLG);  /* Size of structur   */
                  pfdFontdlg.hpsScreen = WinGetPS(hwnd);/* Screen present.    */
                  pfdFontdlg.pszFamilyname  = fontname;
                  pfdFontdlg.usFamilyBufLen = FACESIZE;
                  fontSize = fontsize;
                  pfdFontdlg.fxPointSize = MAKEFIXED(fontSize,0);
                  pfdFontdlg.fl = FNTS_CENTER | FNTS_RESETBUTTON |
                                  FNTS_HELPBUTTON | FNTS_FIXEDWIDTHONLY;
                  pfdFontdlg.clrFore = CLR_BLACK;       /* Foreground color   */
                  pfdFontdlg.clrBack = CLR_WHITE;       /* Background color   */
                  pfdFontdlg.usWeight = FWEIGHT_NORMAL;
                  pfdFontdlg.usWidth  = FWIDTH_NORMAL;
                  hwndFontDlg = WinFontDlg(HWND_DESKTOP, hwnd, &pfdFontdlg);
                  if (hwndFontDlg && (pfdFontdlg.lReturn == DID_OK))
                  {
                    HPS         hps;
                    FONTMETRICS fm;

                    fontsize = pfdFontdlg.sNominalPointSize/10;
                    strcpy(fontname, pfdFontdlg.fAttrs.szFacename);

                    WinSendMsg(hwnd,        UM_FONT, 0L, 0L);
                    WinSendMsg(hwndLegend,  UM_FONT, 0L, 0L);
                    WinSendMsg(hwndDirlist, UM_FONT, 0L, 0L);
                    WinSendMsg(hwndAll,     UM_FONT, 0L, 0L);

                    WinPostMsg( hwnd, UM_PAINT, 0L, 0L );
                    WinPostMsg( hwndLegend, UM_LEGEND, 0L, 0L );
                    WinPostMsg( hwndAll, UM_ALL, 0L, 0L );
//                  fnRefreshDirectory( hwnd );

//                  WinLoadString (hab, 0L, IDS_FONTTITLE,  50, szTitle);
//                  WinLoadString (hab, 0L, IDS_FONTTEXT, 80, szText);

//                  WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle, 1,
//                                 MB_OK | MB_MOVEABLE | MB_INFORMATION);
                  }

                }
                break;

             case IDM_USINGHELP:       /* Display Using Help Facility window  */
               {                       /* Just post HM_DISPLAY_HELP message   */
                HWND hwndHelpId;

                hwndHelpId = WinQueryHelpInstance (hwnd);
                WinSendMsg (hwndHelpId, HM_DISPLAY_HELP, 0L, 0L);
               }                       /* end case (IDM_USINGHELP)            */
               break;

             case IDM_HELPPRODINFO:    /* Display Product Information window  */
               {
                 LONG lLogoDisplay = -1;
                 WinDlgBox (HWND_DESKTOP, hwnd, wpAboutDlgProc,
                            0L, IDD_ABOUT, (PVOID) &lLogoDisplay);
               }                       /* end case (IDM_HELPPRODINFO)         */
               break;

             default:                  /* Let PM handle the rest of messages  */
                mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
                break;
            }                          /* end switch                          */
        }                              /* end case (WM_COMMAND)               */
        break;

     case HM_QUERY_KEYS_HELP:          /* User requested keys help            */
       mr = (MRESULT) IDH_HELPKEYS;    /* Return keys help panel id           */
       break;

     case WM_SAVEAPPLICATION:          /* Application about to terminate.     */
       {                               /* Save information in profile data.   */
         SWP         swp;
         PROFILEINFO profinfo;

         if( useProfile == YES )
         {
           profinfo.boola = graphtype;
           profinfo.boolb = graphlevel;
           profinfo.boolc = graphpercent;
           profinfo.boold = graphsize;
           profinfo.boole = graphswapsize;
           profinfo.boolf = graphswapstat;
           profinfo.usa   = graphmaxpies;
                                       /* write profile data in OS2.INI       */
           PrfWriteProfileData( hini, szAppName, szGraphDef,
                                &profinfo, sizeof (PROFILEINFO));

           profinfo.boola = initdirlist;
           profinfo.boolb = initlegend;
           profinfo.boolc = showexit;
           profinfo.boold = showrefresh;
           profinfo.boole = initall;
           profinfo.usa   = units;
           profinfo.usb   = directorytree;
           profinfo.ula   = fontsize;
           strcpy(profinfo.sza, fontname);
                                       /* write profile data in OS2.INI       */
           PrfWriteProfileData( hini, szAppName, szSettingsDef,
                                &profinfo, sizeof (PROFILEINFO));

           profinfo.ula   = refreshmin;
           profinfo.ulc   = thresfree;
           profinfo.uld   = thresswap;
           profinfo.boola = alarmfree;
           profinfo.boolb = messagefree;
           profinfo.boolc = alarmswap;
           profinfo.boold = messageswap;
           strcpy( profinfo.sza, commandfree );
           strcpy( profinfo.szb, commandswap );
                                       /* write profile data in OS2.INI       */
           PrfWriteProfileData( hini, szAppName, szAutorefDef,
                                &profinfo, sizeof (PROFILEINFO));

           WinQueryWindowPos (WinQueryWindow (hwnd, QW_PARENT), &swp);
           PrfWriteProfileData( hini, szAppName, szWinPos, &swp, sizeof (SWP));

           WinQueryWindowPos ( hwndLegend, &swp);
           PrfWriteProfileData( hini, szAppName, szLegendPos, &swp, sizeof (SWP));

           WinQueryWindowPos ( hwndDirlist, &swp);
           PrfWriteProfileData( hini, szAppName, szDirlistPos, &swp, sizeof (SWP));

           if( hini != 0L )
             PrfCloseProfile( hini );

           mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
         }
       }                               /* end case (WM_SAVEAPPLICATION)       */
       break;

//   case WM_ACTIVATE:
//     {
//       if( SHORT1FROMMP(mp1)==TRUE && HWNDFROMMP(mp2)==hwndFrame )
//       {
//         WinSetActiveWindow( HWND_DESKTOP, hwndDirlist );
//         WinSetActiveWindow( HWND_DESKTOP, hwndLegend  );
//         WinSetActiveWindow( HWND_DESKTOP, hwndAll     );
//         WinSetActiveWindow( HWND_DESKTOP, hwndFrame   );
//       }
//       mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
//     }
//     break;

     case WM_HELP:                     /* WM_HELP message processing here     */
        {
         switch (SHORT1FROMMP (mp1))
            {

             case DID_HELP_BUTTON:     /* Help push button pressed for        */
                {                      /* standard Font dialog window         */
                  HWND hwndHelpId;

                  hwndHelpId = WinQueryHelpInstance (hwnd);
                  WinSendMsg(hwndHelpId, HM_DISPLAY_HELP,
                             MPFROM2SHORT(IDH_FONT_DLG, NULL),
                             MPFROMSHORT(HM_RESOURCEID));
                }                      /* end case DID_HELP_BUTTON            */
                break;

             default:                  /* Let PM handle the rest of messages  */
                mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
                break;
            }                          /* end switch                          */
        }                              /* end case (WM_HELP)                  */
        break;

     case UM_PAINT:
       {
         RECTL rectl;

         WinQueryWindowRect( hwnd, &rectl );
         WinInvalidateRect( hwnd, &rectl, FALSE );
       }
       break;

     case UM_WARNING:                  /* notification if thresholds reached  */
       {                               /* only if refresh interval > 0        */
         if( refreshmin > 0 )
         {
           UCHAR szText1[80];
                                       /* check free space threshold limit    */
           if( ((float)totalfree * 100 / (float)totalsize) < (float)thresfree )
           {

             WinLoadString (hab, 0L, IDS_WARNING, 50, szTitle);
             WinLoadString (hab, 0L, IDS_THRESFREE, 60, szText);
             sprintf( szText1, "%s %s %d%%.", Drive, szText, thresfree );

             if( alarmfree == YES )
               DosBeep( 100, 100 );

             if( messagefree == YES )
                WinMessageBox (HWND_DESKTOP, hwnd, szText1, szTitle,
                               1, MB_OK | MB_MOVEABLE | MB_INFORMATION);

             if( commandfree[0] != ' ' && commandfree[0] != '\0' )
               fnStartCommand( commandfree, szText1 );

           }
                                       /* check SWAPPER.DAT threshold limit   */
           if( ((float)swapsize * 100 / (float)totalsize) > (float)thresswap )
           {

             WinLoadString (hab, 0L, IDS_WARNING, 50, szTitle);
             WinLoadString (hab, 0L, IDS_THRESSWAP, 60, szText);
             sprintf( szText1, "%s %s %d%%.", Drive, szText, thresswap );

             if( alarmswap == YES )
               DosBeep( 100, 100 );

             if( messageswap == YES )
                WinMessageBox (HWND_DESKTOP, hwnd, szText1, szTitle,
                               1, MB_OK | MB_MOVEABLE | MB_INFORMATION);

             if( commandswap[0] != ' ' && commandswap[0] != '\0' )
               fnStartCommand( commandswap, szText1 );

           }
         }
       }                               /* end case (UM_WARNING)               */
       break;

     case UM_AUTOREF:
       WinStopTimer( hab, hwnd, ID_TIMER );
       if( refreshmin > 0 )
       {
         refreshtimecount = refreshmin;
         WinStartTimer( hab, hwnd, ID_TIMER, 60000L );
       }
       break;

     case UM_LISTBOX:
       {
         TID tidThread;                /* must redo the directory listbox     */
                                       /* delete all items and insert again   */

         WinSendMsg( hwndList, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE));
         WinSendMsg( hwndList, CM_REMOVEDETAILFIELDINFO, NULL, MPFROM2SHORT(0, CMA_FREE));
//       WinSendMsg( hwndList, LM_DELETEALL, MPFROMSHORT(0), (MPARAM) NULL );

         DosCreateThread( &tidThread, (PFNTHREAD)fnListBoxEntry,
                          0L, 0L, STACKSIZE );
         WinSendMsg( hwndAll, UM_ALL, 0L, 0L );
       }
       break;

     case UM_REFDIR:
       rootdrive = YES;
       fnRefreshDirectory( hwnd );
       break;

     case UM_PRINT:
       {
         BOOL flag;

         flag = (BOOL)LONGFROMMP(mp1);

         WinEnableMenuItem(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
                           FID_MENU), IDM_REFRESH, flag);
         WinEnableMenuItem(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
                           FID_MENU), IDM_SETTINGS, flag);
         WinEnableMenuItem(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
                           FID_MENU), IDM_FONT, flag);
         WinEnableMenuItem(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
                           FID_MENU), IDM_PRINT, flag);
         WinEnableMenuItem(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
                           FID_MENU), IDM_DRIVE, flag);
       }
       break;

     case WM_ERASEBACKGROUND:
        mr = (MRESULT) TRUE;
        break;

     case WM_CLOSE:                    /* User requested to exit application  */
       {                               /* so just post a WM_QUIT message      */
         if( showexit == YES )
         {
                                       /* Confirmation message box is used in */
                                       /* this example, remove if unnecessary.*/
                                       /* Text displayed in message box       */
                                       /* Get text from resource string table */
           WinLoadString (hab, 0L, IDS_TITLEEXIT,  50, szTitle);
           WinLoadString (hab, 0L, IDS_EXITPROMPT, 80, szText);

           if (WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle, 1,
                              MB_MOVEABLE |
                              MB_YESNO    |
                              MB_WARNING  |
                              MB_DEFBUTTON2
                             ) == MBID_YES)
              WinPostMsg (hwnd, WM_QUIT, 0L, 0L); /* user really wants to exit*/
         }
         else
           WinPostMsg (hwnd, WM_QUIT, 0L, 0L);
        }
        break;

     case WM_DESTROY:                  /* Window about to be destroyed.       */
        {
          fnFreeList( );               /* free linked list memory             */
          free( pci );
          WinSendMsg (hwndLegend, UM_CLOSE, 0L, 0L);
          DosCloseEventSem( hev );     /* close event semaphore               */
        }                              /* end case (WM_DESTROY)               */
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
        break;

    }                                  /* end main switch statement           */

 return mr;

}

/******************************************************************************/
/*                                                                            */
/* fnDrawChart: Draw pie chart in client window. Called by WM_PAINT and       */
/*              print thread. If pci->hwnd = 0, then we know it is from the   */
/*              print thread.                                                 */
/*                                                                            */
/******************************************************************************/

VOID fnDrawChart( HPS hps, PCHARTINFO pci )

{

  RECTL       rect;
  POINTL      point, ptpct;
  ARCPARAMS   arcp = { 1, 1, 0, 0 };
  PDIRINFO    item;
  LONG        i, maxpies, shade, size;
  FIXED       sweepangle, startangle;
  UCHAR       szUnit[10];
  CHAR        szTitle[50], szText[80];
  APIRET      rc;
  float       pct, pctangle;

  rect.xLeft   = 0;
  rect.xRight  = pci->cxClient;
  rect.yBottom = 0;
  rect.yTop    = pci->cyClient;
  if(pci->hwnd)
    WinFillRect( hps, &rect, SYSCLR_WINDOW );

  point.x = 0;
  point.y = pci->cyChar;
  GpiMove( hps, &point );
  point.x = rect.xRight;
  GpiLine( hps, &point );

  if(pci->hwnd)
  {
    point.x = pci->cxChar;             /* write patience message in bottom    */
    point.y = pci->cyDesc;             /* status area                         */
    GpiCharStringAt( hps, &point, strlen( globStrings.szPatient ),
                     globStrings.szPatient );
  }

 /* Wait for semaphore to be posted to ensure data (LL) is available          */

  rc = DosWaitEventSem( hev, 5L );     /* don't wait for semaphore too long   */
  if( rc == ERROR_TIMEOUT )            /* still reading drive, get out, a     */
    return;                            /* WM_PAINT message will be posted l8r */

 /* Write the amount of free and total space in the desired units on the      */
 /* status area in the bottom of the main window                              */

  if(pci->hwnd)
  {
    rect.xLeft   = 0;                  /* clear the bottom status area        */
    rect.xRight  = pci->cxClient;
    rect.yBottom = 0;
    rect.yTop    = pci->cyChar-1;
    WinFillRect( hps, &rect, CLR_PALEGRAY );
    WinDrawBorder (hps, &rect, 1, 1,
                   SYSCLR_BUTTONDARK, SYSCLR_BUTTONMIDDLE, DB_DEPRESSED);
  }

  point.x = pci->cxChar;               /* Write information in bottom status  */
  point.y = pci->cyDesc;               /* area - total size and total free    */

  switch( units )
  {
    case UNITS_BYTES:
      {
        WinLoadString( pci->hab, 0L, IDS_BYTES,  10, szUnit );
        sprintf( szText, "%s   %d %s %s   %d %s %s\0", Drive, totalfree,
                 szUnit, globStrings.szFree, totalsize, szUnit,
                 globStrings.szTotal );
        if( graphswapstat == YES && swapsize > 0 )
          sprintf( szTitle, "SWAPPER.DAT  %d %s", swapsize, szUnit );
      }
      break;
    case UNITS_KBYTES:
      {
        WinLoadString( pci->hab, 0L, IDS_KBYTES,  10, szUnit );
        sprintf( szText, "%s   %d %s %s   %d %s %s\0", Drive,
                 totalfree / 1024, szUnit, globStrings.szFree,
                 totalsize / 1024, szUnit, globStrings.szTotal );
        if( graphswapstat == YES && swapsize > 0 )
          sprintf( szTitle, "SWAPPER.DAT  %d %s", swapsize/1024, szUnit );
      }
      break;
    case UNITS_MEGS:
      {
        WinLoadString( pci->hab, 0L, IDS_MBYTES,  10, szUnit );
        sprintf( szText, "%s   %6.1f %s %s   %6.1f %s %s\0", Drive,
                 ((float)totalfree) / (1024*1024), szUnit, globStrings.szFree,
                 ((float)totalsize) / (1024*1024), szUnit, globStrings.szTotal );
        if( graphswapstat == YES && swapsize > 0 )
          sprintf( szTitle, "SWAPPER.DAT  %6.1f %s",
                   ((float)swapsize) / (1024*1024), szUnit );
      }
      break;
    default:
      break;
  }
  GpiCharStringAt( hps, &point, strlen( szText ), szText );
  if( graphswapstat == YES && swapsize > 0 )
  {
    point.x = pci->cxChar;
    point.y = 2*pci->cyDesc + pci->cyChar;
    GpiCharStringAt( hps, &point, strlen( szTitle ), szTitle );
  }

  if(pci->hwnd)
  {
    WinQueryWindowRect (pci->hwnd, &rect);
    if( rect.xRight < 10*pci->cxChar || rect.yTop < 5*pci->cyChar )
    {
      WinSetPointer( HWND_DESKTOP, WinQuerySysPointer( HWND_DESKTOP,
                     SPTR_ARROW, FALSE ) );
      return;
    }
  }

 /* Write the instructions at the top of the main window                      */

  if(pci->hwnd)
  {
    point.x = pci->cxChar;
    point.y = pci->cyClient - 4*pci->cyDesc;
    GpiCharStringAt( hps, &point, strlen( globStrings.szInstruct ),
                     globStrings.szInstruct );
  }

  /* Determine center of pie chart and radius using relative coordinates, then*/
  /* draw the pie chart with a blank line. Note: Angles are in FIXED.         */

  point.x = pci->ptctr.x;
  point.y = pci->ptctr.y;
  GpiMove( hps, &point );
  GpiSetArcParams( hps, &arcp );
  GpiFullArc( hps, DRO_OUTLINE, pci->radius );

  /* Determine starting address of linked list and maximum number of pies     */

  if( graphlevel == GRAPH_LEVEL_1ST )
    item = startsort1st;               /* 1st level directories only, will    */
  else                                 /* use cumulative sizes                */
    item = startsort;                  /* all subdirectories wanted           */
  if( graphtype == GRAPH_TYPE_USED )
    maxpies = graphmaxpies - 1;        /* percentages based on used space only*/
  else                                 /* so we will ignore free space        */
    maxpies = graphmaxpies - 2;        /* percentages based on total space    */
  if( graphswapsize == YES && swapsize > 0 ) /* SWAPPER.DAT placed on own pie */
    maxpies = maxpies - 1;             /* reduce maximum number of pies by 1  */

  if( maxpies < 0 )
    maxpies = 0;

  /* From the biggest directory, draw each pie slice. Increment the   */
  /* color for each as we go along. Draw a black outline also.        */

  startangle = 0;                      /* initialize starting angle           */
  sweepangle = 0;                      /* and sweep angle for pie slices      */
  pctangle   = 0.0;
  shade = 0;                           /* used for color increments           */
  for( i = 0; i < maxpies; i++ )
  {
    if( i == 15 || i == 31 || i == 47) /* change pattern after 15 slices from */
    {                                  /* solid to dense5, also start color   */
      shade = -i;                      /* numbering back to 1 (blue)          */
      switch( i )
      {
        case( 15 ):
          GpiSetPattern( hps, GRAPH_SHADE_1 );
          break;
        case( 31 ):
          GpiSetPattern( hps, GRAPH_SHADE_2 );
          break;
        case( 47 ):
          GpiSetPattern( hps, GRAPH_SHADE_3 );
          break;
        default:
          GpiSetPattern( hps, GRAPH_SHADE_4 );
      }
    }
    GpiSetColor( hps, i+1+shade );
                                       /* use GpiBeginArea to we can fill the */
    GpiBeginArea( hps, BA_BOUNDARY | BA_WINDING ); /* pie with color  */
    GpiMove( hps, &point );            /* move to center of circle            */

    if( graphtype == GRAPH_TYPE_USED )
                                       /* calculate sweepangle with is the    */
    {                                  /* slice angle, % * 3.6 since 360/100  */
      if ( graphlevel == GRAPH_LEVEL_1ST )
                                       /* 1st level subdirectories only so use*/
      {                                /* cumulative size and convert to %    */
        if( item->level == 1 )
        {
          pct = item->cumpercent2;     /* percentage  */
          size = item->cumbytes;       /* actual size */
        }
        else                           /* root directory, don't use cumulative*/
        {
          pct = item->percent2;
          size = item->bytes;
        }
      }
      else                             /* normal, all directories wanted      */
      {
        pct = item->percent2;
        size = item->bytes;
      }
    }
    else                               /* % of total wanted instead           */
    {
      if ( graphlevel == GRAPH_LEVEL_1ST )
      {
        if( item->level == 1 )
        {
          pct = item->cumpercent1;
          size = item->cumbytes;
        }
        else                           /* root directory, don't use cumulative*/
        {
          pct = item->percent1;
          size = item->bytes;
        }
      }
      else                             /* normal, all directories wanted      */
      {
        pct = item->percent1;
        size = item->bytes;
      }
    }
    /* using 65536 * instead of MAKEFIXED due to Help index problem           */
    /* problem may be because pct is float and MAKEFIXED needs LONG           */
    sweepangle = 3.6 * 65536 * pct;

    GpiPartialArc( hps, &point, pci->radius, startangle, sweepangle );
    GpiEndArea( hps );                 /* end the drawing area                */

    GpiSetColor ( hps, CLR_BLACK );    /* draw the outline of the pieslice*/
    GpiMove( hps, &point );            /* in black, it's nicer - really!      */
    GpiPartialArc( hps, &point, pci->radius, startangle, sweepangle / 2 );
    GpiQueryCurrentPosition( hps, &ptpct );
    GpiMove( hps, &point );            /* in black, it's nicer - really!      */
    GpiPartialArc( hps, &point, pci->radius, startangle, sweepangle );
    GpiLine( hps, &point );

    if( graphpercent == YES || graphsize == YES )
      fnDrawPercent( hps, startangle, sweepangle, pct, size, &ptpct,
                     pci->cxChar, pci->cyChar );
    startangle = startangle + sweepangle;  /* increment starting angle        */
    pctangle   = pctangle + pct * 3.6;
    item->angle = pctangle;
                                       /* ok, get the next item in linked list*/
    if( graphlevel == GRAPH_LEVEL_1ST )
      item = (PDIRINFO)item->nextsort1st; /* 1st level directories            */
    else
      item = (PDIRINFO)item->nextsort; /* all directories wanted              */

    if( item == NULL )                 /* end of linked list, let's get out by*/
      i = maxpies;                     /* setting i to the limit, end for loop*/

  }

  /* Draw the pie slice for SWAPPER.DAT separately                            */

  if( graphswapsize == YES && swapsize > 0 )
  {
    size = swapsize;
    if( graphtype == GRAPH_TYPE_TOTAL )
      pct = (float)swapsize * 100.0 / (float)totalsize;
    else
      pct = (float)swapsize * 100.0 / (float)totalused;
    sweepangle = 3.6 * 65536 * pct;
    GpiSetPattern( hps, GRAPH_SHADE_SWAP );
    GpiSetColor ( hps, CLR_BLACK );    /* draw the outline of the pieslice    */
    GpiBeginArea( hps, BA_BOUNDARY | BA_WINDING );
    GpiMove( hps, &point );            /* in black, it's nicer - really!      */
    GpiPartialArc( hps, &point, pci->radius, startangle, sweepangle );
    GpiEndArea( hps );                 /* end the drawing area                */
    GpiSetPattern( hps, PATSYM_DEFAULT );
    GpiMove( hps, &point );            /* in black, it's nicer - really!      */
    GpiPartialArc( hps, &point, pci->radius, startangle, sweepangle / 2 );
    GpiQueryCurrentPosition( hps, &ptpct );
    GpiMove( hps, &point );            /* in black, it's nicer - really!      */
    GpiPartialArc( hps, &point, pci->radius, startangle, sweepangle );
    GpiLine( hps, &point );
    if( graphpercent == YES || graphsize == YES )
      fnDrawPercent( hps, startangle, sweepangle, pct, size, &ptpct,
                     pci->cxChar, pci->cyChar );
    startangle = startangle + sweepangle;  /* increment starting angle        */
    pctangle   = pctangle + pct * 3.6;
  }

  /* Draw the pie slices for other directories left and free space            */

  if( graphtype == GRAPH_TYPE_TOTAL )
  {
    /* using 65536 * instead of MAKEFIXED due to Help index problem           */
    sweepangle = MAKEFIXED(360,0) - 65536 * (totalfree * 3.6 * 100 / totalsize )
                 - startangle;
    pct = 100 - (float)totalfree * 100.0 / (float)totalsize - pctangle / 3.6;
  }
  else
  {
    sweepangle = MAKEFIXED(360,0) - startangle;
    pct = 100 - pctangle / 3.6;
  }

  GpiSetPattern( hps, GRAPH_SHADE_OTHER ); /* Other directories pie slice     */
  GpiSetColor ( hps, CLR_BLACK );
  GpiBeginArea( hps, BA_BOUNDARY | BA_WINDING );
  GpiMove( hps, &point );              /* in black, it's nicer - really!      */
  GpiPartialArc( hps, &point, pci->radius, startangle, sweepangle );
  GpiEndArea( hps );
  GpiSetPattern( hps, PATSYM_DEFAULT );
  GpiMove( hps, &point );
  GpiPartialArc( hps, &point, pci->radius, startangle, sweepangle / 2 );
  GpiQueryCurrentPosition( hps, &ptpct );
  GpiMove( hps, &point );              /* draw the famous black pie outline   */
  GpiPartialArc( hps, &point, pci->radius, startangle, sweepangle );
  GpiLine( hps, &point );

  if( graphpercent == YES || graphsize == YES )
  {
    if( graphtype == GRAPH_TYPE_USED )
      size = (ULONG)( totalused * pct / 100 );
    else
      size = (ULONG)( totalsize * pct / 100 );
    fnDrawPercent( hps, startangle, sweepangle, pct, size, &ptpct,
                   pci->cxChar, pci->cyChar );
  }

  if( graphtype == GRAPH_TYPE_TOTAL && (graphpercent == YES || graphsize == YES) )
  {
    startangle = startangle + sweepangle;
    sweepangle = MAKEFIXED(360,0) - startangle;
    GpiMove( hps, &point );
    GpiPartialArc( hps, &point, pci->radius, startangle, sweepangle / 2 );
    GpiQueryCurrentPosition( hps, &ptpct );
    pctangle = pctangle + 3.6 * pct;
    pct = 100 - pctangle / 3.6;
    fnDrawPercent( hps, startangle, sweepangle, pct, totalfree, &ptpct,
                   pci->cxChar, pci->cyChar );
  }

}

/******************************************************************************/
/*                                                                            */
/* fnDrawPercent: Write the % values near each pie slice.                     */
/*                                                                            */
/******************************************************************************/

VOID fnDrawPercent( HPS hps, FIXED angle, FIXED sweep, float percent,
                    ULONG actsize, PPOINTL point, LONG cx, LONG cy )

{

  CHAR szText[80];

  if( percent > 1.99 )                 /* forget about pie slices that are    */
  {                                    /* less than 2%, leave out             */
                                       /* right side of the circle            */
    if( ( angle + sweep / 2 > MAKEFIXED(90,0) ) &&
        ( angle + sweep / 2 < MAKEFIXED(270,0) ) )
    {                                               /* left side of pie       */
      if( angle + sweep / 2 > MAKEFIXED(180,0) )
        if( angle + sweep / 2 > MAKEFIXED(225,0) )
          point->y = point->y - cy;                 /* bottom left            */
        else
          point->y = point->y - (cy/2);
      if( angle + sweep / 2 < MAKEFIXED(135,0) )
        point->y = point->y + (cy/2);
      if( graphsize == YES && units == UNITS_BYTES )
        point->x = point->x - 13 * cx;
      else
        point->x = point->x - 7 * cx;
      if( graphsize == YES )
      {
        switch( units )
        {
          case UNITS_MEGS:
            sprintf( szText, "%-3.1f", ((float)actsize/(1024*1024)) );
            break;
          case UNITS_KBYTES:
            sprintf( szText, "%-6.0f", ((float)actsize/1024) );
            break;
          case UNITS_BYTES:
            sprintf( szText, "%-9.0d", actsize );
            break;
          default:
            break;
        }
      }
      else
        sprintf( szText, "%2.1f%%", percent );
    }
    else                                            /* right side of pie      */
    {
      if( angle + sweep / 2 > MAKEFIXED(270,0) )    /* bottom right           */
        point->y = point->y - cy;
      else                                          /* top right              */
      {
        if( angle + sweep / 2 > MAKEFIXED(45,0) )
          point->y = point->y + (cy/2);
      }
      point->x = point->x + cx;
      if( graphsize == YES )
      {
        switch( units )
        {
          case UNITS_MEGS:
            sprintf( szText, "%3.1f", ((float)actsize/(1024*1024)) );
            break;
          case UNITS_KBYTES:
            sprintf( szText, "%6.0f", ((float)actsize/1024) );
            break;
          case UNITS_BYTES:
            sprintf( szText, "%9.0d", actsize );
            break;
          default:
            break;
        }
      }
      else
        sprintf( szText, "%-2.1f%%", percent );
    }

    GpiCharStringAt( hps, point, strlen( szText ), szText );

  }

}

/******************************************************************************/
/*                                                                            */
/* fnMenuBarUpdate: Insert/Delete menu items in Drives menu bar option.       */
/*                                                                            */
/******************************************************************************/

VOID fnMenuBarUpdate( HWND hwnd )

{
  SHORT    i;
  PDRIVES  item;
  HWND     hwndPulldown;
  MENUITEM mi;
  USHORT   menuItem = 0;

  WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
             FID_MENU), MM_QUERYITEM, MPFROM2SHORT(IDM_DRIVE, TRUE),
             (MPARAM)&mi);
  hwndPulldown = mi.hwndSubMenu;

  /* Delete all items in Drives menu pull-down */
  while(WinSendMsg(hwndPulldown, MM_DELETEITEM,
        MPFROM2SHORT(menuItem++, TRUE), NULL) != 0);

  /* Add drives in Drives menu pull-down */
  mi.iPosition = MIT_END;
  mi.afStyle = MIS_TEXT;
  mi.afAttribute = 0;
  mi.hwndSubMenu = 0;
  mi.hItem = 0;

  i = 0;
  item = drives;
  while( item )
  {
    mi.id = MENU_DRIVE + item->drivenumber;
    WinSendMsg(hwndPulldown, MM_INSERTITEM, (MPARAM)&mi,
               (PSZ)item->driveletter);
    if( item->drivenumber == DriveNumber )
      WinSendDlgItemMsg (WinQueryWindow(hwnd,QW_PARENT),
                         (ULONG) FID_MENU,
                         (ULONG) MM_SETITEMATTR,
                         MPFROM2SHORT(mi.id,TRUE),
                         MPFROM2SHORT(MIA_CHECKED,MIA_CHECKED));
    item = (PDRIVES)item->next;
    i++;
  }

}

/******************************************************************************/
/*                                                                            */
/* wpLegendWndProc: Window procedure for Legend PM window. Handle is          */
/*                  hwndLegend.                                               */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpLegendWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{

 MRESULT     mr = (MRESULT) FALSE;
 HPS         hps;
 static HAB  hab;
 static PLEGENDINFO pli;
 FONTMETRICS fm;

 switch (msg)
    {

     case WM_CREATE:                   /* Creation of legend window           */
        {

           pli = (PLEGENDINFO)malloc(sizeof(LEGENDINFO));
           pli->hwnd = hwnd;

           hab = WinQueryAnchorBlock( hwnd );
           pli->hab = hab;

           pli->hwndHscroll = WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ),
                              FID_HORZSCROLL );
           pli->hwndVscroll = WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ),
                              FID_VERTSCROLL );

           WinSendMsg(hwnd, UM_FONT, 0L, 0L);
        }
        break;

     case UM_FONT:
       {
         CHAR szText[80];

         sprintf(szText, "%d.%-s", fontsize, fontname);
         WinSetPresParam(hwnd, PP_FONTNAMESIZE, sizeof(szText), szText);

         hps = WinBeginPaint (hwnd, 0L, 0L);

         GpiQueryFontMetrics( hps, sizeof(FONTMETRICS), &fm );
         pli->cxChar      = (LONG)fm.lAveCharWidth;
         pli->cxCaps      = (LONG)fm.lMaxCharInc;
         pli->cyChar      = (LONG)fm.lMaxBaselineExt;
         pli->cyDesc      = (LONG)fm.lMaxDescender;
         pli->cyHeight    = (LONG)fm.lXHeight;
         pli->cyLowHeight = (LONG)fm.lLowerCaseAscent;

         WinEndPaint( hps );
       }
       break;

     case UM_LEGEND:
        pli->sHscrollPos = 0;
        pli->sVscrollPos = 0;

        WinShowWindow( WinQueryWindow( hwnd, QW_PARENT ), FALSE );
        WinSendMsg( WinQueryWindow( hwnd, QW_PARENT), WM_SIZE,
                    MPFROM2SHORT( pli->cxClient, pli->cyClient ),
                    MPFROM2SHORT( pli->cxClient, pli->cyClient ) );
        if( showlegend == YES )
          WinShowWindow( WinQueryWindow( hwnd, QW_PARENT ), TRUE  );
        break;

     case WM_SIZE:
        {
          APIRET rc;

          pli->cxClient = SHORT1FROMMP( mp2 );
          pli->cyClient = SHORT2FROMMP( mp2 );

          rc = DosWaitEventSem( hev, 5L );
          if( rc == ERROR_TIMEOUT )
            break;

          if( graphlevel == GRAPH_LEVEL_1ST )
          {
//          pli->cxTextTotal = 8 * pli->cxChar + pli->cxChar * char1stdirectories;
            pli->cxTextTotal = 3 * pli->cxChar + pli->cxChar * char1stdirectories;
            if( total1stdirectories < graphmaxpies - 2 )
              pli->cyLineTotal = total1stdirectories + 2;
            else
              pli->cyLineTotal = graphmaxpies;
          }
          else
          {
//          pli->cxTextTotal = 6 * pli->cxChar + pli->cxChar * chardirectories;
            pli->cxTextTotal = 3 * pli->cxChar + pli->cxChar * chardirectories;
            if( totaldirectories < graphmaxpies - 2 )
              pli->cyLineTotal = totaldirectories + 2;
            else
              pli->cyLineTotal = graphmaxpies;
          }

          pli->sHscrollMax = max( 0, pli->cxTextTotal - pli->cxClient );
          pli->sHscrollPos = min( pli->sHscrollPos, pli->sHscrollMax );

          WinSendMsg( pli->hwndHscroll, SBM_SETSCROLLBAR,
                      MPFROM2SHORT( pli->sHscrollPos, 0 ),
                      MPFROM2SHORT( 0, pli->sHscrollMax ) );

          WinEnableWindow( pli->hwndHscroll, pli->sHscrollMax ? TRUE : FALSE );

          pli->sVscrollMax = max( 0, pli->cyLineTotal - pli->cyClient / pli->cyChar );
          pli->sVscrollPos = min( pli->sVscrollPos, pli->sVscrollMax );

          WinSendMsg( pli->hwndVscroll, SBM_SETSCROLLBAR,
                      MPFROM2SHORT( pli->sVscrollPos, 0 ),
                      MPFROM2SHORT( 0, pli->sVscrollMax ) );

          WinEnableWindow( pli->hwndVscroll, pli->sVscrollMax ? TRUE : FALSE );

        }
        break;

     case WM_PAINT:                    /* Draw the legend contents here ...   */
        {
          HPS   hps;
          RECTL rect;

          hps = WinBeginPaint (pli->hwnd, 0L, 0L);
          WinQueryWindowRect (pli->hwnd, &rect);
          WinFillRect( hps, &rect, SYSCLR_WINDOW );

          fnDrawLegend( hps, pli );

          WinEndPaint (hps);
        }
        break;

//   case WM_ACTIVATE:
//     {
//       if( SHORT1FROMMP(mp1)==TRUE && HWNDFROMMP(mp2)==hwndLegend )
//       {
//         WinSetActiveWindow( HWND_DESKTOP, hwndDirlist );
//         WinSetActiveWindow( HWND_DESKTOP, hwndAll     );
//         WinSetActiveWindow( HWND_DESKTOP, hwndFrame   );
//         WinSetActiveWindow( HWND_DESKTOP, hwndLegend  );
//       }
//       mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
//     }
//     break;

     case WM_HSCROLL:
        {
          LONG sHscrollInc;

          switch( SHORT2FROMMP( mp2 ))
          {
            case SB_LINELEFT:
              sHscrollInc = -pli->cxChar;
              break;

            case SB_LINERIGHT:
              sHscrollInc = pli->cxChar;
              break;

            case SB_PAGELEFT:
              sHscrollInc = -(8 * pli->cxChar);
              break;

            case SB_PAGERIGHT:
              sHscrollInc = 8 * pli->cxChar;
              break;

            case SB_SLIDERPOSITION:
              sHscrollInc = SHORT1FROMMP( mp2 ) - pli->sHscrollPos;
              break;

            default:
              sHscrollInc = 0;
              break;
          }

          sHscrollInc = max( -pli->sHscrollPos,
                        min( sHscrollInc, pli->sHscrollMax - pli->sHscrollPos ) );
          if( sHscrollInc != 0 )
          {
            pli->sHscrollPos += sHscrollInc;
            WinScrollWindow( hwnd, -sHscrollInc, 0,
                             (PRECTL)NULL, (PRECTL)NULL,
                             (HRGN)NULLHANDLE, (PRECTL)NULL,
                             SW_INVALIDATERGN );
            WinSendMsg( pli->hwndHscroll, SBM_SETPOS, MPFROMSHORT( pli->sHscrollPos ),
                        NULL );
          }
        }
        break;

     case WM_VSCROLL:
        {
          LONG sVscrollInc;

          switch( SHORT2FROMMP( mp2 ))
          {
            case SB_LINEUP:
              sVscrollInc = -1;
              break;

            case SB_LINEDOWN:
              sVscrollInc = 1;
              break;

            case SB_PAGEUP:
              sVscrollInc = min( -1, -pli->cyClient / pli->cyChar );
              break;

            case SB_PAGEDOWN:
              sVscrollInc = max( 1, pli->cyClient / pli->cyChar );
              break;

            case SB_SLIDERTRACK:
              sVscrollInc = SHORT1FROMMP( mp2 ) - pli->sVscrollPos;
              break;

            default:
              sVscrollInc = 0;
              break;
          }

          sVscrollInc = max( -pli->sVscrollPos, min( sVscrollInc, pli->sVscrollMax -
                        pli->sVscrollPos ) );
          if( sVscrollInc != 0 )
          {
            pli->sVscrollPos += sVscrollInc;
            WinScrollWindow( hwnd, 0, pli->cyChar * sVscrollInc,
                             (PRECTL)NULL, (PRECTL)NULL,
                             (HRGN)NULLHANDLE, (PRECTL)NULL,
                             SW_INVALIDATERGN );
            WinSendMsg( pli->hwndVscroll, SBM_SETPOS, MPFROMSHORT( pli->sVscrollPos ),
                        NULL );
            WinUpdateWindow( hwnd );
          }

        }
        break;

     case UM_CLOSE:
        free( pli );
        break;
                                       /* Close the window - actually just    */
     case WM_CLOSE:                    /* hide it and remove check mark from  */
        {                              /* pull-down choice                    */
          WinShowWindow( WinQueryWindow(hwnd,QW_PARENT), FALSE );
          showlegend = NO;
          WinSendDlgItemMsg (hwndFrame,
                             (ULONG) FID_MENU,
                             (ULONG) MM_SETITEMATTR,
                             MPFROM2SHORT(IDM_LEGEND,TRUE),
                             MPFROM2SHORT(MIA_CHECKED, showlegend ?
                                          MIA_CHECKED : 0));
        }                              /* end case (WM_CLOSE)                 */
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
        break;

    }                                  /* end main switch statement           */

 return mr;

}

/******************************************************************************/
/*                                                                            */
/* fnDrawLegend: Draw legend text.                                            */
/*                                                                            */
/******************************************************************************/

VOID fnDrawLegend( HPS hps, PLEGENDINFO pli )

{

  POINTL      point;
  PDIRINFO    item;
  ULONG       i, j, maxpies, shade;
  APIRET      rc;
  CHAR        szText[80];

  /* Wait for semaphore to be posted to ensure data (LL) is available */

  rc = DosWaitEventSem( hev, 5L );
  if( rc == ERROR_TIMEOUT )
    return;

  /* Determine starting address of linked list & max number of pies.  */

  if( graphlevel == GRAPH_LEVEL_1ST )
    item = startsort1st;
  else
    item = startsort;
  if( graphtype == GRAPH_TYPE_USED )
    maxpies = graphmaxpies - 1;
  else
    maxpies = graphmaxpies - 2;
  if( graphswapsize == YES && swapsize > 0 )
    maxpies = maxpies - 1;

  if( maxpies < 0 )
    maxpies = 0;

  /* From the biggest directory, we will draw each legend box.        */
  /* Increment the color for each as we go along. Draw black outline  */
  /* also (looks better).                                             */

  shade = 0;
  j     = 0;
  for( i = 0; i < maxpies; i++ )
  {
    if( i == 15 || i == 31 || i == 47) /* change pattern after 15 slices from */
    {                                  /* solid to dense5, also start color   */
      shade = -i;                      /* numbering back to 1 (blue)          */
      switch( i )
      {
        case( 15 ):
          GpiSetPattern( hps, GRAPH_SHADE_1 );
          break;
        case( 31 ):
          GpiSetPattern( hps, GRAPH_SHADE_2 );
          break;
        case( 47 ):
          GpiSetPattern( hps, GRAPH_SHADE_3 );
          break;
        default:
          GpiSetPattern( hps, GRAPH_SHADE_4 );
      }
    }
    j++;
    GpiSetColor( hps, i+1+shade );
                                       /* use GpiBeginArea to we can fill     */
    GpiBeginArea( hps, BA_BOUNDARY | BA_WINDING );/*the legend w/color*/
    point.x = pli->cxChar - pli->sHscrollPos;
    point.y = pli->cyClient - pli->cyChar * (i + 1 -pli->sVscrollPos) + pli->cyDesc;
    GpiMove( hps, &point );
    point.x = 3 * pli->cxChar - pli->sHscrollPos;
    point.y = point.y + pli->cyHeight;
    GpiBox( hps, DRO_OUTLINE, &point, 0L, 0L );
    GpiEndArea( hps );         /* end the drawing area                */

    GpiSetColor ( hps, CLR_BLACK ); /* draw the outline of legend box */
    point.x = pli->cxChar - pli->sHscrollPos; /* using a blank line,looks nicer!*/
    point.y = pli->cyClient - pli->cyChar * (i + 1 - pli->sVscrollPos) + pli->cyDesc;
    GpiMove( hps, &point );
    point.x = 3 * pli->cxChar - pli->sHscrollPos;
    point.y = point.y + pli->cyHeight;
    GpiBox( hps, DRO_OUTLINE, &point, 0L, 0L );

    point.x = 4 * pli->cxChar - pli->sHscrollPos;
    point.y = pli->cyClient - pli->cyChar * (i + 1 - pli->sVscrollPos) + pli->cyDesc;
    if( item->level == 0 )
      GpiCharStringAt( hps, &point, strlen(globStrings.directory),
                       globStrings.directory );
    else
      GpiCharStringAt( hps, &point, strlen( item->fulldirname),
                       item->fulldirname);
                                      /* ok, get next item in linked list    */
    if( graphlevel == GRAPH_LEVEL_1ST )
      item = (PDIRINFO)item->nextsort1st; /* 1stlevel directories only*/
    else
      item = (PDIRINFO)item->nextsort; /* we want all subdirectories  */

    if( item == NULL )         /* end of linked list, let's get out by*/
      i = maxpies;             /* setting i to the limit, end for loop*/
  }

  /* Draw the legend for other directories left and free space        */
  /* (if wanted)                                                      */

  if( graphswapsize == YES && swapsize > 0 )
  {
    GpiSetPattern( hps, GRAPH_SHADE_SWAP );
    GpiBeginArea( hps, BA_BOUNDARY | BA_WINDING );
    point.x = pli->cxChar - pli->sHscrollPos;
    point.y = pli->cyClient - pli->cyChar * (j + 1 - pli->sVscrollPos) +
              pli->cyDesc;
    GpiMove( hps, &point );
    point.x = 3 * pli->cxChar - pli->sHscrollPos;
    point.y = point.y + pli->cyHeight;
    GpiBox( hps, DRO_OUTLINE, &point, 0L, 0L );
    GpiEndArea( hps );
    GpiSetPattern( hps, PATSYM_DEFAULT );
    point.x = pli->cxChar - pli->sHscrollPos;
    point.y = pli->cyClient - pli->cyChar * (j + 1 - pli->sVscrollPos) +
              pli->cyDesc;
    GpiMove( hps, &point );
    point.x = 3 * pli->cxChar - pli->sHscrollPos;
    point.y = point.y + pli->cyHeight;
    GpiBox( hps, DRO_OUTLINE, &point, 0L, 0L );
    point.x = 4 * pli->cxChar - pli->sHscrollPos;
    point.y = pli->cyClient - pli->cyChar * (j + 1 - pli->sVscrollPos) +
              pli->cyDesc;
    GpiCharStringAt( hps, &point, strlen( "SWAPPER.DAT" ), "SWAPPER.DAT" );
    j++;
  }

  /* Draw the legend for other directories left and free space        */
  /* (if wanted)                                                      */

  WinLoadString (pli->hab, 0L, IDS_OTHER, 20, szText);
  GpiSetPattern( hps, GRAPH_SHADE_OTHER );
  GpiBeginArea( hps, BA_BOUNDARY | BA_WINDING );
  point.x = pli->cxChar - pli->sHscrollPos;
  point.y = pli->cyClient - pli->cyChar * (j + 1 - pli->sVscrollPos) + pli->cyDesc;
  GpiMove( hps, &point );
  point.x = 3 * pli->cxChar - pli->sHscrollPos;
  point.y = point.y + pli->cyHeight;
  GpiBox( hps, DRO_OUTLINE, &point, 0L, 0L );
  GpiEndArea( hps );
  GpiSetPattern( hps, PATSYM_DEFAULT );
  point.x = pli->cxChar - pli->sHscrollPos;
  point.y = pli->cyClient - pli->cyChar * (j + 1 - pli->sVscrollPos) + pli->cyDesc;
  GpiMove( hps, &point );
  point.x = 3 * pli->cxChar - pli->sHscrollPos;
  point.y = point.y + pli->cyHeight;
  GpiBox( hps, DRO_OUTLINE, &point, 0L, 0L );
  point.x = 4 * pli->cxChar - pli->sHscrollPos;
  point.y = pli->cyClient - pli->cyChar * (j + 1 - pli->sVscrollPos) + pli->cyDesc;
  GpiCharStringAt( hps, &point, strlen( szText ), szText );

  if( graphtype == GRAPH_TYPE_TOTAL )
  {                                    /*                                     */
    WinLoadString (pli->hab, 0L, IDS_FREE, 20, szText);
    j++;
    point.x = pli->cxChar - pli->sHscrollPos;
    point.y = pli->cyClient - pli->cyChar * (j + 1 - pli->sVscrollPos) + pli->cyDesc;
    GpiMove( hps, &point );
    point.x = 3 * pli->cxChar - pli->sHscrollPos;
    point.y = point.y + pli->cyHeight;
    GpiBox( hps, DRO_OUTLINE, &point, 0L, 0L );
    point.x = 4 * pli->cxChar - pli->sHscrollPos;
    point.y = pli->cyClient - pli->cyChar * (j + 1 - pli->sVscrollPos) + pli->cyDesc;
    GpiCharStringAt( hps, &point, strlen( szText ), szText );
  }

}

/******************************************************************************/
/*                                                                            */
/* wpDirlistWndProc: Window procedure for Directory List window. Handle is    */
/*                   hwndDirlist.                                             */
/*                                                                            */
/* Calls: fnListBoxEntry                                                      */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpDirlistWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{

 MRESULT     mr = (MRESULT) FALSE;
 CHAR        szTitle[50], szText[80];
 static HAB  hab;
 static LONG cxChar, cyChar, cyDesc, cxClient, cyClient;
 static CHAR szListHdrLeft[80], szListHdrRight[80];
 static HWND hwndPopup;

 switch (msg)
    {
     case WM_CREATE:                   /* Window just got created. Create the */
        {                              /* listbox which goes on top of the    */
          RECTL  rectl;                /* frame. Then start the thread to     */
          TID    tidThread;            /* insert items into the listbox       */

          hab = WinQueryAnchorBlock( hwnd );

          WinSendMsg(hwnd, UM_FONT, 0L, 0L);

          WinLoadString( hab, 0L, IDS_LISTHDRLEFT, sizeof( szListHdrLeft ),
                                     szListHdrLeft );
          WinLoadString( hab, 0L, IDS_LISTHDRRIGHT, sizeof( szListHdrRight ),
                                     szListHdrRight );

          hab = WinQueryAnchorBlock( hwnd );

          WinQueryWindowRect( hwnd, &rectl );
//        hwndList = WinCreateWindow( hwnd, WC_LISTBOX, NULL,
//                                 LS_HORZSCROLL | LS_NOADJUSTPOS,
//                                 0, 0, rectl.xRight, rectl.yTop,
//                                 hwnd, HWND_TOP,
//                                 ID_LISTBOX, NULL, NULL);
          hwndList = WinCreateWindow( hwnd, WC_CONTAINER, NULL,
                                   CCS_MINIRECORDCORE | CCS_SINGLESEL | CCS_READONLY,
//                                 0, 0, rectl.xRight, rectl.yTop,
                                   0, 0, 0, 0,
                                   hwnd, HWND_TOP,
                                   ID_LISTBOX, NULL, NULL);
                                       /* load directory list popup menu      */
          hwndPopup = WinLoadMenu( hwnd, (HMODULE)NULL, idPopup );


          DosCreateThread( &tidThread, (PFNTHREAD)fnListBoxEntry, 0L, 0L,
                           STACKSIZE );
        }                              /* end case (WM_CREATE)                */
        break;

#if 0
     case WM_PAINT:                    /* Repaint the frame window            */
        {                              /* Nothing exciting, just normal stuff */
          RECTL  rectl;
          HPS    hps;
          POINTL point;

          hps = WinBeginPaint (hwnd, 0L, 0L);

          rectl.xLeft   = 0;
          rectl.xRight  = cxClient;
          rectl.yBottom = 0;
          rectl.yTop    = cyClient;
          WinFillRect (hps, &rectl, SYSCLR_WINDOW);

          point.x = cxChar;
          point.y = cyClient - cyChar + cyDesc;
          if( directorytree == RIGHT )
            GpiCharStringAt( hps, &point, strlen( szListHdrRight ),
                             szListHdrRight );
          else
            GpiCharStringAt( hps, &point, strlen( szListHdrLeft ),
                             szListHdrLeft );

          WinEndPaint (hps);
        }                              /* end case (WM_PAINT)                 */
        break;

     case UM_DIRLIST:
        {
          RECTL  rectl;

          rectl.xLeft   = 0;
          rectl.xRight  = cxClient;
          rectl.yBottom = cyClient - cyChar;
          rectl.yTop    = cyClient;
          WinInvalidateRect (hwnd, &rectl, FALSE);
        }
        break;
#endif

     case UM_FONT:
       {
         HPS    hps;
         FONTMETRICS fm;
         CHAR szText[80];

         sprintf(szText, "%d.%-s", fontsize, fontname);
         WinSetPresParam(hwnd, PP_FONTNAMESIZE, sizeof(szText), szText);
         WinSetPresParam(hwndList, PP_FONTNAMESIZE, sizeof(szText), szText);

         hps = WinBeginPaint( hwnd, 0L, 0L );

         GpiQueryFontMetrics( hps, sizeof(FONTMETRICS), &fm );
         cxChar = (LONG)fm.lAveCharWidth;
         cyChar = (LONG)fm.lMaxBaselineExt;
         cyDesc = (LONG)fm.lMaxDescender;

         WinEndPaint( hps );
       }
       break;

     case WM_SIZE:                     /* Window got sized, re-position the   */
        {                              /* listbox on top of frame window      */
          RECTL rectl;

          cxClient = SHORT1FROMMP( mp2 );
          cyClient = SHORT2FROMMP( mp2 );

          WinSetWindowPos (WinWindowFromID (hwnd, ID_LISTBOX), HWND_TOP,
//                         0, 0, cxClient, cyClient - cyChar,
                           0, 0, cxClient, cyClient,
                           SWP_SIZE | SWP_MOVE | SWP_SHOW);
          WinSetFocus (HWND_DESKTOP, WinWindowFromID (hwnd, ID_LISTBOX));
        }                              /* end case (WM_SIZE)                  */
        break;

//   case WM_ACTIVATE:
//     {
//       if( SHORT1FROMMP(mp1)==TRUE && HWNDFROMMP(mp2)==hwndDirlist )
//       {
//         WinSetActiveWindow( HWND_DESKTOP, hwndLegend  );
//         WinSetActiveWindow( HWND_DESKTOP, hwndAll     );
//         WinSetActiveWindow( HWND_DESKTOP, hwndFrame   );
//         WinSetActiveWindow( HWND_DESKTOP, hwndDirlist );
//       }
//       mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
//     }
//     break;

     case WM_BUTTON1DBLCLK:
     case WM_BUTTON2DBLCLK:
        WinSendMsg( hwnd, WM_COMMAND, MPFROM2SHORT( IDM_MAKEROOT, TRUE ), 0L );
        break;

     case WM_COMMAND:                  /* Application command processing here */
        {                              /* These are for popup menu commands   */
         switch (SHORT1FROMMP (mp1))   /* Message contained in mp1 variable   */
            {
             case IDM_MOVE:
                {
                  PDIRINFO item;
                  APIRET rc;
                  CHAR   szSText[80];

                  item = fnSelectItem( hab, hwnd );
                  if( item )
                  {
                    if (WinDlgBox (HWND_DESKTOP, hwnd,
                        wpDirectoryDlgProc, 0L, IDD_DIRECTORY, NULL ))
                    {
                      if( item->fulldirname[0] == globStrings.targetdir[0] )
                      {
                        rc = DosMove( item->fulldirname, globStrings.targetdir );
                        if( rc != 0 )
                        {
                          WinLoadString (hab, 0L, IDS_MOVETITLE, 50, szTitle);
                          WinLoadString (hab, 0L, IDS_MOVEERROR, 80, szSText);
                          sprintf( szText, "%s %d.", szSText, rc );
                          WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle,
                                         0, MB_ERROR | MB_OK | MB_MOVEABLE);
                        }
                      }
                      else
                      {
                        rc = DosCopy( item->fulldirname, globStrings.targetdir,
                                      DCPY_EXISTING | DCPY_FAILEAS );
                        if( rc != 0 )
                        {
                          WinLoadString (hab, 0L, IDS_MOVETITLE, 50, szTitle);
                          WinLoadString (hab, 0L, IDS_COPYERROR, 80, szSText);
                          sprintf( szText, "%s %d.", szSText, rc );
                          WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle,
                                         0, MB_ERROR | MB_OK | MB_MOVEABLE);
                        }
                        else
                        {
                          delstart = NULL;
                          dellast  = NULL;

                          rc = fnGetDeleteDirectoryInfo( item->fulldirname );
                          if( rc != 0 )
                          {
                            WinLoadString (hab, 0L, IDS_MOVETITLE, 50, szTitle);
                            WinLoadString (hab, 0L, IDS_DELETEERROR1, 80,
                                           szSText);
                            sprintf( szText, "%s %d.", szSText, rc );
                            WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle,
                                           0, MB_ERROR | MB_OK | MB_MOVEABLE);
                          }
                          else
                          {
                            rc = fnDeleteDirectories( );
                            if( rc != 0 )
                            {
                              WinLoadString (hab, 0L, IDS_MOVETITLE, 50,
                                             szTitle);
                              WinLoadString (hab, 0L, IDS_DELETEERROR2, 80,
                                             szSText);
                              sprintf( szText, "%s %d.", szSText, rc );
                              WinMessageBox (HWND_DESKTOP, hwnd, szText,
                                             szTitle, 0, MB_ERROR | MB_OK |
                                             MB_MOVEABLE);
                            }
                            fnFreeDeleteList();
                          }
                        }
                      }
                    }
                  }
                }                      /* end case IDM_MOVE                   */
                break;

             case IDM_DELETE:
              {

                PDIRINFO item;
                APIRET   rc;
                CHAR     szSText[80];

                item = fnSelectItem( hab, hwnd );
                if( item )
                  {
                    WinLoadString (hab, 0L, IDS_DELETETITLE, 50, szTitle);
                    WinLoadString (hab, 0L, IDS_DELETEWARN, 80, szSText);
                    sprintf( szText, "%s %s?", szSText, item->fulldirname );
                    if (WinMessageBox(HWND_DESKTOP, hwnd, szText, szTitle, 1,
                        MB_MOVEABLE | MB_YESNO |
                        MB_WARNING  | MB_DEFBUTTON2 ) == MBID_YES)
                    {

                      delstart = NULL;
                      dellast  = NULL;

                      rc = fnGetDeleteDirectoryInfo( item->fulldirname );
                      if( rc != 0 )
                      {
                        WinLoadString (hab, 0L, IDS_DELETEERROR1, 80, szSText);
                        sprintf( szText, "%s %d.", szSText, rc );
                        WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle,
                                       0, MB_ERROR | MB_OK | MB_MOVEABLE);
                      }
                      else
                      {
                        rc = fnDeleteDirectories( );
                        if( rc != 0 )
                        {
                          WinLoadString (hab, 0L, IDS_DELETEERROR2, 80, szSText);
                          sprintf( szText, "%s %d.", szSText, rc );
                          WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle,
                                         0, MB_ERROR | MB_OK | MB_MOVEABLE);
                        }
                        fnFreeDeleteList();
                      }
                    }
                  }
              }                        /* end case IDM_DELETE                 */
              break;

             case IDM_COPY:
                {
                  CHAR     szSText[80];
                  PDIRINFO item;

                  item = fnSelectItem( hab, hwnd );
                  if( item )
                  {
                    if (WinDlgBox (HWND_DESKTOP, hwnd,
                        wpDirectoryDlgProc, 0L, IDD_DIRECTORY, NULL ))
                    {
                      APIRET rc;
                      rc = DosCopy( item->fulldirname, globStrings.targetdir,
                                    DCPY_EXISTING | DCPY_FAILEAS );
                      if( rc != 0 )
                      {
                        WinLoadString (hab, 0L, IDS_COPYTITLE, 50, szTitle);
                        WinLoadString (hab, 0L, IDS_COPYERROR, 80, szSText);
                        sprintf( szText, "%s %d.", szSText, rc );
                        WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle,
                                       0, MB_ERROR | MB_OK | MB_MOVEABLE);
                      }
                    }
                  }
                }                      /* end case WM_COPY                    */
                break;

             case IDM_MAKEROOT:        /* use the selected directory as the   */
                {                      /* virtual root directory              */
                  CHAR     szSText[80];
                  PDIRINFO item;
                                       /* determine which is the selected dir */
                  item = fnSelectItem( hab, hwnd );
                  if( item )
                  {
                    if( !strncmp( globStrings.directory, item->directory,
                                  strlen(globStrings.directory)))
                    {                  /* selected dir is already root dir    */
                      WinLoadString (hab, 0L, IDS_MKRTTITLE, 50, szTitle);
                      WinLoadString (hab, 0L, IDS_MKRTERROR, 80, szSText);
                      sprintf( szText, "%s %s.", szSText, globStrings.directory );
                      WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle,
                                     0, MB_WARNING | MB_OK | MB_MOVEABLE);
                    }
                    else               /* ok, all normal, let's make the      */
                    {                  /* selected directory the root, great  */
                      BOOL oldshowrefresh = showrefresh;

                      strcpy( globStrings.searchdir, item->fulldirname );
                      strcpy( globStrings.directory, Drive );
                      strcat( globStrings.directory, item->directory );
                      rootdrive = NO;
                      showrefresh = NO;
                      fnRefreshDirectory( hwnd );
                      showrefresh = oldshowrefresh;
                    }
                  }
                }                      /* end case IDM_MAKEROOT               */
                break;

             case IDM_ROOT:            /* go to the real root directory       */
                {
                  strcpy( szTitle, Drive );
                  strcat( szTitle, "(ROOT)" );
                  if( !strncmp( globStrings.directory, szTitle,
                       strlen( globStrings.directory)))
                  {                    /* real root directory already root    */
                    WinLoadString (hab, 0L, IDS_ROOTTITLE, 50, szTitle);
                    WinLoadString (hab, 0L, IDS_ROOTERROR, 80, szText);
                    WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle,
                                  0, MB_WARNING | MB_OK | MB_MOVEABLE);
                  }
                  else
                  {
                    BOOL oldshowrefresh = showrefresh;

                    rootdrive = YES;
                    showrefresh = NO;
                    fnRefreshDirectory( hwnd );
                    showrefresh = oldshowrefresh;
                  }
                }                      /* end case WM_ROOT                    */
                break;

             default:                  /* Let PM handle the rest of messages  */
                mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
                break;
            }                          /* end switch                          */
        }                              /* end case (WM_COMMAND)               */
        break;

     case WM_BUTTON2DOWN:              /* button 2 click, display menu popup  */
        WinPopupMenu( hwnd, WinQueryWindow( hwnd, QW_PARENT ),
                      hwndPopup, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1), 0,
                      PU_HCONSTRAIN | PU_VCONSTRAIN |
                      PU_KEYBOARD | PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 );
        break;

     case WM_CLOSE:                    /* Close the window - actually just    */
        {                              /* hide it and remove check mark       */
          WinShowWindow( WinQueryWindow(hwnd,QW_PARENT), FALSE );
          showdirlist = NO;
          WinSendDlgItemMsg (hwndFrame,
                             (ULONG) FID_MENU,
                             (ULONG) MM_SETITEMATTR,
                             MPFROM2SHORT(IDM_DIRLIST,TRUE),
                             MPFROM2SHORT(MIA_CHECKED, showdirlist ?
                                          MIA_CHECKED : 0));
        }                              /* end case (WM_CLOSE)                 */
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
        break;

    }                                  /* end main switch statement           */

 return mr;

}

/******************************************************************************/
/*                                                                            */
/* wpAllWndProc: Window procedure for All Summary PM window. Handle is        */
/*               hwndAll. Calls fnDrawSummary().                              */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpAllWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{

 MRESULT     mr = (MRESULT) FALSE;
 HPS         hps;
 FONTMETRICS fm;
 static HAB  hab;
 static PLEGENDINFO pli;

 switch (msg)
    {

     case WM_CREATE:
        {

           pli = (PLEGENDINFO)malloc(sizeof(LEGENDINFO));
           pli->hwnd = hwnd;

           WinSendMsg(hwnd, UM_FONT, 0L, 0L);

           hab = WinQueryAnchorBlock( hwnd );
           pli->hab = hab;
           pli->hwndHscroll = WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ),
                              FID_HORZSCROLL );
           pli->hwndVscroll = WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ),
                              FID_VERTSCROLL );

        }
        break;

     case UM_ALL:
        pli->sHscrollPos = 0;
        pli->sVscrollPos = 0;

        WinShowWindow( WinQueryWindow( hwnd, QW_PARENT ), FALSE );
        WinSendMsg( WinQueryWindow( hwnd, QW_PARENT), WM_SIZE,
                    MPFROM2SHORT( pli->cxClient, pli->cyClient ),
                    MPFROM2SHORT( pli->cxClient, pli->cyClient ) );
        if( showall == YES )
          WinShowWindow( WinQueryWindow( hwnd, QW_PARENT ), TRUE  );
        break;

     case UM_FONT:
       {
         CHAR szText[80];

         sprintf(szText, "%d.%-s", fontsize, fontname);
         WinSetPresParam(hwnd, PP_FONTNAMESIZE, sizeof(szText), szText);

         hps = WinBeginPaint (hwnd, 0L, 0L);

         GpiQueryFontMetrics( hps, sizeof(FONTMETRICS), &fm );
         pli->cxChar      = (LONG)fm.lAveCharWidth;
         pli->cxCaps      = (LONG)fm.lMaxCharInc;
         pli->cyChar      = (LONG)fm.lMaxBaselineExt;
         pli->cyDesc      = (LONG)fm.lMaxDescender;
         pli->cyHeight    = (LONG)fm.lXHeight;
         pli->cyLowHeight = (LONG)fm.lLowerCaseAscent;

         WinEndPaint( hps );

       }
       break;

     case WM_SIZE:
        {
          APIRET rc;

          pli->cxClient = SHORT1FROMMP( mp2 );
          pli->cyClient = SHORT2FROMMP( mp2 );

          rc = DosWaitEventSem( hev, 5L );
          if( rc == ERROR_TIMEOUT )
            break;

          pli->cxTextTotal = pli->cxChar * 50;
          pli->cyLineTotal = totaldrives+1;
          pli->sHscrollMax = max( 0, pli->cxTextTotal - pli->cxClient );
          pli->sHscrollPos = min( pli->sHscrollPos, pli->sHscrollMax );

          WinSendMsg( pli->hwndHscroll, SBM_SETSCROLLBAR,
                      MPFROM2SHORT( pli->sHscrollPos, 0 ),
                      MPFROM2SHORT( 0, pli->sHscrollMax ) );

          WinEnableWindow( pli->hwndHscroll, pli->sHscrollMax ? TRUE : FALSE );

          pli->sVscrollMax = max( 0, pli->cyLineTotal - pli->cyClient / pli->cyChar );
          pli->sVscrollPos = min( pli->sVscrollPos, pli->sVscrollMax );

          WinSendMsg( pli->hwndVscroll, SBM_SETSCROLLBAR,
                      MPFROM2SHORT( pli->sVscrollPos, 0 ),
                      MPFROM2SHORT( 0, pli->sVscrollMax ) );

          WinEnableWindow( pli->hwndVscroll, pli->sVscrollMax ? TRUE : FALSE );

        }
        break;

     case WM_PAINT:                    /* Draw the summary contents here  .   */
        fnDrawSummary( pli );
        break;

//   case WM_ACTIVATE:
//     {
//       if( SHORT1FROMMP(mp1)==TRUE && HWNDFROMMP(mp2)==hwndAll )
//       {
//         WinSetActiveWindow( HWND_DESKTOP, hwndDirlist );
//         WinSetActiveWindow( HWND_DESKTOP, hwndLegend  );
//         WinSetActiveWindow( HWND_DESKTOP, hwndFrame   );
//         WinSetActiveWindow( HWND_DESKTOP, hwndAll     );
//       }
//       mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
//     }
//     break;

#if 0
     case WM_BUTTON1DBLCLK:
     case WM_BUTTON2DBLCLK:
        {
          LONG x, y;
          CHAR szDebug[80];

          x = SHORT1FROMMP(mp1);
          y = SHORT2FROMMP(mp1);

          sprintf(szDebug, "x=%d, y=%d", x, y);
          WinMessageBox (HWND_DESKTOP, hwnd, szDebug, "Debug", 1,
                         MB_MOVEABLE | MB_OK);
        }
        break;
#endif

     case WM_HSCROLL:
        {
          LONG sHscrollInc;

          switch( SHORT2FROMMP( mp2 ))
          {
            case SB_LINELEFT:
              sHscrollInc = -pli->cxChar;
              break;

            case SB_LINERIGHT:
              sHscrollInc = pli->cxChar;
              break;

            case SB_PAGELEFT:
              sHscrollInc = -(8 * pli->cxChar);
              break;

            case SB_PAGERIGHT:
              sHscrollInc = 8 * pli->cxChar;
              break;

            case SB_SLIDERPOSITION:
              sHscrollInc = SHORT1FROMMP( mp2 ) - pli->sHscrollPos;
              break;

            default:
              sHscrollInc = 0;
              break;
          }

          sHscrollInc = max( -pli->sHscrollPos,
                        min( sHscrollInc, pli->sHscrollMax - pli->sHscrollPos ) );
          if( sHscrollInc != 0 )
          {
            pli->sHscrollPos += sHscrollInc;
            WinScrollWindow( hwnd, -sHscrollInc, 0,
                             (PRECTL)NULL, (PRECTL)NULL,
                             (HRGN)NULLHANDLE, (PRECTL)NULL,
                             SW_INVALIDATERGN );
            WinSendMsg( pli->hwndHscroll, SBM_SETPOS, MPFROMSHORT( pli->sHscrollPos ),
                        NULL );
          }
        }
        break;

     case WM_VSCROLL:
        {
          LONG sVscrollInc;

          switch( SHORT2FROMMP( mp2 ))
          {
            case SB_LINEUP:
              sVscrollInc = -1;
              break;

            case SB_LINEDOWN:
              sVscrollInc = 1;
              break;

            case SB_PAGEUP:
              sVscrollInc = min( -1, -pli->cyClient / pli->cyChar );
              break;

            case SB_PAGEDOWN:
              sVscrollInc = max( 1, pli->cyClient / pli->cyChar );
              break;

            case SB_SLIDERTRACK:
              sVscrollInc = SHORT1FROMMP( mp2 ) - pli->sVscrollPos;
              break;

            default:
              sVscrollInc = 0;
              break;
          }

          sVscrollInc = max( -pli->sVscrollPos, min( sVscrollInc, pli->sVscrollMax -
                        pli->sVscrollPos ) );
          if( sVscrollInc != 0 )
          {
            pli->sVscrollPos += sVscrollInc;
            WinScrollWindow( hwnd, 0, pli->cyChar * sVscrollInc,
                             (PRECTL)NULL, (PRECTL)NULL,
                             (HRGN)NULLHANDLE, (PRECTL)NULL,
                             SW_INVALIDATERGN );
            WinSendMsg( pli->hwndVscroll, SBM_SETPOS, MPFROMSHORT( pli->sVscrollPos ),
                        NULL );
            WinUpdateWindow( hwnd );
          }

        }
        break;

     case UM_CLOSE:
        free( pli );
        break;
                                       /* Close the window - actually just    */
     case WM_CLOSE:                    /* hide it and remove check mark from  */
        {                              /* pull-down choice                    */
          WinShowWindow( WinQueryWindow(hwnd,QW_PARENT), FALSE );
          showall = NO;
          WinSendDlgItemMsg (hwndFrame,
                             (ULONG) FID_MENU,
                             (ULONG) MM_SETITEMATTR,
                             MPFROM2SHORT(IDM_ALL,TRUE),
                             MPFROM2SHORT(MIA_CHECKED, showall ?
                                          MIA_CHECKED : 0));
        }                              /* end case (WM_CLOSE)                 */
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefWindowProc (hwnd, msg, mp1, mp2);
        break;

    }                                  /* end main switch statement           */

 return mr;

}

/******************************************************************************/
/*                                                                            */
/* fnDrawSummary: Draw All summary text.                                      */
/*                                                                            */
/******************************************************************************/

VOID fnDrawSummary( PLEGENDINFO pli )

{
  HPS         hps;
  RECTL       rect;
  POINTL      point;
  PDRIVES     item;
  ULONG       i;
  CHAR        szAll[80];
  CHAR        szDrive[10], szVolume[12], szTotal[10], szFree[10];
  APIRET      rc;
  float       sweepangle;

  hps = WinBeginPaint (pli->hwnd, 0L, 0L);
  WinQueryWindowRect (pli->hwnd, &rect);
  WinFillRect( hps, &rect, SYSCLR_WINDOW );

  /* Wait for semaphore to be posted to ensure data (LL) is available */

  rc = DosWaitEventSem( hev, 5L );
  if( rc == ERROR_TIMEOUT )
    return;

  /* First, write the heading in the first line of the window, get strings */
  /* from resource file                                                    */

  WinLoadString(pli->hab, 0L, IDS_DRIVE,  sizeof(szDrive), szDrive);
  WinLoadString(pli->hab, 0L, IDS_VOLUME, sizeof(szVolume), szVolume);
  WinLoadString(pli->hab, 0L, IDS_FREE,   sizeof(szFree), szFree);
  WinLoadString(pli->hab, 0L, IDS_TOTAL,  sizeof(szTotal), szTotal);
  sprintf(szAll, "%-5s %-12s %10s %10s\0", szDrive, szVolume, szFree, szTotal);
  point.x = 6*pli->cxChar - pli->sHscrollPos;
  point.y = pli->cyClient - pli->cyChar * (1 - pli->sVscrollPos)
            + pli->cyDesc;
  GpiCharStringAt( hps, &point, strlen(szAll), szAll );

  /* Loop thru all valid drives, those whose item->ignore indicator is YES */
  /* and write the drive information (free and total size) in the window.  */
  /* Also, draw a half circle (fuel gauge) showing how full the drive is.  */

  i = 1;
  item = drives;
  while(item)
  {
    point.y = pli->cyClient - pli->cyChar * (i + 1 - pli->sVscrollPos)
              + pli->cyDesc;
    if(item->ignore == NO)
    {
      point.x = 3*pli->cxChar - pli->sHscrollPos;

      GpiSetColor( hps, CLR_DARKGRAY);
      GpiBeginArea( hps, BA_BOUNDARY | BA_WINDING );
      GpiMove( hps, &point );
      sweepangle = 1.8 * 65536 * (float)(100*(float)item->free/(float)item->totalsize);
      GpiPartialArc( hps, &point, MAKEFIXED(pli->cyLowHeight,0),
                     sweepangle, MAKEFIXED(180,0)-sweepangle);
      GpiLine( hps, &point );
      GpiEndArea( hps );

      GpiSetColor ( hps, CLR_BLACK );
      GpiMove( hps, &point );
      GpiPartialArc( hps, &point, MAKEFIXED(pli->cyLowHeight,0),
                     MAKEFIXED(0,0), MAKEFIXED(180,0));
      GpiLine( hps, &point );

      point.x = 6*pli->cxChar - pli->sHscrollPos;
      switch( units )
      {
        case UNITS_BYTES:
          sprintf(szAll, "%-5s %-12s %10d %10d\0", item->driveletter,
                  item->volume, item->free, item->totalsize);
          break;
        case UNITS_KBYTES:
          sprintf(szAll, "%-5s %-12s %10.1f %10.1f\0", item->driveletter,
                  item->volume,
                  (float)(item->free/1024),
                  (float)(item->totalsize/1024));
          break;
        case UNITS_MEGS:
          sprintf(szAll, "%-5s %-12s %10.1f %10.1f\0", item->driveletter,
                  item->volume,
                  ((float)item->free)/(1024*1024),
                  ((float)item->totalsize)/(1024*1024));
          break;
        default:
          break;
      }
    }
    else
    {
      point.x = 6*pli->cxChar - pli->sHscrollPos;
      sprintf(szAll, "%-5s\0", item->driveletter);
    }
    GpiCharStringAt( hps, &point, strlen(szAll), szAll );
    i++;
    item = (PDRIVES)item->next;
  }

  WinEndPaint (hps);

}

/******************************************************************************/
/*                                                                            */
/*                     G E N E R A L    F U N C T I O N S                     */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* fnReadDrive: Determine all valid drives. Read the directory information    */
/*              for selected drive (variable is (CHAR)Drive).                 */
/*                                                                            */
/* Calls: fnGetDriveInfo                                                      */
/*        fnGetValidDrives                                                    */
/*        fnGetDirectoryInfo                                                  */
/*        fnPutLinkList1stSorted                                              */
/*                                                                            */
/******************************************************************************/

VOID fnReadDrive( VOID )
{

  HAB      hab;
  HMQ      hmq;
  ULONG    cumbytes = 0;               /* cumulative bytes, initialize to 0   */
  USHORT   level    = 0;               /* directory level, 0 for root         */
  PDIRINFO item;                       /* work variable used                  */
  PDRIVES  driveitem;
  ULONG    ulPostCount;
  ULONG    totsize, totfree, ulx;
  APIRET   rc;
  CHAR     volume[VOLUME_LENGTH+1];

  hab = WinInitialize (0);             /* initialization procedures, PM and   */
  hmq = WinCreateMsgQueue (hab, 0);    /* application message queue           */

  /* First, set an event semaphore while disk information is being read. This */
  /* will stop other threads (fnListBoxEntry) from accessing the directory    */
  /* linked list before we finish creating it                                 */

  DosResetEventSem( hev, &ulPostCount );

  WinEnableMenuItem( WinWindowFromID(hwndFrame, FID_MENU), IDM_REFRESH, FALSE );
  WinEnableMenuItem( WinWindowFromID(hwndFrame, FID_MENU), IDM_DRIVE, FALSE );

  /* Get a list of logical drives. Loop thru this list and get the device     */
  /* information for all valid drives to place in the All Drive Information   */
  /* window. Disable OS/2 error handling first, and then enable again.        */

  drives = fnGetValidDrives( );        /* determine all valid drives and get  */

  DosError(DISABLE_ERRORPOPUPS);       /* disable OS/2 error handling so that */
                                       /* drives not ready return with error  */
  totaldrives = 0;                     /* total number of logical drives      */
  driveitem = drives;
  while(driveitem)                     /* loop thru all logical drives        */
  {                                    /* check if drive is current drive     */
    if(driveitem->drivenumber == DriveNumber)
    {
      fnGetDriveInfo( DriveNumber, &totalsize, &totalfree, &totalused,
                      &allocunits, &swapsize, volume );
      driveitem->totalsize = totalsize;
      driveitem->free      = totalfree;
      driveitem->ignore    = NO;
      strcpy(driveitem->volume, volume);
    }
    else
    {
      if(driveitem->drivenumber > 2)   /* don't check floppy drives 1 or 2    */
      {
        rc = fnGetDriveInfo( driveitem->drivenumber, &totsize, &totfree,
                             &ulx, &ulx, &ulx, volume );
        if(rc == 0)                    /* valid drive, save total size and    */
        {                              /* space information                   */
          driveitem->totalsize = totsize;
          driveitem->free      = totfree;
          driveitem->ignore    = NO;   /* put in All Drives Information       */
          strcpy(driveitem->volume, volume);
        }
        else
        {
          driveitem->totalsize = 0;    /* invalid drive, set data to 0        */
          driveitem->free      = 0;
          driveitem->ignore    = YES;  /* set flag to ignore this drive       */
          driveitem->volume[0] = '\0';
        }
      }
      else
      {
        driveitem->totalsize = 0;      /* floppy drive (1 and 2), ignore it   */
        driveitem->free      = 0;
        driveitem->ignore    = YES;
        driveitem->volume[0] = '\0';
      }
    }
    totaldrives++;
    driveitem = (PDRIVES)driveitem->next;  /* get next drive in linked list   */
  }                                    /* end do while(driveitem)             */

  DosError(ENABLE_ERRORPOPUPS);

  /* Now, read the disk directory information. This is a recursive routine    */
  /* and builds a double linked list containing all subdirectories in drive   */

  if( rootdrive == YES )
  {
    strcpy( globStrings.searchdir, Drive );
    strcpy( globStrings.directory, Drive );
    strcat( globStrings.directory, "(ROOT)" );
  }

  fnGetDirectoryInfo( globStrings.searchdir, globStrings.directory, "",
                      totalsize, totalused,
                      allocunits, level, &cumbytes, 0,
                      graphswapsize, &swapsize );

  if( rootdrive == NO )                /* if a virtual root directory selected*/
  {                                    /* when use the cum size of that root  */
    totalsize = start->cumbytes + swapsize;
    totalused = start->cumbytes + swapsize;
    totalfree = 0;
    item = start;
    while( item )
    {
      item->percent1    = (float)item->bytes * 100 / totalsize;
      item->percent2    = item->percent1;
      item->cumpercent1 = (float)item->cumbytes * 100 / totalsize;
      item->cumpercent2 = item->cumpercent1;
      item = item->next;
    }
  }

  item = start;                        /* Build linked list for 1st level     */
  while( item )                        /* directories only                    */
  {
    startsort1st = fnPutLinkList1stSorted( item, startsort1st );
    item = item->next;
  }

  DosPostEventSem( hev );              /* Let other thread at the data now    */

  fnMenuBarUpdate( hwndClient );       /* Add drives to Drive menu bar        */

  WinPostMsg( hwndClient, UM_PAINT, 0L, 0L );
  WinPostMsg( hwndLegend, UM_LEGEND, 0L, 0L );
  WinPostMsg( hwndAll, UM_ALL, 0L, 0L );
  WinPostMsg( hwndClient, UM_WARNING, 0L, 0L );
  WinEnableMenuItem( WinWindowFromID(hwndFrame, FID_MENU), IDM_REFRESH, TRUE );
  WinEnableMenuItem( WinWindowFromID(hwndFrame, FID_MENU), IDM_DRIVE, TRUE );

  WinDestroyMsgQueue (hmq);
  WinTerminate (hab);

  DosExit( EXIT_THREAD, 0L );          /* Out of this thread, free it         */
}

/******************************************************************************/
/*                                                                            */
/* fnRefreshDirectory: Refresh the directory or read new disk.                */
/*                                                                            */
/* Calls: fnFreeList                                                          */
/*        fnInitialize                                                        */
/*        fnReadDrive (threaded)                                              */
/*        fnListBoxEntry (threaded)                                           */
/*                                                                            */
/******************************************************************************/

BOOL fnRefreshDirectory( HWND hwnd )
{

  HAB  hab;
  BOOL refresh;
  CHAR szTitle[50], szText[80];

  /* First, display a message box to let the user know that this may take a   */
  /* while so give them a chance to change their mind (return FALSE then)     */

  refresh = NO;
  if( showrefresh == YES )
  {
    hab = WinQueryAnchorBlock (hwnd);
    WinLoadString (hab, 0L, IDS_REFRESH, 80, szText);
    WinLoadString (hab, 0L, IDS_TITLEREF, 50, szTitle);
    if( WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle, IDD_REFRESH,
                       MB_MOVEABLE | MB_OKCANCEL | MB_HELP | MB_INFORMATION |
                       MB_DEFBUTTON1 ) == MBID_OK )
      refresh = YES;
  }
  else
    refresh = YES;

  if( refresh == YES )
  {
    HAB   hab;
    RECTL rectl;
    TID   tidDrive;                    /* thread identifiers                  */
    TID   tidList;

    /* Set the event semaphore so no other thread use the data since we are   */
    /* are going to refresh it with new information                           */

    DosResetEventSem( hev, &hevcount );

    fnFreeList( );                     /* free the memory for the linked list */

    fnInitialize( );                   /* initialize pointers to NULL         */
                                       /* re-read the disk information        */
                                       /* use a thread, it will post the sem  */
    DosCreateThread( &tidDrive, (PFNTHREAD)fnReadDrive, 0L, 0L, STACKSIZE );

    /* Delete all items in directory listbox and insert items again.          */

    WinSendMsg( hwndList, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE));
    WinSendMsg( hwndList, CM_REMOVEDETAILFIELDINFO, NULL, MPFROM2SHORT(0, CMA_FREE));
//  WinSendMsg( hwndList, LM_DELETEALL, MPFROMSHORT(0), (MPARAM) NULL );

    DosCreateThread( &tidList, (PFNTHREAD)fnListBoxEntry, 0L, 0L, STACKSIZE );

    return( 1 );
  }
  else
    return( 0 );                       /* return FALSE if user cancelled the  */
}                                      /* refresh                             */


/*****************************************************************************/
/*                                                                           */
/* fnListBoxEntry: Insert directory information into listbox.                */
/*                                                                           */
/*****************************************************************************/

VOID fnListBoxEntry( VOID )
{

  HAB      hab;
  HMQ      hmq;
  PDIRINFO item;
  APIRET   rc;

  PCUSTOBJ        pCustObj;
  PCUSTOBJ        pCustObjHead;
  PMINIRECORDCORE pRecord;
  RECORDINSERT    ri;

  hab = WinInitialize (0);             /* initialization procedures, PM and   */
  hmq = WinCreateMsgQueue (hab, 0);    /* application message queue           */

  /* Wait for semaphore to be posted to ensure data (linked list) is avail.   */

  rc = DosWaitEventSem( hev, SEM_INDEFINITE_WAIT );

  item = start;
  while( item )
  {

    ri.cb                = sizeof(RECORDINSERT);
    ri.pRecordOrder      = (PRECORDCORE)CMA_END;
    ri.pRecordParent     = NULL;
    ri.zOrder            = CMA_TOP;
    ri.cRecordsInsert    = 1;
    ri.fInvalidateRecord = FALSE;

    pCustObj     = (PCUSTOBJ)PVOIDFROMMR(WinSendMsg(hwndList, CM_ALLOCRECORD,
                         MPFROMLONG(sizeof(CUSTOBJ) - sizeof(MINIRECORDCORE)),
                         MPFROMSHORT(1)));
    if(!pCustObj)
    {
      /* error */
    }

    switch( units )
    {
      case UNITS_BYTES:
        {
          if( graphtype == GRAPH_TYPE_TOTAL )
          {
            if( directorytree == RIGHT )
              sprintf( item->listtext, "%10d %5.1f%% %9d %5.1f%% %5d %-20s",
                       item->bytes, item->percent1, item->cumbytes,
                       item->cumpercent1, item->files, item->dirname );
            else
              sprintf( item->listtext, "%-20s %10d %5.1f%% %9d %5.1f%% %5d",
                       item->dirname, item->bytes, item->percent1,
                       item->cumbytes, item->cumpercent1, item->files );
            sprintf( item->szPercent, "%5.1f", item->percent1 );
            sprintf( item->szCumPercent, "%5.1f", item->cumpercent1 );
          }
          else
          {
            if( directorytree == RIGHT )
              sprintf( item->listtext, "%10d %5.1f%% %9d %5.1f%% %5d %-20s",
                       item->bytes, item->percent2, item->cumbytes,
                       item->cumpercent2, item->files, item->dirname );
            else
              sprintf( item->listtext, "%-20s %10d %5.1f%% %9d %5.1f%% %5d",
                       item->dirname, item->bytes, item->percent2,
                       item->cumbytes, item->cumpercent2, item->files );
            sprintf( item->szPercent, "%5.1f", item->percent2 );
            sprintf( item->szCumPercent, "%5.1f", item->cumpercent2 );
          }
          sprintf( item->szSize,   "%10d",  item->bytes );
          sprintf( item->szCumSize,"%9d",   item->cumbytes );
        }
        break;
      case UNITS_KBYTES:
        {
          if( graphtype == GRAPH_TYPE_TOTAL )
          {
            if( directorytree == RIGHT )
              sprintf( item->listtext, "%10.0f %5.1f%% %9.0f %5.1f%% %5d %-20s",
                       (float)(item->bytes/1024), item->percent1,
                       (float)(item->cumbytes/1024), item->cumpercent1,
                       item->files, item->dirname );
            else
              sprintf( item->listtext, "%-20s %10.0f %5.1f%% %9.0f %5.1f%% %5d",
                       item->dirname, (float)(item->bytes/1024), item->percent1,
                       (float)(item->cumbytes/1024), item->cumpercent1,
                       item->files );
            sprintf( item->szPercent,    "%5.1f", item->percent1 );
            sprintf( item->szCumPercent, "%5.1f", item->cumpercent1 );
          }
          else
          {
            if( directorytree == RIGHT )
              sprintf( item->listtext, "%10.0f %5.1f%% %9.0f %5.1f%% %5d %-20s",
                       (float)(item->bytes/1024), item->percent2,
                       (float)(item->cumbytes/1024), item->cumpercent2,
                       item->files, item->dirname );
            else
              sprintf( item->listtext, "%-20s %10.0f %5.1f%% %9.0f %5.1f%% %5d",
                       item->dirname, (float)(item->bytes/1024), item->percent2,
                       (float)(item->cumbytes/1024), item->cumpercent2,
                       item->files );
            sprintf( item->szPercent,    "%5.1f", item->percent2 );
            sprintf( item->szCumPercent, "%5.1f", item->cumpercent2 );
          }
          sprintf( item->szSize,    "%10.0f", (float)(item->bytes/1024) );
          sprintf( item->szCumSize, "%9.0f",  (float)(item->cumbytes/1024) );
        }
        break;
      case UNITS_MEGS:
        {
          if( graphtype == GRAPH_TYPE_TOTAL )
          {
            if( directorytree == RIGHT )
              sprintf( item->listtext, "%10.1f %5.1f%% %9.1f %5.1f%% %5d %-20s",
                       ((float)item->bytes)/(1024*1024),
                       item->percent1, ((float)item->cumbytes)/(1024*1024),
                       item->cumpercent1, item->files, item->dirname );
            else
              sprintf( item->listtext, "%-20s %10.1f %5.1f%% %9.1f %5.1f%% %5d",
                       item->dirname, ((float)item->bytes)/(1024*1024),
                       item->percent1, ((float)item->cumbytes)/(1024*1024),
                       item->cumpercent1, item->files );
            sprintf( item->szPercent,    "%5.1f", item->percent1 );
            sprintf( item->szCumPercent, "%5.1f", item->cumpercent1 );
          }
          else
          {
            if( directorytree == RIGHT )
              sprintf( item->listtext, "%10.1f %4.1f%% %9.1f %4.1f%% %5d %-20s",
                       ((float)item->bytes)/(1024*1024),
                       item->percent2, ((float)item->cumbytes)/(1024*1024),
                       item->cumpercent2, item->files, item->dirname );
            else
              sprintf( item->listtext, "%-20s %10.1f %4.1f%% %9.1f %4.1f%% %5d",
                       item->dirname, ((float)item->bytes)/(1024*1024),
                       item->percent2, ((float)item->cumbytes)/(1024*1024),
                       item->cumpercent2, item->files );
            sprintf( item->szPercent,    "%5.1f", item->percent2 );
            sprintf( item->szCumPercent, "%5.1f", item->cumpercent2 );
          }
          sprintf( item->szSize,    "%10.1f", ((float)item->bytes)/(1024*1024) );
          sprintf( item->szCumSize, "%9.1f",  ((float)item->cumbytes)/(1024*1024) );
        }
        break;
      default:
        break;
    }

    /* We must post the message since this is in a separate thread from the   */
    /* window procedure. Insert items at the end using LIT_END.               */

//  WinSendMsg (hwndList, LM_INSERTITEM, MPFROMSHORT(LIT_END),
//              MPFROMP (item->listtext));

    pCustObj->pszDirectory  = item->dirname;
    pCustObj->pszSize       = item->szSize;
    pCustObj->pszPercent    = item->szPercent;
    pCustObj->pszCumSize    = item->szCumSize;
    pCustObj->pszCumPercent = item->szCumPercent;
    pCustObj->ulFiles       = item->files;
    pCustObj->pDirInfo      = item;
    pRecord = &pCustObj->miniRecordCore;
    pRecord->cb = sizeof(MINIRECORDCORE);
    WinSendMsg(hwndList, CM_INSERTRECORD, MPFROMP(pCustObj), MPFROMP(&ri));

    item = (PDIRINFO)item->next;       /* get next item in the linked list    */
  }

  fnCreateDetailsCols( hab );

  WinDestroyMsgQueue (hmq);
  WinTerminate (hab);

  DosExit( EXIT_THREAD, 0L );

}

/******************************************************************************/
/*                                                                            */
/* fnCreateDetailsCols:                                                       */
/*                                                                            */
/******************************************************************************/

BOOL fnCreateDetailsCols( HAB hab )
{

  FIELDINFOINSERT fieldInsert;
  CNRINFO         cnrInfo;
  PFIELDINFO      pFieldInfoHead;
  PFIELDINFO      pFieldInfo;
  RECTL           rectl;

  WinQueryWindowRect (hwndList, &rectl);

  fieldInsert.cb                   = sizeof(FIELDINFOINSERT);
  fieldInsert.pFieldInfoOrder      = (PFIELDINFO)CMA_END;
  fieldInsert.cFieldInfoInsert     = 6;
  fieldInsert.fInvalidateFieldInfo = TRUE;

  pFieldInfoHead = (PFIELDINFO)PVOIDFROMMR(WinSendMsg(hwndList,
                      CM_ALLOCDETAILFIELDINFO, MPFROMSHORT(6), NULL));
  pFieldInfo = pFieldInfoHead;

  if(directorytree == LEFT)
    pFieldInfo = pFieldInfo->pNextFieldInfo;

  /* Size */
  pFieldInfo->cb = sizeof(FIELDINFO);
  pFieldInfo->pTitleData = globStrings.szDirSize;
  pFieldInfo->flTitle    = CFA_FITITLEREADONLY | CFA_BOTTOM | CFA_RIGHT;
  pFieldInfo->flData     = CFA_STRING    | CFA_FIREADONLY | CFA_BOTTOM |
                           CFA_SEPARATOR | CFA_RIGHT      | CFA_HORZSEPARATOR;
  pFieldInfo->offStruct  = (ULONG)FIELDOFFSET(CUSTOBJ, pszSize);
  pFieldInfo = pFieldInfo->pNextFieldInfo;
  /* Percent */
  pFieldInfo->cb = sizeof(FIELDINFO);
  pFieldInfo->pTitleData = globStrings.szPercent;
  pFieldInfo->flTitle    = CFA_FITITLEREADONLY | CFA_BOTTOM | CFA_RIGHT;
  pFieldInfo->flData     = CFA_STRING    | CFA_FIREADONLY | CFA_BOTTOM  |
                           CFA_SEPARATOR | CFA_RIGHT      | CFA_HORZSEPARATOR;
  pFieldInfo->offStruct  = (ULONG)FIELDOFFSET(CUSTOBJ, pszPercent);
  pFieldInfo = pFieldInfo->pNextFieldInfo;
  /* Cumulative Size */
  pFieldInfo->cb = sizeof(FIELDINFO);
  pFieldInfo->pTitleData = globStrings.szCumSize;
  pFieldInfo->flTitle    = CFA_FITITLEREADONLY | CFA_BOTTOM  | CFA_RIGHT;
  pFieldInfo->flData     = CFA_STRING    | CFA_FIREADONLY | CFA_BOTTOM  |
                           CFA_SEPARATOR | CFA_RIGHT      | CFA_HORZSEPARATOR;
  pFieldInfo->offStruct  = (ULONG)FIELDOFFSET(CUSTOBJ, pszCumSize);
  pFieldInfo = pFieldInfo->pNextFieldInfo;
  /* Cumulative Percent */
  pFieldInfo->cb = sizeof(FIELDINFO);
  pFieldInfo->pTitleData = globStrings.szPercent;
  pFieldInfo->flTitle    = CFA_FITITLEREADONLY | CFA_BOTTOM  | CFA_RIGHT;
  pFieldInfo->flData     = CFA_STRING    | CFA_FIREADONLY | CFA_BOTTOM  |
                           CFA_SEPARATOR | CFA_RIGHT      | CFA_HORZSEPARATOR;
  pFieldInfo->offStruct  = (ULONG)FIELDOFFSET(CUSTOBJ, pszCumPercent);
  pFieldInfo = pFieldInfo->pNextFieldInfo;
  /* Files */
  pFieldInfo->cb = sizeof(FIELDINFO);
  pFieldInfo->pTitleData = globStrings.szFiles;
  pFieldInfo->flTitle    = CFA_FITITLEREADONLY | CFA_BOTTOM  | CFA_RIGHT;
  pFieldInfo->flData     = CFA_ULONG     | CFA_FIREADONLY | CFA_BOTTOM  |
                           CFA_SEPARATOR | CFA_RIGHT      | CFA_HORZSEPARATOR;
  pFieldInfo->offStruct  = (ULONG)FIELDOFFSET(CUSTOBJ, ulFiles);

  if(directorytree == LEFT)
    pFieldInfo = pFieldInfoHead;
  else
    pFieldInfo = pFieldInfo->pNextFieldInfo;

  /* Directory */
  pFieldInfo->cb = sizeof(FIELDINFO);
  pFieldInfo->pTitleData = globStrings.szDirectory;
  pFieldInfo->flTitle    = CFA_FITITLEREADONLY | CFA_BOTTOM | CFA_LEFT;
  pFieldInfo->flData     = CFA_STRING    | CFA_FIREADONLY | CFA_BOTTOM |
                           CFA_LEFT       | CFA_HORZSEPARATOR;
  pFieldInfo->offStruct  = (ULONG)FIELDOFFSET(CUSTOBJ, pszDirectory);
  pFieldInfo->cxWidth    = rectl.xRight / 3;
  pFieldInfo = pFieldInfo->pNextFieldInfo;

  WinSendMsg(hwndList, CM_INSERTDETAILFIELDINFO, MPFROMP(pFieldInfoHead),
                       MPFROMP(&fieldInsert));

  cnrInfo.cb             = sizeof(CNRINFO);
  cnrInfo.flWindowAttr   = CV_DETAIL | CA_DETAILSVIEWTITLES;
  if(directorytree == LEFT)
  {
    cnrInfo.pFieldInfoLast = pFieldInfoHead;
    cnrInfo.xVertSplitbar  = rectl.xRight / 3;
  }
  else
  {
    cnrInfo.pFieldInfoLast = NULL;
    cnrInfo.xVertSplitbar  = -1;
  }

  WinSendMsg(hwndList, CM_SETCNRINFO, MPFROMP(&cnrInfo),
                       MPFROMLONG(CMA_FLWINDOWATTR | CMA_PFIELDINFOLAST |
                                  CMA_XVERTSPLITBAR));

  return TRUE;
}

/******************************************************************************/
/*                                                                            */
/* fnStartCommand:                                                            */
/*                                                                            */
/******************************************************************************/

APIRET fnStartCommand( CHAR *szCommand, CHAR *szMsg )
{

  UCHAR     szObjBuf[100];
  UCHAR     szProgram[48];
  UCHAR     szParms[80];
  APIRET    rc;
  PID       pid;
  STARTDATA startData;
  ULONG     sid;
  ULONG     i, j;

  for( i=0; i<strlen(szCommand); i++ ) /* get the first work of the command   */
  {                                    /* string, this is the program name    */
    if( szCommand[i] == ' ' || szCommand[i] == '\0' )
      break;
    else
      szProgram[i] = szCommand[i];
  }
  szProgram[i] = '\0';

  for( j=i+1; j<strlen(szCommand); j++ )
    szParms[j-i-1] = szCommand[j];     /* the parameters to the program are   */
  szParms[j-i-1] = '\0';               /* the rest of the command string + the*/
  strcat( szParms, " " );              /* message displayed in the message box*/
  strcat( szParms, szMsg );

  startData.Length        = sizeof(STARTDATA);
  startData.Related       = SSF_RELATED_CHILD;
  startData.FgBg          = SSF_FGBG_FORE;
  startData.TraceOpt      = SSF_TRACEOPT_NONE;
  startData.PgmTitle      = NULL;
  startData.PgmName       = szProgram;
  startData.PgmInputs     = szParms;
  startData.TermQ         = 0;
  startData.Environment   = 0;
  startData.InheritOpt    = SSF_INHERTOPT_SHELL;
  startData.SessionType   = SSF_TYPE_DEFAULT;
  startData.IconFile      = 0;
  startData.PgmHandle     = NULLHANDLE;
  startData.PgmControl    = 0;
  startData.Reserved      = 0;
  startData.ObjectBuffer  = szObjBuf;
  startData.ObjectBuffLen = 100;
  rc = DosStartSession( &startData, &sid, &pid );

  return( rc );

}

/******************************************************************************/
/*                                                                            */
/* fnSelectItem: Determine which directory was selected in Directory List     */
/*               window.                                                      */
/*                                                                            */
/******************************************************************************/

PDIRINFO fnSelectItem( HAB hab, HWND hwnd )
{

   PDIRINFO listitem;
// SHORT    selitem;
   ULONG    i;
   CHAR     szTitle[50], szText[80];
   PMINIRECORDCORE pRecord;

// selitem = WinQueryLboxSelectedItem( hwndList );
// if( selitem < 0 )                   /* no directory selected in list box   */
   pRecord = (PMINIRECORDCORE)WinSendMsg(hwndList, CM_QUERYRECORDEMPHASIS,
               MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_SELECTED));

   if(pRecord == NULL)
   {                                   /* inform the user using a message box */
     WinLoadString (hab, 0L, IDS_DIRTITLE, 50, szTitle);
     WinLoadString (hab, 0L, IDS_SELECT,   80, szText);
     WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle,
                    0, MB_WARNING | MB_OK | MB_MOVEABLE);
     return (PDIRINFO)NULL;            /* return NULL to indicate error       */
   }
   else                                /* use the index returned to point the */
   {                                   /* return address to the directory     */
//   listitem = start;                 /* seleted                             */
//   for( i=0 ; i < selitem; i++ )
//     listitem = (PDIRINFO)listitem->next;
     listitem = ((PCUSTOBJ)(PVOID)pRecord)->pDirInfo;
     return listitem;
   }

}

/******************************************************************************/
/*                                                                            */
/* fnInitialize: Initialization routine.                                      */
/*                                                                            */
/******************************************************************************/

VOID fnInitialize( VOID )
{
   start        = NULL;                /* initialize starting and end linked  */
   last         = NULL;                /* list pointer to NULL                */
   startsort    = NULL;                /* sorted by size, all directories     */
   lastsort     = NULL;
   startsort1st = NULL;                /* sorted by size, 1st level direct.   */
   lastsort1st  = NULL;
   drives = NULL;                      /* linked list of valid drives         */

   totalfiles          = 0;            /* total files                         */
   totaldirectories    = 0;            /* total number of subdirectories      */
   total1stdirectories = 0;            /* total number of 1st level subdir.   */
   chardirectories     = 0;            /* max number of characters for subdir */
   char1stdirectories  = 0;            /* max number of char for 1st lev dir. */

}

/******************************************************************************/
/*                                                                            */
/* fnGraphDefaults: Default settings. Initialize variables.                   */
/*                                                                            */
/******************************************************************************/

VOID fnGraphDefaults( VOID )
{
   graphtype     = GRAPH_TYPE_TOTAL;   /* used only (1) or total space (0)    */
   graphlevel    = GRAPH_LEVEL_ALL;    /* 1st level (1) or all directories (0)*/
   graphmaxpies  = 20;                 /* maximum # of pie slices (1-20)      */
   graphpercent  = YES;                /* show %s (1) or don't show %s (0)    */
   graphsize     = NO;                 /* show (1) actual size instead of %s  */
   graphswapsize = NO;                 /* show (1) SWAPPER.DAT in pie chart   */
   graphswapstat = NO;                 /* show (1) SWAPPER.DAT in status area */
}

/******************************************************************************/
/*                                                                            */
/* fnOptionsDefaults: Default options. Initialize variables.                  */
/*                                                                            */
/******************************************************************************/

VOID fnOptionsDefaults( VOID )
{

   initlegend  = YES;                  /* toggle BOOL variable for legend     */
   initdirlist = YES;                  /* toggle BOOL variable for dir list   */
   initall     = YES;                  /* toggle BOOL variable for all summary*/
   showexit    = YES;                  /* toggle BOOL variable for exit box   */
   showrefresh = YES;                  /* toggle BOOL variable for refresh box*/
   directorytree = LEFT;               /* directory tree display (1=L, 2=R)   */
   units       = UNITS_KBYTES;         /* bytes (0), kbytes (1), MB (2)       */
}

/******************************************************************************/
/*                                                                            */
/* fnAutorefDefaults: Automated Refresh settings. Initialize variables.       */
/*                                                                            */
/******************************************************************************/

VOID fnAutorefDefaults( VOID )
{

   refreshmin  = 0;                    /* auto refresh interval (minutes)     */
   thresfree   = 0;                    /* threshold minimum free space (%)    */
   thresswap   = 0;                    /* threshold maximum SWAPPER.DAT (%)   */
   alarmfree   = NO;                   /* sound alarm when min free space     */
   messagefree = NO;                   /* show msg box when min free space    */
   alarmswap   = NO;                   /* sound alarm when max SWAPPER.DAT    */
   messageswap = NO;                   /* show msg box when max SWAPPER.DAT   */
   strcpy( commandfree, "" );          /* spawned command when min free space */
   strcpy( commandswap, "" );          /* spawned command when max SWAPPER.DAT*/
}

/******************************************************************************/
/*                                                                            */
/* fnToUpperString: Convert string to upper case.                             */
/*                                                                            */
/******************************************************************************/

VOID fnToUpperString( char *string )

{
  int i;

  for(i=0;i<strlen(string);i++)
  {
    if(string[i] > 96 && string[i] < 123 )
      string[i] = string[i] - 32;
  }
}

/******************************************************************************/
/*                                                                            */
/* fnToPolar: Convert graph coordinates to polar coordinates.                 */
/*                                                                            */
/******************************************************************************/
VOID fnToPolar( float x, float y, float *radius, float *angle )
{

/*        Y
             /
          | /
          |/ t
  -X -----+----- X
          |
          |
         -Y
*/

    *angle = atan2( y, x ) / M_PI * 180;
    if( *angle < 0 )
      *angle = 360 + *angle;
    *radius = sqrt( x * x + y * y );

}

/******************************************************************************/
/*                                                                            */
/* fnSubstr: Return a portion of a string.                                    */
/*                                                                            */
/******************************************************************************/

VOID fnSubstr( CHAR *source, CHAR *dest, ULONG base, ULONG ofs )
{

  char *sptr=source, *dptr=dest;   /* get a couple of pointer to data         */

  if ( base > 0 )                  /* Return null if offset is 0 or negative. */
    {
                                   /* Move source ptr to base.                */
    for ( ;(base > 1) && (*sptr != '\0'); base-- )
      sptr++;
                                   /* Copy source characters to dest.         */
    for ( ;(ofs > 0) && (*sptr != '\0'); ofs-- )
      *dptr++ = *sptr++;
                                   /* Now pad string out with blanks if       */
                                   /* offset is more than strlen              */
    for ( ;(ofs > 0); ofs-- )
      *dptr++ = ' ';
    }

  *dptr = '\0';                    /* End the dest with a null.               */

}
