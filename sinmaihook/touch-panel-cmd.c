#include <stdint.h>

/*
    Command analysis
read timeout = 1ms
response length = 9
request length = 6
- ConditioningMode(HALT to enter, STAT to exit):
    host clear all receive buffer
    change response length to 6
Request structure:
| 123 | var1 | var2 | cmd | var3 | 125 |
*/
// reset touch panel                (cmd = 69)
uint8_t command_RSET[] = {123, 82, 83, 69, 84, 125};
// stop touch event transmission    (cmd = 76)
uint8_t command_HALT[] = {123, 72, 65, 76, 84, 125};
// start touch event transmission   (cmd = 65)
uint8_t command_STAT[] = {123, 83, 84, 65, 84, 125};
/*
                                    (cmd = 114)
ratio[1] = sideChar(1P: 76, 2P: 82)
ratio[2] = switchDataChar(65 + (int)TouchPanelArea)
ratio[4] = ratio(default=50)
levelWrite = 12

Response (just copy the request):
resp[2] = 65 + (int)TouchPanelArea (might the same as ratio[2])
resp[4] = ratio[4]

Retry condition(receive again after 3000ms):
Receive timeout 3000ms
resp[2] = 64
resp[4] != 50(must the same as ratio[4])

ConditioningMode Process:
set ratio to 50 from 65 to 65+35
*/
uint8_t command_Ratio[] = {123, 42, 42, 114, 42, 125};
/*
                                    (cmd = 107)
sens[1] = sideChar(1P: 76, 2P: 82)
sens[2] = switchDataChar(65 + (int)TouchPanelArea)
sens[4] = sensitivity(range: 1-255, )
levelWrite = 12

Response (just copy the request):
resp[2] = 65 + (int)TouchPanelArea (might the same as sens[2])
resp[4] = sens[4]

Retry condition(receive again after 3000ms):
Receive timeout 3000ms
resp[2] = 64
resp[4] must the same as sens[4]

ConditioningMode Process:
set sensitivity from 65 to 65+33
*/
uint8_t command_Sensitivity[] = {123, 42, 42, 107, 42, 125};

/*
                                    (cmd = 116)
chk[1] = sideChar(1P: 76, 2P: 82)
chk[2] = switchDataChar(65 + (int)TouchPanelArea)
levelWrite = 12

Response (just copy the request):
resp[2] = 65 + (int)TouchPanelArea (might the same as chk[2])
resp[4] = chk[4]

Retry condition(receive again after 3000ms):
Receive timeout 3000ms
resp[2] = 64
resp[4] must the same as chk[4]

ConditioningMode Process:
set sensitivity from 65 to 65+33
*/
uint8_t command_SensCheck[] = {123, 42, 42, 116, 104, 125};

/*
Tapping report
response length = 9

Response structure:
|--1byte--|---7bytes---|--1byte--|
|   40    |   report   |   41    |
every bytes in report[i]:
byte &= 0x1F
panel_data[i * 5 + j] = byte & (1 << j) ? 1 : 0
*/
