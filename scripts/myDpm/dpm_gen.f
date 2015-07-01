C------Last edition 25.08.05 V.Uzhinsky--------------------
C------modify by A.Galoyan 10.09.08--------------------
      subroutine DPM_GEN(Pluto, Seed)
      COMMON /LUJETS/ N,K(1000,2),P(1000,5)
      
      COMMON/UZHI/SqrtS,Ecms,Vcms,Gamma,Proc_Prob(7),P_5str,CS_in,
     ,            CS_el,A1,T1,A2,T2,A3,Tmax,Tmin,Weight1,AMProton
 
      COMMON/AB/aelm,betav, sigma_tot, parB, rho      !aida
      COMMON /AGT/ TTR 
      double precision Seed

      call DPM_EVENT(Nhad)
      IF (Pluto.lt.0.5) then
         CALL TOPITH(Nhad)      ! To transfer to PITHYA format
      ELSE
         CALL TOPLUTO(Nhad)     ! To transfer to PLUTO format
      ENDIF

      call RNDMGET1(Seed)

      return
      end

      SUBROUTINE DPM_EVENT(Nhad)
C-----------------------------------------------------------------------
      
      COMMON/UZHI/SqrtS,Ecms,Vcms,Gamma,Proc_Prob(7),P_5str,CS_in,
     ,            CS_el,A1,T1,A2,T2,A3,Tmax,Tmin,Weight1,AMProton
 
C-----------------------------------------------------------------------
C             PARAMETERS OF QUARK-GLUON STRING MODEL
C-----------------------------------------------------------------------
      COMMON/QGMPAR/ IFL(12), ALPHAR, ALPHAN, ALPHAF, ALPHA(4),
     *               PUDSC(4), BSLOP, VQMASS, DIQMAS
C =========================================================================
      COMMON/FINPAR/PXF(10000),PYF(10000),PZF(10000),HEF(10000),
     *AMF(10000),ICHF(10000),IBARF(10000),ANF(10000),NREF(10000)
      CHARACTER*8 ANF

      COMMON/MIDPAR/
     *PXM(1000),PYM(1000),PZM(1000),HEM(1000),AMM(1000),ICHM(1000),
     *IBARM(1000),ANM(1000),NREM(1000)
      CHARACTER*8 ANM
      common/prob_el/prob_col, prob_int, prob_had    !aida
      COMMON/AB/aelm,betav, sigma_tot, parB, rho      !aida

      COMMON/PART/ANAME(180),AM(180),GA(180),TAU(180),ICH(180),IBAR(180)
     ,           ,K1(180),K2(180)
      COMMON /AGT/ TTR
      CHARACTER*8 ANAME

C-----------------------------------------------------------------------
      REAL Typ_aQ(7), Px_aQ(7), Py_aQ(7), Pz_aQ(7), Mt_aQ_2(7), X_aQ(7)
      REAL Typ_Q(7) , Px_Q(7) , Py_Q(7) , Pz_Q(7) , Mt_Q_2(7) , X_Q(7)
c-----------------------------------------------------------------------
      DIMENSION PROJ(5), TAR(5)
c-----------------------------------------------------------------------
      SS=SqrtS
      S=SS**2

C--------------------------------------- Coise of a process ------------
      EPS=RNDM1(-1)

      do I=1,7
        if(EPS.le.Proc_Prob(I)) go to 10
      enddo

 10   CONTINUE
*            I=1, 2, 3, 4, 5, 6 - Processes A, B, C, E, G, H, 7 -Elastic Sctr.
      goto(100,200,300,400,500,600,700), I

 100  CONTINUE
C####################################### Simulation of process A ############
C                   3 or 5 string configurations
C############################################################################

      if(RNDM1(-1).le.P_5str) then
        N_QS=5                    ! N_QS - number of valence quarks
      else
        N_QS=3
      endif
      N_QS=3                                      ! Uzhi

c ------------------------------------ Determination of \bar P contants--
      if(RNDM1(-1).le.0.33333) then
        Typ_aQ(1)=8.
        Typ_aQ(2)=7.
        Typ_aQ(3)=7.
      else
        Typ_aQ(1)=7.
        if(RNDM1(-1).le.0.5) then
          Typ_aQ(2)=7.
          Typ_aQ(3)=8.
        else
          Typ_aQ(2)=8.
          Typ_aQ(3)=7.
        endif
      endif

      if(N_QS.gt.3) then
        if(RNDM1(-1).le.0.5) then
          Typ_aQ(4)=1.
          Typ_aQ(5)=7.
        else
          Typ_aQ(4)=2.
          Typ_aQ(5)=8.
        endif
      endif

c ------------------------------------ Determination of P contants--
      if(RNDM1(-1).le.0.33333) then
        Typ_Q(1)=2.
        Typ_Q(2)=1.
        Typ_Q(3)=1.
      else
        Typ_Q(1)=1.
        if(RNDM1(-1).le.0.5) then
          Typ_Q(2)=1.
          Typ_Q(3)=2.
        else
          Typ_Q(2)=2.
          Typ_Q(3)=1.
        endif
      endif

      if(N_QS.gt.3) then
        if(RNDM1(-1).le.0.5) then
          Typ_Q(4)=7.
          Typ_Q(5)=1.
        else
          Typ_Q(4)=8.
          Typ_Q(5)=2.
        endif
      endif

 109   continue

      Nrepit=0
      Bslp=Bslop*2.
 110  CONTINUE
      Nrepit=Nrepit+1
      if(mod(Nrepit,50).eq.0) Bslp=Bslp*2
