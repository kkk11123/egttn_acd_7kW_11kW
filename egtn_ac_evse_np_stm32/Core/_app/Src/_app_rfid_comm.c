/*
 * _app_rfid_comm.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */


#include <_app_rfid_comm.h>
#include <_app_charging_service.h>

#include <_lib_bitop.h>
#include <_lib_logging.h>
#include <_lib_userdelay.h>

#include <_config.h>

static uint8_t rfid_ring_buffer[RFID_RING_BUFFER_SIZE] = {'\0', };
static s__LIB_QUEUE rfid_queue = {rfid_ring_buffer, RFID_RING_BUFFER_SIZE, 0, RFID_RING_BUFFER_SIZE, 0};
static s__APP_RFID rfid_app = {0, 0, {0, }, {'\0', }, 0, RFID_Init};

static gUserDelay gDelay_rfid_comm_periodic_loop_time;
static gUserDelay gTimeout_rfid_comm_fault;
static gUserDelay gDelay_rfid_finish;

static const uint16_t CRC16_BUYPASS_TABLE[] =
{
	0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
	0x8033, 0x0036, 0x003C,	0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
	0x8063, 0x0066,	0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
	0x0050, 0x8055, 0x805F,	0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
	0x80C3, 0x00C6, 0x00CC, 0x80C9,	0x00D8, 0x80DD, 0x80D7, 0x00D2,
	0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB,	0x00EE, 0x00E4, 0x80E1,
	0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE,	0x00B4, 0x80B1,
	0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087,	0x0082,
	0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
	0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
	0x01E0,	0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
	0x81D3, 0x01D6,	0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
	0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
	0x8173, 0x0176, 0x017C, 0x8179,	0x0168, 0x816D, 0x8167, 0x0162,
	0x8123, 0x0126, 0x012C, 0x8129, 0x0138,	0x813D, 0x8137, 0x0132,
	0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E,	0x0104, 0x8101,
	0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317,	0x0312,
	0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
	0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
	0x8353,	0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
	0x03C0, 0x83C5,	0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
	0x83F3, 0x03F6, 0x03FC,	0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
	0x83A3, 0x03A6, 0x03AC, 0x83A9,	0x03B8, 0x83BD, 0x83B7, 0x03B2,
	0x0390, 0x8395, 0x839F, 0x039A, 0x838B,	0x038E, 0x0384, 0x8381,
	0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E,	0x0294, 0x8291,
	0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7,	0x02A2,
	0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
	0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
	0x8243,	0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
	0x0270, 0x8275,	0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
	0x0220, 0x8225, 0x822F,	0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
	0x8213, 0x0216, 0x021C, 0x8219,	0x0208, 0x820D, 0x8207, 0x0202
};

void rfid_comPut(uint8_t *data, uint8_t cnt)
{
	_MW_UART_tx01(data, cnt);
}

uint16_t buypass_crc16 (uint8_t *nData, uint8_t length)
{
	uint8_t nTemp;
    uint16_t wCRCWord = 0x0000;

    while (length--)
    {
    	nTemp = *nData++ ^ (wCRCWord>>8);
    	wCRCWord = wCRCWord << 8;
    	wCRCWord ^= CRC16_BUYPASS_TABLE[nTemp];
    }
    return wCRCWord;
}

void rfid_frame_search() //slave -> master에게 보내는 데이터 검증
{
	uint8_t uch, ucl, bh, bl;
	uint16_t mclen, crc_data = 0;
	uint8_t crc_calc[30];
	uint16_t temp_pointer;

	mclen = _LIB_U8QUEUE_comLen(&rfid_queue);

	if(mclen < 8) return;

	//printf("mclen:%d\r\n",mclen);

	temp_pointer = rfid_queue.action_pointer;

	_LIB_U8QUEUE_inc_pointer(&rfid_queue); //STX

	for (uint8_t i=0; i < mclen-4; i++){ //-4를 한 이유는 crc 16
		crc_calc[i] = _LIB_U8QUEUE_get_byte(&rfid_queue);
		//printf("data:0x%02x\r\n",crc_calc[i]);
	}

	bh = _LIB_U8QUEUE_get_byte(&rfid_queue);
	bl = _LIB_U8QUEUE_get_byte(&rfid_queue);

	crc_data = buypass_crc16(crc_calc, mclen-4);
	//printf("crc:0x%04x\r\n",crc_data);
	uch = crc_data >> 8;
	ucl = crc_data;

	rfid_queue.action_pointer = temp_pointer;

	if((bh == uch) && (bl == ucl))
	{
		rfid_app.found_frame = 1;
		//printf("frame\r\n");
	}
	else
	{


		//printf("bh:0x%02x\r\n",bh);
		//printf("uch:0x%02x\r\n",uch);
		//printf("bl:0x%02x\r\n",bl);
		//printf("ucl:0x%02x\r\n",ucl);
	}
}

