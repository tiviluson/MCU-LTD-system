# A SMART LOW-TEMPERATURE FOOD DEHYDRATION SYSTEM

This is an assignment for my course Micro-controller & Micon-processor in Ho Chi Minh University of Technology.

## Description
The dry food can be preserved for a longer duration and is less susceptible to spoilage caused by the growth of
bacteria, molds, and insects. One of the most common techniques for the dry food is the use of Low Temperature
Dehydration (LTD). LTD is a process of reducing moisture of food to low level in low temperature environments. The
dry food using LTD can improve palatability, digestibility, color, flavor and appearance. In the industry, many
machines that embed the LTD technique for food preservation. Most of these machines are imported, stand-alone,
locally controlled and expensive. These machines mainly operate continuously or periodically or fixed
conditions. It leads to wasting energy, decreasing the machine life expectancy, increasing labor cost. Most importantly,
it does not save the sensor data that can be used to improve the system for the next operation.    
  
In this project, I target to build a SMART LTD System that supports centralized and real-time monitoring for
multiple LTD machines, high reliability, high availability and scalability as well as affordability for Vietnamese users.
The system includes LTD controllers, a centralized
web server and a cell-phone app. The LTD controller controls the LTD machine based on the temperature and humidity
inside and outside of the dry room. It sends temperature and humidity information to the IoT server in real time via
Wifi/3G connectivity. The IoT server saves the data to the database for the future use and send them to the user app.
The user app that can run on an Android or iOS cellphone, is used to check the status of the current operation of the
LTD machines and to send new commands for any LTD controller.

## Implementation
An individual LTD machine normally includes three **simulation** fans, a **simulation** heater, a **simulation** heat-pump/dehydration machine and an LTD
controller and an OLED display. The LTD controller can sense the temperature and humidity inside and outside the dry room as well as can
control fans, the heater and heat-pump/dehydration machine adaptively. Moreover, the LTD controller can also
obtain commands from the user app to operate appropriately.  
  
Since the operations of the SMART LTD system based on temperature and humidity sensor located inside the dry
room, it becomes a single point of failure in this system. We use a triple redundancy technique that uses three sensors
connected to three pins of a processor in order to improve the system reliability. This system still operates if one
sensor fails. The failed sensor is notified to the system admin so that it will be replaced as soon as possible without
interrupting the LTD machine operation.

## MCUs and Peripherals
- STM32F103RB Board
<p align="center">
    <img src="/assests/81EKNlvGY1L.jpg" width="200">
</p>
- DHT11 sensor for temperature and humidity
<p align="center">
    <img src="/assests/41015728-1.jpg" width="200">
</p>
- 0.96 inch OLED display with I2C protocol
<p align="center">
    <img src="/assests/61wdwkxs6ol-sl1000.jpg" width="200">
</p>
- Heater and Heat pump are simulated (a relationship of linearity is assumed between the temperature and time).

## Results
<p align="center">
    <img src="/20201231_155339.jpg" height="200">
    <img src="/assests/133009585_238083141023115_1379654123980068988_n%20(1).jpg" height="200">
</p>
