/**
 ****************************************************************************************
 *
 * @file user_custs1_impl.c
 *
 * @brief Peripheral project Custom1 Server implementation source code.
 *
 * Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
 * All rights reserved. Confidential Information.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "string.h"

#include "gpio.h"
#include "app_api.h"
#include "app.h"
#include "prf_utils.h"
#include "custs1.h"
#include "custs1_task.h"
#include "user_custs1_def.h"
#include "user_custs1_impl.h"
#include "user_peripheral.h"
#include "user_periph_setup.h"
#include "adc.h"
#include "epd.h"
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

ke_msg_id_t timer_used __SECTION_ZERO("retention_mem_area0");      //@RETENTION MEMORY
uint16_t indication_counter __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint16_t non_db_val_counter __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

uint8_t moshi __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t firstime __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint16_t adcval __SECTION_ZERO("retention_mem_area0");


MCU_Object_t retention_objects[20] __SECTION_ZERO("retention_mem_area0");
uint8_t ble_rx_buffer[3072] __SECTION_ZERO("retention_mem_area0");
uint8_t obj_visible __SECTION_ZERO("retention_mem_area0");
uint16_t ble_rx_offset __SECTION_ZERO("retention_mem_area0");
uint16_t targetlength __SECTION_ZERO("retention_mem_area0");
uint16_t obj_next_add __SECTION_ZERO("retention_mem_area0");
uint16_t obj_data_wr_length __SECTION_ZERO("retention_mem_area0");
uint8_t obj_data[8192]; // 16kb ram luu obj chua tinh font 



int32_t obj_data_arrangement(uint8_t type ,uint8_t* incoming_data, uint16_t len)
{
	
	uint32_t obj_last_add=obj_next_add;
	if((obj_next_add+len)<6144)
	{
			if(type==1) 
			{
					memcpy(&obj_data[obj_next_add], incoming_data, len);
					obj_data[obj_next_add+len]='\0';
		     	obj_next_add+=(len+1);
			}
			if(type==2) // line
			{
				
			}
			if(type==3) // bitmap
			{
					memcpy(&obj_data[obj_next_add], incoming_data, len);
		     	obj_next_add+=len;
			}
			if(type==4) // bitmap
			{
				
			}
			if(type==5) // calendar
			{
				
			}
	 return obj_last_add;
	}
	else
	{
		return -1;
	}

}

void  obj_save()
{
	obj_data_wr_length=obj_next_add;
	save_objects_to_flash(retention_objects, 20);
	obj_data[0]=obj_visible;
  obj_data[1] = (uint8_t)(obj_data_wr_length >> 8);   // Lấy byte cao (0x03)
  obj_data[2] = (uint8_t)(obj_data_wr_length & 0xFF);  // Lấy byte thấp (0x8D)
	save_objects_map_to_flash(obj_data,obj_data_wr_length);
}
void handle_ble_layout_data(uint8_t* incoming_data, uint16_t len) {
	
	if(ble_rx_offset==0)
	{
		memcpy(&ble_rx_buffer[ble_rx_offset], incoming_data, len);
    ble_rx_offset += len;
		if(ble_rx_buffer[2]==1) //txt
		targetlength=ble_rx_buffer[11]+12;
		else if(ble_rx_buffer[2]==2) //line
				targetlength=11;
			else if(ble_rx_buffer[2]==3) // bimap
					targetlength=(ble_rx_buffer[11] | (ble_rx_buffer[12] << 8))+13;
				else if(ble_rx_buffer[2]==4) // battery
					targetlength=7;
				 else if(ble_rx_buffer[2]==5) // calendar
					targetlength=8;
				  else if(ble_rx_buffer[2]==6) // box
					 targetlength=12;
				
				
		if(ble_rx_offset>=targetlength)
		{
			ble_rx_offset=0;
		}
	}
	else
	{
		memcpy(&ble_rx_buffer[ble_rx_offset], incoming_data, len);
    ble_rx_offset += len;
		if(ble_rx_offset>=targetlength)
		{
			ble_rx_offset=0;
			
		}
	}
		
	if(ble_rx_offset==0)
	{
		uint8_t id = ble_rx_buffer[1];
		obj_visible=id+1;
		obj_data[0]=obj_visible;
		uint8_t type = ble_rx_buffer[2];
    int16_t x = ble_rx_buffer[3] | (ble_rx_buffer[4] << 8);
    int16_t y = ble_rx_buffer[5] | (ble_rx_buffer[6] << 8);

    if (id >=20) { ble_rx_offset = 0; return; }	
		if(id==0) obj_next_add=3;
		if(type==1) //text
		{
			retention_objects[id].type = type;
      retention_objects[id].x = x;
      retention_objects[id].y = y;
      retention_objects[id].font_type = ble_rx_buffer[7];
			retention_objects[id].line_dir = ble_rx_buffer[8]; // txt = @ or not @, line =direction , calender= language
			retention_objects[id].bmp_w = ble_rx_buffer[9];    
			retention_objects[id].bmp_h = ble_rx_buffer[10];  
			uint8_t text_len = ble_rx_buffer[11];
			retention_objects[id].add=obj_data_arrangement(1,&ble_rx_buffer[12],text_len);			
		}
		if(type==2)// line
		{
		  retention_objects[id].type = type;
      retention_objects[id].x = x;
      retention_objects[id].y = y;
      retention_objects[id].line_dir = ble_rx_buffer[7];
      retention_objects[id].line_len = ble_rx_buffer[8] | (ble_rx_buffer[9] << 8);
      retention_objects[id].thickness = ble_rx_buffer[10];			
		}
		if(type==3)// bitmap
		{
			
			retention_objects[id].type = type;
      retention_objects[id].x = x;
      retention_objects[id].y = y;
      retention_objects[id].bmp_w = ble_rx_buffer[7] | (ble_rx_buffer[8] << 8);
      retention_objects[id].bmp_h = ble_rx_buffer[9] | (ble_rx_buffer[10] << 8);
			retention_objects[id].add=obj_data_arrangement(3,&ble_rx_buffer[13],(ble_rx_buffer[11] | (ble_rx_buffer[12] << 8)));			
		//	memcpy(&obj_data, &ble_rx_buffer[13],(ble_rx_buffer[11] | (ble_rx_buffer[12] << 8)));
		}
		if(type==4)// bitmap
		{
			
			retention_objects[id].type = type;
      retention_objects[id].x = x;
      retention_objects[id].y = y;
		}
		if(type==5)// Calendar
		{
			
			retention_objects[id].type = type;
      retention_objects[id].x = x;
      retention_objects[id].y = y;	
			retention_objects[id].line_dir=ble_rx_buffer[7]; // muon line dir de luu ngon ngu
		}
		if(type==6)// Calendar
		{			
			retention_objects[id].type = type;
      retention_objects[id].x = x;
      retention_objects[id].y = y;	
			retention_objects[id].bmp_w = ble_rx_buffer[7] | (ble_rx_buffer[8] << 8);
      retention_objects[id].bmp_h = ble_rx_buffer[9] | (ble_rx_buffer[10] << 8);
			retention_objects[id].thickness=ble_rx_buffer[11];
		}
	}         
}

 /* FUNCTION DEFINITIONS
 ****************************************************************************************
 */

