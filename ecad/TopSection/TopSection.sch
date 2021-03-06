EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 1 21
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 1950 900  1550 600 
U 604DFF37
F0 "PEL12T_1" 50
F1 "../common/PEL12T_w_Driver.sch" 50
F2 "DI" I L 1950 1400 50 
F3 "DO" O R 3500 1400 50 
F4 "QA" O L 1950 1100 50 
F5 "QB" O L 1950 1200 50 
F6 "SW_COL" I R 3500 1000 50 
F7 "SW_ROW" O L 1950 1000 50 
$EndSheet
$Sheet
S 4300 900  1600 600 
U 604E23DB
F0 "PEL12T_2" 50
F1 "../common/PEL12T_w_Driver.sch" 50
F2 "DI" I L 4300 1400 50 
F3 "DO" O R 5900 1400 50 
F4 "QA" O L 4300 1100 50 
F5 "QB" O L 4300 1200 50 
F6 "SW_COL" I R 5900 1000 50 
F7 "SW_ROW" O L 4300 1000 50 
$EndSheet
$Sheet
S 9050 900  1550 600 
U 604E26D3
F0 "PEL12T_4" 50
F1 "../common/PEL12T_w_Driver.sch" 50
F2 "DI" I L 9050 1400 50 
F3 "DO" O R 10600 1400 50 
F4 "QA" O L 9050 1100 50 
F5 "QB" O L 9050 1200 50 
F6 "SW_COL" I R 10600 1000 50 
F7 "SW_ROW" O L 9050 1000 50 
$EndSheet
$Sheet
S 11400 900  1550 600 
U 604E277D
F0 "PEL12T_5" 50
F1 "../common/PEL12T_w_Driver.sch" 50
F2 "DI" I L 11400 1400 50 
F3 "DO" O R 12950 1400 50 
F4 "QA" O L 11400 1100 50 
F5 "QB" O L 11400 1200 50 
F6 "SW_COL" I R 12950 1000 50 
F7 "SW_ROW" O L 11400 1000 50 
$EndSheet
$Sheet
S 1950 2150 1550 400 
U 604E279F
F0 "MX_1" 50
F1 "../common/MX_LED_Underneath.sch" 50
F2 "SW_COL" I R 3500 2250 50 
F3 "SW_ROW" O L 1950 2250 50 
F4 "DI" I L 1950 2450 50 
F5 "DO" O R 3500 2450 50 
$EndSheet
$Sheet
S 4300 2150 1600 400 
U 604E30DA
F0 "MX_2" 50
F1 "../common/MX_LED_Underneath.sch" 50
F2 "SW_COL" I R 5900 2250 50 
F3 "SW_ROW" O L 4300 2250 50 
F4 "DI" I L 4300 2450 50 
F5 "DO" O R 5900 2450 50 
$EndSheet
$Sheet
S 6700 2150 1550 400 
U 604E3276
F0 "MX_3" 50
F1 "../common/MX_LED_Underneath.sch" 50
F2 "SW_COL" I R 8250 2250 50 
F3 "SW_ROW" O L 6700 2250 50 
F4 "DI" I L 6700 2450 50 
F5 "DO" O R 8250 2450 50 
$EndSheet
$Sheet
S 9050 2150 1550 400 
U 604E3363
F0 "MX_4" 50
F1 "../common/MX_LED_Underneath.sch" 50
F2 "SW_COL" I R 10600 2250 50 
F3 "SW_ROW" O L 9050 2250 50 
F4 "DI" I L 9050 2450 50 
F5 "DO" O R 10600 2450 50 
$EndSheet
$Sheet
S 11400 2150 1550 400 
U 604E34B4
F0 "MX_5" 50
F1 "../common/MX_LED_Underneath.sch" 50
F2 "SW_COL" I R 12950 2250 50 
F3 "SW_ROW" O L 11400 2250 50 
F4 "DI" I L 11400 2450 50 
F5 "DO" O R 12950 2450 50 
$EndSheet
Wire Wire Line
	1750 1200 1750 1900
Wire Wire Line
	1650 1100 1650 1800
Wire Wire Line
	1550 1000 1550 1700
Wire Wire Line
	6500 1200 6500 1900
Wire Wire Line
	6400 1100 6400 1800
Wire Wire Line
	6300 1000 6300 1700
Wire Wire Line
	11200 1200 11200 1900
Wire Wire Line
	11100 1100 11100 1800
Wire Wire Line
	11000 1000 11000 1700
Wire Wire Line
	8650 1000 8650 1700
Wire Wire Line
	1750 2250 1750 2750
Connection ~ 1550 1700
Wire Wire Line
	1550 1700 1400 1700
Connection ~ 1650 1800
Wire Wire Line
	1650 1800 1400 1800
Connection ~ 1750 1900
Wire Wire Line
	1750 1900 1400 1900
Wire Wire Line
	11200 2250 11200 2750
Connection ~ 1750 2750
Wire Wire Line
	1750 2750 1400 2750
Wire Wire Line
	4100 2250 4100 2750
Wire Wire Line
	8850 2250 8850 2750
Text Label 1400 1700 2    50   ~ 0
R1
Text Label 1400 2750 2    50   ~ 0
R2
Text Label 6200 700  0    50   ~ 0
C2
Text Label 8550 700  0    50   ~ 0
C3
Text Label 10900 700  0    50   ~ 0
C4
Text Label 13200 700  0    50   ~ 0
C5
Text Label 1400 1800 2    50   ~ 0
QA1
Text Label 1400 1900 2    50   ~ 0
QB1
NoConn ~ 5350 7000
NoConn ~ 5350 9100
$Comp
L power:GND #PWR0101
U 1 1 60683B4B
P 5700 6300
F 0 "#PWR0101" H 5700 6050 50  0001 C CNN
F 1 "GND" V 5705 6172 50  0000 R CNN
F 2 "" H 5700 6300 50  0001 C CNN
F 3 "" H 5700 6300 50  0001 C CNN
	1    5700 6300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5350 6300 5700 6300
