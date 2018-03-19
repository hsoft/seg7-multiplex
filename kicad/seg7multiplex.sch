EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:custom
LIBS:seg7multiplex-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L 7SEGMENTS AFF1
U 1 1 5AAD7BA2
P 4200 3800
F 0 "AFF1" H 4200 4350 50  0000 C CNN
F 1 "7SEGMENTS" H 4200 3350 50  0000 C CNN
F 2 "" H 4200 3800 50  0000 C CNN
F 3 "" H 4200 3800 50  0000 C CNN
	1    4200 3800
	1    0    0    -1  
$EndComp
$Comp
L 7SEGMENTS AFF2
U 1 1 5AAD7CF0
P 5750 3800
F 0 "AFF2" H 5750 4350 50  0000 C CNN
F 1 "7SEGMENTS" H 5750 3350 50  0000 C CNN
F 2 "" H 5750 3800 50  0000 C CNN
F 3 "" H 5750 3800 50  0000 C CNN
	1    5750 3800
	1    0    0    -1  
$EndComp
$Comp
L 7SEGMENTS AFF3
U 1 1 5AAD7D67
P 7300 3800
F 0 "AFF3" H 7300 4350 50  0000 C CNN
F 1 "7SEGMENTS" H 7300 3350 50  0000 C CNN
F 2 "" H 7300 3800 50  0000 C CNN
F 3 "" H 7300 3800 50  0000 C CNN
	1    7300 3800
	1    0    0    -1  
$EndComp
$Comp
L 7SEGMENTS AFF4
U 1 1 5AAD7DEE
P 8850 3800
F 0 "AFF4" H 8850 4350 50  0000 C CNN
F 1 "7SEGMENTS" H 8850 3350 50  0000 C CNN
F 2 "" H 8850 3800 50  0000 C CNN
F 3 "" H 8850 3800 50  0000 C CNN
	1    8850 3800
	1    0    0    -1  
$EndComp
$Comp
L SN74HC595 U1
U 1 1 5AADB644
P 4100 1750
F 0 "U1" H 4250 2350 50  0000 C CNN
F 1 "SN74HC595" H 4100 1150 50  0000 C CNN
F 2 "" H 4100 1750 50  0000 C CNN
F 3 "" H 4100 1750 50  0000 C CNN
	1    4100 1750
	1    0    0    -1  
$EndComp
$Comp
L SN74HC161 U2
U 1 1 5AAE5EEA
P 5450 5650
F 0 "U2" H 5450 5750 50  0000 C CNN
F 1 "SN74HC161" H 5450 5550 50  0000 C CNN
F 2 "" H 5450 5650 50  0000 C CNN
F 3 "" H 5450 5650 50  0000 C CNN
	1    5450 5650
	1    0    0    -1  
$EndComp
$Comp
L 74LS48 U3
U 1 1 5AAE6041
P 7500 5750
F 0 "U3" H 7500 5850 50  0000 C CNN
F 1 "74LS48" H 7500 5650 50  0000 C CNN
F 2 "" H 7500 5750 50  0000 C CNN
F 3 "" H 7500 5750 50  0000 C CNN
	1    7500 5750
	1    0    0    -1  
