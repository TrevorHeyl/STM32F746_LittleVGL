/**
 * @file disp.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include <string.h>

#include "tft.h"
#include "stm32f7xx_hal.h"

/**
  * @brief  RK043FN48H Size
  */
#define  RK043FN48H_WIDTH    ((uint16_t)480)          /* LCD PIXEL WIDTH            */
#define  RK043FN48H_HEIGHT   ((uint16_t)272)          /* LCD PIXEL HEIGHT           */

/**
  * @brief  RK043FN48H Timing
  */
#define  RK043FN48H_HSYNC            ((uint16_t)41)   /* Horizontal synchronization */
#define  RK043FN48H_HBP              ((uint16_t)13)   /* Horizontal back porch      */
#define  RK043FN48H_HFP              ((uint16_t)32)   /* Horizontal front porch     */
#define  RK043FN48H_VSYNC            ((uint16_t)10)   /* Vertical synchronization   */
#define  RK043FN48H_VBP              ((uint16_t)2)    /* Vertical back porch        */
#define  RK043FN48H_VFP              ((uint16_t)2)    /* Vertical front porch       */

/**
  * @brief  RK043FN48H frequency divider
  */
#define  RK043FN48H_FREQUENCY_DIVIDER    5            /* LCD Frequency divider      */
//#include "stm32f769i_discovery.h"
//#include "stm32f769i_discovery_lcd.h"
//#include "stm32f769i_discovery_sdram.h"

/*********************
 *      DEFINES
 *********************/


#define VSYNC               1
#define VBP                 1
#define VFP                 1
#define VACT                480
#define HSYNC               1
#define HBP                 1
#define HFP                 1
#define HACT                800

#define LAYER0_ADDRESS               (0xc0000000)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/*For LittlevGL*/
static void tft_flush_cb(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);

/*LCD*/
static void LCD_Config(void);
static void LTDC_Init(void);

/*SD RAM*/
static void CopyBuffer(const uint16_t *pSrc, uint16_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize);

/**********************
 *  STATIC VARIABLES
 **********************/

extern LTDC_HandleTypeDef hltdc_discovery;
static DMA2D_HandleTypeDef hdma2d;
//extern DSI_HandleTypeDef hdsi_discovery;
//DSI_VidCfgTypeDef hdsivideo_handle;
//DSI_CmdCfgTypeDef CmdCfg;
//DSI_LPCmdTypeDef LPCmd;
//DSI_PLLInitTypeDef dsiPllInit;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;


static uint32_t * my_fb = (uint32_t *)LAYER0_ADDRESS;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize your display here
 */
