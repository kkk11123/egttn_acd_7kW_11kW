/*
 * _app_system_control.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */



#include <_app_system_control.h>
#include <_app_charging_service.h>
#include <_config.h>

static uint8_t config_ring_buffer[CONFIG_RING_BUFFER_SIZE] = {'\0', };
static s__LIB_QUEUE config_queue = {config_ring_buffer, CONFIG_RING_BUFFER_SIZE, 0, CONFIG_RING_BUFFER_SIZE, 0};
static s__APP_CONFIG config_app = {0, {0, }, {'\0', }, 0, { }, 0};


#if 0
static uint8_t mem_sector_buf[MAX_FLASH_DATA_SIZE] = {0xff,};
#endif

static gUserDelay gDelay_systemctl_comm_periodic_loop_time;

#if 1
static uint16_t wCRCTable[] = {
0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };
#endif

//USART2 데이터 송신 함수 -> RFID
void config_comPut(uint8_t *data, uint8_t cnt)
{
	_MW_UART_tx02(data, cnt);
}

//CRC 16 계산함수
uint16_t config_crc16(uint8_t *nData, uint8_t length)
{
    uint8_t nTemp;
    uint16_t wCRCWord = 0xFFFF;

    while (length--){
      nTemp = *nData++ ^ wCRCWord;
      wCRCWord >>= 8;
      //wCRCWord ^= config_wCRCTable[nTemp];
      wCRCWord ^= wCRCTable[nTemp];
    }
    return wCRCWord;
}

//유효 프레임 탐색 함수
void _APP_SYSTEMCTL_framesearch()
{
	uint8_t uch = 0xff, ucl = 0xff, bh, bl;
	uint16_t mclen, callen, i;
	uint16_t bsave_pointer;
	uint8_t crc_calc[100];
	uint16_t crc_data = 0;
	uint8_t temp_func = 0;
	uint8_t temp_cmd = 0;
	uint8_t temp_length = 0;
	//static uint8_t frame_index_cnt;

	mclen =  _LIB_U8QUEUE_comLen(&config_queue);

	//frame을 찾았거나 데이터 길이가 8 byte 미만일 때
	if((config_app.found_frame == 1) || (mclen < 4))
	{
		return;
	}

	//backup pointer
	bsave_pointer = config_queue.action_pointer;

	//RFID 승인원 참고
	//CMD check
	temp_cmd = _LIB_U8QUEUE_get_byte(&config_queue);//UI_COMM_mb_get_byte_rtu(); // CMD
	//cmd가 쓰기 동작일 때
	if(temp_cmd == CONFIG_WRITE_COMMAND)
	{
		//FUNC check
		temp_func = _LIB_U8QUEUE_get_byte(&config_queue);
		//LEN check
		temp_length = _LIB_U8QUEUE_get_byte(&config_queue);
		callen = 5 + temp_length;
	}
	//cmd가 읽기 동작일 때
	else if(temp_cmd == CONFIG_READ_COMMAND)
	{
		//FUNC check
		temp_func = _LIB_U8QUEUE_get_byte(&config_queue);
		callen = 4;
	}
	else
	{
		return;
	}

	//mclen check before CRC check
	if(callen > mclen)
	{
		config_queue.action_pointer = bsave_pointer; //Restore Pointer
		return;
	}

	for (i = 0; i < callen - 2; i++) //crc 데이터 전까지
	{
		if(temp_cmd == CONFIG_WRITE_COMMAND)
		{
			if(i == 0)		crc_calc[i] = temp_cmd;
			else if(i == 1)	crc_calc[i] = temp_func;
			else if(i == 2) crc_calc[i] = temp_length;
			else			crc_calc[i] = _LIB_U8QUEUE_get_byte(&config_queue);//UI_COMM_mb_get_byte_rtu();
		}
		else if(temp_cmd == CONFIG_READ_COMMAND)
		{
			if(i == 0)		crc_calc[i] = temp_cmd;
			else if(i == 1)	crc_calc[i] = temp_func;
		}
	}

	bl = _LIB_U8QUEUE_get_byte(&config_queue);//UI_COMM_mb_get_byte_rtu();
	bh = _LIB_U8QUEUE_get_byte(&config_queue);//UI_COMM_mb_get_byte_rtu();

	crc_data = config_crc16(crc_calc, callen-2);
	ucl = crc_data;
	uch = crc_data >> 8;

	//crc 데이터와 계산된 데이터가 동일한지 확인 -> 동일 시 : 옳은 데이터
	if(((bh == uch) && (bl == ucl)))
	{
		//_LIB_LOGGING_printf("DM modbus search frame ok\r\n");
		config_queue.action_pointer = bsave_pointer; //Restore Pointer
		config_app.found_frame = 1;
	}
	else
	{
		return;
	}
}

