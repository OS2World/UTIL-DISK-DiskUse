/******************************************************************************/
/*                       (c) IBM Canada Ltd. 1992                             */
/*                                                                            */
/* Program:     DISKPROC (used by DISKUSE)                                    */
/* Description: Get all drive and subdirectory information using OS/2's       */
/*              Dos calls.                                                    */
/*                                                                            */
/* Routines:    fnGetValidDrives                                              */
/*              fnGetDriveInfo                                                */
/*              fnGetDirectoryInfo                                            */
/*              fnPutLinkList                                                 */
/*              fnPutLinkListSorted                                           */
/*              fnPutLinkList1stSorted                                        */
/*              fnFreeList                                                    */
/*                                                                            */
/* History: 18Jan93  ELZ  Fix for routine for determine valid logical drives  */
/*                        by using bit map from LogicalDriveMap.              */
/*          18Mar93  ELZ  Fix DosFindFirst to add FILE_xxxxxxx attrib flags.  */
/*          31Aug93  ELZ  Determine drive volume label.                       */
/******************************************************************************/

#define INCL_DOSMISC

#include <os2.h>
#include <string.h>
#include <stdio.h>
#include "diskproc.h"

/******************************************************************************/
/*                                                                            */
/* fnGetValidDrives: Get a list of all valid drives in the system and place   */
/*                   in a linked list (pointer to structure is PDRIVES,       */
/*                   starting address is drive).                              */
/*                                                                            */
/******************************************************************************/

PDRIVES fnGetValidDrives( VOID )
{

  UCHAR  FSInfoBuf[40];
  APIRET rc;
  ULONG  i;
  ULONG  DriveNo;
  ULONG  LogicalDriveMap;

  PDRIVES drive, dstart, dlast;

  dstart = NULL;
  dlast  = NULL;

  DosQueryCurrentDisk( &DriveNo, &LogicalDriveMap );
  LogicalDriveMap &= 0xffffffff;       /* bit map of all valid logical drives */
  for( i = 0; i < 26; i++ )            /* from 0-25                           */
  {
    if( LogicalDriveMap & (1L<<i) )
    {
      /* valid drive */
      drive = (PDRIVES)malloc(sizeof(DRIVES));
      drive->drivenumber = (SHORT)i+1;
      drive->driveletter[0] = (CHAR)(drive->drivenumber + 64);
      drive->driveletter[1] = ':';
      drive->driveletter[2] = '\0';
      drive->totalsize = 0;
      drive->free = 0;
      if( !dlast )
      {
        dlast  = drive;
        dstart = drive;
      }
      else
        dlast->next = drive;
      drive->next = NULL;
      dlast = drive;
    }
  }

  return dstart;

}

/******************************************************************************/
/*                                                                            */
/* fnGetDriveInfo: Get drive information. Input is drive number (ie. 3 is     */
/*                 C, 4=D, 5=E, etc.). Returns total size, total free and     */
/*                 total used in bytes.                                       */
/*                                                                            */
/******************************************************************************/

USHORT fnGetDriveInfo( ULONG DriveNumber, ULONG *size, ULONG *free,
                       ULONG *used, ULONG *alloc, ULONG *swapsize,
                       CHAR  *volume )
{

  UCHAR  FSInfoBuf[40];                /* data buffer area                    */
  APIRET rc;                           /* result code                         */

  PDRIVEINFO   dinfo;                  /* define return structure for drive   */
  PDRIVEVOLSER dvolser;                /* information                         */

  rc = DosQueryFSInfo(DriveNumber, FSIL_ALLOC, (PVOID)FSInfoBuf,
                      40);
  if (!rc)                             /* no error, put data in return vars   */
  {
     dinfo = (PDRIVEINFO)FSInfoBuf;    /* cast data buffer into my own struct */
     *size = dinfo->bytesnum * dinfo->sector * dinfo->unit;   /* total size   */
     *free = dinfo->bytesnum * dinfo->sector * dinfo->avail;  /* total free   */
     *used = *size - *free;                                   /* total used   */
     *alloc = dinfo->bytesnum * dinfo->sector;            /* allocation units */
     *swapsize = 0;                                       /* SWAPPER.DAT size */
  }
                                       /* Determine volume information        */
  rc = DosQueryFSInfo(DriveNumber, FSIL_VOLSER, (PVOID)FSInfoBuf,
                      40);
  if (!rc)
  {
     dvolser = (PDRIVEVOLSER)FSInfoBuf;
     strcpy(volume, dvolser->volumelabel);
  }

  return (rc);                          /* return the rc from DosQueryFSInfo  */

}