c ------------------------------------ Determination of Pt_i and X_i -

      SumPxa=0.
      SumPya=0.

      SumPx =0.
      SumPy =0.

      do i=1,N_QS
        FI=RNDM1(-1)*6.28318
        Pt=-1./Bslp*ALOG(RNDM1(-1)*RNDM1(-1))
        Px_aQ(i)=Pt*Cos(Fi)                 ! Uzhi 0.
        Py_aQ(i)=Pt*Sin(Fi)                 ! Uzhi 0.
**        X_aQ(i)=1./float(N_QS)
caida        X_aQ(i)=RNDM1(-1)              !1./float(N_QS) ! Uzhi   1/3

        SumPxa=SumPxa+Px_aQ(i)
        SumPya=SumPya+Py_aQ(i)

        FI=RNDM1(-1)*6.28318
        Pt=-1./Bslp*ALOG(RNDM1(-1)*RNDM1(-1))
        Px_Q(i) =Pt*Cos(Fi)                 ! Uzhi 0.
        Py_Q(i) =Pt*Sin(Fi)                 ! Uzhi 0.
***         X_Q(i)=1./float(N_QS) 
caida        X_Q(i)=RNDM1(-1)              !1./float(N_QS)   ! Uzhi  1/3

        SumPx =SumPx +Px_Q(i)
        SumPy =SumPy +Py_Q(i)
      enddo

      SumPxa=SumPxa/float(N_QS)
      SumPya=SumPya/float(N_QS)

 
      SumPx =SumPx/float(N_QS)
      SumPy =SumPy/float(N_QS)
      Sum_Mt=0.

      do i=1,N_QS
        Px_aQ(i)=Px_aQ(i)-SumPxa
        Py_aQ(i)=Py_aQ(i)-SumPya
        Mt_aQ_2(i)=VQMASS**2+Px_aQ(i)**2+Py_aQ(i)**2
        Sum_Mt=Sum_Mt+sqrt(Mt_aQ_2(i))

        Px_Q(i) =Px_Q(i) -SumPx
        Py_Q(i) =Py_Q(i) -SumPy
        Mt_Q_2(i) =VQMASS**2+Px_Q(i)**2 +Py_Q(i)**2
        Sum_Mt=Sum_Mt+sqrt(Mt_Q_2(i))
      enddo

      if(Sum_Mt.ge.SS) go to 110

120   CONTINUE

      ALPHAR=2.0               !aida  changed quark mass distribution
      Alfa=1./ALPHAR
      BetaB=(ALPHAR-1.)+(N_QS-2)*ALPHAR
      N_QSm1=N_QS-1

      ProdXaq=1.
      Sum_Xaq=0.

      ProdXq=1.
      Sum_Xq=0.

      do i=1,N_QSm1
      Beta=1./(BetaB+1.)
130   R1=RNDM1(-1)
      R2=RNDM1(-1)
      R1a=R1**Alfa
      R2b=R2**Beta
      R12=R1a+R2b
      if(R12.gt.1.)go to 130
      Xa=R1a/R12*(1.-Sum_Xaq)
      X_aQ(i)=Xa
      ProdXaq=ProdXaq*Xa
      Sum_Xaq=Sum_Xaq+Xa

140   R1=RNDM1(-1)
      R2=RNDM1(-1)
      R1a=R1**Alfa
      R2b=R2**Beta
      R12=R1a+R2b
      if(R12.gt.1.)go to 140
      Xq=R1a/R12*(1.-Sum_Xq)
      X_Q(i)=Xq
      ProdXq = ProdXq*Xq
      Sum_Xq = Sum_Xq+ Xq
      
      BetaB=BetaB-ALPHAR
      enddo

      X_aQ(N_QS)=1.-Sum_Xaq
      ProdXaq = ProdXaq*X_aQ(N_QS)

      X_Q(N_QS) = 1.-Sum_Xq
      ProdXq = ProdXq*X_Q(N_QS)

      if(ProdXaq.eq.0.) go to 120
      if(ProdXq .eq.0.) go to 120

      Alfa=0.
      Beta=0.
      do i=1,N_QS
        Alfa=Alfa+Mt_aQ_2(i)/X_aQ(i)
        Beta=Beta+Mt_Q_2(i) /X_Q(i)
      enddo

      if(sqrt(Alfa)+sqrt(Beta).ge.SS) go to 110

      DET=S**2+Alfa**2+Beta**2-2.*S*Alfa-2.*S*Beta-2.*Alfa*Beta
      IF(DET.LT.0.)     GO TO 110
      DET=SQRT(DET)
C
      WA=(S+Alfa-Beta+DET)/2./SS
      WB=(S-Alfa+Beta+DET)/2./SS

      IF((WA.LE.0.).OR.(WB.LE.0.))      GOTO 110

      do i=1,N_QS
        Pz_aQ(i)= (WA*X_aQ(i)-Mt_aQ_2(i)/X_aQ(i)/WA)/2.
        Pz_Q(i) =-(WB*X_Q(i) -Mt_Q_2(i) /X_Q(i) /WB)/2.
      enddo

      SumPx=0.
      SumPy=0.
      SumPz=0.
      do i=1,N_QS
        SumPx=SumPx+Px_aQ(i)
        SumPy=SumPy+Py_aQ(i)
        SumPz=SumPz+Pz_aQ(i)
      enddo

      SumPx=0.
      SumPy=0.
      SumPz=0.
      do i=1,N_QS
        SumPx=SumPx+Px_Q(i)
        SumPy=SumPy+Py_Q(i)
        SumPz=SumPz+Pz_Q(i)
      enddo

 150  CONTINUE

      Nhad=0

      do i=1,N_QS
        PROJ(1)=Typ_aQ(i)
        PROJ(2)=Px_aQ(i)
        PROJ(3)=Py_aQ(i)
        PROJ(4)=Pz_aQ(i)
        PROJ(5)=VQMASS