void _APP_SYSTEMCTL_req(uint8_t cmd, uint8_t func, uint8_t len, uint8_t *data)
{
	uint16_t crc_data = 0;
	uint8_t set_cmd = cmd;
	uint8_t set_func = func;
	uint8_t set_len = len;
	uint8_t i;

	config_app.tx_cnt = 0; //cnt init


	if(set_cmd == CONFIG_WRITE_COMMAND) //쓰기 명령일 때
	{
		config_app.tx_buf[config_app.tx_cnt++] = set_cmd;
		config_app.tx_buf[config_app.tx_cnt++] = set_func;
		config_app.tx_buf[config_app.tx_cnt++] = 'O';
		config_app.tx_buf[config_app.tx_cnt++] = 'K';
	}
	else if(set_cmd == CONFIG_READ_COMMAND) //읽기 명령일 때
	{
		config_app.tx_buf[config_app.tx_cnt++] = set_cmd;
		config_app.tx_buf[config_app.tx_cnt++] = set_func;
		config_app.tx_buf[config_app.tx_cnt++] = set_len;

		for(i=0;i<set_len;i++) //data
		{
			config_app.tx_buf[config_app.tx_cnt++] = data[i];
		}

		crc_data = config_crc16(&config_app.tx_buf[0], (config_app.tx_cnt));
		//crc
		config_app.tx_buf[config_app.tx_cnt++] = crc_data;
		config_app.tx_buf[config_app.tx_cnt++] = crc_data >> 8;
	}

	config_comPut(config_app.tx_buf, config_app.tx_cnt); //송신
}
void _APP_SYSTEMCTL_push_ringbuffer(uint8_t comdt)
{
	_LIB_U8QUEUE_push_ring(&config_queue, comdt);
}