$Comp
L power:GND #PWR0102
U 1 1 6068F64E
P 5700 8400
F 0 "#PWR0102" H 5700 8150 50  0001 C CNN
F 1 "GND" V 5705 8272 50  0000 R CNN
F 2 "" H 5700 8400 50  0001 C CNN
F 3 "" H 5700 8400 50  0001 C CNN
	1    5700 8400
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5350 8400 5700 8400
$Comp
L Device:C C?
U 1 1 606A76A5
P 3150 8800
AR Path="/604B4498/606A76A5" Ref="C?"  Part="1" 
AR Path="/6057F9D3/606A76A5" Ref="C?"  Part="1" 
AR Path="/6057FBE3/606A76A5" Ref="C?"  Part="1" 
AR Path="/6057FCA8/606A76A5" Ref="C?"  Part="1" 
AR Path="/6059A8EA/606A76A5" Ref="C?"  Part="1" 
AR Path="/6059A8F8/606A76A5" Ref="C?"  Part="1" 
AR Path="/6059A906/606A76A5" Ref="C?"  Part="1" 
AR Path="/6059A90C/606A76A5" Ref="C?"  Part="1" 
AR Path="/605A1BCD/606A76A5" Ref="C?"  Part="1" 
AR Path="/605A1BDB/606A76A5" Ref="C?"  Part="1" 
AR Path="/605A1BE9/606A76A5" Ref="C?"  Part="1" 
AR Path="/605A1BEF/606A76A5" Ref="C?"  Part="1" 
AR Path="/605A28C5/606A76A5" Ref="C?"  Part="1" 
AR Path="/605A28D3/606A76A5" Ref="C?"  Part="1" 
AR Path="/605A28E1/606A76A5" Ref="C?"  Part="1" 
AR Path="/605A28E7/606A76A5" Ref="C?"  Part="1" 
AR Path="/605B6306/606A76A5" Ref="C?"  Part="1" 
AR Path="/605B6314/606A76A5" Ref="C?"  Part="1" 
AR Path="/605B6322/606A76A5" Ref="C?"  Part="1" 
AR Path="/605B6328/606A76A5" Ref="C?"  Part="1" 
AR Path="/605E1DC3/606A76A5" Ref="C?"  Part="1" 
AR Path="/605E1DD1/606A76A5" Ref="C?"  Part="1" 
AR Path="/605E1DDF/606A76A5" Ref="C?"  Part="1" 
AR Path="/605E1DE5/606A76A5" Ref="C?"  Part="1" 
AR Path="/605E98A7/606A76A5" Ref="C?"  Part="1" 
AR Path="/605E98B5/606A76A5" Ref="C?"  Part="1" 
AR Path="/605E98C3/606A76A5" Ref="C?"  Part="1" 
AR Path="/605E98C9/606A76A5" Ref="C?"  Part="1" 
AR Path="/605F434F/606A76A5" Ref="C?"  Part="1" 
AR Path="/605F435D/606A76A5" Ref="C?"  Part="1" 
AR Path="/605F436B/606A76A5" Ref="C?"  Part="1" 
AR Path="/605F4371/606A76A5" Ref="C?"  Part="1" 
AR Path="/60900772/60906151/606A76A5" Ref="C?"  Part="1" 
AR Path="/60900772/6090616D/606A76A5" Ref="C?"  Part="1" 
AR Path="/60900772/60906186/606A76A5" Ref="C?"  Part="1" 
AR Path="/60900772/6090619C/606A76A5" Ref="C?"  Part="1" 
AR Path="/60900772/609061B2/606A76A5" Ref="C?"  Part="1" 
AR Path="/60900772/609061C8/606A76A5" Ref="C?"  Part="1" 
AR Path="/60900772/609061CE/606A76A5" Ref="C?"  Part="1" 
AR Path="/60900772/609061D4/606A76A5" Ref="C?"  Part="1" 
AR Path="/60900772/609061DA/606A76A5" Ref="C?"  Part="1" 
AR Path="/60900772/609061E3/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3CF36/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3D212/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3D2C3/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3D329/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3D38F/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3E33D/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3E39E/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3E431/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3E46F/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3E4B2/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C6B/60906151/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C6B/6090616D/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C6B/60906186/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C6B/6090619C/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061B2/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061C8/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061CE/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061D4/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061DA/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061E3/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3CF36/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3D212/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3D2C3/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3D329/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3D38F/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3E33D/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3E39E/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3E431/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3E46F/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3E4B2/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67560/60906151/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67560/6090616D/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67560/60906186/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67560/6090619C/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67560/609061B2/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67560/609061C8/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67560/609061CE/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67560/609061D4/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67560/609061DA/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67560/609061E3/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3CF36/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3D212/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3D2C3/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3D329/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3D38F/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3E33D/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3E39E/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3E431/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3E46F/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3E4B2/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B69076/60906151/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B69076/6090616D/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B69076/60906186/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B69076/6090619C/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B69076/609061B2/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B69076/609061C8/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B69076/609061CE/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B69076/609061D4/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B69076/609061DA/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B69076/609061E3/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3CF36/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3D212/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3D2C3/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3D329/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3D38F/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3E33D/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3E39E/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3E431/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3E46F/606A76A5" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3E4B2/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E279F/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E2E0B/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E2EAD/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E30DA/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E3276/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E3363/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E34B4/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E5003/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E511D/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E52D2/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E5455/606A76A5" Ref="C?"  Part="1" 
AR Path="/604E5501/606A76A5" Ref="C?"  Part="1" 
AR Path="/606A76A5" Ref="C2"  Part="1" 
F 0 "C2" H 3265 8846 50  0000 L CNN
F 1 "100nF" H 3265 8755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 3188 8650 50  0001 C CNN
F 3 "~" H 3150 8800 50  0001 C CNN
	1    3150 8800
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0103
U 1 1 606B1220
P 4850 5950
F 0 "#PWR0103" H 4850 5800 50  0001 C CNN
F 1 "+3.3V" H 4865 6123 50  0000 C CNN
F 2 "" H 4850 5950 50  0001 C CNN
F 3 "" H 4850 5950 50  0001 C CNN
	1    4850 5950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 606BB3D1
