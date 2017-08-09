#include <stdint.h>
#include "log.h"
#include "process.h"

unsigned int CRC_check(unsigned char *Data,unsigned char Data_length)
{
	unsigned int mid=0;
	unsigned char times=0,Data_index=0;
	unsigned int CRC=0xFFFF;
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
	return CRC;
}

uint32_t process_message(int msgid)
{
	struct msg_st data;
	if(msgrcv(msgid, (void*)&data, 
				sizeof(struct msg_st)-sizeof(long int), 0x33 , 0)>=0) {
		int crc = (data.text[data.len - 2] << 8) | data.text[data.len - 1];
		if(crc!=CRC_check((unsigned char *)data.text,data.len - 2)) {
			int message_type = (data.text[2] << 8) | data.text[3];
			int message_len = data.len - 7;

			switch (message_type) {
				case MESSAGE_TYPE_CAP_TIME:
					/* time got , need send ?*/
					break;
				case MESSAGE_TYPE_CAP_ACK:
					/* got ack from cap board*/
					break;
				case MESSAGE_TYPE_SENSOR_LIST:
					/* got sensor list,
					 * should be in turn zero
					 * or verify sensor page
					 */
					break;
				case MESSAGE_TYPE_VERIFY_LIST:
					/* got sensor verify point
					 * and adjust value
					 */
					break;
				case MESSAGE_TYPE_VERSION:
					/* got cap board firmware
					 * version, should be in 
					 * program startup
					 */
					break;
				default :
					/* real cap data, co,
					 * co2, hcho, etc
					 */
					break;
			}
		}
	}
	return 0;
}
