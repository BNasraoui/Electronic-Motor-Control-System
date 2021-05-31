
#include "motor.h"


/* motor operation task */
void motorOperation()
{

   UInt events;

   for(;;) // spin here
    {
        events = Event_pend(motor_evtHandle, Event_Id_NONE, START_MOTOR | CHANGE_SPEED | ESTOP, BIOS_WAIT_FOREVER); // events to handle motor operation state machine

        switch(events)
        {
          case START_MOTOR:
              startMotorRoutine(); // start motor routine
              break;

          case CHANGE_SPEED: // make Ben change his stop motor event to a change speed event whereby the new speed = 0
              changeSpeedRoutine(0, false);
              break;

          case ESTOP:
              changeSpeedRoutine(0, true);
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