P 3150 9050
F 0 "#PWR0104" H 3150 8800 50  0001 C CNN
F 1 "GND" H 3155 8877 50  0000 C CNN
F 2 "" H 3150 9050 50  0001 C CNN
F 3 "" H 3150 9050 50  0001 C CNN
	1    3150 9050
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 5950 4850 6100
$Comp
L power:+3.3V #PWR0105
U 1 1 6073F6B2
P 3150 8550
F 0 "#PWR0105" H 3150 8400 50  0001 C CNN
F 1 "+3.3V" H 3165 8723 50  0000 C CNN
F 2 "" H 3150 8550 50  0001 C CNN
F 3 "" H 3150 8550 50  0001 C CNN
	1    3150 8550
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 8550 3150 8650
Wire Wire Line
	3150 8950 3150 9050
$Comp
L Device:C C?
U 1 1 60776422
P 3100 6650
AR Path="/604B4498/60776422" Ref="C?"  Part="1" 
AR Path="/6057F9D3/60776422" Ref="C?"  Part="1" 
AR Path="/6057FBE3/60776422" Ref="C?"  Part="1" 
AR Path="/6057FCA8/60776422" Ref="C?"  Part="1" 
AR Path="/6059A8EA/60776422" Ref="C?"  Part="1" 
AR Path="/6059A8F8/60776422" Ref="C?"  Part="1" 
AR Path="/6059A906/60776422" Ref="C?"  Part="1" 
AR Path="/6059A90C/60776422" Ref="C?"  Part="1" 
AR Path="/605A1BCD/60776422" Ref="C?"  Part="1" 
AR Path="/605A1BDB/60776422" Ref="C?"  Part="1" 
AR Path="/605A1BE9/60776422" Ref="C?"  Part="1" 
AR Path="/605A1BEF/60776422" Ref="C?"  Part="1" 
AR Path="/605A28C5/60776422" Ref="C?"  Part="1" 
AR Path="/605A28D3/60776422" Ref="C?"  Part="1" 
AR Path="/605A28E1/60776422" Ref="C?"  Part="1" 
AR Path="/605A28E7/60776422" Ref="C?"  Part="1" 
AR Path="/605B6306/60776422" Ref="C?"  Part="1" 
AR Path="/605B6314/60776422" Ref="C?"  Part="1" 
AR Path="/605B6322/60776422" Ref="C?"  Part="1" 
AR Path="/605B6328/60776422" Ref="C?"  Part="1" 
AR Path="/605E1DC3/60776422" Ref="C?"  Part="1" 
AR Path="/605E1DD1/60776422" Ref="C?"  Part="1" 
AR Path="/605E1DDF/60776422" Ref="C?"  Part="1" 
AR Path="/605E1DE5/60776422" Ref="C?"  Part="1" 
AR Path="/605E98A7/60776422" Ref="C?"  Part="1" 
AR Path="/605E98B5/60776422" Ref="C?"  Part="1" 
AR Path="/605E98C3/60776422" Ref="C?"  Part="1" 
AR Path="/605E98C9/60776422" Ref="C?"  Part="1" 
AR Path="/605F434F/60776422" Ref="C?"  Part="1" 
AR Path="/605F435D/60776422" Ref="C?"  Part="1" 
AR Path="/605F436B/60776422" Ref="C?"  Part="1" 
AR Path="/605F4371/60776422" Ref="C?"  Part="1" 
AR Path="/60900772/60906151/60776422" Ref="C?"  Part="1" 
AR Path="/60900772/6090616D/60776422" Ref="C?"  Part="1" 
AR Path="/60900772/60906186/60776422" Ref="C?"  Part="1" 
AR Path="/60900772/6090619C/60776422" Ref="C?"  Part="1" 
AR Path="/60900772/609061B2/60776422" Ref="C?"  Part="1" 
AR Path="/60900772/609061C8/60776422" Ref="C?"  Part="1" 
AR Path="/60900772/609061CE/60776422" Ref="C?"  Part="1" 
AR Path="/60900772/609061D4/60776422" Ref="C?"  Part="1" 
AR Path="/60900772/609061DA/60776422" Ref="C?"  Part="1" 
AR Path="/60900772/609061E3/60776422" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3CF36/60776422" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3D212/60776422" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3D2C3/60776422" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3D329/60776422" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3D38F/60776422" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3E33D/60776422" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3E39E/60776422" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3E431/60776422" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3E46F/60776422" Ref="C?"  Part="1" 
AR Path="/60B3CBC1/60B3E4B2/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C6B/60906151/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C6B/6090616D/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C6B/60906186/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C6B/6090619C/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061B2/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061C8/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061CE/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061D4/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061DA/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C6B/609061E3/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3CF36/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3D212/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3D2C3/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3D329/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3D38F/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3E33D/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3E39E/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3E431/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3E46F/60776422" Ref="C?"  Part="1" 
AR Path="/60B62C78/60B3E4B2/60776422" Ref="C?"  Part="1" 
AR Path="/60B67560/60906151/60776422" Ref="C?"  Part="1" 
AR Path="/60B67560/6090616D/60776422" Ref="C?"  Part="1" 
AR Path="/60B67560/60906186/60776422" Ref="C?"  Part="1" 
AR Path="/60B67560/6090619C/60776422" Ref="C?"  Part="1" 
AR Path="/60B67560/609061B2/60776422" Ref="C?"  Part="1" 
AR Path="/60B67560/609061C8/60776422" Ref="C?"  Part="1" 
AR Path="/60B67560/609061CE/60776422" Ref="C?"  Part="1" 
AR Path="/60B67560/609061D4/60776422" Ref="C?"  Part="1" 
AR Path="/60B67560/609061DA/60776422" Ref="C?"  Part="1" 
AR Path="/60B67560/609061E3/60776422" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3CF36/60776422" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3D212/60776422" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3D2C3/60776422" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3D329/60776422" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3D38F/60776422" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3E33D/60776422" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3E39E/60776422" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3E431/60776422" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3E46F/60776422" Ref="C?"  Part="1" 
AR Path="/60B67B63/60B3E4B2/60776422" Ref="C?"  Part="1" 
AR Path="/60B69076/60906151/60776422" Ref="C?"  Part="1" 
AR Path="/60B69076/6090616D/60776422" Ref="C?"  Part="1" 
AR Path="/60B69076/60906186/60776422" Ref="C?"  Part="1" 
AR Path="/60B69076/6090619C/60776422" Ref="C?"  Part="1" 
AR Path="/60B69076/609061B2/60776422" Ref="C?"  Part="1" 
AR Path="/60B69076/609061C8/60776422" Ref="C?"  Part="1" 
AR Path="/60B69076/609061CE/60776422" Ref="C?"  Part="1" 
AR Path="/60B69076/609061D4/60776422" Ref="C?"  Part="1" 
AR Path="/60B69076/609061DA/60776422" Ref="C?"  Part="1" 
AR Path="/60B69076/609061E3/60776422" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3CF36/60776422" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3D212/60776422" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3D2C3/60776422" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3D329/60776422" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3D38F/60776422" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3E33D/60776422" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3E39E/60776422" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3E431/60776422" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3E46F/60776422" Ref="C?"  Part="1" 
AR Path="/60B694C9/60B3E4B2/60776422" Ref="C?"  Part="1" 
AR Path="/604E279F/60776422" Ref="C?"  Part="1" 
AR Path="/604E2E0B/60776422" Ref="C?"  Part="1" 
AR Path="/604E2EAD/60776422" Ref="C?"  Part="1" 
AR Path="/604E30DA/60776422" Ref="C?"  Part="1" 
AR Path="/604E3276/60776422" Ref="C?"  Part="1" 
AR Path="/604E3363/60776422" Ref="C?"  Part="1" 
AR Path="/604E34B4/60776422" Ref="C?"  Part="1" 
AR Path="/604E5003/60776422" Ref="C?"  Part="1" 
AR Path="/604E511D/60776422" Ref="C?"  Part="1" 
AR Path="/604E52D2/60776422" Ref="C?"  Part="1" 
AR Path="/604E5455/60776422" Ref="C?"  Part="1" 
AR Path="/604E5501/60776422" Ref="C?"  Part="1" 
AR Path="/60776422" Ref="C1"  Part="1" 
F 0 "C1" H 3215 6696 50  0000 L CNN
F 1 "100nF" H 3215 6605 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.08x0.95mm_HandSolder" H 3138 6500 50  0001 C CNN
F 3 "~" H 3100 6650 50  0001 C CNN
	1    3100 6650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 60776428
