/******************************************************************************/
/*                       (c) IBM Canada Ltd. 1993                             */
/*                                                                            */
/* Program:     DISKUSE - PM Graphical Disk Usage                             */
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

MRESULT EXPENTRY wpGraphDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpGraphDlgProc, system)
MRESULT EXPENTRY wpOptionsDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpOptionsDlgProc, system)
MRESULT EXPENTRY wpAutorefDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#pragma linkage (wpAutorefDlgProc, system)

VOID fnGraphDlgInit( HWND hwnd );
VOID fnOptionsDlgInit( HWND hwnd );
VOID fnAutorefDlgInit( HWND hwnd );

/******************************************************************************/
/*                                                                            */
/* wpDirectoryDlgProc: Window procedure for Directory Name dialog window.     */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpDirectoryDlgProc (HWND hwnd, ULONG msg, MPARAM mp1,
                                   MPARAM mp2)

{

 MRESULT mr = (MRESULT) FALSE;
 static BOOL  fcFldChg = FALSE;
 CHAR   szTitle[50], szText[80];

 switch (msg)
    {
     case WM_INITDLG:
       WinSetWindowText( WinWindowFromID( hwnd, IDE_DIRECTORY ),
                         globStrings.targetdir );
       break;

     case WM_CONTROL:
       switch( SHORT1FROMMP( mp1 ))
       {
         case IDE_DIRECTORY:
           if( (USHORT) SHORT2FROMMP(mp1) == EN_CHANGE )
             fcFldChg = TRUE;
           break;
         default:
           mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
           break;
       }
       break;

     case WM_COMMAND:
        switch (SHORT1FROMMP(mp1))
           {
            case DID_OK:               /* User clicked on OK push button      */
               {                       /* Process options settings and place  */
                                       /* into appropriate variables          */
                 /* import commandfree and commandswap from entry fields      */
                 if( fcFldChg == TRUE )
                   WinQueryWindowText( WinWindowFromID( hwnd, IDE_DIRECTORY ),
                                       sizeof( globStrings.targetdir ),
                                       globStrings.targetdir );

                 fnToUpperString( globStrings.targetdir );
                 if( globStrings.targetdir[1] != ':' )
                 {
                   HAB hab;
                   hab = WinQueryAnchorBlock( hwnd );
                   WinLoadString (hab, 0L, IDS_DIRTITLE, 50, szTitle);
                   WinLoadString (hab, 0L, IDS_DIRERROR, 80, szText);
                   WinMessageBox (HWND_DESKTOP, hwnd, szText, szTitle,
                                  0, MB_WARNING | MB_OK | MB_MOVEABLE);
                 }
                 else
                   WinDismissDlg (hwnd, TRUE);      /* remove the dialog box  */
               }                       /* end case (DID_OK)                   */
               break;

            case DID_CANCEL:           /* User clicked on Cancel push button  */
               WinDismissDlg (hwnd, FALSE);
               break;

            default:                   /* Let PM handle rest of messages      */
               mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
               break;
           }
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
        break;
    }

 return (mr);

}

/******************************************************************************/
/*                                                                            */
/* wpDetailsDlgProc: Window procedure for directory details dialog window.    */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpDetailsDlgProc (HWND hwnd, ULONG msg, MPARAM mp1,
                                   MPARAM mp2)

{

 MRESULT mr = (MRESULT) FALSE;
 CHAR    szTitle[50], szText[80];

 switch (msg)
    {
     case WM_INITDLG:
       {
         PDIRINFO pdir;

         pdir = PVOIDFROMMP (mp2);
         WinSetWindowPtr( hwnd, QWL_USER, pdir );

         WinSetWindowText( WinWindowFromID( hwnd, IDE_DETDIR ),
                           pdir->fulldirname );
         sprintf( szText, "%d", pdir->level );
         WinSetWindowText( WinWindowFromID( hwnd, IDE_DETLEVEL ), szText );

         switch( units )
         {
           case( UNITS_BYTES ):
             sprintf( szText, "%d", pdir->bytes );
             WinSetWindowText( WinWindowFromID( hwnd, IDE_DETSIZE ), szText );
             sprintf( szText, "%d", pdir->cumbytes );
             WinSetWindowText( WinWindowFromID( hwnd, IDE_DETCUMSIZE ), szText );
             break;
           case( UNITS_KBYTES ):
             sprintf( szText, "%-6.0f", (float)(pdir->bytes/1024) );
             WinSetWindowText( WinWindowFromID( hwnd, IDE_DETSIZE ), szText );
             sprintf( szText, "%-6.0f", (float)(pdir->cumbytes/1024) );
             WinSetWindowText( WinWindowFromID( hwnd, IDE_DETCUMSIZE ), szText );
             break;
           case( UNITS_MEGS ):
             sprintf( szText, "%-5.1f", ((float)pdir->bytes)/(1024*1024) );
             WinSetWindowText( WinWindowFromID( hwnd, IDE_DETSIZE ), szText );
             sprintf( szText, "%-5.1f", ((float)pdir->cumbytes)/(1024*1024) );
             WinSetWindowText( WinWindowFromID( hwnd, IDE_DETCUMSIZE ), szText );
             break;
           default:
             break;
         }

         if( graphtype == GRAPH_TYPE_TOTAL )
         {
           sprintf( szText, "%-5.1f", pdir->percent1 );
           WinSetWindowText( WinWindowFromID( hwnd, IDE_DETPERC ), szText );
           sprintf( szText, "%-5.1f", pdir->cumpercent1 );
           WinSetWindowText( WinWindowFromID( hwnd, IDE_DETCUMPERC ), szText );
         }
         else
         {
           sprintf( szText, "%-5.1f", pdir->percent2 );
           WinSetWindowText( WinWindowFromID( hwnd, IDE_DETPERC ), szText );
           sprintf( szText, "%-5.1f", pdir->cumpercent2 );
           WinSetWindowText( WinWindowFromID( hwnd, IDE_DETCUMPERC ), szText );
         }

         sprintf( szText, "%d", pdir->files );
         WinSetWindowText( WinWindowFromID( hwnd, IDE_DETFILES ), szText );
       }
       break;                          /* end case WM_INITDLG                 */

     case WM_COMMAND:
        switch (SHORT1FROMMP(mp1))
           {
            case DID_OK:               /* User clicked on OK push button      */
               WinDismissDlg (hwnd, TRUE);          /* remove the dialog box  */
               break;

            default:                   /* Let PM handle rest of messages      */
               mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
               break;
           }
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
        break;
    }

 return (mr);

}