void rfid_read_card_id() ////rfid_rx_buf[3]~[6]에 카드 ID 16자리 저장
{
	uint16_t ln;

	ln = _LIB_U8QUEUE_get_word(&rfid_queue);
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Type
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Reserve_#01
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Reserve_#02
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Reserve_#03
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Reserve_#04

	//Number of Card numbers is 8 bytes.
	for (uint8_t i = RFID_CARD_ID_01; i <= RFID_CARD_ID_04; i++){
		rfid_app.rfid_rx_buf[i] = _LIB_U8QUEUE_get_word(&rfid_queue);
	}

	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Reserve_#05
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Reserve_#06
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // CRC Hi
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // CRC Low
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // ETX
}

void rfid_reader_terminate()//cmd가 0xE3일 때 태그 종료 플래그 1
{
	uint16_t ln;
	ln = _LIB_U8QUEUE_get_word(&rfid_queue);
	if(RFID_TERMINATE == _LIB_U8QUEUE_get_byte(&rfid_queue))
	{
		rfid_app.reg.tag_terminate = 1;
	}
	else
	{
		//Todo
		//Fault
	}
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // CRC Hi
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // CRC Low
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // ETX
}

void rfid_reader_version()//rfid_rx_buf[1]~[2]에 버전 값 저장
{
	uint8_t ln;

	ln = _LIB_U8QUEUE_get_word(&rfid_queue);

	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // ACK

	for (uint8_t i = RFID_H_VERSION; i <= RFID_L_VERSION; i++){
		rfid_app.rfid_rx_buf[i] = _LIB_U8QUEUE_get_word(&rfid_queue);
	}

	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Reserve_#01
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Reserve_#02
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Reserve_#03
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // Reserve_#04

	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // CRC Hi
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // CRC Low
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // ETX
}

void rfid_reader_status() //rfid_rx_buf[0]에 동작 상태 값 저장
{
	uint8_t ln;

	ln = _LIB_U8QUEUE_get_word(&rfid_queue);
	rfid_app.rfid_rx_buf[RFID_STATUS] = _LIB_U8QUEUE_get_word(&rfid_queue);

	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // CRC Hi
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // CRC Low
	_LIB_U8QUEUE_inc_pointer(&rfid_queue); // ETX
}



void _APP_RFID_push_ringbuffer(uint8_t comdt) //data를 rfid_queue에 넣는 함수
{
	if(rfid_queue.blank_check > 50)	{
		_LIB_U8QUEUE_comFlush(&rfid_queue);
		printf("rfid_comFlush\r\n");
	}
	rfid_queue.blank_check = 0;

	_LIB_U8QUEUE_push_ring(&rfid_queue, comdt);
	//rfid_frame_search();
}

void _APP_RFID_req(uint8_t cmd) //slave에게 Request 함수
{
	uint16_t crc_data = 0;

	rfid_app.rfid_tx_cnt = 0;

	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = RFID_STX;
	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = RFID_SEQ_REQ;
	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = RFID_SIDX_H; //0XB0이여야될 것같은데..
	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = RFID_SIDX_L;
	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = RFID_RIDX_H;
	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = RFID_RIDX_L;
	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = cmd;
	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = 0x00;
	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = 0x00;

	crc_data = buypass_crc16(&rfid_app.rfid_tx_buf[1], rfid_app.rfid_tx_cnt-1); //Sequence Number를 가지고 CRC 계산
	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = crc_data >> 8;
	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = crc_data;

	rfid_app.rfid_tx_buf[rfid_app.rfid_tx_cnt++] = RFID_ETX;

	rfid_comPut(rfid_app.rfid_tx_buf, rfid_app.rfid_tx_cnt);
}

