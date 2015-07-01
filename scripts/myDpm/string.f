C=========================== STRING ===========================
C LAST DATE OF CHANGE 2.03.2012 A. galoyan
      SUBROUTINE STRING(PROJ,TAR,NHAD)
      DIMENSION PROJ(5),TAR(5)
      COMMON/MIDPAR/
     *PXF(1000),PYF(1000),PZF(1000),HEF(1000),AMF(1000),ICHF(1000),
     *IBARF(1000),ANF(1000),NREF(1000)

      COMMON/PART/ANAME(180),AM(180),GA(180),TAU(180),ICH(180),
     *IBAR(180),K1(180),K2(180)

      COMMON/INPDAT/IMPS(6,6),IMVE(6,6),IB08(6,21),IB10(6,21),
     *IA08(6,21),IA10(6,21),A1,B1,B2,B3,ISU,BET,AS,B8,AME,DIQ

      DIMENSION IDSQS(24), IDSAQS(24)
      DIMENSION PQ(6), PQQ(24)

      CHARACTER*8 ANF,ANAME

      DOUBLE PRECISION PX1, PY1, PZ1, QM1, E1
      DOUBLE PRECISION PX2, PY2, PZ2, QM2, E2
      DOUBLE PRECISION S,    SS

      DATA RM/0.30/, RMK/0.5/, DM/1.863/, PDB/0.430/          

      DATA UM/0.3/,SM/0.430/,CM/2.1/

      DATA IDSQS/
     1    1,    2,    3,    4,
     2  100,  200,  300,  400,
     3  707,  708,  709,  710,
     4  807,  808,  809,  810,
     5  907,  908,  909,  910,
     6 1007, 1008, 1009, 1010/

      DATA IDSAQS/
     1    7,    8,    9,   10,
     2  700,  800,  900, 1000,
     3  101,  102,  103,  104,
     4  201,  202,  203,  204,
     5  301,  302,  303,  304,
     6  401,  402,  403,  404/

      SAVE RM, RMK, DM, PDB
      SAVE UM, SM, CM
      SAVE IDSQS, IDSAQS

      IDPROJ=PROJ(1)
      PX1=PROJ(2)
      PY1=PROJ(3)
      PZ1=PROJ(4)
      QM1=PROJ(5)
      E1=DSQRT(PX1**2+PY1**2+PZ1**2+QM1**2)

      IDTAR=TAR(1)
      PX2=TAR(2)
      PY2=TAR(3)
      PZ2=TAR(4)
      QM2=TAR(5)
      E2=DSQRT(PX2**2+PY2**2+PZ2**2+QM2**2)

      IF((E1.EQ.0.).OR.(E2.EQ.0.)) GO TO 7
C
C          FOR TWO JETS EVENT
C
      PASSP=0.
      PASST=0.
      DO 1  I=1,24

      IF(IDPROJ.EQ.IDSQS(I)) PASSP=I
      IF(IDPROJ.EQ.IDSAQS(I)) PASSP=-I

      IF(IDTAR.EQ.IDSQS(I)) PASST=I
      IF(IDTAR.EQ.IDSAQS(I)) PASST=-I
 1    CONTINUE

      IF(PASSP*PASST.LT.0.) GO TO 3

      NHAD=0
*      PRINT 2  ,PROJ, TAR
 2    FORMAT(' FLAVORS OF THE QUARKS SYSTEMS ARE WRONG FOR TWO-JETS ',
     , ' EVENTS'/' PROJ = ',5E11.4/' TAR = ',5E11.4,'-----STRING--')
      RETURN

 3    CONTINUE
      NJET=2

      S=DABS((E1+E2)**2-(PX1+PX2)**2-(PY1+PY2)**2-(PZ1+PZ2)**2)
      SS=DSQRT(S)

      IF(SS.GT.0.) GO TO 5

      NHAD=0
*      PRINT 4  , PROJ, TAR
 4    FORMAT(' ENERGY IS TOO SMALL FOR STRING'/' PROJ = ',5E11.4/
     / ' TAR =',5E11.4/' -----STRING---')
      RETURN
 5    CONTINUE

      GAM=(E1+E2)/SS
      ETTAX=(PX1+PX2)/SS
      ETTAY=(PY1+PY2)/SS
      ETTAZ=(PZ1+PZ2)/SS
C
C          DETERMINATION OF THE MOMENTUM IN C. M. SYSTEM
C
      ETTAP=-(ETTAX*PX1+ETTAY*PY1+ETTAZ*PZ1)
      BRAC=ETTAP/(GAM+1.)+E1
      PX=PX1-ETTAX*BRAC
      PY=PY1-ETTAY*BRAC
      PZ=PZ1-ETTAZ*BRAC
      PT=SQRT(PX**2+PY**2)
      P=SQRT(PT**2+PZ**2)
      E0=SS/2.

      IKF1=IDPROJ-(IDPROJ/100)*100
      IKF2=IDPROJ/100
      IF(IKF1.NE.0) GO TO 6
      IKF1=IKF2
      IKF2=0

 6    CONTINUE

      IKF3=IDTAR-(IDTAR/100)*100
      IKF4=IDTAR/100

      IF(IKF3.NE.0) GO TO 30
      IKF3=IKF4
      IKF4=0
      GO TO 30
C
C
C          FOR SINGLE JET EVENT
C
 7    CONTINUE
      IF(E2.NE.0.) GO TO 11

      PASSP=0.
      DO 8  I=1,24
      IF(IDPROJ.EQ.IDSQS(I)) PASSP=1.
      IF(IDPROJ.EQ.IDSAQS(I)) PASSP=-1.
 8    CONTINUE

      IF(PASSP.NE.0.) GO TO 10

      NHAD=0
