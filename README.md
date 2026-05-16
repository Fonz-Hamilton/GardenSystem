# GardenSystem
 Automated watering for small garden

## Pinout and Wires

ESP32-S3 (Hosyond N16R8)

                                                                      ┌────────────────┐
                                                                      │                │ 
                                                                   ┌──┴────────────────┴──┐
                                                          (3V3)─── ├3V3┌──────────────┐GND┤ ───(GND)
                                                          (3V3)─── ├3V3│              │ TX┤ ~~~(U0TXD)(GPIO43)(CLK_OUT1)
                                                          (RST)─── ├RST│              │ RX┤ ~~~(U0RXD)(GPIO44)(CLK_OUT2)
                                   (ADC1_3)(TOUCH4)(RTC)(GPIO4)~~~ ├ 4 │              │ 1 ┤ ~~~(GPIO1)(RTC)(TOUCH1)(ADC1_0)
                                   (ADC1_4)(TOUCH5)(RTC)(GPIO5)~~~ ├ 5 │              │ 2 ┤ ~~~(GPIO2)(RTC)(TOUCH2)(ADC1_1)
                                   (ADC1_5)(TOUCH6)(RTC)(GPIO6)~~~ ├ 6 │              │42 ┤ ~~~(MTMS)(GPIO42)
                                   (ADC1_6)(TOUCH7)(RTC)(GPIO7)~~~ ├ 7 └──────────────┘41 ┤ ~~~(MTDI)(GPIO41)(CLK_OUT1)
                       (XTAL_32K_P)(ADC2_4)(U0RTS)(RTC)(GPIO15)~~~ ├ 15                40 ┤ ~~~(MTDO)(GPIO40)(CLK_OUT2)
                       (XTAL_32K_N)(ADC2_5)(U0CTS)(RTC)(GPIO16)~~~ ├ 16           ┌───┐39 ┤ ~~~(MTCK)(GPIO39)(CLK_OUT3)(SUBSPICS1)
                                   (ADC2_6)(U1TXD)(RTC)(GPIO17)~~~ ├ 17           │RST│38 ┤ ~~~(GPIO38)(FSPIWP)(SUBSPIWP)(RGB_LED)
                         (CLK_OUT3)(ADC2_7)(U1RXD)(RTC)(GPIO18)~~~ ├ 18           └───┘37 ┤ ~~~(GPIO37)(SPIDQS)(FSPIQ)(SUBSPIQ)
                                   (ADC1_7)(TOUCH8)(RTC)(GPIO8)~~~ ├ 8                 36 ┤ ~~~(GPIO36)(SPIIO7)(FSPICLK)(SUBSPICLK)
                             (JTAG)(ADC1_2)(TOUCH3)(RTC)(GPIO3)~~~ ├ 3            ┌───┐35 ┤ ~~~(GPIO35)(SPIIO6)(FSPID)(SUBSPID)
                                                  (LOG)(GPIO46)~~~ ├ 46           │BT │ 0 ┤ ~~~(GPIO0)(BOOT)
                 (FSPIHD)(SUBSPIHD)(ADC1_8)(TOUCH9)(RTC)(GPIO9)~~~ ├ 9            └───┘45 ┤ ~~~(GPIO45)(VSPI)
     (FSPICS0)(SUBSPICS0)FSPIIO4)(ADC1_9)(TOUCH10)(RTC)(GPIO10)~~~ ├ 10                48 ┤ ~~~(GPIO48)(SPICLK_N)
        (FSPID)(SUBSPID)(FSPIIO5)(ADC2_0)(TOUCH11)(RTC)(GPIO11)~~~ ├ 11                47 ┤ ~~~(GPIO47)(SPICLK_P)
    (FSPICLK)(SUBSPICLK)(FSPIIO6)(ADC2_1)(TOUCH12)(RTC)(GPIO12)~~~ ├ 12                21 ┤ ~~~(GPIO21)(RTC)
        (FSPIQ)(SUBSPIQ)(FSPIIO7)(ADC2_2)(TOUCH13)(RTC)(GPIO13)~~~ ├ 13                20 ┤ ~~~(USB_D+)(GPIO20)(RTC)(U1CTS)(ADC2_9)(CLK_OUT1)
      (FSPIWP)(SUBSPIWP)(FSPIDQS)(ADC2_3)(Touch14)(RTC)(GPIO14)~~~ ├ 14                19 ┤ ~~~(USB_D-)(GPIO19)(RTC)(U1RTS)(ADC2_8)(CLK_OUT2)
                                                          (5V0)─── ├5V0 ┌────┐  ┌────┐ GND┤ ───(GND)
                                                          (GND)─── ├GND │USB │  │UART│ GND┤ ───(GND)
                                                                   └────┼────┼──┼────┼────┘ 
                                                                        └────┘  └────┘  


DfRobot moisture sensor:

    Red     -   VCC
    Black   -   GND
    Yellow  -   Signal
    Black   -   GND



VEML7700 Ambient Light Sensor:
Default I2C Address: 0x10

    1       -   Vin; Power in
    2       -   3Vo; 3.3V output from voltage regulator
    3       -   GND; Common Ground
    4       -   SCL; I2C clock pin. Connect to microcontroller I2C clock line
    5       -   SDA; I2C data pin. Connect to Microcontroller I2C Data Line



BME280 Temp + Pressure
By default, the I2C address is 0x77.  If you add a jumper from SDO to GND with a wire, or by soldering the ADDR jumper on the back closed, the address will change to 0x76

    1       -   Vin; Power in
    2       -   3Vo; 3.3V output from voltage regulator
    3       -   GND; Common Ground
    4       -   SCK; SPI Clock pin
    5       -   SDO; Serial Data Out
    6       -   SDI; Serial Data In
    7       -   CS; Chip Select



GY-8511 ML8511 UVB UV Rays Sensor

    1       -   Vin; Power in
    2       -   3V3; Voltage output from voltage regulator
    3       -   GND; Common Ground
    4       -   OUT; Analog Ouput
    5       -   EN; Enable



SHT31 WEATHER-PROOF TEMPERATURE

I2C address is 0x44

    Red     -   VCC
    Black   -   GND
    Yellow  -   SCK or SCL (Datasheet says SCL while tag on it says SCK); SPI clock pin or I2C clock pin. Connect to microcontroller I2C clock line
    Green   -   SDA; I2C data pin. Connect to Microcontroller I2C Data Line