int adc1_update(void)
{
    adc_offset_calibrate(ADC_INPUT_MODE_SINGLE_ENDED);
    adcval = adc_get_vbat_sample(false);
    int volt = (adcval * 225) >> 7;

   /* struct custs1_val_set_req *req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_SET_REQ, prf_get_task_from_id(TASK_ID_CUSTS1), TASK_APP, custs1_val_set_req, DEF_SVC1_ADC_VAL_1_CHAR_LEN);
    req->conidx = app_env->conidx;
    req->handle = SVC1_IDX_ADC_VAL_1_VAL;
    req->length = DEF_SVC1_ADC_VAL_1_CHAR_LEN;
    req->value[0] = volt & 0xff;
    req->value[1] = volt >> 8;
    KE_MSG_SEND(req);
*/
    return volt;
}

/****************************************************************************************/

static const uint16_t lunar_year_info[32] =
    {
        0x07954,
        0x06aa0,
        0x0ad50,
        0x05b52,
        0x04b60,
        0x0a6e6,
        0x0a4e0,
        0x0d260,
        0x0ea65,
        0x0d530, // 2020-2029
        0x05aa0,
        0x076a3,
        0x096d0,
        0x04afb,
        0x04ad0,
        0x0a4d0,
        0x0d0b6,
        0x0d250,
        0x0d520,
        0x0dd45, // 2030-2039
        0x0b5a0,
        0x056d0,
        0x055b2,
        0x049b0,
        0x0a577,
        0x0a4b0,
        0x0aa50,
        0x0b255,
        0x06d20,
        0x0ada0, // 2040-2049
        0x04b63,
        0x09370, // 2050-2051
};
static const uint32_t lunar_year_info2 = 0x48010000;