/******************************************************************************/
/*                                                                            */
/* fnGetDirectoryInfo: This recursive routine finds all the files and         */
/*                     directories in the given search directory (input).     */
/*                     First, all the files in the current directory          */
/*                     are found. Next, all subdirectories in the current     */
/*                     directory are found. For each of these subdirectories, */
/*                     this function is called recursively.                   */
/*                                                                            */
/* Calls: Itself!                                                             */
/*        fnPutLinkList                                                       */
/*        fnPutLinkListSorted                                                 */
/*                                                                            */
/******************************************************************************/

VOID fnGetDirectoryInfo( UCHAR *searchdir, UCHAR *dirname, UCHAR *prefix,
                       ULONG size, ULONG used, ULONG allocunits,
                       USHORT level, ULONG *cumbytes, BOOL lastdir,
                       BOOL swap, ULONG *swapsize )
{

  UCHAR         xdirname[255];         /* work string for directory name      */
  UCHAR         xprefix[255];          /* work string for prefix (tree)       */
  BOOL          xlastdir;              /* flag: indicate if last subdir       */
  ULONG         cfilenames = 1;        /* needed by DosFindxxxx calls         */
  HDIR          hdir = HDIR_SYSTEM;    /* directory handle                    */
  FILEFINDBUF3  findbuf;               /* data structure for file information */
  APIRET        rc;                    /* return code                         */
  PDIRINFO      dirinfo;               /* this is the data linked list!       */
  PSUBDIRINFO   subdirinfo;            /* linked list of subdirectories       */
  PSUBDIRINFO   end = NULL;            /* initialize head and tail of linked  */
  PSUBDIRINFO   top = NULL;            /* list to NULL                        */
  USHORT        i;                     /* used as a counter                   */
  USHORT        total_subdirectory = 0; /* total number of subdirectories     */

  dirinfo = (PDIRINFO) malloc(sizeof(DIRINFO));
  if(!dirinfo)                         /* allocate memory for this item in    */
  {                                    /* linked list, address stored in      */
    printf( "Out of memory.\n" );      /* dirinfo variable                    */
    return;
  }

  /* Initialization processing here first ...                                 */

  strcpy(dirinfo->fulldirname, searchdir );
  dirinfo->files    = 0;               /* total number of files in directory  */
  if( level > 0 )
    dirinfo->bytes  = allocunits;      /* size of directory                   */
  else
    dirinfo->bytes  = 0;
  dirinfo->percent1 = 0;               /* percent of total space              */
  dirinfo->percent2 = 0;               /* percent of used space only          */
  dirinfo->level    = level;           /* directory level (0=root)            */

  /* Build the directory tree which will show in the directory list window    */

  strcpy( dirinfo->dirname, "" );
  strcpy( dirinfo->directory, dirname );
  strcat( dirinfo->dirname, prefix );
  if( lastdir == 1 )
    strcat( dirinfo->dirname, "ÀÄ" );
  else
  {
    if( level > 0 )
      strcat( dirinfo->dirname, "ÃÄ" );
    else
      strcat( dirinfo->dirname, "\\ " );
  }
  strcat( dirinfo->dirname, dirname );

  /* Now search for all files (no subdirectories) in the current directory    */
  /* Add to number of files and directory size                                */

  strcpy( searchdir, dirinfo->fulldirname);
  strcat( searchdir, "\\*.*");         /* "*.*" means search all the files    */
  rc = DosFindFirst( (PSZ)searchdir,
                     &hdir,
                     FILE_NORMAL | FILE_SYSTEM | FILE_HIDDEN |
                     FILE_ARCHIVED | FILE_READONLY,
                     (PVOID) &findbuf,
                     sizeof(findbuf),
                     &cfilenames,
                     FIL_STANDARD);
  if (!rc)                             /* check return code from Dos call     */
  {                                    /* yup, found a file, add to totals    */
    do
    {
      float tbytes;
      ULONG xbytes;

      dirinfo->files = dirinfo->files + 1;
      tbytes = (float)findbuf.cbFile / (float)allocunits + 0.99;
      xbytes = (ULONG)tbytes * allocunits;

      if( strncmp( findbuf.achName, "SWAPPER.DAT",
                  strlen( "SWAPPER.DAT" ) ) == 0 )
      {
        *swapsize = xbytes;
        if( swap == 0 )
          dirinfo->bytes = dirinfo->bytes + xbytes;
      }
      else
        dirinfo->bytes = dirinfo->bytes + xbytes;
      rc = DosFindNext( hdir,
                        &findbuf,
                        sizeof(findbuf),
                        &cfilenames);
    } while (!rc );                    /* keep looking until no more          */
  }                                    /* end if( !rc )                       */

  DosFindClose(hdir);                  /* close the file handle               */

  /* Put appropriate information in the elements of the data structure        */

  dirinfo->percent1 = (float)dirinfo->bytes * 100 / size;
  dirinfo->percent2 = (float)dirinfo->bytes * 100 / used;
  dirinfo->cumbytes = dirinfo->bytes;
  *cumbytes         = dirinfo->bytes;

  /* Place into double linked list, first as non-sorted and then as sorted    */
  /* by decending size                                                        */

  fnPutLinkList( dirinfo );
  startsort    = fnPutLinkListSorted( dirinfo, startsort );
  if( strlen( dirinfo->fulldirname ) > chardirectories )
    chardirectories = strlen( dirinfo->fulldirname );

  totalfiles       = totalfiles    + dirinfo->files;
  totaldirectories = totaldirectories + 1;

  /* Now search for all subdirectories in the current directory. Store the    */
  /* list of subdirectories found in a linked list (PSUBDIRINFO) and we will  */
  /* loop through later on.                                                   */

  cfilenames = 1;
  hdir = HDIR_SYSTEM;
  rc = DosFindFirst( (PSZ)searchdir,       /* look for only subdirectories    */
                     &hdir,
                     MUST_HAVE_DIRECTORY | FILE_ARCHIVED | FILE_SYSTEM |
                     FILE_READONLY | FILE_HIDDEN,
                     (PVOID) &findbuf,
                     sizeof(findbuf),
                     &cfilenames,
                     FIL_STANDARD);
  if (!rc)                             /* check return code, looks good so    */
  {                                    /* continue on and get subdir info     */
    do                                 /* ignore "." and ".." as they are     */
    {                                  /* really not subdirectories           */
        if (strcmp( findbuf.achName, "." ) != 0 &&
            strcmp( findbuf.achName, "..") != 0 )
        {                              /* ensure we only get subdirectories   */
          if( findbuf.attrFile & FILE_DIRECTORY )
          {
            subdirinfo = (PSUBDIRINFO) malloc(sizeof(SUBDIRINFO));
            strcpy( subdirinfo->dirname, findbuf.achName );
            if( !end )                 /* the next few lines builds a linked  */
            {                          /* lists containing all subdirectories */
              top = subdirinfo;        /* head of list                        */
              end = subdirinfo;        /* tail of list                        */
            }
            else
              end->next = (PSUBDIRINFO)subdirinfo;
            subdirinfo->next = NULL;
            end = subdirinfo;
            total_subdirectory++;
          }
        }
        rc = DosFindNext( hdir,        /* find the next subdirectory          */
                          &findbuf,
                          sizeof(findbuf),
                          &cfilenames);
    } while (!rc );                    /* keep going until no more found      */
  }                                    /* end if( !rc )                       */

  DosFindClose(hdir);

  i = 0;                               /* counter                             */
  level++;                             /* since a subdirectory, must be next  */
  while( top )                         /* level, so increment it              */
  {                                    /* loop thru all subdirs in linked list*/
    i++;                               /* increment subdirectory counter      */
    strcpy( xprefix, prefix );         /* build the tree string properly      */
    if( lastdir == 1 )
    {
      if( level > 1 )
        strcat( xprefix, "  " );
    }
    else
    {
      if( level > 1 )
        strcat( xprefix, "³ " );
    }
    strcpy( xdirname, dirinfo->fulldirname);
    strcat( xdirname, "\\" );          /* build the input to the function call*/
    strcat( xdirname, top->dirname );  /* to fnGetDirectoryInfo               */
    if( i == total_subdirectory )
      xlastdir = 1;
    else
      xlastdir = 0;
    fnGetDirectoryInfo( xdirname, top->dirname, xprefix, size, used, allocunits,
                        level, cumbytes, xlastdir, swap, swapsize );
    dirinfo->cumbytes = dirinfo->cumbytes + *cumbytes;
    end = top;
    top = (PSUBDIRINFO)top->next;      /* ok, next subdirectory in list       */
    free( end );                       /* free the memory for this element    */
  }                                    /* and around we go again in the loop  */
                                       /* cumulative information here ...     */
  dirinfo->cumpercent1 = (float)dirinfo->cumbytes * 100 / size;
  dirinfo->cumpercent2 = (float)dirinfo->cumbytes * 100 / used;
  *cumbytes = dirinfo->cumbytes;

  return;

}