P 3100 6900
F 0 "#PWR0106" H 3100 6650 50  0001 C CNN
F 1 "GND" H 3105 6727 50  0000 C CNN
F 2 "" H 3100 6900 50  0001 C CNN
F 3 "" H 3100 6900 50  0001 C CNN
	1    3100 6900
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0107
U 1 1 6077642E
P 3100 6400
F 0 "#PWR0107" H 3100 6250 50  0001 C CNN
F 1 "+3.3V" H 3115 6573 50  0000 C CNN
F 2 "" H 3100 6400 50  0001 C CNN
F 3 "" H 3100 6400 50  0001 C CNN
	1    3100 6400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 6400 3100 6500
Wire Wire Line
	3100 6800 3100 6900
Wire Wire Line
	4350 6300 4000 6300
Wire Wire Line
	4350 6400 4000 6400
Wire Wire Line
	4350 6500 4000 6500
Wire Wire Line
	4350 6600 4000 6600
Wire Wire Line
	4350 6700 4000 6700
Wire Wire Line
	4350 6800 4000 6800
Wire Wire Line
	4350 6900 4000 6900
Wire Wire Line
	4350 7000 4000 7000
$Comp
L power:+3.3V #PWR0108
U 1 1 607FA2FF
P 4850 8050
F 0 "#PWR0108" H 4850 7900 50  0001 C CNN
F 1 "+3.3V" H 4865 8223 50  0000 C CNN
F 2 "" H 4850 8050 50  0001 C CNN
F 3 "" H 4850 8050 50  0001 C CNN
	1    4850 8050
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 8050 4850 8200
$Comp
L power:GND #PWR0109
U 1 1 60812038
P 4850 7450
F 0 "#PWR0109" H 4850 7200 50  0001 C CNN
F 1 "GND" H 4855 7277 50  0000 C CNN
F 2 "" H 4850 7450 50  0001 C CNN
F 3 "" H 4850 7450 50  0001 C CNN
	1    4850 7450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0110
U 1 1 6081EACD
P 4850 9600
F 0 "#PWR0110" H 4850 9350 50  0001 C CNN
F 1 "GND" H 4855 9427 50  0000 C CNN
F 2 "" H 4850 9600 50  0001 C CNN
F 3 "" H 4850 9600 50  0001 C CNN
	1    4850 9600
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 9600 4850 9400
Wire Wire Line
	4350 8500 4000 8500
Wire Wire Line
	4350 8600 4000 8600
Wire Wire Line
	4350 8700 4000 8700
Wire Wire Line
	4350 8800 4000 8800
Wire Wire Line
	4350 8900 4000 8900
NoConn ~ 4350 8400
NoConn ~ 4350 9000
NoConn ~ 4350 9100
Text Label 4000 6600 2    50   ~ 0
R1
Text Label 4000 6700 2    50   ~ 0
R2
Text Label 4000 7000 2    50   ~ 0
R3
Text Label 4000 6300 2    50   ~ 0
R4
Text Label 4000 6400 2    50   ~ 0
QA1
Text Label 4000 6800 2    50   ~ 0
QA2
Text Label 4000 6500 2    50   ~ 0
QB1
Text Label 4000 6900 2    50   ~ 0
QB2
$Comp
L power:GND #PWR0241
U 1 1 6090C5AE
P 9550 6900
F 0 "#PWR0241" H 9550 6650 50  0001 C CNN
F 1 "GND" V 9550 6650 50  0000 C CNN
F 2 "" H 9550 6900 50  0001 C CNN
F 3 "" H 9550 6900 50  0001 C CNN
	1    9550 6900
	0    1    1    0   
$EndComp
$Comp
L woodpecker:VLED #PWR0242
U 1 1 60926282
P 11250 6900
F 0 "#PWR0242" H 11050 6750 50  0001 C CNN
F 1 "VLED" V 11250 7150 50  0000 C CNN
F 2 "" H 11250 6900 50  0001 C CNN
F 3 "" H 11250 6900 50  0001 C CNN
	1    11250 6900
	0    1    1    0   