void tft_init(void)
{

	//HAL_SDRAM_Init();
	//LCD_Config();
	//BSP_LCD_LayerDefaultInit(0, LAYER0_ADDRESS);
	//BSP_LCD_SelectLayer(0);

#if 0
	/* Send Display On DCS Command to display */
	HAL_DSI_ShortWrite(&(hdsi_discovery),
			0,
			DSI_DCS_SHORT_PKT_WRITE_P1,
			OTM8009A_CMD_DISPON,
			0x00);

	/*Refresh the LCD display*/
	HAL_DSI_Refresh(&hdsi_discovery);
#endif
	static lv_disp_buf_t disp_buf;
	static lv_color_t buf[TFT_HOR_RES * 100];
	lv_disp_buf_init(&disp_buf, buf, NULL, TFT_HOR_RES * 100);
	//lv_disp_buf_init(&disp_buf, 0xc0000000, 0xc0050000, 261120);


	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = tft_flush_cb;
	disp_drv.buffer = &disp_buf;

	lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void tft_flush_cb(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{

	CopyBuffer((const uint32_t *)color_p, my_fb, area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area));

	//HAL_DSI_Refresh(&hdsi_discovery);

	lv_disp_flush_ready(drv);
}
LTDC_LayerCfgTypeDef      pLayerCfg;

static void LCD_Config(void)
{
	  static LTDC_HandleTypeDef hltdc_F;
///	  LTDC_LayerCfgTypeDef      pLayerCfg;

	  /* LTDC Initialization -------------------------------------------------------*/

	  /* Polarity configuration */
	  /* Initialize the horizontal synchronization polarity as active low */
	  hltdc_F.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	  /* Initialize the vertical synchronization polarity as active low */
	  hltdc_F.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	  /* Initialize the data enable polarity as active low */
	  hltdc_F.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	  /* Initialize the pixel clock polarity as input pixel clock */
	  hltdc_F.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	  /* The RK043FN48H LCD 480x272 is selected */
	  /* Timing Configuration */
	  hltdc_F.Init.HorizontalSync = (RK043FN48H_HSYNC - 1);
	  hltdc_F.Init.VerticalSync = (RK043FN48H_VSYNC - 1);
	  hltdc_F.Init.AccumulatedHBP = (RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
	  hltdc_F.Init.AccumulatedVBP = (RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
	  hltdc_F.Init.AccumulatedActiveH = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
	  hltdc_F.Init.AccumulatedActiveW = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
	  hltdc_F.Init.TotalHeigh = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP + RK043FN48H_VFP - 1);
	  hltdc_F.Init.TotalWidth = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP + RK043FN48H_HFP - 1);

	  /* Configure R,G,B component values for LCD background color : all black background */
	  hltdc_F.Init.Backcolor.Blue = 0;
	  hltdc_F.Init.Backcolor.Green = 0;
	  hltdc_F.Init.Backcolor.Red = 0;

	  hltdc_F.Instance = LTDC;

	/* Layer1 Configuration ------------------------------------------------------*/

	  /* Windowing configuration */
	  /* In this case all the active display area is used to display a picture then :
	     Horizontal start = horizontal synchronization + Horizontal back porch = 43
	     Vertical start   = vertical synchronization + vertical back porch     = 12
	     Horizontal stop = Horizontal start + window width -1 = 43 + 480 -1
	     Vertical stop   = Vertical start + window height -1  = 12 + 272 -1      */
	  pLayerCfg.WindowX0 = 0;
	  pLayerCfg.WindowX1 = 480;
	  pLayerCfg.WindowY0 = 0;
	  pLayerCfg.WindowY1 = 272;

	  /* Pixel Format configuration*/
	  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;

	  /* Start Address configuration : frame buffer is located at FLASH memory */
	  pLayerCfg.FBStartAdress = 0xc0000000;//(uint32_t)&RGB565_480x272;

	  /* Alpha constant (255 == totally opaque) */
	  pLayerCfg.Alpha = 255;

	  /* Default Color configuration (configure A,R,G,B component values) : no background color */
	  pLayerCfg.Alpha0 = 0; /* fully transparent */
	  pLayerCfg.Backcolor.Blue = 0;
	  pLayerCfg.Backcolor.Green = 0;
	  pLayerCfg.Backcolor.Red = 0;

	  /* Configure blending factors */
	  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;

	  /* Configure the number of lines and number of pixels per line */
	  pLayerCfg.ImageWidth  = 480;
	  pLayerCfg.ImageHeight = 272;

	  /* Configure the LTDC */
	  if(HAL_LTDC_Init(&hltdc_F) != HAL_OK)
	  {
	    /* Initialization Error */
	    Error_Handler();
	  }

	  /* Configure the Layer*/
	  if(HAL_LTDC_ConfigLayer(&hltdc_F, &pLayerCfg, 1) != HAL_OK)
	  {
	    /* Initialization Error */
	    Error_Handler();
	  }
}

/**
 * @brief
 * @param  None
 * @retval None
 */
static void LTDC_Init(void)
{
	/* DeInit */
	HAL_LTDC_DeInit(&hltdc_discovery);

	/* LTDC Config */
	/* Timing and polarity */
	hltdc_discovery.Init.HorizontalSync = HSYNC;
	hltdc_discovery.Init.VerticalSync = VSYNC;
	hltdc_discovery.Init.AccumulatedHBP = HSYNC+HBP;
	hltdc_discovery.Init.AccumulatedVBP = VSYNC+VBP;
	hltdc_discovery.Init.AccumulatedActiveH = VSYNC+VBP+VACT;
	hltdc_discovery.Init.AccumulatedActiveW = HSYNC+HBP+HACT;
	hltdc_discovery.Init.TotalHeigh = VSYNC+VBP+VACT+VFP;
	hltdc_discovery.Init.TotalWidth = HSYNC+HBP+HACT+HFP;

	/* background value */
	hltdc_discovery.Init.Backcolor.Blue = 0;
	hltdc_discovery.Init.Backcolor.Green = 0;
	hltdc_discovery.Init.Backcolor.Red = 0;

	/* Polarity */
	hltdc_discovery.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc_discovery.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc_discovery.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc_discovery.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
	hltdc_discovery.Instance = LTDC;

	HAL_LTDC_Init(&hltdc_discovery);
}


static void CopyBuffer(const uint16_t *pSrc, uint16_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize)
{
#if 1
	uint32_t row;
	for(row = y; row < y + ysize; row++ ) {
		memcpy(&pDst[row * 480 + x], pSrc, xsize * 2);
		pSrc += xsize;
	}
#else
	pLayerCfg.FBStartAdress = pDst;
#endif

	/**********************
	 * Using DMA2D should be better and faster but makes artifacts on the screen
	 **********************/

//	uint32_t destination = (uint32_t)pDst + (y * 800 + x) * 4;
//	uint32_t source      = (uint32_t)pSrc;
//
//	/*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
//	hdma2d.Init.Mode         = DMA2D_M2M;
//	hdma2d.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
//	hdma2d.Init.OutputOffset = 800 - xsize;
//	hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/
//	hdma2d.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */
//
//	/*##-2- DMA2D Callbacks Configuration ######################################*/
//	hdma2d.XferCpltCallback  = NULL;
//
//	/*##-3- Foreground Configuration ###########################################*/
//	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
//	hdma2d.LayerCfg[1].InputAlpha = 0xFF;
//	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
//	hdma2d.LayerCfg[1].InputOffset = 0;
//	hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; /* No ForeGround Red/Blue swap */
//	hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */
//
//	hdma2d.Instance          = DMA2D;
//
//	/* DMA2D Initialization */
//	if(HAL_DMA2D_Init(&hdma2d) == HAL_OK)
//	{
//		if(HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
//		{
//			if (HAL_DMA2D_Start(&hdma2d, source, destination, xsize, ysize) == HAL_OK)
//			{
//				/* Polling For DMA transfer */
//				HAL_DMA2D_PollForTransfer(&hdma2d, 100);
//			}
//		}
//	}
}