/******************************************************************************/
/*                                                                            */
/* fnPutLinkListSorted: Put directory information into a double linked list   */
/*                      sorted by descending size.                            */
/*                                                                            */
/******************************************************************************/

PDIRINFO fnPutLinkListSorted( PDIRINFO item, PDIRINFO top )
{

  PDIRINFO old, p;

  if( lastsort == NULL )
  {
    item->nextsort  = NULL;
    item->priorsort = NULL;
    lastsort = item;
    return item;
  }

  p = top;
  old = NULL;
  while( p )
  {
    if( p->bytes > item->bytes )
    {
      old = p;
      p = (PDIRINFO)p->nextsort;
    }
    else
    {
      if( (PDIRINFO)p->priorsort )
      {
        ((PDIRINFO)(p->priorsort))->nextsort = (PDIRINFO)item;
        item->nextsort = (PDIRINFO)p;
        item->priorsort = (PDIRINFO)p->priorsort;
        p->priorsort = (PDIRINFO)item;
        return top;
      }
      item->nextsort = (PDIRINFO)p;
      item->priorsort = NULL;
      p->priorsort = (PDIRINFO)item;
      return item;
    }
  }
  old->nextsort = (PDIRINFO)item;
  item->nextsort = NULL;
  item->priorsort = (PDIRINFO)old;
  lastsort = item;
  return top;

}