uint8_t _APP_RFID_save_card_num(uint16_t* card_num) //card_num[0] ~ [3]에 카드 번호 저장
{
	uint8_t cnt = 0;

	//Todo
	//This solution is not good.
	if((rfid_app.rfid_rx_buf[RFID_CARD_ID_01] == 0x00)
			&&  (rfid_app.rfid_rx_buf[RFID_CARD_ID_04] == 0x00)){
		return 0;
	}
	else{
		for (uint8_t i = RFID_CARD_ID_01; i <= RFID_CARD_ID_04; i++){
			card_num[cnt++] = rfid_app.rfid_rx_buf[i];

			//Initialize zero
			rfid_app.rfid_rx_buf[i] = 0;
		}
		return 1;
	}
}

uint8_t _APP_RFID_get_stus()
{
	return (uint8_t)rfid_app.rfid_rx_buf[RFID_STATUS];
}

uint32_t _APP_RFID_get_version() //버전을 4바이트로 한번에 가져오기
{
	uint32_t card_reader_version = 0x0;
	card_reader_version = rfid_app.rfid_rx_buf[RFID_H_VERSION];
	card_reader_version <<= 16;
	card_reader_version |= rfid_app.rfid_rx_buf[RFID_L_VERSION];
	return card_reader_version;
}


uint8_t rfid_set_state(eRFID_State cstate)
{
	rfid_app.state = cstate;
	return _TRUE;
}

eRFID_State rfid_get_current_state()
{
	return rfid_app.state;
}


void rfid_main_processing() //slave -> master
{
	uint8_t cmd;
	eRFID_State state = rfid_get_current_state();

	 //STX
	if(RFID_STX != _LIB_U8QUEUE_get_byte(&rfid_queue)){
		_LIB_U8QUEUE_comFlush(&rfid_queue);
		return;
	}
	if(RFID_SEQ_RESP != _LIB_U8QUEUE_get_byte(&rfid_queue)){
		_LIB_U8QUEUE_comFlush(&rfid_queue);
		return;
	}
	if(RFID_RIDX_H != _LIB_U8QUEUE_get_byte(&rfid_queue)){
		_LIB_U8QUEUE_comFlush(&rfid_queue);
		return;
	}
	if(RFID_RIDX_L != _LIB_U8QUEUE_get_byte(&rfid_queue)){
		_LIB_U8QUEUE_comFlush(&rfid_queue);
		return;
	}
	if(RFID_SIDX_H != _LIB_U8QUEUE_get_byte(&rfid_queue)){
		_LIB_U8QUEUE_comFlush(&rfid_queue);
		return;
	}
	if(RFID_SIDX_L != _LIB_U8QUEUE_get_byte(&rfid_queue)){
		_LIB_U8QUEUE_comFlush(&rfid_queue);
		return;
	}

	cmd = _LIB_U8QUEUE_get_byte(&rfid_queue);

	switch(cmd){ //명령에 따라서
		case 0xd6:
			rfid_reader_terminate(); //rfid_app.reg.tag_terminate = 1, 동작 중지 플래그
			break;
		case 0xd7:
			rfid_reader_version(); //rfid_rx_buf[1]~[2]에 버전 값 저장
			break;
		case 0xd8:
			rfid_reader_status(); //rfid_rx_buf[0]에 동작 상태 값 저장
			break;
		case 0xea:
			rfid_read_card_id(); //rfid_rx_buf[3]~[6]에 카드 번호 값 저장
#if 1
			//test
			_LIB_LOGGING_printf("CARD ID : %04x %04x %04x %04x \r\n",
										rfid_app.rfid_rx_buf[RFID_CARD_ID_01],
										rfid_app.rfid_rx_buf[RFID_CARD_ID_02],
										rfid_app.rfid_rx_buf[RFID_CARD_ID_03],
										rfid_app.rfid_rx_buf[RFID_CARD_ID_04]);
#endif
			if(state == RFID_Start) //RFID 시작일 경우
			{
				if(_ON == _APP_RFID_save_card_num(rfid_app.cardnum)) //rfid_app.cardnum[0] ~ [3]에 카드번호 16자리 저장(2byte씩)
				{
					//rfid_write_reg_bit(RFIDAPP_REG_TAG_OK, _ON);
					rfid_app.reg.tag_ok = 1; //저장 완료 시 tag_ok
					//rfid_write_reg_bit(RFIDAPP_REG_TAG_ERROR, _OFF);
					rfid_app.reg.tag_error = 0;
				}
				else
				{
					//rfid_write_reg_bit(RFIDAPP_REG_TAG_OK, _OFF);
					rfid_app.reg.tag_ok = 0;
					//rfid_write_reg_bit(RFIDAPP_REG_TAG_ERROR, _ON);
					rfid_app.reg.tag_error = 1;
				}
			}

			break;
		case 0xeb:
			rfid_read_card_id();
			break;
		default:
			_LIB_U8QUEUE_comFlush(&rfid_queue);
			return;
	}
}

