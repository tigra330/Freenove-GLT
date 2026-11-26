// PC-Vorschau: rendert RLT-Ansicht, Dialoge, Demo-Szenarien und Galerie als PNG.
// Simuliert Touch-Eingaben (Koordinaten passend zum Layout in src/ui/rlt_screen.cpp).
#include <lvgl.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include "model/demo_source.h"
#include "ui/app.h"
#include "ui/ui_common.h"
static uint16_t fb[800*480];
static uint32_t now=0;
static int tx=-1,ty=-1;
static void flush(lv_display_t* d,const lv_area_t* a,uint8_t* px){
  uint16_t* p=(uint16_t*)px; for(int y=a->y1;y<=a->y2;y++)for(int x=a->x1;x<=a->x2;x++)fb[y*800+x]=*p++;
  lv_display_flush_ready(d);}
static void touch(lv_indev_t*,lv_indev_data_t* d){ if(tx>=0){d->point.x=tx;d->point.y=ty;d->state=LV_INDEV_STATE_PRESSED;} else d->state=LV_INDEV_STATE_RELEASED;}
static uint32_t crcT[256];
static uint32_t crc(const uint8_t* b,size_t n,uint32_t c=0xffffffff){for(size_t i=0;i<n;i++)c=crcT[(c^b[i])&255]^(c>>8);return c;}
static void chunk(FILE* f,const char* t,const std::vector<uint8_t>& d){uint8_t l[4]={(uint8_t)(d.size()>>24),(uint8_t)(d.size()>>16),(uint8_t)(d.size()>>8),(uint8_t)d.size()};fwrite(l,1,4,f);std::vector<uint8_t> b(t,t+4);b.insert(b.end(),d.begin(),d.end());fwrite(b.data(),1,b.size(),f);uint32_t c=crc(b.data(),b.size())^0xffffffff;uint8_t cc[4]={(uint8_t)(c>>24),(uint8_t)(c>>16),(uint8_t)(c>>8),(uint8_t)c};fwrite(cc,1,4,f);}
static void png(const char* fn){
  for(uint32_t n=0;n<256;n++){uint32_t c=n;for(int k=0;k<8;k++)c=c&1?0xedb88320^(c>>1):c>>1;crcT[n]=c;}
  std::vector<uint8_t> raw; for(int y=0;y<480;y++){raw.push_back(0);for(int x=0;x<800;x++){uint16_t v=fb[y*800+x];raw.push_back(((v>>11)&31)*255/31);raw.push_back(((v>>5)&63)*255/63);raw.push_back((v&31)*255/31);}}
  std::vector<uint8_t> z={0x78,0x01}; size_t pos=0; uint32_t a=1,b=0; for(uint8_t c:raw){a=(a+c)%65521;b=(b+a)%65521;}
  while(pos<raw.size()){size_t n=std::min<size_t>(65535,raw.size()-pos);z.push_back(pos+n==raw.size());z.push_back(n&255);z.push_back(n>>8);z.push_back(~n&255);z.push_back((~n>>8)&255);z.insert(z.end(),raw.begin()+pos,raw.begin()+pos+n);pos+=n;}
  uint32_t ad=(b<<16)|a; z.push_back(ad>>24);z.push_back(ad>>16);z.push_back(ad>>8);z.push_back(ad);
  FILE* f=fopen(fn,"wb"); const uint8_t sig[8]={137,80,78,71,13,10,26,10}; fwrite(sig,1,8,f);
  std::vector<uint8_t> h={0,0,3,32,0,0,1,224,8,2,0,0,0}; chunk(f,"IHDR",h); chunk(f,"IDAT",z); chunk(f,"IEND",{}); fclose(f);}
static uint32_t tick(){return now;}
static void run(int ms){for(int i=0;i<ms;i+=5){now+=5;ui::app::tick(now);lv_timer_handler();}}
static void tap(int x,int y){tx=x;ty=y;run(60);tx=-1;run(100);}
int main(int argc,char**argv){
  lv_init(); lv_tick_set_cb(tick);
  auto* d=lv_display_create(800,480); lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);
  static uint8_t buf[800*48*2]; lv_display_set_flush_cb(d,flush); lv_display_set_buffers(d,buf,nullptr,sizeof buf,LV_DISPLAY_RENDER_MODE_PARTIAL);
  auto* in=lv_indev_create(); lv_indev_set_type(in,LV_INDEV_TYPE_POINTER); lv_indev_set_read_cb(in,touch);
  static model::DemoSource src; ui::app::begin(src); run(300);
  std::string out=argv[1];
  png((out+"/rlt.png").c_str());
  tap(565,290); png((out+"/dlg_m01.png").c_str());
  tap(20,20); run(100); // close by tapping overlay
  tap(374,457); run(1100); png((out+"/fault.png").c_str()); // demo button
  tap(374,457); run(1100); png((out+"/comm.png").c_str());
  tap(600,99); png((out+"/dlg_b05.png").c_str()); tap(20,20);
  tap(374,457); run(1100); png((out+"/off.png").c_str());
  tap(200,457); run(200); png((out+"/gallery.png").c_str());
  tap(66,457); run(200); png((out+"/back.png").c_str());
  printf("ok\n");
}