/******************************************************************************/
/*                                                                            */
/* fnPutLinkList1stSorted: Put 1st level directory information into a linked  */
/*                         list sorted by descending size.                    */
/*                                                                            */
/******************************************************************************/

PDIRINFO fnPutLinkList1stSorted( PDIRINFO item, PDIRINFO top )
{

  PDIRINFO old, p;

  if( item->level > 1 )
    return top;

  total1stdirectories = total1stdirectories + 1;
  if( strlen( item->fulldirname ) > char1stdirectories )
    char1stdirectories = strlen( item->fulldirname );

  if( lastsort1st == NULL )
  {
    item->nextsort1st  = NULL;
    item->priorsort1st = NULL;
    lastsort1st = item;
    return item;
  }

  p = top;
  old = NULL;
  while( p )
  {
    if( p->cumbytes > item->cumbytes )
    {
      old = p;
      p = (PDIRINFO)p->nextsort1st;
    }
    else
    {
      if( (PDIRINFO)p->priorsort1st )
      {
        ((PDIRINFO)(p->priorsort1st))->nextsort1st = (PDIRINFO)item;
        item->nextsort1st  = (PDIRINFO)p;
        item->priorsort1st = (PDIRINFO)p->priorsort1st;
        p->priorsort1st    = (PDIRINFO)item;
        return top;
      }
      item->nextsort1st  = (PDIRINFO)p;
      item->priorsort1st = NULL;
      p->priorsort1st    = (PDIRINFO)item;
      return item;
    }
  }
  old->nextsort1st   = (PDIRINFO)item;
  item->nextsort1st  = NULL;
  item->priorsort1st = (PDIRINFO)old;
  lastsort1st = item;
  return top;

}

