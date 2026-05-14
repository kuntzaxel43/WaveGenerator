#include "retarget.h"

static UART_HandleTypeDef *gHuart = NULL;

void Retarget_Init(UART_HandleTypeDef *huart) {
    gHuart = huart;
    
    // Schaltet den internen C-Textpuffer komplett ab
    setvbuf(stdout, NULL, _IONBF, 0);
}

// Kopplung für picolibc / GCC-Standard
int __io_putchar(int ch) {
    if (gHuart != NULL) {
        HAL_UART_Transmit(gHuart, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    }
    return ch;
}

// Kopplung für Newlib-nano / alternative Compiler
int _write(int file, char *ptr, int len) {
    if (gHuart != NULL) {
        HAL_UART_Transmit(gHuart, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    }
    return len;
}