C
        TAR(1)= Typ_Q(i)
        TAR(2)= Px_Q(i)
        TAR(3)= Py_Q(i)
        TAR(4)= Pz_Q(i)
        TAR(5)= VQMASS

        CALL STRING(PROJ,TAR,NHADm) ! Fragmentation of string

        CALL GOBSEC(Nhad,NHADm) ! Storing of produced particles

      enddo

c-------------------------------------------------------------
c--------- Putting all hadrons on mass-shell -----------------

      Nrepeat=0.
 160  CONTINUE
      Nrepeat=Nrepeat+1
      if(Nrepeat.ge.100) go to 150

      SumMt=0.
      SumaPz=0.

      do i=1,Nhad
        if(AMF(i).le.0.) AMF(i)=-Amass(NREF(i))          ! Uzhi -Amass
        SumMt=SumMt+sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2)
        SumaPz=SumaPz+abs(PZF(i))
      enddo

      if(SumMt.gt.SS) go to 160

      if(SumaPz.le.0.2) then             ! Uzhi ???
 165    SumPz=0.
        do i=1,Nhad
          PZF(i)=RNDM1(-1)
          SumPz=SumPz+PZF(i)
        enddo
        SumPz=SumPz/float(Nhad)

        SumaPz=0.
        do i=1,Nhad
          PZF(i)=PZF(i)-SumPz
          SumaPz=SumaPz+abs(PZF(i))
        enddo
        if(SumaPz.eq.0.) go to 165
      endif

      Coefmax=SS/SumaPz  !SumMt
      Coefmin=0.

 170  Coef=(Coefmin+Coefmax)/2.
      SumMt=0.

      do i=1,Nhad
        SumMt=SumMt+sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2+
     &                     Coef**2*PZF(i)**2)
      enddo

      if(abs(SumMt-SS)/SS.gt.0.001) then
       if(SumMt.gt.SS) then
        Coefmax=Coef
        go to 170
       else
        Coefmin=Coef
        go to 170
       endif
      endif

      SumPx=0.
      SumPy=0.
      SumPz=0.
      SumE=0.
      do i=1,Nhad
       if(AMF(i).lt.0.) AMF(i)=-AMF(i)
       PZF(i)=Coef*PZF(i)
       SumPx=SumPx+PXF(i)
       SumPy=SumPy+PYF(i)
       SumPz=SumPz+PZF(i)
       HEF(i)=Sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2+PZF(i)**2)
       SumE=SumE+HEF(i)
      enddo

      CALL DECAY(Nhad)  ! Decays of the particles

      GO TO 800         ! to the end of the event simulation

 200  CONTINUE
C--------------------------------------- Simulation of process B ------------
C   q \bar q - annihilation, qq - \bar qq string
C----------------------------------------------------------------------------
      Typ_aQ(1)=707
      if(RNDM1(-1).ge.0.33333) Typ_aQ(1)=708

      Typ_Q(1)=Typ_aQ(1)-606

      PROJ(1)=Typ_aQ(1)
      PROJ(2)=0.
      PROJ(3)=0.
      PROJ(4)=Ecms
      PROJ(5)=VQMASS
C
      TAR(1)= Typ_Q(1)
      TAR(2)=0.
      TAR(3)=0.
      TAR(4)=-Ecms
      TAR(5)= VQMASS

 210  continue
      Nhad=0
      CALL STRING(PROJ,TAR,NHADm) ! Fragmentation of string

      if(Nhadm.lt.2) go to 210

      CALL GOBSEC(Nhad,NHADm) ! Storing of produced particles
      SumPx=0.
      SumPy=0.
      SumPz=0.
      SumE=0.
      do i=1,Nhad
       SumPx=SumPx+PXF(i)
       SumPy=SumPy+PYF(i)
       SumPz=SumPz+PZF(i)
       HEF(i)=Sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2+PZF(i)**2)
       SumE=SumE+HEF(i)
      enddo

      CALL DECAY(Nhad)  ! Decays of the particles

      if(Nhad.eq.2) then              ! Check that no p pbar
        if((Nref(1).eq.2.and.Nref(2).eq.1) .or.
     .     (Nref(1).eq.1.and.Nref(2).eq.2)     ) then
          Nhad=0.
          go to 210
        endif
      endif

      GO TO 800   ! to the end of the event simulation

 300  CONTINUE
C--------------------------------------- Simulation of process C ------------
C q \bar q and string-junctions annihilation, 2 q\bar q strings
C----------------------------------------------------------------------------
      N_QS=2

      TypeQ_ann=8
      if(RNDM1(-1).ge.0.33333) TypeQ_ann=7

      if(typeQ_ann.eq.7.) then
c------------------------------------- \bar u \bar d +  u d
        if(RNDM1(-1).le.0.5) then
          Typ_aQ(1)=7
          Typ_aQ(2)=8
        else
          Typ_aQ(1)=8
          Typ_aQ(2)=7
        endif

         if(RNDM1(-1).le.0.5) then
          Typ_Q(1)=1
          Typ_Q(2)=2
        else
          Typ_Q(1)=2
          Typ_Q(2)=1
        endif

      else
c------------------------------------- \bar u \bar u + u u
        Typ_aQ(1)=7
        Typ_aQ(2)=7

        Typ_Q(1)=1
        Typ_Q(2)=1

      endif

      GO TO 109

 400  CONTINUE
C--------------------------------------- Simulation of process E ------------
C           qq \bar qq annihilation, only q\bar q string
C----------------------------------------------------------------------------
      Typ_aQ(1)=8
      if(RNDM1(-1).ge.0.33333) Typ_aQ(1)=7
      Typ_aQ(1)=7

      Typ_Q(1)=Typ_aQ(1)-6

      PROJ(1)=Typ_aQ(1)
      PROJ(2)=0.
      PROJ(3)=0.
      PROJ(4)=Ecms
      PROJ(5)=VQMASS
