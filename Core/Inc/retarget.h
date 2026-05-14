#ifndef __RETARGET_H__
#define __RETARGET_H__

#include "stm32f7xx_hal.h" // Tausche f7 gegen deine Serie (z.B. f4, g4, h7), falls du einen anderen Chip nutzt
#include <stdio.h>

// Initialisiert die printf-Umleitung und schaltet den Puffer aus
void Retarget_Init(UART_HandleTypeDef *huart);

#endif /* __RETARGET_H__ */