static int jieqi_info[] =
    {
        0,
        1272283,
        2547462,
        3828568,
        5117483,
        6416376,
        7726093,
        9047327,
        10379235,
        11721860,
        13072410,
        14428379,
        15787551,
        17145937,
        18501082,
        19850188,
        21190911,
        22520708,
        23839844,
        25146961,
        26443845,
        27730671,
        29010666,
        30284551,
};

#define xiaohan_2020 451804

int year = 2025, month = 0, date = 0, wday = 3;
int l_year = 4, l_month = 11, l_date = 1;
int hour = 0, minute = 0, second = 0;

static void solar_to_lunar(int y, int m, int d, int *ly, int *lm, int *ld)
{
    static const int base_y = 2020, base_m = 0, base_d = 24;
    int days = 0;
    int i;
    for (i = base_y; i < y; ++i)
    {
        days += ((i % 4 == 0 && i % 100 != 0) || i % 400 == 0) ? 366 : 365;
    }
    for (i = 0; i < m; ++i)
    {
        static const int d2m[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        days += d2m[i];
        if (i == 1 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0))
            days += 1;
    }
    days += d;
    for (i = base_y; i < 2020; ++i)
    {
        days -= ((i % 4 == 0 && i % 100 != 0) || i % 400 == 0) ? 366 : 365;
    }
    for (i = 0; i < base_m; ++i)
    {
        static const int d2m[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        days -= d2m[i];
    }
    days -= base_d;

    int ly_ = 0, lm_ = 0, ld_ = 0;
    int yinfo, mdays;
    ly_ = 0; 
    lm_ = 0;
    ld_ = 0;
    while (days > 0)
    {
        yinfo = lunar_year_info[ly_];
        if (lunar_year_info2 & (1 << ly_))
            yinfo |= 0x10000;
        int leap = yinfo & 0x0f;
        int mon = 0;
        for (mon = 0; mon < 12; ++mon)
        {
            int lflag = 0;
            mdays = 29 + ((yinfo & (0x8000 >> mon)) ? 1 : 0);
            if (leap && mon == leap)
            {
                if (days < mdays)
                    break;
                days -= mdays;
                lflag = 0x80;
                mdays = 29 + ((yinfo & 0x10000) ? 1 : 0);
                if (days < mdays)
                {
                    lm_ = mon | lflag;
                    ld_ = days;
                    days = 0;
                    goto done;
                }
                days -= mdays;
                continue;
            }
            if (days < mdays)
            {
                lm_ = mon | lflag;
                ld_ = days;
                days = 0;
                goto done;
            }
            days -= mdays;
        }
        ly_++;
        lm_ = 0;
        ld_ = 0;
    }
done:
    if (ly)
        *ly = ly_;
    if (lm)
        *lm = lm_;
    if (ld)
        *ld = ld_;
}


int jieqi(int year, int month, int date)
{
    uint8_t d2m[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int is_leap = (year % 4) ? 0 : (year % 100) ? 1
                               : (year % 400)   ? 0
                                                : 1;
    d2m[1] += is_leap;

    int i, d = 0;
    for (i = 0; i < month; i++)
    {
        d += d2m[i];
    }
    d += date;

    int Y = year - 2020;
    int L = (Y) ? (Y - 1) / 4 + 1 : 0;
    int xiaohan_sec = xiaohan_2020 + 20950 * Y - L * 86400;

    for (i = 0; i < 24; i++)
    {
        int sec = xiaohan_sec + jieqi_info[i];
        int day = sec / 86400;
        if (day == d)
            return i;
    }

    return -1;
}

/****************************************************************************************/

static int get_month_day(int mon)
{
    uint8_t d2m[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int is_leap = (year % 4) ? 0 : (year % 100) ? 1
                               : (year % 400)   ? 0
                                                : 1;
    d2m[1] += is_leap;

    return d2m[month];
}

void date_inc(void)
{
    wday += 1;
    if (wday >= 7)
        wday = 0;

    date += 1;
    if (date == get_month_day(month))
    {
        date = 0;
        month += 1;
        if (month >= 12)
        {
            month = 0;
            year += 1;
        }
    }	
}
static int get_lunar_mdays(int mon, int *yinfo_out)
{
    // 获取闰月标志（最高位）
    int lflag = mon&0x80;
    // 获取实际月份（去除闰月标志）
    mon &= 0x7f;

	// 取得当年的信息
	int yinfo = lunar_year_info[l_year];
	if(lunar_year_info2&(1<<l_year))
		yinfo |= 0x10000;

	// 取得当月的天数
	int mdays = 29;
	if(lflag){
		if(yinfo&0x10000) mdays += 1;
	}else{
		if(yinfo&(0x8000>>mon))	mdays += 1;
	}

	if(yinfo_out)
		*yinfo_out = yinfo;
	return mdays;
}

void ldate_inc(void)
{
	int lflag = l_month&0x80;
	int mon = l_month&0x7f;
	int yinfo;

	int mdays = get_lunar_mdays(l_month, &yinfo);

	l_date += 1;
	if(l_date==mdays){
		l_date = 0;
		mon += 1;
		if(lflag==0 && mon==(yinfo&0x0f)){
			lflag = 0x80;
			mon -= 1;
		}else{
			lflag = 0;
		}
		if(mon==12){
			mon = 0;
			l_year += 1;
		}
		l_month = lflag|mon;
	}
}

static int epd_wait_state;
static timer_hnd epd_wait_hnd;


int clock_update(int inc)
{
    int retv = 0;

    second += inc;
    if (second < 60)
        return retv;
    second -= 60;

    minute += 1;
    retv = 1;
    if ((minute % 10) == 0)
		{ retv = 2;	firstime=1;}

    if (minute >= 60)
    {
        minute = 0;
        hour += 1;
        
        if ((hour % 2) == 0)
        {
            retv = 4;
        }
            
        if (hour >= 24)
        {
            hour = 0;
            date_inc();
						ldate_inc();
            retv = 4;
					 adc1_update();
        }
    }

    return retv;
}

void clock_set(uint8_t *buf)
{
    year = buf[1] + buf[2] * 256;
    month = buf[3];
    date = buf[4] - 1;
    hour = buf[5];
    minute = buf[6];
    second = buf[7];
    wday = buf[8];
		l_year = buf[9];
	  l_month= buf[10];
	  l_date = buf[11]-1;
	  solar_to_lunar(year, month, date, &l_year, &l_month, &l_date);
}

void clock_push(void)
{
    uint8_t buf[8];
    struct custs1_val_set_req *req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_SET_REQ, prf_get_task_from_id(TASK_ID_CUSTS1), TASK_APP, custs1_val_set_req, 8);

    req->conidx = app_env->conidx;
    req->handle = SVC1_IDX_LONG_VALUE_VAL;
    req->length = 8;
    req->value[0] = year & 0xff;
    req->value[1] = year >> 8;
    req->value[2] = month;
    req->value[3] = date + 1;
    req->value[4] = hour;
    req->value[5] = minute;
    req->value[6] = second;
    req->value[7] = 0;
    KE_MSG_SEND(req);
}

void clock_print(void)
{
    printk("\n%04d-%02d-%02d %02d:%02d:%02d  L: %d-%d\n", year, month + 1, date + 1, hour, minute, second, l_month + 1, l_date + 1);
}


static void ldate_str(char *buf)
{
    int ly, lm, ld;
    solar_to_lunar(year, month, date, &ly, &lm, &ld);

    int lm_show = lm & 0x7f;
    if (lm_show == 0)
        lm_show = 12;

    int hi = ld / 10;
    int lo = ld % 10;

    if (lo == 9)
    {
        if (hi == 1)
            hi = 3;
        else if (hi == 2)
            hi = 4;
    }

    if (hi == 0) {
        sprintf(buf, "Day %d", lo + 1);
    } else {
        sprintf(buf, "Day %d", ld + 1);
    }
}


static uint8_t batt_cal(uint16_t adc_sample)
{
    uint8_t batt_lvl;

    if (adc_sample > 1705)
        batt_lvl = 100;
    else if (adc_sample <= 1705 && adc_sample > 1584)
        batt_lvl = 28 + (uint8_t)(((((adc_sample - 1584) << 16) / (1705 - 1584)) * 72) >> 16);
    else if (adc_sample <= 1584 && adc_sample > 1360)
        batt_lvl = 4 + (uint8_t)(((((adc_sample - 1360) << 16) / (1584 - 1360)) * 24) >> 16);
    else if (adc_sample <= 1360 && adc_sample > 1136)
        batt_lvl = (uint8_t)(((((adc_sample - 1136) << 16) / (1360 - 1136)) * 4) >> 16);
    else
        batt_lvl = 0;

    return batt_lvl;
}

static void draw_batt(int x, int y)
{
    int p = batt_cal(adcval);
    p /= 10;
	
    draw_box(x,y-4,x+14,y+4,WHITE);// clear
	
    draw_rect(x, y - 4, x + 14, y + 4, BLACK);
    draw_box(x - 2, y - 1, x - 1, y + 1, BLACK);
    draw_box(x + 12 - p, y - 2, x + 12, y + 2, BLACK);
}

static void draw_bt(int x, int y)
{
    int i;
    draw_vline(x, y - 4, y + 4, BLACK);
    for (i = 0; i < 5; i++)
    {
        draw_pixel(x - 2 + i, y - 2 + i, BLACK);
        draw_pixel(x - 2 + i, y + 2 - i, BLACK);
    }
    draw_pixel(x + 1, y - 3, BLACK);
    draw_pixel(x + 1, y + 3, BLACK);
}

static void epd_wait_state_handler(void) 
{
    if (epd_busy())
    {
        epd_wait_hnd = app_easy_timer(100, epd_wait_state_handler);
    }
    else
    {
        epd_wait_hnd = EASY_TIMER_INVALID_TIMER;
        arch_set_sleep_mode(ARCH_EXT_SLEEP_ON);
    }
}

int get_first_wday_sun0(int year, int month)
{
    int q = 1; 
    int m = month;
    int y = year;

    if (m < 3)
    {
        m += 12;
        y--;
    }

    int k = y % 100;
    int j = y / 100;

    int h = (q + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    int wday_res = (h + 6) % 7;

    return wday_res; 
}

int get_month_days(int year, int month)
{
    static const int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0))
        return 29;
    return mdays[month - 1];
}

void draw_calendar(int year, int month, int day, int start_x, int start_y,uint8_t language)
{
    int calendar[6][7] = {0};                      
    int first_wday = get_first_wday_sun0(year, month); 
    int days = get_month_days(year, month);

    int d = 1, i, j;
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 7; j++)
        {
            if (i == 0 && j < first_wday)
                continue;
            if (d > days)
                break;
            calendar[i][j] = d++;
        }
    }

    int cell_w = 20;
    int cell_h = 13;
    select_font(0);

    if (1)
    {
        int total_w = 7 * cell_w;
        int header_h =12;
			
			  draw_box(start_x, start_y - header_h, start_x + total_w, start_y +78, WHITE);// clear
			
        draw_box(start_x, start_y - header_h, start_x + total_w, start_y + 2, BLACK);
        
        // CHUYỂN TIÊU ĐỀ THỨ SANG TIẾNG ANH THUẦN
        char *weeks_en[] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};
				char *weeks_cn[] = {"日", "一", "二", "三", "四", "五", "六"};
				char *weeks_vi[] = {"CN", "T2", "T3", "T4", "T5", "T6", "T7"};
        for (j = 0; j < 7; j++)
        {
            int text_offset = (cell_w - 12) / 2; 
            int text_y_offset =-13;
					if(language==1)
            draw_text(start_x + text_offset + j * cell_w, start_y + text_y_offset, weeks_cn[j], WHITE);
					else if (language==0)
						 draw_text(start_x + text_offset + j * cell_w, start_y + text_y_offset, weeks_en[j], WHITE);
					else 
						draw_text(start_x + text_offset + j * cell_w, start_y + text_y_offset, weeks_vi[j], WHITE);
        }
    }

    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 7; j++)
        {
            int date = calendar[i][j];
            if (date == 0)
                continue;
            int x = start_x + j * cell_w;
            int y = start_y + i * cell_h;
            char buf[4];
            sprintf(buf, "%02d", date);

            int num_offset_x = (cell_w - 11) / 2; 

            if (date == day)
            {
                int box_pad_x = 1; 
                int box_pad_y = 1;
                draw_box(x + box_pad_x, y + box_pad_y+5, x + cell_w - box_pad_x, y + cell_h + 4, BLACK); 
                draw_text(x + num_offset_x, y + 2, buf, WHITE);                                                                         
            }
            else
            {
                draw_text(x + num_offset_x, y + 2, buf, BLACK); 
            }
        }
    }
}


