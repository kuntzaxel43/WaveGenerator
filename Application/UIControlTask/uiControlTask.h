/**
 * @file    uiControlTask.h
 * @brief   Processes user inputs from the rotary encoder without missing any steps.
 * @author  Axel Kuntz
 * @date    2026-06-16
 */

#ifndef UI_CONTROL_TASK_H
#define UI_CONTROL_TASK_H

/* -------------------------------------------------------------------------- */
/* Includes                                                                   */
/* -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Macros & Constants (#define)                                               */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* Typdefs(typedef, enum, struct)                                             */
/* -------------------------------------------------------------------------- */
/**
 * @brief Enumeration of possible encoder rotary directions
 */
typedef enum {
    ENCODER_DIR_NONE = 0,
    ENCODER_DIR_CW,       // clockwise
    ENCODER_DIR_CCW       // counter clockwise
} EncoderDirection_t;

/**
 * @brief struct, which holds current status
 */
typedef struct {
    int32_t currentPosition;
    EncoderDirection_t lastDirection;
    bool isPressed; // button press event, if rotary encoder supports this common feature
} EncoderStatus_t;

/* -------------------------------------------------------------------------- */
/* Global Variables (Use sparingly only when absolutely necessary!)           */
/* -------------------------------------------------------------------------- */
extern EncoderStatus_t g_encoderStatus; 

/* -------------------------------------------------------------------------- */
/* Function Prototypes (Public Interface)                                    */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Initializes the hardware and internal structures of the module.
 * @param  None
 * @retval None
 */
void UI_Control_Task_Init(void);

/**
 * @brief  Processes the latest encoder step and determines the rotation direction.
 * @param  status: Pointer to the status structure to be updated.
 * @retval The determined rotation direction as an enum.
 */
EncoderDirection_t UI_Control_Task_Process(EncoderStatus_t *status);


#endif /* UI_CONTROL_TASK_H */