//EEPROM에 카드번호 저장 및 삭제 함수
uint8_t config_write_common_command(uint8_t func,uint8_t* temp_data,uint8_t len)
{
	uint8_t ret_value = _TRUE;

	uint8_t txtemp[30] = {0,};
	uint8_t rxtemp[30] = {0,};

	int i = 0;
	int j = 0;

	switch(func)
	{
		case CONFIG_FUNC_SAVE_CARDNUM :
			if(len == 8)
			{
				for(i = 0; i<MAX_CARDNUM_MEMORY ; i++)
				{
					if(true == ee24_read(MEM_ADDR_CARDNUM_INDEX_LSB(i), rxtemp, 1, 100))
					{
						if(0xFF == rxtemp[0]) //EEPROM 메모리 주소를 읽었는데 데이터가 비어있으면
						{
							txtemp[0] = 0x00;
							txtemp[1] = 0x00;
							txtemp[2] = 0x00;
							txtemp[3] = 0x00;
							txtemp[4] = 0x00;
							txtemp[5] = 0x00;
							txtemp[6] = 0x00;
							txtemp[7] = (i+1); //인덱스
							txtemp[8] = temp_data[0];
							txtemp[9] = temp_data[1];
							txtemp[10] = temp_data[2];
							txtemp[11] = temp_data[3];
							txtemp[12] = temp_data[4];
							txtemp[13] = temp_data[5];
							txtemp[14] = temp_data[6];
							txtemp[15] = temp_data[7];

							ee24_write(MEM_ADDR_CARDNUM_INDEX_MSB(i), txtemp, 16, 100); //rfid card num EEPROM에 저장

							config_app.usecardnum[i][0] = txtemp[8];
							config_app.usecardnum[i][1] = txtemp[9];
							config_app.usecardnum[i][2] = txtemp[10];
							config_app.usecardnum[i][3] = txtemp[11];
							config_app.usecardnum[i][4] = txtemp[12];
							config_app.usecardnum[i][5] = txtemp[13];
							config_app.usecardnum[i][6] = txtemp[14];
							config_app.usecardnum[i][7] = txtemp[15];

							break;
						}
					}
				}



				if(MAX_CARDNUM_MEMORY == i)
				{

				}
				else
				{
					printf("CARDNUM_save ok : %d\r\n", i);
				}
			}
		break;
		case CONFIG_FUNC_DELETE_CARDNUM : //카드 번호 삭제
			if(len == 8)
			{
				for(i = 0; i<MAX_CARDNUM_MEMORY ; i++)
				{
					if(true == ee24_read(MEM_ADDR_CARDNUM_INDEX_MSB(i), rxtemp, 16, 100))
					{
						for(j = 0; j<8;j++)
						{
							if(temp_data[j] != rxtemp[8+j]) //카드 번호가 다를경우
							{
								break;
							}
						}

						if(8 == j) //초기화
						{
							txtemp[0] = 0xFF;
							txtemp[1] = 0xFF;
							txtemp[2] = 0xFF;
							txtemp[3] = 0xFF;
							txtemp[4] = 0xFF;
							txtemp[5] = 0xFF;
							txtemp[6] = 0xFF;
							txtemp[7] = 0xFF;
							txtemp[8] = 0xFF;
							txtemp[9] = 0xFF;
							txtemp[10] = 0xFF;
							txtemp[11] = 0xFF;
							txtemp[12] = 0xFF;
							txtemp[13] = 0xFF;
							txtemp[14] = 0xFF;
							txtemp[15] = 0xFF;

							ee24_write(MEM_ADDR_CARDNUM_INDEX_MSB(i), txtemp, 16, 100); //EEPROM에 쓰기

							break;
						}
					}
				}

				if(MAX_CARDNUM_MEMORY == i)
				{

				}
				else
				{
					printf("CARDNUM_delete ok : %d\r\n", i);
				}
			}
		break;


		default :
			ret_value = _FALSE;
		break;
	}

	return ret_value;
}

uint8_t config_write_pm_command(uint8_t func,uint8_t* temp_data)
{

	return 0;
}

void config_write_command_parser()
{
	uint8_t func = 0x00, ln = 0x00;
	uint8_t temp_data[50] = {'\0',};
	uint8_t i;
	uint8_t ret_value = _FALSE;

	func = _LIB_U8QUEUE_get_byte(&config_queue);
	ln = _LIB_U8QUEUE_get_byte(&config_queue);

	for (i = 0; i < ln; i++)
	{
		temp_data[i] = _LIB_U8QUEUE_get_byte(&config_queue);
	}

	if((CONFIG_FUNC_COMMON_BASE <= func) && (func <= CONFIG_FUNC_COMMON_END))
	{
		ret_value = config_write_common_command(func,temp_data,ln);

		if(_TRUE == ret_value)
		{
			_APP_SYSTEMCTL_req(CONFIG_WRITE_COMMAND, func, 0, NULL);
		}
	}
}

uint8_t config_read_pm_command(uint8_t func)
{

	return 0;
}