void clock_draw(int flags)
{
    char tbuf[64];

    if (ota_state)
    {
        return;
    }     				
		user_app_adv_start();
		epd_hw_open();
    epd_update_mode(flags & 3);
		if(firstime)
     memset(fb_bw, 0xff, scr_h * line_bytes);   
		
		if(obj_visible<=20&&obj_visible>0)	
    for (uint8_t i = 0; i <obj_visible; i++) 		
		{ 				
			MCU_Object_t* obj = &retention_objects[i];
			if ((obj->type == 0)||(obj->type == 0xff)) continue; 
			if (obj->type == 1)  //txt
			{
				if(obj->add!=-1)
				{			
				if(firstime==1&&obj->line_dir==0)
				{
									select_font(obj->font_type);     		
									obj->thickness=draw_text(obj->x, obj->y, (char *)&obj_data[obj->add], BLACK);  				    
				}
				else if(firstime==1&&obj->line_dir==1)
					{
									select_font(obj->font_type);      					
									obj->thickness=draw_text(obj->x, obj->y, (char *)&obj_data[obj->add], BLACK);  				  
					}
				else if(firstime==0&&obj->line_dir==1)
					{
									select_font(obj->font_type);      
								  draw_box(obj->x,obj->y +obj->bmp_w,obj->thickness,obj->y + obj->bmp_h,WHITE);						
									obj->thickness=draw_text(obj->x, obj->y, (char *)&obj_data[obj->add], BLACK);  				  
					}
			}
      }
			else if (firstime==1&&obj->type == 2) // line
			{
				draw_line(obj->x,obj->y, obj->line_dir,obj->line_len,obj->thickness, BLACK);
			}
			else if (firstime==1&&obj->type == 3)// bitmap
			{       
				if(obj->add!=-1)
				{
				drawbitmap(obj->x, obj->y,(u8 *)&obj_data[obj->add], obj->bmp_w, obj->bmp_h,BLACK);
				}
			}  
			else if (obj->type == 4)// battery
			{       
		   draw_batt(obj->x,obj->y);			
			}  
			else if (obj->type == 5)// calendar
			{     
			 static int last_date=0;
			 if(last_date!=date||firstime==1)
		   draw_calendar(year,month+1,date+1,obj->x,obj->y, obj->line_dir);			
			 last_date=date;
			} 
			else if (firstime==1&&obj->type == 6)// calendar
			{     
       draw_rect_shape(obj->x,obj->y,obj->bmp_w,obj->bmp_h,obj->thickness,BLACK);
			} 
   }
				
		 firstime=0;
	 
     epd_init();
     epd_screen_update();
     epd_update();
     arch_set_sleep_mode(ARCH_SLEEP_OFF);
     epd_wait_hnd = app_easy_timer(200, epd_wait_state_handler);
}