/******************************************************************************/
/*                                                                            */
/* fnPutLinkList: Put directory information into a double linked list.        */
/*                                                                            */
/******************************************************************************/

VOID fnPutLinkList( PDIRINFO item )
{

  if( last == NULL )
  {
    last = item;
    start = item;
    item->prior = NULL;
  }
  else
  {
    last->next = (PDIRINFO)item;
    item->prior = (PDIRINFO)last;
  }
  item->next = NULL;
  last = item;

}


/******************************************************************************/
/*                                                                            */
/* fnFreeList: Free memory for double linked list.                            */
/*                                                                            */
/******************************************************************************/

VOID fnFreeList( VOID )
{

  PDIRINFO item, temp;

  item = start;
  while( item )
  {
    temp = (PDIRINFO)item->next;
    free( item );
    item = temp;
  }

}

/*****************************************************************************/
/*                                                                           */
/*                  DIRECTORY DELETION ROUTINES                              */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

APIRET fnGetDeleteDirectoryInfo( UCHAR *fulldirname )
{

  UCHAR          xdirname[255];
  ULONG          cfilenames;
  HDIR           hdir;
  FILEFINDBUF3   findbuf;
  APIRET         rc;
  PDELDIRINFO    dirinfo;
  PDELSUBDIRINFO subdirinfo;
  PDELSUBDIRINFO end = NULL;
  PDELSUBDIRINFO top = NULL;

  cfilenames = 1;
  hdir = HDIR_SYSTEM;

  dirinfo = (PDELDIRINFO) malloc(sizeof(DELDIRINFO));
  if(!dirinfo)
  {
    printf( "Out of memory.\n" );
    return( 99 );
  }

  strcpy(dirinfo->fulldirname, fulldirname );
  dirinfo->files = 0;
  dirinfo->bytes = 0;

  strcat( fulldirname, "\\*.*");
  rc = DosFindFirst( (PSZ)fulldirname,
                     &hdir,
                     FILE_NORMAL | FILE_SYSTEM | FILE_HIDDEN | FILE_ARCHIVED,
                     (PVOID) &findbuf,
                     sizeof(findbuf),
                     &cfilenames,
                     FIL_STANDARD);
  if (!rc)
  {
    do
    {
      dirinfo->files = dirinfo->files + 1;
      dirinfo->bytes = dirinfo->bytes + findbuf.cbFile;
      rc = DosFindNext( hdir,
                        &findbuf,
                        sizeof(findbuf),
                        &cfilenames);
    } while (!rc );
    if( rc != 18 )
      return( rc );
  }
  else
  {
    if( rc != 18 )
      return( rc );
  }

  DosFindClose(hdir);

  fnPutDeleteList( dirinfo );

  cfilenames = 1;
  hdir = HDIR_SYSTEM;

  rc = DosFindFirst( (PSZ)fulldirname,
                     &hdir,
                     MUST_HAVE_DIRECTORY | FILE_ARCHIVED,
                     (PVOID) &findbuf,
                     sizeof(findbuf),
                     &cfilenames,
                     FIL_STANDARD);

  if (!rc)
  {
    do
    {
      if (strcmp( findbuf.achName, "." ) != 0 &&
          strcmp( findbuf.achName, "..") != 0 )
      {
        if( findbuf.attrFile & FILE_DIRECTORY )
        {
          subdirinfo = (PDELSUBDIRINFO) malloc(sizeof(DELSUBDIRINFO));
          strcpy( subdirinfo->dirname, findbuf.achName );
          if(!end)
          {
            top = subdirinfo;
            end = subdirinfo;
          }
          else
            end->next = (PDELSUBDIRINFO)subdirinfo;
          subdirinfo->next = NULL;
          end = subdirinfo;
        }
      }
      rc = DosFindNext( hdir, &findbuf, sizeof(findbuf), &cfilenames);
    } while (!rc );
    if( rc != 18 )
      return( rc );
  }
  else
  {
    if( rc != 18 )
      return( rc );
  }

  DosFindClose(hdir);

  while( top )
  {
    strcpy( xdirname, dirinfo->fulldirname);
    strcat( xdirname, "\\" );
    strcat( xdirname, top->dirname );
    fnGetDeleteDirectoryInfo( xdirname );
    end = top;
    top = (PDELSUBDIRINFO)top->next;
    free( end );
  }

  return( 0 );

}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

