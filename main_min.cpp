#include <cstdio>  // cray_ray - a very silly ray tracer
#include <cmath>   // tom milsom | github.com/colourblind | monochromcy.net
#include <cstdlib> // lighting, reflections, fog
#define op operator
#define re return
typedef unsigned char uc; typedef float f; typedef int i;uc B[]={66,77,29,250,
2,0,0,0,0,0,26,0,0,0};uc D[]={12,0,0,0,0,2,0,2,1,0,24,0};i S[]={0x80808003,
0x84838202,0x79857c05,0x817e8301,0x84867409,0x767f8003,0x7d7d8401};f C(f x){re
x<0?0:x>1?1:x;}f R(){re (f)rand()/RAND_MAX;}struct v{f x,y,z;v(){x=y=z=0;}v(f 
a,f b,f c){x=a,y=b,z=c;}v op+(v a){re v(x+a.x,y+a.y,z+a.z);}v op-(v a){re v(x-
a.x,y-a.y,z-a.z);}v op*(f s){re v(x*s,y*s,z*s);}f op^(v a){re x*a.x+y*a.y+z*a.
z;}v op%(v a){re v(y*a.z-z*a.y,z*a.x-x*a.z,x*a.y-y*a.x);}v op~(){re*this*(1.f/
sqrtf(*this^*this));}v lerp(v a, f f){re v(x*f+a.x*(1-f),y*f+a.y*(1-f),z*f+a.z
*(1-f));}void s(f a,f b,f c){x=a;y=b;z=c;}void s(v v){x=v.x;y=v.y;z=v.z;}};f H
(v o,v k,i z,v *c){f F=30.f,d=-1,g,h,u,w;i j;v t,a,n,l,r,p(1,12,3);if(z<1)re F
;for(j=0;j<7;j++){l=v((f)(S[j]>>24&255)-128,(f)(S[j]>>16&255)-128,(f)(S[j]>>8&
255)-128);g=S[j]&0xff;r=(o-l);h=k^r;u=(h*h)-(r^r)+(g*g);g=-h+sqrtf(u);w=-h-
sqrtf(u);if(g>=0||w>=0){u=g<w?g:w;if(d<0||u<d){d=u;t=o+(k*d);n=~(t-l);}}}if(d
>=0){H(t+n*0.001f,n,z-1,&a);a=a*0.8f;l=~(p-t);r=~(n*(n^l)*-2+l);w=pow(C(r^k),
80)*255;a=a+v(w,w,w);}else{n=v(0,1,0);d=((v(0,-4,0)-o)^n)/(k^n);if(d>0){t=o+k*
d;j=abs((int)ceil(t.x))%2==abs((int)ceil(t.z))%2;a.s(j?0:255,j?0:255,j?255:0
);l=~(p-t);w=H(t,l,1,&r)<F?0.4:1;a=a*(l^n)*w;}else{a.s(255,0,0);d=F;}}c->s(a.
lerp(v(128,0,0),((F-(d>F?F:d))/F)));re d;}void main(){uc d[786432],*p=d;v b(0,
0,-1),c,a,e;i x,y,z;for(y=0;y<512;y++){for (x=0;x<512;x++){a=v();for(z=0;z<16;
z++){e=v(b.x+(x-256+R())*0.0025f,b.y+(y-256+R())*0.0025f,b.z);H(v(0,0,15),~e,
99,&c);a=a+c*(1.f/16);}*(p++)=(uc)(a.x>255?255:a.x);*(p++)=(uc)(a.y>255?255:a.
y);*(p++)=(uc)(a.z>255?255:a.z);}}FILE *f=fopen("o.bmp","wb");fwrite(B,14,1,f)
;fwrite(D,12,1,f);fwrite(d,786432,1,f);fclose(f);}