/****************************************************************************************/

void user_svc1_ctrl_wr_ind_handler(ke_msg_id_t const msgid, struct custs1_val_write_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    printk("Control Point: %02x\n", param->value[0]);
}

void user_svc1_long_val_wr_ind_handler(ke_msg_id_t const msgid, struct custs1_val_write_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	
     if (ble_rx_offset > 0) {
        handle_ble_layout_data((uint8_t *)param->value, param->length);
        return; 
    }
    uint8_t cmd = param->value[0];
    if (cmd >= 0xa0) {
        ota_handle((u8 *)param->value);
    } 
    else {
        switch (cmd) {
            case 0x91:
						  	firstime=1;
                clock_set((uint8_t *)param->value);
                clock_draw(DRAW_BT | UPDATE_FAST);				    
                break;

            case 0x92:
							  firstime=1;
                moshi = (param->value[1] << 8) | param->value[2];
                clock_draw(DRAW_BT | UPDATE_FAST);
                break;
						
            case 0x95:
                // Chunk đầu tiên của gói 0x95 đi vào đây
						    ble_rx_offset = 0;targetlength=0;
                handle_ble_layout_data((uint8_t *)param->value, param->length);
						    firstime=1;
                break;
						case 0x98:
						    firstime=1;
						    adc1_update();
					  	solar_to_lunar(year, month, date, &l_year, &l_month, &l_date);
						     clock_draw(DRAW_BT | UPDATE_FAST);
                break;
						case 0x9a:
							  firstime=1;
						    SetWord16(SYS_CTRL_REG, (GetWord16(SYS_CTRL_REG) & ~REMAP_ADR0) | SW_RESET );
                break;
						case 0x99:
							 firstime=1;
						    obj_save();
                break;
            default:
                // Các mã lệnh không hợp lệ khác hoặc không xử lý
                break;
        }
    }
}

void user_svc1_long_val_att_info_req_handler(ke_msg_id_t const msgid, struct custs1_att_info_req const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct custs1_att_info_rsp *rsp = KE_MSG_ALLOC(CUSTS1_ATT_INFO_RSP, src_id, dest_id, custs1_att_info_rsp);
    rsp->conidx = app_env[param->conidx].conidx;
    rsp->att_idx = param->att_idx;
    rsp->length = 0;
    rsp->status = ATT_ERR_NO_ERROR;

    KE_MSG_SEND(rsp);
}

void user_svc1_rest_att_info_req_handler(ke_msg_id_t const msgid,
                                         struct custs1_att_info_req const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    struct custs1_att_info_rsp *rsp = KE_MSG_ALLOC(CUSTS1_ATT_INFO_RSP,
                                                   src_id,
                                                   dest_id,
                                                   custs1_att_info_rsp);
    rsp->conidx = app_env[param->conidx].conidx;
    rsp->att_idx = param->att_idx;
    rsp->length = 0;
    rsp->status = ATT_ERR_WRITE_NOT_PERMITTED;

    KE_MSG_SEND(rsp);
}