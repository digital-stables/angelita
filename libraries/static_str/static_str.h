// Static String
// John Curtis
#ifndef _STATIC_STR_H
#define _STATIC_STR_H
#pragma once
#ifdef ARDUINO
#include "Arduino.h"
#include "WString.h"
#endif
#include <assert.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////
// Flash memory Helpers
//NB. variables must be globally defined, OR defined with the static keyword, to work with PROGMEM.
//  The following code will NOT work when inside a function:
//  const char long_str[] PROGMEM = "Hi, I would like to tell you a bit about myself.\n";
//  The following code WILL work, even if locally defined within a function:
//  const static char long_str[] PROGMEM = "Hi, I would like to tell you a bit about myself.\n"
//
//Define flash ie Program string const
//ie.
//DEFINE_PSTR(my_const_str_val,"This is a literal string held completely in flash memory."); 
#define DEFINE_PSTR(lbl,s) static const char pstr_##lbl[] PROGMEM = s
//Use GET_PSTR() to access:
//NB. Use F() for temp versions of strings
#define GET_PSTR(lbl) (reinterpret_cast<const __FlashStringHelper*>(pstr_##lbl))

//Do nothing dummy versions of above - to allow testing
#define DEFINE_RSTR(lbl,s) static const char lbl[] = s 
#define GET_RSTR(lbl) lbl
//#define DEFINE_RSTR(lbl,s) DEFINE_PSTR(lbl,s)
//#define GET_RSTR(lbl) GET_RSTR(lbl)
///////////////////////////////////////////////////////////////////////////////////////////
//  Radix
//  used for u_int display: see static_str constructors
///////////////////////////////////////////////////////////////////////////////////////////
enum Radix{base8,base10,base16};

