/**
 * @file indev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

#include "stm32f7xx.h"

#include "ts.h"

#include "ft5336.h"

#include "tft.h"

//#include "stm32f769i_discovery.h"
//#include "stm32f769i_discovery_ts.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
bool touchpad_read_cb(lv_indev_drv_t * drv, lv_indev_data_t *data);

typedef enum
{
  TS_OK                = 0x00, /*!< Touch Ok */
  TS_ERROR             = 0x01, /*!< Touch Error */
  TS_TIMEOUT           = 0x02, /*!< Touch Timeout */
  TS_DEVICE_NOT_FOUND  = 0x03  /*!< Touchscreen device not found */
}TS_StatusTypeDef;
#define TS_MAX_NB_TOUCH                 ((uint32_t) FT5336_MAX_DETECTABLE_TOUCH)

/**
*  @brief TS_StateTypeDef
*  Define TS State structure
*/
typedef struct
{
  uint8_t  touchDetected;                /*!< Total number of active touches detected at last scan */
  uint16_t touchX[TS_MAX_NB_TOUCH];      /*!< Touch X[0], X[1] coordinates on 12 bits */
  uint16_t touchY[TS_MAX_NB_TOUCH];      /*!< Touch Y[0], Y[1] coordinates on 12 bits */


} TS_StateTypeDef;


#define TS_I2C_ADDRESS                   ((uint16_t)0x70)


/**********************
 *  STATIC VARIABLES
 **********************/
//static TS_StateTypeDef  TS_State;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/** @defgroup STM32746G_DISCOVERY_TS_Private_Variables STM32746G_DISCOVERY_TS Private Variables
  * @{
  */
static TS_DrvTypeDef *tsDriver;
static uint16_t tsXBoundary, tsYBoundary;
static uint8_t  tsOrientation;
static uint8_t  I2cAddress;
#if 0
/* Touch screen driver structure initialization */
TS_DrvTypeDef ft5336_ts_drv =
{
  ft5336_Init,
  ft5336_ReadID,
  ft5336_Reset,

  ft5336_TS_Start,
  ft5336_TS_DetectTouch,
  ft5336_TS_GetXY,

  ft5336_TS_EnableIT,
  ft5336_TS_ClearIT,
  ft5336_TS_ITStatus,
  ft5336_TS_DisableIT

};
#endif
/** @brief With FT5336 : maximum 5 touches detected simultaneously
  */
#define TS_MAX_NB_TOUCH                 ((uint32_t) FT5336_MAX_DETECTABLE_TOUCH)

#define TS_NO_IRQ_PENDING               ((uint8_t) 0)
#define TS_IRQ_PENDING                  ((uint8_t) 1)

#define TS_SWAP_NONE                    ((uint8_t) 0x01)
#define TS_SWAP_X                       ((uint8_t) 0x02)
#define TS_SWAP_Y                       ((uint8_t) 0x04)
#define TS_SWAP_XY                      ((uint8_t) 0x08)

/**
/**
/**
  * @brief  Initializes and configures the touch screen functionalities and
  *         configures all necessary hardware resources (GPIOs, I2C, clocks..).
  * @param  ts_SizeX: Maximum X size of the TS area on LCD
  * @param  ts_SizeY: Maximum Y size of the TS area on LCD
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_Init(uint16_t ts_SizeX, uint16_t ts_SizeY)
{
  uint8_t status = TS_OK;
  tsXBoundary = ts_SizeX;
  tsYBoundary = ts_SizeY;

  /* Read ID and verify if the touch screen driver is ready */
  ft5336_ts_drv.Init(TS_I2C_ADDRESS);
  if(ft5336_ts_drv.ReadID(TS_I2C_ADDRESS) == FT5336_ID_VALUE)
  {
    /* Initialize the TS driver structure */
    tsDriver = &ft5336_ts_drv;
    I2cAddress = TS_I2C_ADDRESS;
    tsOrientation = TS_SWAP_XY;

    /* Initialize the TS driver */
    tsDriver->Start(I2cAddress);
  }
  else
  {
    status = TS_DEVICE_NOT_FOUND;
  }

  return status;
}


/**
 * Initialize your input devices here
 */
void touchpad_init(void)
{
  BSP_TS_Init(TFT_HOR_RES, TFT_VER_RES);

  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.read_cb = touchpad_read_cb;
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


/**
  * @brief  Returns status and positions of the touch screen.
  * @param  TS_State: Pointer to touch screen current state structure
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_GetState(TS_StateTypeDef *TS_State)
{
  static uint32_t _x[TS_MAX_NB_TOUCH] = {0, 0};
  static uint32_t _y[TS_MAX_NB_TOUCH] = {0, 0};
  uint8_t ts_status = TS_OK;
  uint16_t x[TS_MAX_NB_TOUCH];
  uint16_t y[TS_MAX_NB_TOUCH];
  uint16_t brute_x[TS_MAX_NB_TOUCH];
  uint16_t brute_y[TS_MAX_NB_TOUCH];
  uint16_t x_diff;
  uint16_t y_diff;
  uint32_t index;

  /* Check and update the number of touches active detected */
  TS_State->touchDetected = tsDriver->DetectTouch(I2cAddress);

  if(TS_State->touchDetected)
  {
    for(index=0; index < TS_State->touchDetected; index++)
    {
      /* Get each touch coordinates */
      tsDriver->GetXY(I2cAddress, &(brute_x[index]), &(brute_y[index]));

      if(tsOrientation == TS_SWAP_NONE)
      {
        x[index] = brute_x[index];
        y[index] = brute_y[index];
      }

      if(tsOrientation & TS_SWAP_X)
      {
        x[index] = 4096 - brute_x[index];
      }

      if(tsOrientation & TS_SWAP_Y)
      {
        y[index] = 4096 - brute_y[index];
      }

      if(tsOrientation & TS_SWAP_XY)
      {
        y[index] = brute_x[index];
        x[index] = brute_y[index];
      }

      x_diff = x[index] > _x[index]? (x[index] - _x[index]): (_x[index] - x[index]);
      y_diff = y[index] > _y[index]? (y[index] - _y[index]): (_y[index] - y[index]);

      if ((x_diff + y_diff) > 5)
      {
        _x[index] = x[index];
        _y[index] = y[index];
      }

      if(I2cAddress == FT5336_I2C_SLAVE_ADDRESS)
      {
        TS_State->touchX[index] = x[index];
        TS_State->touchY[index] = y[index];
      }
      else
      {
        /* 2^12 = 4096 : indexes are expressed on a dynamic of 4096 */
        TS_State->touchX[index] = (tsXBoundary * _x[index]) >> 12;
        TS_State->touchY[index] = (tsYBoundary * _y[index]) >> 12;
      }


    } /* of for(index=0; index < TS_State->touchDetected; index++) */

  } /* end of if(TS_State->touchDetected != 0) */

  return (ts_status);
}

void touch(void)
{
	TS_StateTypeDef  ts;
	BSP_TS_GetState(&ts);
}

bool touchpad_read_cb(lv_indev_drv_t * drv, lv_indev_data_t *data)
{
	static int16_t last_x = 0;
	static int16_t last_y = 0;
	TS_StateTypeDef  ts;
	BSP_TS_GetState(&ts);
#if 1
	if(ts.touchDetected != 0) {
		data->point.x = ts.touchX[0];
		data->point.y = ts.touchY[0];
		last_x = data->point.x;
		last_y = data->point.y;
		data->state = LV_INDEV_STATE_PR;
	} else {
		data->point.x = last_x;
		data->point.y = last_y;
		data->state = LV_INDEV_STATE_REL;
	}
#endif
	return false;
}