$EndComp
Text Label 8300 5400 0    60   ~ 0
a
Text Label 8300 5500 0    60   ~ 0
b
Text Label 8300 5600 0    60   ~ 0
c
Text Label 8300 5700 0    60   ~ 0
d
Text Label 8300 5800 0    60   ~ 0
e
Text Label 8300 5900 0    60   ~ 0
f
Text Label 8300 6000 0    60   ~ 0
g
Text Label 8150 3400 0    60   ~ 0
a
Text Label 8150 3500 0    60   ~ 0
b
Text Label 8150 3600 0    60   ~ 0
c
Text Label 8150 3700 0    60   ~ 0
d
Text Label 8150 3800 0    60   ~ 0
e
Text Label 8150 3900 0    60   ~ 0
f
Text Label 8150 4000 0    60   ~ 0
g
Text Label 6600 3400 0    60   ~ 0
a
Text Label 6600 3500 0    60   ~ 0
b
Text Label 6600 3600 0    60   ~ 0
c
Text Label 6600 3700 0    60   ~ 0
d
Text Label 6600 3800 0    60   ~ 0
e
Text Label 6600 3900 0    60   ~ 0
f
Text Label 6600 4000 0    60   ~ 0
g
Text Label 5050 3400 0    60   ~ 0
a
Text Label 5050 3500 0    60   ~ 0
b
Text Label 5050 3600 0    60   ~ 0
c
Text Label 5050 3700 0    60   ~ 0
d
Text Label 5050 3800 0    60   ~ 0
e
Text Label 5050 3900 0    60   ~ 0
f
Text Label 5050 4000 0    60   ~ 0
g
Text Label 3500 3400 0    60   ~ 0
a
Text Label 3500 3500 0    60   ~ 0
b
Text Label 3500 3600 0    60   ~ 0
c
Text Label 3500 3700 0    60   ~ 0
d
Text Label 3500 3800 0    60   ~ 0
e
Text Label 3500 3900 0    60   ~ 0
f
Text Label 3500 4000 0    60   ~ 0
g
Text Label 4800 1700 0    60   ~ 0
DP0
Text Label 4800 1800 0    60   ~ 0
DP1
Text Label 4800 1900 0    60   ~ 0
DP2
Text Label 4800 2000 0    60   ~ 0
DP3
Text Label 4800 4050 0    60   ~ 0
DP3
Text Label 6350 4050 0    60   ~ 0
DP2
Text Label 7900 4050 0    60   ~ 0
DP1
Text Label 9450 4050 0    60   ~ 0
DP0
$Comp
L PWR_FLAG #FLG01
U 1 1 5AAE80FD
P 950 1000
F 0 "#FLG01" H 950 1095 50  0001 C CNN
F 1 "PWR_FLAG" H 950 1180 50  0000 C CNN
F 2 "" H 950 1000 50  0000 C CNN
F 3 "" H 950 1000 50  0000 C CNN
	1    950  1000
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR02
U 1 1 5AAE812D
P 1350 1100
F 0 "#PWR02" H 1350 850 50  0001 C CNN
F 1 "GND" H 1350 950 50  0000 C CNN
F 2 "" H 1350 1100 50  0000 C CNN
F 3 "" H 1350 1100 50  0000 C CNN
	1    1350 1100
	1    0    0    -1  
$EndComp
Text Label 950  1600 0    60   ~ 0
VCC
Text Label 1350 1100 0    60   ~ 0
GND
Text Label 3200 1600 0    60   ~ 0
VCC
NoConn ~ 4800 2200
NoConn ~ 4800 3450
NoConn ~ 6350 3450
NoConn ~ 7900 3450
NoConn ~ 9450 3450
Text Label 6200 5900 0    60   ~ 0
VCC
NoConn ~ 6150 5800
NoConn ~ 4750 6000
Text Label 4600 5800 0    60   ~ 0
VCC
NoConn ~ 6150 5300
NoConn ~ 4750 5600
NoConn ~ 4750 5500
NoConn ~ 4750 5400
NoConn ~ 4750 5300
NoConn ~ 6800 6100
NoConn ~ 6800 6000
NoConn ~ 6800 5900
$Comp
L ATTINY45-P IC1
U 1 1 5AAE97E0
P 1950 2750
F 0 "IC1" H 800 3150 50  0000 C CNN
F 1 "ATTINY45-P" H 2950 2350 50  0000 C CNN
F 2 "DIP8" H 2950 2750 50  0000 C CIN
F 3 "" H 1950 2750 50  0000 C CNN
	1    1950 2750
	-1   0    0    1   
$EndComp
NoConn ~ 3300 2500
Text Label 3300 3000 0    60   ~ 0
PB0
Text Label 4600 5900 0    60   ~ 0
PB0
$Comp
L C C1
U 1 1 5AAEAC1D
P 700 1250
F 0 "C1" H 725 1350 50  0000 L CNN
F 1 "1uF" H 725 1150 50  0000 L CNN
F 2 "" H 738 1100 50  0000 C CNN
F 3 "" H 700 1250 50  0000 C CNN
	1    700  1250
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 5AAEB0A8
P 4800 2950
F 0 "R1" V 4880 2950 50  0000 C CNN
F 1 "110" V 4800 2950 50  0000 C CNN
F 2 "" V 4730 2950 50  0000 C CNN
F 3 "" H 4800 2950 50  0000 C CNN
	1    4800 2950
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 5AAEB58C
P 6350 2950
F 0 "R2" V 6430 2950 50  0000 C CNN
F 1 "110" V 6350 2950 50  0000 C CNN
F 2 "" V 6280 2950 50  0000 C CNN
F 3 "" H 6350 2950 50  0000 C CNN
	1    6350 2950
	1    0    0    -1  
