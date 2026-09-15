#include "gnssCtrlTasks.h"
/*
*
*
*
*
*/
gnss_pwr_state gnss_pwr_stt=
{
    .on=false
};
void gnss_pwr(bool state)
{
    gpio_pin_set(gpio0, GNSS_PWR_PIN, state);
    gnss_pwr_stt.on=!state;
    if(state) uart_rx_disable(uart0);
}
/*******************************************************************************************************/
/*                                     GNSS FSM IMPLEMENTATION START                                   */
/*******************************************************************************************************/
gnss_state_t gnss_fsm[3]=
{
    /* state 0: PWROFF */
    {
		.ns={GNSS_PWROFF, GNSS_ACQUI, GNSS_TRACKING}
	},
	/* state 1: ACQUI */
	{
		.ns={GNSS_PWROFF, GNSS_ACQUI, GNSS_TRACKING}
	},
	/* state 2: TRACKING */
	{
		.ns={GNSS_PWROFF, GNSS_ACQUI, GNSS_TRACKING}
	},
};
gnss_state gnss_cs=GNSS_PWROFF;
uint8_t* gnss_stt2str[3]={"POWER OFF", "ACQUISITION", "TRACKING"};
void gnss_finite_sm(gnss_state_input input)
{
	gnss_cs=gnss_fsm[gnss_cs].ns[input];
    printk("\n----> GNSS: %s", gnss_stt2str[gnss_cs]);
    switch(gnss_cs)
    {
        case GNSS_PWROFF:
        {
			if(gnss_pwr_stt.on) 
            {
                printk("\n    - gnss power state now: %d!(1:ON, 0:OFF)", gnss_pwr_stt.on);
                gnss_pwr(OFF);
                uart0_suspend();
            }
            printk("\n    - gnss power state now: %d!(1:ON, 0:OFF)", gnss_pwr_stt.on);
            printk("\n    - gnss_pwroff entered!");
            break;
        }
        case GNSS_ACQUI:
        {
            if(!gnss_pwr_stt.on)
            {
                printk("\n    - gnss power state now: %d!(1:ON, 0:OFF)", gnss_pwr_stt.on);
                uart0_resume();
			    gnss_pwr(ON);
                k_sem_give(&gnss_rx_start_sem);
            }
            printk("\n    - gnss power state now: %d!(1:ON, 0:OFF)", gnss_pwr_stt.on);
            printk("\n    - gnss_acqui entered!");
            break;
        }
        case GNSS_TRACKING:
        {
            printk("\n    - gnss_tracking entered!");
            break;
        }	
        default: break;
    }
}
/*******************************************************************************************************/
/*                                     GNSS FSM IMPLEMENTATION END                                     */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                    GNSS READ & PROC DATA START                                      */
/*******************************************************************************************************/
void uart0_cb_handler(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) 
	{
		case UART_TX_DONE:
		{
			// do something
			break;
		}	
		case UART_TX_ABORTED:
        {
			// do something
			break;	
        }		
		case UART_RX_STOPPED:
        {
			// do something
			break;	
        }
        case UART_RX_RDY:
        {
            // do something
			break;
        }
        case UART_RX_BUF_REQUEST:
        {
            // do something
            break;
        }
        case UART_RX_BUF_RELEASED:
        {
            // do something
            break;	
        }
        case UART_RX_DISABLED:
		{
            if(gnss_pwr_stt.on) k_sem_give(&gnss_rx_done_sem);
			break;
		}
        default:
			break;
	}
}

char nmea_buf[NMEA_BUF_SIZE];
double lat_center=21.006293642533574;
double lon_center=105.84310431530237;
// char nmea_test_buf[]=",2055.2891,N,10551.3357,E"; // sim65m-w
char nmea_test_buf[]=",2055.289168,N,10551.335768,E"; // lc76g-pb