*      PRINT 9  ,PROJ
 9    FORMAT(' FLAVOR OF PROJECTILE QUARKS SYSTEM IS WRONG FOR ONE-JET',
     , 'EVENTS'/' PROJ = ',5E11.4/' -----STRING---')
      RETURN

 10   CONTINUE
      PX=PX1
      PY=PY1
      PZ=PZ1
      PT=SQRT(PX1**2+PY1**2)
      P=SQRT(PT**2+PZ1**2)
      E0=P
      NJET=1

      IKF1=IDPROJ-(IDPROJ/100)*100
      IKF2=IDPROJ/100

      IF(IKF1.NE.0) GO TO 30
      IKF1=IKF2
      IKF2=0
      GO TO 30

 11   CONTINUE
C
C     FOR ONE JET EVENTS WITH "TARGET"
C
      PASST=0.
      DO 12  I=1,24
      IF(IDTAR.EQ.IDSQS(I)) PASST=1.
      IF(IDTAR.EQ.IDSAQS(I)) PASST=-1.
 12   CONTINUE

      IF(PASST.NE.0.) GO TO  14

      NHAD=0
*      PRINT 13  , TAR
 13   FORMAT(' FLAVOR OF TARGET QUARKS SYSTEM IS WRONG FOR ONE-JET',
     , ' EVENTS '/' TAR = ',5E11.4/' -----STRING---')
      RETURN
 14   CONTINUE
      PX=PX2
      PY=PY2
      PZ=PZ2
      PT=SQRT(PX2**2+PY**2)
      P=SQRT(PT**2+PZ2**2)
      E0=P
      NJET=1

      IKF1=IDTAR -(IDTAR /100)*100
      IKF2=IDTAR /100

      IF(IKF1.NE.0) GO TO 30
      IKF1=IKF2
      IKF2=0
C
C
C   QUARKS SYSTEM IS QUATE RIGHT FOR STRING
C
C
C
   30 CONTINUE
      IF(P.GE.1.0E-6) GO TO  33
        C13=0.
        C23=0.
        C33=1.
        GO TO 35
 33   CONTINUE
            C13=PX/P
            C23=PY/P
            C33=PZ/P
 35   CONTINUE
      IF(PT.LE.1.0E-10) GO TO 40
      C11=PY/PT
      C21=-PX/PT
      C31=0.
      GO TO 50

   40 CONTINUE
      C11=1.
      C21=0.
      C31=0.

   50 CONTINUE
      C12=C23*C31-C33*C21
      C22=C33*C11-C13*C31
      C32=C13*C21-C23*C11
C
C
C
C   BEGINING OF A FRAGMENTATION
C
C
      IJET=0
      KFA1=IKF1
      KFA2=IKF2
      E=E0
      NHAD=0
      SUMEN=SS

   60 IJET=IJET+1
      PGX=0
      PGY=0
      PGZ=0
      NSTEPS=0

      FTYPE=0.
      IF(KFA1*KFA2.NE.0) FTYPE=1.

   70 CONTINUE
C
C
C          A CHOISE BETWEEN A END OR A CONTINUED OF FRAGMENTATION
C
C
C
      IDQS=KFA1+100*KFA2
      PASS=0.
      DO 71  I=1,24
      IF(IDQS.EQ.IDSQS(I)) PASS=I
      IF(IDQS.EQ.IDSAQS(I)) PASS=I
 71   CONTINUE

      IF(PASS.NE.0.) GO TO 73
      NHAD=0
*      PRINT 72
 72   FORMAT(' SOMETHING IS WRONG IN STRING WITH KFA1 AND KFA2')
      RETURN
 73   CONTINUE
      
      AM0=1
      IF((KFA1.EQ.1 ).OR.(KFA1.EQ. 7)) AM0=RM
      IF((KFA1.EQ.2 ).OR.(KFA1.EQ. 8)) AM0=RM
      IF((KFA1.EQ.3 ).OR.(KFA1.EQ. 9)) AM0=RMK
      IF((KFA1.EQ.4 ).OR.(KFA1.EQ.10)) AM0=DM
      IF((AM0 .EQ.1.).AND.(KFA1.NE.0)) RETURN

      BM0=0.
      IF((KFA2.EQ.1 ).OR.(KFA2.EQ. 7)) BM0=RM
      IF((KFA2.EQ.2 ).OR.(KFA2.EQ. 8)) BM0=RM
      IF((KFA2.EQ.3 ).OR.(KFA2.EQ. 9)) BM0=RMK
      IF((KFA2.EQ.4 ).OR.(KFA2.EQ.10)) BM0=DM
      IF((BM0 .EQ.0.).AND.(KFA2.NE.0)) RETURN

      IF(AM0.LT.BM0) AM0=BM0

      EPS=RNDM1(-1)+1.0E-10
      AMM=AM0-1./B1*ALOG(EPS)
      EPS=RNDM1(-1)+1.0E-10
      ESA=AM0-1./B2*ALOG(EPS)
      PSA=SQRT(abs(ESA**2-AM0**2))
      EAB=SQRT(3./2.*PSA**2+AMM**2)

      IF(E.LE.EAB) GO TO 280
      NSTEPS=NSTEPS+1
C
C          THE  FRAGMENTATION TAKES PLACE
C
C
C          A CHOISE OF A VERTEX
C
 76   CONTINUE

      B8 =0.5      
      AME=0.75
      SM =0.430
      PDB=0.430
      ISU=4
      AS=0.65    ! Uzhi 0 - only heavy resonances, 1. - light ones

      IV=1
      IF((KFA1.NE.0).AND.(KFA2.NE.0)) IV=2

      IF(IV.EQ.2) GO TO 90

 80   IV=1
      B8=0.50                           
      IF(KFA1.LE.6) then
        IF(KFA1.GE.3) THEN
          AME=0.96                        
        ELSE
          AME=0.88                        
        ENDIF
      ELSE
        IF(KFA1.GE.9) THEN
          AME=0.96                        
        ELSE
          AME=0.88                        
        ENDIF
      ENDIF

      IF(RNDM1(-1).GE.AME) IV=2

      SM=0.50                           

      GO TO 100

 90   IV=3
      PDB=0.68   ! 0.43                          ! Uzhi
      SM=0.50    ! 0.600                         ! Uzhi
      B8=0.50    ! 0.80                          ! Uzhi
      if(NSTEPS.eq.1) B8=1.                      ! Uzhi

      IF(RNDM1(-1).GT.PDB) IV=4
      IF((RNDM1(-1).LE.0.5).AND.(IV.EQ.4)) IV=5

 100  CONTINUE

      IF((KFA1.GT.6).OR.(KFA2.GT.6)) IV=IV+5
