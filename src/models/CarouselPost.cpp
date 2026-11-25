#include "models/CarouselPost.h"
#include <algorithm>
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
using namespace std;
static void put_string(ostream& out, const string& value) { if(value.size()>1000000) throw runtime_error("String too long"); uint32_t n=static_cast<uint32_t>(value.size()); out.write(reinterpret_cast<const char*>(&n),4); out.write(value.data(),static_cast<streamsize>(n)); if(!out) throw runtime_error("Write failed"); }
static string get_string(istream& in) { uint32_t n=0; in.read(reinterpret_cast<char*>(&n),4); if(!in||n>1000000) throw runtime_error("Invalid string length"); string s(n,'\0'); if(n) in.read(&s[0],n); if(!in) throw runtime_error("Read failed"); return s; }
CarouselPost::CarouselPost():Post(),image_paths_(),current_index_(0){}
CarouselPost::CarouselPost(int a,const Time& t,const string& c,const vector<string>& p):Post(a,t,c),image_paths_(p),current_index_(0){if(p.size()>1000000)throw invalid_argument("Too many images"); for(const string& s:p)if(s.empty())throw invalid_argument("Empty image path");}
string CarouselPost::type_name()const{return "Carousel";}
string CarouselPost::search_text()const{string s=caption();for(const string& p:image_paths_)s+=" "+p;return s;}
float CarouselPost::render_height(float w)const{return w>0?w+24.0f+(caption().empty()?0:42.0f):0;}
void CarouselPost::draw(DrawContext& c,const Rectangle& r)const{(void)c;(void)r;}
void CarouselPost::draw(DrawContext& c,float x,float y,float w)const{(void)c;(void)x;(void)y;(void)w;}
bool CarouselPost::handle_click(float mx,float my,float x,float y,float w,int v){(void)my;(void)v;if(image_paths_.empty()||mx<x||mx>x+w)return false;if(mx<x+w/3){prev_slide();return true;}if(mx>x+2*w/3){next_slide();return true;}return false;}
void CarouselPost::add_image(const string& p){if(p.empty())throw invalid_argument("Empty image path");image_paths_.push_back(p);}
const vector<string>& CarouselPost::image_paths()const{return image_paths_;} int CarouselPost::image_count()const{return static_cast<int>(image_paths_.size());} int CarouselPost::current_index()const{return current_index_;}
void CarouselPost::next_slide(){if(!image_paths_.empty())current_index_=(current_index_+1)%image_count();} void CarouselPost::prev_slide(){if(!image_paths_.empty())current_index_=(current_index_+image_count()-1)%image_count();}
void CarouselPost::set_current_index(int i){current_index_=image_paths_.empty()?0:(max)(0,(min)(i,image_count()-1));} string CarouselPost::current_image_path()const{return image_paths_.empty()?string():image_paths_[current_index_];}
void CarouselPost::write(ostream& o)const{Post::write(o);if(image_paths_.size()>1000000)throw runtime_error("Too many images");uint32_t n=static_cast<uint32_t>(image_paths_.size());o.write(reinterpret_cast<const char*>(&n),4);for(const string& p:image_paths_)put_string(o,p);int32_t i=current_index_;o.write(reinterpret_cast<const char*>(&i),4);if(!o)throw runtime_error("Write failed");}
void CarouselPost::read(istream& in){Post::read(in);uint32_t n=0;in.read(reinterpret_cast<char*>(&n),4);if(!in||n>1000000)throw runtime_error("Invalid image count");vector<string> p;for(uint32_t j=0;j<n;++j)p.push_back(get_string(in));int32_t i=0;in.read(reinterpret_cast<char*>(&i),4);if(!in||i<0||(n==0&&i!=0)||(n>0&&static_cast<uint32_t>(i)>=n))throw runtime_error("Invalid slide index");image_paths_.swap(p);current_index_=i;}
