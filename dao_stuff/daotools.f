C
C#######################################################################
C
      SUBROUTINE  HELP (CMD, NCMD, RCMD, NUMBER)
C
C=======================================================================
C
C This subroutine produces a simple listing on the terminal of all of
C the elements of the character vector CMD.  They are sorted into 
C alphabetical order by the first two characters.
C
C             OFFICIAL DAO VERSION:  1991 April 18
C
C Argument
C
C CMD is a character array containing the names of the defined commands.
C
C=======================================================================
C
      IMPLICIT NONE
C
      INTEGER ICNVRT, I, NCMD
C
C NCMD is the total number of defined commands.
C
      CHARACTER*10 CMD(NCMD)
      REAL RCMD(NCMD)
      INTEGER NUMBER(NCMD)
C
C-----------------------------------------------------------------------
C
      WRITE (6,610)
  610 FORMAT (/' The commands currently recognized are:'/)
C
C Determine the numerical equivalent of the first two characters in each
C of the defined commands.
C
      DO 1010 I=1,NCMD
 1010 RCMD(I)=FLOAT(ICNVRT(CMD(I)(1:2)))
C
C Sort the commands into alphabetical order.
C
      CALL QUICK (RCMD, NCMD, NUMBER)
C
C Now type the command names out on the screen.
C
      WRITE (6,611) (CMD(NUMBER(I)), I=1,NCMD)
  611 FORMAT (1X, 5A14)
      WRITE (6,612)
  612 FORMAT (/' Any command may be abbreviated down to its first two',
     .     ' characters.')
C
      RETURN
      END!
C
C#######################################################################
C
      SUBROUTINE  GETSKY  (D, S, INDEX, MAX, READNS, HIBAD, SKYMN, 
     .     SKYMED, SKYMOD, SKYSIG, N)
C
C=======================================================================
C
C This subroutine estimates an average sky value for a picture by taking
C individual pixels scattered over the picture.  The brightness values 
C are sorted, and the modal value is estimated using the MMM subroutine.
C
C               OFFICIAL DAO VERSION:  1991 April 18
C
C=======================================================================
C
      IMPLICIT NONE
      INTEGER MAX
C
C MAX    is the maximum number of sky pixels we can deal with,
C        given the limited amount of working space.
C
      REAL S(MAX), D(MAX), AMAX1
      INTEGER INDEX(MAX)
C
      REAL READNS, HIBAD, SKYMN, SKYMED, SKYMOD, SKYSIG, SKYSKW
      INTEGER NCOL, NROW, ISTEP, LX, LY, NX, NY, IROW, I, N, K
      INTEGER ISTAT, IFIRST
      COMMON /SIZE/ NCOL, NROW
C
C-----------------------------------------------------------------------
C
C The spacing between pixels that will be included in the sample is
C estimated by the ratio of the total number of pixels in the picture to
C the maximum number of pixels that can be accomodated in the vector S.
C
      ISTEP = NCOL*NROW/MAX+1
C
C Go through the disk file reading a row at a time and extracting every 
C ISTEP-th pixel.  If ISTEP is not equal to 1, make sure that the
C starting pixel for each row is staggered.
C
      LX = 1
      NX = NCOL
      NY = 1
      IFIRST = 0
      N = 0
      DO IROW=1,NROW
         LY = IROW
         CALL RDARAY ('DATA', LX, LY, NX, NY, MAX, D, ISTAT)
         IF (ISTAT .NE. 0) RETURN
         IFIRST = IFIRST + 1
         IF (IFIRST .GT. ISTEP) IFIRST = IFIRST - ISTEP
         I = IFIRST
 1010    IF (ABS(D(I)) .LE. HIBAD) THEN
            N = N+1
            S(N) = D(I)
            k = nint(s(n))
            IF (N .EQ. MAX) GO TO 1100
            I = I + ISTEP
         ELSE
            I = I+1
         END IF
         IF (I .LE. NCOL) GO TO 1010
      END DO