$EndComp
$Comp
L power:+3V3 #PWR0243
U 1 1 6097573D
P 11250 7000
F 0 "#PWR0243" H 11250 6850 50  0001 C CNN
F 1 "+3V3" V 11250 7250 50  0000 C CNN
F 2 "" H 11250 7000 50  0001 C CNN
F 3 "" H 11250 7000 50  0001 C CNN
	1    11250 7000
	0    1    1    0   
$EndComp
Wire Wire Line
	10700 6900 11250 6900
Wire Wire Line
	10700 7000 11250 7000
Wire Wire Line
	5350 6500 5700 6500
Wire Wire Line
	5350 6600 5700 6600
Wire Wire Line
	5350 6700 5700 6700
Wire Wire Line
	5350 6800 5700 6800
Wire Wire Line
	5350 8600 5700 8600
Wire Wire Line
	5350 8700 5700 8700
Wire Wire Line
	5350 8800 5700 8800
Wire Wire Line
	5350 8900 5700 8900
Text Label 5700 6500 0    50   ~ 0
~CS_ROW
Text Label 5700 6600 0    50   ~ 0
MISO
Text Label 5700 6800 0    50   ~ 0
MOSI
Text Label 5700 6700 0    50   ~ 0
SCK
Text Label 5700 8600 0    50   ~ 0
~CS_COL
Text Label 5700 8700 0    50   ~ 0
MISO
Text Label 5700 8900 0    50   ~ 0
MOSI
Text Label 5700 8800 0    50   ~ 0
SCK
Text Label 10900 7200 0    50   ~ 0
~CS_ROW
Text Label 10050 7100 2    50   ~ 0
MOSI
Text Label 10900 7100 0    50   ~ 0
~CS_COL
Wire Wire Line
	10200 6900 9550 6900
Wire Wire Line
	10050 7000 10200 7000
Wire Wire Line
	10200 7100 10050 7100
Wire Wire Line
	10200 7200 10050 7200
Wire Wire Line
	10700 7100 10900 7100
Wire Wire Line
	10700 7200 10900 7200
Wire Wire Line
	1750 3300 1950 3300
Wire Wire Line
	1650 3200 1950 3200
Wire Wire Line
	1550 3100 1950 3100
Wire Wire Line
	1850 3500 1850 4100
Wire Wire Line
	1950 3500 1850 3500
Text Notes 1950 5200 2    50   ~ 0
Key / Encoder Matrix
Wire Notes Line
	12900 5100 1550 5100
Wire Wire Line
	10600 3500 10700 3500
Text Label 1400 4000 2    50   ~ 0
QB2
Text Label 1400 3900 2    50   ~ 0
QA2
Text Label 1350 4850 2    50   ~ 0
R4
Text Label 1400 3800 2    50   ~ 0
R3
Wire Wire Line
	1700 4850 1350 4850
Connection ~ 1700 4850
Wire Wire Line
	1700 4350 1700 4850
Wire Wire Line
	8800 4350 8800 4850
Wire Wire Line
	11150 4350 11150 4850
Wire Wire Line
	1750 4000 1400 4000
Connection ~ 1750 4000
Wire Wire Line
	1650 3900 1400 3900
Connection ~ 1650 3900
Wire Wire Line
	1550 3800 1400 3800
Connection ~ 1550 3800
Wire Wire Line
	1550 3100 1550 3800
Wire Wire Line
	1650 3200 1650 3900
Wire Wire Line
	1750 3300 1750 4000
Wire Wire Line
	11000 3100 11000 3800
Wire Wire Line
	11100 3200 11100 3900
Wire Wire Line
	11200 3300 11200 4000
Wire Wire Line
	8650 3100 8650 3800
$Sheet
S 11350 4250 1550 400 
U 604E5501
F0 "MX_10" 50
F1 "../common/MX_LED_Underneath.sch" 50
F2 "SW_COL" I R 12900 4350 50 
F3 "SW_ROW" O L 11350 4350 50 
F4 "DI" I L 11350 4550 50 
F5 "DO" O R 12900 4550 50 
$EndSheet
$Sheet
S 6650 4250 1550 400 
U 604E52D2
F0 "MX_8" 50
F1 "../common/MX_LED_Underneath.sch" 50
F2 "SW_COL" I R 8200 4350 50 
F3 "SW_ROW" O L 6650 4350 50 
F4 "DI" I L 6650 4550 50 
F5 "DO" O R 8200 4550 50 
$EndSheet
$Sheet
S 4300 4250 1600 400 
U 604E511D
F0 "MX_7" 50
F1 "../common/MX_LED_Underneath.sch" 50
F2 "SW_COL" I R 5900 4350 50 
F3 "SW_ROW" O L 4300 4350 50 
F4 "DI" I L 4300 4550 50 
F5 "DO" O R 5900 4550 50 
$EndSheet
$Sheet
S 1900 4250 1550 400 
U 604E5003
F0 "MX_6" 50
F1 "../common/MX_LED_Underneath.sch" 50
F2 "SW_COL" I R 3450 4350 50 
F3 "SW_ROW" O L 1900 4350 50 
F4 "DI" I L 1900 4550 50 
F5 "DO" O R 3450 4550 50 
$EndSheet
$Sheet
S 11400 3000 1550 600 
U 604E4BEC
F0 "PEL12T_10" 50
F1 "../common/PEL12T_w_Driver.sch" 50
F2 "DI" I L 11400 3500 50 
F3 "DO" O R 12950 3500 50 
F4 "QA" O L 11400 3200 50 
F5 "QB" O L 11400 3300 50 
F6 "SW_COL" I R 12950 3100 50 
F7 "SW_ROW" O L 11400 3100 50 
$EndSheet
$Sheet
S 9050 3000 1550 600 
U 604E4878
F0 "PEL12T_9" 50
F1 "../common/PEL12T_w_Driver.sch" 50
F2 "DI" I L 9050 3500 50 
F3 "DO" O R 10600 3500 50 
F4 "QA" O L 9050 3200 50 
F5 "QB" O L 9050 3300 50 
F6 "SW_COL" I R 10600 3100 50 
F7 "SW_ROW" O L 9050 3100 50 
$EndSheet
$Sheet
S 6700 3000 1550 600 
U 604E45AC
F0 "PEL12T_8" 50
F1 "../common/PEL12T_w_Driver.sch" 50
F2 "DI" I L 6700 3500 50 
F3 "DO" O R 8250 3500 50 
F4 "QA" O L 6700 3200 50 
F5 "QB" O L 6700 3300 50 
F6 "SW_COL" I R 8250 3100 50 
F7 "SW_ROW" O L 6700 3100 50 
$EndSheet
$Sheet
S 4300 3000 1600 600 
U 604E4406
F0 "PEL12T_7" 50
F1 "../common/PEL12T_w_Driver.sch" 50
F2 "DI" I L 4300 3500 50 
F3 "DO" O R 5900 3500 50 
F4 "QA" O L 4300 3200 50 
F5 "QB" O L 4300 3300 50 
F6 "SW_COL" I R 5900 3100 50 
F7 "SW_ROW" O L 4300 3100 50 
$EndSheet
$Sheet
S 1950 3000 1550 600 
U 604E40D1
F0 "PEL12T_6" 50
F1 "../common/PEL12T_w_Driver.sch" 50
F2 "DI" I L 1950 3500 50 
F3 "DO" O R 3500 3500 50 
F4 "QA" O L 1950 3200 50 
F5 "QB" O L 1950 3300 50 
F6 "SW_COL" I R 3500 3100 50 
F7 "SW_ROW" O L 1950 3100 50 
$EndSheet
Wire Wire Line
	1850 2450 1850 2850