/******************************************************************************/
/*                                                                            */
/* wpAboutDlgProc: Dialog window procedure for Product Information window.    */
/*                 Use PM timer to display for exactly as long as necessary.  */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpAboutDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{

 MRESULT mr = (MRESULT) FALSE;

 switch (msg)
    {
     case WM_CHAR:
       {
         if(CHARMSG(&msg)->fs & KC_KEYUP)
           return(MRESULT)FALSE;
         if(CHARMSG(&msg)->fs & KC_VIRTUALKEY)
         {
           switch( CHARMSG(&msg)->vkey )
           {
              case VK_ESC:
              case VK_NEWLINE:
              case VK_ENTER:
                WinDismissDlg( hwnd, TRUE );
                return(MRESULT)TRUE;
           }
         }
       }
       break;

     case WM_INITDLG:
        {
         LONG lLogoDisplay;
         HAB  hab;

         lLogoDisplay = * (PLONG) PVOIDFROMMP (mp2);
         hab = WinQueryAnchorBlock (hwnd);

         if (lLogoDisplay != -1)
            WinStartTimer (hab, hwnd, TID_LOGOTIMER, lLogoDisplay);
        }
        break;

     case WM_TIMER:
        {
         HAB hab;

         hab = WinQueryAnchorBlock (hwnd);
         WinStopTimer (hab, hwnd, TID_LOGOTIMER);
         WinDismissDlg (hwnd, TRUE);
        }
        break;

     case WM_COMMAND:
        switch (SHORT1FROMMP(mp1))
           {
            case DID_OK:
               WinDismissDlg (hwnd, TRUE);
            default:
               break;
           }
        break;

     default:
        mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
        break;
    }

 return (mr);

}

/******************************************************************************/
/*                                                                            */
/* fnCreateNotebook: Create Settings window (as a notebook).                  */
/*                                                                            */
/******************************************************************************/

BOOL fnCreateNotebook( HAB hab, HWND hwnd )
{

  HWND     hwndNBFrame;                /* Notebook window handle           */
  HWND     hwndNBClient;               /* Notebook client window           */
  ULONG    flFrameFlags;               /* Window creation flags            */
  CHAR     szTitle[50];

  WinLoadString (hab, 0L, IDS_SETTINGS, 50, szTitle);

  flFrameFlags = FCF_TITLEBAR   | FCF_SYSMENU | FCF_MINMAX |
                 FCF_SIZEBORDER | FCF_SHELLPOSITION;

  hwndNBFrame = WinCreateStdWindow(HWND_DESKTOP, WS_SAVEBITS,
                                   &flFrameFlags, NULL, szTitle,
                                   0L, 0L, ID_NBWINDOW, &hwndNBClient);

  if (hwndNBFrame)
    {
      RECTL rectlDesktop;              /* rectangle structure                 */
      SWP   swp;                       /* set window position structure       */
      ULONG ulSize;                    /* size of SWP structure               */
      LONG  x, y, cx, cy;              /* window position coordinates and size*/
      BOOL  windowSize;

      hwndNBClient = WinCreateWindow(hwndNBFrame, szNotebookClass, NULL, 0L,
                      0, 0, 0, 0, hwndNBFrame, HWND_TOP, FID_CLIENT,
                      NULL, NULL);
      if (hwndHelp)                    /* Associate help instance             */
        WinAssociateHelpInstance(hwndHelp, hwndNBFrame);

      windowSize = TRUE;               /* Position NB window and show it      */
      if( useProfile == YES )
      {
        ulSize = sizeof (SWP);
        if( PrfQueryProfileData( hini, szAppName, szSetWinPos, &swp, &ulSize ) )
        {
          x = swp.x;                   /* ok, got it, set coordinates and     */
          y = swp.y;                   /* window size                         */
          cx = swp.cx;
          cy = swp.cy;
          if( x >= 0 || y >= 0 )
            windowSize = FALSE;
        }
      }
      if( windowSize == TRUE )         /* no profile data, use system default */
      {                                /* window size and position            */
        WinQueryWindowRect (HWND_DESKTOP, &rectlDesktop);
        x = rectlDesktop.xRight / 5;
        y = rectlDesktop.yTop / 7;
        cx = rectlDesktop.xRight * 2 / 3;
        cy = rectlDesktop.yTop * 4 / 5;
      }

      WinSetWindowPos (hwndNBFrame, HWND_TOP, x, y, cx, cy,
                  SWP_SHOW | SWP_SIZE | SWP_MOVE | SWP_ACTIVATE);
      WinShowWindow(hwndNBFrame, TRUE);
      WinSetFocus(HWND_DESKTOP, hwndNBFrame);
    }

  return TRUE;
}                                         /* End of CreateNotebook function*/