uint8_t config_read_common_command(uint8_t func, uint8_t* temp_data, uint8_t *len)
{
	uint8_t ret_value = _TRUE;

	switch(func)
	{
		case CONFIG_FUNC_SAVE_CARDNUM_COUNT :

		break;

		default :
			ret_value = _FALSE;
		break;
	}

	return ret_value;
}

void config_read_command_parser()
{
	uint8_t func = 0x00;
	uint8_t temp_data[50] = {'\0', };
	uint8_t temp_len = 0;
	uint8_t ret_value = _FALSE;

	func = _LIB_U8QUEUE_get_byte(&config_queue);

	if((CONFIG_FUNC_COMMON_BASE <= func) && (func <= CONFIG_FUNC_COMMON_END))
	{
		ret_value = config_read_common_command(func,temp_data,&temp_len);

		if(_TRUE == ret_value)
		{
			_APP_SYSTEMCTL_req(CONFIG_READ_COMMAND, func, temp_len, temp_data);
		}
	}
}

void config_main_processing()
{
	uint8_t cmd = 0x00, uch = 0xff, ucl = 0xff;

	cmd = _LIB_U8QUEUE_get_byte(&config_queue);

	switch(cmd)
	{
		case CONFIG_WRITE_COMMAND:
			config_write_command_parser();
		break;
		case CONFIG_READ_COMMAND:
			config_read_command_parser();
		break;
		default:
			_LIB_U8QUEUE_comFlush(&config_queue);
		return;
	}

	ucl = _LIB_U8QUEUE_get_byte(&config_queue);
	uch = _LIB_U8QUEUE_get_byte(&config_queue);
}

//온도 75도 이상 시 오류 플래그 설정 함수
void check_temp_loop()
{
	int16_t temp_value = 0;
	static uint8_t over_temp_flag = _FALSE;


	if(_TRUE == _MW_NTC_loop())
	{
#if ((_CERTIFICATION_MODE_)==_CERTIFICATION_KC_EVSE_)
		temp_value = 0;
#else
		temp_value = _MW_NTC_get_temp();
#endif

		if((temp_value >= OVER_TEMP_THRESHOLD) && (over_temp_flag == _FALSE)) //temp_value = 75 OVER
		{
#if 0
			if(_OFF == config_app.send_msg_flag)
			{
				over_temp_flag = _TRUE;
				config_app.send_msg_flag = _ON;
				config_app.send_msg_cmd = ITC_CONFIGAPP_TO_CHARGSERV_STATUS_CMD;
				config_app.send_msg_length = ITC_CONFIGAPP_TO_CHARGSERV_STATUS_LENGTH;
				config_app.send_msg_data[ITC_CONFIGAPP_TO_CHARGSERV_STATUS_INDEX] = ITC_CONFIGAPP_TO_CHARGSERV_STATUS_OVER_TEMP;

				config_app.send_msg_mq = ChargServMQHandle;
			}
			
#else
			over_temp_flag = _TRUE;
			_APP_CHARGSERV_over_temperature_fault_set();
#endif
		}
		else if((temp_value < NORMAL_TEMP_THRESHOLD) && (over_temp_flag == _TRUE))
		{
#if 0
			if(_OFF == config_app.send_msg_flag)
			{
				over_temp_flag = _FALSE;
				config_app.send_msg_flag = _ON;
				config_app.send_msg_cmd = ITC_CONFIGAPP_TO_CHARGSERV_STATUS_CMD;
				config_app.send_msg_length = ITC_CONFIGAPP_TO_CHARGSERV_STATUS_LENGTH;
				config_app.send_msg_data[ITC_CONFIGAPP_TO_CHARGSERV_STATUS_INDEX] = ITC_CONFIGAPP_TO_CHARGSERV_STATUS_NOR_TEMP;

				config_app.send_msg_mq = ChargServMQHandle;
			}
#else
			over_temp_flag = _FALSE;
			_APP_CHARGSERV_over_temperature_fault_reset();
#endif
		}
	}
}

