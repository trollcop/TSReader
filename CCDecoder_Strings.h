// XDS Strings/Data
char * programType[128] = {
	"education",
	"entertainment",
	"movie",
	"news",
	"religious",
	"sports",
	"other",
	"action",
	"advertisement",
	"animated",
	"anthology",
	"automobile",
	"awards",
	"baseball",
	"basketball",
	"bulletin",
	"business",
	"classical",
	"college",
	"combat",
	"comedy",
	"commentary",
	"concert",
	"consumer",
	"contemporary",
	"crime",
	"dance",
	"documentary",
	"drama",
	"elementary",
	"erotica",
	"exercise",
	"fantasy",
	"farm",
	"fashion",
	"fiction",
	"food",
	"football",
	"foreign",
	"fund raiser",
	"game/quiz",
	"garden",
	"golf",
	"government",
	"health",
	"high school",
	"history",
	"hobby",
	"hockey",
	"home",
	"horror",
	"information",
	"instruction",
	"international",
	"interview",
	"language",
	"legal",
	"live",
	"local",
	"math",
	"medical",
	"meeting",
	"military",
	"miniseries",
	"music",
	"mystery",
	"national",
	"nature",
	"police",
	"politics",
	"premiere",
	"prerecorded",
	"product",
	"professional",
	"public",
	"racing",
	"reading",
	"repair",
	"repeat",
	"review",
	"romance",
	"science",
	"series",
	"service",
	"shopping",
	"soap opera",
	"special",
	"suspense",
	"talk",
	"technical",
	"tennis",
	"travel",
	"variety",
	"video",
	"weather",
	"western"};

char *rating[8] = {
	"N/A",
	"G",
	"PG",
	"PG-13",
	"R",
	"NC-17",
	"X",
	"Not Rated"};

char *guidelines[8] = {
	"None",
	"TV-Y",
	"TV-Y7",
	"TV-G",
	"TV-PG",
	"TV-14",
	"TV-MA",
	"None"};

// Canadian English Rating System
char *CERating[8] = {
	"E",
	"C",
	"C8+",
	"G",
	"PG",
	"14+",
	"18+",
	"Illegal Rating Value"};

// Canadian French Rating System
	char *CFRating[8] = {
	"E",
	"G",
	"8 ans+",
	"13 ans+",
	"16 ans+",
	"18 ans+",
	"Illegal Rating Value",
	"Illegal Rating Value"};

char *audioLanguage[8] = {
	"Unknown",
	"English",
	"Spanish",
	"French",
	"German",
	"Italian",
	"Other",
	"None"};

char *audioMainType[8] = {
	"Unknown",
	"Mono",
	"Simulated Stereo",
	"True Stereo",
	"Stereo Surround",
	"Data Service",
	"Other",
	"None"};

char *audioSecondaryType[8] = {
	"Unknown",
	"Mono",
	"Video Descriptions",
	"Non-program Audio",
	"Special Effects",
	"Data Service",
	"Other",
	"None"};

char *captionService[8] = {
	"field one, channel C1, captioning",
	"field one, channel C1, text",
	"field one, channel C2, captioning",
	"field one, channel C2, text",
	"field two, channel C1, captioning",
	"field two, channel C1, text",
	"field two, channel C2, captioning",
	"field two, channel C2, text"};

char *dayString[8] = {
	"Not Used",
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"};

char *categoryCode[26] = {
	"TOA - Tornado Watch",
	"TOR - Tornado Warning",
	"SVA - Severe Thunderstorm Watch",
	"SVR - Severe Thunderstorm Warning",
	"SVS - Severe Weather Statement",
	"SPS - Special Weather Statement",
	"FFA - Flash Flood Watch",
	"FFW - Flash Flood Warning",
	"FFS - Flash Flood Statement",
	"FLA - Flood Watch",
	"FLW - Flood Warning",
	"FLS - Flood Statement",
	"WSA - Winter Storm Watch",
	"WSW - Winter Storm Warning",
	"BZW - Blizzard Warning",
	"HWA - High Wind Watch",
	"HWW - High Wind Warning",
	"HUA - Hurricane Watch",
	"HUW - Hurricane Warning",
	"HLS - Hurricane Statement",
	"LFP - Service Area Forecast",
	"BRT - Composite Broadcast Statement",
	"CEM - Civil Emergency Message",
	"DMO - Practice/Demo Warning",
	"ADR - Administrative Message",
	"XXX - Possibly Unknown Code"};

char *misc1Code[16] = {
	"(RCL) Resume Caption Loading",		//0
	"(BS)  Backspace",					//1
	"(AOF) Reserved (formerly alarm off)",	//2
	"(AON) Reserved (formerly alarm on)",	//3
	"(DER) Delete to end of row",			//4
	"(RU2) Roll-up captions, 2 rows",		//5
	"(RU3) Roll-up captions, 3 rows",		//6
	"(RU4) Roll-up captions, 4 rows",		//7
	"(FON) Flash on",						//8
	"(RDC) Resume direct captioning",		//9
	"(TR)  Text restart",					//10
	"(RTD) Resume text display",			//11
	"(EDM) Erase displayed memory",			//12
	"(CR)  Carriage return",				//13
	"(ENM) Erase nondisplayed memory",		//14
	"(EOC) End of caption (flip memories)"};//15

char *misc2Code[3] = {
	"(TO1) Tab offset (1 column)",
	"(TO2) Tab offset (2 column)",
	"(TO3) Tab offset (3 column)"};

char *underlineCode[2] = {
	"No Underline",
	"Underline"};

char *transparentCode[2] = {
	"Opaque",
	"Semi-Transparent"};

char *midRowCode[8] = {
	"White",
	"Green",
	"Blue",
	"Cyan",
	"Red",
	"Yellow",
	"Magenta",
	"Italics"};

char *preambleCode[16] = {
	"White",
	"Green",
	"Blue",
	"Cyan",
	"Red",
	"Yellow",
	"Magenta",
	"Italics",
	"Indent 0, White",
	"Indent 4, White",
	"Indent 8, White",
	"Indent 12, White",
	"Indent 16, White",
	"Indent 20, White",
	"Indent 24, White",
	"Indent 28, White"};

char *attributeCode[8] = {
	"White",
	"Green",
	"Blue",
	"Cyan",
	"Red",
	"Yellow",
	"Magenta",
	"Black"};

char *closedGroupCode[8] = {
	"Standard character set (normal size)",
	"Standard character set (double size)",
	"First private character set",
	"Second private character set",
	"People's Republic of China character set (GB 2312-80)",
	"Korean standard character set (KSC 5601-1987)",
	"First registered character set",
	"UNKNOWN Closed Group Code"};

char *xdsClassType[16] = {
	"",
	"Current Start",
	"Current Continue",
	"Future Start",
	"Future Continue",
	"Channel Start",
	"Channel Continue",
	"Miscellaneous Start",
	"Miscellaneous Continue",
	"Public Service Start",
	"Public Service Continue",
	"Reserved Start",
	"Reserved Continue",
	"Undefined Start",
	"Undefined Continue",
	"END"};

char *ratingSystem[8] = {
	"MPAA",
	"U.S. Parental Guidelines",
	"MPAA",
	"Canadian English Language Rating",
	"Canadian French Language Rating",
	"Reserved for non-U.S. & non-Canadian systems",
	"Reserved for non-U.S. & non-Canadian systems",
	"Error - Rating System type code not valid"};