/******************************************************************************/
/*                                                                            */
/* wpNBWndProc: Window procedure the Settings notebook window.                */
/*                                                                            */
/******************************************************************************/

MRESULT wpNBWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  MRESULT  mResult = FALSE;            /* Function result parameter        */
  HWND     hwndNb;                     /* Notebook control window handle   */
  ULONG    ulPageID;                   /* Notebook page ID number          */
  HWND     hwndPage;                   /* Notebook page window handle      */
  static   HWND  hwndNBPage[3];
  static   BOOL  oldgraphtype, oldgraphlevel, oldgraphpercent,
                 oldgraphsize, oldgraphswapsize, oldgraphswapstat;
  static   SHORT oldgraphmaxpies, oldunits, olddirectorytree;
  static   ULONG oldrefreshmin;

  switch (msg)
    {
      case WM_CREATE:                  /* Window initialisation*/
        {
          FONTMETRICS fm;              /* Font metric information          */
          HPS      hps;                /* Presentation space handle        */
          LONG     lCharHeight;        /* Character height                 */
          LONG     lCharWidth;         /* Character width                  */
          HAB      hab;
          CHAR     szTitle[50];
          USHORT   usAttr;             /* Notebook page attributes         */

          oldgraphtype     = graphtype;
          oldgraphlevel    = graphlevel;
          oldgraphpercent  = graphpercent;
          oldgraphsize     = graphsize;
          oldgraphswapsize = graphswapsize;
          oldgraphswapstat = graphswapstat;
          oldgraphmaxpies  = graphmaxpies;
          oldunits         = units;
          olddirectorytree = directorytree;
          oldrefreshmin    = refreshmin;

          WinEnableMenuItem( WinWindowFromID(WinQueryWindow(hwndClient, QW_PARENT),
                             FID_MENU), IDM_SETTINGS, FALSE );

          hab = WinQueryAnchorBlock( hwnd );

          hwndNb = WinCreateWindow(hwnd, WC_NOTEBOOK, "",
                                   WS_VISIBLE | BKS_MAJORTABRIGHT |
                                   BKS_BACKPAGESBR | BKS_SQUARETABS,
                                   0, 0, 0, 0, hwnd, HWND_TOP, ID_NBSETTINGS,
                                   0L, 0L);
          if (hwndHelp)                /* Associate help instance             */
            WinAssociateHelpInstance(hwndHelp, hwndNb);

          hps = (HPS)WinGetPS(hwndNb);    /* Get system font information      */
          GpiQueryFontMetrics(hps, (LONG)sizeof(FONTMETRICS), &fm);
          WinReleasePS(hps);
          lCharHeight = fm.lMaxBaselineExt + 7;
          lCharWidth  = (fm.lAveCharWidth + 4) * 10;

          WinSendMsg(hwndNb, BKM_SETDIMENSIONS,
                     MPFROM2SHORT(lCharWidth, lCharHeight),
                     MPFROMSHORT(BKA_MAJORTAB));
          WinSendMsg(hwndNb, BKM_SETDIMENSIONS,
                     MPFROM2SHORT(0, 0),
                     MPFROMSHORT(BKA_MINORTAB));
          WinSendMsg(hwndNb, BKM_SETNOTEBOOKCOLORS,
                     MPFROMLONG(SYSCLR_DIALOGBACKGROUND),
                     MPFROMLONG(BKA_BACKGROUNDPAGECOLORINDEX));
          WinSendMsg(hwndNb, BKM_SETNOTEBOOKCOLORS,
                     MPFROMLONG(SYSCLR_DIALOGBACKGROUND),
                     MPFROMLONG(BKA_BACKGROUNDMAJORCOLORINDEX));

          usAttr = BKA_MAJOR | BKA_STATUSTEXTON | BKA_AUTOPAGESIZE;

          WinLoadString (hab, 0L, IDS_OPTIONS, 50, szTitle);
          ulPageID = (ULONG)WinSendMsg(hwndNb, BKM_INSERTPAGE, NULL,
                            MPFROM2SHORT(usAttr, BKA_LAST));
          WinSendMsg(hwndNb, BKM_SETTABTEXT,
                     MPFROMLONG(ulPageID), MPFROMP(szTitle));
          hwndPage = WinLoadDlg(hwndNb, hwndNb, wpOptionsDlgProc,
                                0L, IDD_OPTIONS, NULL);
          WinSendMsg(hwndNb, BKM_SETPAGEWINDOWHWND, MPFROMLONG(ulPageID),
                     MPFROMHWND(hwndPage));
          hwndNBPage[0] = hwndPage;

          WinLoadString (hab, 0L, IDS_GRAPH, 50, szTitle);
          ulPageID = (ULONG)WinSendMsg(hwndNb, BKM_INSERTPAGE, NULL,
                            MPFROM2SHORT(usAttr, BKA_LAST));
          WinSendMsg(hwndNb, BKM_SETTABTEXT,
                     MPFROMLONG(ulPageID), MPFROMP(szTitle));
          hwndPage = WinLoadDlg(hwndNb, hwndNb, wpGraphDlgProc,
                                0L, IDD_GRAPH, NULL);
          WinSendMsg(hwndNb, BKM_SETPAGEWINDOWHWND, MPFROMLONG(ulPageID),
                     MPFROMHWND(hwndPage));
          hwndNBPage[1] = hwndPage;

          WinLoadString (hab, 0L, IDS_TITLEREF, 50, szTitle);
          ulPageID = (ULONG)WinSendMsg(hwndNb, BKM_INSERTPAGE, NULL,
                            MPFROM2SHORT(usAttr, BKA_LAST));
          WinSendMsg(hwndNb, BKM_SETTABTEXT,
                     MPFROMLONG(ulPageID), MPFROMP(szTitle));
          hwndPage = WinLoadDlg(hwndNb, hwndNb, wpAutorefDlgProc,
                                0L, IDD_AUTOREF, NULL);
          WinSendMsg(hwndNb, BKM_SETPAGEWINDOWHWND, MPFROMLONG(ulPageID),
                     MPFROMHWND(hwndPage));
          hwndNBPage[2] = hwndPage;

          WinShowWindow(hwndNb, TRUE);
          WinPostMsg(hwnd, UM_CREATENB, 0L, 0L);
        }
        break;

      case UM_CREATENB:
        WinSendMsg(hwndNBPage[0], UM_SELECT, 0L, 0L);
        break;

      case WM_CLOSE:                      /* Don't close the parent window!*/
        {
           ULONG i;
           for( i=0; i<3; i++)
             WinSendMsg(hwndNBPage[i], WM_COMMAND, MPFROMSHORT(DID_OK), 0L);

           if( oldgraphtype     != graphtype     ||
               oldgraphlevel    != graphlevel    ||
               oldgraphpercent  != graphpercent  ||
               oldgraphsize     != graphsize     ||
               oldgraphswapstat != graphswapstat ||
               oldgraphmaxpies  != graphmaxpies  ||
               olddirectorytree != directorytree ||
               oldunits != units )
           {
             WinSendMsg( hwndClient, UM_PAINT, 0L, 0L );
             if( oldgraphtype     != graphtype  ||
                 oldgraphlevel    != graphlevel ||
                 oldgraphmaxpies  != graphmaxpies )
               WinSendMsg( hwndLegend, UM_LEGEND, 0L, 0L );
             if( oldgraphtype     != graphtype     ||
                 olddirectorytree != directorytree ||
                 oldunits != units )
               WinSendMsg( hwndClient, UM_LISTBOX, 0L, 0L );
           }
           if( oldrefreshmin != refreshmin )
             WinSendMsg( hwndClient, UM_AUTOREF, 0L, 0L );
           if( oldgraphswapsize != graphswapsize )
             WinSendMsg( hwndClient, UM_REFDIR, 0L, 0L );

           WinEnableMenuItem( WinWindowFromID(WinQueryWindow(hwndClient, QW_PARENT),
                              FID_MENU), IDM_SETTINGS, TRUE );
           WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
        }
        break;

      case WM_SAVEAPPLICATION:                      /* Save window position*/
        {
          SWP swp;

          if( useProfile == YES )
          {
            WinQueryWindowPos (WinQueryWindow (hwnd, QW_PARENT), &swp);
            PrfWriteProfileData( hini, szAppName, szSetWinPos, &swp, sizeof (SWP));
          }
        }
        break;

      case WM_SIZE:                  /* Make sure notebook fills the window   */
        hwndNb = WinWindowFromID(hwnd, ID_NBSETTINGS);
        WinSetWindowPos(hwndNb, HWND_TOP, 0, 0,
                        SHORT1FROMMP(mp2), SHORT2FROMMP(mp2),
                        SWP_SIZE | SWP_MOVE);
        break;

      case WM_CONTROL:               /* Turn the notebook page here           */
        if ((SHORT1FROMMP(mp1) == ID_NBSETTINGS)
         && (SHORT2FROMMP(mp1) == BKN_PAGESELECTED))
          {
            PPAGESELECTNOTIFY pPsn;

            pPsn = (PPAGESELECTNOTIFY)mp2;
            hwndPage = (HWND)WinSendMsg(pPsn->hwndBook,
                                        BKM_QUERYPAGEWINDOWHWND,
                                        MPFROMLONG(pPsn->ulPageIdNew), 0L);
            WinSendMsg(hwndPage, UM_SELECT, 0L, 0L);
          };                                                      /* End if*/
        break;

      default:
        mResult = WinDefWindowProc(hwnd, msg, mp1, mp2);
        break;
    }                                                         /* End switch*/

  return mResult;
}                                            /* End of NotebookWnd function*/