void _APP_SYSTEMCTL_located_in_timer()
{
	if(config_app.found_frame) //유효 프레임이 있을 때마다 main_processing 실행
	{
		config_main_processing();
		config_app.found_frame = 0;
	}
}

void _APP_flash_init()
{

}

#if 0
uint8_t _APP_flash_sector_write_data(uint8_t *pBuffer, uint32_t Sector_num)
{
	// pBuffer data size = flash sector size(fixed)
	// You must use the same pBuffer as the sector size.

	uint32_t backup_Sector_num = 0;
	uint32_t data_size = acevse.flash_info.sector_size;

	// flash write process
	// x : Sector_num
	// backup_x : backup Sector num = max_Sector_size/2 + x
	backup_Sector_num = acevse.flash_info.sector_count / 2 + Sector_num;

	// #1. Erase Sector 'backup_x'
	_MW_SFLASH_W25qxx_EraseSector(backup_Sector_num);

	// #2. Write Data Sector 'backup_x'
	_MW_SFLASH_W25qxx_WriteSector(pBuffer, backup_Sector_num, 0, data_size);

	// #3. Erase Sector 'x'
	_MW_SFLASH_W25qxx_EraseSector(Sector_num);

	// #4. Copy Sector 'backup_x' to Sector 'x'
	_MW_SFLASH_W25qxx_ReadSector(acevse.flash_info.copy_data_buf, backup_Sector_num, 0, data_size);
	_MW_SFLASH_W25qxx_WriteSector(acevse.flash_info.copy_data_buf, Sector_num, 0, data_size);

	return _TRUE;
}
#else
uint8_t _APP_flash_sector_write_data(uint8_t *pBuffer, uint32_t Sector_num)
{


	return _TRUE;
}
#endif

uint8_t _APP_flash_sector_read_data(uint8_t *pBuffer, uint32_t Sector_num)
{

	return _TRUE;
}



void _APP_SYSTEMCTL_write_memory(uint32_t Sector_num, void* value, uint32_t length)
{

}

void _APP_SYSTEMCTL_read_memory(uint32_t Sector_num, void* value, uint32_t length)
{

}

void _APP_SYSTEMCTL_memory_loop()
{

}

/*
 *
 */
uint8_t _APP_SYSTEMCTL_check_cardnum(uint8_t* cardnumbuf)
{
	uint8_t ret_value = _FALSE;
	int i = 0;
	int j = 0;
	uint8_t rxtemp[30] = {0,};

	for(i=0;i<MAX_CARDNUM_MEMORY;i++)
	{
		if(true == ee24_read(MEM_ADDR_CARDNUM_VALUE_MSB(i), rxtemp, 8, 100))
		{
			for(j=0;j<8;j++)
			{
				if(cardnumbuf[j] != rxtemp[j])
				{
					break;
				}
			}

			if(8==j)
			{
				ret_value = _TRUE;
				break;
			}
		}
	}

	return ret_value;
}