C
C
C          A CHOISE OF THE CREATED QUARKS FLAVOR
C
C
C     IF(E.LE.1.019) GO TO 110
      PU=BETA(E,UM,BET)
      PD=PU

      PS=BETA(E,SM,BET)
      PC=BETA(E,CM,BET)
      PBB=0.
      PTT=0.

      if((NSTEPS.eq.1).and.(KFA1.eq.KFA2).and.     
     &  ((IV.eq.3).or.(IV.eq.8))) then
        if(KFA1.le.6) then
          if(KFA1.eq.1) then
            PU=0.
            PD=2.*PD
          endif
          if(KFA1.eq.2) then
            PU=2.*PU
            PD=0.
          endif
          if(KFA1.eq.3) PS=0.
        else
          if(KFA1.eq.7) then
            PU=0.
            PD=2.*PD
          endif
          if(KFA1.eq.8) then
            PU=2.*PU
            PD=0.
          endif
          if(KFA1.eq.9) PS=0.
        endif
      endif

      PQ(1)=PU
      PQ(2)=PD
      PQ(3)=PS
      PQ(4)=PC
      PQ(5)=PBB
      PQ(6)=PTT

      ISU=4                             

      ISUS=ISU
      IF(E.LE.2.14  .AND. ISU.GT.3)  ISUS=3     ! Uzhi ?
      IF(E.LE.1.019 .AND. ISUS.GT.2) ISUS=2     ! Uzhi ?

      IF(.NOT.(IV.EQ.2.OR.IV.EQ.7)) THEN
        DO I=2,ISUS
          PQ(I)=PQ(I-1)+PQ(I)
        ENDDO
        EPS=RNDM1(-1)
        DO I=1,ISUS
          IF(EPS.LE.PQ(I)/PQ(ISUS)) GO TO 110
        ENDDO
 110    KF1=I
        KF2=I
      ELSE
        K=0
        SUM=0.
        DO I=1,ISUS
          DO J=I,ISUS
            K=K+1
            PQQ(K)=PQ(I)*PQ(J)
            SUM=SUM+PQQ(K)
          ENDDO
        ENDDO
        DO I=2,K
          PQQ(I)=PQQ(I-1)+PQQ(I)
        ENDDO
        EPS=RNDM1(-1)
        L=0
        DO I=1,ISUS
          DO J=I,ISUS
            L=L+1
            IF(EPS.LE.PQQ(L)/PQQ(K)) GO TO 120
          ENDDO
        ENDDO
 120    KF1=I
        KF2=J
        IF(RNDM1(-1).LE.0.5) THEN
          KF1=J
          KF2=I
        ENDIF
      ENDIF

      GO TO(141,142,143,144,145,146,147,148,149,150),IV
 141  KF2=0.
      GO TO 160
 142  KF1=KF1+6
      KF2=KF2+6
      GO TO 160
 143  KF1=KF1+6
      KF2=0.
      GO TO 160
 144  KF2=KFA2
      GO TO 160
 145  KF1=KFA1
      GO TO 160
 146  KF1=KF1+6
      KF2=0.
      GO TO 160
 147  CONTINUE
      GO TO 160
 148  KF2=0.
      GO TO 160
 149  KF1=KF1+6
      KF2=KFA2
      GO TO 160
 150  KF1=KFA1
      KF2=KF2+6

 160  CONTINUE
C
C          DETERMINATION OF THE PRODUCED PARTICLE INDEX
C
      GO TO(161,162,163,164,165,166,167,168,169,170),IV
 161  LI=KF1+6
      LJ=KFA1
      GO TO 180
 162  LI=KFA1
      KA=KF1-6
      KB=KF2-6
      GO TO 190
 163  LI=KF1-6
      KA=KFA1
      KB=KFA2
      GOTO 190
 164  LI=KF1+6
      LJ=KFA1
      GO TO 180
 165  LI=KF2+6
      LJ=KFA2
      GO TO 180
 166  LI=KFA1
      LJ=KF1-6
      GO TO 180
 167  LI=KFA1
      KA=KF1+6
      KB=KF2+6
      GO TO 200
 168  LI=KF1+6
      KA=KFA1
      KB=KFA2
      GO TO 200
 169  LI=KFA1
      LJ=KF1-6
      GO TO 180
 170  LI=KFA2
      LJ=KF2-6
      GO TO 180

 180  CONTINUE

      AS=0.80                           ! Uzhi
C
C    DETERMINATION OF PRODUCED MESON
C
      IF((LI.GE.7).AND.(LI.LE.12).AND.(LJ.GE.1).AND.(LJ.LE.6)) GO TO 182
      NHAD=0
*      PRINT 181
 181  FORMAT(37H QUARKS FOR MESON IN STRING ARE WRONG)
      RETURN

 182  CONTINUE
      CALL MESON(LI,LJ,INDEX)
      GO TO 210
C
C   DETERMINATION OF PRODUCED BARYON
C
 190  CONTINUE
 200  CONTINUE

      IF(LI.GE.7) GO TO 202

      IF((LI.GE.1).AND.(LI.LE.6).AND.
     .   (KA.GE.1).AND.(KA.LE.6).AND.
     .   (KB.GE.1).AND.(KB.LE.6)) GO TO 203
      NHAD=0
