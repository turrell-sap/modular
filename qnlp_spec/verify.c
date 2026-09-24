/* Reference checks for the QNLP spec. Little-endian: qubit j = bit j. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>
typedef double complex cd;
#define MAXW 6
static void apply_1q(cd*psi,int W,int k,cd u00,cd u01,cd u10,cd u11){
  unsigned long np=1ul<<(W-1), lowmask=(1ul<<k)-1;
  for(unsigned long j=0;j<np;j++){unsigned long i0=((j>>k)<<(k+1))|(j&lowmask), i1=i0|(1ul<<k);
    cd a=psi[i0],b=psi[i1]; psi[i0]=u00*a+u01*b; psi[i1]=u10*a+u11*b;}
}
static void Rx(cd*p,int W,int k,double t){double c=cos(t/2),s=sin(t/2);apply_1q(p,W,k,c,-I*s,-I*s,c);}
static void Ry(cd*p,int W,int k,double t){double c=cos(t/2),s=sin(t/2);apply_1q(p,W,k,c,-s,s,c);}
static void Rz(cd*p,int W,int k,double t){apply_1q(p,W,k,cexp(-I*t/2),0,0,cexp(I*t/2));}
static void H(cd*p,int W,int k){double r=1/sqrt(2);apply_1q(p,W,k,r,r,r,-r);}
static void CRz(cd*p,int W,int c,int t,double th){ /* control c, target t */
  for(unsigned long i=0;i<(1ul<<W);i++) if((i>>c)&1) p[i]*= ((i>>t)&1)? cexp(I*th/2): cexp(-I*th/2);
}
static void CNOT(cd*p,int W,int c,int t){
  for(unsigned long i=0;i<(1ul<<W);i++) if(((i>>c)&1) && !((i>>t)&1)){cd tmp=p[i];p[i]=p[i|(1ul<<t)];p[i|(1ul<<t)]=tmp;}
}
/* IQP word state on Q qubits (no closing H), L layers, angles th[] ; Q=1: Rx Rz Rx */
static void iqp(cd*p,int Q,int L,const double*th){
  memset(p,0,sizeof(cd)<<Q); p[0]=1;
  if(Q==1){Rx(p,1,0,th[0]);Rz(p,1,0,th[1]);Rx(p,1,0,th[2]);return;}
  for(int l=0;l<L;l++){for(int j=0;j<Q;j++)H(p,Q,j); for(int j=0;j<Q-1;j++)CRz(p,Q,j,j+1,th[l*(Q-1)+j]);}
}
/* N TV N, q_n=q_s=1: sigma[s]=sum_ab n1[a] V[a+2s+4b] n2[b] */
static void sent_tvn(const double*a,const double*v,const double*b,int L,cd*sig){
  cd n1[2],n2[2],V[8]; iqp(n1,1,1,a); iqp(n2,1,1,b); iqp(V,3,L,v);
  for(int s=0;s<2;s++){sig[s]=0;for(int x=0;x<2;x++)for(int y=0;y<2;y++)sig[s]+=n1[x]*V[x+2*s+4*y]*n2[y];}
}
static void sent_iv(const double*a,const double*v,int L,cd*sig){
  cd n1[2],V[4]; iqp(n1,1,1,a); iqp(V,2,L,v);
  for(int s=0;s<2;s++){sig[s]=0;for(int x=0;x<2;x++)sig[s]+=n1[x]*V[x+2*s];}
}
static void probs(const cd*sig,double*p,double*Z){*Z=cabs(sig[0])*cabs(sig[0])+cabs(sig[1])*cabs(sig[1]);p[0]=cabs(sig[0])*cabs(sig[0])/(*Z);p[1]=1-p[0];}
static double loss_tvn(const double*th,int y){cd sig[2];double p[2],Z;sent_tvn(th,th+6,th+3,1,sig);probs(sig,p,&Z);return -log(p[y]);}
static double p0_tvn(const double*th){cd sig[2];double p[2],Z;sent_tvn(th,th+6,th+3,1,sig);probs(sig,p,&Z);return p[0];}
static double N_tvn(const double*th,int s){cd sig[2];sent_tvn(th,th+6,th+3,1,sig);return cabs(sig[s])*cabs(sig[s]);}
int main(){
  double alice[3]={0.3,0.7,1.1}, bob[3]={0.5,-0.4,0.9}, loves[2]={0.8,-0.6}, sleeps[1]={0.8};
  cd sig[2];double p[2],Z;
  printf("== word states ==\n");
  cd n[2]; iqp(n,1,1,alice); printf("Alice: [%.8f%+.8fi, %.8f%+.8fi]\n",creal(n[0]),cimag(n[0]),creal(n[1]),cimag(n[1]));
  iqp(n,1,1,bob); printf("Bob  : [%.8f%+.8fi, %.8f%+.8fi]\n",creal(n[0]),cimag(n[0]),creal(n[1]),cimag(n[1]));
  cd V[8]; iqp(V,3,1,loves); for(int k=0;k<8;k++)printf("loves[%d]=%.8f%+.8fi  |.|=%.8f arg=%.8f\n",k,creal(V[k]),cimag(V[k]),cabs(V[k]),carg(V[k]));
  cd IV[4]; iqp(IV,2,1,sleeps); for(int k=0;k<4;k++)printf("sleeps[%d]=%.8f%+.8fi\n",k,creal(IV[k]),cimag(IV[k]));
  printf("== W=2: Alice sleeps ==\n");
  sent_iv(alice,sleeps,1,sig); probs(sig,p,&Z);
  printf("sigma=[%.8f%+.8fi, %.8f%+.8fi] Z=%.8f p=[%.8f, %.8f]\n",creal(sig[0]),cimag(sig[0]),creal(sig[1]),cimag(sig[1]),Z,p[0],p[1]);
  double z1[3]={0,0,0},z0[1]={0}; sent_iv(z1,z0,1,sig);probs(sig,p,&Z);printf("all-zero: sigma=[%.8f%+.8fi,%.8f%+.8fi] Z=%.8f p0=%.8f\n",creal(sig[0]),cimag(sig[0]),creal(sig[1]),cimag(sig[1]),Z,p[0]);
  printf("== W=3: Alice loves Bob ==\n");
  sent_tvn(alice,loves,bob,1,sig); probs(sig,p,&Z);
  printf("sigma=[%.8f%+.8fi, %.8f%+.8fi] Z=%.8f p=[%.8f, %.8f]\n",creal(sig[0]),cimag(sig[0]),creal(sig[1]),cimag(sig[1]),Z,p[0],p[1]);
  printf("loss(y=0)=%.8f loss(y=1)=%.8f\n",-log(p[0]),-log(p[1]));
  double z2[2]={0,0}; sent_tvn(z1,z2,z1,1,sig);probs(sig,p,&Z);printf("all-zero: sigma=[%.8f,%.8f] Z=%.8f p0=%.8f\n",creal(sig[0]),creal(sig[1]),Z,p[0]);
  /* bent-wire W=3 circuit check: V on q0..2; U_Alice^T on q0 = Rx(t2),Rz(t1),Rx(t0) in list order; U_Bob^T on q2; read amps at index 2s */
  {cd psi[8]; iqp(psi,3,1,loves);
   Rx(psi,3,0,alice[2]);Rz(psi,3,0,alice[1]);Rx(psi,3,0,alice[0]);
   Rx(psi,3,2,bob[2]);Rz(psi,3,2,bob[1]);Rx(psi,3,2,bob[0]);
   sent_tvn(alice,loves,bob,1,sig);
   printf("bent-wire: psi[0]=%.8f%+.8fi psi[2]=%.8f%+.8fi  diff=%.2e %.2e\n",creal(psi[0]),cimag(psi[0]),creal(psi[2]),cimag(psi[2]),cabs(psi[0]-sig[0]),cabs(psi[2]-sig[1]));
   double Ppost=0; for(int s=0;s<2;s++)Ppost+=cabs(psi[2*s])*cabs(psi[2*s]); printf("P_post(bent)=%.8f\n",Ppost);}
  /* monolithic W=5: Alice q0, verb q1..3 (n^r q1, s q2, n^l q3), Bob q4; cups (0,1),(3,4) as delta and as Bell(CNOT,H,<00|) */
  {cd psi[32]; memset(psi,0,sizeof psi); cd n1[2],n2[2]; iqp(n1,1,1,alice);iqp(n2,1,1,bob);iqp(V,3,1,loves);
   for(int i=0;i<32;i++){int a=i&1,v=(i>>1)&7,b=(i>>4)&1; psi[i]=n1[a]*V[v]*n2[b];}
   cd sd[2]={0,0}; for(int k1=0;k1<2;k1++)for(int k2=0;k2<2;k2++)for(int s=0;s<2;s++) sd[s]+=psi[k1|(k1<<1)|(s<<2)|(k2<<3)|(k2<<4)];
   printf("delta-cup W=5: sigma=[%.8f%+.8fi,%.8f%+.8fi]\n",creal(sd[0]),cimag(sd[0]),creal(sd[1]),cimag(sd[1]));
   CNOT(psi,5,0,1);H(psi,5,0);CNOT(psi,5,3,4);H(psi,5,3);
   printf("Bell-cup  W=5: psi[0]=%.8f%+.8fi psi[4]=%.8f%+.8fi (=sigma/2)  P_post=%.8f (=Z/4)\n",creal(psi[0]),cimag(psi[0]),creal(psi[4]),cimag(psi[4]),cabs(psi[0])*cabs(psi[0])+cabs(psi[4])*cabs(psi[4]));}
  /* gradients of p0 and CE loss (y=0) wrt 8 params: [alice0..2, bob0..2, loves0..1] by central FD */
  double th[8]={0.3,0.7,1.1,0.5,-0.4,0.9,0.8,-0.6}; double h=1e-6;
  printf("== gradients (central FD, h=1e-6) param order: Alice t0 t1 t2, Bob t0 t1 t2, loves t0 t1 ==\n");
  for(int k=0;k<8;k++){double tp[8],tm[8];memcpy(tp,th,sizeof th);memcpy(tm,th,sizeof th);tp[k]+=h;tm[k]-=h;
    printf("k=%d dp0=%.7f dLoss(y=0)=%.7f\n",k,(p0_tvn(tp)-p0_tvn(tm))/(2*h),(loss_tvn(tp,0)-loss_tvn(tm,0))/(2*h));}
  /* parameter-shift checks: 2-term on Alice t0 (Rx), 4-term on loves t0 (CRz), quotient rule */
  {int k=0; double tp[8],tm[8];memcpy(tp,th,sizeof th);memcpy(tm,th,sizeof th);tp[k]+=M_PI/2;tm[k]-=M_PI/2;
   double N0=N_tvn(th,0),N1=N_tvn(th,1),D=N0+N1; double dN0=0.5*(N_tvn(tp,0)-N_tvn(tm,0)),dN1=0.5*(N_tvn(tp,1)-N_tvn(tm,1)),dD=dN0+dN1;
   printf("2-term shift Rx (Alice t0): dN0=%.7f dD=%.7f dp0=(dN0*D-N0*dD)/D^2=%.7f  naive-shift-on-p0=%.7f\n",dN0,dD,(dN0*D-N0*dD)/(D*D),0.5*(p0_tvn(tp)-p0_tvn(tm)));}
  {int k=6; double d1=(sqrt(2)+1)/(4*sqrt(2)), d2=-(sqrt(2)-1)/(4*sqrt(2));
   double N0=N_tvn(th,0),N1=N_tvn(th,1),D=N0+N1; double dN[2];
   for(int s=0;s<2;s++){double t1[8],t2[8],t3[8],t4[8];memcpy(t1,th,sizeof th);memcpy(t2,th,sizeof th);memcpy(t3,th,sizeof th);memcpy(t4,th,sizeof th);
     t1[k]+=M_PI/2;t2[k]-=M_PI/2;t3[k]+=3*M_PI/2;t4[k]-=3*M_PI/2;
     dN[s]=d1*(N_tvn(t1,s)-N_tvn(t2,s))+d2*(N_tvn(t3,s)-N_tvn(t4,s));}
   double dD=dN[0]+dN[1]; double t1[8],t2[8];memcpy(t1,th,sizeof th);memcpy(t2,th,sizeof th);t1[k]+=M_PI/2;t2[k]-=M_PI/2;
   double two=0.5*(N_tvn(t1,0)-N_tvn(t2,0));
   printf("4-term shift CRz (loves t0): dN0=%.7f (2-term would give %.7f) dp0=%.7f  d1=%.6f d2=%.6f\n",dN[0],two,(dN[0]*D-N0*dD)/(D*D),d1,d2);}
  /* CRz decomposition check and CRx = Rx CZ Rx CZ */
  {cd a[4],b[4]; for(int i=0;i<4;i++){a[i]=b[i]=(0.3+0.1*i)+I*(0.2-0.15*i);} double th0=1.3;
   CRz(a,2,0,1,th0); CNOT(b,2,0,1);Rz(b,2,1,-th0/2);CNOT(b,2,0,1);Rz(b,2,1,th0/2);
   double d=0;for(int i=0;i<4;i++)d+=cabs(a[i]-b[i]); printf("CRz decomposition residual=%.2e\n",d);}
  /* periodicity: loss at loves t0 + 2pi vs +4pi */
  {double t1[8],t2[8];memcpy(t1,th,sizeof th);memcpy(t2,th,sizeof th);t1[6]+=2*M_PI;t2[6]+=4*M_PI;
   printf("CRz periodicity: p0(th)=%.8f p0(th+2pi)=%.8f p0(th+4pi)=%.8f\n",p0_tvn(th),p0_tvn(t1),p0_tvn(t2));
   memcpy(t1,th,sizeof th);t1[0]+=2*M_PI;printf("Rx periodicity: p0(th+2pi on Alice t0)=%.8f\n",p0_tvn(t1));}
  /* index helpers */
  {int W=6; for(int a=0;a<W;a++)for(int b=a+1;b<W;b++){int ok=1;for(unsigned long j=0;j<(1ul<<(W-2));j++){unsigned long t=((j>>a)<<(a+1))|(j&((1ul<<a)-1)),i=((t>>b)<<(b+1))|(t&((1ul<<b)-1));
      unsigned long c=((i>>(b+1))<<(b-1))|(((i>>(a+1))&((1ul<<(b-a-1))-1))<<a)|(i&((1ul<<a)-1)); if(c!=j||i<j||((i>>a)&1)||((i>>b)&1))ok=0;} if(!ok)printf("compact FAIL a=%d b=%d\n",a,b);} printf("compact()/i00() verified W=6 all a<b\n");}
  return 0;
}