Wire Wire Line
	1850 2450 1950 2450
Wire Wire Line
	1750 2250 1950 2250
Wire Wire Line
	1850 1400 1850 2000
Wire Wire Line
	1850 1400 1950 1400
Wire Wire Line
	1550 1000 1950 1000
Wire Wire Line
	1650 1100 1950 1100
Wire Wire Line
	1750 1200 1950 1200
Text Label 3800 700  0    50   ~ 0
C1
Wire Wire Line
	3550 4550 3550 4100
Wire Wire Line
	3550 4100 1850 4100
Wire Wire Line
	3450 4550 3550 4550
Wire Wire Line
	3900 3100 3900 3800
Wire Wire Line
	1550 3800 3900 3800
Wire Wire Line
	1650 3900 4000 3900
Wire Wire Line
	1750 4000 4100 4000
Wire Wire Line
	1550 1700 3900 1700
Wire Wire Line
	1650 1800 4000 1800
Wire Wire Line
	1750 1900 4100 1900
Wire Wire Line
	3900 1700 3900 1000
Wire Wire Line
	1750 2750 4100 2750
Wire Wire Line
	4000 3200 4000 3900
Wire Wire Line
	4100 3300 4100 4000
Wire Wire Line
	4100 1200 4100 1900
Wire Wire Line
	4000 1100 4000 1800
Wire Wire Line
	4300 3500 4200 3500
Wire Wire Line
	4200 4100 6000 4100
Wire Wire Line
	6700 3500 6600 3500
Wire Wire Line
	6600 3500 6600 4100
Wire Wire Line
	6600 4100 8300 4100
Wire Wire Line
	8300 4550 8200 4550
Wire Wire Line
	8300 4100 8300 4550
Wire Wire Line
	8250 3500 8350 3500
Wire Wire Line
	8350 3500 8350 2850
Wire Wire Line
	8350 2850 6600 2850
Wire Wire Line
	5900 3500 6000 3500
Wire Wire Line
	4200 2450 4300 2450
Wire Wire Line
	5900 2450 6000 2450
Wire Wire Line
	6000 2000 4200 2000
Wire Wire Line
	6600 2850 6600 2450
Wire Wire Line
	6600 2450 6700 2450
Wire Wire Line
	8250 2450 8350 2450
Wire Wire Line
	8350 2000 6600 2000
Wire Wire Line
	6600 2000 6600 1400
Wire Wire Line
	6600 1400 6700 1400
Wire Wire Line
	6300 1000 6700 1000
Wire Wire Line
	6400 1100 6700 1100
$Sheet
S 6700 900  1550 600 
U 604E2542
F0 "PEL12T_3" 50
F1 "../common/PEL12T_w_Driver.sch" 50
F2 "DI" I L 6700 1400 50 
F3 "DO" O R 8250 1400 50 
F4 "QA" O L 6700 1100 50 
F5 "QB" O L 6700 1200 50 
F6 "SW_COL" I R 8250 1000 50 
F7 "SW_ROW" O L 6700 1000 50 
$EndSheet
Wire Wire Line
	6500 1200 6700 1200
Wire Wire Line
	8350 2000 8350 2450
$Sheet
S 9000 4250 1550 400 
U 604E5455
F0 "MX_9" 50
F1 "../common/MX_LED_Underneath.sch" 50
F2 "SW_COL" I R 10550 4350 50 
F3 "SW_ROW" O L 9000 4350 50 
F4 "DI" I L 9000 4550 50 
F5 "DO" O R 10550 4550 50 
$EndSheet
Wire Wire Line
	8750 3200 8750 3900
Wire Wire Line
	8850 3300 8850 4000
Wire Wire Line
	8950 3500 9050 3500
Wire Wire Line
	8650 3100 9050 3100
Wire Wire Line
	8750 3200 9050 3200
Wire Wire Line
	8850 3300 9050 3300
Wire Wire Line
	8850 2250 9050 2250
Wire Wire Line
	9050 2450 8950 2450
Wire Wire Line
	8950 2850 10700 2850
Wire Wire Line
	10600 2450 10700 2450
Wire Wire Line
	10700 2000 8950 2000
Wire Wire Line
	8750 1100 8750 1800
Wire Wire Line
	8850 1200 8850 1900
Wire Wire Line
	8650 1000 9050 1000
Wire Wire Line
	8750 1100 9050 1100
Wire Wire Line
	8850 1200 9050 1200
Wire Wire Line
	12900 4550 13000 4550
Wire Wire Line
	11300 3500 11400 3500
Wire Wire Line
	12950 3500 13050 3500
Wire Wire Line
	13050 3500 13050 2850
Wire Wire Line
	13050 2850 11300 2850
Wire Wire Line
	11300 2850 11300 2450
Wire Wire Line
	11300 2450 11400 2450
Wire Wire Line
	12950 2450 13050 2450
Wire Wire Line
	13050 2450 13050 2000
Wire Wire Line
	13050 2000 11300 2000