*      PRINT 201
 201  FORMAT(38H QUARKS FOR BARYON IN STRING ARE WRONG)
      RETURN

 202  CONTINUE
      IF((LI.GE.7).AND.(LI.LE.12).AND.
     .   (KA.GE.7).AND.(KA.LE.12).AND.
     .   (KB.GE.7).AND.(KB.LE.12)) GO TO 203
      NHAD=0
*      PRINT 201
      RETURN

 203  CONTINUE

      CALL BARYON(LI,KA,KB,INDEX)

 210  CONTINUE
C
C     THE MASS OF PRODUCED PARTICLE MUST BE LESS THAN ENERGY
C
      HMA=AMASS(INDEX)

      IF(HMA.GT.E)GO TO 76
      NHAD=NHAD+1
      ANF(NHAD)=ANAME(INDEX)
      AMF(NHAD)=HMA
      ICHF(NHAD)=ICH(INDEX)
      IBARF(NHAD)=IBAR(INDEX)
      NREF(NHAD)=INDEX

 240  CONTINUE
C
C
C          DETERMINATION OF THE PRODUCED PARTICLES INPULS
C
C
      B3=11. !4.5                         ! Uzhi

      ESMAX=E-HMA
      PRMAX=EXP(-B3*ESMAX)
      OPRMAX=1.-PRMAX
!       IF(ESMAX.GT.0.05) GO TO 250          !aida
      IF(ESMAX.GT.0.) GO TO 250              !aida
      HE=E
      ES=HE-HMA
      HPS=SQRT(ABS(E**2-HMA**2))
      HPZ=0.0
      GO TO 270

 250  CONTINUE
      EPS =OPRMAX*RNDM1(-1)+PRMAX
      EPS1=OPRMAX*RNDM1(-1)+PRMAX
      PR=EPS*EPS1
      IF(PR.LE.PRMAX) GO TO 250
      ES=-1./B3*ALOG(PR)

 260  CONTINUE
C ===========================================================
C
C
C          DETERMINATION OF THE PRODUCED PARTICLES ENERGY
C
C
C ===========================================================
 211  CONTINUE
      IF(IV.NE.1)    GO TO 212
      ALFA=-0.5
      IF(KF1.GE.3) ALFA=0.

      BE=0.0                             
      IF(KFA1.GE.3) BE=0.35              
      GO TO 221
C =============================================================
 212  CONTINUE
      IF(IV.NE.2)  GO TO 213
      ALFA=1.5
      IF(KF1.GE.9) ALFA=ALFA+0.5
      IF(KF2.GE.9) ALFA=ALFA+0.5

      BE=-0.25                                     
      IF(KFA1.GE.3) BE=0.                          
      GO TO 221
C =============================================================
 213  CONTINUE
      IF(IV.NE.3)  GO TO 214
      ALFA=-0.5
      IF(KF1.GE.9) ALFA=0.

      if(FTYPE.ne.0.) then
        BE=1. !2.                                  ! Uzhi
        IF(KFA1.ne.KFA2) BE=2.                     
c      IF(KFA1.GE.3.OR.KFA2.GE.3) BE=2.5
       FTYPE=0.
      else
        ALFA=1.5                                   
        BE=0.
      endif
      GO TO 221
C ==============================================================
 214  CONTINUE
      IF(IV.NE.4)  GO TO 215
      ALFA=1.5
      IF(KF1.GE.3) ALFA=ALFA+0.5

      BE=0.
      GO TO 221
C ============================================================
 215  CONTINUE
      IF(IV.NE.5)  GO TO 216
      ALFA=1.5
      IF(KF2.GE.3) ALFA=ALFA+0.5

      BE=0.
      GO TO 221
C =======================================================
 216  CONTINUE
      IF(IV.NE.6)  GO TO 217
      ALFA=-0.5
      IF(KF1.GE.9) ALFA=0.

      BE=0.
      IF(KFA1.GE.9) BE=0.35
      GO TO 221
C =======================================================
 217  CONTINUE
      IF(IV.NE.7) GO TO 218
      ALFA=1.5
      IF(KF1.GE.3) ALFA=ALFA+0.5
      IF(KF2.GE.3) ALFA=ALFA+0.5

      BE=-0.25
      IF(KFA1.GE.9) BE=0.           ! 0.35        ! Uzhi
      GO TO 221
C =======================================================
 218  CONTINUE
      IF(IV.NE.8)  GO TO 219
      ALFA=-0.5
      IF(KF1.GE.3) ALFA=0.

      if(FTYPE.ne.0.) then
        BE=1.0  !2. !1.0                          ! Uzhi
        IF(KFA1.ne.KFA2) BE=2.                    
c      IF(KFA1.GE.3.OR.KFA2.GE.3) BE=2.5
       FTYPE=0.
      else
        ALFA=1.5                                  
        BE=0.                                     
      endif
      GO TO 221
C ========================================================
 219  CONTINUE
      IF(IV.NE.9)  GO TO 220
      ALFA=1.5
      IF(KF1.GE.9) ALFA=ALFA+0.5

      BE=0.
      GO TO 221
C ========================================================
 220  CONTINUE
      ALFA=1.5
      IF(KF2.GE.9) ALFA=ALFA+0.5

      BE=0.