void _APP_SYSTEMCTL_startup()
{
	int i = 0;
	int j = 0;
	uint8_t rxtemp[30] = {0,};

	_APP_flash_init();


	/*
	 * MEM_SECTOR_COMMON_CONFIG
	 */

	printf("------------------------------------------------\r\n");

	for(i=0;i<MAX_CARDNUM_MEMORY;i++)
	{
		if(true == ee24_read(MEM_ADDR_CARDNUM_INDEX_MSB(i), rxtemp, 16, 100))
		{
			//printf("0x%02x\r\n",rxtemp[7]);
			if((i+1) == rxtemp[7])
			{
				printf("--%d -- %02x%02x %02x%02x %02x%02x %02x%02x --\r\n",
						rxtemp[7],
						rxtemp[8],
						rxtemp[9],
						rxtemp[10],
						rxtemp[11],
						rxtemp[12],
						rxtemp[13],
						rxtemp[14],
						rxtemp[15]);

				config_app.usecardnum[i][0] = rxtemp[8];
				config_app.usecardnum[i][1] = rxtemp[9];
				config_app.usecardnum[i][2] = rxtemp[10];
				config_app.usecardnum[i][3] = rxtemp[11];
				config_app.usecardnum[i][4] = rxtemp[12];
				config_app.usecardnum[i][5] = rxtemp[13];
				config_app.usecardnum[i][6] = rxtemp[14];
				config_app.usecardnum[i][7] = rxtemp[15];
			}
			else
			{
				printf("                                                    \r\n");
				config_app.usecardnum[i][0] = 0xFF;
				config_app.usecardnum[i][1] = 0xFF;
				config_app.usecardnum[i][2] = 0xFF;
				config_app.usecardnum[i][3] = 0xFF;
				config_app.usecardnum[i][4] = 0xFF;
				config_app.usecardnum[i][5] = 0xFF;
				config_app.usecardnum[i][6] = 0xFF;
				config_app.usecardnum[i][7] = 0xFF;
			}
		}
	}

	printf("------------------------------------------------\r\n");

#if ((_CERTIFICATION_MODE_)==_CERTIFICATION_KC_EVSE_)
	_LIB_LOGGING_printf("CERTIFICATION : KC_EVSE \r\n");
#elif((_CERTIFICATION_MODE_)==_CERTIFICATION_TYPE_EVSE_)
	_LIB_LOGGING_printf("CERTIFICATION : TYPE_EVSE \r\n");
#elif((_CERTIFICATION_MODE_)==_CERTIFICATION_KC_FLUG_)
	_LIB_LOGGING_printf("CERTIFICATION : KC_FLUG \r\n");
#elif((_CERTIFICATION_MODE_)==_CERTIFICATION_TYPE_FLUG_)
	_LIB_LOGGING_printf("CERTIFICATION : TYPE_FLUG \r\n");
#endif


	_LIB_USERDELAY_set(&gDelay_systemctl_comm_periodic_loop_time,5);
}

void _APP_SYSTEMCTL_main()
{
	_APP_SYSTEMCTL_framesearch();
	_APP_SYSTEMCTL_located_in_timer();

	_APP_SYSTEMCTL_memory_loop();
}

void _APP_SYSTEMCTL_get_event(void)
{


}

void _APP_SYSTEMCTL_set_event(void)
{
#if 0
	osMessageQueueId_t temp_mq;
	uint8_t i;

	if(_ON == config_app.send_msg_flag)
	{
		config_app.send_msg_flag = _OFF;

		msgtx->cmd = config_app.send_msg_cmd;
		msgtx->length = config_app.send_msg_length;
		for(i=0;i<msgtx->length;i++)
		{
			msgtx->data[i] = config_app.send_msg_data[i];
		}

		temp_mq = config_app.send_msg_mq;
		_LIB_FREERTOS_message_send(temp_mq, msgtx);
	}
#endif
}

//uint16_t zctadc = 0;

void _APP_SYSTEMCTL_process()
{
	_LIB_USERDELAY_start(&gDelay_systemctl_comm_periodic_loop_time, DELAY_RENEW_OFF);

    if(_LIB_USERDELAY_isfired(&gDelay_systemctl_comm_periodic_loop_time))
    {
    	_APP_SYSTEMCTL_get_event();

		_MW_GPIO_update();
		check_temp_loop();

		//_APP_CHARGSERV_check_Vrms_loop();
		//_APP_CHARGSERV_check_Irms_loop();

    	_APP_SYSTEMCTL_main();

    	_APP_SYSTEMCTL_set_event();

    	//HAL_IWDG_Refresh(&hiwdg);

	  	_LIB_USERDELAY_start(&gDelay_systemctl_comm_periodic_loop_time, DELAY_RENEW_ON);
    }
}