void _APP_RFID_located_in_timer()
{
	if(rfid_app.found_frame) //유효 프레임 찾으면
	{

		_LIB_USERDELAY_stop(&gTimeout_rfid_comm_fault); //오류 타이머 정지

		rfid_queue.blank_check = 0;
		rfid_app.interval_counter++;
		if(rfid_app.interval_counter > RETURN_INTERVAL)
		{
			rfid_app.interval_counter = 0;
			rfid_app.found_frame = 0;
			rfid_main_processing();
		}
	}
	//if(rfid_queue.blank_check < 0xffff) rfid_queue.blank_check++;
}


void _APP_RFID_state_machine()
{
	eRFID_State state = rfid_get_current_state();

	switch(state)
	{
		case RFID_Init :
			if(1 == rfid_app.reg.init_ok) //start_up 함수에서 자동으로 1 -> 딜레이 타이머 set 완료
			{
				//rfid_app.reg.init_ok = 0;
				rfid_set_state(RFID_Standby);
			}
		break;

		case RFID_Standby :
			//if(_ON == rfid_read_reg_bit(RFIDAPP_REG_TAG_START))
			if(1 == rfid_app.reg.tag_start) //set_start_tagging_mode(0)이면 1
			{
				rfid_app.reg.tag_start = 0;
				_APP_RFID_req(0xEA); //카드 읽기 무한 대기 -> 리더기 무한 활성화
				rfid_set_state(RFID_Start);
				rfid_app.reg.tag_ok = 0;
				printf("rfid_tagging start\r\n");
			}
		break;

		case RFID_Start :
			//if(_ON == rfid_read_reg_bit(RFIDAPP_REG_TAG_OK))
			if(1 == rfid_app.reg.tag_ok) //rfid_app.cardnum[0] ~ [3]에 카드번호 16자리 저장(2byte씩) 완료 시
			{
				rfid_app.reg.tag_ok = 0;
				rfid_app.reg.data_standby = 1;
				rfid_set_state(RFID_Tagging);
				//rfid_write_reg_bit(RFIDAPP_REG_MSG_TX_ENABLE, _ON);
				printf("rfid_tagging\r\n");
			}
			else if((1 == rfid_app.reg.tag_error) || (1 == rfid_app.reg.tag_timeout))
			{
				//rfid_write_reg_bit(RFIDAPP_REG_MSG_TX_ENABLE, _ON);
				rfid_set_state(RFID_Error);
			}
		break;

		case RFID_Tagging :
			if(1 == rfid_app.reg.read_data) //rfid_app.cardnum[0]~[3]에 2byte씩 저장되어 있던 카드 번호를 cardnum[0] ~ [7]에 1byte씩 저장 완료 시
			{
				rfid_app.reg.read_data = 0;
				rfid_set_state(RFID_Finish);
			}
		break;
		case RFID_Error :
			if((0 == rfid_app.reg.tag_error) || (0 == rfid_app.reg.tag_timeout))
			{
				rfid_set_state(RFID_Finish);
			}
		break;
		case RFID_Terminate :
			if(1 == rfid_app.reg.tag_terminate) //cmd가 0XE3일 때 태그 종료
			{
				printf("#### rfid_terminate_ok ####\r\n");
				rfid_app.reg.tag_terminate = 0;
				rfid_set_state(RFID_Finish);
			}
		break;
		case RFID_Finish : //RFID 종료 상태에서 2초 뒤 다시 standby 상태로 변환
			if(_LIB_USERDELAY_start(&gDelay_rfid_finish, DELAY_RENEW_OFF))
			{
				printf("#### rfid_finish_delay_start ####\r\n");
			}

			if(_LIB_USERDELAY_isfired(&gDelay_rfid_finish)) //2초
			{
				_LIB_USERDELAY_stop(&gDelay_rfid_finish);
				printf("#### rfid_finish_delay_ok ####\r\n");
				rfid_set_state(RFID_Standby);
			}

		break;
		default :

		break;
	}
}

 
void _APP_RFID_set_start_tagging(uint8_t mode)
{
	eRFID_State mstate = rfid_get_current_state();

	if(0 == mode) //mode가 0이면 리더기 활성화
	{
		rfid_app.reg.tag_error = 0;
		rfid_app.reg.tag_timeout = 0;
		rfid_app.reg.tag_start = 1;
		printf("#### rfid_set start #### \r\n");
	}
	else if(1 == mode) //mode가 1이면 리더기 동작 중지
	{
		if(RFID_Start == mstate)
		{
			printf("#### rfid_terminate start #### \r\n");
			rfid_app.reg.tag_start = 0;
			_APP_RFID_req(0xD6); //RFID START인데 cmd가 0xD6면 동작 중지
			rfid_set_state(RFID_Terminate);
		}
	}
}

