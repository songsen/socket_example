//****************************************************************//
//	   DHT21Ê¹ÓÃ·¶Àý 
//µ¥Æ¬»ú AT89S52 »ò STC89C52RC 
//¹ŠÄÜ   Ž®¿Ú·¢ËÍÎÂÊª¶ÈÊýŸÝ Ÿ§Õñ 11.0592M ²šÌØÂÊ 9600 
//Ó²Œþ   P2.0¿ÚÎªÍšÑ¶¿ÚÁ¬œÓDHT11,DHT11µÄµçÔŽºÍµØÁ¬œÓµ¥Æ¬»úµÄµçÔŽºÍµØ£¬µ¥Æ¬»úŽ®¿ÚŒÓMAX232Á¬œÓµçÄÔ 
//¹«ËŸ  ÑãÁèµç×Ó    
//****************************************************************//

#include <reg51.h>
#include <intrins.h> 
//
typedef unsigned char  U8;       /* defined for unsigned 8-bits integer variable 	  ÎÞ·ûºÅ8Î»ÕûÐÍ±äÁ¿  */
typedef signed   char  S8;       /* defined for signed 8-bits integer variable		  ÓÐ·ûºÅ8Î»ÕûÐÍ±äÁ¿  */
typedef unsigned int   U16;      /* defined for unsigned 16-bits integer variable 	  ÎÞ·ûºÅ16Î»ÕûÐÍ±äÁ¿ */
typedef signed   int   S16;      /* defined for signed 16-bits integer variable 	  ÓÐ·ûºÅ16Î»ÕûÐÍ±äÁ¿ */
typedef unsigned long  U32;      /* defined for unsigned 32-bits integer variable 	  ÎÞ·ûºÅ32Î»ÕûÐÍ±äÁ¿ */
typedef signed   long  S32;      /* defined for signed 32-bits integer variable 	  ÓÐ·ûºÅ32Î»ÕûÐÍ±äÁ¿ */
typedef float          F32;      /* single precision floating point variable (32bits) µ¥Ÿ«¶Èž¡µãÊý£š32Î»³€¶È£© */
typedef double         F64;      /* double precision floating point variable (64bits) Ë«Ÿ«¶Èž¡µãÊý£š64Î»³€¶È£© */
//
#define uchar unsigned char
#define uint unsigned int
#define   Data_0_time    4

//----------------------------------------------//
//----------------IO¿Ú¶šÒåÇø--------------------//
//----------------------------------------------//
sbit  P2_0  = P2^0 ;

//----------------------------------------------//
//----------------¶šÒåÇø--------------------//
//----------------------------------------------//
U8  U8FLAG,k;
U8  U8count,U8temp;
U8  U8T_data_H,U8T_data_L,U8RH_data_H,U8RH_data_L,U8checkdata;
U8  U8T_data_H_temp,U8T_data_L_temp,U8RH_data_H_temp,U8RH_data_L_temp,U8checkdata_temp;
U8  U8comdata;
U8  outdata[5];  //¶šÒå·¢ËÍµÄ×ÖœÚÊý	   
U8  indata[5];
U8  count, count_r=0;
U8  str[5]={"RS232"};
U16 U16temp1,U16temp2;
SendData(U8 *a)
{
	outdata[0] = a[0]; 
	outdata[1] = a[1];
	outdata[2] = a[2];
	outdata[3] = a[3];
	outdata[4] = a[4];
	count = 1;
	SBUF=outdata[0];
}

void Delay(U16 j)
{      
    U8 i;
    for(;j>0;j--)
    { 	
    for(i=0;i<27;i++);

}
}
void  Delay_10us(void)
{
    U8 i;
    i--;
    i--;
    i--;
    i--;
    i--;
    i--;
}

void  COM(void)
{

    U8 i;
    
    for(i=0;i<8;i++)	   
    {

        U8FLAG=2;	
   		while((!P2_0)&&U8FLAG++);

        Delay_10us();
        Delay_10us();
        Delay_10us();

        U8temp=0;
        if(P2_0) 
			U8temp=1;

        U8FLAG=2;
        while((P2_0)&&U8FLAG++);
    //³¬Ê±ÔòÌø³öforÑ­»·		  
        if(U8FLAG==1) 
			break;
    //ÅÐ¶ÏÊýŸÝÎ»ÊÇ0»¹ÊÇ1	 
        
    // Èç¹ûžßµçÆœžß¹ýÔ€¶š0žßµçÆœÖµÔòÊýŸÝÎ»Îª 1 
        
        U8comdata<<=1;
        U8comdata|=U8temp;        //0
        }//rof

}

	//--------------------------------
	//-----Êª¶È¶ÁÈ¡×Ó³ÌÐò ------------
	//--------------------------------
	//----ÒÔÏÂ±äÁ¿ŸùÎªÈ«ŸÖ±äÁ¿--------
	//----ÎÂ¶Èžß8Î»== U8T_data_H------
	//----ÎÂ¶ÈµÍ8Î»== U8T_data_L------
	//----Êª¶Èžß8Î»== U8RH_data_H-----
	//----Êª¶ÈµÍ8Î»== U8RH_data_L-----
	//----Ð£Ñé 8Î» == U8checkdata-----
	//----µ÷ÓÃÏà¹Ø×Ó³ÌÐòÈçÏÂ----------
	//---- Delay();, Delay_10us();,COM(); 
	//--------------------------------