C
      TAR(1)= Typ_Q(1)
      TAR(2)=0.
      TAR(3)=0.
      TAR(4)=-Ecms
      TAR(5)= VQMASS

      GO TO 210

 500  CONTINUE
C--------------------------------------- Simulation of process G ------------
C  2-string configuration, q \bar q and qq \bar qq
C----------------------------------------------------------------------------
      Typ_aQ(1)=707
      Typ_aQ(2)=  8
      if(RNDM1(-1).ge.0.33333) then
        Typ_aQ(1)=708
        Typ_aQ(2)=  7
      endif

      Typ_Q(1)=101
      Typ_Q(2)=  2
      if(RNDM1(-1).ge.0.33333) then
        Typ_Q(1)=102
        Typ_Q(2)=  1
      endif

      Nrepit=0
      Bslp=Bslop
 510  CONTINUE
      Nrepit=Nrepit+1
      if(mod(Nrepit,50).eq.0) Bslp=Bslp*2.

c ------------------------------------ Determination of Pt_i and X_i -

      N_QS=2

      FI=RNDM1(-1)*6.28318
      Pt=-1./Bslp*ALOG(RNDM1(-1)*RNDM1(-1))
      Px_aQ(1)= Pt*Cos(Fi)
      Py_aQ(1)= Pt*Sin(Fi)
      Px_aQ(2)=-Pt*Cos(Fi)
      Py_aQ(2)=-Pt*Sin(Fi)

      FI=RNDM1(-1)*6.28318
      Pt=-1./Bslp*ALOG(RNDM1(-1)*RNDM1(-1))
      Px_Q(1) = Pt*Cos(Fi)
      Py_Q(1) = Pt*Sin(Fi)
      Px_Q(2) =-Pt*Cos(Fi)
      Py_Q(2) =-Pt*Sin(Fi)

      Sum_Mt=0.

      do i=1,N_QS
        if(Typ_aQ(i).ge.707) then
          Mt_aQ_2(i)=DIQMAS**2+Px_aQ(i)**2+Py_aQ(i)**2
        else
          Mt_aQ_2(i)=VQMASS**2+Px_aQ(i)**2+Py_aQ(i)**2
        endif

        Sum_Mt=Sum_Mt+sqrt(Mt_aQ_2(i))

        if(Typ_Q(i).ge.101) then
          Mt_Q_2(i) =DIQMAS**2+Px_Q(i)**2 +Py_Q(i)**2
        else
          Mt_Q_2(i) =VQMASS**2+Px_Q(i)**2 +Py_Q(i)**2
        endif

        Sum_Mt=Sum_Mt+sqrt(Mt_Q_2(i))
      enddo

      if(Sum_Mt.ge.SS) go to 510

520   CONTINUE

      Alfa=1./ALPHAR
      BetaB=ALPHAR-2.*ALPHAN
      if(Typ_aQ(1).eq.708.) BetaB=BetaB+1.

      Beta=1./(BetaB+1.)
 530  R1=RNDM1(-1)
      R2=RNDM1(-1)
      R1a=R1**Alfa
      R2b=R2**Beta
      R12=R1a+R2b
      if(R12.gt.1.) go to 530
      Xa=R1a/R12
      X_aQ(1)=1.-Xa
      X_aQ(2)=Xa
      if(Xa.eq.0.) go to 530

      Alfa=1./ALPHAR
      BetaB=ALPHAR-2.*ALPHAN
      if(Typ_Q(1).eq.102.) BetaB=BetaB+1.

      Beta=1./(BetaB+1.)
 540  R1=RNDM1(-1)
      R2=RNDM1(-1)
      R1a=R1**Alfa
      R2b=R2**Beta
      R12=R1a+R2b
      if(R12.gt.1.) go to 540
      Xa=R1a/R12
      X_Q(1)=1.-Xa
      X_Q(2)=Xa
      if(Xa.eq.0.) go to 540

      Alfa=0.
      Beta=0.
      do i=1,N_QS
        Alfa=Alfa+Mt_aQ_2(i)/X_aQ(i)
        Beta=Beta+Mt_Q_2(i) /X_Q(i)
      enddo

      if(sqrt(Alfa)+sqrt(Beta).ge.SS) go to 510

      DET=S**2+Alfa**2+Beta**2-2.*S*Alfa-2.*S*Beta-2.*Alfa*Beta
      IF(DET.LT.0.)     GO TO 510
      DET=SQRT(DET)
C
      WA=(S+Alfa-Beta+DET)/2./SS
      WB=(S-Alfa+Beta+DET)/2./SS

      IF((WA.LE.0.).OR.(WB.LE.0.))      GOTO 520

      do i=1,N_QS
        Pz_aQ(i)= (WA*X_aQ(i)-Mt_aQ_2(i)/X_aQ(i)/WA)/2.
        Pz_Q(i) =-(WB*X_Q(i) -Mt_Q_2(i) /X_Q(i) /WB)/2.
      enddo

 550  CONTINUE

      Nhad=0

      SumPx=0.
      SumPy=0.
      SumPz=0.
      do i=1,N_QS
        PROJ(1)=Typ_aQ(i)
        PROJ(2)=Px_aQ(i)
        PROJ(3)=Py_aQ(i)
        PROJ(4)=Pz_aQ(i)
        if(Typ_aQ(i).ge.707.) then
          PROJ(5)=DIQMAS
        else
          PROJ(5)=VQMASS
        endif