/******************************************************************************/
/*                                                                            */
/* wpOptionsDlgProc:  Window procedure for Options dialog window.             */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpOptionsDlgProc (HWND hwnd, ULONG msg, MPARAM mp1,
                                   MPARAM mp2)

{

 MRESULT mr = (MRESULT) FALSE;
 static BOOL oldinitdirlist, oldinitlegend, oldshowexit, oldshowrefresh,
             oldinitall;
 static SHORT oldunits, olddirectorytree;

 switch (msg)
    {
     case WM_INITDLG:                  /* Dialog box just created, initialize */
        fnOptionsDlgInit( hwnd );
        oldinitdirlist   = initdirlist;
        oldinitlegend    = initlegend;
        oldinitall       = initall;
        oldshowexit      = showexit;
        oldshowrefresh   = showrefresh;
        oldunits         = units;
        olddirectorytree = directorytree;
        break;                         /* end case (WM_INITDLG)               */

     case UM_SELECT:
       WinSetFocus (HWND_DESKTOP, WinWindowFromID (hwnd, IDC_DIRLIST));
       break;

     case WM_COMMAND:
        switch (SHORT1FROMMP(mp1))
           {
            case DID_OK:               /* User clicked on OK push button      */
                                       /* Process options settings and place  */
               initdirlist = (BOOL) WinSendDlgItemMsg( hwnd, IDC_DIRLIST,
                             BM_QUERYCHECK, 0L, 0L );

               initlegend  = (BOOL) WinSendDlgItemMsg( hwnd, IDC_LEGEND,
                             BM_QUERYCHECK, 0L, 0L );

               initall     = (BOOL) WinSendDlgItemMsg( hwnd, IDC_ALL,
                             BM_QUERYCHECK, 0L, 0L );

               showexit    = (BOOL) WinSendDlgItemMsg( hwnd, IDC_EXIT,
                             BM_QUERYCHECK, 0L, 0L );

               showrefresh = (BOOL) WinSendDlgItemMsg( hwnd, IDC_REFRESH,
                             BM_QUERYCHECK, 0L, 0L );

               directorytree = (USHORT) WinSendDlgItemMsg( hwnd, IDB_RIGHT,
                               BM_QUERYCHECK, 0L, 0L );

               if( (BOOL) WinSendDlgItemMsg( hwnd, IDB_BYTES,
                          BM_QUERYCHECK, 0L, 0L ) == 1 )
                 units = UNITS_BYTES;
               else
               {
                 if( (BOOL) WinSendDlgItemMsg( hwnd, IDB_KBYTES,
                                       BM_QUERYCHECK, 0L, 0L ) == 1 )
                   units = UNITS_KBYTES;
                 else
                   units = UNITS_MEGS;
               }

               WinDismissDlg (hwnd, TRUE);          /* remove the dialog box  */
               break;

            case IDP_SETDEF:           /* User clicked on Default push button */
               fnOptionsDefaults( );   /* Re-initialize variables to system   */
               fnOptionsDlgInit( hwnd );
               break;                  /* end case (IDP_SETDEF)               */

            case IDP_SETHELP:
               break;

            case DID_CANCEL:           /* User clicked on Cancel push button  */
               initdirlist   = oldinitdirlist;
               initlegend    = oldinitlegend;
               initall       = oldinitall;
               showexit      = oldshowexit;
               showrefresh   = oldshowrefresh;
               units         = oldunits;
               directorytree = olddirectorytree;
               fnOptionsDlgInit( hwnd );
               break;

            default:                   /* Let PM handle rest of messages      */
               mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
               break;
           }
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
        break;
    }

 return (mr);

}