double location_decimal_convert(const char* raw)
{
    double v = atof(raw);
    int deg = (int)(v / 100);
    double min = v - deg * 100;
    return deg + (min / 60.0);
}
int is_valid_location(const char* f)
{
    return isdigit((unsigned char)f[0]);    // kiểm tra field phải bắt đầu bằng chữ số => tránh trường hợp bị mất số
}
int nmea_process(const char* nmea, double* lat, double* lon)
{
    const char* p = nmea;
    char field[32];
    int f_len = 0;

    char last_field[32];
    int last_len = 0;

    int lat_found = 0;
    int lon_found = 0;
    char lat_dir = 0;
    char lon_dir = 0;

    while (*p)
    {
        // printk("gnss process nmea");
        char c = *p;

        if (c == ',' || c == '\n')
        {
            field[f_len] = '\0';
            strcpy(last_field, field);
            last_len = f_len;
            f_len = 0;
        }
        else
        {
            if (f_len < (int)sizeof(field) - 1)
                field[f_len++] = c;
        }

        // kiểm tra ký tự hướng
        if ((*p == 'N' || *p == 'S') && !lat_found)
        {
            //  yêu cầu:
            //  1. last_field phải bắt đầu bằng số -> tránh mất số (ex: "07.12" vẫn ok, "" thì ko)
            //  2. field phải nằm giữa 2 dấu phẩy -> ta đã đảm bảo khi đọc last_field
            //  3. last_field phải dài tối thiểu 4 ký tự (vd: ddmm)
            if (last_len == 11 && is_valid_location(last_field))
            {
                *lat = location_decimal_convert(last_field);
                lat_dir = *p;
                lat_found = 1;
            }
        }
        else if ((*p == 'E' || *p == 'W') && !lon_found)
        {
            if (last_len == 12 && is_valid_location(last_field))
            {
                *lon = location_decimal_convert(last_field);
                lon_dir = *p;
                lon_found = 1;
            }
        }

        p++;
    }

    // nếu không tìm thấy đủ lat + lon thì bỏ qua
    if (!lat_found || !lon_found)
        return 0;

    if (lat_dir == 'S') *lat = -(*lat);
    if (lon_dir == 'W') *lon = -(*lon);

    return 1;
}
void location_string_convert(double num, char* buffer) // Hàm chuyển double thành chuỗi với đúng 9 chữ số sau dấu phẩy
{
    int i = 0;
    char temp[32];
    int pos = 0;

    // Xử lý số âm
    if (num < 0) {
        buffer[i++] = '-';
        num = -num;
    }

    // Tách phần nguyên
    long long int_part = (long long)num;
    double dec_part = num - (double)int_part;

    // === Chuyển phần nguyên ===
    if (int_part == 0) {
        buffer[i++] = '0';
    } else {
        while (int_part > 0) {
            temp[pos++] = '0' + (int_part % 10);
            int_part /= 10;
        }
        for (int j = pos - 1; j >= 0; j--) {
            buffer[i++] = temp[j];
        }
        pos = 0;
    }

    // Dấu chấm
    buffer[i++] = '.';

    // === 9 chữ số thập phân ===
    for (int digit = 0; digit < 9; digit++) {
        dec_part *= 10;
        int digit = (int)dec_part;
        buffer[i++] = '0' + digit;
        dec_part -= digit;
    }
    buffer[i] = '\0';
}
void location_strings_merge(double so1, double so2, char* result_str) // ghép 2 double thành 1 chuỗi, có dấu cách ở giữa
{
    char str1[32], str2[32];
    int idx = 0;

    // Chuyển 2 số thành chuỗi
    location_string_convert(so1, str1);
    location_string_convert(so2, str2);

    // Ghép: so1 + " " + so2
    for (int i = 0; str1[i] != '\0'; i++) {
        result_str[idx++] = str1[i];
    }
    result_str[idx++] = ' ';  // Dấu cách ở giữa

    for (int i = 0; str2[i] != '\0'; i++) {
        result_str[idx++] = str2[i];
    }
    result_str[idx++] = '\n';
    result_str[idx] = '\0';  // Kết thúc chuỗi
}
double haversine_distance(double lat1, double lon1, double lat2, double lon2) // Hàm tính khoảng cách giữa 2 tọa độ GPS (lat, lon tính bằng độ)
{
    double lat1_rad = lat1 * 3.141592654 / 180.0;
    double lat2_rad = lat2 * 3.141592654 / 180.0;
    double delta_lat = (lat2 - lat1) * 3.141592654 / 180.0;
    double delta_lon = (lon2 - lon1) * 3.141592654 / 180.0;

    double a = sin(delta_lat / 2) * sin(delta_lat / 2) +
               cos(lat1_rad) * cos(lat2_rad) *
               sin(delta_lon / 2) * sin(delta_lon / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    double distance = 6371000 * c; // đơn vị: mét
    return distance;
}

K_SEM_DEFINE(gnss_rx_start_sem,0,1);
K_SEM_DEFINE(gnss_rx_done_sem,0,1);
K_SEM_DEFINE(gnss_rx_stop_sem,0,1);
K_EVENT_DEFINE(modules_rx_stopped_evt);
location_data_t location_data_pingpong[2][LOCATION_DATA_PINGPONG_SIZE];
location_data_t* location_data_pingpong_wr_ptr=location_data_pingpong[0];
uint32_t location_data_pingpong_wr_idx=0;
void gnss_ctrl_thread()
{
    double lat, lon; lat=0; lon=0;
    double distance;
    evt_q_t evt_q_put;
    double lat_center_tmp=lat_center;
    double lon_center_tmp=lon_center;
    double distance_tmp;
    while(1)
    {
        k_sem_take(&gnss_rx_start_sem, K_FOREVER);
        uart_rx_enable(uart0, nmea_buf, sizeof(nmea_buf), SYS_FOREVER_US);
        k_sem_take(&gnss_rx_done_sem, K_FOREVER);
        printk("\nGNSS_REV_DATA_THREAD: received, ready process nmea buffer!");
        if(nmea_process(nmea_buf, &lat, &lon))
        {
            printk("\nGNSS_REV_DATA_THREAD: gps fixed!");
            distance=haversine_distance(lat, lon, lat_center, lon_center);
            if(distance>GEOFENCING_R)
            {
                printk("\nGNSS_REV_DATA_THREAD: out geofence zone (distance=%d)!", (uint32_t)(round(distance)));
                evt_q_put.evt_src=GEOF_OUT_EVT;
                k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
                printk("\nGNSS_REV_DATA_THREAD: lat_int=%d, lon_int=%d!", (int32_t)(round(lat*1000000.0)), (int32_t)(round(lon*1000000.0)));

                distance_tmp=haversine_distance(lat, lon, lat_center_tmp, lon_center_tmp);
                if(distance_tmp>MINI_GEOFENCING_R)
                {
                    printk("\nGNSS_REV_DATA_THREAD: out mini geofence zone (distance=%d), put lat_lon to tx buffer!", (uint32_t)(round(distance_tmp)));
                    
                    int32_t lat_int=(int32_t)round(lat * 1000000.0);
                    int32_t lon_int=(int32_t)round(lon * 1000000.0);
                    location_data_t lat_lon={.lat=lat_int, .lon=lon_int};
                    if(location_data_pingpong_wr_idx==0) 
                    {
                        printk("\nGNSS_REV_DATA_THREAD: first sample in buffer, start timer!");
                        k_timer_start(&lte_start_tx_timer, K_MSEC(30000), K_FOREVER);
                    }
                    location_data_pingpong_wr_ptr[location_data_pingpong_wr_idx++]=lat_lon;
                    lat_center_tmp=lat; lon_center_tmp=lon;
                    printk("\nGNSS_REV_DATA_THREAD: updated lat_lon center tmp!");
                }
            }
            else
            {
                evt_q_put.evt_src=GEOF_IN_EVT;
                k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
            }
        }
        else
        {
            printk("\nGNSS_REV_DATA_THREAD: gps no fixed!");
            evt_q_put.evt_src=NO_FIX_GPS_EVT;
            k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
        }
        int thread_stop_ret=k_sem_take(&gnss_rx_stop_sem, K_NO_WAIT);
        if(!thread_stop_ret)
        {
            printk("\nGNSS_REV_DATA_THREAD: ready stop thread!");
            lat_center_tmp=lat_center; lon_center_tmp=lon_center;
            k_event_post(&modules_rx_stopped_evt, 0x1);
        }
        else k_sem_give(&gnss_rx_start_sem);
        k_msleep(1);
    }
}
K_THREAD_DEFINE(gnss_ctrl_thread_tid, 1024, gnss_ctrl_thread, NULL, NULL, NULL, 7, 0, 0);
/*******************************************************************************************************/
/*                                    GNSS READ & PROC DATA END                                        */
/*******************************************************************************************************/