C
        TAR(1)= Typ_Q(i)
        TAR(2)= Px_Q(i)
        TAR(3)= Py_Q(i)
        TAR(4)= Pz_Q(i)
        if(Typ_Q(i).ge.101.) then
          TAR(5)= DIQMAS
        else
          TAR(5)= VQMASS
        endif

      SumPx=SumPx+Proj(2)+Tar(2)
      SumPy=SumPy+Proj(3)+Tar(3)
      SumPz=SumPz+Proj(4)+Tar(4)
        CALL STRING(PROJ,TAR,NHADm) ! Fragmentation of string

        CALL GOBSEC(Nhad,NHADm) ! Storing of produced particles

      enddo
c-------------------------------------------------------------
c--------- Putting all hadrons on mass-shell -----------------

      Nrepeat=0.
 560  CONTINUE
      Nrepeat=Nrepeat+1
      if(Nrepeat.ge.100) go to 550

      SumMt=0.
      SumaPz=0.

      do i=1,Nhad
        if(AMF(i).le.0.) AMF(i)=-Amass(NREF(i))     ! Uzhi -Amass
        SumMt=SumMt+sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2)
        SumaPz=SumaPz+abs(PZF(i))
      enddo

      if(SumMt.gt.SS) go to 560

      Coefmax=SS/SumaPz  !SumMt
      Coefmin=0.
 570  Coef=(Coefmin+Coefmax)/2.
      SumMt=0.

      do i=1,Nhad
        SumMt=SumMt+sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2+
     &                     Coef**2*PZF(i)**2)
      enddo

      if(abs(SumMt-SS)/SS.gt.0.001) then
       if(SumMt.gt.SS) then
        Coefmax=Coef
        go to 570
       else
        Coefmin=Coef
        go to 570
       endif
      endif

      SumaPz=0.
      do i=1,Nhad
       PZF(i)=Coef*PZF(i)
       SumaPz=SumaPz+PZF(i)
      enddo

      SumPx=0.
      SumPy=0.
      SumPz=0.
      SumE=0.
      do i=1,Nhad
       if(AMF(i).lt.0.) AMF(i)=-AMF(i)
       SumPx=SumPx+PXF(i)
       SumPy=SumPy+PYF(i)
       SumPz=SumPz+PZF(i)
       HEF(i)=Sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2+PZF(i)**2)
       SumE=SumE+HEF(i)
      enddo

      CALL DECAY(Nhad)  ! Decays of the particles

      GO TO 800   ! to the end of the event simulation

 600  CONTINUE
C--------------------------------------- Simulation of process H ------------
C  Low mass difraction
C----------------------------------------------------------------------------
      Typ_aQ(1)=707
      Typ_aQ(2)=  8
      if(RNDM1(-1).ge.0.33333) then
        Typ_aQ(1)=708
        Typ_aQ(2)=  7
      endif

      N_QS=2

      Nrepit=0
      Bslp=Bslop
 610  CONTINUE
      Nrepit=Nrepit+1
      if(mod(Nrepit,50).eq.0) Bslp=Bslp*2.

C---------------------- Momentum transfer --------------------------
      FI=RNDM1(-1)*6.28318
      Pt=-0.05*ALOG(RNDM1(-1)*RNDM1(-1)) ! 1/2/Bslope
      Px1=Pt*Cos(Fi)
      Py1=Pt*Sin(Fi)

      FI=RNDM1(-1)*6.28318
      Pt=-0.05*ALOG(RNDM1(-1)*RNDM1(-1)) ! 1/2/Bslope
      Px2=Pt*Cos(Fi)
      Py2=Pt*Sin(Fi)

c ------------------------------------ Determination of Pt_i and X_i -
      FI=RNDM1(-1)*6.28318
      Pt=-1./Bslp*ALOG(RNDM1(-1)*RNDM1(-1))
      Px_aQ(1)= Pt*Cos(Fi)+Px1
      Py_aQ(1)= Pt*Sin(Fi)+Py1
      Px_aQ(2)=-Pt*Cos(Fi)+Px2
      Py_aQ(2)=-Pt*Sin(Fi)+Py2

      Sum_Mt=0.

      do i=1,N_QS
        if(Typ_aQ(i).ge.707) then
          Mt_aQ_2(i)=DIQMAS**2+Px_aQ(i)**2+Py_aQ(i)**2
        else
          Mt_aQ_2(i)=VQMASS**2+Px_aQ(i)**2+Py_aQ(i)**2
        endif

        Sum_Mt=Sum_Mt+sqrt(Mt_aQ_2(i))
      enddo

C---------------- Storing of the saved nucleon --------------------
      NhadM=1
      IREF=1
      PXM(1)=-(Px1+Px2)
      PYM(1)=-(Py1+Py2)
      PZM(1)=1
      AMM(1)=AM(IREF)
      HEM(1)=SQRT(AMM(1)**2+PXM(1)**2+PYM(1)**2)

      Sum_Mt=Sum_Mt+HEM(1)

      if(Sum_Mt.ge.SS) go to 610

620   CONTINUE

      Alfa=1./ALPHAR
      BetaB=ALPHAR-2.*ALPHAN
      if(Typ_aQ(1).eq.708.) BetaB=BetaB+1.

      Beta=1./(BetaB+1.)
 625  R1=RNDM1(-1)
      R2=RNDM1(-1)
      R1a=R1**Alfa
      R2b=R2**Beta
      R12=R1a+R2b
      if(R12.gt.1.) go to 625
      Xa=R1a/R12
      X_aQ(1)=1.-Xa
      X_aQ(2)=Xa
      if(Xa.eq.0..or.1.-Xa.eq.0.) go to 625

      Alfa=0.
      do i=1,N_QS
        Alfa=Alfa+Mt_aQ_2(i)/X_aQ(i)
      enddo

      Beta=HEM(1)*HEM(1)

      if(sqrt(Alfa)+sqrt(Beta).ge.SS) go to 620

      DET=S**2+Alfa**2+Beta**2-2.*S*Alfa-2.*S*Beta-2.*Alfa*Beta
      IF(DET.LT.0.)     GO TO 620
      DET=SQRT(DET)