$EndComp
$Comp
L R R3
U 1 1 5AAEB60B
P 7900 2950
F 0 "R3" V 7980 2950 50  0000 C CNN
F 1 "110" V 7900 2950 50  0000 C CNN
F 2 "" V 7830 2950 50  0000 C CNN
F 3 "" H 7900 2950 50  0000 C CNN
	1    7900 2950
	1    0    0    -1  
$EndComp
$Comp
L R R4
U 1 1 5AAEB830
P 9450 2950
F 0 "R4" V 9530 2950 50  0000 C CNN
F 1 "110" V 9450 2950 50  0000 C CNN
F 2 "" V 9380 2950 50  0000 C CNN
F 3 "" H 9450 2950 50  0000 C CNN
	1    9450 2950
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR03
U 1 1 5AAEF323
P 950 1000
F 0 "#PWR03" H 950 850 50  0001 C CNN
F 1 "VCC" H 950 1150 50  0000 C CNN
F 2 "" H 950 1000 50  0000 C CNN
F 3 "" H 950 1000 50  0000 C CNN
	1    950  1000
	0    -1   -1   0   
$EndComp
$Comp
L PWR_FLAG #FLG04
U 1 1 5AAEF527
P 1350 1000
F 0 "#FLG04" H 1350 1095 50  0001 C CNN
F 1 "PWR_FLAG" H 1350 1180 50  0000 C CNN
F 2 "" H 1350 1000 50  0000 C CNN
F 3 "" H 1350 1000 50  0000 C CNN
	1    1350 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	8200 6000 8350 6000
Wire Wire Line
	8200 5900 8350 5900
Wire Wire Line
	8200 5800 8350 5800
Wire Wire Line
	8200 5700 8350 5700
Wire Wire Line
	8200 5600 8350 5600
Wire Wire Line
	8200 5500 8350 5500
Wire Wire Line
	8200 5400 8350 5400
Wire Wire Line
	8250 3400 8150 3400
Wire Wire Line
	8250 3500 8150 3500
Wire Wire Line
	8250 3600 8150 3600
Wire Wire Line
	8250 3700 8150 3700
Wire Wire Line
	8250 3800 8150 3800
Wire Wire Line
	8250 3900 8150 3900
Wire Wire Line
	8250 4000 8150 4000
Wire Wire Line
	6700 3400 6600 3400
Wire Wire Line
	6700 3500 6600 3500
Wire Wire Line
	6700 3600 6600 3600
Wire Wire Line
	6700 3700 6600 3700
Wire Wire Line
	6700 3800 6600 3800
Wire Wire Line
	6700 3900 6600 3900
Wire Wire Line
	6700 4000 6600 4000
Wire Wire Line
	5150 3400 5050 3400
Wire Wire Line
	5150 3500 5050 3500
Wire Wire Line
	5150 3600 5050 3600
Wire Wire Line
	5150 3700 5050 3700
Wire Wire Line
	5150 3800 5050 3800
Wire Wire Line
	5150 3900 5050 3900
Wire Wire Line
	5150 4000 5050 4000
Wire Wire Line
	3600 3400 3500 3400
Wire Wire Line
	3600 3500 3500 3500
Wire Wire Line
	3600 3600 3500 3600
Wire Wire Line
	3600 3700 3500 3700
Wire Wire Line
	3600 3800 3500 3800
Wire Wire Line
	3600 3900 3500 3900
Wire Wire Line
	3600 4000 3500 4000
Wire Wire Line
	4800 2800 4800 2600
Wire Wire Line
	4800 2600 5200 2600
Wire Wire Line
	6350 2800 6350 1500
Wire Wire Line
	6350 1500 4800 1500
Wire Wire Line
	7900 2800 7900 1400
Wire Wire Line
	7900 1400 4800 1400
Wire Wire Line
	9450 2800 9450 1300
Wire Wire Line
	9450 1300 4800 1300
Wire Wire Line
	5200 2600 5200 1600
Wire Wire Line
	5200 1600 4800 1600
Wire Wire Line
	4800 1700 4900 1700
Wire Wire Line
	4800 1800 4900 1800
Wire Wire Line
	4800 1900 4900 1900
Wire Wire Line
	4800 2000 4900 2000