uint8_t _APP_RFID_get_tagging_result() //태깅 결과 반환
{
	uint8_t ret_value = _CONTINUE;

	if(1 == rfid_app.reg.data_standby) //rfid_app.cardnum[0] ~ [3]에 카드번호 16자리 저장(2byte씩) 완료 시
	{
		ret_value = _TRUE;
	}
	else if((1 == rfid_app.reg.tag_error) || (1 == rfid_app.reg.tag_timeout))
	{
		ret_value = _FALSE;
	}

	return ret_value;
}

#if 0
void _APP_RFID_get_card_number(char * cardnum)
{
	//220517 PES : Todo
	//get card number function

	sprintf((char *)cardnum,"%04x%04x%04x%04x"
			,rfid_app.cardnum[0]
			,rfid_app.cardnum[1]
			,rfid_app.cardnum[2]
			,rfid_app.cardnum[3]);
	printf("card : %s \r\n",cardnum);

	rfid_app.reg.data_standby = 0;
	rfid_app.reg.read_data = 1;
}
#else
void _APP_RFID_get_card_number(uint8_t * cardnum) //2byte씩 저장되어있는 카드 번호를 1byte로 나눠서 저장
{												//cardnum[0]~[7]
	//220517 PES : Todo
	//get card number function

	cardnum[0] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[0], _FILED_MSB_);
	cardnum[1] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[0], _FILED_LSB_);
	cardnum[2] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[1], _FILED_MSB_);
	cardnum[3] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[1], _FILED_LSB_);
	cardnum[4] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[2], _FILED_MSB_);
	cardnum[5] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[2], _FILED_LSB_);
	cardnum[6] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[3], _FILED_MSB_);
	cardnum[7] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[3], _FILED_LSB_);

	printf("%02x%02x %02x%02x %02x%02x %02x%02x \r\n",
			cardnum[0],
			cardnum[1],
			cardnum[2],
			cardnum[3],
			cardnum[4],
			cardnum[5],
			cardnum[6],
			cardnum[7]);

	rfid_app.reg.data_standby = 0;
	rfid_app.reg.read_data = 1;
}
#endif

void _APP_RFID_get_event(void)
{
#if 0
	eRFID_State state = rfid_get_current_state();

	switch(state)
	{
		case RFID_Init :

		break;
		case RFID_Standby :
			if(_APP_RFID_get_start_tagging())
			{
				rfid_write_reg_bit(RFIDAPP_REG_TAG_START, _ON);
				//_LIB_LOGGING_printf("rfid_tagstart\r\n");
			}
		break;
		default :

		break;
	}
#endif
}