C
C Sort these values, then estimate the mode.
C
 1100 CONTINUE
      CALL QUICK (S, N, INDEX)
      CALL MMM (S, N, HIBAD, READNS, SKYMN, SKYMED, SKYMOD, SKYSIG, 
     .     SKYSKW, 1.0)
      K = INT(ALOG10(AMAX1(1., SKYMN, -10.*SKYMN, SKYMED, 
     .     -10.*SKYMED, SKYMOD, -10.*SKYMOD, SKYSIG))) + 7
      WRITE (6,610) SKYMOD, SKYSIG
  610 FORMAT (/'       Modal sky value for this frame =', F9.3/
     .         ' Standard deviation of sky brightness =', F9.3/)
            WRITE (6,6) SKYMN, SKYMED, N
    6       FORMAT ('              Clipped mean and median =', 
     .           2F9.3/
     .         '   Number of pixels used (after clip) =', I9)

C Normal return.
C
      RETURN
      END!
C
C#######################################################################
C
      SUBROUTINE  APPEND
C
C=======================================================================
C
C A simple subroutine to append two DAOPHOT stellar data files, 
C omitting the superfluous file header.
C
C=======================================================================
C
      IMPLICIT NONE
      CHARACTER*132 LINE
      CHARACTER*200 IFILE1, IFILE2, SWITCH, CASE*4
      REAL R1, R2, R3, R4, R5, R6, R7
      INTEGER ISTAT, K, I1, I2, I3
C
C-----------------------------------------------------------------------
C
      CALL TBLANK                                   ! Type a blank line
      IFILE1=' '
      CALL GETNAM ('First input file:', IFILE1)
      IF (IFILE1 .EQ. 'END-OF-FILE') THEN
         IFILE1 = ' '
         RETURN
      END IF
C
  950 CALL INFILE (1, IFILE1, ISTAT)
      IF ((IFILE1 .EQ. 'END-OF-FILE') .OR. 
     .     (IFILE1 .EQ. 'GIVE UP')) THEN
         IFILE1 = ' '
         RETURN
      END IF
