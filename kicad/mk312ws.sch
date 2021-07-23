EESchema Schematic File Version 4
EELAYER 30 0
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
L Connector:Conn_01x06_Male J1
U 1 1 60F96B84
P 3550 3150
F 0 "J1" H 3658 3531 50  0000 C CNN
F 1 "Conn_01x06_Male" V 3450 3150 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Horizontal" H 3550 3150 50  0001 C CNN
F 3 "~" H 3550 3150 50  0001 C CNN
	1    3550 3150
	1    0    0    -1  
$EndComp
$Comp
L ESP32_mini:mini_esp32 U1
U 1 1 60F96E24
P 5450 2200
F 0 "U1" H 5475 2325 50  0000 C CNN
F 1 "mini_esp32" H 5475 2234 50  0000 C CNN
F 2 "ESP32_mini:ESP32_mini" H 5600 2300 50  0001 C CNN
F 3 "" H 5600 2300 50  0001 C CNN
	1    5450 2200
	1    0    0    -1  
$EndComp
Text GLabel 3750 2950 2    50   Input ~ 0
EN_Vout
Text GLabel 3750 3050 2    50   Input ~ 0
5V
Text GLabel 3750 3150 2    50   Input ~ 0
GND
Text GLabel 3750 3250 2    50   Input ~ 0
TXD_Vout
Text GLabel 3750 3350 2    50   Input ~ 0
RXD_Vout
Text GLabel 3750 3450 2    50   Input ~ 0
STATE_Vout
Text GLabel 6000 2300 2    50   Input ~ 0
5V
Text GLabel 6000 2500 2    50   Input ~ 0
GND
Text GLabel 6000 2600 2    50   Input ~ 0
GND
Text GLabel 6000 2700 2    50   Input ~ 0
GND
Text GLabel 4950 3200 0    50   Input ~ 0
RXD_3V3
Text GLabel 4950 3300 0    50   Input ~ 0
TXD_3V3
Text GLabel 4950 2400 0    50   Input ~ 0
STATE_3V3
Text GLabel 6000 2400 2    50   Input ~ 0
3V3
$Comp
L Transistor_FET:BSS138 Q1
U 1 1 60F9D745
P 8400 1300
F 0 "Q1" V 8649 1300 50  0000 C CNN
F 1 "BSS138" V 8740 1300 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 8600 1225 50  0001 L CIN
F 3 "https://www.onsemi.com/pub/Collateral/BSS138-D.PDF" H 8400 1300 50  0001 L CNN
	1    8400 1300
	0    1    1    0   
$EndComp
$Comp
L Device:R R1
U 1 1 60F9FDF5
P 8050 1250
F 0 "R1" H 7981 1204 50  0000 R CNN
F 1 "10K" H 7981 1295 50  0000 R CNN
F 2 "Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder" V 7980 1250 50  0001 C CNN
F 3 "~" H 8050 1250 50  0001 C CNN
	1    8050 1250
	1    0    0    1   
$EndComp
Text GLabel 8400 1100 1    50   Input ~ 0
3V3
$Comp
L Device:R R5
U 1 1 60FA212D
P 8750 1250
F 0 "R5" H 8820 1296 50  0000 L CNN
F 1 "10K" H 8820 1205 50  0000 L CNN
F 2 "Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder" V 8680 1250 50  0001 C CNN
F 3 "~" H 8750 1250 50  0001 C CNN
	1    8750 1250
	1    0    0    -1  
$EndComp
Connection ~ 8750 1400
Wire Wire Line
	8750 1400 9200 1400
Wire Wire Line
	8600 1400 8750 1400
Wire Wire Line
	8050 1400 8200 1400
Wire Wire Line
	8050 1400 7800 1400
Connection ~ 8050 1400
Text GLabel 8050 1100 1    50   Input ~ 0
3V3
Text GLabel 8750 1100 1    50   Input ~ 0
5V
$Comp
L Transistor_FET:BSS138 Q2
U 1 1 60FAFBF5
P 8400 2200
F 0 "Q2" V 8649 2200 50  0000 C CNN
F 1 "BSS138" V 8740 2200 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 8600 2125 50  0001 L CIN
F 3 "https://www.onsemi.com/pub/Collateral/BSS138-D.PDF" H 8400 2200 50  0001 L CNN
	1    8400 2200
	0    1    1    0   
$EndComp
$Comp
L Device:R R2
U 1 1 60FAFDF5
P 8050 2150
F 0 "R2" H 7981 2104 50  0000 R CNN
F 1 "10K" H 7981 2195 50  0000 R CNN
F 2 "Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder" V 7980 2150 50  0001 C CNN
F 3 "~" H 8050 2150 50  0001 C CNN
	1    8050 2150
	1    0    0    1   
$EndComp
Text GLabel 8400 2000 1    50   Input ~ 0
3V3
$Comp
L Device:R R6
U 1 1 60FAFE00
P 8750 2150
F 0 "R6" H 8820 2196 50  0000 L CNN
F 1 "10K" H 8820 2105 50  0000 L CNN
F 2 "Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder" V 8680 2150 50  0001 C CNN
F 3 "~" H 8750 2150 50  0001 C CNN
	1    8750 2150
	1    0    0    -1  
