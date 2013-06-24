/******************************************************************************/
/*                       (c) IBM Corp. 1994                                   */
/*                                                                            */
/* Program: DISKUSE - Print Support                                           */
/*                                                                            */
/* Author : E. L. Zapanta                                                     */
/* Date   : 23Aug94                                                           */
/*                                                                            */
/******************************************************************************/

#define INCL_WIN
#define INCL_BASE
#define INCL_GPI
#define INCL_SPL
#define INCL_SPLDOSPRINT
#define INCL_DEV
#define INCL_DOS

#define PROGRESSBARLENGTH   30
#define DEVICENAME_LENGTH   32
#define DRIVERNAME_LENGTH   128
#define MAX_PRINT_FORMS     20

#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "diskproc.h"                  /* include file for disk procedures    */
#include "diskuse.h"                   /* main include file                   */
#include "diskdlg.h"                   /* include file for dialog boxes       */

VOID fnProcessPrint( HWND hwnd, LONG printgraph );
LONG fnPrint( LONG printgraph );
#pragma linkage (fnPrint, system)

HWND      hwndPrint;
PPRQINFO3 pPrqInfo;
PDRIVDATA pDriverData;
TID       threadID;


/******************************************************************************/
/*                                                                            */
/* wpPrintDlgProc: Window procedure for Print dialog window.                  */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpPrintDlgProc (HWND hwnd, ULONG msg, MPARAM mp1,
                                 MPARAM mp2)