/******************************************************************************/
/*                                                                            */
/* fnOptionsDlgInit: Window procedure for Options dialog box.                 */
/*                                                                            */
/******************************************************************************/

VOID fnOptionsDlgInit( HWND hwnd )
                                       /* Dialog box just created. Initialize */
{                                      /* all controls in dialog box.         */

  if( initdirlist == YES )
    WinSendDlgItemMsg (hwnd, IDC_DIRLIST, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_DIRLIST, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);

  if( initlegend == YES )
    WinSendDlgItemMsg (hwnd, IDC_LEGEND, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_LEGEND, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);

  if( initall == YES )
    WinSendDlgItemMsg (hwnd, IDC_ALL, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_ALL, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);

  if( showexit == YES )
    WinSendDlgItemMsg (hwnd, IDC_EXIT, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_EXIT, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);

  if( showrefresh == YES )
    WinSendDlgItemMsg (hwnd, IDC_REFRESH, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_REFRESH, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);

  if( directorytree == LEFT )
    WinSendDlgItemMsg (hwnd, IDB_LEFT, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDB_RIGHT, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);

  switch( units )                      /* radio buttons for graph units       */
  {
    case UNITS_BYTES:
      WinSendDlgItemMsg (hwnd, IDB_BYTES, BM_SETCHECK,
                         MPFROMSHORT (TRUE), 0L);
      break;
    case UNITS_KBYTES:
      WinSendDlgItemMsg (hwnd, IDB_KBYTES, BM_SETCHECK,
                         MPFROMSHORT (TRUE), 0L);
      break;
    case UNITS_MEGS:
      WinSendDlgItemMsg (hwnd, IDB_MBYTES, BM_SETCHECK,
                         MPFROMSHORT (TRUE), 0L);
      break;
    default:
      break;
  }

}

/******************************************************************************/
/*                                                                            */
/* wpGraphDlgProc: Window procedure for Customize Graph dialog window.        */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpGraphDlgProc (HWND hwnd, ULONG msg, MPARAM mp1,
                                   MPARAM mp2)

