# EEE4168_Embedded_System_Design  

***  

## Environment  
    IDE: STM32CubeIDE 1.16.0  
    OS: FreeRTOS  
    Board: NUCLEO-F746ZG  
    Book: FreeRTOS Manual  

***  

### Assignment 1: Toggling LED  
    score 20/20  
      
    Problem 1: The attached main.c file toggles the Green LED at a fixed interval using the GreenTask. Using this as a reference: Create two additional tasks, BlueTask and RedTask, with the same priority as GreenTask. Modify the program so that each task toggles a different LED (Green, Blue, Red) at distinct intervals.  
      
    Problem 2: Achieve the same LED toggling behavior as in Problem 1, but rewrite the code using only software timers and callback functions, as shown in Example 6 of the textbook.  
      
    Review Point 1: Use the implementation from Problem 1 to observe task preemption by assigning different priorities to the tasks and testing their behavior.  
      
    Review Point 2: In Problem 1, implement a single function to create all three tasks. Use parameters to ensure the tasks behave as specified in the result of Problem 1.  
      

***  

### Assignment 2: Interrupt Service Routine Using Semaphore and Queue  
    score 20/20  
      
    Problem: Write code to implement the following functionality on the board: Each time the User Button is pressed, the system cycles through states (0 → 1 → 2 → 3 → 0 ...). The states determine the status of the Red LED and Blue LED.  
      
    To gain experience with Semaphore and Queue, create the following two tasks:  
      
    1. Deferred Processing Task  
    This task works in conjunction with an ISR and processes events triggered by the User Button: When the User Button is pressed, an interrupt is triggered, calling the HAL_GPIO_EXTI_callback function (defined in main.c). The ISR uses xSemaphoreGiveFromISR to synchronize the task using a semaphore. Each time the task is unblocked, it increments the state variable by 1, cycling through values 0 to 3. The updated state is sent to the next task via the QueueSend API.  
      
    2. LED Display Task  
    This task performs the following: Receives the state data sent by the previous task using the QueueReceive API. Depending on the received state:  
      
    0: Turn both LEDs off.  
    1: Turn Red LED on.  
    2: Turn Blue LED on.  
    3: Turn both LEDs on.  
      
***  

### Project: String Display with Dot matrix display  
    score 108/110  
      
    refer to Project2024.pdf file for description  
    there is a report named 20211606_한석기_보고서  

***  
