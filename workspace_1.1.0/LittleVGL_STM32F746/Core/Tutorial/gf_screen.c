
#include "lvgl.h"


/**********************
 *   STATIC FUNCTIONS
 **********************/
lv_obj_t * gauge1;
lv_obj_t * led1;

static void slider_event_cb(lv_obj_t * slider, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED)
    {
        //static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        //snprintf(buf, 4, "%u", lv_slider_get_value(slider));
        //lv_label_set_text(slider_label, buf);
    	lv_gauge_set_value(gauge1, 0,lv_slider_get_value(slider));
    	if (lv_slider_get_value(slider) > 60)
    	{
    		lv_led_on(led1);
    	}else
    	{
    		lv_led_off(led1);
    	}
    }
}

void FirstTest(void)
{

	/*Create a style*/
	static lv_style_t style;
	lv_style_copy(&style, &lv_style_pretty_color);
	style.body.main_color = lv_color_hex3(0x666);     /*Line color at the beginning*/
	style.body.grad_color =  lv_color_hex3(0x666);    /*Line color at the end*/
	style.body.padding.left = 10;                      /*Scale line length*/
	style.body.padding.inner = 8 ;                    /*Scale label padding*/
	style.body.border.color = lv_color_hex3(0x333);   /*Needle middle circle color*/
	style.line.width = 3;
	style.text.color = lv_color_hex3(0x333);
	style.line.color = LV_COLOR_RED;                  /*Line color after the critical value*/

	/*Get Current Scrren*/
	lv_obj_t * obj1 = lv_page_create(NULL,NULL);
	lv_disp_load_scr(obj1);
	lv_obj_set_size(obj1,480,272);
	lv_obj_t * btn1 = lv_btn_create(obj1,NULL);
	lv_obj_set_size(btn1,100,40);
	lv_obj_set_pos(btn1,50,10);

	//create two more buttons
	lv_obj_t * btn2 = lv_btn_create(obj1,btn1);
	lv_obj_t * btn3 = lv_btn_create(obj1,btn1);

	lv_obj_align(btn2,btn1,LV_ALIGN_OUT_RIGHT_MID,10,0);
	lv_obj_align(btn3,btn1,LV_ALIGN_OUT_BOTTOM_LEFT,0,10);

	lv_obj_t * label1 = lv_label_create(btn1,NULL);
	lv_label_set_text(label1,"Graeme");
	lv_obj_align(label1,btn1,LV_ALIGN_IN_BOTTOM_LEFT,0,0);

	gauge1 = lv_gauge_create(obj1,NULL);
	lv_obj_align(gauge1,obj1,LV_ALIGN_CENTER,0,70);
	lv_gauge_set_style(gauge1, LV_GAUGE_STYLE_MAIN, &style);

	/*Set the values*/
	//lv_gauge_set_value(gauge1, 0, 45);

	lv_obj_t * slider1 = lv_slider_create(obj1,NULL);
	lv_obj_align(slider1,btn2,LV_ALIGN_OUT_RIGHT_MID,0,0);
    lv_obj_set_event_cb(slider1, slider_event_cb);
    lv_slider_set_range(slider1, 0, 100);

    led1  = lv_led_create(obj1, NULL);
	lv_obj_align(led1,btn3,LV_ALIGN_CENTER,0,70);
	lv_led_off(led1);




}
