C------------ Common subroutines --------------------------------------
      FUNCTION AMASS(IND)
C
      COMMON/PART/ANAME(180),AM(180),GA(180),TAU(180),ICH(180),IBAR(180)
     ,           ,K1(180),K2(180)
C
      COMMON/LIMMAS/IDSTAB(180),SUMKM(533),AML(180),FI0ML(180)

      CHARACTER*8 ANAME
C
      DATA PIHALF /1.57074/
      SAVE PIHALF
C
      IF(AM(IND).NE.AML(IND)) GO TO 1
      AMASS=AM(IND)
      RETURN
C
 1    CONTINUE
      EPS=RNDM1(-1)
      FI=PIHALF*EPS+(1.-EPS)*FI0ML(IND)
      SINFI=SIN(FI)
      COSFI=COS(FI)
      IF(COSFI.GE.1.0E-5) GO TO 2
      TGFI=10.
      GO TO 3
 2    CONTINUE
      TGFI=SINFI/COSFI
 3    CONTINUE
      AM2=AM(IND)**2+AM(IND)*GA(IND)*TGFI
      AMASS=SQRT(AM2)
      RETURN
      END

      SUBROUTINE SFECFE(SFE,CFE)
      INTEGER V
      V=-1
    1 X=RNDM1(V)
      Y=RNDM1(V)
      XX=X*X
      YY=Y*Y
      XY=XX+YY
      IF(XY.GT.1) GO TO 1
      CFE=(XX-YY)/XY
      SFE=2.*X*Y/XY
      IF(RNDM1(V).LT.0.5) GO TO 2
      RETURN
    2 SFE=-SFE
      RETURN
      END

C---------------------------------------------------------------------
      SUBROUTINE GOBSEC(NPS,NHAD)
C---------------------------------------------------------------------
      COMMON/FINPAR/PXF(10000),PYF(10000),PZF(10000),HEF(10000),
     *AMF(10000),ICHF(10000),IBARF(10000),ANF(10000),NREF(10000)

      COMMON /MIDPAR/PXM(1000),PYM(1000),PZM(1000),HEM(1000),AMM(1000),
     *ICHM(1000),IBARM(1000),ANM(1000),NREM(1000)

C
      CHARACTER*8 ANF, ANM
*      write(6,*)' GOBSEC ----',NPS,NHAD
      IF(NHAD.EQ.0) GO TO 20
C
      DO 10  I=1,NHAD
      NPS=NPS+1
      PXF(NPS)=PXM(I)
      PYF(NPS)=PYM(I)
      PZF(NPS)=PZM(I)
      HEF(NPS)=HEM(I)
      AMF(NPS)=AMM(I)
      ICHF(NPS)=ICHM(I)
*      write(6,*)i,ICHM(i)
      IBARF(NPS)=IBARM(I)
      ANF(NPS)=ANM(I)
      NREF(NPS)=NREM(I)
C
 10   CONTINUE
 20   CONTINUE
*      write(6,*)' END GOBSEC------'
      RETURN
      END

