#include "observer.h"


struct Wheel{  int delta;};

typedef etl::observer<Wheel> Wheel_Observer;

class Event_Handler1 : public Wheel_Observer{
  public:
  void notification(Wheel wheel)   {     
    Serial.println(wheel.delta);   
    } 
 };
 
   
class Mouse_Driver : public etl::observable<Wheel_Observer, 1> {
  public:
   void Wheel_Event_Up()   {     Wheel wheel = { 50 };     notify_observers(wheel);   }
   void Wheel_Event_Down()   {     Wheel wheel = { -25 };     notify_observers(wheel);   } };

void setup() {   
  Serial.begin(9600);
  Mouse_Driver   mouse_driver;  
  Event_Handler1 event_handler1;  
   mouse_driver.add_observer(event_handler1); 
   mouse_driver.Wheel_Event_Down(); 
   mouse_driver.Wheel_Event_Up(); 

  }

  void loop() {}