C
      WA=(S+Alfa-Beta+DET)/2./SS
      WB=(S-Alfa+Beta+DET)/2./SS

      IF((WA.LE.0.).OR.(WB.LE.0.))      GOTO 620

      SumPx=0.
      SumPy=0.
      SumPz=0.
      do i=1,N_QS
        Pz_aQ(i)= (WA*X_aQ(i)-Mt_aQ_2(i)/X_aQ(i)/WA)/2.
      SumPx=SumPx+Px_aQ(i)
      SumPy=SumPy+Py_aQ(i)
      SumPz=SumPz+Pz_aQ(i)
      enddo

      PZM(1) =-(WB -HEM(1)**2/WB)/2.
      SumPx=SumPx+PXM(1)
      SumPy=SumPy+PYM(1)
      SumPz=SumPz+PZM(1)

      if(RNDM1(-1).le.0.5) then
c----------------------------------------------- P BAR dissociation
        IREF=1                                   ! P is saved
        HEM(1)=SQRT(AMM(1)**2+PXM(1)**2+PYM(1)**2+PZM(1)**2)
        ICHM(1)=ICH(IREF)
        IBARM(1)=IBAR(IREF)
        ANM(1)=ANAME(IREF)
        NREM(1)=IREF

        PROJ(1)=Typ_aQ(1)
        PROJ(2)=Px_aQ(1)
        PROJ(3)=Py_aQ(1)
        PROJ(4)=Pz_aQ(1)
        PROJ(5)=DIQMAS
C
        TAR(1)= Typ_aQ(2)
        TAR(2)= Px_aQ(2)
        TAR(3)= Py_aQ(2)
        TAR(4)= Pz_aQ(2)
        TAR(5)= VQMASS

      else
c----------------------------------------------- P     dissociation
        IREF=2                                   ! P BAR is saved
        HEM(1)=SQRT(AMM(1)**2+PXM(1)**2+PYM(1)**2+PZM(1)**2)
        ICHM(1)=ICH(IREF)
        IBARM(1)=IBAR(IREF)
        ANM(1)=ANAME(IREF)
        NREM(1)=IREF
        PZM(1)=-PZM(1)

        PROJ(1)= Typ_aQ(1)-606.
        PROJ(2)= Px_aQ(1)
        PROJ(3)= Py_aQ(1)
        PROJ(4)=-Pz_aQ(1)
        PROJ(5)= DIQMAS
C
        TAR(1)=  Typ_aQ(2)-6.
        TAR(2)=  Px_aQ(2)
        TAR(3)=  Py_aQ(2)
        TAR(4)= -Pz_aQ(2)
        TAR(5)=  VQMASS
      endif

      Nhad=0
      CALL GOBSEC(Nhad,NHADm) ! Storing of saved baryon

 630  CONTINUE

      CALL STRING(PROJ,TAR,NHADm) ! Fragmentation of string

      IF((Nhadm.eq.1).and.((Nrem(1).eq.1).or.(Nrem(1).eq.2))) go to 630
      CALL GOBSEC(Nhad,NHADm) ! Storing of produced particles
c-------------------------------------------------------------
c--------- Putting all hadrons on mass-shell -----------------

      Nrepeat=0.
 640  CONTINUE
      Nrepeat=Nrepeat+1
      if(Nrepeat.ge.100) go to 630

      SumMt=0.
      SumaPz=0.

      do i=1,Nhad
        if(AMF(i).le.0.) AMF(i)=-Amass(NREF(i))          ! Uzhi -Amass
        SumMt=SumMt+sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2)
        SumaPz=SumaPz+abs(PZF(i))
      enddo

      if(SumMt.gt.SS) go to 640

      Coefmax=SS/SumaPz  !SumMt
      Coefmin=0.
 650  Coef=(Coefmin+Coefmax)/2.
      SumMt=0.

      do i=1,Nhad
        SumMt=SumMt+sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2+
     &                     Coef**2*PZF(i)**2)
      enddo

      if(abs(SumMt-SS)/SS.gt.0.001) then
       if(SumMt.gt.SS) then
        Coefmax=Coef
        go to 650
       else
        Coefmin=Coef
        go to 650
       endif
      endif

      SumaPz=0.
      do i=1,Nhad
       PZF(i)=Coef*PZF(i)
       SumaPz=SumaPz+PZF(i)
      enddo

      SumPx=0.
      SumPy=0.
      SumPz=0.
      SumE=0.
      do i=1,Nhad
       if(AMF(i).lt.0.) AMF(i)=-AMF(i)
       SumPx=SumPx+PXF(i)
       SumPy=SumPy+PYF(i)
       SumPz=SumPz+PZF(i)
       HEF(i)=Sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2+PZF(i)**2)
       SumE=SumE+HEF(i)
      enddo

      CALL DECAY(Nhad)  ! Decays of the particles

      GO TO 800   ! to the end of the event simulation

 700  CONTINUE
C############################################################################
C                   Simulation of elastic scattering
C############################################################################
c      print *, 'started 700', prob_col, prob_int

      Pcms=sqrt(Ecms**2-AMProton**2)
      X0=RNDM1(-1)

      imode=0
      
c      print *, X0

      IF(X0.LE.prob_col) then 
