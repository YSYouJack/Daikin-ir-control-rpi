# Daikin-ir-control-rpi
This project contains ir remote control utility programs to decode and send the ir signal of Daikin air conditioner. The host system is RaspberryPi 3B+ with Raspbian. The Daikin remoter model is ARC433A59. 

The decoder program is `arc433A59_ir_decoder`, which reads the raw ir signal from standard input and showes the decoded bytes in hex on standard output. The `arc433A59_ir_decoder` is designed to be used with the `ir-ctl` utlity program in vl4-utils package, just like the following command.
  
    ir-ctl -r | arc433A59_ir_decoder

The sender program is `arc433A59_ir_tx`, which reads the command-line arguments, composes the ir signal and sends to the AC. Instead of using the LIRC package, the `arc433A59_ir_tx` use pigpio library to send the modulated ir signal. (Note: The timer and econo functions has not been implemented in the sender)

    arc433A59_ir_tx -e pigpiod -g 17 -p on -t 25 # GPIO 17, Power on, Temperture 25

## System
- RaspberryPi 3B+ & Raspbian
- IR Receiver: [KY-022](http://sensorkit.en.joy-it.net/index.php?title=KY-022_Infrared_receiver_module) Signal: GPIO18, VCC: 3.3V
- IR Transmitter: [KY-005](http://sensorkit.en.joy-it.net/index.php?title=KY-005_Infrared_Transmitter_module) Signal: GPIO17, VCC: 5V

## Setup IR Receiver
Edit the `/boot/config.txt` and adds the following line.

    dtoverlay=gpio-ir,gpio_pin=18

Reboot. Type `ir-ctl -r` in shell and you should see the input.

## Setup IR Transmitter
Just make sure the `pigpiod` is running. You can type `sudo pigpiod` to start the daemon.

## Daikin IR Protocol Details (ARC433A59)
The bytes format of the Daikin protocol is Least Significant Bit(LSB). Each remote command contains 2 ir message frames. The 1st message frame contains 8 bytes, and the 2nd is 19 bytes. Each ir frame has a starting bit and an end bit. The encoding method is Pulse Distance Encoding. The following table shows the wave property of the protocol.

Name                 | Pulse Length (us) | Space Length (us)
---------------------|-------------------|-------------------
Start bits           | 3400              | 1750
Bit 1                | 430               | 1320
Bit 0                | 430               | 430
End bits             | 430               | N/A
Gap between ir frame | N/A               | 35000

### The First Frame 8Bytes
The 1st frame is the header. The content is fixed.
    
    11 da 27 f0 00 00 00 02
    
### The Second Frame 19Bytes
The second frame composes the control command.

Byte Offset     | Description                                            | Example
----------------|--------------------------------------------------------|------------------
0~4             | Fixed                                                  | `11 da 27 00 00`
5               | Mode (Auto: 00, Dry: 02, Cold: 03, Heat: 04, Fan: 06)  | `03`
6               | Temperature = Input x 2                                | `32` # 25 degree
7               | Fixed                                                  | `00`
8(Upper 4 bits) | Fan (Auto: A, Silent: B, 1: 3, 2: 4, 3: 5, 4: 6, 5: 7) | `AF`
8(Lower 4 bits) | Swing (On: F, Off: 0)                                  | 
9               | Fixed                                                  | `00`
10~12           | Timer(Not Used)                                        | `00 00 00`
13              | Powerful (On: 1, Off: 0)                               | `00`
14~15           | Fixed                                                  | `00 c0`
16              | Econo(Not Used)                                        | `00`
17              | Fixed                                                  | `00`
18              | Checksum = Sum(Byte[0] ~ Byte[17]) AND 0xFF            | `32`

## References
1. https://blog.bschwind.com/2016/05/29/sending-infrared-commands-from-a-raspberry-pi-without-lirc/
2. https://github.com/bschwind/ir-slinger
3. https://github.com/blafois/Daikin-IR-Reverse

## Contributors
- The cmake\Findpigpio.cmake comes from the github of [joan2937/pigpio](https://github.com/joan2937/pigpio).