void RH(void)
{
    //Ö÷»úÀ­µÍ18ms 
    P2_0=0;
    Delay(180);
    P2_0=1;
    //×ÜÏßÓÉÉÏÀ­µç×èÀ­žß Ö÷»úÑÓÊ±20us
    Delay_10us();
    Delay_10us();
    Delay_10us();
    Delay_10us();
    //Ö÷»úÉèÎªÊäÈë ÅÐ¶ÏŽÓ»úÏìÓŠÐÅºÅ 
    P2_0=1;
    //ÅÐ¶ÏŽÓ»úÊÇ·ñÓÐµÍµçÆœÏìÓŠÐÅºÅ Èç²»ÏìÓŠÔòÌø³ö£¬ÏìÓŠÔòÏòÏÂÔËÐÐ	  
    if(!P2_0)		 //T !	  
    {
		U8FLAG=2;
		//ÅÐ¶ÏŽÓ»úÊÇ·ñ·¢³ö 80us µÄµÍµçÆœÏìÓŠÐÅºÅÊÇ·ñœáÊø	 
		while((!P2_0)&&U8FLAG++);
		U8FLAG=2;
		//ÅÐ¶ÏŽÓ»úÊÇ·ñ·¢³ö 80us µÄžßµçÆœ£¬Èç·¢³öÔòœøÈëÊýŸÝœÓÊÕ×ŽÌ¬
		while((P2_0)&&U8FLAG++);
		//ÊýŸÝœÓÊÕ×ŽÌ¬		 
		COM();
		U8RH_data_H_temp=U8comdata;
		COM();
		U8RH_data_L_temp=U8comdata;
		COM();
		U8T_data_H_temp=U8comdata;
		COM();
		U8T_data_L_temp=U8comdata;
		COM();
		U8checkdata_temp=U8comdata;
		P2_0=1;
		//ÊýŸÝÐ£Ñé 
		
		U8temp=(U8T_data_H_temp+U8T_data_L_temp+U8RH_data_H_temp+U8RH_data_L_temp);
		if(U8temp==U8checkdata_temp)
		{
			U8RH_data_H=U8RH_data_H_temp;
			U8RH_data_L=U8RH_data_L_temp;
			U8T_data_H=U8T_data_H_temp;
			U8T_data_L=U8T_data_L_temp;
			U8checkdata=U8checkdata_temp;
		}//fi
    }//fi

}

//----------------------------------------------
//main()¹ŠÄÜÃèÊö:  AT89C51  11.0592MHz 	Ž®¿Ú·¢ 
//ËÍÎÂÊª¶ÈÊýŸÝ,²šÌØÂÊ 9600 
//----------------------------------------------
void main()
{
	U8  i,j;
	
	//uchar str[6]={"RS232"};
	/* ÏµÍ³³õÊŒ»¯ */
	TMOD = 0x20;	  //¶šÊ±Æ÷T1Ê¹ÓÃ¹€×÷·œÊœ2
	TH1 = 253;        // ÉèÖÃ³õÖµ
	TL1 = 253;
	TR1 = 1;          // ¿ªÊŒŒÆÊ±
	SCON = 0x50;	  //¹€×÷·œÊœ1£¬²šÌØÂÊ9600bps£¬ÔÊÐíœÓÊÕ   
	ES = 1;
	EA = 1;           // Žò¿ªËùÒÔÖÐ¶Ï   
	TI = 0;
	RI = 0;
	SendData(str) ;   //·¢ËÍµœŽ®¿Ú 
	Delay(1);         //ÑÓÊ±100US£š12MŸ§Õñ)
	while(1)
	{  

	   //------------------------
	   //µ÷ÓÃÎÂÊª¶È¶ÁÈ¡×Ó³ÌÐò 
	   RH();
	   //Ž®¿ÚÏÔÊŸ³ÌÐò 
	   //--------------------------

	   str[0]=U8RH_data_H;
	   str[1]=U8RH_data_L;
	   str[2]=U8T_data_H;
	   str[3]=U8T_data_L;
	   str[4]=U8checkdata;
	   SendData(str) ;  //·¢ËÍµœŽ®¿Ú  
	   //¶ÁÈ¡Ä£¿éÊýŸÝÖÜÆÚ²»Ò×Ð¡ÓÚ 2S 
	   Delay(20000);
	}//elihw
	
}// main

void RSINTR() interrupt 4 using 2
{
	U8 InPut3;
	if(TI==1) //·¢ËÍÖÐ¶Ï	  
	{
		TI=0;
		if(count!=5) //·¢ËÍÍê5Î»ÊýŸÝ	 
		{
			SBUF= outdata[count];
			count++;
		}
	}

	if(RI==1)	 //œÓÊÕÖÐ¶Ï		  
	{	
		InPut3=SBUF;
		indata[count_r]=InPut3;
		count_r++;
		RI=0;								 
		if (count_r==5)//œÓÊÕÍê4Î»ÊýŸÝ 
		{
		//ÊýŸÝœÓÊÕÍê±ÏŽŠÀí¡£
			count_r=0;
		str[0]=indata[0];
		 str[1]=indata[1];
		   str[2]=indata[2];
			 str[3]=indata[3];
				 str[4]=indata[4];
				 P0=0;
		}
	}
}