{

 MRESULT mr = (MRESULT) FALSE;
 static BOOL  oldgraphtype;
 static BOOL  oldgraphlevel;
 static BOOL  oldgraphpercent;
 static SHORT oldgraphsize;
 static SHORT oldgraphmaxpies;
 static BOOL  oldgraphswapsize;
 static BOOL  oldgraphswapstat;

 switch (msg)
    {
     case WM_INITDLG:
        fnGraphDlgInit( hwnd );
        oldgraphtype     = graphtype;
        oldgraphlevel    = graphlevel;
        oldgraphpercent  = graphpercent;
        oldgraphsize     = graphsize;
        oldgraphmaxpies  = graphmaxpies;
        oldgraphswapsize = graphswapsize;
        oldgraphswapstat = graphswapstat;
        break;                         /* end case (WM_INITDLG)               */

     case UM_SELECT:
       switch( graphtype )
       {
         case( GRAPH_TYPE_TOTAL ):
           WinSetFocus (HWND_DESKTOP, WinWindowFromID (hwnd, IDB_TOTAL));
           break;
         case( GRAPH_TYPE_USED ):
           WinSetFocus (HWND_DESKTOP, WinWindowFromID (hwnd, IDB_USED));
           break;
         default:
           break;
       }
       break;

     case WM_COMMAND:
        switch (SHORT1FROMMP(mp1))
           {
            case DID_OK:               /* User clicked on OK push button      */
               {                       /* Process options settings and place  */
                                       /* into appropriate variables          */
                                       /* get graph type from radio button    */
                 CHAR szText[6];

                 graphtype  = (BOOL) WinSendDlgItemMsg( hwnd, IDB_USED,
                              BM_QUERYCHECK, 0L, 0L );
                                       /* get graph level from radio button   */
                 graphlevel = (BOOL) WinSendDlgItemMsg( hwnd, IDB_1ST,
                              BM_QUERYCHECK, 0L, 0L );
                                       /* get graph percent from radio button */
                 graphpercent = (BOOL) WinSendDlgItemMsg( hwnd, IDB_PERCENTON,
                                BM_QUERYCHECK, 0L, 0L );
                                       /* get graph size from radio button    */
                 graphsize = (BOOL) WinSendDlgItemMsg( hwnd, IDB_SIZEON,
                             BM_QUERYCHECK, 0L, 0L );
                                       /* get graph swap from check box       */
                 graphswapsize = (BOOL) WinSendDlgItemMsg( hwnd, IDC_SWAPPIE,
                               BM_QUERYCHECK, 0L, 0L );
                                       /* get graph swap from check box       */
                 graphswapstat = (BOOL) WinSendDlgItemMsg( hwnd, IDC_SWAPSTAT,
                               BM_QUERYCHECK, 0L, 0L );
                                    /* Get maximum # of pies from spin button */
                 WinSendDlgItemMsg( hwnd, IDC_MAXPIES, SPBM_QUERYVALUE,
                                    szText, MPFROM2SHORT( sizeof( szText ),0L ));
                 graphmaxpies = atoi( szText );
                 if( graphmaxpies < 1 )   /* must be between 0 and 20 slices  */
                   graphmaxpies = 1;
                 if( graphmaxpies > GRAPH_MAX_PIES )
                   graphmaxpies = GRAPH_MAX_PIES;

                 WinDismissDlg (hwnd, TRUE);        /* remove the dialog box  */
               }                       /* end case (DID_OK)                   */
               break;

            case IDP_OPTDEF:           /* User clicked on Default push button */
               {
                 fnGraphDefaults( );
                 fnGraphDlgInit( hwnd );
               }                       /* end case (IDP_OPTDEF)               */
               break;

            case IDP_OPTHELP:
               break;

            case DID_CANCEL:           /* User clicked on Cancel push button  */
               graphtype     = oldgraphtype;
               graphlevel    = oldgraphlevel;
               graphpercent  = oldgraphpercent;
               graphsize     = oldgraphsize;
               graphmaxpies  = oldgraphmaxpies;
               graphswapsize = oldgraphswapsize;
               graphswapstat = oldgraphswapstat;
               fnGraphDlgInit( hwnd );
               break;

            default:                   /* Let PM handle rest of messages      */
               mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
               break;
           }
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
        break;
    }

 return (mr);

}

/******************************************************************************/
/*                                                                            */
/* fnGraphDlgInit: Initialize Graph dialog box.                               */
/*                                                                            */
/******************************************************************************/