C
      IF (ISTAT .NE. 0) THEN
         CALL STUPID ('Error opening input file '//IFILE1)
         IFILE1 = 'GIVE UP'
         GO TO 950
      END IF
C
      IFILE2=' '
  960 CALL GETNAM ('Second input file:', IFILE2)
      IF ((IFILE2 .EQ. 'END-OF-FILE') .OR.
     .     (IFILE2 .EQ. 'GIVE UP')) THEN
         CALL CLFILE (1)
         RETURN
      END IF
      CALL INFILE (2, IFILE2, ISTAT)
      IF (ISTAT .NE. 0) THEN
         CALL STUPID ('Error opening input file '//IFILE2)
         IFILE2 = 'GIVE UP'
         GO TO 960
      END IF
C
      IFILE1 = SWITCH(IFILE1, CASE('.cmb'))
  970 CALL GETNAM ('Output file:', IFILE1)
      IF ((IFILE1 .EQ. 'END-OF-FILE') .OR.
     .     (IFILE1 .EQ. 'GIVE UP')) THEN
         CALL CLFILE (1)
         CALL CLFILE (2)
         RETURN
      END IF
      CALL OUTFIL (3, IFILE1, ISTAT)
      IF (ISTAT .NE. 0) THEN
         CALL STUPID ('Error opening output file '//IFILE1)
         IFILE1 = 'GIVE UP'
         GO TO 970
      END IF
C
C-----------------------------------------------------------------------
C
C Copy first file's header and data verbatim into the output file.
C
 2000 CALL RDCHAR (1, LINE, K, ISTAT)
      IF (ISTAT .GT. 0) GO TO 2900
      IF (ISTAT .LT. 0) GO TO 2000
      IF (K .LE. 0) LINE=' '
      K = MAX0(1,K)
      WRITE (3,310) LINE(1:K)
  310 FORMAT (A)
      GO TO 2000
C
 2900 CALL CLFILE (1)
C
C-----------------------------------------------------------------------
C
C Add to the output file the stellar data, but not the header, from the
C second input file.
C
      I1=-1
      CALL RDHEAD (2, I1, I2, I3, R1, R2, R3, R4, R5, R6, R7)
C
C RDHEAD will leave the pointer positioned at the top of the input 
C file's stellar data whether there was a header there or not.  Now
C copy the remainder of the second input file verbatim into the output
C file.
C
 3010 CALL RDCHAR (2, LINE, K, ISTAT)
      IF (ISTAT .GT. 0) THEN
         CALL CLFILE (2)
         CALL CLFILE (3)
         RETURN
      END IF
      IF (ISTAT .LT. 0) GO TO 3010
      K=MAX0(1,K)
      WRITE (3,310) LINE(1:K)
      GO TO 3010
      END!
C
C#######################################################################
C
      SUBROUTINE  DAOSLT (ID, XC, YC, MAG, SKY, MAX)
C
C=======================================================================
C
C This is a simple subroutine which selects groups within a certain 
C range of sizes from a group file, and puts them into a new group file.
C
C              OFFICIAL DAO VERSION: 1991 April 18
C
C=======================================================================
C
      IMPLICIT NONE
      INTEGER MAX
C
C MAX is the largest number of stars that can be held in working space.
C
      CHARACTER*200 COOFIL, MAGFIL, PSFFIL, PROFIL, GRPFIL, EXTEND
      CHARACTER CASE*3
      REAL XC(MAX), YC(MAX), MAG(MAX), SKY(MAX)
      REAL SIZE(2)
      INTEGER ID(MAX)
C
      REAL ALOG10
C
      CHARACTER*4 PLSTR, PLGRP
      REAL LOBAD, HIBAD, THRESH, AP1, PHPADU, RONOIS, DUM, RADIUS
      INTEGER ISTAT, NL, NCOL, NROW, MINGRP, MAXGRP
      INTEGER NGRP, NTOT, I, NSTAR, LENGRP
C
      COMMON /FILNAM/ COOFIL, MAGFIL, PSFFIL, PROFIL, GRPFIL
C
C-----------------------------------------------------------------------
C
C SECTION 1
C
C Setup.
C
C Ascertain the name of the input group file and open it.
C
      CALL TBLANK                                   ! Type a blank line
  950 CALL GETNAM ('Input group file:', GRPFIL)
      IF ((GRPFIL .EQ. 'END-OF-FILE') .OR.
     .     (GRPFIL .EQ. 'GIVE UP')) THEN
         GRPFIL = ' '
         RETURN
      END IF
C
      CALL INFILE (2, GRPFIL, ISTAT)
      IF (ISTAT .NE. 0) THEN
         CALL STUPID ('Error opening input file '//GRPFIL)
         GRPFIL = 'GIVE UP'
         GO TO 950
      END IF
C
      CALL RDHEAD (2, NL, NCOL, NROW, LOBAD, HIBAD, THRESH, AP1, 
     .     PHPADU, RONOIS, DUM)
      IF (NL .NE. 3) THEN
         CALL STUPID ('Not a group file.')
         CALL CLFILE (2)
         GRPFIL = 'GIVE UP'
         GO TO 950
      END IF
C
C Get the desired range of group sizes.
C
      CALL GETDAT ('Minimum, maximum group size:', SIZE, 2)
      IF (SIZE(1) .LT. -1.E30) GO TO 9010           ! CTRL-Z was entered
      MINGRP=NINT(SIZE(1))
      MAXGRP=NINT(SIZE(2))
C
C Get the name of the output group file and open it.
C
      MAGFIL=GRPFIL
  960 CALL GETNAM ('Output group file:', MAGFIL)
      IF ((MAGFIL .EQ. 'END-OF-FILE') .OR.
     .     (MAGFIL .EQ. 'GIVE UP')) THEN
         CALL CLFILE (2)
         MAGFIL = ' '
         RETURN
      END IF
C
      MAGFIL = EXTEND(MAGFIL, CASE('grp'))
      CALL OUTFIL (3, MAGFIL, ISTAT)
      IF (ISTAT .NE. 0) THEN
         CALL STUPID ('Error opening output file '//MAGFIL)
         MAGFIL = 'GIVE UP'
         GO TO 960
      END IF
      CALL WRHEAD (3, 3, NCOL, NROW, 6, LOBAD, HIBAD, THRESH, AP1, 
     .     PHPADU, RONOIS, RADIUS)
      NGRP=0
      NTOT=0
C
C-----------------------------------------------------------------------
C
C SECTION 2
C
C Actually do it.
C
C Read in the next group of stars.
C
 2000 I=0                                       ! Begin loop over groups
 2010 I=I+1                                     ! Begin loop over stars
C
      IF (I .GT. MAX) THEN                 ! Too many stars in group
         CALL STUPID ('/Group too large.')
         WRITE (6,6) MAX
    6    FORMAT (I10, ' is the largest number of stars I can',
     .        ' possibly consider.  I''m throwing it out.'/)
C
C Look for the next blank line.
C
 2015    CALL RDSTAR (2, 3, ID(1), XC(1), YC(1), MAG(1), SKY(1))
         IF (ID(1) .GT. 0) GO TO 2015
         IF (ID(1) .EQ. 0) GO TO 2000
         GO TO 9000
      END IF
C
      CALL RDSTAR (2, 3, ID(I), XC(I), YC(I), MAG(I), SKY(I))
C
      IF (ID(I) .LT. 0) GO TO 2020         ! End-of-file was encountered
      IF (ID(I) .EQ. 0) GO TO 2030         ! Blank line was encountered
      GO TO 2010
C
C Either a blank line or an EOF has been encountered.  If there
C have been some stars read in since the last blank line, we
C want to write out the group, if appropriate.  Otherwise,
C if EOF was encountered, then return; if a blank line, then keep
C trying to start reading a new group.
C
 2020 IF (I .EQ. 1) GO TO 9000        ! EOF and no stars in group
 2030 IF (I .EQ. 1) GO TO 2000        ! Blank line and no stars in group
      NSTAR=I-1
C
C NSTAR is the number of stars in the current group.  If this is outside
C the range of group sizes being selected, start reading the next group.
C Otherwise, write the group into the output file and increment the
C accumulators before reading the next group.
C
      IF ((NSTAR .LT. MINGRP) .OR. (NSTAR .GT. MAXGRP)) GO TO 2000
      NGRP=NGRP+1
      NTOT=NTOT+NSTAR
      DO 2040 I=1,NSTAR
 2040 WRITE (3,320) ID(I), XC(I), YC(I), MAG(I), SKY(I)
  320 FORMAT (1X, I5, 4F9.3)
      WRITE (3,320)                                 ! Write a blank line
      GO TO 2000
C
C-----------------------------------------------------------------------
C
C Normal return.
C
C Type out the number of stars and the number of groups NEATLY.
C
 9000 PLSTR='s in'
      IF (NTOT .EQ. 1) PLSTR=' in '
      PLGRP='s.  '
      IF (NGRP .EQ. 1) PLGRP='.   '
      LENGRP=INT(ALOG10(NGRP+0.5))+2
      IF (NTOT .EQ. 1)LENGRP=LENGRP-1
      WRITE (6,690) NTOT, PLSTR, NGRP, PLGRP
  690 FORMAT (/I6, ' star', A4, I5, ' group', A4/)
      CALL CLFILE (3)
 9010 CALL CLFILE (2)
      RETURN
      END!
C
C#######################################################################
C
      SUBROUTINE  DUMP  (F, NCOL, NROW)
C
C=======================================================================
C
C A trivial subroutine to type the brightness values in a small 
C subarray of the picture onto the terminal.
C
C            OFFICIAL DAO VERSION:  1991 April 18
C
C=======================================================================
C
      IMPLICIT NONE
C
      INTEGER NSQUARE, NCOL, NROW
      PARAMETER  (NSQUARE=21)
C
      REAL F(NCOL,NROW)
      REAL COORDS(2), D(NSQUARE*NSQUARE)
C
      REAL AMIN1, AMAX1
      INTEGER NINT
C
      
      REAL SIZE
      INTEGER NBOX, NHALF, LX, LY, ISTAT
      INTEGER I, J, N, NX, NY
C
C Parameter
C
C NSQUARE is the side of the largest square subarray that can be 
C         comfortably fit on the terminal screen.  NSQUARE = 21 is
C         for 24-line terminals, to accomodate the array, a two-line
C         header across the top, and and a query at the bottom.  In 
C         fact, if the user specifies SIZE = 21, then one of the header
C         lines at the top will be lost.
C         (Terminal must have 132-column capability to prevent 
C         wraparound.)
C
C
C-----------------------------------------------------------------------
C
      CALL TBLANK                                   ! Type a blank line
      CALL GETDAT ('Box size:', SIZE, 1)
      IF (SIZE .LT. -1.E30) RETURN                  ! CTRL-Z was entered
      NBOX=MAX0(1, MIN0(NINT(SIZE), NSQUARE))
      NHALF=(NBOX-1)/2
C
 1000 CALL GETDAT ('Coordinates of central pixel:', COORDS, 2)
C
C Return to calling program upon entry of CTRL-Z or invalid coordinates.
C
      IF ((COORDS(1) .LE. 0.5) .OR. (COORDS(2) .LE. 0.5)) RETURN
      IF ((NINT(COORDS(1)) .GT. NCOL) .OR.
     .     (NINT(COORDS(2)) .GT. NROW)) RETURN
C 
      LX = NINT(COORDS(1))-NHALF
      LY = NINT(COORDS(2))-NHALF
      NX = NINT(COORDS(1))+NHALF - LX + 1
      NY = NINT(COORDS(2))+NHALF - LY + 1
      CALL RDARAY ('DATA', LX, LY, NX, NY, NCOL, F, ISTAT)
C
C LX and LY are the lower limits of the box in X and Y; NX and NY are
C the number of pixels in the box in X and Y.  They will have been
C modified by RDARAY if the box would have extended outside the 
C picture.
C
      WRITE (6,609) (I, I=LX,LX+NX-1)
  609 FORMAT (/7X, 21I6)
      WRITE (6,610) ('------', I=1,NX)
  610 FORMAT (6X, '+', 21A6)
C
      N=0
      DO 1020 J=NY,1,-1
      WRITE (6,611) LY+J-1, 
     .     (NINT(AMAX1(-99999.,AMIN1(99999.,F(I,J)))), I=1,NX)
  611 FORMAT (1X, I4, ' |', 21I6)
      DO 1010 I=1,NX
      N=N+1
 1010 D(N)=F(I,J)
 1020 CONTINUE
C
      CALL QUICK (D, N, F)
C
      IF (NBOX .LT. NSQUARE) CALL TBLANK
      WRITE (6,612) NINT(AMAX1(-99999.,AMIN1(99999.,D(1)))), 
     .     NINT(AMAX1(-99999.,AMIN1(99999.,
     .     0.5*D((N+1)/2)+0.5*D((N/2)+1)))), 
     .     NINT(AMAX1(-99999.,AMIN1(99999.,D(N))))
  612 FORMAT(26X, 'Minimum, median, maximum: ', 3I7)
C
      GO TO 1000
C
      END!
C
C#######################################################################
C
      SUBROUTINE  OFFSET
C
C=======================================================================
C
C A simple routine to read in an arbitrary DAOPHOT stellar data file,
C shift the stars' x,y coordinates by a constant amount, and write
C out an otherwise identical data file.
C
C            OFFICIAL DAO VERSION:  1991 April 18
C
C=======================================================================
C
      IMPLICIT NONE
      CHARACTER*133 LINE1, LINE2
      CHARACTER*200 FILE, SWITCH
      CHARACTER CASE*4
      REAL DELTA(4)
      REAL LOBAD, COLMAX, ROWMAX, HIBAD, THRESH, AP1, PHPADU
      REAL READNS, FRAD, X, Y, AMAG
      INTEGER I, ISTAT, IDDEL, NL, NCOL, NROW, ITEMS, NLINE1
      INTEGER NLINE2, ID
      LOGICAL WROTE
C
C-----------------------------------------------------------------------
C
      WROTE=.FALSE.

C SECTION 1
C
C Get ready.
C
      CALL TBLANK                                   ! Type a blank line
C
C Get input file name.
C
      FILE=' '
  950 CALL GETNAM ('Input file name:', FILE)
      IF ((FILE .EQ. 'END-OF-FILE') .OR. (FILE .EQ. 'GIVE UP')) RETURN
      CALL INFILE (2, FILE, ISTAT)
      IF (ISTAT .NE. 0) THEN
         CALL STUPID ('Error opening input file '//FILE)
         FILE = 'GIVE UP'
         GO TO 950
      END IF
C
C Get offsets.
C
      DO I=1,4
         DELTA(I)=0.
      END DO
      CALL GETDAT ('Additive offsets ID, DX, DY, DMAG:', DELTA, 4)
      IF (DELTA(1) .LT. -1.E30) THEN
         CALL CLFILE (1)
         RETURN
      END IF
      IDDEL = NINT(DELTA(1))
C
C Get output file name.
C
      FILE = SWITCH(FILE, CASE('.off'))
  960 CALL GETNAM ('Output file name:', FILE)
      IF ((FILE .EQ. 'END-OF-FILE') .OR. (FILE .EQ. 'GIVE UP')) THEN
         CALL CLFILE (2)
         RETURN
      END IF
      CALL OUTFIL (3, FILE, ISTAT)
      IF (ISTAT .NE. 0) THEN
         CALL STUPID ('Error opening output file '//FILE)
         FILE = 'GIVE UP'
         GO TO 960
      END IF
C
C Copy input file's header into output file.
C
      COLMAX=1.E30
      ROWMAX=1.E30
      NL=-1
      CALL RDHEAD (2, NL, NCOL, NROW, LOBAD, HIBAD, THRESH, AP1, 
     .     PHPADU, READNS, FRAD)
C
      IF (NL .LE. 0) GO TO 2000                     ! No header in input
C
      ITEMS=6
      IF (FRAD .GT. 0.001) ITEMS=7
      CALL WRHEAD (3, NL, NCOL, NROW, ITEMS, LOBAD, HIBAD, THRESH, 
     .     AP1, PHPADU, READNS, FRAD)
      COLMAX=FLOAT(NCOL)+0.5
      ROWMAX=FLOAT(NROW)+0.5
C
C-----------------------------------------------------------------------
C
C SECTION 2
C
C Copy the data file, line by line, altering X and Y as we go.
C
 2000 CALL RDCHAR (2, LINE1, NLINE1, ISTAT)
      IF (ISTAT .GT. 0) GO TO 9000
      IF (NLINE1 .LE. 1) THEN
C
C A blank line was encountered.  The toggle, WROTE, prevents more than
C one blank output line in a row from being produced.
C
         IF (WROTE) WRITE (3,*) ' '
         WROTE=.FALSE.
         GO TO 2000
      END IF
C
      READ (LINE1, *, IOSTAT=ISTAT) ID, X, Y, AMAG
      IF (ISTAT .NE. 0) THEN
         CALL STUPID ('Error reading data from input file.')
         WRITE (6,*) LINE1(1:NLINE1)
         CALL TBLANK
         RETURN
      END IF
C
      IF (NL .EQ. 2) CALL RDCHAR (2, LINE2, NLINE2, ISTAT)
      IF (ISTAT .NE. 0) THEN
         CALL STUPID ('Error reading data from input file.')
         WRITE (6,*) LINE1(2:NLINE2)
         CALL TBLANK
         RETURN
      END IF
      ID=ID+IDDEL
      X=X+DELTA(2)
      Y=Y+DELTA(3)
      AMAG=AMAG+DELTA(4)
CC
CC Any star which winds up outside the picture after offsetting will be
CC discarded.  (This implicitly assumes that the user will only be
CC offsetting the coordinates in order to match them with another frame
CC taken with the same device.  Otherwise the scale and possibly the
CC orientation would be different, and a simple offset would not be a
CC good enough transformation.)
CC
CC                    Removed 1995 April 26
CC
C     IF ((X .LT. 0.5) .OR. (X .GT. COLMAX) .OR. (Y .LT. 0.5) .OR. 
C    .     (Y .GT. ROWMAX)) GO TO 2000
C
      IF ((X .LT. -999.99) .OR. (X .GT. 9999.99) .OR. (Y .LT. -999.99)
     .     .OR. (Y .GT. 9999.99)) THEN
         WRITE (LINE1(1:33),220) ID, X, Y, AMAG
  220    FORMAT (I6, 2F9.2, F9.3)
      ELSE
         WRITE (LINE1(1:33),221) ID, X, Y, AMAG
  221    FORMAT (I6, 3F9.3)
      END IF
      WRITE (3,301) LINE1(1:MAX0(33,NLINE1))
  301 FORMAT (A)
      IF (NL .EQ. 2) WRITE (3,301) LINE2(1:NLINE2)
      WROTE=.TRUE.
      GO TO 2000
C
C-----------------------------------------------------------------------
C
C Normal return.
C
 9000 CALL CLFILE (3)
      CALL CLFILE (2)
      RETURN
      END!