C ===================================================

 221  CONTINUE

       HPS=SQRT(ES**2+2.*ES*HMA)

       ALFA=ALFA+0.5  !2.*HPS**2         ! Uzhi
       ALFA=ALFA+1.
       BE=BE+1.
       ALFA=1./ALFA
                   BE=1./BE

 222  RAN1=RNDM1(-1)
      RAN2=RNDM1(-1)
      R1A=RAN1**ALFA
      R2B=RAN2**BE
      R12=R1A+R2B
      IF(R12.GT.1.)  GO TO 222
      ETA=R1A/R12
      ETA=ETA*(1.-(ES+HMA)/E/2.)

      HPZ=(E*(1.-ETA)-(ES+HMA)**2/(1.-ETA)/E/4.)
      HE=SQRT((ES+HMA)**2+HPZ**2)

 270  CONTINUE
      HEF(NHAD)=HE
      E=E-(HE+HPZ)/2.
      HPZ=HPZ*(-1)**(IJET+1)
      SUMEN=SUMEN-HE
      CALL SFECFE(SFE,CFE)
      HPX=HPS*CFE
      HPY=HPS*SFE

      PXF(NHAD)=HPX
      PYF(NHAD)=HPY
      PZF(NHAD)=HPZ

      PGX=PGX+HPX
      PGY=PGY+HPY
      PGZ=PGZ+HPZ

      KFA1=KF1
      KFA2=KF2
C
C
C   RETURN TO THE BEGINING OF THE CIKLE
C
	GO TO 70
C
C
C   THE FRAGMENTATION IS FINISHED
C
C
 280  CONTINUE
      IF(NJET.NE.1) GO TO  320
C
      IF(E2.NE.0.) GO TO 290
C
      PROJ(1)=KFA1+KFA2*100
      PROJ(2)=-C11*PGX-C12*PGY+C13*( -PGZ)
      PROJ(3)=-C21*PGX-C22*PGY+C23*( -PGZ)
      PROJ(4)=-C31*PGX-C32*PGY+C33*( -PGZ)
      PROJ(5)=E
      GO TO 300
C
C
 290  CONTINUE
       TAR(1)=KFA1+KFA2*100
       TAR(2)=-C11*PGX-C12*PGY+C13*( -PGZ)
       TAR(3)=-C21*PGX-C22*PGY+C23*( -PGZ)
       TAR(4)=-C31*PGX-C32*PGY+C33*( -PGZ)
      TAR(5)=E
 300  CONTINUE
C
      IF(NHAD.EQ.0) RETURN
      DO 310  I=1,NHAD
      PX=PXF(I)
      PY=PYF(I)
      PZ=PZF(I)
      PXF(I)=C11*PX+C12*PY+C13*PZ
      PYF(I)=C21*PX+C22*PY+C23*PZ
      PZF(I)=C31*PX+C32*PY+C33*PZ
 310  CONTINUE
      RETURN
C
C
C   THIES PART IS REALIZED ONLY FOR THE TWO JETS EVENTS
C
C
 320  CONTINUE
C
      IF(IJET.EQ.2) GO TO 340
C
C   STORING OF THE FINAL QUARKS CHARCTERISTICS
C
      KR1R=KFA1
      KR2R=KFA2
      RPXR=-PGX
      RPYR=-PGY
      RPZR=-PGZ
      RER=E
      IAR=NHAD
C
C   SETTING UP OF THE OTHER QUARKS SYSTEM
C
      KFA1=IKF3
      KFA2=IKF4
      IF(KFA1.NE.0) GO TO 330
      KFA1=IKF4
      KFA2=0
 330  CONTINUE
      E=E0
      PGX=0.
      PGY=0.
      PGZ=0.
C
C   TRANSITION TO THE OTHER QUARKS SYSTEM FRAGMENTATION
C
      GO TO 60
C
C
 340  CONTINUE
C
C
C   BUILD UP OF A HADRONIC STATE FROM THE FINAL QUARKS SYSTEM
C
C
      KR1L=KFA1
      KR2L=KFA2
      RPXL=-PGX
      RPYL=-PGY
      RPZL=-PGZ
      REL=E
      IAL=NHAD
C
C
      IF((KR1R*KR2R.NE.0).AND.(KR1L*KR2L.NE.0)) GO TO 410
C
C
C   ONE ADDING HADRON IS CREATED
C
C
      EPS=RNDM1(-1)
      IF(IAR*IAL.EQ.0)GO TO 520
      IAK=IAL
      IF(EPS.GE.0.5)IAK=IAR

*      IF((IBARF(IAL).EQ.0).AND.(IBARF(IAR).EQ.0))GO TO 530
*      IF(IBARF(IAR).NE.0)IAK=IAL                            ! Uzhi ???
*      IF(IBARF(IAL).NE.0)IAK=IAR							   ! Uzhi ???
      GO TO 530
 520  CONTINUE
      IF(IAR.EQ.0.AND.IAL.EQ.0)GO TO 400
      IAK=IAL
      IF(IAK.EQ.0)IAK=IAR
 530  CONTINUE
C
C
C   THIES PART IS REALIZED IF THERE ARE THE HADRONS
C          CREATED ON THE FRAGMENTATION STAGE
C
      REG=SUMEN+HEF(IAK)
      RPXG=RPXR+RPXL+PXF(IAK)
      RPYG=RPYR+RPYL+PYF(IAK)
      RPZG=RPZR+RPZL+PZF(IAK)
C
      I=NHAD+1
      J=IAK
C
      IF((KR1R*KR2R.NE.0).OR.(KR1L*KR2L.NE.0)) GO TO 370
C
C   IT IS CREATED ONLY ONE MESON
C
      KF1=KR1R
      IF(KF1.EQ.0) KF1=KR2R
      KF2=KR1L
      IF(KF2.EQ.0) KF2=KR2L
      IF(KF1.GT.6) GO TO 350
      CALL MESON(KF2,KF1,INDEX)
      GO TO 360
C
 350  CONTINUE
      CALL MESON(KF1,KF2,INDEX)
C
 360  CONTINUE
C
      GO TO 390
C
C
 370  CONTINUE
C
C   IT IS CREATED ONE BARYON
C
      IF(KR1R*KR2R.EQ.0) GO TO 380
      KF1=KR1L
      IF(KF1.EQ.0) KF1=KR2L
      CALL BARYON(KF1,KR1R,KR2R,INDEX)
      GO TO 390
C
 380  CONTINUE