! modeling colomb part
c       print *, 'now is modeling colomb part'
*1100    T=Tmin/(RNDM1(-1))              !abs(Tmin) ili net?
c1100     T=Tmin/(1.-RNDM1(-1)*(1-Tmin/Tmax) )
1100  TLRA=RNDM1(-1)
c      print *, Tmin, Tmax
c      T=Tmin+TLRA*(Tmax-Tmin)
      T=Tmin/(1.-TLRA*(1.-Tmin/Tmax))
c      print *, "dpm current", Tmin, T
c      T=Tmin/sqrt(1.-TLRA*(1.-(Tmin/Tmax)**2))
c      print *, "mine", Tmin, T
      imode=1

      Gbrak4=1./((1.+abs(T)/0.71)**8)
      TTRANDO=RNDM1(-1)
c      print *, TTRANDO, Gbrak4
      if(TTRANDO.gt.Gbrak4) goto 1100 
      
      endif !end if coulomb part
c      print *, T, Tmin, Tmax
       
      IF(X0.LE.(prob_col+prob_int) .and. X0.GT.prob_col) then 
! modeling interf part
c         print *, 'now is modeling interf part'
*1200   T=2./parB * log(RNDM1(-1))+Tmin
1200    TLRA=RNDM1(-1)
        T=2./parB*log(exp(parB*Tmin/2.)-TLRA*
     &  (exp(parB*Tmin/2.) - exp(parB*Tmax/2.) ) )

        imode=2
      PBrac=abs(Tmin)/abs(T)*1./((1.+abs(T)/0.71)**4)
      if(RNDM1(-1).gt.PBrac) goto 1200
      endif

      IF(X0.GT.(prob_col+prob_int)) then   !modeling hadron part
c         print *, 'now is modelin hadron part' 
      
      imode=3

      IF(RNDM1(-1).le.Weight1) then    !model hadron elast 
c         T=T2*ALOG(1.-RNDM1(-1)*(1.-exp(Tmax/T2))) !Last exponent
      TLRA=RNDM1(-1)

      T=T2*ALOG(exp(Tmin/T2)-
     & TLRA*(exp(Tmin/T2)-exp(Tmax/T2))) ! Last exponent

      ELSE
c        W1=A1*T1*(1.-exp(Tmax/T1))/
c     /  (A1*T1*(1.-exp(Tmax/T1))+A1*A2**2*T2*(1.-exp(Tmax/T2)))

      W1=A1*T1*(exp(Tmin/T1)-exp(Tmax/T1))/
     &  (A1*T1*(exp(Tmin/T1)-exp(Tmax/T1))+
     &   A1*A2**2*T2*(exp(Tmin/T2)-exp(Tmax/T2)))

 710    continue
      IF(RNDM1(-1).le.W1) then
c      T=T1*ALOG(1.-RNDM1(-1)*(1.-exp(Tmax/T1))) ! First exponent
      TLRA=RNDM1(-1)
      T=T1*ALOG(exp(Tmin/T1)-
     &   TLRA*(exp(Tmin/T1)-exp(Tmax/T1))) ! First exponent
      ELSE
c           T=T2*ALOG(1.-RNDM1(-1)*(1.-exp(Tmax/T2))) ! Second exponent
         TLRA=RNDM1(-1)
         T=T2*ALOG(exp(Tmin/T2)-
     &   TLRA*(exp(Tmin/T2)-exp(Tmax/T2))) ! Second exponent
      ENDIF
      W2=(exp(T/2./T1)-A2*exp(T/2./T2))**2/(exp(T/T1)+A2**2*exp(T/T2))
      IF(RNDM1(-1).gt.W2) GO TO 710
      ENDIF
      
      endif      !end if modeling hadron part

*      print *, ' calculation of brac factor'
      Fp=((1+abs(T)/0.71)**(-2))
      DeltaT=abs(T)/0.71
      pkoef1=10./(5.0677**2)          !/10. naoborot
      DSIG_C=4*3.1416*aelm**2*(Fp**4)*pkoef1/((betav * T)**2)

      delt=aelm*(0.577+log(parB*abs(T)/2.+ 4*DeltaT)+
     & 4*DeltaT*log(4*DeltaT) + 2*DeltaT) 
      
      DSIG_I=aelm*sigma_tot*(Fp**2)*exp(0.5*parB*T)*
     & (rho*cos(delt)+sin(delt)) / betav/ abs(T)   

       DSIG_IM=aelm*sigma_tot*(Fp**2)*exp(0.5*parB*T)*
     & (1+rho**2) / betav/ abs(T)
       
       DSIG_H=A1*((exp(T/2./T1)-A2*exp(T/2./T2))**2) + 
     &  A3*exp(T/T2)

      ds_dt  = DSIG_C+DSIG_I +DSIG_H
      ds_dtM = DSIG_C+DSIG_IM + DSIG_H
       IF(RNDM1(-1).GT.(ds_dt/ds_dtM))goto 700
*      print*, 'T ', T 

 720    continue
c      if(imode.eq.1) then
c        goto 721
c      else 
c        goto 722
c      endif
c
c 721    continue
c      if(T.gt.-0.00001) then
c        T=-TLRA
c      print *, T
c      else
c 722    continue
c        T=-1.2
c      endif
c      print *, TLRA, imode
c      print *, T
      TTR=T
      Cos_Theta=1.+T/2./Pcms**2
      Sin_Theta=sqrt(ABS(1.-Cos_Theta**2))
      Pz=Pcms*Cos_Theta
      Pt=Pcms*Sin_Theta
*      print*, ' Pz', Pz, ' pt', Pt
*      read (5,*)ccc

      FI=RNDM1(-1)*6.28318
c                                  Storing of Pbar
      NhadM=1
      IREF=2
      PXM(1)=Pt*Sin(Fi)
      PYM(1)=Pt*Cos(Fi)
      PZM(1)=Pz
      AMM(1)=AM(IREF)
      HEM(1)=SQRT(AMM(1)**2+PXM(1)**2+PYM(1)**2+PZM(1)**2)
      ICHM(1)=ICH(IREF)
      IBARM(1)=IBAR(IREF)
      ANM(1)=ANAME(IREF)
      NREM(1)=IREF