VOID fnGraphDlgInit( HWND hwnd )
{                                      /* Dialog box just created. Initialize */
                                       /* combo box, radio buttons and entry  */
                                       /* field                               */
                                       /* radio buttons for graph type        */
  if( graphtype == GRAPH_TYPE_USED )
    WinSendDlgItemMsg (hwnd, IDB_USED, BM_SETCHECK, MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDB_TOTAL, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
                                       /* radio buttons for graph level       */
  if( graphlevel == GRAPH_LEVEL_1ST )
    WinSendDlgItemMsg (hwnd, IDB_1ST, BM_SETCHECK, MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDB_ALLDIR, BM_SETCHECK, MPFROMSHORT (TRUE), 0L);
                                       /* radio buttons for graph percentages */

  if( graphpercent == YES )
    WinSendDlgItemMsg (hwnd, IDB_PERCENTON, BM_SETCHECK, MPFROMSHORT (TRUE),
                       0L);
  else
  {
    if( graphsize == YES )
      WinSendDlgItemMsg (hwnd, IDB_SIZEON, BM_SETCHECK, MPFROMSHORT (TRUE),
                         0L);
    else
      WinSendDlgItemMsg (hwnd, IDB_DISPLAYNONE, BM_SETCHECK, MPFROMSHORT (TRUE),
                         0L);
  }

  if( graphswapsize == YES )
    WinSendDlgItemMsg (hwnd, IDC_SWAPPIE, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_SWAPPIE, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);

  if( graphswapstat == YES )
    WinSendDlgItemMsg (hwnd, IDC_SWAPSTAT, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_SWAPSTAT, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);
                                       /* Maximum # of pies entry field       */
  WinSendDlgItemMsg( hwnd, IDC_MAXPIES, SPBM_SETLIMITS,
                     MPFROMSHORT(GRAPH_MAX_PIES), MPFROMSHORT(1) );
  WinSendDlgItemMsg( hwnd, IDC_MAXPIES, SPBM_SETCURRENTVALUE,
                     MPFROMSHORT(graphmaxpies), 0L );
  WinSendDlgItemMsg( hwnd, IDC_MAXPIES, SPBM_SETTEXTLIMIT,
                     MPFROMSHORT(2L), 0L );

}

/******************************************************************************/
/*                                                                            */
/* wpAutorefDlgProc: Window procedure for Automated Refresh dialog window.    */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpAutorefDlgProc (HWND hwnd, ULONG msg, MPARAM mp1,
                                   MPARAM mp2)

{

 MRESULT mr = (MRESULT) FALSE;
 static ULONG oldrefreshmin;
 static ULONG oldthresfree;
 static ULONG oldthresswap;
 static BOOL  oldalarmfree;
 static BOOL  oldmessagefree;
 static BOOL  oldalarmswap;
 static BOOL  oldmessageswap;
 static CHAR  oldcommandfree[40];
 static CHAR  oldcommandswap[40];
 static BOOL  fcfreeFldChg = FALSE;
 static BOOL  fcswapFldChg = FALSE;

 switch (msg)
    {
     case WM_INITDLG:
        fnAutorefDlgInit( hwnd );
        oldrefreshmin  = refreshmin;
        oldthresfree   = thresfree;
        oldthresswap   = thresswap;
        oldalarmfree   = alarmfree;
        oldmessagefree = messagefree;
        oldalarmswap   = alarmswap;
        oldmessageswap = messageswap;
        strcpy( oldcommandfree, commandfree );
        strcpy( oldcommandswap, commandswap );
        break;                         /* end case (WM_INITDLG)               */

     case UM_SELECT:
        WinSetFocus (HWND_DESKTOP, WinWindowFromID (hwnd, IDC_REFMINUTES));
        break;

     case WM_CONTROL:
       switch( SHORT1FROMMP( mp1 ))
       {
         case IDE_COMMANDFREE:
           if( (USHORT) SHORT2FROMMP(mp1) == EN_CHANGE )
             fcfreeFldChg = TRUE;
           break;
         case IDE_COMMANDSWAP:
           if( (USHORT) SHORT2FROMMP(mp1) == EN_CHANGE )
             fcswapFldChg = TRUE;
           break;
         default:
           mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
           break;
       }
       break;

     case WM_COMMAND:
        switch (SHORT1FROMMP(mp1))
           {
            case DID_OK:               /* User clicked on OK push button      */
               {                       /* Process options settings and place  */
                                       /* into appropriate variables          */
                 CHAR szText[6];

                 WinSendDlgItemMsg( hwnd, IDC_REFMINUTES, SPBM_QUERYVALUE,
                                    szText, MPFROM2SHORT( sizeof( szText ),0L ));
                 refreshmin = atoi( szText );
                 if( refreshmin < 0 )
                   refreshmin = 0;
                 if( refreshmin > 1440 )
                   refreshmin = 1440;

                 WinSendDlgItemMsg( hwnd, IDC_THRESFREE, SPBM_QUERYVALUE,
                                    szText, MPFROM2SHORT( sizeof( szText ),0L ));
                 thresfree = atoi( szText );
                 if( thresfree < 0 )
                   thresfree = 0;
                 if( thresfree > 99 )
                   thresfree = 99;
                 WinSendDlgItemMsg( hwnd, IDC_THRESSWAP, SPBM_QUERYVALUE,
                                    szText, MPFROM2SHORT( sizeof( szText ),0L ));
                 thresswap = atoi( szText );
                 if( thresswap < 0 )
                   thresswap = 0;
                 if( thresswap > 99 )
                   thresswap = 99;

                 alarmfree   = (BOOL) WinSendDlgItemMsg( hwnd, IDC_ALARMFREE,
                               BM_QUERYCHECK, 0L, 0L );
                 messagefree = (BOOL) WinSendDlgItemMsg( hwnd, IDC_MESSAGEFREE,
                               BM_QUERYCHECK, 0L, 0L );
                 alarmswap   = (BOOL) WinSendDlgItemMsg( hwnd, IDC_ALARMSWAP,
                               BM_QUERYCHECK, 0L, 0L );
                 messageswap = (BOOL) WinSendDlgItemMsg( hwnd, IDC_MESSAGESWAP,
                               BM_QUERYCHECK, 0L, 0L );

                 /* import commandfree and commandswap from entry fields      */
                 if( fcfreeFldChg == TRUE )
                   WinQueryWindowText( WinWindowFromID( hwnd, IDE_COMMANDFREE ),
                                       sizeof( commandfree ), commandfree );
                 if( fcswapFldChg == TRUE )
                   WinQueryWindowText( WinWindowFromID( hwnd, IDE_COMMANDSWAP ),
                                       sizeof( commandswap ), commandswap );

                 WinDismissDlg (hwnd, TRUE);        /* remove the dialog box  */
               }                       /* end case (DID_OK)                   */
               break;

            case IDP_NONE:             /* User clicked on Default push button */
               {
                 refreshmin  = 0;
                 WinSendDlgItemMsg( hwnd, IDC_REFMINUTES, SPBM_SETCURRENTVALUE,
                                    MPFROMSHORT(refreshmin), 0L );
               }                       /* end case (IDP_NONE)                 */
               break;

            case IDP_REFHELP:
               break;

            case DID_CANCEL:           /* User clicked on Cancel push button  */
               refreshmin  = oldrefreshmin;
               thresfree   = oldthresfree;
               thresswap   = oldthresswap;
               alarmfree   = oldalarmfree;
               messagefree = oldmessagefree;
               alarmswap   = oldalarmswap;
               messageswap = oldmessageswap;
               strcpy( commandfree, oldcommandfree );
               strcpy( commandswap, oldcommandswap );
               fnAutorefDlgInit( hwnd );
               break;

            default:                   /* Let PM handle rest of messages      */
               mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
               break;
           }
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefDlgProc (hwnd, msg, mp1, mp2);
        break;
    }

 return (mr);

}

/******************************************************************************/
/*                                                                            */
/* fnAutorefDlgInit: Initialize Auto Refresh dialog box.                      */
/*                                                                            */
/******************************************************************************/

VOID fnAutorefDlgInit( HWND hwnd )

{

                                       /* auto refresh interval - minutes     */
  WinSendDlgItemMsg( hwnd, IDC_REFMINUTES, SPBM_SETLIMITS,
                     MPFROMSHORT(1440), MPFROMSHORT(0) );
  WinSendDlgItemMsg( hwnd, IDC_REFMINUTES, SPBM_SETCURRENTVALUE,
                     MPFROMSHORT(refreshmin), 0L );
  WinSendDlgItemMsg( hwnd, IDC_REFMINUTES, SPBM_SETTEXTLIMIT,
                     MPFROMSHORT(4L), 0L );

                                       /* threshold - minimum free space (%)  */
  WinSendDlgItemMsg( hwnd, IDC_THRESFREE, SPBM_SETLIMITS,
                     MPFROMSHORT(99), MPFROMSHORT(0) );
  WinSendDlgItemMsg( hwnd, IDC_THRESFREE, SPBM_SETCURRENTVALUE,
                     MPFROMSHORT(thresfree), 0L );
  WinSendDlgItemMsg( hwnd, IDC_THRESFREE, SPBM_SETTEXTLIMIT,
                     MPFROMSHORT(2L), 0L );
                                       /* threshold - maximum SWAPPER.DAT (%) */
  WinSendDlgItemMsg( hwnd, IDC_THRESSWAP, SPBM_SETLIMITS,
                     MPFROMSHORT(99), MPFROMSHORT(0) );
  WinSendDlgItemMsg( hwnd, IDC_THRESSWAP, SPBM_SETCURRENTVALUE,
                     MPFROMSHORT(thresswap), 0L );
  WinSendDlgItemMsg( hwnd, IDC_THRESSWAP, SPBM_SETTEXTLIMIT,
                     MPFROMSHORT(2L), 0L );

                                       /* Warnings (check boxes)              */
                                       /* Minimum free space check            */
  if( alarmfree == YES )
    WinSendDlgItemMsg (hwnd, IDC_ALARMFREE, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_ALARMFREE, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);
  if( messagefree == YES )
    WinSendDlgItemMsg (hwnd, IDC_MESSAGEFREE, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_MESSAGEFREE, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);
                                       /* Maximum SWAPPER.DAT check           */
  if( alarmswap == YES )
    WinSendDlgItemMsg (hwnd, IDC_ALARMSWAP, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_ALARMSWAP, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);
  if( messageswap == YES )
    WinSendDlgItemMsg (hwnd, IDC_MESSAGESWAP, BM_SETCHECK,
                       MPFROMSHORT (TRUE), 0L);
  else
    WinSendDlgItemMsg (hwnd, IDC_MESSAGESWAP, BM_SETCHECK,
                       MPFROMSHORT (FALSE), 0L);

                                       /* Spawned command when min free space */
   WinSetWindowText( WinWindowFromID( hwnd, IDE_COMMANDFREE ), commandfree );
                                       /* Spawned command when max SWAPPER.DAT*/
   WinSetWindowText( WinWindowFromID( hwnd, IDE_COMMANDSWAP ), commandswap );
   WinEnableWindow( WinWindowFromID( hwnd, IDE_COMMANDFREE ), TRUE );
   WinEnableWindow( WinWindowFromID( hwnd, IDE_COMMANDSWAP ), TRUE );

}