Wire Wire Line
	11300 2000 11300 1400
Wire Wire Line
	11300 1400 11400 1400
Wire Wire Line
	11000 1000 11400 1000
Wire Wire Line
	11100 1100 11400 1100
Wire Wire Line
	11200 1200 11400 1200
Wire Wire Line
	11200 2250 11400 2250
Wire Wire Line
	11000 3100 11400 3100
Wire Wire Line
	11100 3200 11400 3200
Wire Wire Line
	11200 3300 11400 3300
Wire Wire Line
	11150 4350 11350 4350
Wire Wire Line
	11300 4100 13000 4100
Wire Wire Line
	11300 3500 11300 4100
Wire Wire Line
	13000 4100 13000 4550
Wire Wire Line
	5900 4350 6200 4350
Wire Wire Line
	6200 4350 6200 3100
Wire Wire Line
	5900 1000 6200 1000
Connection ~ 6200 1000
Wire Wire Line
	6200 1000 6200 700 
Wire Wire Line
	5900 2250 6200 2250
Connection ~ 6200 2250
Wire Wire Line
	6200 2250 6200 1000
Wire Wire Line
	5900 3100 6200 3100
Connection ~ 6200 3100
Wire Wire Line
	6200 3100 6200 2250
Wire Wire Line
	8200 4350 8550 4350
Wire Wire Line
	8550 4350 8550 3100
Wire Wire Line
	10550 4350 10900 4350
Wire Wire Line
	10900 4350 10900 3100
Wire Wire Line
	3900 1700 6300 1700
Connection ~ 3900 1700
Connection ~ 6300 1700
Wire Wire Line
	6300 1700 8650 1700
Connection ~ 8650 1700
Wire Wire Line
	8650 1700 11000 1700
Wire Wire Line
	11100 1800 8750 1800
Connection ~ 4000 1800
Connection ~ 6400 1800
Wire Wire Line
	6400 1800 4000 1800
Connection ~ 8750 1800
Wire Wire Line
	8750 1800 6400 1800
Wire Wire Line
	4100 1900 6500 1900
Connection ~ 4100 1900
Connection ~ 6500 1900
Wire Wire Line
	6500 1900 8850 1900
Connection ~ 8850 1900
Wire Wire Line
	8850 1900 11200 1900
Wire Wire Line
	4100 2750 6500 2750
Connection ~ 4100 2750
Connection ~ 8850 2750
Wire Wire Line
	8850 2750 11200 2750
Wire Wire Line
	1850 2850 3600 2850
Wire Wire Line
	3600 3500 3500 3500
Wire Wire Line
	3600 2850 3600 3500
Wire Wire Line
	3600 2450 3600 2000
Wire Wire Line
	3500 2450 3600 2450
Wire Wire Line
	1850 2000 3600 2000
Wire Wire Line
	3450 4350 3800 4350
Wire Wire Line
	3800 4350 3800 3100
Wire Wire Line
	3500 1000 3800 1000
Connection ~ 3800 1000
Wire Wire Line
	3800 1000 3800 700 
Wire Wire Line
	3500 2250 3800 2250
Connection ~ 3800 2250
Wire Wire Line
	3800 2250 3800 1000
Wire Wire Line
	3500 3100 3800 3100
Connection ~ 3800 3100
Wire Wire Line
	3800 3100 3800 2250
Wire Wire Line
	3900 1000 4300 1000
Wire Wire Line
	4000 1100 4300 1100
Wire Wire Line
	4100 1200 4300 1200
Wire Wire Line
	4100 2250 4300 2250
Wire Wire Line
	3900 3100 4300 3100
Wire Wire Line
	4000 3200 4300 3200
Wire Wire Line
	4100 3300 4300 3300
Wire Wire Line
	8250 1000 8550 1000
Connection ~ 8550 1000
Wire Wire Line
	8550 1000 8550 700 
Wire Wire Line
	8250 2250 8550 2250
Connection ~ 8550 2250
Wire Wire Line
	8550 2250 8550 1000
Wire Wire Line
	8250 3100 8550 3100
Connection ~ 8550 3100
Wire Wire Line
	8550 3100 8550 2250
Wire Wire Line
	6650 4350 6500 4350
Wire Wire Line
	6500 4350 6500 4850
Connection ~ 6500 4850
Wire Wire Line
	6500 4850 8800 4850
Wire Wire Line
	3900 3800 6300 3800
Connection ~ 3900 3800
Connection ~ 8650 3800
Wire Wire Line
	8650 3800 11000 3800
Wire Wire Line
	4000 3900 6400 3900
Connection ~ 4000 3900
Connection ~ 8750 3900
Wire Wire Line
	8750 3900 11100 3900
Wire Wire Line
	4100 4000 6500 4000
Connection ~ 4100 4000
Connection ~ 8850 4000
Wire Wire Line
	8850 4000 11200 4000
Wire Wire Line
	6700 2250 6500 2250
Wire Wire Line
	6500 2250 6500 2750
Connection ~ 6500 2750
Wire Wire Line
	6500 2750 8850 2750
Wire Wire Line
	10600 1000 10900 1000
Connection ~ 10900 1000
Wire Wire Line
	10900 1000 10900 700 
Wire Wire Line
	10600 2250 10900 2250
Connection ~ 10900 2250
Wire Wire Line
	10900 2250 10900 1000
Wire Wire Line
	10600 3100 10900 3100
Connection ~ 10900 3100
Wire Wire Line
	10900 3100 10900 2250
Wire Wire Line
	13200 4350 13200 3100
Wire Wire Line
	12900 4350 13200 4350
Wire Wire Line
	12950 1000 13200 1000
Connection ~ 13200 1000
Wire Wire Line
	13200 1000 13200 700 
Wire Wire Line
	12950 2250 13200 2250
Connection ~ 13200 2250
Wire Wire Line
	13200 2250 13200 1000
Wire Wire Line
	12950 3100 13200 3100
Connection ~ 13200 3100
Wire Wire Line
	13200 3100 13200 2250
Wire Wire Line
	12950 1400 13300 1400
Wire Wire Line
	6700 3300 6500 3300
Wire Wire Line
	6500 3300 6500 4000
Connection ~ 6500 4000
Wire Wire Line
	6500 4000 8850 4000