c                                  Storing of P
      NhadM=2
      IREF=1                                   
      PXM(2)=-PXM(1)
      PYM(2)=-PYM(1)
      PZM(2)=-PZM(1)
      AMM(2)=AM(IREF)
      HEM(2)=SQRT(AMM(1)**2+PXM(1)**2+PYM(1)**2+PZM(1)**2)
      ICHM(2)=ICH(IREF)
      IBARM(2)=IBAR(IREF)
      ANM(2)=ANAME(IREF)
      NREM(2)=IREF

      Nhad=0
      CALL GOBSEC(Nhad,NHADm) ! Storing of baryon
      CALL DECAY(Nhad) 

      GO TO 800   ! to the end of the event simulation 
c-------------------------------------------------------------
 800    CONTINUE
c-------------- End of event simulation ----------------------


      SumPx=0.
      SumPy=0.
      SumPz=0.
      SumE=0.
      do i=1,Nhad
c      Print *, i, NREF(i), PXF(i), PYF(i), PZF(i), Vcms, HEF(i), Gamma
       PZF(i)=(PZF(i)+Vcms*HEF(i))*Gamma   ! Uzhi
       SumPx=SumPx+PXF(i)
       SumPy=SumPy+PYF(i)
       SumPz=SumPz+PZF(i)
       HEF(i)=Sqrt(AMF(i)**2+PXF(i)**2+PYF(i)**2+PZF(i)**2)
       p2=sqrt(PXF(i)**2+PYF(i)**2+PZF(i)**2)
c      Print *, i, NREF(i), PXF(i), PYF(i), PZF(i), p2, HEF(i), Gamma
       SumE=SumE+HEF(i)    
      enddo

      RETURN
      END

      SUBROUTINE TOPITH(Nhad)
C =========================================================================
      COMMON /LUJETS/ N,K(1000,2),P(1000,5)
C =========================================================================
      COMMON/FINPAR/PXF(10000),PYF(10000),PZF(10000),HEF(10000),
     *AMF(10000),ICHF(10000),IBARF(10000),ANF(10000),NREF(10000)

      CHARACTER*8 ANF
C
      DIMENSION ID1(102), ID2(78), IDPITH(180)
      DATA ID1/
     1   2212, -2212,    11,   -11,    12,   -12,    22,  2112, -2112,
     2    -13,    13,   130,   211,  -211,   321,  -321,  3122, -3122,
     3    310,  3122,  3222,  3212,   111,   311,  -311,   5*0,   221,
     4    213,   113,  -213,   223,   323,   313,  -323,  -313,  13*0,
     5   2224,  2214,  2114,  1114,  10*0, -2224, -2214, -2114, -1114,
     6   24*0,   331,   333,  3322,  3312, -3222, -3212, -3122, -3322/

      DATA ID2/
     1 -3312,  3224,  3214,  3114,  3324,  3314,  3334, -3224, -3214,
     2 -3114, -3324, -3314, -3334,   421,   411,  -411,  -421,   431,
     3  -431,   441,   423,   413,  -413,  -423,   433,  -433,     0,
     4   443,   -15,    15,    16,   -16,    14,   -14,  4122,  4232,
     5  4132,  4222,  4212,  4112,  4214,  4114,  4312,  4224,  34*0/

      DATA INIT/0/
      SAVE ID1, ID2, INIT, IDPITH

      IF(INIT.EQ.0) THEN
        DO I=1, 102
          IDPITH(I)=ID1(I)
        ENDDO

        DO I=103,180
          J=I-102
          IDPITH(I)=ID2(J)
        ENDDO
        INIT=1
      ENDIF

      N=NHAD
      DO I=1,NHAD
        K(I,1)=0
        K(I,2)=IDPITH(NREF(I))

        P(I,1)=PXF(I)
        P(I,2)=PYF(I)
        P(I,3)=PZF(I)
        P(I,4)=HEF(I)
        P(I,5)=AMF(I)
c        print *, I, K(I,2), P(I,1), P(I,2), P(I,3), P(I,4), P(I,5)
      ENDDO
      END

      SUBROUTINE TOPLUTO(Nhad)
C =========================================================================
      COMMON /LUJETS/ N,K(1000,2),P(1000,5)
C =========================================================================
      COMMON/FINPAR/PXF(10000),PYF(10000),PZF(10000),HEF(10000),
     *AMF(10000),ICHF(10000),IBARF(10000),ANF(10000),NREF(10000)

      CHARACTER*8 ANF
C
      DIMENSION ID1(102), ID2(78), IDPITH(180)
      DATA ID1/
     1  14, 15,  3,  2,  4,  4,  1, 13, 25, 5, 6, 10, 8,
     2   9, 11, 12, 18, 26, 16, 21, 19, 20, 80*0/

      DATA ID2/78*0/

      DATA INIT/0/
      SAVE ID1, ID2, INIT, IDPITH

      IF(INIT.EQ.0) THEN
        DO I=1, 102
          IDPITH(I)=ID1(I)
        ENDDO

        DO I=103,180
          J=I-102
          IDPITH(I)=ID2(J)
        ENDDO
        INIT=1
      ENDIF

      N=NHAD
      DO I=1,NHAD
        K(I,1)=0
        K(I,2)=IDPITH(NREF(I))

        P(I,1)=PXF(I)
        P(I,2)=PYF(I)
        P(I,3)=PZF(I)
        P(I,4)=HEF(I)
        P(I,5)=AMF(I)
      ENDDO
      END
