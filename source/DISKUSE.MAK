diskuse.exe: diskuse.obj diskdlgs.obj diskprt.obj diskproc.obj diskuse.def diskuse.res diskuse.hlp
        link386 /NOI /MAP diskuse.obj diskdlgs.obj diskprt.obj diskproc.obj,,,,diskuse.def;
        rc diskuse.res diskuse.exe

diskuse.hlp: diskuse.ipf
        ipfc diskuse.ipf

diskuse.res: diskuse.rc diskuse.h diskdlg.h diskuse.dlg diskuse.ico diskuse.bmp
        rc -r diskuse.rc

diskuse.obj: diskuse.c diskuse.h diskproc.h diskdlg.h
        icc /c /Ss /O+ /Ti- diskuse.c

diskdlgs.obj: diskdlgs.c diskuse.h diskproc.h diskdlg.h
        icc /c /Ss /O+ /Ti- diskdlgs.c

diskprt.obj: diskprt.c diskuse.h diskproc.h diskdlg.h
        icc /c /Ss /O+ /Ti- diskprt.c

diskproc.obj: diskproc.c diskproc.h
        icc /c /Ss /O+ /Ti- diskproc.c