C
      KF1=KR1R
      IF(KF1.EQ.0) KF1=KR2R
      CALL BARYON(KF1,KR1L,KR2L,INDEX)
C
C
 390  CONTINUE
C
      NHAD=NHAD+1
      ANF(NHAD)=ANAME(INDEX)
      AMF(NHAD)=AMASS(INDEX)
      ICHF(NHAD)=ICH(INDEX)
      IBARF(NHAD)=IBAR(INDEX)
      NREF(NHAD)=INDEX
      GO TO 440
C
C
 400  CONTINUE
C
C
C   THIES PART IS REALIZED IF THERE ARE NOT ANY HADRONS
C          CREATED ON THE FRAGMENTATION STAGE
C
C
      IF((KR1R*KR2R.NE.0).OR.(KR1L*KR2L.NE.0))GO TO 402
C
C   IT IS CREATED ONLY ONE MESON
C
      KF1=KR1R
      IF(KF1.EQ.0) KF1=KR2R
      KF2=KR1L
      IF(KF2.EQ.0) KF2=KR2L
      IF(KF1.GT.6) GO TO 401
      CALL MESON(KF2,KF1,INDEX)
      GO TO 404
 401  CONTINUE
      CALL MESON(KF1,KF2,INDEX)
      GO TO 404
 402  CONTINUE
C
C   IT IS CREATED ONE BARYON
C
      IF(KR1R*KR2R.EQ.0) GO TO 403
      KF1=KR1L
      IF(KF1.EQ.0) KF1=KR2L
      CALL BARYON(KF1,KR1R,KR2R,INDEX)
      GO TO 404
 403  CONTINUE
C
      KF1=KR1R
      IF(KF1.EQ.0) KF1=KR2R
      CALL BARYON(KF1,KR1L,KR2L,INDEX)
C
 404  CONTINUE
      NHAD=NHAD+1
      ANF(NHAD)=ANAME(INDEX)
      AMF(NHAD)=-1.E-6
      ICHF(NHAD)=ICH(INDEX)
      IBARF(NHAD)=IBAR(INDEX)
      NREF(NHAD)=INDEX
      HEF(NHAD)=E1+E2
      PXF(NHAD)=PX1+PX2
      PYF(NHAD)=PY1+PY2
      PZF(NHAD)=PZ1+PZ2
      
      RETURN
C
C
 410  CONTINUE
C
C
C   THIES PART IS REALIZED IF THE FINAL QUARKS SYSTEM
C          CONSIST FROM TWO DIQUAKRS
C
C     IF((NHAD.EQ.0).AND.(SS.LE.0.3))  RETURN
C
      REG=SUMEN
      RPXG=RPXR+RPXL
      RPYG=RPYR+RPYL
      RPZG=RPZR+RPZL
C
      I=NHAD+2
      J=NHAD+1
C
C
      IF(KR1R.GT.6) GO TO 420
      CALL MESON(KR1L,KR1R,INDX1)
      CALL MESON(KR2L,KR2R,INDX2)
      GO TO 430
C
 420  CONTINUE
      CALL MESON(KR1R,KR1L,INDX1)
      CALL MESON(KR2R,KR2L,INDX2)
C
 430  CONTINUE
C
      ANF(NHAD+1)=ANAME(INDX1)
      AMF(NHAD+1)=AMASS(INDX1)
      ICHF(NHAD+1)=ICH(INDX1)
      IBARF(NHAD+1)=IBAR(INDX1)
      NREF(NHAD+1)=INDX1
      ANF(NHAD+2)=ANAME(INDX2)
      AMF(NHAD+2)=AMASS(INDX2)
      ICHF(NHAD+2)=ICH(INDX2)
      IBARF(NHAD+2)=IBAR(INDX2)
      NREF(NHAD+2)=INDX2
      NHAD=NHAD+2
C
C
      IF(NHAD.NE.2) GO TO 440
C
 446  CONTINUE
      
      IREPIT=0
      KR2R=0
      KR2L=0
C
      IF(RNDM1(-1).LE.0.5) GO TO 447
      KFA1=IKF1
      IKF1=IKF2
      IKF2=KFA1
 447  CONTINUE
C
      IF(IKF1.GE.7)   GO TO 450
C
      IF(IKF1+6.NE.IKF3) GO TO 448
         KR1R=IKF2
         KR1L=IKF4
         NHAD=0
         GO TO 400
C
 448  CONTINUE
      IF(IKF1+6.NE.IKF4)  GO TO 449
         KR1R=IKF2
         KR1L=IKF3
         NHAD=0
         GO TO 400
C
 449  CONTINUE
      IREPIT=IREPIT+1
      IF(IREPIT.EQ.2) GO TO 451
      KFA1=IKF1
      IKF1=IKF2
      IKF2=KFA1
      GO TO 447
C
 450  CONTINUE
      KFA1=IKF1
      IKF1=IKF3
      IKF3=KFA1
C
      KFA1=IKF2
      IKF2=IKF4
      IKF4=KFA1
      GO TO 446
C
 451  CONTINUE
      NHAD=1
      INDEX=1
      ANF(NHAD)=ANAME(INDEX)
      AMF(NHAD)=1.E+5
      ICHF(NHAD)=ICH(INDEX)
      IBARF(NHAD)=IBAR(INDEX)
      NREF(NHAD)=INDEX
      HEF(NHAD)=E1+E2
      PXF(NHAD)=PX1+PX2
      PYF(NHAD)=PY1+PY2
      PZF(NHAD)=PZ1+PZ2
      RETURN
C
 440  CONTINUE
C
C
C   DETERMINATION OF THE ENERGY AND THE MOMENTUMS OF
C          THE LAST PRODUCED HADRONS
C
C
      RPG=SQRT(RPXG**2+RPYG**2+RPZG**2)
      IF(RPG.LT.REG) GO TO 459
