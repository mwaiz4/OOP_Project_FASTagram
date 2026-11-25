#include "models/NoticePost.h"
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
using namespace std;
static void notice_put(ostream& o,const string& s){if(s.size()>1000000)throw runtime_error("Text too long");uint32_t n=static_cast<uint32_t>(s.size());o.write(reinterpret_cast<const char*>(&n),4);o.write(s.data(),n);if(!o)throw runtime_error("Write failed");}
static string notice_get(istream& i){uint32_t n=0;i.read(reinterpret_cast<char*>(&n),4);if(!i||n>1000000)throw runtime_error("Invalid text length");string s(n,'\0');if(n)i.read(&s[0],n);if(!i)throw runtime_error("Read failed");return s;}
NoticePost::NoticePost():Post(),title_(),department_(),is_urgent_(false),expires_at_(0){}
NoticePost::NoticePost(int a,const Time& t,const string& c,const string& title,const string& dept,bool urgent,const Time& expiry):Post(a,t,c),title_(title),department_(dept),is_urgent_(urgent),expires_at_(expiry){}
string NoticePost::type_name()const{return "Notice";}string NoticePost::search_text()const{return caption()+" "+title_+" "+department_;}float NoticePost::render_height(float w)const{return w>0?72.0f+(caption().empty()?0:32.0f):0;}
void NoticePost::draw(DrawContext& c,const Rectangle& r)const{(void)c;(void)r;}void NoticePost::draw(DrawContext& c,float x,float y,float w)const{(void)c;(void)x;(void)y;(void)w;}bool NoticePost::handle_click(float mx,float my,float x,float y,float w,int v){(void)my;(void)y;(void)w;(void)v;return mx>=x;}
const string& NoticePost::title()const{return title_;}void NoticePost::set_title(const string& s){title_=s;}const string& NoticePost::department()const{return department_;}void NoticePost::set_department(const string& s){department_=s;}bool NoticePost::is_urgent()const{return is_urgent_;}void NoticePost::set_urgent(bool b){is_urgent_=b;}const Time& NoticePost::expires_at()const{return expires_at_;}void NoticePost::set_expires_at(const Time& t){expires_at_=t;}bool NoticePost::is_expired()const{return expires_at_.seconds()>0&&Time::now().seconds()>expires_at_.seconds();}
void NoticePost::write(ostream& o)const{Post::write(o);notice_put(o,title_);notice_put(o,department_);uint8_t b=is_urgent_;int64_t s=expires_at_.seconds();o.write(reinterpret_cast<const char*>(&b),1);o.write(reinterpret_cast<const char*>(&s),8);if(!o)throw runtime_error("Write failed");}
void NoticePost::read(istream& in){Post::read(in);string title=notice_get(in),dept=notice_get(in);uint8_t b=0;int64_t s=0;in.read(reinterpret_cast<char*>(&b),1);in.read(reinterpret_cast<char*>(&s),8);if(!in||b>1||s<0)throw runtime_error("Invalid notice fields");Time expiry(s);title_.swap(title);department_.swap(dept);is_urgent_=b!=0;expires_at_=expiry;}
