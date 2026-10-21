

#include "epd.h"

/******************************************************************************/

// FB参数
int fb_w;
int fb_h;

#define FB_SIZE 4736
u8 fb_bw[FB_SIZE];
//u8 fb_rr[FB_SIZE];
static char *wday_str[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
static char *wday_en_str[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char *wday_vi_str[] = {"Chủ Nhật", "Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7"};
/******************************************************************************/




#define FLASH_FONT_BASE_ADDR   0x20000


void draw_pixel(int x, int y, int color)
{
    int nx, ny;
    int rmode = scr_mode & 0x03;

    if (rmode == 0)
    {
        nx = x;
        ny = y;
    }
    else if (rmode == 1)
    {
        nx = scr_w - 1 - y;
        ny = x;
    }
    else if (rmode == 2)
    {
        nx = scr_w - 1 - x;
        ny = scr_h - 1 - y;
    }
    else if (rmode == 3)
    {
        nx = y;
        ny = scr_h - 1 - x;
    }
    if (scr_mode & MIRROR_H)
    {
        nx += scr_padding;
    }

    int byte_pos = ny * line_bytes + (nx >> 3);
    int bit_mask = 0x80 >> (nx & 7);

    if (color != WHITE)
    {
        fb_bw[byte_pos] &= ~bit_mask;
    }
    if (color == WHITE)
    {
        fb_bw[byte_pos] |= bit_mask;
    }
//    if (scr_mode & EPD_BWR && color == RED)
//    {
//        fb_rr[byte_pos] |= bit_mask;
//    }
}





void drawbitmap(int x, int y,uint8_t *bitmap, int w, int h, int color) {
    int byteWidth = (w + 7) / 8; // Số byte trên một hàng ngang

    for (int j = 0; j < h; j++) {
        // Lấy con trỏ đến đầu hàng hiện tại trong mảng bitmap
        const uint8_t *row_ptr = &bitmap[j * byteWidth];
        
        for (int i = 0; i < w; i++) {
            // Xác định byte hiện tại dựa trên pixel index i
            // Sử dụng dịch bit (i >> 3) thay cho chia 8 (i / 8)
            uint8_t current_byte = row_ptr[i >> 3];
            
            // Tạo mask kiểm tra bit dịch từ MSB (0x80) dịch dần sang phải
            // Sử dụng phép AND bit (i & 7) thay cho lấy dư 8 (i % 8)
            uint8_t mask = 0x80 >> (i & 7);
            
            if (current_byte & mask) {
                draw_pixel(x + i, y + j, color);
            }
        }
    }
}

void draw_hline(int y, int x1, int x2, int color)
{
    int x;
    if (x1 > x2)
    {
        x = x1;
        x1 = x2;
        x2 = x;
    }

    for (x = x1; x <= x2; x += 1)
    {
        draw_pixel(x, y, color);
    }
}

void draw_vline(int x, int y1, int y2, int color)
{
    int y;
    if (y1 > y2)
    {
        x = y1;
        y1 = y2;
        y2 = x;
    }

    for (y = y1; y <= y2; y += 1)
    {
        draw_pixel(x, y, color);
    }
}

void draw_line(int x, int y, int dir,int length,int thickness, int color)
{
	if(dir)
	{
		for(int i=0;i<thickness;i++)
		{	
			draw_vline(x+i,y,y+length,color);
		}
	
	}
	else
	{
		for(int i=0;i<thickness;i++)
		{	
			draw_hline(y+i,x,x+length,color);
		}
	}
}
void draw_rect(int x1, int y1, int x2, int y2, int color)
{
    draw_hline(y1, x1, x2, color);
    draw_hline(y2, x1, x2, color);
    draw_vline(x1, y1, y2, color);
    draw_vline(x2, y1, y2, color);
}

void draw_rect_shape(int x1, int y1, int width, int height,int thickness,int color)
{
	for(int i=0;i<thickness;i++)
	{
		draw_hline(y1+i, x1, x1+width, color);
    draw_hline(y1+height-i, x1, x1+width, color);
    draw_vline(x1+i, y1, y1+height, color);
    draw_vline(x1+width-i, y1, y1+height, color);
	}  
}


void draw_box(int x1, int y1, int x2, int y2, int color)
{
    int y;

    for (y = y1; y <= y2; y++)
    {
        draw_hline(y, x1, x2, color);
    }
}

/******************************************************************************/

static const uint32_t font_flash_offsets[] = {
    0x00002750, // ID 0: T_font_1
    0x00000000, // ID 1: N_font_1
    0x00000730, // ID 2: N_font_2 (Ví dụ giá trị offset từ tool Python)
    0x000009f5, // ID 3: N_font_3
    0x00000e1f, // ID 4: N_font_4
    0x00001376, // ID 5: N_font_5
    0x00001b87, // ID 6: N_font_6
    0x00004482, // ID 7: T_font_2
		0x000063ca, // ID 8: T_font_3
		0x0000799e,  // ID 9: T_font_4
		0x00009bd3,  // ID 10: T_font_5
	  0x0000d0aa,  // ID 11: T_font_6
	0x00012a8a,  // ID 11: T_font_7
	0x000132d5,  // ID 11: T_font_8
	0x000139bf,  // ID 11: T_font_9
	0x00003a73  // ID 11: T_font_10
};

static int current_cached_font_id = -1;

#define NUM_FONTS 16

typedef struct {
    u16 total_chars;      
    u32 flash_bitmap_addr; 
    u8 *index_table;       
} font_header_t;

static font_header_t font_headers[NUM_FONTS];

#define TOTAL_INDEX_RAM_SIZE (10 * 1024) 
static u8 global_index_buffer[TOTAL_INDEX_RAM_SIZE];

static int current_selected_font_id = -1;

void font_system_init(void)
{
    u32 current_ram_offset = 0;
    
    fspi_init();
    
    for (int id = 0; id < NUM_FONTS; id++)
    {
        u32 font_flash_base = FLASH_FONT_BASE_ADDR + font_flash_offsets[id];
        u16 total = 0;
        sf_read(font_flash_base, 2, (u8 *)&total);        
        font_headers[id].total_chars = total;
        u32 index_size = total * 4;        
			printk("load id font %d : total byte :%d\n",id,index_size );
        if (current_ram_offset + index_size > TOTAL_INDEX_RAM_SIZE) {
					printk("over size %d font init!\n",current_ram_offset + index_size);
            continue; 					
        }
				font_headers[id].index_table = &global_index_buffer[current_ram_offset];
        sf_read(font_flash_base + 2, index_size, font_headers[id].index_table);
        font_headers[id].flash_bitmap_addr = font_flash_base + 2 + index_size;
        current_ram_offset += index_size;
    }    
		// printk("Save %d font init!\n", current_ram_offset);
    fspi_exit();
    current_selected_font_id = -1;
}


int select_font(int id)
{
    if (id < 0 || id >= NUM_FONTS) return -1;
    
    current_selected_font_id = id; 
    return 0;
}
static int find_font_offset_in_ram(int font_id, int ucs)
{
    if (font_id < 0 || font_id >= NUM_FONTS) return -1;
    
    font_header_t *font = &font_headers[font_id];
    int low = 0;
    int high = font->total_chars - 1;
    u8 *index = font->index_table;

    while (low <= high)
    {
        int mid = low + (high - low) / 2;
        u16 mid_ucs = *(u16 *)(index + mid * 4 + 0); 

        if (mid_ucs == ucs)
        {
            return *(u16 *)(index + mid * 4 + 2); 
        }
        else if (mid_ucs < ucs)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    if (ucs != '?') {
        return find_font_offset_in_ram(font_id, '?');
    }

    return -1;
}

static int utf8_to_ucs(char **ustr)
{
    u8 *str = (u8 *)*ustr;
    int ucs = 0;

    if (*str == 0)
    {
        return 0;
    }
    else if (*str < 0x80)
    {
        *ustr = (char *)str + 1;
        return *str;
    }
    else if (*str < 0xe0)
    {
        ucs = ((str[0] & 0x1f) << 6) | (str[1] & 0x3f);
        *ustr = (char *)str + 2;
        return ucs;
    }
    else
    {
        ucs = ((str[0] & 0x0f) << 12) | ((str[1] & 0x3f) << 6) | (str[2] & 0x3f);
        *ustr = (char *)str + 3;
        return ucs;
    }
}
int fb_draw_font(int x, int y, int ucs, int color)
{
    if (current_selected_font_id == -1) return 0;
    int offset = find_font_offset_in_ram(current_selected_font_id, ucs);
    if (offset == -1) return 0; 

    font_header_t *font = &font_headers[current_selected_font_id];
    u32 char_flash_addr = font->flash_bitmap_addr + (offset - (2 + font->total_chars * 4));

    u8 attr[5];
    fspi_init();
    sf_read(char_flash_addr, 5, attr);

    int ft_adv = attr[0];
    int ft_bw  = attr[1];
    int ft_bh  = attr[2];
    int ft_bx  = (signed char)attr[3];
    int ft_by  = (signed char)attr[4];
    int ft_lsize = (ft_bw + 7) / 8;
    
    u32 bitmap_size = ft_bh * ft_lsize;
    static u8 local_bitmap_buf[320]; 
    if (bitmap_size > sizeof(local_bitmap_buf)) {
        fspi_exit();
        return 0; 
    }
    
    sf_read(char_flash_addr + 5, bitmap_size, local_bitmap_buf);
    fspi_exit();

    x += ft_bx;
    y += ft_by;

    int r, c;
    u8 *p_bmp = local_bitmap_buf;
    for (r = 0; r < ft_bh; r++)
    {
        for (c = 0; c < ft_bw; c++)
        {
            if (p_bmp[c >> 3] & (0x80 >> (c & 7)))
            {
                draw_pixel(x + c, y, color);
            }
        }
        p_bmp += ft_lsize;
        y += 1;
    }

    return ft_adv;
}

int draw_text(int x, int y, char *str, int color)
{
    int ch;
    while (1)
    {
        ch = utf8_to_ucs(&str);
        if (ch == 0)
            break;
				if (ch == '@')
        {
            char next_ch = *str; 
            
            if (next_ch == 'H' || next_ch == 'M')
            {
                int val = (next_ch == 'H') ? hour : minute;               
                char digit_tens = '0' + (val / 10);
                char digit_ones = '0' + (val % 10);               
                x += fb_draw_font(x, y, (int)digit_tens, color);
                x += fb_draw_font(x, y, (int)digit_ones, color);               
                str++; 
                continue; 
            }
						if (next_ch == 't' || next_ch == 'n')
            {
                int val = (next_ch == 't') ? (l_month+1) : (l_date+1);               
                char digit_tens = '0' + (val / 10);
                char digit_ones = '0' + (val % 10);               
                x += fb_draw_font(x, y, (int)digit_tens, color);
                x += fb_draw_font(x, y, (int)digit_ones, color);               
                str++; 
                continue; 
            }
						if (next_ch == 'b')
            {
							  int volt = (adcval * 225) >> 7;             
                char digit_tens = '0' + (volt /1000);
                char digit_ones = '0' + ((volt / 100)%10);               
                x += fb_draw_font(x, y, (int)digit_tens, color);
							  x += fb_draw_font(x, y,'.', color);
                x += fb_draw_font(x, y, (int)digit_ones, color);  
								x += fb_draw_font(x, y,'v', color);							
                str++; 
                continue; 
            }
						if(next_ch=='r'||next_ch=='R'||next_ch=='x')  // them chuc nang
							{
								if(next_ch=='r')
								{
									char *ptr = wday_str[wday];
									for(int count=0;count<3; count++)
									{
									x += fb_draw_font(x, y, utf8_to_ucs(&ptr),color);															
									}
								}
							
								else if (next_ch=='R')
								{									
									char *ptr = wday_en_str[wday];
									for(int count=0;count<3; count++)
									{
									x += fb_draw_font(x, y, utf8_to_ucs(&ptr),color);															
									}
								}
								else if (next_ch=='x')
								{									
									char *ptr = wday_vi_str[wday];
									if(wday==0)
									{
										for(int count=0;count<8; count++)
										{
										x += fb_draw_font(x, y, utf8_to_ucs(&ptr),color);															
										}
									}
									else
									{
										for(int count=0;count<5; count++)
										{
										x += fb_draw_font(x, y, utf8_to_ucs(&ptr),color);															
										}
									}
									
								}
								  str++; 
								  continue; 								
							}
				
						if (next_ch == 'y' || next_ch == 'd'|| next_ch == 'm')
            {
							if(next_ch=='y')
							{
							//	x += fb_draw_font(x, y, '0' +(int)year/1000, color);
              //  x += fb_draw_font(x, y, '0' +(int)(year/100)%10, color);    
								x += fb_draw_font(x, y, '0' +(int)(year/10)%10, color);
                x += fb_draw_font(x, y, '0' +(int)year%10, color); 
								
							}
							else if(next_ch=='m')
							{ 
								x += fb_draw_font(x, y, '0' +(int)((month+1)/10), color);
                x += fb_draw_font(x, y, '0' +(int)(month+1)%10, color); 
							}
							else if(next_ch=='d')
							{
								x += fb_draw_font(x, y, '0' +(int)((date+1)/10), color);
                x += fb_draw_font(x, y, '0' +(int)(date+1)%10, color); 
							}  
							else
							{
								x += fb_draw_font(x, y, ch, color);
								continue;
							}					
						str++; 
            continue; 						                     
            }
        }
        x += fb_draw_font(x, y, ch, color);
    }
		return x;
}

#if 0
char *wday_str[] = {
	"一",
	"二",
	"三",
	"四",
	"五",
	"六",
	"日",
};


static int wday = 0;
void fb_test(void)
{
	memset(fb_bw, 0xff, scr_h*line_bytes);
	if(scr_mode&EPD_BWR){
		memset(fb_rr, 0x00, scr_h*line_bytes);
	}

	draw_rect(0, 0, fb_w-1, fb_h-1, BLACK);
	draw_rect(1, 1, fb_w-2, fb_h-2, BLACK);

#if 0
	for(int i=0; i<3; i++){
		int x = 8+i*38;
		int y = 30;
		draw_rect(x, y, x+29, y+29, BLACK);
		//draw_rect(x+1, y+1, x+29-1, y+29-1, BLACK);
		draw_box(x+2, y+2, x+29-2, y+29-2, i);
	}
#endif

	select_font(0);

	char tbuf[64];
	sprintk(tbuf, "%4d年%2d月%2d日 星期%s", 2025, 4, 29, wday_str[wday]);
	draw_text(15, 85, tbuf, BLACK);
	
	wday += 1;
	if(wday==7)
		wday = 0;

	select_font(1);
	sprintk(tbuf, "%02d:%02d", 2+wday, 30+wday);
	draw_text(12, 20, tbuf, BLACK);

	epd_screen_update();
}
#endif

/******************************************************************************/
