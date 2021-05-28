
#include "motor.h"


/* motor operation task */
void motorOperation()
{

   UInt events;

   for(;;) // spin here
    {
        events = Event_pend(motor_evtHandle, Event_Id_NONE, START_MOTOR | STOP_MOTOR | ESTOP | SPEED_UP | SLOW_DOWN, BIOS_WAIT_FOREVER); // events to handle motor operation state machine

        switch(events)
        {
          case START_MOTOR:
              startMotorRoutine(); // start motor routine
              break;

          case STOP_MOTOR:
              stopMotorRoutine(false);
              break;

          case ESTOP:
              stopMotorRoutine(true);
              break;

          case SPEED_UP:
              // remove ?
              break;

          case SLOW_DOWN:
              // remove ?
              break;

          default:
              break;

        }
    }
}


/*
 *  ======== main ========
 */
int main(void)
{
    // initialise the motor subsystem
    initMotorDriver();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
