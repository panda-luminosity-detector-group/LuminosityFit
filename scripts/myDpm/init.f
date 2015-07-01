
C-----------------A. Galoyan last edition 2 March 2012 -----
       SUBROUTINE INIT1(Plab, seed, Elastic, tetmin)
C-----------------------------------------------------------------------
       COMMON/UZHI/SqrtS,Ecms,Vcms,Gamma,Proc_Prob(7),P_5str,CS_in,
     ,            CS_el,A1,T1,A2,T2,A3,Tmax,Tmin,Weight1,AMProton
C-----------------------------------------------------------------------
C             PARAMETERS OF QUARK-GLUON STRING MODEL
C-----------------------------------------------------------------------
      COMMON/QGMPAR/ IFL(12), ALPHAR, ALPHAN, ALPHAF, ALPHA(4),
     *               PUDSC(4), BSLOP, VQMASS, DIQMAS
C =========================================================================
C-----------------------------------------------------------------------
C             PARAMETERS OF PARTICLE DECAYS
C-----------------------------------------------------------------------
      COMMON/LIMMAS/IDSTAB(180), SUMKM(533), AML(180), FI0ML(180)
C
      COMMON/PRINT/ ISYS   /IDGB/ IDGB, IDG
      COMMON/AB/aelm,betav, sigma_tot, parB, rho      !aida
      common/prob_el/prob_col, prob_int, prob_had          !aida

C
C-----------------------------------------------------------------------
      double precision seed
      
C-----------------------------------------------------------------------
c      print*, plab, seed, Elastic, tetmin

      AMProton=0.938272
      Elab=sqrt(AMProton**2+Plab**2)
      Etot=Elab+AMProton
      S=2.0*AMProton**2+2.0*AMProton*Elab
      SS=sqrt(S)
      SqrtS=SS
      Ecms=SS/2.
      Pcms=sqrt(Ecms**2-AMProton**2)
      Vcms=Plab/(Elab+AMProton)
      Gamma=(Elab+AMProton)/SqrtS

*      write(6,*)'" ',Plab,SqrtS,'Plab,SqrtS'
*      write(6,*)'" ',Ecms,Pcms ,'Ecms,Pcms '
c-----------------------------------------------------------------------

      call RNDMSET1(seed)

C=========================================================
C FOR U, D, S, C, B, T QUARKS AND FOR ANTI-U, ANTI-D,
C  ANTI-S, ANTI-C, ANTI-B, ANTI-T QUARKS WE USE
C    THE FOLLOWING IDENTIFICATORS
C
      IFL(1)=1     ! u-quark
      IFL(2)=2     ! d-quark
      IFL(3)=3     ! s-quark
      IFL(4)=4     ! c-quark
      IFL(5)=5     ! b-quark
      IFL(6)=6     ! t-quark
      IFL(7)=7     ! anti_u-quark
      IFL(8)=8     ! anti_d-quark
      IFL(9)=9     ! anti_s-quark
      IFL(10)=10   ! anti_c-quark
      IFL(11)=11   ! anti_b-quark
      IFL(12)=12   ! anti_t-quark
C==========================================================
C       PARAMETERS OF QUARK DISTRIBUTIONS
C----------------------------------------------------------
C
      ALPHAR=+0.5  
      ALPHAN=-0.5
      ALPHAF=0.
C
      ALPHA(1)=ALPHAR
      ALPHA(2)=ALPHAR
      ALPHA(3)=0.
      ALPHA(4)=0.
C
C================================================================
C WE PUT THE CREATION PROBABILITIES OF U - ANTI-U, D - ANTI-D,
C   S - ANTI-S, C - ANTI-C SYSTEMS AS 0.43, 0.43, 0.14, 0. , SO
C
      PUDSC(2)=1.-0.18*(1.-3./SQRT(SqrtS))
      PUDSC(1)=PUDSC(2)/2.
      PUDSC(3)=1.
      PUDSC(4)=1.
C
C===============================================================
C   WE DESCRIBE THE QUARK TRANSVERSAL MOMENTUM DISTRIBUTION BY
C      BSLOP*EXP(-BSLOP*PT), BSLOP IN (GEV/C)**(-1)
C
                     BSLOP=4.5           ! now pbar p
C
C===============================================================
C        WE PUT THE FOLLOWING VALUES OF VALENCE QUARKS
C                 AND DIQUARKS MASSES
C -------------------------------------------------------------
      VQMASS=0.   ! 0.350 pbar p
      DIQMAS=0.   ! 0.700 pbar p
C
C
C==============================================================
C         WE MUST PREPARE TO STRINGS FRAGMENTATION
C              AND PARTICLES DECAYS,
C       SO WE PUT THE FOLLOWING PARTICLES ARE STABLE
C              DEPENDING ON ISTAB

      ISTAB=2   ! to put all particle stable write ISTAB=3 ! Uzhi 09.05

      DO 230  I=1,180
 230  IDSTAB(I)=0
C
C   FOR ISTAB=1
C
      DO 240  I=1,7
 240  IDSTAB(I)=1
C
      IDSTAB(26)=1
      IDSTAB(29)=1
      IDSTAB(30)=1
C
      DO 250  I=133,136
 250  IDSTAB(I)=1
C
      IF(ISTAB.EQ.1)  GO TO 280
C
C   FOR ISTAB=2
C
      DO 260  I=8,16
 260  IDSTAB(I)=1
C
      IF(ISTAB.EQ.2) GO TO 280
C
C   FOR ISTAB=3
C
      DO 270  I=17,180
 270  IDSTAB(I)=1
C
 280  CONTINUE
C
C-----------------------------------------------------------------
C    IF IT IS NEEDED TO POINT OUT THE OTHER STABLE
C    PARTICLES, PUT CORRESPONDING ISTAB = 1
C-----------------------------------------------------------------
       IDSTAB(17)=1  ! \Lambda
       IDSTAB(18)=1  ! Anti_Lambda
       IDSTAB(19)=1  ! K^0_s
       IDSTAB(23)=1  ! \pi^0
C      IDSTAB(33)=1  ! \rho_0
ccc aida
c       IDSTAB(31)=1  ! eta
c       IDSTAB(35)=1  ! omega
c        IDSTAB(95)=1  !eta'  
       CALL DATAR3
C
C==============================================================
C         FOR SIMULATION OF PARTICLE DECAYS WE PUT
C
      ISYS=6
      IDGB=-1
C-------------------------------------------------------------
c   PDG parametrization of total and elastic Pbar+P X-sections
c                 Phys. Rev. D54 (1996) 125.
C-------------------------------------------------------------
      ALOGp=Alog(Plab)
!      Xtotal=38.4+77.6*Plab**(-0.64)+0.260*ALOGp**2-1.20*ALOGp
!      Xelast=10.2+52.7*Plab**(-1.16)+0.125*ALOGp**2-1.28*ALOGp
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!    new parametrization for Xtotal and Xelast 27 Febr. 2012
!     our parameters                 !aida
       AMn=0.93827231
       b0=11.92
       b2=0.3036
       SqrtS0=20.74
       S0=33.0625
       
       B=b0 + b2*(ALOG(SqrtS/SqrtS0))**2
       SigAss=36.04+0.304*(alog(S/S0))**2
       R0=sqrt(0.40874044*SigAss - B)

        C=13.55
        d1=-4.47
        d2=12.38
        d3=-12.43
        Xtotal = SigAss*(1.+1./sqrt(S-4.*AMn**2)/R0**3 *
     *  C*(1.+D1/SqrtS+D2/SqrtS**2 + D3/SqrtS**3))

        C=59.27
        d1=-6.95
        d2=23.54
        d3=-25.34
        SigAss = 4.5 +0.101*(alog(S/S0))**2
        Xelast =  SigAss*(1.+1./sqrt(S-4.*AMn**2)/R0**3 *
     *  C*(1.+D1/SqrtS+D2/SqrtS**2 + D3/SqrtS**3))

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        Welast=Xelast/Xtotal

      if(Elastic.eq.0.) Welast=0.  ! Inelastic interactions only
      if(Elastic.eq.2.) Welast=1.  ! Elastic   interactions only

