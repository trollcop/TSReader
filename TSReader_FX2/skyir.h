/*
The RF is modulated using the same bit patterns as the usual IR. 
It looks like this [view with fixed font!] 


_ = carrier off 
- = carrier on 


___------__-_-_-__-__--_-_-_-_- _-_-_-_-_-_-_-_-_--_-_-__-_-_-_-____ 


   <-----digibox-prefix-------> 
                               <--------16-data-bits----------> 




Each character in the above string is 450us long. 


Each data bit is _- to send a '0' and -_ to send a '1', 
so the above pattern sends 0070 hex (blue button). 


You just need to send the string once to register a keypress. 
Leave 300ms between button presses, to ensure all are accepted 
(particularly important if you want to press the same button twice). 


Here are the codes for the buttons (in hex): 


0=0000 1=0001 ... 9=0009 Off=000C 
ChUp=0020 ChDown=0021 Text=003C Up=0058 Down=0059 Left=005A Right=005B 
Select=005C Red=006D Green=006E Yellow=006F Blue=0070 BoxOff=007D 
Service=007E Sky=0080 Help=0081 Backup=0083 Info=00CB Guide=00CC 
Interactive=00F5 
*/
BYTE sky_power[] =        {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xa9, 0x6b}; // 11010101101100101010101010101010101010101010100101101011
BYTE sky_tv_guide[] =     {0xd5, 0xb2, 0xaa, 0xaa, 0xa9, 0x69, 0x6b}; // 11010101101100101010101010101010101010010110100101101011
BYTE sky_box_office[] =   {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x55, 0x67}; // 11010101101100101010101010101010101010100101010101100111
BYTE sky_services[] =     {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x55, 0x5b}; // 11010101101100101010101010101010101010100101010101011011
BYTE sky_interactive[] =  {0xd5, 0xb2, 0xaa, 0xaa, 0xa9, 0x56, 0x67}; // 11010101101100101010101010101010101010010101011001100111
BYTE sky_help[] =         {0xd5, 0xb2, 0xaa, 0xaa, 0xa9, 0xaa, 0xa7}; // 11010101101100101010101010101010101010011010101010100111
BYTE sky_back_up[] =      {0xd5, 0xb2, 0xaa, 0xaa, 0xa9, 0xaa, 0x97}; // 11010101101100101010101010101010101010011010101010010111
BYTE sky_info[] =         {0xd5, 0xb2, 0xaa, 0xaa, 0xa9, 0x69, 0x97}; // 11010101101100101010101010101010101010010110100110010111
BYTE sky_up[] =           {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x65, 0xab}; // 11010101101100101010101010101010101010100110010110101011
BYTE sky_down[] =         {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x65, 0xa7}; // 11010101101100101010101010101010101010100110010110100111
BYTE sky_left[] =         {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x65, 0x9b}; // 11010101101100101010101010101010101010100110010110011011
BYTE sky_right[] =        {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x65, 0x97}; // 11010101101100101010101010101010101010100110010110010111
BYTE sky_select[] =       {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x65, 0x6b}; // 11010101101100101010101010101010101010100110010101101011
BYTE sky_ch_up[] =        {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x9a, 0xab}; // 11010101101100101010101010101010101010101001101010101011
BYTE sky_ch_down[] =      {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x9a, 0xa7}; // 11010101101100101010101010101010101010101001101010100111
BYTE sky_red[] =          {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x59, 0x67}; // 11010101101100101010101010101010101010100101100101100111
BYTE sky_green[] =        {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x59, 0x5b}; // 11010101101100101010101010101010101010100101100101011011
BYTE sky_yellow[] =       {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x59, 0x57}; // 11010101101100101010101010101010101010100101100101010111
BYTE sky_blue[] =         {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0x56, 0xab}; // 11010101101100101010101010101010101010100101011010101011
BYTE sky_1[] =            {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xaa, 0xa7}; // 11010101101100101010101010101010101010101010101010100111
BYTE sky_2[] =            {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xaa, 0x9b}; // 11010101101100101010101010101010101010101010101010011011
BYTE sky_3[] =            {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xaa, 0x97}; // 11010101101100101010101010101010101010101010101010010111
BYTE sky_4[] =            {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xaa, 0x6b}; // 11010101101100101010101010101010101010101010101001101011
BYTE sky_5[] =            {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xaa, 0x67}; // 11010101101100101010101010101010101010101010101001100111
BYTE sky_6[] =            {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xaa, 0x5b}; // 11010101101100101010101010101010101010101010101001011011
BYTE sky_7[] =            {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xaa, 0x57}; // 11010101101100101010101010101010101010101010101001010111
BYTE sky_8[] =            {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xa9, 0xab}; // 11010101101100101010101010101010101010101010100110101011
BYTE sky_9[] =            {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xa9, 0xa7}; // 11010101101100101010101010101010101010101010100110100111
BYTE sky_0[] =            {0xd5, 0xb2, 0xaa, 0xaa, 0xaa, 0xaa, 0xab}; // 11010101101100101010101010101010101010101010101010101011
BYTE sky_sky[] =		  {0xd5, 0xb2, 0xaa, 0xaa, 0xa9, 0xaa, 0xab}; // 11010101101100101010101010101010101010011010101010101011