C
C   IF FROM THE FINAL QUARKS SYSTEM COULD NOT BE PRODUCED
C          THE REAL HADRONS
C
 445  CONTINUE
      NHAD=0
      IJET=0
      E=E0
      SUMEN=SS
      PGX=0.
      PGY=0.
      PGZ=0.
      KFA1=IKF1
      KFA2=IKF2

      GO TO 60
C
C
 459  CONTINUE
      RMG=SQRT(REG**2-RPG**2)
      HM1=AMF(J)
      HM2=AMF(I)
      IF(RMG.GT.HM1+HM2) GO TO 460
C
C
C  ENERGY OF THE QUARKS SYSTEM IS NOT SUFFITIENT FOR
C          PRODUCTION OF LAST HADRONS
C
      GO TO 445
C
C
 460  CONTINUE
C
      GAA=REG/RMG
      GABE=RPG/RMG
      HE1=(RMG**2+HM1**2-HM2**2)/(2.*RMG)
      HE2=RMG-HE1
      USQRT=HE1**2-HM1**2
      IF(USQRT.GT.0.) THEN
         HP1=SQRT(USQRT)
      ELSE
         HP1=0.
      ENDIF
      HE=HE1
      HMA=HM1
C
C   DETERMINATION OF A HADRON MOMENTUM IN C.M.S
C
 470  CONTINUE
      ESMAX=HE-HMA
      PRMAX=EXP(-B3*ESMAX)
      OPRMAX=1.-PRMAX
      IF(ESMAX.GT.0.05) GO TO 471
      HPS=SQRT(HE**2-HMA**2)
      HPZ=0.0
      GO TO 472
C
 471  CONTINUE
      EPS =OPRMAX*RNDM1(-1)+PRMAX
      EPS1=OPRMAX*RNDM1(-1)+PRMAX
      PR=EPS*EPS1
      IF(PR.LE.PRMAX) GO TO 471
      ES=-1./B3*ALOG(PR)
      HPS=SQRT(ES**2+2.*ES*HMA)
      HPZ=SQRT(HE**2-HMA**2-HPS**2)
 472  CONTINUE
      CALL SFECFE(SFE,CFE)
      HPX=HPS*CFE
      HPY=HPS*SFE
C
C
      HP1X=HPX
      HP1Y=HPY
      HP1Z=HPZ
      HP2X=-HP1X
      HP2Y=-HP1Y
      HP2Z=-HP1Z
      IF(RPG.EQ.0.) GO TO 480
      HEX=HE1*GAA+HP1Z*GABE
      HEY=HE2*GAA+HP2Z*GABE
      HP1Z=HE1*GABE+HP1Z*GAA
      HP2Z=HE2*GABE+HP2Z*GAA
      HE1=HEX
      HE2=HEY
C
      COTE=RPZG/RPG
      SITE=SQRT(1.-COTE**2)
      IF(SITE.EQ.0.) GO TO 480
      COPS=-RPYG/(RPG*SITE)
      SIPS=RPXG/(RPG*SITE)
      GO TO 490
C
 480  CONTINUE
      COPS=1.
      SIPS=0.
      COTE=1.
      SITE=0.
C
 490  CONTINUE
C
      X1=COPS*HP1X-SIPS*COTE*HP1Y+SIPS*SITE*HP1Z
      X2=SIPS*HP1X+COPS*COTE*HP1Y-COPS*SITE*HP1Z
      X3=SITE*HP1Y+COTE*HP1Z
C
      HP1X=X1
      HP1Y=X2
      HP1Z=X3
C
C
      X1=COPS*HP2X-SIPS*COTE*HP2Y+SIPS*SITE*HP2Z
      X2=SIPS*HP2X+COPS*COTE*HP2Y-COPS*SITE*HP2Z
      X3=SITE*HP2Y+COTE*HP2Z
C
      HP2X=X1
      HP2Y=X2
      HP2Z=X3
C
C
 500  CONTINUE
      HEF(J)=HE1
      PXF(J)=HP1X
      PYF(J)=HP1Y
      PZF(J)=HP1Z
C
      HEF(I)=HE2
      PXF(I)=HP2X
      PYF(I)=HP2Y
      PZF(I)=HP2Z
C
C
C   BACKWARD LORENC AND EILER TRANSFORMATION
C
C
      SUMEN=0.
      SUMPZ=0.
      DO 510  I=1,NHAD
      SUMEN=SUMEN+HEF(I)
      SUMPZ=SUMPZ+PZF(I)
      PX=PXF(I)
      PY=PYF(I)
      PZ=PZF(I)
      PXX=C11*PX+C12*PY+C13*PZ
      PYY=C21*PX+C22*PY+C23*PZ
      PZZ=C31*PX+C32*PY+C33*PZ
C
      E=AMF(I)**2+PXX**2+PYY**2+PZZ**2
      E=SQRT(E)
      ETTAP=ETTAX*PXX+ETTAY*PYY+ETTAZ*PZZ
      BRAC=ETTAP/(GAM+1.)+E
      PXF(I)=PXX+ETTAX*BRAC
      PYF(I)=PYY+ETTAY*BRAC
      PZF(I)=PZZ+ETTAZ*BRAC
      HE=AMF(I)**2+PXF(I)**2+PYF(I)**2+PZF(I)**2
      HE=SQRT(HE)
      HEF(I)=HE
 510  CONTINUE
C     IF(ABS(SUMEN-SS).LE.0.01) GO TO 540
C     PRINT 515  ,SUMEN,SS
 515  FORMAT(' NOT ENERGY IN STRING ',2E11.4)
 540  CONTINUE