///////////////////////////////////////////////////////////////////////////////////////////
// static_str
// A wrapper class around a stack based fixed string char my_str[c_storage_size];
// We also store the length in a byte (Thus the 256 byte limit!) to speed up operations.
// the size must be <=256 and divisible by 4
// Also added is support for a flash string printf style format() call.
//
// It is best used on the stack as a drop-in replacement for String to avoid dynamic memory allocation
// NB.
// * No memory is allocated so the string will overflow if the fixed size is too small!
// * If a concat call overflows the return will be false
// eg.
// void to_serial(int widget_no,double d_val)
//  {
//  static_str<48> s;
//	s.format(F("widget no: %i - val = %6.2f"),widget_no,d_val);
//  Serial.print(s);
//  s="45.3456";
//	double d=s.toDouble();
//  
//  }
// c_storage_size:  is the total bytes used for this class
///////////////////////////////////////////////////////////////////////////////////////////
template<int c_storage_size>
class static_str final
{
private:
	//Min == 4 bytes Need 1 length AND 1 for zero byte so 2 char string is smallest allowed!
	static_assert(c_storage_size>=4 && c_storage_size<=256,"Must be between between 4 & 256"); 
	static_assert(c_storage_size%4==0,"Must be divisble by 4");//4 byte boundary
	static constexpr const int get_min(int i1,int i2){return i1<i2 ? i1:i2;}
	static constexpr const int get_max(int i1,int i2){return i1>i2 ? i1:i2;}
public:
	typedef char char_type;
	typedef const char_type* const_pointer;
#ifdef ARDUINO
	typedef __FlashStringHelper FlashHelper;
	typedef const FlashHelper* FlashPtr;
#endif
	static constexpr const int npos = -1;
	static constexpr const int capacity(){return c_storage_size-2;}//byte for len and zero char
	static constexpr bool valid_rhs(const_pointer data,int len){return data!=NULL ? len>=0:len==0;}//allow empty
	static constexpr bool valid_len(int len){return len>=0 && len<=capacity();}//byte for len and zero char
	static const int safe_len(const_pointer str){return str==NULL ? 0:static_cast<int>(strlen(str));}
	static char get_radix_char(Radix r=base10)
		{
		switch(r)
			{
			case base8: return 'o';
			case base10:return 'u';
			case base16:return 'x';
			default: assert(false); return 'u';
			}
		}
public:
	static_str():m_len(0),m_str{0}{} //Make sure null added
	static_str(const_pointer str){assign(str);}
	static_str(const_pointer lpch,int len){assign(lpch,len);}
	static_str(char_type ch,int repeat=1){assign(repeat,ch);}
	template<int c_storage_size2>
	static_str(const static_str<c_storage_size2>& rhs){assign(rhs);}
	//Numeric
	explicit static_str(int i){set(i);}
	explicit static_str(long l){set(l);}
	explicit static_str(unsigned char u,Radix r=base10){set(u,r);}
	explicit static_str(unsigned int u,Radix r=base10){set(u,r);}
	explicit static_str(unsigned long u,Radix r=base10){set(u,r);}
	explicit static_str(float f,int decPlaces=2){set(f,decPlaces);}
	explicit static_str(double d,int decPlaces=2){set(d,decPlaces);}
	//overloaded assignment
	static_str& operator=(const_pointer str){assign(str); return *this;}
	static_str& operator=(char_type c){assign(c); return *this;}
	template<int c_storage_size2>
	static_str& operator=(const static_str<c_storage_size2>& rhs){assign(rhs); return *this;}
	//Numeric
	template<typename Num>
	static_str& operator=(Num n){set(n); return *this;}
#ifdef ARDUINO
	explicit static_str(double d,int s_width,int decPlaces){set_float(d,s_width,decPlaces);}
	static_str(FlashPtr str):static_str(){concat(str);}
	static_str(const String& s):static_str(s.c_str(),s.length()){}
	static_str& operator=(FlashPtr str){assign(str); return *this;}
	static_str& operator=(const String& s){assign(s); return *this;}
#endif
public:
	int	length()const{return m_len;}
	int available()const{return capacity()-length();}
	bool full()const{return available()==0;}
	bool empty()const{return length()==0;}
	//Data access
	char_type at(int pos)const{return check_valid_pos(pos) ? data()[pos]:0;}
	char_type charAt(int pos)const{return at(pos);}
	void setCharAt(int pos,char_type c){if(check_valid_pos(pos))m_str[pos]=c;}
	char_type operator[](int pos)const{return at(pos);}
	const_pointer data()const{return m_str;}
	const_pointer data_offset(int off)const{assert(is_valid_offset(off)); return data()+off;}
	const_pointer c_str()const{return data(); }
	operator const_pointer()const{return data();}
	char_type* begin(){return m_str;}
	char_type* end(){return begin()+length();}
	const_pointer begin()const{return data();}
	const_pointer end()const{return begin()+length();}
public:
	void clear(){set_len(0);}//set to empty string
public:
	//assign
	bool assign(const_pointer p_data,int len){clear(); return concat(p_data,len);}
	bool assign(const_pointer p_str){return assign(p_str,safe_len(p_str));}
	bool assign(int repeat,char_type ch){clear(); return concat(repeat,ch);}
	template<int c_storage_size2>
	bool assign(const static_str<c_storage_size2>& rhs){clear(); return concat(rhs);}
	//Numeric
	bool set(int i){return format("%i",i);}
	bool set(long l){return format("%li",l);}
	bool set(unsigned char u,Radix r=base10){return format_u(u,r,"hh");}
	bool set(unsigned int u,Radix r=base10){return format_u(u,r);}
	bool set(unsigned long u,Radix r=base10){return format_u(u,r,"l");}
	bool set(float f,int decPlaces=2){return set(static_cast<double>(f),decPlaces);}
	bool set(double d,int decPlaces=2){return format("%.*lf",decPlaces,d);}
#ifdef ARDUINO
	bool assign(const String& s){clear(); return concat(s);}
	bool assign(FlashPtr str){clear(); return concat(str);}
	bool set_float(double d,int s_width,int decPlaces=2)
		{
		clear();
		assert(valid_len(s_width+1) && decPlaces<s_width);
		if(!valid_len(s_width+1) || decPlaces>=s_width)
			return false;
		dtostrf(d,s_width,decPlaces,m_str);
		update_len();
		return true;
		}
#endif
public:
	bool shrink(int len){assert(len<=length()); return len<length() ? set_len(len):false;}
	bool remove(int pos){return shrink(pos);}//from pos to end
	bool remove(int pos,int cnt)
		{
		assert(cnt>=0);
		if(cnt<=0)
			return cnt==0;
		if(!check_valid_pos(pos))
			return false;
		assert(is_valid_pos(pos+cnt-1));
		const int rem_sz=length()-pos;
		int actual_cnt=get_min(cnt,rem_sz);//shrink to max available cnt
		memmove(m_str+pos,m_str+pos+actual_cnt,rem_sz-actual_cnt+1);//shift rem chars down (including null char)
		set_len(length()-actual_cnt);
		return actual_cnt==cnt;
		}
	bool insert(int pos,const_pointer s){return insert(pos,s,safe_len(s));}
	bool insert(int pos,const_pointer data,int len)
		{
		if(!check_valid_offset(pos))
			return false;
		if(!valid_rhs(data,len) || len==0)
			return len==0;
		if(full())
			return false;
		const int actual_cnt=get_min(available(),len);
		memmove(m_str+pos+actual_cnt,m_str+pos,length()-pos);//shift rem chars up (including null char)
		memcpy(m_str+pos,data,actual_cnt);//copy data
		set_len(length()+actual_cnt);
		return actual_cnt==len;
		}
	bool insert(int pos,int repeat,char_type ch)
		{
		if(!check_valid_offset(pos))
			return false;
		assert(repeat>0 && ch!=0);
		if(repeat<=0 || ch==0)
			return false;
		if(full())
			return false;
		const int actual_cnt=get_min(available(),repeat);
		memmove(m_str+pos+actual_cnt,m_str+pos,length()-pos);//shift rem chars up (including null char)
		for(int i=0; i<actual_cnt; i++)
			m_str[pos+i]=ch;
		set_len(length()+actual_cnt);
		return actual_cnt==repeat;
		}
	bool trim()
		{
		if(empty()) 			
			return false;
		const int old_len=length();
		const_pointer start=begin();
		const_pointer last=end()-1;
		while (isspace(*start))start++;
		while(isspace(*last) && last>=start)last--;
		if(start>begin())
			remove(0,static_cast<int>(start-begin()));
		set_len(static_cast<int>(last-start+1));
		return old_len!=length();
		}
	static_str& toLowerCase()
		{
		for(char_type& p:*this)
			p=static_cast<char_type>(tolower(p));
		return *this;
		}
	static_str& toUpperCase()
		{
		for(char_type& p:*this)
			p=static_cast<char_type>(toupper(p));
		return *this;
		}
	int replace(char_type c,char_type new_c)
		{
		assert(c!=0 && new_c!=0);
		if(c==0 || new_c==0 || c==new_c)
			return 0;
		int cnt=0;
		for(char_type& p:*this)
			if(p==c)
				{
				p=new_c;
				cnt++;
				}
		return cnt;
		}
	int replace(const_pointer s,const_pointer new_s)
		{
		if(empty()) 
			return 0;
		int src_len=safe_len(s);
		int repl_len=safe_len(new_s);
		if(src_len==0)
			return 0;
		if(repl_len>0 && src_len==repl_len && strcmp(s,new_s)==0)
			return 0;
		int cnt=0;
		for(int pos=indexOf(s); is_valid_pos(pos); pos=indexOf(s,pos))
			{
			if(repl_len>0)
				{
				if(!replace(pos,src_len,new_s,repl_len))
					return cnt;
				pos+=repl_len;	//Move to end of this replace string ready for next search
				}
			else
				remove(pos,src_len);
			cnt++;			
			if(!is_valid_pos(pos))//Drop out if pos over the end
				break;
			}
		return cnt;
		}
public:
	//concat
	template<typename Param>
	static_str& operator+=(Param p){concat(p); return *this;}
	template<int c_storage_size2>
	static_str& operator+=(const static_str& s){concat(s); return *this;}
	//direct
	bool concat(const_pointer p_data,int len){return insert(length(),p_data,len);}
	bool concat(const_pointer p_str){return concat(p_str,safe_len(p_str));}
	bool concat(char_type c){return concat(&c,1);}
	bool concat(int repeat,char_type c){return insert(length(),repeat,c);}
	template<int c_storage_size2>
	bool concat(const static_str<c_storage_size2>& rhs){return concat(rhs.data(),rhs.length());}
	//Numeric
	bool concat(float f,int decPlaces=2){return concat(static_str(f,decPlaces));}
	bool concat(double d,int decPlaces=2){return concat(static_str(d,decPlaces));}
	bool concat(int i){return concat(static_str(i));}
	bool concat(long l){return concat(static_str(l));}
	template<typename U>
	bool concat(U u,Radix r=base10){return concat(static_str(u,r));}
	//Special case - Force end char if full, otherwise just add
	bool force_concat(char_type ch)
		{
		if(!full())
			return concat(1,ch);
		m_str[length()-1]=ch;//Overwrite current end char
		return true;
		}
#ifdef ARDUINO
	static_str& operator+=(const String& s){concat(s); return *this;}
	static_str& operator+=(FlashPtr str){concat(str); return *this;}
	bool concat(const String& s){return concat(s.c_str(),s.length());}
	bool concat(FlashPtr str)
		{
		if(str==NULL)
			return false;
		const_pointer prog_ptr=reinterpret_cast<const_pointer>(str);
		const int prog_len=strlen_P(prog_ptr);
		const int actual_cnt=get_min(available(),prog_len);
		if(actual_cnt>0)
			memcpy_P(m_str+length(),prog_ptr,actual_cnt);
		return actual_cnt==prog_len;
		}
#endif
public:
	bool formatV(const_pointer pFormat,va_list args)
		{
		clear();
		if(safe_len(pFormat)==0)//must be at least 1 char!
			return false;		
		va_list argssave;
		va_copy(argssave,args);
		int ret=vsnprintf(m_str,capacity()+1,pFormat,args);
		va_end(argssave);
		assert(ret<=capacity());//flag overrun
		if(ret>=0 && ret<=capacity())
			return set_len(ret);
		else if(ret>capacity())
			return set_len(capacity());//Assume overflow so truncated
		clear();
		return false;
		}
	bool format(const_pointer pFormat,...)
		{
		clear();
		if(safe_len(pFormat)==0)
			return false;
		va_list args;
		va_start(args,pFormat);
		bool b=formatV(pFormat,args);
		va_end(args);
		return b;
		}
#ifdef ARDUINO
	bool format(FlashPtr pFormat,...)
		{
		clear();
		static_str fmt(pFormat);//store in temp
		if(fmt.empty())
			return false;
		va_list args;
		va_start(args,pFormat);
		bool b=formatV(fmt.data(),args);
		va_end(args);
		return b;
		}
#endif
public:
	long	toInt()const{return to_int<long>();}
	float	toFloat()const{return to_float<float>();}
	double	toDouble()const{return to_float<double>();}
	template<typename T=double>
	auto to_float()const -> T
		{
		T d=0.0f;
		get_float(d);
		return d;
		}
	template<typename T=double>
	bool get_float(T& d)const
		{
		d=0.0f;
		if(empty())
			return false;
		d=static_cast<T>(::atof(data()));
		return !isnan(d);
		}
	template<typename T=int>
	auto to_int()const -> T
		{
		T i=0;
		get_int(i);
		return i;
		}
	template<typename T=int>
	bool get_int(T& i)const
		{
		i=0;
		if(empty())
			return false;
		i=static_cast<T>(::atol(data()));
		return true;
		}
public:
	//string comparison
	int compareTo(const static_str& rhs)const{return (&rhs==this) ? 0:compare(rhs.data(),rhs.length());}
	template<int c_storage_size2>
	int compareTo(const static_str<c_storage_size2>& rhs)const{return compare(rhs.data(),rhs.length());}
	int compareTo(const_pointer rhs)const{return compare(rhs,safe_len(rhs));}
	bool equalsIgnoreCase(const static_str& rhs)const{return equals(rhs,true);}
	bool equalsIgnoreCase(const_pointer rhs)const{return equals(rhs,true);}
	bool operator==(const_pointer rhs)const{return equals(rhs);}
	bool operator!=(const_pointer rhs)const{return !equals(rhs);}
	bool operator==(const static_str& rhs)const{return equals(rhs);}
	bool operator!=(const static_str& rhs)const{return !equals(rhs);}
	template<int c_storage_size2>
	bool operator==(const static_str<c_storage_size2>& rhs)const{return equals(rhs);}
	template<int c_storage_size2>
	bool operator!=(const static_str<c_storage_size2>& rhs)const{return !equals(rhs);}
	bool operator<(const static_str& rhs)const{return compareTo(rhs)<0;}
	bool operator>(const static_str& rhs)const{return compareTo(rhs)>0;}
	bool operator<=(const static_str& rhs)const{return compareTo(rhs)<=0;}
	bool operator>=(const static_str& rhs)const{return compareTo(rhs)>=0;}
	bool operator<(const_pointer rhs)const{return compareTo(rhs)<0;}
	bool operator>(const_pointer rhs)const{return compareTo(rhs)>0;}
	bool operator<=(const_pointer rhs)const{return compareTo(rhs)<=0;}
	bool operator>=(const_pointer rhs)const{return compareTo(rhs)>=0;}
public:
	bool startsWith(const_pointer s,int offset=0)const
		{
		const int rhs_len=safe_len(s);
		if(rhs_len==0 || (offset+rhs_len)>length())
			return false;
		return strncmp(data_offset(offset),s,rhs_len)==0;
		}
	bool endsWith(const_pointer s)const
		{
		const int rhs_len=safe_len(s);
		if(rhs_len==0 || rhs_len>length())
			return false;
		return strncmp(data_offset(length()-rhs_len),s,rhs_len)==0;
		}
	int indexOf(char_type c,int start_pos=0)const
		{
		if(!check_valid_pos(start_pos))
			return npos;
		const_pointer p=strchr(data_offset(start_pos),c);
		if(p==NULL)return npos;
		return static_cast<int>(p-data());
		}
	int indexOf(const_pointer s,int start_pos=0)const
		{
		if(!check_valid_pos(start_pos))
			return npos;
		const int rhs_len=safe_len(s);
		if(rhs_len==0 || (start_pos+rhs_len)>length())
			return npos;
		const_pointer p=strstr(data_offset(start_pos),s);
		if(p==NULL)return npos;
		return static_cast<int>(p-data());
		}
	int lastIndexOf(char c,int from_pos=npos)const
		{
		if(from_pos==npos)
			from_pos=length()-1;//end
		if(!check_valid_pos(from_pos))
			return npos;
		const_pointer p=memchr(data(),c,from_pos);
		if(p==NULL)return npos;
		return static_cast<int>(p-data());
		}
	int lastIndexOf(const_pointer s,int from_pos=npos)const
		{
		if(from_pos==npos)
			from_pos=length()-1;//end
		if(!check_valid_pos(from_pos))
			return npos;
		const int rhs_len=safe_len(s);
		const int start_pos=from_pos+1-rhs_len;//pos to start testing
		if(rhs_len==0 || start_pos<0)
			return npos;
		for(int pos=start_pos; pos>=0; pos--) 
			{
			const_pointer p=strstr(data_offset(pos),s);
			if(p!=NULL) 
				return static_cast<int>(p-begin());
			}
		return npos;
		}
	//For my money String::substring() is wrong
	//In description:
	//String stringOne = "Content-Type: text/html";
	//if(stringOne.substring(14,18) == "text") 
	//{...} 
	//In fact if left/from and right/to are position 
	//stringOne.substring(14,18) should return "text/"
	//  
	static_str substring(int left,int right)const
		{
		static_str sub;
		int len=right-left/*+1*/;//TODO
		assert(len>0);
		if(!check_valid_pos(left) || len<=0)
			return sub;
		len=get_min(len,length()-left);
		sub.assign(data_offset(left),len);
		return sub;
		}
public:
	//helpers
	//bool can_add_all(int len)const{return len>=0 && (length()+len)<=capacity();}
	bool is_valid_offset(int off)const{return off>=0 && off<=length();}//end() is past last
	bool is_valid_pos(int pos)const{return pos>=0 && pos<length();}
private:
	 bool replace(int pos,int erase_cnt,const_pointer lpch,int len)
		{
		if(erase_cnt>0)
			remove(pos,erase_cnt);
		return insert(pos,lpch,len);
		}
	bool replace(int pos,int erase_cnt,int repeat,char_type ch)
		{
		if(erase_cnt>0)
			remove(pos,erase_cnt);
		return insert(pos,repeat,ch);
		}
	bool check_valid_pos(int pos)const{assert(is_valid_pos(pos)); return is_valid_pos(pos);}
	bool check_valid_offset(int off)const{assert(is_valid_offset(off)); return is_valid_offset(off);}
	bool update_len(){m_len=static_cast<uint8_t>(safe_len(m_str));}
	bool set_len(int len)
		{
		assert(valid_len(len));
		if(!valid_len(len))
			return false;
		m_len=static_cast<uint8_t>(len);
		m_str[m_len]=0;
		return true;
		}
private:
	bool equals(const static_str& rhs,bool b_insens=false)const
		{return (&rhs==this) ? true:equals(rhs.data(),rhs.length(),b_insens);}
	bool equals(const_pointer rhs,bool b_insens=false)const{return equals(rhs,safe_len(rhs),b_insens);}
	bool equals(const_pointer rhs,int len,bool b_insens=false)const
		{
		if(length()!=len)
			return false;
		assert(valid_rhs(rhs,len));
		if(!valid_rhs(rhs,len))
			return false;
		if(!b_insens)
			return compare(rhs,len)==0;
		//case insensitive
		for(int i=0; i<len; i++)
			if(toupper(m_str[i])!=toupper(rhs[i])) 
				return false;
		return true;
		}
	int compare(const_pointer rhs,int len)const
		{
		assert(valid_rhs(rhs,len));
		if(!valid_rhs(rhs,len) || len==0)
			return empty() ? 0:+1;
		return strncmp(data(),rhs,get_max(length(),len));
		}
	template<typename U>
	bool format_u(U u,Radix r=base10,const_pointer prefix="")
		{
		static_str<8> fmt("%");
		fmt+=prefix;
		fmt+=get_radix_char(r);
		return format(fmt.data(),u);
		}
private:
	uint8_t m_len;
	char_type m_str[c_storage_size-1];

public:
#ifdef ARDUINO
	friend static_str operator+(const String& s,const static_str& rhs){return static_str(s)+=rhs;}
#endif
	friend static_str operator+(const_pointer s,const static_str& rhs){return static_str(s)+=rhs;}
	template<typename TRhs> //All others
	friend static_str operator+(TRhs lhs,const static_str& rhs){return static_str(lhs)+=rhs;}
};

///////////////////////////////////////////////////////////////////////////////////////////
#endif