Wire Wire Line
	4800 4050 4950 4050
Wire Wire Line
	6350 4050 6500 4050
Wire Wire Line
	7900 4050 8050 4050
Wire Wire Line
	9450 4050 9600 4050
Wire Wire Line
	6150 5400 6800 5400
Wire Wire Line
	6150 5500 6800 5500
Wire Wire Line
	6150 5600 6800 5600
Wire Wire Line
	6150 5700 6800 5700
Wire Wire Line
	950  1000 950  1600
Wire Wire Line
	1350 1100 1350 1000
Wire Wire Line
	3400 1600 3200 1600
Wire Wire Line
	4750 5800 4600 5800
Wire Wire Line
	3300 2600 3350 2600
Wire Wire Line
	3350 2600 3350 2150
Wire Wire Line
	3350 2150 2850 2150
Wire Wire Line
	2850 2150 2850 1300
Wire Wire Line
	2850 1300 3400 1300
Wire Wire Line
	3300 2700 3400 2700
Wire Wire Line
	3400 2700 3400 2100
Wire Wire Line
	2900 2100 3400 2100
Wire Wire Line
	2900 1500 2900 2100
Wire Wire Line
	2900 1500 3400 1500
Wire Wire Line
	3400 1800 2900 1800
Connection ~ 2900 1800
Wire Wire Line
	3400 1900 3400 2000
Wire Wire Line
	3400 2000 3600 2000
Wire Wire Line
	3600 2000 3600 3000
Wire Wire Line
	3600 3000 3300 3000
Wire Wire Line
	4750 5900 4600 5900
Wire Wire Line
	700  1100 950  1100
Connection ~ 950  1100
Wire Wire Line
	700  1400 950  1400
Connection ~ 950  1400
Wire Wire Line
	4800 3100 4800 3350
Wire Wire Line
	6350 3100 6350 3350
Wire Wire Line
	7900 3100 7900 3350
Wire Wire Line
	9450 3100 9450 3350
Wire Wire Line
	6150 5900 6350 5900
Text Label 3800 900  0    60   ~ 0
GND
$Comp
L C C2
U 1 1 5AAF0E21
P 600 2750
F 0 "C2" H 625 2850 50  0000 L CNN
F 1 "0.1uF" H 625 2650 50  0000 L CNN
F 2 "" H 638 2600 50  0000 C CNN
F 3 "" H 600 2750 50  0000 C CNN
	1    600  2750
	1    0    0    -1  
$EndComp
Connection ~ 600  2500
Connection ~ 600  3000
Wire Wire Line
	600  2100 600  2600
Wire Wire Line
	600  2900 600  3350
$Comp
L C C3
U 1 1 5AAF14C6
P 3800 1050
F 0 "C3" H 3825 1150 50  0000 L CNN
F 1 "0.1uF" H 3825 950 50  0000 L CNN
F 2 "" H 3838 900 50  0000 C CNN
F 3 "" H 3800 1050 50  0000 C CNN
	1    3800 1050
	1    0    0    -1  
$EndComp
Text Label 5150 4950 0    60   ~ 0
GND
$Comp
L C C4
U 1 1 5AAF22BF
P 5150 5100
F 0 "C4" H 5175 5200 50  0000 L CNN
F 1 "0.1uF" H 5175 5000 50  0000 L CNN
F 2 "" H 5188 4950 50  0000 C CNN
F 3 "" H 5150 5100 50  0000 C CNN
	1    5150 5100
	1    0    0    -1  
$EndComp
$Comp
L C C5
U 1 1 5AAF2315
P 7200 5200
F 0 "C5" H 7225 5300 50  0000 L CNN
F 1 "0.1uF" H 7225 5100 50  0000 L CNN
F 2 "" H 7238 5050 50  0000 C CNN
F 3 "" H 7200 5200 50  0000 C CNN
	1    7200 5200
	1    0    0    -1  
$EndComp
Text Label 7200 5050 0    60   ~ 0
GND
Text Label 600  2200 0    60   ~ 0
GND
Text Label 600  3350 0    60   ~ 0
VCC
Wire Wire Line
	3300 2800 3550 2800
Wire Wire Line
	3300 2900 3550 2900
NoConn ~ 3550 2800
NoConn ~ 3550 2900
Text Label 3250 2800 0    60   ~ 0
INCLK
Text Label 3250 2900 0    60   ~ 0
INSER
$EndSCHEMATC