C-------------------------------------------------------------
c      Parameters of differencial elastic X-section                    
C-------------------------------------------------------------
      CS_el=Xelast

      A1=115.0+650.0*exp(-Plab/4.08) 
      T1=0.0899
      A2=0.0687+0.307*exp(-Plab/2.367)
      T2=-2.979+3.353*exp(-Plab/483.4)
      A3=0.8372+39.53*exp(-Plab/0.765)

      If(Elastic .eq. 0.) then
      Tmin=0.
      Tmax=-4.*Pcms**2
      Weight1=A3*T2*(1.-exp(Tmax/T2))/(
     (A1*T1*(1.-exp(Tmax/T1))+A1*A2**2*T2*(1.-exp(Tmax/T2))-
     -2.*A1*A2*2.*T1*T2/(T1+T2)*(1.-exp(Tmax*(T1+T2)/2./T1/T2))+
     +A3*T2*(1.-exp(Tmax/T2))      )
      endif

      If(Elastic .gt. 0.) then
 
      Tetmin=tetmin*3.1416/180.
      Tantet2=(sin(Tetmin)/cos(Tetmin))**2       !9.06.09
      Plmin=2*0.938*Plab/(2*0.938+Etot*Tantet2)  !9.06.09      
      Ptmin=Plmin*sin(Tetmin)/cos(Tetmin)
      
      sqmin=Plmin**2+Ptmin**2+0.938**2
      Tmin=2*(Plmin*Plab+0.938**2-Elab*sqrt(sqmin))

      Tmax=-4.*Pcms**2
      tsito=2.*(plab**2)*(1.-cos(tetmin))     !T by Tsito formul
      bbb=(Plab*sin(Tetmin))**2               !T by approx formul
caida  bbbmax=(Plab*sin(0.0082))**2              !11.06.09 dlya lumi

      Tmin=-bbb
caida  Tmax=-bbbmax


       Weight1=A3*T2*(exp(Tmin/T2)-exp(Tmax/T2))/(
     & A1*T1*(exp(Tmin/T1)-exp(Tmax/T1))+
     & A1*A2**2*T2*(exp(Tmin/T2)-exp(Tmax/T2))-
     & 2.*A1*A2*2.*T1*T2/(T1+T2)*(exp(Tmin*(T1+T2)/2./T1/T2)
     &  -exp(Tmax*(T1+T2)/2./T1/T2))+
     &  A3*T2*(exp(Tmin/T2) - exp(Tmax/T2)) )

*   aida begin parameters
       betav=Plab/Elab
       rho=-sqrt(A3)/(sqrt(A1)*(1-A2))
       parB=(A1/T1+A1*(A2**2)/T2-2*A1*A2*(1./2./T1+1./2./T2)+
     & A3/T2)/(A1*(1-A2)**2+A3)
       
       sigma_tot=4*sqrt(3.1416*A1*0.1/5.067**2)*(1-A2)*10
caida       print *, 'sigma_tot',sigma_tot,' B',parB,' ro',rho;
       aelm = 1./137.036    !  0- not Colomb and Interf, 
*  aida end parameters    
***  aida calculation of integrals Colomb, 
c     ok lets initialize sample points equi-distancally
      Ndiv=10000000
      dto=(Tmax-Tmin)/float(Ndiv)
      SIG_col=0.
      do i=1,Ndiv
c     transfer sample points into a 1/t frame (so finer sampling at low t)
c      T11=Tmin+(i-1)*dt
c      T22=T11+dt
      T22=Tmin/(1.0 - float(i-1)/float(Ndiv)*(1.0-Tmin/Tmax));
      T11=Tmin/(1.0 - float(i)/float(Ndiv)*(1.0-Tmin/Tmax));
c     in this sub intervall T11-T22 use simpsons rule to estimate integral   
      dt=(T22-T11)/2.0
c      df_col=DSIG_COL(T22)+DSIG_COL(T11)
      SIG_COL=SIG_COL+dt*(1./3.*DSIG_COL(T11)+
     & 4./3.*DSIG_COL(T11+dt)+1./3.*DSIG_COL(T22))
c      print *, T11, T22, df_col, dt, 0.5*df_col*abs(dt)
c      SIG_COL=sig_col+(0.5*df_col*abs(dt))
      enddo
      PRINT *,'sig_col',SIG_COL, Tmin, Tmax

* aida calculation of integral interfer
      SIG_INTER=0.
      SIG_IEXACT=0.
      do i=1,Ndiv
      T11=Tmin+(i-1)*dto
      T22=T11+dto
c      df_int=DSIG_INTER(T22)+DSIG_INTER(T11)
c      SIG_INTER=SIG_INTER+(0.5*df_int*abs(dt))
c      df_iex=DSIG_INT_Ex(T22)+DSIG_INT_Ex(T11)
c      SIG_IEXACT=SIG_IEXACT+(0.5* df_iex*abs(dt))
c      T11=Tmin/(1.0 + float(i-1)/float(Ndiv)*(1.0-Tmin/Tmax));
c      T22=Tmin/(1.0 + float(i)/float(Ndiv)*(1.0-Tmin/Tmax));
      dt=(T22-T11)/2.0
      SIG_INTER=SIG_INTER+abs(dt)*(1./3.*DSIG_INTER(T11)+
     & 4./3.*DSIG_INTER(T11+dt)+1./3.*DSIG_INTER(T22))
      SIG_IEXACT=SIG_IEXACT+abs(dt)*(1./3.*DSIG_INT_Ex(T11)+
     & 4./3.*DSIG_INT_Ex(T11+dt)+1./3.*DSIG_INT_Ex(T22))
      enddo
      PRINT *,'sig_int', sig_inter, sig_iexact

!  numerical calculation of SIG_had using form.(1)
      sig_had=dsig_had(0.)/parB-dsig_had(Tmax)/parB
      PRINT *,'sig_had_el', sig_had
!     calculation of sigma_hadron using our parametrization
      sig_had_p=SIG_HADi(Tmin)-SIG_HADi(Tmax)
      PRINT *,'sig_had_p',sig_had_p
    
!       sig_col=0              ! kulon ==0
!       sig_inter=0            ! inter ==0 
!       sig_had_p=0            ! hadron ==0  

      SIG_MAG=SIG_COL+SIG_INTER+SIG_HAD_p      

      SIG_NORM=SIG_COL+SIG_IEXACT+SIG_HAD_p      

caida      print*, 'Xtotal', Xtotal, ' Xelast, hadronic part', Xelast
caida      PRINT *,'sigma_tot', sigma_tot, 'sig_mag', SIG_MAG
caida      PRINT *,'sig_iexact ',sig_iexact,'SIGMA_norm ',sig_norm

!calculation of probability hadron, colomb, interf- elastic
      prob_col=1./sig_mag * sig_col   
c      prob_col=1.   
      prob_int=1./sig_mag * sig_inter
      prob_had=1./sig_mag * sig_had_p         !our parametr
c      print *, prob_col, prob_inter, prob_had
      print *, "elastic probabilities", prob_col, prob_int, prob_had
      print *, "elastic cross section", sig_norm, '(mb)'
      endif                                    ! aida end of elastic
     
c ---------------------------- Determination of processes prababilities
      CS_a=0.4*( 129./sqrt(S)-147./S+41./S**1.5)   
      CS_b=0.6*( 129./sqrt(S)-147./S+41./S**1.5)   
      CS_c=93./S-106./S**1.5+30./S**2
       CS_e=0.
      CS_pom=18.6*S**0.08-33.5/sqrt(S)+30.8/S
      CS_lmd=0.

      CS_in=CS_a + CS_b + CS_c + CS_e + CS_pom + CS_lmd

      print*,'--------------- Plab= ',Plab,'-----------------------'
      print*,' Cross-sections     a      b      c      e      Pom  '
      print*,                   CS_a, CS_b, CS_c,   CS_e,  CS_pom
      print*,' ----------------------------------------------------'
      print*,' Inelastic cross-section = ',CS_in,' (mb)'
      print*,' ----------------------------------------------------'

*      write(6,*)' ----------------------------------------------------'
*      write(6,*)'--------------- Plab= ',Plab,'-----------------------'
*      write(6,*)' Cross-sections     a      b      c      e      Pom  '
*      write(6,1)                    CS_a, CS_b, CS_c,   CS_e,  CS_pom
*1     format(    17x,              f6.2,1x,f6.2,1x,f6.2,1x,f6.2,1x,f6.2)
*      write(6,*)' ----------------------------------------------------'
*      write(6,*)' Inelastic cross-section = ',CS_in,' (mb)'
*      write(6,*)' ----------------------------------------------------'
*      write(6,*)' Coefficients       a      b      c      e           '
*      write(6,1)                    C_a,   C_b,   C_c,   C_e
*      write(6,*)' ----------------------------------------------------'

!     modification of XS ratio  6.02.2012
      Welast = SIG_NORM/(SIG_NORM+Xtotal-Xelast)
      print*, "Welast", Welast
      if(Elastic.lt.0.5) Welast=0.0  ! Inelastic interactions only
      if(Elastic.gt.1.5) Welast=1.0  ! Elastic   interactions only

      print *, CS_a, CS_in, Welast
      Proc_Prob(1)=CS_a/CS_in               *(1.0-Welast)
      Proc_Prob(2)=Proc_Prob(1)+CS_b/CS_in  *(1.0-Welast)
      Proc_Prob(3)=Proc_Prob(2)+CS_c/CS_in  *(1.0-Welast)
      Proc_Prob(4)=Proc_Prob(3)+CS_e/CS_in  *(1.0-Welast)
      Proc_Prob(5)=Proc_Prob(4)+CS_pom/CS_in*(1.0-Welast)
      Proc_Prob(6)=Proc_Prob(5)+CS_lmd/CS_in*(1.0-Welast)
      Proc_Prob(7)=Proc_Prob(6)+Welast


      print*,'--------------- Probabilities -----------------------'
      print*,Proc_Prob(1),Proc_Prob(2),Proc_Prob(3),Proc_Prob(4)
      print*,Proc_Prob(5),Proc_Prob(6),Proc_Prob(7)
      print*,' ----------------------------------------------------'


      P_5str=0.    ! 0.3      Uzhi

*	write(6,*)Proc_Prob

      RETURN
      END

      SUBROUTINE DATAR3
      DIMENSION IV(36),IP(36),IB(126),IBB(126),IA(126),IAA(126)

      COMMON/INPDAT/IMPS(6,6),IMVE(6,6),IB08(6,21),IB10(6,21),
     *IA08(6,21),IA10(6,21),A1,B1,B2,B3,ISU,BET,AS,B8,AME,DIQ
C
      COMMON/PART/ANAME(180),AM(180),GA(180),TAU(180),ICH(180),IBAR(180)
     ,           ,K1(180),K2(180)
      COMMON/DECAYC/ZKNAME(533),NZK(533,3),WT(533)
C
      COMMON/LIMMAS/IDSTAB(180),SUMKM(533),AML(180),FI0ML(180)
      CHARACTER*8 ANAME
      CHARACTER*8 ZKNAME

      SAVE IP, IV, IB, IBB, IA, IAA
C
      DATA IP/
     *23,14,16,116,2*0,13,23,25,117,2*0,15,24,31,120,2*0,119,118,121,
     *122,14*0/

      DATA IV/
     *33,34,38,123,0,0,32,33,39,124,0,0,36,37,96,127,0,0,126,125,128,
     *129,14*0/

      DATA IB/
     *0,1,21,140,0,0,8,22,137,0,0,97,138,0,0,146,5*0,
     *1,8,22,137,0,0,0,20,142,0,0,98,139,0,0,147,5*0,
     *21,22,97,138,0,0,20,98,139,0,0,0,145,0,0,148,5*0,
     *140,137,138,146,0,0,142,139,147,0,0,145,148,50*0/

      DATA IBB/
     *53,54,104,161,0,0,55,105,162,0,0,107,164,0,0,167,5*0,
     *54,55,105,162,0,0,56,106,163,0,0,108,165,0,0,168,5*0,
     *104,105,107,164,0,0,106,108,165,0,0,109,166,0,0,169,5*0,
     *161,162,164,167,0,0,163,165,168,0,0,166,169,0, 0,170,47*0/

      DATA IA/
     *0,2,99,152,0,0,9,100,149,0,0,102,150,0,0,158,5*0,
     *2,9,100,149,0,0,0,101,154,0,0,103,151,0,0,159,5*0,
     *99,100,102,150,0,0,101,103,151,0,0,0,157,0,0,160,5*0,
     *152,149,150,158,0,0,154,151,159,0,0,157,160,50*0/

      DATA IAA/
     *67,68,110,171,0,0,69,111,172,0,0,113,174,0,0,177,5*0,
     *68,69,111,172,0,0,70,112,173,0,0,114,175,0,0,178,5*0,
     *110,111,113,174,0,0,112,114,175,0,0,115,176,0,0,179,5*0,
     *171,172,174,177,0,0,173,175,178,0,0,176,179,0,0,180,47*0/

      AM(122) = 1.275                          ! added f2 meson 
      GA(122) = 0.185                          ! added f2 meson 

      L=0
      DO 1 I=1,6
      DO 2  J=1,6
      L=L+1
      IMPS(I,J)=IP(L)
    2 CONTINUE
    1 CONTINUE

      L=0
      DO 3 I=1,6
      DO 4 J=1,6
      L=L+1
      IMVE(I,J)=IV(L)
    4 CONTINUE
    3 CONTINUE

      L=0
      DO 5 I=1,6
      DO 6 J=1,21
      L=L+1
      IB08(I,J)=IB(L)
    6 CONTINUE
    5 CONTINUE

      L=0
      DO 7 I=1,6
      DO 8 J=1,21
      L=L+1
      IB10(I,J)=IBB(L)
    8 CONTINUE
    7 CONTINUE

      L=0
      DO 9 I=1,6
      DO 10 J=1,21
      L=L+1
      IA08(I,J)=IA(L)
   10 CONTINUE
    9 CONTINUE

      L=0
      DO 11 I=1,6
      DO 12 J=1,21
      L=L+1
      IA10(I,J)=IAA(L)
   12 CONTINUE
   11 CONTINUE

      A1=0.88
      B3=4.5
      B1=8.0
      B2=8.0
      ISU=4
      BET=8.0
      AS=0.65    ! Uzhi 0 - only heavy resonances, 1. - light ones
      AME=0.75
      B8=0.5
      DIQ=0.375
C
      DO 13  I=1,180
      AML(I)=-1.
      IF(GA(I).EQ.0.) AML(I)=AM(I)
 13   CONTINUE
C
      DO 20  I=1,533
 20   SUMKM(I)=-1.
C
C
 80   CONTINUE
C
      DO 110  I=1,180
C
C     IF(AML(I).GE.0.) GO TO 110
C
      IK1=K1(I)
      IK2=K2(I)
C
      DO 90  J=IK1, IK2
      ID1=NZK(J,1)
      IF(ID1.EQ.0) ID1=29
      ID2=NZK(J,2)
      IF(ID2.EQ.0) ID2=29
      ID3=NZK(J,3)
      IF(ID3.EQ.0) ID3=29
      IF(AML(ID1).LT.0.) GO TO 105
      IF(AML(ID2).LT.0.) GO TO 105
      IF(AML(ID3).LT.0.) GO TO 105
      SUMKM(J)=AML(ID1)+AML(ID2)+AML(ID3)
 90   CONTINUE
C
      AMIN=100.
      DO 100  J=IK1,IK2
      IF(WT(J).EQ.0.) GO TO 100
      IF(SUMKM(J).LE.AMIN) AMIN=SUMKM(J)
 100  CONTINUE
C
      IF(AML(I).LT.0.) AML(I)=AMIN
C
 105  CONTINUE
 110  CONTINUE
C
      RETFL=1.
      DO 120  I=1,180
      IF(AML(I).LT.0.) RETFL=-1.
 120  CONTINUE
C
      IF(RETFL.LT.0.) GO TO 80
C
      DO 140  I=1,180
      IF(GA(I).NE.0.) GO TO 130
      FI0ML(I)=0.
      GO TO 140
 130  CONTINUE
      TGFI0=AML(I)**2/AM(I)/GA(I)-AM(I)/GA(I)
      FI0ML(I)=ATAN(TGFI0)
 140  CONTINUE
C
      RETURN
      END

      BLOCK DATA
C*****BLOCK DATA
      COMMON/PART/ANAME(115),ANAM1(65),AM(180),GA(180),TAU(180),ICH(180)
     *,IBAR(180),K1(180),K2(180)

C***BLOCK DATA 2
      COMMON/DECAYC/ZKNAM1(87),ZKNAM2(59),ZKNAM3(105),ZKNAM4(79),
     *ZKNAM5(103),ZKNAM6(100),NZK01(195),NZK1(183),NZK11(155),
     *NZK02(195),NZK2(183),
     *NZK22(155),NZK03(195),NZK3(183),NZK33(155),
     *WT(135),WT1(131),WT2(64),WT3(103),WT4(100)

      COMMON/DQSHR/XDQ,XDQ1 /FLPRO/PUDSC(4) /FLID/IFL(12)
      COMMON/REGPAR/ALPHAR,ALPHAN,ALPHAF,ALPHA(4)
      COMMON/FRAG1/DQFRAG,RM,RDI,BSLOP,PTMIN /IDGB/IDGB,IDG
      COMMON/PRINT/ISYS

      character*8 ANAME
      character*8 ANAM1
      character*8 ZKNAM1,ZKNAM2,ZKNAM3,ZKNAM4,ZKNAM5,ZKNAM6
C
C     PARTICLE NAMES
C
      DATA ANAME/'P','AP','E-','E+','NUE','ANUE','GAM','NEU','ANEU',
     *'MUE+','MUE-','K0L','PI+','PI-','K+','K-','LAM','ALAM','K0S',
     *'SIGM-','SIGM+','SIGM0','PI0','K0','AK0','     ','AN*-14',
     *'AN*014',2*'    ',
     *'ETA550','RHO+77','RHO077','RHO-77','OM0783','K*+892','K*0892',
     *'K*-892','AK*089','KA+125','KA0125','KA-125','AKA012','K*+142',
     *'K*0142','K*-142','AK*014','S+1385','S01385','S-1385','L01820',
     *'L02030',
     *'N*++12','N*+ 12','N*012 ','N*-12 ','N*++16','N*+16 ','N*016 ',
     *'N*-16 ','N*+14 ','N*014 ','N*+15 ','N*015 ','N*+18 ','N*018 ',
     *'AN--12','AN*-12','AN*012','AN*+12','AN--16','AN*-16','AN*016',
     *'AN*+16','AN*-15','AN*015','DE*-24',
     *'RPI+49','RPI049','RPI-49','PIN++ ','PIN+0 ','PIN+- ','PIN-0 ',
     *'PPPI','PNPI','APPPI','APNPI','K+PPI','K-PPI','K+NPI','K-NPI',
     *'S+1820','S-2030',
     *'ETA*  ','PHI   ','TETA0 ','TETA- ','ASIG- ','ASIG0 ','ASIG+ ',
     *'ATETA0','ATETA+','SIG*+ ','SIG*0 ','SIG*- ','TETA*0','TETA* ',
     *'OMEGA-','ASIG*-','ASIG*0','ASIG*+','ATET*0','ATET*+','OMEGA+'/
      DATA ANAM1/'D0','D+','D-','AD0','F+','F-','ETAC','D*0','D*+',
     *'D*-','AD*0','F*+','F*-','PSI','JPSI','TAU+','TAU-','NUET',
     *'ANUET','NUEM','ANUEM',
     *'C0+   ','A+    ','A0    ','C1++  ','C1+   ','C10   ','S+    ',
     *'S0    ','T0    ','XU++  ','XD+   ','XS+   ','AC0-  ','AA-   ',
     *'AA0   ','AC1-- ','AC1-  ','AC10  ','AS-   ','AS0   ','AT0   ',
     *'AXU-- ','AXD-  ','AXS   ','C1*++ ','C1*+  ','C1*0  ','S*+   ',
     *'S*0   ','T*0   ','XU*++ ','XD*+  ','XS*+  ','TETA++','AC1*--',
     *'AC1*- ','AC1*0 ','AS*-  ','AS*0  ','AT*0  ','AXU*--','AXD*- ',
     *'AXS*- ','ATET--'/
C
C    PARTICLE MASSES IN GEV
C
      DATA AM/2*0.938272,2*0.0005,3*0,2*0.94,2*0.106,0.498,2*0.140,
     *2*0.494,2*1.116,0.498,1.197,1.189,1.193,0.134,2*0.498,
     *0.,2*1.43,
     *2*0,0.548,3*0.776,0.783,4*0.896,4*1.25,4*1.421,3*1.382,1.82,2.03, ! Uzhi
     *4*1.232,4*1.675,2*1.43,2*1.515,2*1.775,4*1.232,4*1.675,2*1.515,
     *2.4,3*0.489,4*1.3,4*2.2,4*1.7,1.82,2.03,0.958,1.019,1.315,1.321,
     *1.189,1.193,1.197,1.315,1.321,3*1.385,2*1.534,1.672,3*1.385,
     *2*1.534,1.672,4*1.867,3*2.03,2.006,2*2.008,2.006,2*2.14,3.684,
     *3.097,2*1.807,2*0.6,2*0.003,2.285,2.47,2.47,2.41,2.42,2.41,2.56,
     *2.56,2.73,3.61,3.61,3.79,2.26,2.47,2.47,2.41,2.42,2.41,2.56,2.56,
     *2.73,3.61,3.61,3.79,3*2.49,2*2.61,2.77,2*3.67,3.85,4.89,3*2.49,
     *2*2.61,2.77,2*3.67,3.85,4.89/
C
C       RESONANCE WIDTH GAMMA IN GEV
C
      DATA GA/26*0,2*0.2,2*0.,
     *1.29E-6,3*0.150,0.01,4*0.051,4*0.45,4*0.108,3*0.05,             ! Uzhi
     *0.085,0.18,4*0.120,4*0.2,2*0.2,2*0.1,2*0.2,4*0.120,4*0.2,2*0.1,
     *0.2,3*0.1,4*0.1,4*0.2,4*0.15,0.085,0.18,2.E-4,0.004,7*0,0.036,
     *0.036,0.039,0.009,0.009,0,0.036,0.036,0.039,0.009,0.009,0,7*0,
     *0.005,0.002,0.002,0.005,0.002,0.002,0.0002,0.0007,6*0,44*0/
C
C       MEAN LIFE TIME IN SECONDS
C
      DATA TAU/7*1.E+18,2*918.,2*2.2E-06,5.2E-08,2*2.6E-08,2*1.2E-08,
     *2*2.6E-10,0.9E-10,1.5E-10,8.E-11,7.4E-20,8.E-17,
     *61*0,10*0,2*0,3.E-10,1.7E-10,0.8E-10,1.E-14,1.5E-10,3.E-10,
     *1.7E-10,5*0,1.E-10,5*0,1.E-10,15*0,4*9.E-12,2*1.E+18,44*0/
C
C      CHARGE OF PARTICLES AND RESONANCES
C
      DATA ICH/+1,-1,-1,
     *+1,
     *5*0,1,-1,0,1,-1,1,-1,0,0,0,-1,1,5*0,-1,0,2*0,0,1,0,-1,0,
     *1,0,-1,0,1,0,-1,0,1,0,-1,0,1,0,-1,0,0,2,1,0,-1,2,1,0,-1,1,0,1,0,
     *1,0,-2,-1,0,1,-2,-1,0,1,-1,0,1,1,0,-1,2,1,0,-1,2,1,0,-1,2,0,1,-1,
     *1,-1,3*0,-1,-1,0,1,0,2*1,0,-1,0, 3*-1,0,1,0,1,1,0,1,-1,0,1,-1,0,0,
     *1,-1,0,1,-1,0,0,1,-1,4*0,1,1,0,2,1,0,1,0,0,2,1,1,-1,-1,0,-2,-1,0,
     *-1,2*0,-2,2*-1,2,1,0,1,2*0,2,2*1,2,-2,-1,0,-1,2*0,-2,2*-1,-2/
C
C      BARYONIC CHARGE
C
      DATA IBAR/1,-1,5*0,1,-1,7*0,1,-1,0,3*1,4*0,2*-1,7*0
     *                                           ,12*0,5*1,14*1,10*-1,
     *2,3*0,4*1,2,2,0,0,4*1,2*1,0,0,1,1,5*-1,6*1,6*-1,21*0,12*1,12*-1,
     *10*1,10*-1/
C
C      FIRST NUMBER OF DECAY CHANNELS USED FOR RESONANCES
C      AND DECYING PARTICLES
C
C************K1
      DATA K1/
     *  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 16, 17, 18,
     * 24, 30, 34, 38, 40, 41, 43, 44,136,138,330,310,317,330,330,
     * 46, 51, 52, 54, 55, 58, 60, 62, 64, 66, 68, 70, 72, 74, 82,
     * 90, 98,106,109,112,114,123,140,141,143,145,146,150,157,164,
     *168,175,182,189,196,204,212,213,215,217,218,222,229,236,240,
     *247,254,256,257,258,259,261,264,267,269,271,274,278,281,284,
     *288,292,295,301,333,337,341,342,343,345,346,347,348,349,352,
     *355,358,360,362,365,368,371,374,376,378,381,385,387,389,393,
     *396,399,402,404,407,410,412,414,416,419,422,427,432,433,434,
     *435,436,450,454,459,460,461,462,463,464,468,470,472,474,488,
     *492,497,498,499,500,501,502,506,508,510,512,513,514,515,516,
     *517,518,519,520,521,524,525,526,527,528,529,530,531,532,324/
C************K2
      DATA K2/
     *  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 15, 16, 17, 23,
     * 29, 31, 35, 39, 40, 42, 43, 45,137,139,330,316,323,330,330,
     * 50, 51, 53, 54, 57, 59, 61, 63, 65, 67, 69, 71, 73, 81, 89,
     * 97,105,108,111,113,122,135,140,142,144,145,149,156,163,167,
     *174,181,188,195,203,211,212,214,216,217,221,228,235,239,246,
     *253,255,256,257,258,260,263,266,268,270,273,277,280,283,287,
     *291,294,300,309,336,340,341,342,344,345,346,347,348,351,354,
     *357,359,361,364,367,370,373,375,377,380,384,386,388,392,395,
     *398,401,403,406,409,411,413,415,418,421,426,431,432,433,434,
     *435,449,453,458,459,460,461,462,463,467,469,471,473,487,491,
     *496,497,498,499,500,501,505,507,509,511,512,513,514,515,516,
     *517,518,519,520,523,524,525,526,527,528,529,530,531,532,326/
C*************ZKNAM1
      DATA ZKNAM1/
     *'P       ','AP      ','E-      ','E+      ','NUE     ','ANUE    ',
     *'GAM     ','PE-NUE  ','APEANU  ','EANUNU  ','E-NUAN  ','3PI0    ',
     *'PI+-0   ','PIMUNU  ','PIE-NU  ','MU+NUE  ','MU-NUE  ','MU+NUE  ',
     *'PI+PI0  ','PI++-   ','PI+00   ','M+P0NU  ','E+P0NU  ','MU-NU   ',
     *'PI-0    ','PI+--   ','PI-00   ','M-P0NU  ','E-P0NU  ','PPI-    ',
     *'NPI0    ','PE-NUE  ','PM-NUE  ','APPI+   ','ANPI0   ','PE+NU   ',
     *'APM+NU  ','PI+PI-  ','PI0PI0  ','NPI-    ','PPI0    ','NPI+    ',
     *'LAGA    ','GAGA    ','GAE+E-  ','GAGA    ','GAGAP0  ','PI000   ',
     *'PI+-0   ','PI+-GA  ','PI+0    ','PI+-    ','PI00    ','PI-0    ',
     *'PI+-0   ','PI+-    ','PI0GA   ','K+PI0   ','K0PI+   ','K0PI0   ',
     *'K+PI-   ','K-PI0   ','AK0PI-  ','AK0PI0  ','K-PI+   ','K+PI0   ',
     *'K0PI+   ','K0PI0   ','K+PI-   ','K-PI0   ','K0PI-   ','AK0PI0  ',
     *'K-PI+   ','K+PI0   ','K0PI+   ','K+89P0  ','K08PI+  ','K+RO77  ',
     *'K0RO+7  ','K+OM07  ','K+E055  ','K0PI0   ','K+PI+   ','K089P0  ',
     *'K+8PI-  ','K0R077  ','K+R-77  '/
C**************ZKNAM2
      DATA ZKNAM2/
     *'K+R-77  ','K0OM07  ','K0E055  ','K-PI0   ','K0PI-   ','K-89P0  ',
     *'AK08P-  ','K-R077  ','AK0R-7  ','K-OM07  ','K-E055  ','AK0PI0  ',
     *'K-PI+   ','AK08P0  ','K-8PI+  ','AK0R07  ','AK0OM7  ','AK0E05  ',
     *'LA0PI+  ','SI0PI+  ','SI+PI0  ','LA0PI0  ','SI+PI-  ','SI-PI+  ',
     *'LA0PI-  ','SI0PI-  ','NEUAK0  ','PK-     ','SI+PI-  ','SI0PI0  ',
     *'SI-PI+  ','LA0ET0  ','S+1PI-  ','S-1PI+  ','SO1PI0  ','NEUAK0  ',
     *'PK-     ','LA0PI0  ','LA0OM0  ','LA0RO0  ','SI+RO-  ','SI-RO+  ',
     *'SI0RO0  ','LA0ET0  ','SI0ET0  ','SI+PI-  ','SI-PI+  ','SI0PI0  ',
     *'K0S     ','K0L     ','K0S     ','K0L     ','P PI+   ','P PI0   ',
     *'N PI+   ','P PI-   ',' PI0    ','N PI-   ','P PI+   '/
C****************ZKNAM3
      DATA ZKNAM3/
     *'N*>PI0  ','N*+PI+  ','PRHO+   ','P PI0   ','N PI+   ','N*>PI-  ',
     *'N*+PI0  ','N*0PI+  ','PRHO0   ','NRHO+   ','P PI-   ','N PI0   ',
     *'N*+PI-  ','N*0PI0  ','N*-PI+  ','PRHO-   ','NRHO0   ','N PI-   ',
     *'N*0PI-  ','N*-PI0  ','NRHO-   ','P PI0   ','N PI+   ','N*>PI-  ',
     *'N*+PI0  ','N*0PI+  ','PRHO0   ','NRHO+   ','N PI0   ','P  PI-  ',
     *'N*+PI-  ','N*0PI0  ','N*-PI+  ','PRHO-   ','NRHO0   ','P PI0   ',
     *'N PI+   ','N*>PI-  ','N*+PI0  ','N*0PI+  ','PRHO0   ','NRHO+   ',
     *'P PI-   ','N PI0   ','N*+PI-  ','N*0PI0  ','N*-PI+  ','PRHO-   ',
     *'NRHO0   ','P PI0   ','N PI+   ','PRHO0   ','NRHO+   ','LAMK+   ',
     *'S+ K0   ','S0 K+   ','PETA0   ','P PI-   ','N PI0   ','PRHO-   ',
     *'NRHO0   ','LAMK0   ','S0 K0   ','S- K+   ','NETA/   ','APPI-   ',
     *'APPI0   ','ANPI-   ','APPI+   ','ANPI0   ','ANPI+   ','APPI-   ',
     *'AN*=P0  ','AN*-P-  ','APRHO-  ','APPI0   ','ANPI-   ','AN*=P+  ',
     *'AN*-P0  ','AN*0P-  ','APRHO0  ','ANRHO-  ','APPI+   ','ANPI0   ',
     *'AN*-P+  ','AN*0P0  ','AN*+P-  ','APRHO+  ','ANRHO0  ','ANPI+   ',
     *'AN*0P+  ','AN*+P0  ','ANRHO+  ','APPI0   ','ANPI-   ','AN*=P+  ',
     *'AN*-P0  ','AN*0P-  ','APRHO0  ','ANRHO-  ','APPI+,  ','ANPI0   ',
     *'AN*-P+  ','AN*0P0  ','AN*+P-  '/
C*****************ZKNAM4
      DATA ZKNAM4/
     *'APRHO+  ','ANRHO0  ','PN*014  ','NN*=14  ','PI+0    ','PI+-    ',
     *'PI-0    ','P+0     ','N++     ','P+-     ','P00     ','N+0     ',
     *'N+-     ','N00     ','P-0     ','N-0     ','P--     ','PPPI0   ',
     *'PNPI+   ','PNPI0   ','PPPI-   ','NNPI+   ','APPPI0  ','APNPI+  ',
     *'ANNPI0  ','ANPPI-  ','APNPI0  ','APPPI-  ','ANNPI-  ','K+PPI0  ',
     *'K+NPI+  ','K0PPI0  ','K-PPI0  ','K-NPI+  ','AKPPI-  ','AKNPI0  ',
     *'K+NPI0  ','K+PPI-  ','K0PPI0  ','K0NPI+  ','K-NPI0  ','K-PPI-  ',
     *'AKNPI-  ','PAK0    ','SI+PI0  ','SI0PI+  ','SI+ETA  ','S+1PI0  ',
     *'S01PI+  ','NEUK-   ','LA0PI-  ','SI-OM0  ','LA0RO-  ','SI0RO-  ',
     *'SI-RO0  ','SI-ET0  ','SI0PI-  ','SI-0    ','AP PI0  ','AN  PI+ ',
     *'AN*>PI+ ','AN*-PI0 ','AN*0PI- ','APRHO0  ','AN*RHO- ','AN PI0  ',
     *'AP PI+  ','AN*-PI+ ','AN*0PI0 ','AN*+PI- ','APRHO+  ','AN*RHO0 ',
     *'AA-KPI  ','AT02PI  ','AC1--K  ','        ','        ','        ',
     *'        '/
C*******************ZKNAM5
      DATA ZKNAM5/
     *'        ','        ','EPI+-   ','EPI00   ','GAPI+-  ','GAGA*   ',
     *'K+-     ','KLKS    ','PI+-0   ','EGA     ','LPI0    ','LPI     ',
     *'APPI0   ','ANPI-   ','ALAGA   ','ANPI    ','ALPI0   ','ALPI+   ',
     *'LAPI+   ','SI+PI0  ','SI0PI+  ','LAPI0   ','SI+PI-  ','SI-PI+  ',
     *'LAPI-   ','SI-PI0  ','SI0PI-  ','TE0PI0  ','TE-PI+  ','TE0PI-  ',
     *'TE-PI0  ','TE0PI   ','TE-PI   ','LAK-    ','ALPI-   ','AS-PI0  ',
     *'AS0PI-  ','ALPI0   ','AS+PI-  ','AS-PI+  ','ALPI+   ','AS+PI0  ',
     *'AS0PI+  ','AT0PI0  ','AT+PI-  ','AT0PI+  ','AT+PI0  ','AT0PI   ',
     *'AT+PI   ','ALK+    ','K-PI+   ','K-PI+0  ','K0PI+-  ','K0PI0   ',
     *'K-PI++  ','AK0PI+  ','K+PI--  ','K0PI-   ','K+PI-   ','K+PI-0  ',
     *'AKPI-+  ','AK0PI0  ','ETAPIF  ','K++-    ','K+AK0   ','ETAPI-  ',
     *'K--+    ','K-KO    ','PI00    ','PI+-    ','GAGA    ','D0PI0   ',
     *'D0GA    ','D0PI+   ','D+PI0   ','DFGA    ','AD0PI-  ','D-PI0   ',
     *'D-GA    ','AD0PI0  ','AD0GA   ','F+GA    ','F+GA    ','F-GA    ',
     *'F-GA    ','PSPI+-  ','PSPI00  ','PSETA   ','E+E-    ','MUE+-   ',
     *'PI+-0   ','M+NN    ','E+NN    ','RHO+NT  ','PI+ANT  ','K*+ANT  ',
     *'M-NN    ','E-NN    ','RHO-NT  ','PI-NT   ','K*-NT   ','NUET    ',
     *'ANUET   '/
C**************ZKNAM6
      DATA ZKNAM6/
     *'NUEM    ','ANUEM   ','SI+ETA  ','SI+ET*  ','PAK0    ','TET0K+  ',
     *'SI*+ET  ','N*+AK0  ','N*++K-  ','LAMRO+  ','SI0RO+  ','SI+RO0  ',
     *'SI+OME  ','PAK*0   ','N*+AK*  ','N*++K*  ','SI+AK0  ','TET0PI  ',
     *'SI+AK*  ','TET0RO  ','SI0AK*  ','SI+K*-  ','TET0OM  ','TET-RO  ',
     *'SI*0AK  ','C0+PI+  ','C0+PI0  ','C0+PI-  ','A+GAM   ','A0GAM   ',
     *'TET0AK  ','TET0K*  ','OM-RO+  ','OM-PI+  ','C1++AK  ','A+PI+   ',
     *'C0+AK0  ','A0PI+   ','A+AK0   ','T0PI+   ','ASI-ET  ','ASI-E*  ',
     *'APK0    ','ATET0K  ','ASI*-E  ','AN*-K0  ','AN*--K  ','ALAMRO  ',
     *'ASI0RO  ','ASI-RO  ','ASI-OM  ','APK*0   ','AN*-K*  ','AN*--K  ',
     *'ASI-K0  ','ATETPI  ','ASI-K*  ','ATETRO  ','ASI0K*  ','ASI-K*  ',
     *'ATE0OM  ','ATE+RO  ','ASI*0K  ','AC-PI-  ','AC-PI0  ','AC-PI+  ',
     *'AA-GAM  ','AA0GAM  ','ATET0K  ','ATE0K*  ','AOM+RO  ','AOM+PI  ',
     *'AC1--K  ','AA-PI-  ','AC0-K0  ','AA0PI-  ','AA-K0   ','AT0PI-  ',
     *'C1++GA  ','C1++GA  ','C10GAM  ','S+GAM   ','S0GAM   ','T0GAM   ',
     *'XU++GA  ','XD+GAM  ','XS+GAM  ','A+AKPI  ','T02PI+  ','C1++2K  ',
     *'AC1--G  ','AC1-GA  ','AC10GA  ','AS-GAM  ','AS0GAM  ','AT0GAM  ',
     *'AXU--G  ','AXD-GA  ','AXS-GA  ','        '/
C*****************WT
      DATA WT/
     *1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,
     *1.0000,1.0000,0.2100,0.1300,0.2700,0.3900,1.0000,1.0000,0.6300,
     *0.2100,0.0200,0.0600,0.0300,0.0500,0.6300,0.2100,0.0200,0.0600,
     *0.0300,0.0500,0.6400,0.3600,0.0   ,0.0   ,0.6400,0.3600,0.0   ,
     *0.0   ,0.6900,0.3100,1.0000,0.5200,0.4800,1.0000,0.9900,0.0100,
     *0.3800,0.0300,0.3000,0.2400,0.0500,1.0000,1.0000,0.0   ,1.0000,
     *0.9000,0.0100,0.0900,0.3300,0.6700,0.3300,0.6700,0.3300,0.6700,
     *0.3300,0.6700,0.3300,0.6700,0.3300,0.6700,0.3300,0.6700,0.3300,
     *0.6700,0.1900,0.3800,0.0900,0.2000,0.0300,0.0400,0.0500,0.0200,
     *0.1900,0.3800,0.0900,0.2000,0.0300,0.0400,0.0500,0.0200,0.1900,
     *0.3800,0.0900,0.2000,0.0300,0.0400,0.0500,0.0200,0.1900,0.3800,
     *0.0900,0.2000,0.0300,0.0400,0.0500,0.0200,0.8800,0.0600,0.0600,
     *0.8800,0.0600,0.0600,0.8800,0.1200,0.1900,0.1900,0.1600,0.1600,
     *0.1700,0.0300,0.0300,0.0300,0.0400,0.1000,0.1000,0.2000,0.1200,
     *0.1000,0.0400,0.0400,0.0500,0.0750,0.0750,0.0300,0.0300,0.0400/
      DATA WT1/
     *0.5000,0.5000,0.5000,0.5000,1.0000,0.6700,0.3300,0.3300,0.6700,
     *1.0000,0.2500,0.2700,0.1800,0.3000,0.1700,0.0800,0.1800,0.0300,
     *0.2400,0.2000,0.1000,0.0800,0.1700,0.2400,0.0300,0.1800,0.1000,
     *0.2000,0.2500,0.1800,0.2700,0.3000,0.225 ,0.375 ,0.15  ,0.0938,
     *0.0562,0.0375,0.0626,0.225 ,0.375 ,0.15  ,0.0938,0.0562,0.0375,
     *0.0625,0.1800,0.3700,0.1300,0.0800,0.0400,0.0700,0.1300,0.3700,
     *0.1800,0.0400,0.0800,0.1300,0.1300,0.0700,0.0700,0.1300,0.2300,
     *0.4700,0.0500,0.0200,0.0100,0.0200,0.1300,0.0700,0.4700,0.2300,
     *0.0500,0.0100,0.0200,0.0200,1.0000,0.6700,0.3300,0.3300,0.6700,
     *1.0000,0.2500,0.2700,0.1800,0.3000,0.1700,0.0800,0.1800,0.0300,
     *0.2400,0.2000,0.1000,0.0800,0.1700,0.2400,0.0300,0.1800,0.1000,
     *0.2000,0.2500,0.1800,0.2700,0.3000,0.1800,0.3700,0.1300,0.0800,
     *0.0400,0.0700,0.1300,0.3700,0.1800,0.0400,0.0800,0.1300,0.1300,
     *0.0700,0.5000,0.5000,1.0000,1.0000,1.0000,0.8000,0.2000,0.6000,
     *0.3000,0.1000,0.6000,0.3000,0.1000/
C************WT2
      DATA WT2/
     *0.8000,0.2000,0.3300,0.6700,0.6600,0.1700,0.1700,0.3200,0.1700,
     *0.3200,0.1900,0.3300,0.3300,0.3400,0.3000,0.0500,0.6500,0.3800,
     *0.1200,0.3800,0.1200,0.3800,0.1200,0.3800,0.1200,0.3000,0.0500,
     *0.6500,0.3800,0.2500,0.2500,0.0200,0.0500,0.0500,0.2000,0.2000,
     *0.1200,0.1000,0.0700,0.0700,0.1400,0.0500,0.0500,0.225 ,0.375 ,
     *0.15  ,0.0938,0.0562,0.0375,0.0625,0.225 ,0.375 ,0.15  ,0.0938,
     *0.0562,0.0375,0.0625,0.3000,0.3000,0.4000,1.0   ,1.0   ,1.0   ,
     *1.0000/
C***************WT4
      DATA WT4/
     *1.0000,1.0000,0.0200,0.0300,0.0700,0.0200,0.0200,0.0400,0.1300,
     *0.0700,0.0600,0.0600,0.2000,0.1400,0.0400,0.1000,0.2500,0.0300,
     *0.3000,0.4200,0.2200,0.3500,0.1900,0.1600,0.0800,1.0000,1.0000,
     *1.0000,1.0000,1.0000,0.3700,0.2000,0.3600,0.0700,0.5000,0.5000,
     *0.5000,0.5000,0.5000,0.5000,0.0200,0.0300,0.0700,0.0200,0.0200,
     *0.0400,0.1300,0.0700,0.0600,0.0600,0.2000,0.1400,0.0400,0.1000,
     *0.2500,0.0300,0.3000,0.4200,0.2200,0.3500,0.1900,0.1600,0.0800,
     *1.0000,1.0000,1.0000,1.0000,1.0000,0.3700,0.2000,0.3600,0.0700,
     *0.5000,0.5000,0.5000,0.5000,0.5000,0.5000,1.0000,1.0000,1.0000,
     *1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,0.3000,0.3000,0.4000,
     *1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,
     *1.    /
C****************WT3
      DATA WT3/
     *1.0   ,1.0   ,0.4800,0.2400,0.2600,0.0200,0.4700,0.3500,0.1500,
     *0.0300,1.0000,1.0000,0.5200,0.4800,1.0000,1.0000,1.0000,1.0000,
     *0.9000,0.0500,0.0500,0.9000,0.0500,0.0500,0.9000,0.0500,0.0500,
     *0.3300,0.6700,0.6700,0.3300,0.2500,0.2500,0.5000,0.9000,0.0500,
     *0.0500,0.9000,0.0500,0.0500,0.9000,0.0500,0.0500,0.3300,0.6700,
     *0.6700,0.3300,0.2500,0.2500,0.5000,0.1000,0.5000,0.1600,0.2400,
     *0.7000,0.3000,0.7000,0.3000,0.1000,0.5000,0.1600,0.2400,0.3000,
     *0.4000,0.3000,0.3000,0.4000,0.3000,0.4900,0.4900,0.0200,0.5500,
     *0.4500,0.6800,0.3000,0.0200,0.6800,0.3000,0.0200,0.5500,0.4500,
     *0.9000,0.1000,0.9000,0.1000,0.6000,0.3000,0.1000,0.1000,0.1000,
     *0.8000,0.2800,0.2800,0.3500,0.0700,0.0200,0.2800,0.2800,0.3500,
     *0.0700,0.0200,1.0000,1.0000/
C************NZK1
      DATA NZK01/
     *  1,  2,  3,  4,  5,  6,  7,  1,  2,  4,  3, 23, 13, 13, 13,
     * 10, 11, 10, 13, 13, 13, 10,  4, 11, 14, 14, 14, 11,  3,  1,
     *  8,  1,  1,  2,  9,  2,  2, 13, 23,  8,  1,  8, 17,  7,  7,
     *  7, 23, 23, 13, 13, 13, 13, 23, 14, 13, 13, 23, 15, 24, 24,
     * 15, 16, 25, 25, 16, 15, 24, 24, 15, 16, 24, 25, 16, 15, 24,
     * 36, 37, 15, 24, 15, 15, 24, 15, 37, 36, 24, 15, 24, 24, 16,
     * 24, 38, 39, 16, 25, 16, 16, 25, 16, 39, 38, 25, 16, 25, 25,
     * 17, 22, 21, 17, 21, 20, 17, 22,  8,  1, 21, 22, 20, 17, 48,
     * 50, 49,  8,  1, 17, 17, 17, 21, 20, 22, 17, 22, 21, 20, 22,
     * 19, 12, 19, 12,  1,  1,  8,  1,  8,  8,  1, 53, 54,  1,  1,
     *  8, 53, 54, 55,  1,  8,  1,  8, 54, 55, 56,  1,  8,  8, 55,
     * 56,  8,  1,  8, 53, 54, 55,  1,  8,  8,  1, 54, 55, 56,  1,
     *  8,  1,  8, 53, 54, 55,  1,  8,  1,  8, 54, 55, 56,  1,  8/
      DATA NZK1/
     *  1,  8,  1,  8, 17, 21, 22,  1,  1,  8,  1,  8, 17, 22, 20,
     *  8,  2,  2,  9,  2,  9,  9,  2, 67, 68,  2,  2,  9, 67, 68,
     * 69,  2,  9,  2,  9, 68, 69, 70,  2,  9,  9, 69, 70,  9,  2,
     *  9, 67, 68, 69,  2,  9,  2,  9, 68, 69, 70,  2,  9,  1,  8,
     * 13, 13, 14,  1,  8,  1,  1,  8,  8,  8,  1,  8,  1,  1,  1,
     *  1,  1,  8,  2,  2,  9,  9,  2,  2,  9, 15, 15, 24, 16, 16,
     * 25, 25, 15, 15, 24, 24, 16, 16, 25,  1, 21, 22, 21, 48, 49,
     *  8, 17, 20, 17, 22, 20, 20, 22, 20,  2,  9, 67, 68, 69,  2,
     *  9,  9,  2, 68, 69, 70,  2,  9,150,157,152,  0,  0,  0,  0,
     *  0,  0, 31, 31, 13,  7, 15, 12, 13, 31, 17, 17,  2,  9, 18,
     *  9, 18, 18, 17, 21, 22, 17, 21, 20, 17, 20, 22, 97, 98, 97,
     * 98, 97, 98, 17, 18, 99,100, 18,101, 99, 18,101,100,102,103,
     *102,103,102/
C************NZK11
      DATA NZK11/
     *103, 18, 16, 16, 24, 24, 16, 25, 15, 24, 15, 15, 25, 25, 31,
     * 15, 15, 31, 16, 16, 23, 13,  7,116,116,116,117,117,119,118,
     *118,119,119,120,120,121,121,130,130,130,  4, 10, 13, 10,  4,
     * 32, 13, 36, 11,  3, 34, 14, 38,133,134,135,136, 21, 21,  1,
     * 97,104, 54, 53, 17, 22, 21, 21,  1, 54, 53, 21, 97, 21, 97,
     * 22, 21, 97, 98,105,137,137,137,138,139, 97, 97,109,109,140,
     *138,137,139,138,145, 99, 99,  2,102,110, 68, 67, 18,100, 99,
     * 99,  2, 68, 67, 99,102, 99,102,100, 99,102,103,111,149,149,
     *149,150,151,113,113,115,115,152,150,149,151,150,157,140,141,
     *142,143,144,145,146,147,148,138,145,140,152,153,154,155,156,
     *157,158,159,160,  0/
C************NZK2
      DATA NZK02/
     *  0,  0,  0,  0,  0,  0,  0,  3,  4,  6,  5, 23, 14, 11,  3,
     *  5,  5,  5, 23, 13, 23, 23, 23,  5, 23, 13, 23, 23, 23, 14,
     * 23,  3, 11, 13, 23,  4, 10, 14, 23, 14, 23, 13,  7,  7,  4,
     *  7,  7, 23, 14, 14, 23, 14, 23, 23, 14, 14,  7, 23, 13, 23,
     * 14, 23, 14, 23, 13, 23, 13, 23, 14, 23, 14, 23, 13, 23, 13,
     * 23, 13, 33, 32, 35, 31, 23, 14, 23, 14, 33, 34, 35, 31, 23,
     * 14, 23, 14, 33, 34, 35, 31, 23, 13, 23, 13, 33, 32, 35, 31,
     * 13, 13, 23, 23, 14, 13, 14, 14, 25, 16, 14, 23, 13, 31, 14,
     * 13, 23, 25, 16, 23, 35, 33, 34, 32, 33, 31, 31, 14, 13, 23,
     *  0,  0,  0,  0, 13, 23, 13, 14, 23, 14, 13, 23, 13, 78, 23,
     * 13, 14, 23, 13, 79, 78, 14, 23, 14, 23, 13, 80, 79, 14, 14,
     * 23, 80, 23, 13, 14, 23, 13, 79, 78, 23, 14, 14, 23, 13, 80,
     * 79, 23, 13, 14, 23, 13, 79, 78, 14, 23, 14, 23, 13, 80, 79/
      DATA NZK2/
     * 23, 13, 33, 32, 15, 24, 15, 31, 14, 23, 34, 33, 24, 24, 15,
     * 31, 14, 23, 14, 13, 23, 13, 14, 23, 14, 80, 23, 14, 13, 23,
     * 14, 79, 80, 13, 23, 13, 23, 14, 78, 79, 13, 13, 23, 78, 23,
     * 14, 13, 23, 14, 79, 80, 13, 23, 13, 23, 14, 78, 79, 62, 61,
     * 23, 14, 23, 13, 13, 13, 23, 13, 13, 23, 14, 14, 14,  1,  8,
     *  8,  1,  8,  1,  8,  8,  1,  8,  1,  8,  1,  8,  1,  1,  8,
     *  1,  8,  8,  1,  1,  8,  8,  1,  8, 25, 23, 13, 31, 23, 13,
     * 16, 14, 35, 34, 34, 33, 31, 14, 23, 23, 14, 13, 23, 14, 79,
     * 80, 23, 13, 13, 23, 14, 78, 79, 24, 14, 24,  0,  0,  0,  0,
     *  0,  0, 13, 23, 14,  7, 16, 19, 14,  7, 23, 14, 23, 14,  7,
     * 13, 23, 13, 13, 23, 13, 23, 14, 13, 14, 23, 14, 23, 13, 14,
     * 23, 14, 23, 16, 14, 23, 14, 23, 14, 13, 13, 23, 13, 23, 14,
     * 13, 23, 13/
C************NZK22
      DATA NZK22/
     * 23, 15, 13, 13, 13, 23, 13, 13, 14, 14, 14, 14, 14, 23, 13,
     * 16, 25, 14, 15, 24, 23, 14,  7, 23,  7, 13, 23,  7, 14, 23,
     *  7, 23,  7,  7,  7,  7,  7, 13, 23, 31,  3, 11, 14,135,  5,
     *134,134,134,136,  6,133,133,133,  0,  0,  0,  0, 31, 95, 25,
     * 15, 31, 95, 16, 32, 32, 33, 35, 39, 39, 38, 25, 13, 39, 32,
     * 39, 38, 35, 32, 39, 13, 23, 14,  7,  7, 25, 37, 32, 13, 25,
     * 13, 25, 13, 25, 13, 31, 95, 24, 16, 31, 24, 15, 34, 34, 33,
     * 35, 37, 37, 36, 24, 14, 37, 34, 37, 36, 35, 34, 37, 14, 23,
     * 13,  7,  7, 24, 39, 34, 14, 24, 14, 24, 14, 24, 14,  7,  7,
     *  7,  7,  7,  7,  7,  7,  7, 25, 13, 25,  7,  7,  7,  7,  7,
     *  7,  7,  7,  7,  0/
C************NZK3
      DATA NZK03/
     *  0,  0,  0,  0,  0,  0,  0,  5,  6,  5,  6, 23, 23,  5,  5,
     *  0,  0,  0,  0, 14, 23,  5,  5,  0,  0, 14, 23,  5,  5,  0,
     *  0,  5,  5,  0,  0,  5,  5,  0,  0,  0,  0,  0,  0,  0,  3,
     *  0,  7, 23, 23,  7,  0,  0,  0,  0, 23,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0/
      DATA NZK3/
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0, 23, 13, 14, 23, 23, 14, 23, 23, 23, 14, 23, 13,
     * 23, 14, 13, 23, 13, 23, 14, 23, 14, 14, 23, 13, 13, 23, 13,
     * 14, 23, 23, 14, 23, 13, 23, 14, 14,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0, 14, 14, 24,  0,  0,  0,  0,
     *  0,  0, 14, 23,  7,  0,  0,  0, 23,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0/
      DATA NZK33/
     *  0,  0,  0, 23, 14,  0, 13,  0, 14,  0,  0, 23, 13,  0,  0,
     * 15,  0,  0, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0, 14, 23,  0,  0,  0, 23,134,134,
     *  0,  0,  0,133,133,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0,  0,  0, 13, 13, 25,  0,  0,  0,  0,  0,
     *  0,  0,  0,  0,  0/
      DATA XDQ/0./
      DATA XDQ1/0./
      DATA IFL/1,2,3,4,5,6,7,8,9,10,11,12/
      DATA PUDSC/0.43,0.86,1.,1./
      DATA ALPHAR/0.5/
      DATA ALPHAN/-0.5/
      DATA ALPHAF/0./
      DATA ALPHA/0.5,0.5,0.,0./
      DATA ISYS,IDGB,DQFRAG,RM/6,-1,0.5,0.764/
      DATA RDI,BSLOP,PTMIN/1.2,8.0,0./
      END

      SUBROUTINE DATESS
      COMMON/DECAYC/ZKNAME(533),NZK(533,3),WT(533)
      COMMON/PART/ANAME(180),AM(180),GA(180),TAU(180),ICH(180),IBAR(180)
     *,K1(180),K2(180)
      COMMON/METLSP/ IS,ITS(100000),CXS(100000),CYS(100000),CZS(100000),
     *ELS(100000),PLS(100000)
      COMMON/FINLSP/IR,ITR(100),CXR(100),CYR(100),CZR(100),ELR(100),
     *PLR(100)
      DIMENSION ICHAR(180)
      EQUIVALENCE (ICH(1),ICHAR(1))
      DIMENSION Z(3)
*      DIMENSION IREACT(24),HSI(31)
      CHARACTER*8 ZKNAME
      CHARACTER*8 ANAME
      CHARACTER*8 Z
*      DATA IREACT/13,1,13,8,14,1,14,8,15,1,15,8,16,1,16,8,1,1,1,8,2,1,2,
*     *8/
      I12=12
*      PRINT 102
  102 FORMAT(///,' TABLE OF USED PARTICLES AND RESONANCES (I)',//
     *' I = NUMBER OF PARTICLE OR RESONANCE',/
     *' ANAME = NAME OF I'/,
     *' AM = MASS OF I  (GEV)',/
     *' GA = WIDTH OF I (GEV)',/
     *' TAU = LIFE TIME OF I  (SEC.)',/
     *' ICH = ELECTRIC CHARGE OF I, IBAR = BARYONIC CHARGE OF I',/' ',
     *' K1 = FIRST DECAY CHANNEL NUMBER, K2 = LAST DECAY CHANNEL NUMBER'
     *,' OF I')
      JOO=180
      DO 41 I=1,JOO
   41 CONTINUE
*      PRINT 111
*      PRINT 92
   92 FORMAT(///' DECAY CHANNELS OF PARTICLES  AND RESONANCES',//)
*      PRINT 93
   93 FORMAT(' ANAME = PARTICLE AND RESONANCE NAME'/,
     *' DNAME = DECAY CHANNEL NAME'/,
     *' J = DECAY CHANNEL NUMBER'/,
     *' I = NUMBER OF DECAYING PARTICLE'/,
     *' WT = SUM OF DECAY CHANNEL WEIGHTS FROM K1(I) UP TO J'/,
     *' NZK = PROGRAM INTERNKAL NUMBERS OF DECAY PRODUCTS')
      DO 2 I=1,JOO
      IK1=K1(I)
      IK2=K2(I)
      IF(IK1.LE.0) GO TO 2
      DO 3 IK=IK1,IK2
      I1=NZK(IK,1)
      I2=NZK(IK,2)
      I3=NZK(IK,3)
      IF(I1.LE.0) I1=29
      IF(I2.LE.0) I2=29
      IF(I3.LE.0) I3=29
      J1=I1
      J2=I2
      J3=I3
      Z(1)=ANAME(I1)
      Z(2)=ANAME(I2)
      Z(3)=ANAME(I3)
      AMTEST= AM(I)-AM(J1)-AM(J2)-AM(J3)
      IBTEST=IBAR(I)-IBAR(J1)-IBAR(J2)-IBAR(J3)
      ICTEST=ICHAR(I)-ICHAR(J1)-ICHAR(J2)-ICHAR(J3)
      IF(AMTEST) 51,52,52
   51 MTEST=1
      GO TO 53
   52 MTEST=0
   53 CONTINUE
    3 CONTINUE
    2 CONTINUE
      RETURN
      END

      FUNCTION DSIG_COL(T)
      COMMON/ab/ aelm, betav, sigma_tot, parB, rho
      G4=((1.0+abs(T)/0.71)**(-2.0))**4.0
      pkoef= 10./(5.0677**2)        !/10.  
      DSIG_COL=4.0*3.1416*(aelm**2.0)*G4*pkoef/((betav * T)**2.0)
      RETURN 
      END

      Function  DSIG_INTER(T)
      COMMON/ab/ aelm, betav, sigma_tot, parB, rho
      G2=((1+abs(T)/0.71)**(-2))**2
      DSIG_INTER=aelm*sigma_tot*G2*exp(0.5*parB*T)*
     & sqrt(1+rho**2)/ betav/abs(T)
      RETURN
      END

      Function  DSIG_INT_Ex(T)
      COMMON/ab/ aelm, betav, sigma_tot, parB, rho
      G2=((1+abs(T)/0.71)**(-2))**2
      delT=aelm*(0.577+log(parB*abs(T)/2.+ 4*abs(T)/0.71)+
     &  4.*abs(T)/0.71*log(4.*abs(T)/0.71)+2.*abs(T)/0.71)
      DSIG_INT_Ex=aelm*sigma_tot*G2*exp(0.5*parB*T)*
     & (rho*cos(delT) + sin(delT))/ betav/abs(T)
      RETURN
      END


      FUNCTION SIG_HADi(T)
      COMMON/UZHI/SqrtS,Ecms,Vcms,Gamma,Proc_Prob(7),P_5str,CS_in,
     ,            CS_el,A1,T1,A2,T2,A3,Tmax,Tmin,Weight1
      
      pk=0.1/(5.0677**2)     !normir 
      SIG_HADi= A1*T1*exp(T/T1)+A1*(A2**2)*T2*exp(T/T2)-
     - 4.*A1*A2*T1*T2/(T1+T2)*exp(T*(T1+T2)/(2.*T1*T2))+
     + A3*T2*exp(T/T2)                                     

       RETURN 
       END


      FUNCTION DSIG_HAD(T)
      COMMON/UZHI/SqrtS,Ecms,Vcms,Gamma,Proc_Prob(7),P_5str,CS_in,
     ,            CS_el,A1,T1,A2,T2,A3,Tmax,Tmin,Weight1
      COMMON/ab/ aelm, betav, sigma_tot, parB, rho
       pk=(5.0677**2)/10.     !normirovka  
      DSIG_HAD=sigma_tot**2*(1+rho**2)*exp(parB*T)/16./3.1416*pk
      RETURN
      END