{

 MRESULT mr = (MRESULT) FALSE;
 static  HAB hab;

 switch (msg)
    {
     case WM_INITDLG:                  /* Dialog box just created. Initialize */
        {                              /* list box, etc.                      */

          ULONG     cbReturned;
          ULONG     cbTotal;
          ULONG     cbNeeded;
          ULONG     cbBuf;
          PBYTE     pBuf;
          SPLERR    splErr;
          PPRQINFO3 prq;
          USHORT    i;

          hab = WinQueryAnchorBlock (hwnd);

          /* First find all the available printer queues in the system */
          /* which will be place in a listbox of the Select Printer    */
          /* dialog window.                                            */
          splErr = SplEnumQueue((PSZ)NULL, 3, pBuf, 0L,
                                &cbReturned, &cbTotal, &cbNeeded, NULL);
          if(splErr == ERROR_MORE_DATA)
          {
            if(!DosAllocMem((PVOID)&pBuf, cbNeeded,
                            PAG_READ | PAG_WRITE | PAG_COMMIT))
            {
              cbBuf = cbNeeded;
              splErr = SplEnumQueue((PSZ)NULL, 3, pBuf, cbBuf,
                                    &cbReturned, &cbTotal, &cbNeeded, NULL);
            }
            else
            {
              CHAR title[21];
              CHAR text[81];
              CHAR szError[200];

              WinLoadString (hab, 0L, IDS_PRINTERROR, 20, title );
              WinLoadString (hab, 0L, IDS_ERROR_PRINT_1, 80, text );
              sprintf(szError, "%s %d.", text, splErr);
              WinMessageBox (HWND_DESKTOP, 0L, szError,
                             title, 1, MB_ERROR | MB_OK | MB_MOVEABLE);

            }
          }

          if(splErr == NO_ERROR && cbReturned > 0)
          {
            pPrqInfo = (PPRQINFO3)pBuf; /* save this address, use later       */
            prq      = (PPRQINFO3)pBuf; /* temporary, will increment          */
            for(i = 0; i < cbReturned; i++)
            {                           /* insert printer queues in listbox   */
              WinSendDlgItemMsg(hwnd, IDL_PRINT, LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END, 0),
                                MPFROMP(prq->pszName));
              prq++;
            }
            /* select the first printer queue automatically */
            WinSendDlgItemMsg( hwnd, IDL_PRINT, LM_SELECTITEM,
                       MPFROMLONG( 0 ), MPFROMSHORT( TRUE ) );
          }
          else
          {
            CHAR title[21];
            CHAR text[81];
            CHAR szError[200];

            DosFreeMem(pBuf);
            WinLoadString (hab, 0L, IDS_PRINTERROR, 20, title );
            WinLoadString (hab, 0L, IDS_ERROR_PRINT_2, 80, text );
            sprintf(szError, "%s %d.", text, splErr);

            WinMessageBox (HWND_DESKTOP, 0L, szError,
                           title, 1, MB_ERROR | MB_OK | MB_MOVEABLE);

          }
        }
        break;                         /* end case (WM_INITDLG)               */

     case WM_COMMAND:
        switch (SHORT1FROMMP(mp1))
           {
            case DID_OK:               /* User clicked on OK push button      */
               {
                 LONG      i;
                 LONG      l;
                 PPRQINFO3 prq;
                 LONG      printgraph;

                 /* Determine which printer queue the user selected in listbox*/
                 /* and position prq pointer to that address                  */
                 prq = pPrqInfo;
                 i = (LONG) WinSendDlgItemMsg( hwnd, IDL_PRINT,
                                               LM_QUERYSELECTION, 0L, 0L);
                 for(l=0; l< i; l++)
                   prq++;
                                       /* Print graph option checkbox         */
                 printgraph = (LONG) WinSendDlgItemMsg( hwnd, IDC_PRINTGRAPH,
                                     BM_QUERYCHECK, 0L, 0L );

                 fnProcessPrint(hwnd, printgraph);  /* Main print processing  */

                 WinDismissDlg( hwnd, TRUE );       /* Remove the dialog box  */
               }                       /* end case (DID_OK)                   */
               break;

            case IDP_JOBPROP:          /* User clicked on Job Properties      */
               {
                 CHAR      achDriverName[DRIVERNAME_LENGTH];
                 CHAR      achDeviceName[DEVICENAME_LENGTH];
                 LONG      i;
                 LONG      l;
                 PSZ       pszTemp;
                 PPRQINFO3 prq;

                 i = (LONG) WinSendDlgItemMsg( hwnd, IDL_PRINT,
                                               LM_QUERYSELECTION, 0L, 0L);
                 prq = pPrqInfo;
                 for(l=0; l< i; l++)
                   prq++;

                 pDriverData = prq->pDriverData;

                 /* The pszDriverName is of the form DRIVER.DEVICE (e.g.,     */
                 /* LASERJET.HP LaserJet IID) so we need to separate it       */
                 /* at the dot                                                */
                 i = strcspn(prq->pszDriverName, ".");
                 if (i)
                 {
                   strncpy(achDriverName, prq->pszDriverName, i);
                   achDriverName[i] = '\0';
                   strcpy(achDeviceName, &prq->pszDriverName[i + 1]);
                 }
                 else
                 {
                   strcpy(achDriverName, prq->pszDriverName);
                   *achDeviceName = '\0';
                 }

                 /* There may be more than one printer assigned to this print */
                 /* queue. We will use the first in the comma separated list. */
                 /* We would /* need an expanded dialog for the user to be    */
                 /* more specific.                                            */
                 pszTemp = strchr(prq->pszPrinters, ',');
                 if( pszTemp )
                 {
                   /* Strip off comma and trailing printer names */
                   *pszTemp = '\0' ;
                 }

                 /* Post the job properties dialog for the printer to allow   */
                 /* the user to modify the options                            */
                 l = DevPostDeviceModes( hab,
                                         pDriverData,
                                         achDriverName,
                                         achDeviceName,
                                         prq->pszPrinters,
                                         DPDM_POSTJOBPROP );
               }
               break;

            case DID_CANCEL:           /* User clicked on Cancel push button  */
               DosFreeMem(pPrqInfo);
               WinDismissDlg( hwnd, FALSE );
               break;

            case IDP_HELPPRINT:
               break;

            default:                   /* Let PM handle rest of messages      */
               mr = WinDefDlgProc( hwnd, msg, mp1, mp2 );
               break;
           }
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefDlgProc( hwnd, msg, mp1, mp2 );
        break;
    }

 return( mr );

}

/******************************************************************************/
/*                                                                            */
/* wpPrtStatDlgProc: Window procedure for Print Status dialog window.         */
/*                                                                            */
/******************************************************************************/

MRESULT EXPENTRY wpPrtStatDlgProc (HWND hwnd, ULONG msg, MPARAM mp1,
                                MPARAM mp2)

