# 🔄 UI-Input Task & ISR Synchronization Workflow
This document explains the real-time synchronization between the hardware Interrupt Service Routine (ISR) and the FreeRTOS UI-Input Task using the **Deferred Interrupt Processing** pattern tailored for the **STM32 Nucleo-F767ZI (Cortex-M7)**.

```mermaid
graph TD
    A["Hardware: Encoder Rotation"] -->|Triggers Falling Edge| B["HAL_GPIO_EXTI_Callback<br/>(ISR Context via EXTI2)"]
    B -->|xQueueSendFromISR| C["FreeRTOS Message Queue<br/>(Buffers step data: +1 / -1)"]
    C -->|Unblocks highest priority task| D["vUiInputTask<br/>(FreeRTOS Task)"]
    D -->|Processes data & updates Freq<br/>0% CPU while waiting| E["Task Blocked<br/>Yields CPU"]
    
    style B fill:#ff6b6b
    style C fill:#4ecdc4
    style D fill:#45b7d1
    style E:#96ceb4
```

---

## 📌 Hardware Pin Mapping & Configuration
To capture the quadrature encoder signals efficiently without polling, the system maps the digital rotation signals to the expanded **ST Zio Connectors (CN8 and CN9)** of the Nucleo-F767ZI board. The pushbutton switch (SW) is omitted to simplify the architecture for pure frequency adjustment.

| Encoder Pin | Nucleo-F767ZI Pin | Board Header Placement | Mode / STM32 Configuration | Purpose |
| :--- | :--- | :--- | :--- | :--- |
| **GND** | **GND** | Any GND Pin | Ground | Common system ground reference |
| **+ (VCC)** | **3.3V** | Any 3.3V Pin | Power Supply | Powering the internal encoder pull-up board |
| **CLK** | **PG2** | **CN8** (Pin 14) | `GPIO_MODE_IT_FALLING` with `GPIO_PULLUP` | Hardware Interrupt trigger source (`EXTI2_IRQn`) |
| **DT** | **PG1** | **CN9** (Pin 20) | `GPIO_MODE_INPUT` (No Pull) | Digital input pin to sample direction inside the ISR |

---

## 🛠️ Implementation Example

### 1. The Interrupt Service Routine (ISR Context)
Located in your GPIO interrupt management file (e.g., `stm32f7xx_it.c` or a dedicated encoder driver module).

```c
#include "FreeRTOS.h"
#include "queue.h"
#include "main.h" // For STM32 HAL and GPIO Pin definitions

/* Global or extern handle for the communication queue */
extern QueueHandle_t encoderQueueHandle;

/**
 * @brief  EXTI line detection callback.
 *         Triggered by hardware on the falling edge of CLK (PG2).
 * @param  GPIO_Pin: Specifies the pin connected to EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* Check if the interrupt came from Encoder CLK (PG2) */
    if (GPIO_Pin == GPIO_PIN_2) 
    {
        int encoderStep = 0;

        /* Read DT (PG1) to determine the relative direction of rotation */
        if (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_1) == GPIO_PIN_SET) 
        {
            encoderStep = 1;  /* Clockwise (CW) -> Increment */
        } 
        else 
        {
            encoderStep = -1; /* Counter-Clockwise (CCW) -> Decrement */
        }

        /* Track if a context switch is required upon exiting the ISR */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        /* Push the calculated relative step into the queue safely from ISR context */
        xQueueSendFromISR(encoderQueueHandle, &encoderStep, &xHigherPriorityTaskWoken);

        /* Yield the processor if the UI Task has higher priority than the interrupted task */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
```

### 2. The UI-Input Task Wrapper (OS Context)
Located inside your `freertos.c` or specialized task runner file.

```c
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Global handle for the communication queue */
QueueHandle_t encoderQueueHandle = NULL;

/**
 * @brief  UI-Input Task Function.
 *         Highest priority task. Manages user inputs with near-zero latency.
 * @param  argument: Not used
 * @retval None (Must never return!)
 */
void vUiInputTask(void *argument)
{
    /* Initialize the queue with space for 10 entries of 'int' size */
    encoderQueueHandle = xQueueCreate(10, sizeof(int));
    
    int receivedStep = 0;
    int32_t currentFrequencyHz = 1000; // Default start frequency: 1 kHz
    const int32_t frequencyStepDelta = 50; // Change by 50 Hz per click

    /* Infinite Task Loop */
    for(;;)
    {
        /* 
         * Execution stops here (Blocked State). The task sleeps and yields 
         * 100% CPU time to lower priority tasks until a message arrives.
         */
        if (xQueueReceive(encoderQueueHandle, &receivedStep, portMAX_DELAY) == pdPASS)
        {
            /* Calculate relative frequency change */
            currentFrequencyHz += (receivedStep * frequencyStepDelta);
            
            /* Enforce application boundaries (Saturating Logic) */
            if (currentFrequencyHz < 50)     currentFrequencyHz = 50;     // Min Boundary: 50 Hz
            if (currentFrequencyHz > 20000)  currentFrequencyHz = 20000;  // Max Boundary: 20 kHz
            
            /* 
             * TODO: Forward the absolute 'currentFrequencyHz' value to 
             * the Control Task / FPGA Driver Task via an IPC Queue or Event Flag.
             */
        }
    }
}
```

---

## 🔑 Crucial Architectural Rules

1. **Never block inside the ISR:** Only use non-blocking FreeRTOS API calls ending with `...FromISR`. Never use delay functions or blocking reads inside the callback.
2. **Use `portYIELD_FROM_ISR()`:** This enforces immediate execution of your high-priority UI task after the hardware interrupt finishes, eliminating any latency or lagging.
3. **Strict NVIC Prioritization:** Since this ISR interfaces with FreeRTOS APIs, the NVIC priority for `EXTI2_IRQn` **MUST** be set numerically equal to or higher than `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` (typically `5` on STM32 Ecosystems). Setting it to 0-4 will cause system crashes.
4. **Queue Sizing:** A depth of `10` is perfectly balanced for rapid manual user rotations. If the queue fills up, further ISR steps will be safely discarded without corrupting data.