Wire Wire Line
	6700 3200 6400 3200
Wire Wire Line
	6400 3200 6400 3900
Connection ~ 6400 3900
Wire Wire Line
	6400 3900 8750 3900
Wire Wire Line
	6700 3100 6300 3100
Wire Wire Line
	6300 3100 6300 3800
Connection ~ 6300 3800
Wire Wire Line
	6300 3800 8650 3800
Wire Wire Line
	3500 1400 4300 1400
Wire Wire Line
	5900 1400 6000 1400
Wire Wire Line
	6000 1400 6000 2000
Wire Wire Line
	4200 2000 4200 2450
Wire Wire Line
	6000 2450 6000 2850
Wire Wire Line
	6000 2850 4200 2850
Wire Wire Line
	4200 2850 4200 3500
Wire Wire Line
	6000 3500 6000 4100
Wire Wire Line
	1700 4850 4100 4850
Wire Wire Line
	4200 4550 4300 4550
Wire Wire Line
	4200 4100 4200 4550
Wire Wire Line
	4300 4350 4100 4350
Wire Wire Line
	4100 4350 4100 4850
Connection ~ 4100 4850
Wire Wire Line
	4100 4850 6500 4850
Wire Wire Line
	5900 4550 6650 4550
Wire Wire Line
	1700 4350 1900 4350
Wire Wire Line
	10550 4550 11350 4550
Wire Wire Line
	10600 1400 10700 1400
Wire Wire Line
	10700 1400 10700 2000
Wire Wire Line
	8950 2000 8950 2450
Wire Wire Line
	10700 2450 10700 2850
Wire Wire Line
	8950 2850 8950 3500
Wire Wire Line
	10700 3500 10700 4100
Connection ~ 8800 4850
Wire Wire Line
	8800 4850 11150 4850
Wire Wire Line
	8800 4350 9000 4350
Wire Wire Line
	8900 4100 8900 4550
Wire Wire Line
	8900 4550 9000 4550
Wire Wire Line
	8900 4100 10700 4100
Wire Wire Line
	8250 1400 9050 1400
Text Label 4000 8500 0    50   ~ 0
C1
Text Label 4000 8600 0    50   ~ 0
C2
Text Label 4000 8700 0    50   ~ 0
C3
Text Label 4000 8800 0    50   ~ 0
C4
Text Label 4000 8900 0    50   ~ 0
C5
Text Label 10050 7000 2    50   ~ 0
MISO
Text Label 10050 7200 2    50   ~ 0
SCK
Wire Wire Line
	4850 7300 4850 7450
$Comp
L woodpecker:MCP23S09-xP U1
U 1 1 60A94E22
P 4850 6700
F 0 "U1" H 5200 7250 50  0000 C CNN
F 1 "MCP23S09-xP" H 4500 6150 50  0000 C CNN
F 2 "Package_DIP:DIP-18_W7.62mm" H 4850 5650 50  0001 C CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/MCP23008-MCP23S08-Data-Sheet-20001919F.pdf" H 6150 5500 50  0001 C CNN
	1    4850 6700
	-1   0    0    -1  
$EndComp
$Comp
L woodpecker:MCP23S09-xP U2
U 1 1 60A96CC1
P 4850 8800
F 0 "U2" H 5200 9350 50  0000 C CNN
F 1 "MCP23S09-xP" H 4450 8250 50  0000 C CNN
F 2 "Package_DIP:DIP-18_W7.62mm" H 4850 7750 50  0001 C CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/MCP23008-MCP23S08-Data-Sheet-20001919F.pdf" H 6150 7600 50  0001 C CNN
	1    4850 8800
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x05_Odd_Even J1
U 1 1 60D04330
P 10500 7100
AR Path="/60D04330" Ref="J1"  Part="1" 
AR Path="/604E511D/60D04330" Ref="J1"  Part="1" 
F 0 "J1" H 10550 7517 50  0000 C CNN
F 1 "Conn_02x05_Odd_Even" H 10550 7426 50  0000 C CNN
F 2 "woodpecker:ConnectorMicromatch-10" H 10500 7100 50  0001 C CNN
F 3 "~" H 10500 7100 50  0001 C CNN
	1    10500 7100
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1900 4550 1350 4550
Text Label 13300 1400 0    50   ~ 0
DO
Text Label 1350 4550 2    50   ~ 0
DI
Wire Wire Line
	10700 7300 10900 7300
Wire Wire Line
	10200 7300 10050 7300
Text Label 10900 7300 0    50   ~ 0
DI
Text Label 10050 7300 2    50   ~ 0
DO
$Comp
L Mechanical:MountingHole H1
U 1 1 60E82AE2
P 13300 7650
F 0 "H1" H 13400 7696 50  0000 L CNN
F 1 "MountingHole" H 13400 7605 50  0000 L CNN
F 2 "woodpecker:MountingHole_3.2mm_M3_DIN965" H 13300 7650 50  0001 C CNN
F 3 "~" H 13300 7650 50  0001 C CNN
	1    13300 7650
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 605BA47D
P 13300 7850
F 0 "H2" H 13400 7896 50  0000 L CNN
F 1 "MountingHole" H 13400 7805 50  0000 L CNN
F 2 "woodpecker:MountingHole_3.2mm_M3_DIN965" H 13300 7850 50  0001 C CNN
F 3 "~" H 13300 7850 50  0001 C CNN
	1    13300 7850
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H3
U 1 1 605BA91F
P 13300 8050
F 0 "H3" H 13400 8096 50  0000 L CNN
F 1 "MountingHole" H 13400 8005 50  0000 L CNN
F 2 "woodpecker:MountingHole_3.2mm_M3_DIN965" H 13300 8050 50  0001 C CNN
F 3 "~" H 13300 8050 50  0001 C CNN
	1    13300 8050
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H4
U 1 1 605BABC6
P 13300 8250
F 0 "H4" H 13400 8296 50  0000 L CNN
F 1 "MountingHole" H 13400 8205 50  0000 L CNN
F 2 "woodpecker:MountingHole_3.2mm_M3_DIN965" H 13300 8250 50  0001 C CNN
F 3 "~" H 13300 8250 50  0001 C CNN
	1    13300 8250
	1    0    0    -1  
$EndComp
$EndSCHEMATC