{

 MRESULT mr = (MRESULT) FALSE;
 static  HAB hab;

 switch (msg)
    {
     case WM_INITDLG:                  /* Dialog box just created. Initialize */
        {                              /* list box, etc.                      */
           LONG printgraph;

           printgraph = * (PLONG) PVOIDFROMMP (mp2);

           hab = WinQueryAnchorBlock (hwnd);

           hwndPrint = hwnd;           /* Save the window handle, print thread*/
                                       /* will send status msgs during process*/
           WinSendMsg(hwndClient, UM_PRINT, MPFROMLONG(FALSE), 0L);
                                       /* Create thread to perform actual work*/
           DosCreateThread( &threadID, (PFNTHREAD)fnPrint,
                            (ULONG)printgraph, 0L, STACKSIZE );

        }
        break;                         /* end case (WM_INITDLG)               */

     case UM_PRINTSTATUS:              /* Status message received from print  */
        {                              /* thread. Instead info in listbox     */
          LONG      i;
          LONG      item;
          CHAR      szStatus[81];
          CHAR      barIndicator[PROGRESSBARLENGTH+1];
          LONG      barCounter;

          item = LONGFROMMP(mp1);
                                       /* Parameter passed in card address    */

          switch(item)
          {
            case( 1 ):
              WinLoadString (hab, 0L, IDS_PRINTLIST, 80, szStatus );
              break;
            case( 2 ):
              WinLoadString (hab, 0L, IDS_PRINTCHART, 80, szStatus );
              break;
            case( 3 ):
              WinLoadString (hab, 0L, IDS_PRINTLEGEND, 80, szStatus );
              break;
            default:
              break;
          }
          WinSendDlgItemMsg(hwnd, IDL_STATUS, LM_INSERTITEM,
                                  MPFROM2SHORT(LIT_END, 0),
                                  MPFROMP(szStatus));

                                       /* Update progress indicator           */
          strcpy( barIndicator, "лллллллллллллллллллллллллллллл");
          /*                     123456789012345678901234567890 */
          barCounter = PROGRESSBARLENGTH * item / 3;
          for(i = PROGRESSBARLENGTH; i > barCounter; i--)
            barIndicator[i-1] = ' ';
          if(i>0)                      /* Update the entry field with bar     */
            WinSetWindowText(WinWindowFromID(hwnd, IDE_PROGRESS), barIndicator);

        }
        break;                         /* end case UM_PRINTSTATUS             */

     case UM_STOPPRINT:                /* User clicked on Stop button or      */
        {                              /* the print thread has completed,     */
                                       /* let's go some cleanup work here     */
          DosKillThread(threadID);
                                       /* Enable appropriate menu options     */
          WinSendMsg(hwndClient, UM_PRINT, MPFROMLONG(TRUE), 0L);

          DosFreeMem(pPrqInfo);        /* Free memory allocated earlier       */

          WinDismissDlg( hwnd, TRUE ); /* Remove the dialog box               */
        }
        break;                         /* end case (UM_STOPPRINT)             */

     case WM_COMMAND:
        switch (SHORT1FROMMP(mp1))
           {
            case DID_CANCEL:           /* User clicked on Stop button.        */
               {
                 CHAR title[21];
                 CHAR text[81];

                 WinLoadString (hab, 0L, IDS_PRINTTEXT, 20, title );
                 WinLoadString (hab, 0L, IDS_STOPPRINT, 80, text );
                 if( WinMessageBox( HWND_DESKTOP, 0, text, title,
                                    IDD_STOPPRINT,
                                    MB_MOVEABLE | MB_YESNO |
                                    MB_HELP     |
                                    MB_DEFBUTTON1 ) == MBID_YES )
                   WinSendMsg(hwnd, UM_STOPPRINT, 0L, 0L);
               }                       /* end case (DID_OK)                   */
               break;

            default:                   /* Let PM handle rest of messages      */
               mr = WinDefDlgProc( hwnd, msg, mp1, mp2 );
               break;
           }
        break;

     default:                          /* Let PM handle rest of messages      */
        mr = WinDefDlgProc( hwnd, msg, mp1, mp2 );
        break;
    }

 return( mr );

}

/******************************************************************************/
/*                                                                            */
/* fnProcessPrint: Main print process. Just display Print dialog box.         */
/*                                                                            */
/******************************************************************************/

VOID fnProcessPrint( HWND hwnd, LONG printgraph )
{

  WinLoadDlg(HWND_DESKTOP, 0L, wpPrtStatDlgProc, 0L, IDD_PRINTSTATUS,
             (PVOID)&printgraph);

}

/******************************************************************************/
/*                                                                            */
/* fnPrint: Print thread processing.                                          */
/*                                                                            */
/******************************************************************************/

