#include <stdio.h>

int main(int argc,void *argv[])
{
	unsigned char Data[20],Data_length,ch;
	unsigned int mid=0,i=0;
	unsigned char times=0,Data_index=0;
	unsigned int CRC=0xFFFF;
	while(argv[i+1]!=NULL)
	{
		ch=((char *)argv[i+1])[1];
		if(ch>='a')
			Data[i]=ch-'a'+10;
		else
			Data[i]=ch-'0';
		ch=((char *)argv[i+1])[0];
		if(ch>='a')
			Data[i]+=(ch-'a'+10)*16;
		else
			Data[i]+=(ch-'0')*16;
		printf("Data[%d]=%02x\r\n",i,Data[i]);
		i++;
	}
	Data_length=i;
	while(Data_length)
	{
		CRC=Data[Data_index]^CRC;
		for(times=0;times<8;times++)
		{
			mid=CRC;
			CRC=CRC>>1;
			if(mid & 0x0001)
			{
				CRC=CRC^0xA001;
			}
		}
		Data_index++;
		Data_length--;
	}
	printf("CRC is %04x\r\n",CRC);
	return CRC;
}