APIRET fnDeleteDirectories( VOID )
{

  PDELDIRINFO  item;
  APIRET       rc;
  UCHAR        dirname[256];
  UCHAR        filename[256];
  ULONG        cfilenames;
  FILEFINDBUF3 findbuf;
  HDIR         hdir;
  FILESTATUS3  fileinfobuf;

  item = dellast;
  while( item )
  {
    printf( "Deleting %s\n", item->fulldirname );

    cfilenames = 1;
    hdir = HDIR_SYSTEM;

    strcpy( dirname, item->fulldirname );
    strcat( dirname, "\\*.*");

    rc = DosFindFirst( (PSZ)dirname, &hdir,
                     FILE_NORMAL | FILE_SYSTEM | FILE_HIDDEN,
                     (PVOID) &findbuf, sizeof(findbuf), &cfilenames,
                     FIL_STANDARD);
    if (!rc)
    {
      do
      {
        strcpy( filename, item->fulldirname );
        strcat( filename, "\\" );
        strcat( filename, findbuf.achName );

        rc = DosQueryPathInfo(filename, 1, &fileinfobuf, sizeof(fileinfobuf));
        fileinfobuf.attrFile = FILE_NORMAL;
        rc = DosSetPathInfo(filename, 1L, &fileinfobuf, sizeof(fileinfobuf), 0L);

        rc = DosDelete( filename );
        if( rc != 0 )
        {
          printf( "DosDelete failed for %s with a return code of %d.\n",
                  filename, rc );
          DosFindClose(hdir);
          return( rc );
        }
        rc = DosFindNext( hdir, &findbuf, sizeof(findbuf), &cfilenames);
      } while (!rc );
      if( rc != 18 )
      {
        printf( "DosFindNext failed with a return code of %d.\n", rc );
        DosFindClose(hdir);
        return( rc );
      }
    }
    else
    {
      if( rc != 18 )
      {
        printf( "DosFindFirst failed with a return code of %d.\n", rc );
        return( rc );
      }
    }

    DosFindClose(hdir);

    rc = DosDeleteDir( item->fulldirname );
    if( rc != 0 )
    {
      printf( "DosDelete failed for %s with a return code of %d.\n",
              item->fulldirname, rc );
      return( rc );
    }

    item = (PDELDIRINFO)item->prior;
  }

}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

VOID fnPutDeleteList( PDELDIRINFO item )
{

  if( dellast == NULL )
  {
    dellast = item;
    delstart = item;
    item->prior = NULL;
  }
  else
  {
    dellast->next = (PDELDIRINFO)item;
    item->prior = (PDELDIRINFO)dellast;
  }
  item->next = NULL;
  dellast = item;

}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

VOID fnFreeDeleteList( VOID )
{

  PDELDIRINFO item, temp;

  item = delstart;
  while( item )
  {
    temp = (PDELDIRINFO)item->next;
    free( item );
    item = temp;
  }

}