LONG fnPrint( LONG printgraph )
{

  CHAR         achDriverName[DRIVERNAME_LENGTH];
  CHAR         achDeviceName[DEVICENAME_LENGTH];
  LONG         i;
  LONG         l;
  DEVOPENSTRUC dos;
  HDC          hdc;
  HPS          hps;
  HMQ          hmq;
  APIRET       rc;
  HAB          habPrn;

  habPrn = WinInitialize (0);          /* initialization procedures, PM and   */
  hmq = WinCreateMsgQueue (habPrn, 0); /* application message queue           */

  i = strcspn(pPrqInfo->pszDriverName, ".");
  if (i)
  {
    strncpy(achDriverName, pPrqInfo->pszDriverName, i);
    achDriverName[i] = '\0';
    strcpy(achDeviceName, &pPrqInfo->pszDriverName[i + 1]);
  }
  else
  {
    strcpy(achDriverName, pPrqInfo->pszDriverName);
    *achDeviceName = '\0';
  }

  /* Build the device context data for DevOpenDC */
  memset((PVOID)&dos, 0, sizeof(dos));

  dos.pszLogAddress = pPrqInfo->pszName;
  dos.pszDriverName = (PSZ)achDriverName;
  dos.pdriv = pDriverData;
  dos.pszDataType = (PSZ)"PM_Q_RAW";
  dos.pszComment = "DISKUSE";
  dos.pszQueueProcParams = "COP=1";
  dos.pszSpoolerParams = (PSZ)NULL;

  /* Create DC for the printer */
  hdc = DevOpenDC( habPrn, OD_QUEUED, "*", 9L, (PVOID)&dos, (HDC)NULLHANDLE );

  /* Create PS for the printer */
  if (hdc != DEV_ERROR)
  {
    SIZEL sizel;

    sizel.cx = 0;
    sizel.cy = 0;
    hps = GpiCreatePS(habPrn, hdc, &sizel, PU_ARBITRARY | GPIA_ASSOC );

    if (hps != GPI_ERROR)
    {
      POINTL    point;
      APIRET    rc;
      HCINFO    hcInfo[MAX_PRINT_FORMS];
      PDIRINFO  item;
      CHAR      szHeader[80];

      /* Determine the hard copy capabilities of the printer, set width and*/
      /* height to pass to fnDrawCard routine.                             */
      l = DevQueryHardcopyCaps(hdc, 0L, i, hcInfo);
      l = 0;
      while((hcInfo[l].flAttributes != HCAPS_CURRENT) && l < MAX_PRINT_FORMS)
        l++;
      GpiQueryPS(hps, &sizel);

      /* Issue STARTDOC to begin printing */
      DevEscape( hdc, DEVESC_STARTDOC, 8L, (PBYTE)"DISKUSE\0",
                 (PLONG)NULL, (PBYTE)NULL );

      if( directorytree == RIGHT )
        WinLoadString( habPrn, 0L, IDS_LISTHDRRIGHT, 80,
                                   szHeader );
      else
        WinLoadString( habPrn, 0L, IDS_LISTHDRLEFT, 80,
                                   szHeader );
      strcat(szHeader, "\0");
      DevEscape( hdc, DEVESC_RAWDATA,
                 strlen(szHeader), (PBYTE)szHeader,
                 (PLONG)NULL, (PBYTE)NULL );
      DevEscape( hdc, DEVESC_RAWDATA,
                 sizeof("\r\n"), (PBYTE)"\r\n",
                 (PLONG)NULL, (PBYTE)NULL );

      /* Print routine here ... */
      item = start;
      while( item )
      {
        DevEscape( hdc, DEVESC_RAWDATA,
                   sizeof(item->listtext), (PBYTE)item->listtext,
                   (PLONG)NULL, (PBYTE)NULL );
        DevEscape( hdc, DEVESC_RAWDATA,
                   sizeof("\r\n"), (PBYTE)"\r\n",
                   (PLONG)NULL, (PBYTE)NULL );

        item = (PDIRINFO)item->next;
      }
      DevEscape( hdc, DEVESC_NEWFRAME, 0L, (PBYTE)NULL,
                 (PLONG)NULL, (PBYTE)NULL );

      WinSendMsg(hwndPrint, UM_PRINTSTATUS, MPFROMLONG(1), 0L);

      if(printgraph)
      {
        PCHARTINFO  pci;
        PLEGENDINFO pli;
        FONTMETRICS fm;

        GpiQueryFontMetrics( hps, sizeof(FONTMETRICS), &fm );

        /* Print graph pie/chart here */
        pci = (PCHARTINFO)malloc(sizeof(CHARTINFO));
        pci->hwnd     = 0L;
        pci->hab      = habPrn;
        pci->cxChar   = (LONG)fm.lAveCharWidth;
        pci->cyChar   = (LONG)fm.lMaxBaselineExt;
        pci->cyDesc   = (LONG)fm.lMaxDescender;
        pci->cxClient = sizel.cx;
        pci->cyClient = sizel.cy;
        pci->ptctr.x = (pci->cxClient) / 2;
        pci->ptctr.y = (pci->cyClient - pci->cyChar ) / 2 + pci->cyChar;
        if( pci->ptctr.x > pci->ptctr.y - pci->cyChar )
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
            pci->radius = MAKEFIXED(pci->ptctr.x - 2*pci->cyChar, 0 );
        }

        fnDrawChart( hps, pci );

        free( pci );

        DevEscape( hdc, DEVESC_NEWFRAME, 0L, (PBYTE)NULL,
                   (PLONG)NULL, (PBYTE)NULL );

        WinSendMsg(hwndPrint, UM_PRINTSTATUS, MPFROMLONG(2), 0L);

        /* Print graph legend here */

        pli = (PLEGENDINFO)malloc(sizeof(LEGENDINFO));
        pli->hwnd        = 0;
        pli->hab         = habPrn;
        pli->cxChar      = (LONG)fm.lAveCharWidth;
        pli->cxCaps      = (LONG)fm.lMaxCharInc;
        pli->cyChar      = (LONG)fm.lMaxBaselineExt;
        pli->cyDesc      = (LONG)fm.lMaxDescender;
        pli->cyHeight    = (LONG)fm.lXHeight;
        pli->cyLowHeight = (LONG)fm.lLowerCaseAscent;
        pli->cxClient    = sizel.cx;
        pli->cyClient    = sizel.cy;
        if( graphlevel == GRAPH_LEVEL_1ST )
        {
//        pli->cxTextTotal = 8 * pli->cxChar + pli->cxChar * char1stdirectories;
          pli->cxTextTotal = 3 * pli->cxChar + pli->cxChar * char1stdirectories;
          if( total1stdirectories < graphmaxpies - 2 )
            pli->cyLineTotal = total1stdirectories + 2;
          else
            pli->cyLineTotal = graphmaxpies;
        }
        else
        {
//        pli->cxTextTotal = 6 * pli->cxChar + pli->cxChar * chardirectories;
          pli->cxTextTotal = 3 * pli->cxChar + pli->cxChar * chardirectories;
          if( totaldirectories < graphmaxpies - 2 )
            pli->cyLineTotal = totaldirectories + 2;
          else
            pli->cyLineTotal = graphmaxpies;
        }
        pli->sHscrollMax = max( 0, pli->cxTextTotal - pli->cxClient );
        pli->sHscrollPos = min( pli->sHscrollPos, pli->sHscrollMax );
        pli->sVscrollMax = max( 0, pli->cyLineTotal - pli->cyClient / pli->cyChar );
        pli->sVscrollPos = min( pli->sVscrollPos, pli->sVscrollMax );

        fnDrawLegend( hps, pli );

        free( pli );

        DevEscape( hdc, DEVESC_NEWFRAME, 0L, (PBYTE)NULL,
                   (PLONG)NULL, (PBYTE)NULL );

        WinSendMsg(hwndPrint, UM_PRINTSTATUS, MPFROMLONG(3), 0L);

      }

      /* End print routine */

      /* Issue ABORTDOC if user cancelled print job, or ENDDOC */
      /* to for normal job termination.                        */
      DevEscape( hdc, DEVESC_ENDDOC, 0L, (PBYTE)NULL,
                 (PLONG)NULL, (PBYTE)NULL );

      /* Release and destroy the PS */
      GpiAssociate( hps, (HDC)NULLHANDLE );
      GpiDestroyPS( hps );
    }
    else
    {
      CHAR title[21];
      CHAR text[81];
      CHAR szError[200];

      WinLoadString (habPrn, 0L, IDS_PRINTERROR, 20, title );
      WinLoadString (habPrn, 0L, IDS_ERROR_PRINT_3, 80, text );
      sprintf(szError, "%s %d.", text, hps);

      WinMessageBox (HWND_DESKTOP, 0L, szError,
                     title, 1, MB_ERROR | MB_OK | MB_MOVEABLE);

    }
  }
  else
  {
    CHAR title[21];
    CHAR text[81];
    CHAR szError[200];

    WinLoadString (habPrn, 0L, IDS_PRINTERROR, 20, title );
    WinLoadString (habPrn, 0L, IDS_ERROR_PRINT_4, 80, text );
    sprintf(szError, "%s %d.", text, hdc);
    WinMessageBox (HWND_DESKTOP, 0L, szError,
                   title, 1, MB_ERROR | MB_OK | MB_MOVEABLE);
  }

  /* Clean up the DC */
  if (hdc != DEV_ERROR)
    DevCloseDC(hdc);

  /* Send message to Print dialog window that we are finished */
  WinSendMsg(hwndPrint, UM_STOPPRINT, 0L, 0L);

  WinDestroyMsgQueue (hmq);
  WinTerminate (habPrn);

  DosExit( EXIT_THREAD, 0L );

}