void _APP_RFID_set_event(void)
{
#if 0
	eRFID_State state = rfid_get_current_state();

	switch(state)
	{
		case RFID_Init :

		break;

		case RFID_Tagging :
			if()
			{

			}
			msgtx->cmd = ITC_RFIDAPP_CARDNUM_CMD;
			msgtx->length = ITC_RFIDAPP_CARDNUM_LENGTH;
			msgtx->data[0] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[0], _FILED_MSB_);
			msgtx->data[1] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[0], _FILED_LSB_);
			msgtx->data[2] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[1], _FILED_MSB_);
			msgtx->data[3] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[1], _FILED_LSB_);
			msgtx->data[4] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[2], _FILED_MSB_);
			msgtx->data[5] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[2], _FILED_LSB_);
			msgtx->data[6] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[3], _FILED_MSB_);
			msgtx->data[7] = _LIB_BITOP_ext_u16_to_u8one(rfid_app.cardnum[3], _FILED_LSB_);
			temp_mq = ChargServMQHandle;
			//printf("rfid-->chargserv : tagging ok\r\n");
		break;
		case RFID_Error :
			msgtx->cmd = ITC_RFIDAPP_STATE_CMD;
			msgtx->length = ITC_RFIDAPP_STATE_LENGTH;
			msgtx->data[ITC_RFIDAPP_STATE_INDEX] = ITC_RFIDAPP_STATE_TAG_ERROR;
			temp_mq = ChargServMQHandle;
		break;

		default :

		break;
	}
#endif
}

uint8_t _APP_RFID_is_init_ok()
{
	uint8_t ret_value = _FALSE;

	if(1 == rfid_app.reg.init_ok)
	{
		ret_value = _TRUE;
	}

	return ret_value;
}

void _APP_RFID_check_status_loop()
{
	static uint16_t tick = 0;

	tick++;
	if(tick >= 100)
	{
		tick = 0;
		_APP_RFID_req(0xD7); //버전 확인, rfid_app.rfid_rx_buf[1]~[2]에 버전 정보 값 저장
		//printf("0xD7\r\n");
		_LIB_USERDELAY_start(&gTimeout_rfid_comm_fault, DELAY_RENEW_OFF); // found_frame시 타이머 정지
	}

	if(_LIB_USERDELAY_isfired(&gTimeout_rfid_comm_fault)) //20초 경과 시
	{
		//rfid_comm_fault_set
		_APP_CHARGSERV_rfid_fault_set();
	}
	else
	{
		//rfid_comm_fault_reset
		_APP_CHARGSERV_rfid_fault_reset();
	}
}

void _APP_RFID_main()
{
	rfid_frame_search(); //유효 프레임 찾기

	_APP_RFID_located_in_timer(); //main_processing 시작

	_APP_RFID_state_machine();

	//_APP_RFID_check_status_loop();
}

void _APP_RFID_startup()
{
	//rfid_write_reg_bit(RFIDAPP_REG_MSG_TX_ENABLE, _ON);
	rfid_set_state(RFID_Init);

	rfid_app.reg.init_ok = 1;
	_LIB_USERDELAY_set(&gDelay_rfid_comm_periodic_loop_time,100);
	_LIB_USERDELAY_set(&gTimeout_rfid_comm_fault, 20000);
	_LIB_USERDELAY_set(&gDelay_rfid_finish,2000);
}

void _APP_RFID_init()
{
	//Todo : init function - check version packet data is not empty??
}

void _APP_RFID_comm_process()
{
	_LIB_USERDELAY_start(&gDelay_rfid_comm_periodic_loop_time, DELAY_RENEW_OFF);

    if(_LIB_USERDELAY_isfired(&gDelay_rfid_comm_periodic_loop_time))
    {
    	_APP_RFID_get_event();

    	_APP_RFID_main();

    	_APP_RFID_set_event();

	  	_LIB_USERDELAY_start(&gDelay_rfid_comm_periodic_loop_time, DELAY_RENEW_ON);
    }
}


