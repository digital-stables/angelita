// static_str
// A wrapper class around a stack based fixed string char my_str[c_storage_size];
// We also store the length in a byte (Thus the 256 byte limit!) to speed up operations.
// the size must be <=256 and divisible by 4
// Also added is support for a flash string printf style format() call.
#include "static_str.h"

void test(bool b_val)
{
 if(!b_val)
	 Serial.println("Failed....");
}

void test_static_str()
{
	const char cAlphabet[]="abcdefghijklmnopqrstuvwxyz";
	static_str<32> s2(cAlphabet);
	test(s2.length()==26);
	test(s2.available()==4);
	test(s2.at(3)=='d');
	test(s2.charAt(0)=='a');
	test(s2.charAt(25)=='z');
	s2.setCharAt(3,'D');
	test(s2.at(3)=='D');

	static_str<32> s("fred ");
	s.concat("fish");
	s+="_23";
	test(s=="fred fish_23");
	test(strcmp(s.data_offset(5),"fish_23")==0);
	s.remove(2,2);
	test(s=="fr fish_23");
	s.insert(2,"ed");
	test(s=="fred fish_23");
	test(s.length()==12);
	s+=cAlphabet;
	test(s.length()==30);
	test(s.full());

	static_str<32> s1;
	s1+="abc";
	s1+="def";
	s1+="ghij";
	s1+="klm";
	s1+="nopq";
	s1+="rst";
	s1+="uvw";
	s1+="xyz";
	test(s1==cAlphabet);
	test(s1.length()==26);
	s1.replace("def","DEF");
	test(s1!=s2);
	test(s1.equalsIgnoreCase(cAlphabet));

	test(static_str<8>(3)=="3");
	test(static_str<8>(3l)=="3");
	test(static_str<8>(3.348,2)=="3.35");
	test(static_str<8>(3.342,2)=="3.34");
	test(static_str<8>(33u,base10)=="33");
	test(static_str<8>(33u,base8)=="41");
	test(static_str<8>(33u,base16)=="21");

	static_str<16> s8;
	s8.set(255u,base16);
	test(s8=="ff");


	static_str<16> s16("buddy boy 21");
	s1=s16;
	s1.shrink(9);
	test(s1=="buddy boy");
	s1.toUpperCase();
	test(s1=="BUDDY BOY");
	s1.toLowerCase();
	test(s1=="buddy boy");


	test(s1.indexOf('u')==1);
	test(s1.indexOf('y')==4);
	test(s1.indexOf('y',5)==8);
	test(s1.indexOf('u')==1);
	test(s1.indexOf("ud")==1);

	test(s1.startsWith("bud"));
	test(s1.endsWith("boy"));

	test(s1.substring(2,4)=="dd");

	s1=s16;
	s1.remove(9);
	test(s1=="buddy boy");
	s1.remove(4,2);
	test(s1=="buddboy");
	s1.insert(4,"__");
	test(s1=="budd__boy");
	s1.insert(4,"abc",2);
	test(s1=="buddab__boy");
	s1.insert(4,4,'z');
	test(s1=="buddzzzzab__boy");
	s1.replace('z','a');
	test(s1=="buddaaaaab__boy");
	s1.replace("aaa","jjj");
	test(s1=="buddjjjaab__boy");
	s1.replace("jjj","");
	test(s1=="buddaab__boy");

	s1="	 fred is a knob.   ";
	s1.trim();
	test(s1=="fred is a knob.");
	s1="      ";
	s1.trim();
	test(s1.empty());

	s1="wha: ";
	s1+=3;
	s1+=3l;
	s1+=3u;
	s1+='c';
	s1+="de";
	s1+=4.349;
	test(s1=="wha: 333cde4.35");
	s16=s1;
	test(s16=="wha: 333cde4.3");

	//s1.insert();
}

void setup() 
{
    Serial.begin(115200);
 		test_static_str();
}

void loop() 
{
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