$EndComp
Connection ~ 8750 2300
Wire Wire Line
	8750 2300 9200 2300
Wire Wire Line
	8600 2300 8750 2300
Wire Wire Line
	8050 2300 8200 2300
Wire Wire Line
	8050 2300 7800 2300
Connection ~ 8050 2300
Text GLabel 8050 2000 1    50   Input ~ 0
3V3
Text GLabel 8750 2000 1    50   Input ~ 0
5V
$Comp
L Transistor_FET:BSS138 Q3
U 1 1 60FB6CD6
P 8400 3100
F 0 "Q3" V 8649 3100 50  0000 C CNN
F 1 "BSS138" V 8740 3100 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 8600 3025 50  0001 L CIN
F 3 "https://www.onsemi.com/pub/Collateral/BSS138-D.PDF" H 8400 3100 50  0001 L CNN
	1    8400 3100
	0    1    1    0   
$EndComp
$Comp
L Device:R R3
U 1 1 60FB6F9E
P 8050 3050
F 0 "R3" H 7981 3004 50  0000 R CNN
F 1 "10K" H 7981 3095 50  0000 R CNN
F 2 "Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder" V 7980 3050 50  0001 C CNN
F 3 "~" H 8050 3050 50  0001 C CNN
	1    8050 3050
	1    0    0    1   
$EndComp
Text GLabel 8400 2900 1    50   Input ~ 0
3V3
$Comp
L Device:R R7
U 1 1 60FB6FA9
P 8750 3050
F 0 "R7" H 8820 3096 50  0000 L CNN
F 1 "10K" H 8820 3005 50  0000 L CNN
F 2 "Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder" V 8680 3050 50  0001 C CNN
F 3 "~" H 8750 3050 50  0001 C CNN
	1    8750 3050
	1    0    0    -1  
$EndComp
Connection ~ 8750 3200
Wire Wire Line
	8750 3200 9200 3200
Wire Wire Line
	8600 3200 8750 3200
Wire Wire Line
	8050 3200 8200 3200
Wire Wire Line
	8050 3200 7800 3200
Connection ~ 8050 3200
Text GLabel 8050 2900 1    50   Input ~ 0
3V3
Text GLabel 8750 2900 1    50   Input ~ 0
5V
$Comp
L Transistor_FET:BSS138 Q4
U 1 1 60FBC359
P 8400 4000
F 0 "Q4" V 8649 4000 50  0000 C CNN
F 1 "BSS138" V 8740 4000 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 8600 3925 50  0001 L CIN
F 3 "https://www.onsemi.com/pub/Collateral/BSS138-D.PDF" H 8400 4000 50  0001 L CNN
	1    8400 4000
	0    1    1    0   
$EndComp
$Comp
L Device:R R4
U 1 1 60FBC6E9
P 8050 3950
F 0 "R4" H 7981 3904 50  0000 R CNN
F 1 "10K" H 7981 3995 50  0000 R CNN
F 2 "Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder" V 7980 3950 50  0001 C CNN
F 3 "~" H 8050 3950 50  0001 C CNN
	1    8050 3950
	1    0    0    1   
$EndComp
Text GLabel 8400 3800 1    50   Input ~ 0
3V3
$Comp
L Device:R R8
U 1 1 60FBC6F4
P 8750 3950
F 0 "R8" H 8820 3996 50  0000 L CNN
F 1 "10K" H 8820 3905 50  0000 L CNN
F 2 "Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder" V 8680 3950 50  0001 C CNN
F 3 "~" H 8750 3950 50  0001 C CNN
	1    8750 3950
	1    0    0    -1  
$EndComp
Connection ~ 8750 4100
Wire Wire Line
	8750 4100 9200 4100
Wire Wire Line
	8600 4100 8750 4100
Wire Wire Line
	8050 4100 8200 4100
Wire Wire Line
	8050 4100 7800 4100
Connection ~ 8050 4100
Text GLabel 8050 3800 1    50   Input ~ 0
3V3
Text GLabel 8750 3800 1    50   Input ~ 0
5V
Text GLabel 7800 1400 0    50   Input ~ 0
STATE_3V3
Text GLabel 7800 2300 0    50   Input ~ 0
EN_3V3
Text GLabel 7800 3200 0    50   Input ~ 0
RXD_3V3
Text GLabel 7800 4100 0    50   Input ~ 0
TXD_3V3
Text GLabel 9200 1400 2    50   Input ~ 0
STATE_Vout
Text GLabel 9200 2300 2    50   Input ~ 0
EN_Vout
Text GLabel 9200 3200 2    50   Input ~ 0
RXD_Vout
Text GLabel 9200 4100 2    50   Input ~ 0
TXD_Vout
Text GLabel 4950 2600 0    50   Input ~ 0
EN_3V3
$EndSCHEMATC