C     IF(ABS(SUMPZ).LE.0.01) GO TO 550
C     PRINT 525  ,SUMPZ
 525  FORMAT(' NOT MOMENTUM IN STRING ',E11.4)
 550  CONTINUE
      RETURN
      END

      FUNCTION BETA(X1,X2,BET)
      IF(X1.GT.X2) THEN
        AX=0
        IF(X1.LT.40.0) AX=-1./BET**2*(BET*X1+1.)*EXP(-BET*X1)
        AY=1./BET**2*(BET*X2+1.)*EXP(-BET*X2)
        BETA=AX+AY
      ELSE
        BETA=0.
      ENDIF
      RETURN
      END

      SUBROUTINE MESON(IDQ1,IDQ2,INDEX)
C
      COMMON/INPDAT/IMPS(6,6),IMVE(6,6),IB08(6,21),IB10(6,21),
     *IA08(6,21),IA10(6,21),A1,B1,B2,B3,ISU,BET,AS,B8,AME,DIQ
C
      IF(IDQ1.LT.7) GO TO 1
C
      LI=IDQ1-6
      LJ=IDQ2
      GO TO 2
C
 1    LI=IDQ2-6
      LJ=IDQ1
 2    CONTINUE
C
      EPS =RNDM1(-1)
      EPS1=RNDM1(-1)
C
      IF(EPS.LE.AS) GO TO 3
C
C   DETERMINATION OF AN IDENTIFICATOR OF A VECTOR MESON
C
      INDEX=IMVE(LI,LJ)
      IF((INDEX.EQ.33).AND.(EPS1.LE.0.5)) INDEX=35
      RETURN
C
C   DETERMINATION OF AN IDENTIFICATOR OF A PSEVDO SCALAR MESON
C
  3   INDEX=IMPS(LI,LJ)
      IF(INDEX.EQ.23) GO TO 4
      IF((INDEX.EQ.31).AND.(EPS1.LE.0.333)) INDEX=95
      RETURN
C
  4   CONTINUE
      IF(EPS1.LE.0.5)     INDEX=31                    
      IF(EPS1.LE.0.5*0.67) INDEX=95                   
       EPSa=RNDM1(-1)
       if(((INDEX.eq.31).or.(INDEX.eq.95)).and.(EPSa.le.0.5)) INDEX=122
      RETURN
      END

      SUBROUTINE BARYON(IDQ1,IDQ2,IDQ3,INDEX)
C
      COMMON/INPDAT/IMPS(6,6),IMVE(6,6),IB08(6,21),IB10(6,21),
     *IA08(6,21),IA10(6,21),A1,B1,B2,B3,ISU,BET,AS,B8,AME,DIQ
C
      IF(IDQ1.GE.7) GO TO 2
C
C   DETERMINATION OF AN IDENTIFICATOR OF A BARYON
C
      LI=IDQ1
      CALL INDEX2(IDQ2,IDQ3,IND)
      LJ=IND
C
      EPS =RNDM1(-1)
      EPS1=RNDM1(-1)
C
      IF(EPS .LE.B8)  GO TO 1
C
C   A BARYON HAS SPIN 3/2
C
      INDEX=IB10(LI,LJ)
      RETURN
C
 1    CONTINUE
C
C   A BARYON HAS SPIN 1/2
C
      INDEX=IB08(LI,LJ)
      IF(INDEX.EQ. 22.AND.EPS1.LE.0.5) INDEX=17
      IF(INDEX.EQ.137.AND.EPS1.LE.0.5) INDEX=141
      IF(INDEX.EQ.138.AND.EPS1.LE.0.5) INDEX=143
      IF(INDEX.EQ.139.AND.EPS1.LE.0.5) INDEX=144
      IF(INDEX.NE.0)  RETURN
      INDEX=IB10(LI,LJ)
      RETURN
C
C
 2    CONTINUE
C
C   DETERMINATION OF AN IDENTIFICATOR OF AN ANTI-BARYON
C
      LI=IDQ1-6
      CALL INDEX2(IDQ2-6,IDQ3-6,IND)
      LJ=IND
C
      EPS =RNDM1(-1)
      EPS1=RNDM1(-1)
C
      IF(EPS.LE.B8)  GO TO 3
C
C   AN ANTI-BARION HAS SPIN 3/2
C
      INDEX=IA10(LI,LJ)
      RETURN
C
 3    CONTINUE
C
C   AN ANTI-BARYON HAS SPIN 1/2
C
      INDEX=IA08(LI,LJ)
      IF(INDEX.EQ.100.AND.EPS1.LE.0.5)  INDEX=18
      IF(INDEX.EQ.149.AND.EPS1.LE.0.5)  INDEX=153
      IF(INDEX.EQ.150.AND.EPS1.LE.0.5)  INDEX=155
      IF(INDEX.EQ.151.AND.EPS1.LE.0.5)  INDEX=156
      IF(INDEX.NE.0) RETURN
      INDEX=IA10(LI,LJ)
      RETURN
      END

      SUBROUTINE INDEX2(KA,KB,IND)
      KP=KA*KB
      KS=KA+KB
      IF(KP.EQ.1) IND=1
      IF(KP.EQ.2) IND=2
      IF(KP.EQ.3) IND=3
      IF(KP.EQ.4.AND.KS.EQ.5) IND=4
      IF(KP.EQ.5) IND=5
      IF(KP.EQ.6.AND.KS.EQ.7) IND=6
      IF(KP.EQ.4.AND.KS.EQ.4) IND=7
      IF(KP.EQ.6.AND.KS.EQ.5) IND=8
      IF(KP.EQ.8) IND=9
      IF(KP.EQ.10) IND=10
      IF(KP.EQ.12.AND.KS.EQ.8) IND=11
      IF(KP.EQ.9) IND=12
      IF(KP.EQ.12.AND.KS.EQ.7) IND=13
      IF(KP.EQ.15) IND=14
      IF(KP.EQ.18) IND=15
      IF(KP.EQ.16) IND=16
      IF(KP.EQ.20) IND=17
      IF(KP.EQ.24) IND=18
      IF(KP.EQ.25) IND=19
      IF(KP.EQ.30) IND=20
      IF(KP.EQ.36) IND=21
      RETURN
      END
