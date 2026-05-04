#define UK_OFFSET_FLAG 0x00010000

static char szDVBTBandplanNames[][64] = 
{
	"Australia",
	"Complete (7 MHz)",
	"Complete (8 MHz)",
	"Complete (7 & 8 MHz)",
	"Complete 500k (7 MHz)",
	"Complete 500k (8 MHz)",
	"Complete 500k (7 & 8 MHz)",
	"Europe VHF (7 MHz)",
	"Europe VHF (8 MHz)",
	"Europe UHF",
	"France VHF",
	"Ireland VHF",
	"Italy VHF",
	"New Zealand VHF",
	"UK UHF",
	"USA BAS 12 MHz",
	"USA BAS 17 MHz",
	""
};

// Bandwidth:
// bit 0 - 5 MHz
// bit 1 - 6 MHz
// bit 2 - 7 MHz
// bit 3 - 8 MHz
static int nDVBTBandwidths[] =
{
	4, // Australia 7 MHz
	4, // Complete 7 MHz
	8, // Complete 8 MHz
	12, // Complete 7/8 MHz
	4, // Complete 500k 7 MHz
	8, // Complete 500k 8 MHz
	12, // Complete 500k 7/8 MHz
	4, // Europe VHF 7 MHz
	8, // Europe VHF 8 MHz
	8, // Europe UHF 8 MHz
	8, // Feance VHF 8 MHz
	8, // Ireland VHF 8 MHz
	4, // Italy VHF 7 MHz
	4, // New Zealand VHF 7MHz
	8 | UK_OFFSET_FLAG, // UK 8MHz
	8, // USA BAS 12 MHz 8 MHz
	8, // USA BAS 17 MHz 8 MHz
	0
};

static int nDVBTBandplans[] =
{
	177500, // Australia 6
	184500, // Australia 7
	191500, // Australia 8
	198500, // Australia 9
	205500, // Australia 9A
	212500, // Australia 10
	219500, // Australia 11
	226500, // Australia 12
	522500, // Australia 27
	529500, // Australia 28
	536500, // Australia 29
	543500, // Australia 30
	550500, // Australia 31
	557500, // Australia 32
	564500, // Australia 33
	571500, // Australia 34
	578500, // Australia 35
	585500, // Australia 36
	592500, // Australia 37
	599500, // Australia 38
	606500, // Australia 39
	613500, // Australia 40
	620500, // Australia 41
	627500, // Australia 42
	634500, // Australia 43
	641500, // Australia 44
	648500, // Australia 45
	655500, // Australia 46
	662500, // Australia 47
	669500, // Australia 48
	676500, // Australia 49
	683500, // Australia 50
	690500, // Australia 51
	697500, // Australia 52
	704500, // Australia 53
	711500, // Australia 54
	718500, // Australia 55
	725500, // Australia 56
	732500, // Australia 57
	739500, // Australia 58
	746500, // Australia 59
	753500, // Australia 60
	760500, // Australia 61
	767500, // Australia 62
	774500, // Australia 63
	781500, // Australia 64
	788500, // Australia 65
	795500, // Australia 66
	802500, // Australia 67
	809500, // Australia 68
	816500, // Australia 69
	0,      // ******************************* end of Australia
	111000, // Complete 7 MHz
	112000, // Complete 7 MHz
	113000, // Complete 7 MHz
	114000, // Complete 7 MHz
	115000, // Complete 7 MHz
	116000, // Complete 7 MHz
	117000, // Complete 7 MHz
	118000, // Complete 7 MHz
	119000, // Complete 7 MHz
	120000, // Complete 7 MHz
	121000, // Complete 7 MHz
	122000, // Complete 7 MHz
	123000, // Complete 7 MHz
	124000, // Complete 7 MHz
	125000, // Complete 7 MHz
	126000, // Complete 7 MHz
	127000, // Complete 7 MHz
	128000, // Complete 7 MHz
	129000, // Complete 7 MHz
	130000, // Complete 7 MHz
	131000, // Complete 7 MHz
	132000, // Complete 7 MHz
	133000, // Complete 7 MHz
	134000, // Complete 7 MHz
	135000, // Complete 7 MHz
	136000, // Complete 7 MHz
	137000, // Complete 7 MHz
	138000, // Complete 7 MHz
	139000, // Complete 7 MHz
	140000, // Complete 7 MHz
	141000, // Complete 7 MHz
	142000, // Complete 7 MHz
	143000, // Complete 7 MHz
	144000, // Complete 7 MHz
	145000, // Complete 7 MHz
	146000, // Complete 7 MHz
	147000, // Complete 7 MHz
	148000, // Complete 7 MHz
	149000, // Complete 7 MHz
	150000, // Complete 7 MHz
	151000, // Complete 7 MHz
	152000, // Complete 7 MHz
	153000, // Complete 7 MHz
	154000, // Complete 7 MHz
	155000, // Complete 7 MHz
	156000, // Complete 7 MHz
	157000, // Complete 7 MHz
	158000, // Complete 7 MHz
	159000, // Complete 7 MHz
	160000, // Complete 7 MHz
	161000, // Complete 7 MHz
	162000, // Complete 7 MHz
	163000, // Complete 7 MHz
	164000, // Complete 7 MHz
	165000, // Complete 7 MHz
	166000, // Complete 7 MHz
	167000, // Complete 7 MHz
	168000, // Complete 7 MHz
	169000, // Complete 7 MHz
	170000, // Complete 7 MHz
	171000, // Complete 7 MHz
	172000, // Complete 7 MHz
	173000, // Complete 7 MHz
	174000, // Complete 7 MHz
	175000, // Complete 7 MHz
	176000, // Complete 7 MHz
	177000, // Complete 7 MHz
	178000, // Complete 7 MHz
	179000, // Complete 7 MHz
	180000, // Complete 7 MHz
	181000, // Complete 7 MHz
	182000, // Complete 7 MHz
	183000, // Complete 7 MHz
	184000, // Complete 7 MHz
	185000, // Complete 7 MHz
	186000, // Complete 7 MHz
	187000, // Complete 7 MHz
	188000, // Complete 7 MHz
	189000, // Complete 7 MHz
	190000, // Complete 7 MHz
	191000, // Complete 7 MHz
	192000, // Complete 7 MHz
	193000, // Complete 7 MHz
	194000, // Complete 7 MHz
	195000, // Complete 7 MHz
	196000, // Complete 7 MHz
	197000, // Complete 7 MHz
	198000, // Complete 7 MHz
	199000, // Complete 7 MHz
	200000, // Complete 7 MHz
	201000, // Complete 7 MHz
	202000, // Complete 7 MHz
	203000, // Complete 7 MHz
	204000, // Complete 7 MHz
	205000, // Complete 7 MHz
	206000, // Complete 7 MHz
	207000, // Complete 7 MHz
	208000, // Complete 7 MHz
	209000, // Complete 7 MHz
	210000, // Complete 7 MHz
	211000, // Complete 7 MHz
	212000, // Complete 7 MHz
	213000, // Complete 7 MHz
	214000, // Complete 7 MHz
	215000, // Complete 7 MHz
	216000, // Complete 7 MHz
	217000, // Complete 7 MHz
	218000, // Complete 7 MHz
	219000, // Complete 7 MHz
	220000, // Complete 7 MHz
	221000, // Complete 7 MHz
	222000, // Complete 7 MHz
	223000, // Complete 7 MHz
	224000, // Complete 7 MHz
	225000, // Complete 7 MHz
	226000, // Complete 7 MHz
	227000, // Complete 7 MHz
	228000, // Complete 7 MHz
	229000, // Complete 7 MHz
	230000, // Complete 7 MHz
	231000, // Complete 7 MHz
	232000, // Complete 7 MHz
	233000, // Complete 7 MHz
	234000, // Complete 7 MHz
	235000, // Complete 7 MHz
	236000, // Complete 7 MHz
	237000, // Complete 7 MHz
	238000, // Complete 7 MHz
	239000, // Complete 7 MHz
	240000, // Complete 7 MHz
	241000, // Complete 7 MHz
	242000, // Complete 7 MHz
	243000, // Complete 7 MHz
	244000, // Complete 7 MHz
	245000, // Complete 7 MHz
	246000, // Complete 7 MHz
	247000, // Complete 7 MHz
	248000, // Complete 7 MHz
	249000, // Complete 7 MHz
	250000, // Complete 7 MHz
	251000, // Complete 7 MHz
	252000, // Complete 7 MHz
	253000, // Complete 7 MHz
	254000, // Complete 7 MHz
	255000, // Complete 7 MHz
	256000, // Complete 7 MHz
	257000, // Complete 7 MHz
	258000, // Complete 7 MHz
	259000, // Complete 7 MHz
	260000, // Complete 7 MHz
	261000, // Complete 7 MHz
	262000, // Complete 7 MHz
	263000, // Complete 7 MHz
	264000, // Complete 7 MHz
	265000, // Complete 7 MHz
	266000, // Complete 7 MHz
	267000, // Complete 7 MHz
	268000, // Complete 7 MHz
	269000, // Complete 7 MHz
	270000, // Complete 7 MHz
	271000, // Complete 7 MHz
	272000, // Complete 7 MHz
	273000, // Complete 7 MHz
	274000, // Complete 7 MHz
	275000, // Complete 7 MHz
	276000, // Complete 7 MHz
	277000, // Complete 7 MHz
	278000, // Complete 7 MHz
	279000, // Complete 7 MHz
	280000, // Complete 7 MHz
	281000, // Complete 7 MHz
	282000, // Complete 7 MHz
	283000, // Complete 7 MHz
	284000, // Complete 7 MHz
	285000, // Complete 7 MHz
	286000, // Complete 7 MHz
	287000, // Complete 7 MHz
	288000, // Complete 7 MHz
	289000, // Complete 7 MHz
	290000, // Complete 7 MHz
	291000, // Complete 7 MHz
	292000, // Complete 7 MHz
	293000, // Complete 7 MHz
	294000, // Complete 7 MHz
	295000, // Complete 7 MHz
	296000, // Complete 7 MHz
	297000, // Complete 7 MHz
	298000, // Complete 7 MHz
	299000, // Complete 7 MHz
	300000, // Complete 7 MHz
	301000, // Complete 7 MHz
	302000, // Complete 7 MHz
	303000, // Complete 7 MHz
	304000, // Complete 7 MHz
	305000, // Complete 7 MHz
	306000, // Complete 7 MHz
	307000, // Complete 7 MHz
	308000, // Complete 7 MHz
	309000, // Complete 7 MHz
	310000, // Complete 7 MHz
	311000, // Complete 7 MHz
	312000, // Complete 7 MHz
	313000, // Complete 7 MHz
	314000, // Complete 7 MHz
	315000, // Complete 7 MHz
	316000, // Complete 7 MHz
	317000, // Complete 7 MHz
	318000, // Complete 7 MHz
	319000, // Complete 7 MHz
	320000, // Complete 7 MHz
	321000, // Complete 7 MHz
	322000, // Complete 7 MHz
	323000, // Complete 7 MHz
	324000, // Complete 7 MHz
	325000, // Complete 7 MHz
	326000, // Complete 7 MHz
	327000, // Complete 7 MHz
	328000, // Complete 7 MHz
	329000, // Complete 7 MHz
	330000, // Complete 7 MHz
	331000, // Complete 7 MHz
	332000, // Complete 7 MHz
	333000, // Complete 7 MHz
	334000, // Complete 7 MHz
	335000, // Complete 7 MHz
	336000, // Complete 7 MHz
	337000, // Complete 7 MHz
	338000, // Complete 7 MHz
	339000, // Complete 7 MHz
	340000, // Complete 7 MHz
	341000, // Complete 7 MHz
	342000, // Complete 7 MHz
	343000, // Complete 7 MHz
	344000, // Complete 7 MHz
	345000, // Complete 7 MHz
	346000, // Complete 7 MHz
	347000, // Complete 7 MHz
	348000, // Complete 7 MHz
	349000, // Complete 7 MHz
	350000, // Complete 7 MHz
	351000, // Complete 7 MHz
	352000, // Complete 7 MHz
	353000, // Complete 7 MHz
	354000, // Complete 7 MHz
	355000, // Complete 7 MHz
	356000, // Complete 7 MHz
	357000, // Complete 7 MHz
	358000, // Complete 7 MHz
	359000, // Complete 7 MHz
	360000, // Complete 7 MHz
	361000, // Complete 7 MHz
	362000, // Complete 7 MHz
	363000, // Complete 7 MHz
	364000, // Complete 7 MHz
	365000, // Complete 7 MHz
	366000, // Complete 7 MHz
	367000, // Complete 7 MHz
	368000, // Complete 7 MHz
	369000, // Complete 7 MHz
	370000, // Complete 7 MHz
	371000, // Complete 7 MHz
	372000, // Complete 7 MHz
	373000, // Complete 7 MHz
	374000, // Complete 7 MHz
	375000, // Complete 7 MHz
	376000, // Complete 7 MHz
	377000, // Complete 7 MHz
	378000, // Complete 7 MHz
	379000, // Complete 7 MHz
	380000, // Complete 7 MHz
	381000, // Complete 7 MHz
	382000, // Complete 7 MHz
	383000, // Complete 7 MHz
	384000, // Complete 7 MHz
	385000, // Complete 7 MHz
	386000, // Complete 7 MHz
	387000, // Complete 7 MHz
	388000, // Complete 7 MHz
	389000, // Complete 7 MHz
	390000, // Complete 7 MHz
	391000, // Complete 7 MHz
	392000, // Complete 7 MHz
	393000, // Complete 7 MHz
	394000, // Complete 7 MHz
	395000, // Complete 7 MHz
	396000, // Complete 7 MHz
	397000, // Complete 7 MHz
	398000, // Complete 7 MHz
	399000, // Complete 7 MHz
	400000, // Complete 7 MHz
	401000, // Complete 7 MHz
	402000, // Complete 7 MHz
	403000, // Complete 7 MHz
	404000, // Complete 7 MHz
	405000, // Complete 7 MHz
	406000, // Complete 7 MHz
	407000, // Complete 7 MHz
	408000, // Complete 7 MHz
	409000, // Complete 7 MHz
	410000, // Complete 7 MHz
	411000, // Complete 7 MHz
	412000, // Complete 7 MHz
	413000, // Complete 7 MHz
	414000, // Complete 7 MHz
	415000, // Complete 7 MHz
	416000, // Complete 7 MHz
	417000, // Complete 7 MHz
	418000, // Complete 7 MHz
	419000, // Complete 7 MHz
	420000, // Complete 7 MHz
	421000, // Complete 7 MHz
	422000, // Complete 7 MHz
	423000, // Complete 7 MHz
	424000, // Complete 7 MHz
	425000, // Complete 7 MHz
	426000, // Complete 7 MHz
	427000, // Complete 7 MHz
	428000, // Complete 7 MHz
	429000, // Complete 7 MHz
	430000, // Complete 7 MHz
	431000, // Complete 7 MHz
	432000, // Complete 7 MHz
	433000, // Complete 7 MHz
	434000, // Complete 7 MHz
	435000, // Complete 7 MHz
	436000, // Complete 7 MHz
	437000, // Complete 7 MHz
	438000, // Complete 7 MHz
	439000, // Complete 7 MHz
	440000, // Complete 7 MHz
	441000, // Complete 7 MHz
	442000, // Complete 7 MHz
	443000, // Complete 7 MHz
	444000, // Complete 7 MHz
	445000, // Complete 7 MHz
	446000, // Complete 7 MHz
	447000, // Complete 7 MHz
	448000, // Complete 7 MHz
	449000, // Complete 7 MHz
	450000, // Complete 7 MHz
	451000, // Complete 7 MHz
	452000, // Complete 7 MHz
	453000, // Complete 7 MHz
	454000, // Complete 7 MHz
	455000, // Complete 7 MHz
	456000, // Complete 7 MHz
	457000, // Complete 7 MHz
	458000, // Complete 7 MHz
	459000, // Complete 7 MHz
	460000, // Complete 7 MHz
	461000, // Complete 7 MHz
	462000, // Complete 7 MHz
	463000, // Complete 7 MHz
	464000, // Complete 7 MHz
	465000, // Complete 7 MHz
	466000, // Complete 7 MHz
	467000, // Complete 7 MHz
	468000, // Complete 7 MHz
	469000, // Complete 7 MHz
	470000, // Complete 7 MHz
	471000, // Complete 7 MHz
	472000, // Complete 7 MHz
	473000, // Complete 7 MHz
	474000, // Complete 7 MHz
	475000, // Complete 7 MHz
	476000, // Complete 7 MHz
	477000, // Complete 7 MHz
	478000, // Complete 7 MHz
	479000, // Complete 7 MHz
	480000, // Complete 7 MHz
	481000, // Complete 7 MHz
	482000, // Complete 7 MHz
	483000, // Complete 7 MHz
	484000, // Complete 7 MHz
	485000, // Complete 7 MHz
	486000, // Complete 7 MHz
	487000, // Complete 7 MHz
	488000, // Complete 7 MHz
	489000, // Complete 7 MHz
	490000, // Complete 7 MHz
	491000, // Complete 7 MHz
	492000, // Complete 7 MHz
	493000, // Complete 7 MHz
	494000, // Complete 7 MHz
	495000, // Complete 7 MHz
	496000, // Complete 7 MHz
	497000, // Complete 7 MHz
	498000, // Complete 7 MHz
	499000, // Complete 7 MHz
	500000, // Complete 7 MHz
	501000, // Complete 7 MHz
	502000, // Complete 7 MHz
	503000, // Complete 7 MHz
	504000, // Complete 7 MHz
	505000, // Complete 7 MHz
	506000, // Complete 7 MHz
	507000, // Complete 7 MHz
	508000, // Complete 7 MHz
	509000, // Complete 7 MHz
	510000, // Complete 7 MHz
	511000, // Complete 7 MHz
	512000, // Complete 7 MHz
	513000, // Complete 7 MHz
	514000, // Complete 7 MHz
	515000, // Complete 7 MHz
	516000, // Complete 7 MHz
	517000, // Complete 7 MHz
	518000, // Complete 7 MHz
	519000, // Complete 7 MHz
	520000, // Complete 7 MHz
	521000, // Complete 7 MHz
	522000, // Complete 7 MHz
	523000, // Complete 7 MHz
	524000, // Complete 7 MHz
	525000, // Complete 7 MHz
	526000, // Complete 7 MHz
	527000, // Complete 7 MHz
	528000, // Complete 7 MHz
	529000, // Complete 7 MHz
	530000, // Complete 7 MHz
	531000, // Complete 7 MHz
	532000, // Complete 7 MHz
	533000, // Complete 7 MHz
	534000, // Complete 7 MHz
	535000, // Complete 7 MHz
	536000, // Complete 7 MHz
	537000, // Complete 7 MHz
	538000, // Complete 7 MHz
	539000, // Complete 7 MHz
	540000, // Complete 7 MHz
	541000, // Complete 7 MHz
	542000, // Complete 7 MHz
	543000, // Complete 7 MHz
	544000, // Complete 7 MHz
	545000, // Complete 7 MHz
	546000, // Complete 7 MHz
	547000, // Complete 7 MHz
	548000, // Complete 7 MHz
	549000, // Complete 7 MHz
	550000, // Complete 7 MHz
	551000, // Complete 7 MHz
	552000, // Complete 7 MHz
	553000, // Complete 7 MHz
	554000, // Complete 7 MHz
	555000, // Complete 7 MHz
	556000, // Complete 7 MHz
	557000, // Complete 7 MHz
	558000, // Complete 7 MHz
	559000, // Complete 7 MHz
	560000, // Complete 7 MHz
	561000, // Complete 7 MHz
	562000, // Complete 7 MHz
	563000, // Complete 7 MHz
	564000, // Complete 7 MHz
	565000, // Complete 7 MHz
	566000, // Complete 7 MHz
	567000, // Complete 7 MHz
	568000, // Complete 7 MHz
	569000, // Complete 7 MHz
	570000, // Complete 7 MHz
	571000, // Complete 7 MHz
	572000, // Complete 7 MHz
	573000, // Complete 7 MHz
	574000, // Complete 7 MHz
	575000, // Complete 7 MHz
	576000, // Complete 7 MHz
	577000, // Complete 7 MHz
	578000, // Complete 7 MHz
	579000, // Complete 7 MHz
	580000, // Complete 7 MHz
	581000, // Complete 7 MHz
	582000, // Complete 7 MHz
	583000, // Complete 7 MHz
	584000, // Complete 7 MHz
	585000, // Complete 7 MHz
	586000, // Complete 7 MHz
	587000, // Complete 7 MHz
	588000, // Complete 7 MHz
	589000, // Complete 7 MHz
	590000, // Complete 7 MHz
	591000, // Complete 7 MHz
	592000, // Complete 7 MHz
	593000, // Complete 7 MHz
	594000, // Complete 7 MHz
	595000, // Complete 7 MHz
	596000, // Complete 7 MHz
	597000, // Complete 7 MHz
	598000, // Complete 7 MHz
	599000, // Complete 7 MHz
	600000, // Complete 7 MHz
	601000, // Complete 7 MHz
	602000, // Complete 7 MHz
	603000, // Complete 7 MHz
	604000, // Complete 7 MHz
	605000, // Complete 7 MHz
	606000, // Complete 7 MHz
	607000, // Complete 7 MHz
	608000, // Complete 7 MHz
	609000, // Complete 7 MHz
	610000, // Complete 7 MHz
	611000, // Complete 7 MHz
	612000, // Complete 7 MHz
	613000, // Complete 7 MHz
	614000, // Complete 7 MHz
	615000, // Complete 7 MHz
	616000, // Complete 7 MHz
	617000, // Complete 7 MHz
	618000, // Complete 7 MHz
	619000, // Complete 7 MHz
	620000, // Complete 7 MHz
	621000, // Complete 7 MHz
	622000, // Complete 7 MHz
	623000, // Complete 7 MHz
	624000, // Complete 7 MHz
	625000, // Complete 7 MHz
	626000, // Complete 7 MHz
	627000, // Complete 7 MHz
	628000, // Complete 7 MHz
	629000, // Complete 7 MHz
	630000, // Complete 7 MHz
	631000, // Complete 7 MHz
	632000, // Complete 7 MHz
	633000, // Complete 7 MHz
	634000, // Complete 7 MHz
	635000, // Complete 7 MHz
	636000, // Complete 7 MHz
	637000, // Complete 7 MHz
	638000, // Complete 7 MHz
	639000, // Complete 7 MHz
	640000, // Complete 7 MHz
	641000, // Complete 7 MHz
	642000, // Complete 7 MHz
	643000, // Complete 7 MHz
	644000, // Complete 7 MHz
	645000, // Complete 7 MHz
	646000, // Complete 7 MHz
	647000, // Complete 7 MHz
	648000, // Complete 7 MHz
	649000, // Complete 7 MHz
	650000, // Complete 7 MHz
	651000, // Complete 7 MHz
	652000, // Complete 7 MHz
	653000, // Complete 7 MHz
	654000, // Complete 7 MHz
	655000, // Complete 7 MHz
	656000, // Complete 7 MHz
	657000, // Complete 7 MHz
	658000, // Complete 7 MHz
	659000, // Complete 7 MHz
	660000, // Complete 7 MHz
	661000, // Complete 7 MHz
	662000, // Complete 7 MHz
	663000, // Complete 7 MHz
	664000, // Complete 7 MHz
	665000, // Complete 7 MHz
	666000, // Complete 7 MHz
	667000, // Complete 7 MHz
	668000, // Complete 7 MHz
	669000, // Complete 7 MHz
	670000, // Complete 7 MHz
	671000, // Complete 7 MHz
	672000, // Complete 7 MHz
	673000, // Complete 7 MHz
	674000, // Complete 7 MHz
	675000, // Complete 7 MHz
	676000, // Complete 7 MHz
	677000, // Complete 7 MHz
	678000, // Complete 7 MHz
	679000, // Complete 7 MHz
	680000, // Complete 7 MHz
	681000, // Complete 7 MHz
	682000, // Complete 7 MHz
	683000, // Complete 7 MHz
	684000, // Complete 7 MHz
	685000, // Complete 7 MHz
	686000, // Complete 7 MHz
	687000, // Complete 7 MHz
	688000, // Complete 7 MHz
	689000, // Complete 7 MHz
	690000, // Complete 7 MHz
	691000, // Complete 7 MHz
	692000, // Complete 7 MHz
	693000, // Complete 7 MHz
	694000, // Complete 7 MHz
	695000, // Complete 7 MHz
	696000, // Complete 7 MHz
	697000, // Complete 7 MHz
	698000, // Complete 7 MHz
	699000, // Complete 7 MHz
	700000, // Complete 7 MHz
	701000, // Complete 7 MHz
	702000, // Complete 7 MHz
	703000, // Complete 7 MHz
	704000, // Complete 7 MHz
	705000, // Complete 7 MHz
	706000, // Complete 7 MHz
	707000, // Complete 7 MHz
	708000, // Complete 7 MHz
	709000, // Complete 7 MHz
	710000, // Complete 7 MHz
	711000, // Complete 7 MHz
	712000, // Complete 7 MHz
	713000, // Complete 7 MHz
	714000, // Complete 7 MHz
	715000, // Complete 7 MHz
	716000, // Complete 7 MHz
	717000, // Complete 7 MHz
	718000, // Complete 7 MHz
	719000, // Complete 7 MHz
	720000, // Complete 7 MHz
	721000, // Complete 7 MHz
	722000, // Complete 7 MHz
	723000, // Complete 7 MHz
	724000, // Complete 7 MHz
	725000, // Complete 7 MHz
	726000, // Complete 7 MHz
	727000, // Complete 7 MHz
	728000, // Complete 7 MHz
	729000, // Complete 7 MHz
	730000, // Complete 7 MHz
	731000, // Complete 7 MHz
	732000, // Complete 7 MHz
	733000, // Complete 7 MHz
	734000, // Complete 7 MHz
	735000, // Complete 7 MHz
	736000, // Complete 7 MHz
	737000, // Complete 7 MHz
	738000, // Complete 7 MHz
	739000, // Complete 7 MHz
	740000, // Complete 7 MHz
	741000, // Complete 7 MHz
	742000, // Complete 7 MHz
	743000, // Complete 7 MHz
	744000, // Complete 7 MHz
	745000, // Complete 7 MHz
	746000, // Complete 7 MHz
	747000, // Complete 7 MHz
	748000, // Complete 7 MHz
	749000, // Complete 7 MHz
	750000, // Complete 7 MHz
	751000, // Complete 7 MHz
	752000, // Complete 7 MHz
	753000, // Complete 7 MHz
	754000, // Complete 7 MHz
	755000, // Complete 7 MHz
	756000, // Complete 7 MHz
	757000, // Complete 7 MHz
	758000, // Complete 7 MHz
	759000, // Complete 7 MHz
	760000, // Complete 7 MHz
	761000, // Complete 7 MHz
	762000, // Complete 7 MHz
	763000, // Complete 7 MHz
	764000, // Complete 7 MHz
	765000, // Complete 7 MHz
	766000, // Complete 7 MHz
	767000, // Complete 7 MHz
	768000, // Complete 7 MHz
	769000, // Complete 7 MHz
	770000, // Complete 7 MHz
	771000, // Complete 7 MHz
	772000, // Complete 7 MHz
	773000, // Complete 7 MHz
	774000, // Complete 7 MHz
	775000, // Complete 7 MHz
	776000, // Complete 7 MHz
	777000, // Complete 7 MHz
	778000, // Complete 7 MHz
	779000, // Complete 7 MHz
	780000, // Complete 7 MHz
	781000, // Complete 7 MHz
	782000, // Complete 7 MHz
	783000, // Complete 7 MHz
	784000, // Complete 7 MHz
	785000, // Complete 7 MHz
	786000, // Complete 7 MHz
	787000, // Complete 7 MHz
	788000, // Complete 7 MHz
	789000, // Complete 7 MHz
	790000, // Complete 7 MHz
	791000, // Complete 7 MHz
	792000, // Complete 7 MHz
	793000, // Complete 7 MHz
	794000, // Complete 7 MHz
	795000, // Complete 7 MHz
	796000, // Complete 7 MHz
	797000, // Complete 7 MHz
	798000, // Complete 7 MHz
	799000, // Complete 7 MHz
	800000, // Complete 7 MHz
	801000, // Complete 7 MHz
	802000, // Complete 7 MHz
	803000, // Complete 7 MHz
	804000, // Complete 7 MHz
	805000, // Complete 7 MHz
	806000, // Complete 7 MHz
	807000, // Complete 7 MHz
	808000, // Complete 7 MHz
	809000, // Complete 7 MHz
	810000, // Complete 7 MHz
	811000, // Complete 7 MHz
	812000, // Complete 7 MHz
	813000, // Complete 7 MHz
	814000, // Complete 7 MHz
	815000, // Complete 7 MHz
	816000, // Complete 7 MHz
	817000, // Complete 7 MHz
	818000, // Complete 7 MHz
	819000, // Complete 7 MHz
	820000, // Complete 7 MHz
	821000, // Complete 7 MHz
	822000, // Complete 7 MHz
	823000, // Complete 7 MHz
	824000, // Complete 7 MHz
	825000, // Complete 7 MHz
	826000, // Complete 7 MHz
	827000, // Complete 7 MHz
	828000, // Complete 7 MHz
	829000, // Complete 7 MHz
	830000, // Complete 7 MHz
	831000, // Complete 7 MHz
	832000, // Complete 7 MHz
	833000, // Complete 7 MHz
	834000, // Complete 7 MHz
	835000, // Complete 7 MHz
	836000, // Complete 7 MHz
	837000, // Complete 7 MHz
	838000, // Complete 7 MHz
	839000, // Complete 7 MHz
	840000, // Complete 7 MHz
	841000, // Complete 7 MHz
	842000, // Complete 7 MHz
	843000, // Complete 7 MHz
	844000, // Complete 7 MHz
	845000, // Complete 7 MHz
	846000, // Complete 7 MHz
	847000, // Complete 7 MHz
	848000, // Complete 7 MHz
	849000, // Complete 7 MHz
	850000, // Complete 7 MHz
	851000, // Complete 7 MHz
	852000, // Complete 7 MHz
	853000, // Complete 7 MHz
	854000, // Complete 7 MHz
	855000, // Complete 7 MHz
	856000, // Complete 7 MHz
	857000, // Complete 7 MHz
	858000, // Complete 7 MHz
	0,		// ******************************* end of Complete 7 MHz
	111000, // Complete 8 MHz
	112000, // Complete 8 MHz
	113000, // Complete 8 MHz
	114000, // Complete 8 MHz
	115000, // Complete 8 MHz
	116000, // Complete 8 MHz
	117000, // Complete 8 MHz
	118000, // Complete 8 MHz
	119000, // Complete 8 MHz
	120000, // Complete 8 MHz
	121000, // Complete 8 MHz
	122000, // Complete 8 MHz
	123000, // Complete 8 MHz
	124000, // Complete 8 MHz
	125000, // Complete 8 MHz
	126000, // Complete 8 MHz
	127000, // Complete 8 MHz
	128000, // Complete 8 MHz
	129000, // Complete 8 MHz
	130000, // Complete 8 MHz
	131000, // Complete 8 MHz
	132000, // Complete 8 MHz
	133000, // Complete 8 MHz
	134000, // Complete 8 MHz
	135000, // Complete 8 MHz
	136000, // Complete 8 MHz
	137000, // Complete 8 MHz
	138000, // Complete 8 MHz
	139000, // Complete 8 MHz
	140000, // Complete 8 MHz
	141000, // Complete 8 MHz
	142000, // Complete 8 MHz
	143000, // Complete 8 MHz
	144000, // Complete 8 MHz
	145000, // Complete 8 MHz
	146000, // Complete 8 MHz
	147000, // Complete 8 MHz
	148000, // Complete 8 MHz
	149000, // Complete 8 MHz
	150000, // Complete 8 MHz
	151000, // Complete 8 MHz
	152000, // Complete 8 MHz
	153000, // Complete 8 MHz
	154000, // Complete 8 MHz
	155000, // Complete 8 MHz
	156000, // Complete 8 MHz
	157000, // Complete 8 MHz
	158000, // Complete 8 MHz
	159000, // Complete 8 MHz
	160000, // Complete 8 MHz
	161000, // Complete 8 MHz
	162000, // Complete 8 MHz
	163000, // Complete 8 MHz
	164000, // Complete 8 MHz
	165000, // Complete 8 MHz
	166000, // Complete 8 MHz
	167000, // Complete 8 MHz
	168000, // Complete 8 MHz
	169000, // Complete 8 MHz
	170000, // Complete 8 MHz
	171000, // Complete 8 MHz
	172000, // Complete 8 MHz
	173000, // Complete 8 MHz
	174000, // Complete 8 MHz
	175000, // Complete 8 MHz
	176000, // Complete 8 MHz
	177000, // Complete 8 MHz
	178000, // Complete 8 MHz
	179000, // Complete 8 MHz
	180000, // Complete 8 MHz
	181000, // Complete 8 MHz
	182000, // Complete 8 MHz
	183000, // Complete 8 MHz
	184000, // Complete 8 MHz
	185000, // Complete 8 MHz
	186000, // Complete 8 MHz
	187000, // Complete 8 MHz
	188000, // Complete 8 MHz
	189000, // Complete 8 MHz
	190000, // Complete 8 MHz
	191000, // Complete 8 MHz
	192000, // Complete 8 MHz
	193000, // Complete 8 MHz
	194000, // Complete 8 MHz
	195000, // Complete 8 MHz
	196000, // Complete 8 MHz
	197000, // Complete 8 MHz
	198000, // Complete 8 MHz
	199000, // Complete 8 MHz
	200000, // Complete 8 MHz
	201000, // Complete 8 MHz
	202000, // Complete 8 MHz
	203000, // Complete 8 MHz
	204000, // Complete 8 MHz
	205000, // Complete 8 MHz
	206000, // Complete 8 MHz
	207000, // Complete 8 MHz
	208000, // Complete 8 MHz
	209000, // Complete 8 MHz
	210000, // Complete 8 MHz
	211000, // Complete 8 MHz
	212000, // Complete 8 MHz
	213000, // Complete 8 MHz
	214000, // Complete 8 MHz
	215000, // Complete 8 MHz
	216000, // Complete 8 MHz
	217000, // Complete 8 MHz
	218000, // Complete 8 MHz
	219000, // Complete 8 MHz
	220000, // Complete 8 MHz
	221000, // Complete 8 MHz
	222000, // Complete 8 MHz
	223000, // Complete 8 MHz
	224000, // Complete 8 MHz
	225000, // Complete 8 MHz
	226000, // Complete 8 MHz
	227000, // Complete 8 MHz
	228000, // Complete 8 MHz
	229000, // Complete 8 MHz
	230000, // Complete 8 MHz
	231000, // Complete 8 MHz
	232000, // Complete 8 MHz
	233000, // Complete 8 MHz
	234000, // Complete 8 MHz
	235000, // Complete 8 MHz
	236000, // Complete 8 MHz
	237000, // Complete 8 MHz
	238000, // Complete 8 MHz
	239000, // Complete 8 MHz
	240000, // Complete 8 MHz
	241000, // Complete 8 MHz
	242000, // Complete 8 MHz
	243000, // Complete 8 MHz
	244000, // Complete 8 MHz
	245000, // Complete 8 MHz
	246000, // Complete 8 MHz
	247000, // Complete 8 MHz
	248000, // Complete 8 MHz
	249000, // Complete 8 MHz
	250000, // Complete 8 MHz
	251000, // Complete 8 MHz
	252000, // Complete 8 MHz
	253000, // Complete 8 MHz
	254000, // Complete 8 MHz
	255000, // Complete 8 MHz
	256000, // Complete 8 MHz
	257000, // Complete 8 MHz
	258000, // Complete 8 MHz
	259000, // Complete 8 MHz
	260000, // Complete 8 MHz
	261000, // Complete 8 MHz
	262000, // Complete 8 MHz
	263000, // Complete 8 MHz
	264000, // Complete 8 MHz
	265000, // Complete 8 MHz
	266000, // Complete 8 MHz
	267000, // Complete 8 MHz
	268000, // Complete 8 MHz
	269000, // Complete 8 MHz
	270000, // Complete 8 MHz
	271000, // Complete 8 MHz
	272000, // Complete 8 MHz
	273000, // Complete 8 MHz
	274000, // Complete 8 MHz
	275000, // Complete 8 MHz
	276000, // Complete 8 MHz
	277000, // Complete 8 MHz
	278000, // Complete 8 MHz
	279000, // Complete 8 MHz
	280000, // Complete 8 MHz
	281000, // Complete 8 MHz
	282000, // Complete 8 MHz
	283000, // Complete 8 MHz
	284000, // Complete 8 MHz
	285000, // Complete 8 MHz
	286000, // Complete 8 MHz
	287000, // Complete 8 MHz
	288000, // Complete 8 MHz
	289000, // Complete 8 MHz
	290000, // Complete 8 MHz
	291000, // Complete 8 MHz
	292000, // Complete 8 MHz
	293000, // Complete 8 MHz
	294000, // Complete 8 MHz
	295000, // Complete 8 MHz
	296000, // Complete 8 MHz
	297000, // Complete 8 MHz
	298000, // Complete 8 MHz
	299000, // Complete 8 MHz
	300000, // Complete 8 MHz
	301000, // Complete 8 MHz
	302000, // Complete 8 MHz
	303000, // Complete 8 MHz
	304000, // Complete 8 MHz
	305000, // Complete 8 MHz
	306000, // Complete 8 MHz
	307000, // Complete 8 MHz
	308000, // Complete 8 MHz
	309000, // Complete 8 MHz
	310000, // Complete 8 MHz
	311000, // Complete 8 MHz
	312000, // Complete 8 MHz
	313000, // Complete 8 MHz
	314000, // Complete 8 MHz
	315000, // Complete 8 MHz
	316000, // Complete 8 MHz
	317000, // Complete 8 MHz
	318000, // Complete 8 MHz
	319000, // Complete 8 MHz
	320000, // Complete 8 MHz
	321000, // Complete 8 MHz
	322000, // Complete 8 MHz
	323000, // Complete 8 MHz
	324000, // Complete 8 MHz
	325000, // Complete 8 MHz
	326000, // Complete 8 MHz
	327000, // Complete 8 MHz
	328000, // Complete 8 MHz
	329000, // Complete 8 MHz
	330000, // Complete 8 MHz
	331000, // Complete 8 MHz
	332000, // Complete 8 MHz
	333000, // Complete 8 MHz
	334000, // Complete 8 MHz
	335000, // Complete 8 MHz
	336000, // Complete 8 MHz
	337000, // Complete 8 MHz
	338000, // Complete 8 MHz
	339000, // Complete 8 MHz
	340000, // Complete 8 MHz
	341000, // Complete 8 MHz
	342000, // Complete 8 MHz
	343000, // Complete 8 MHz
	344000, // Complete 8 MHz
	345000, // Complete 8 MHz
	346000, // Complete 8 MHz
	347000, // Complete 8 MHz
	348000, // Complete 8 MHz
	349000, // Complete 8 MHz
	350000, // Complete 8 MHz
	351000, // Complete 8 MHz
	352000, // Complete 8 MHz
	353000, // Complete 8 MHz
	354000, // Complete 8 MHz
	355000, // Complete 8 MHz
	356000, // Complete 8 MHz
	357000, // Complete 8 MHz
	358000, // Complete 8 MHz
	359000, // Complete 8 MHz
	360000, // Complete 8 MHz
	361000, // Complete 8 MHz
	362000, // Complete 8 MHz
	363000, // Complete 8 MHz
	364000, // Complete 8 MHz
	365000, // Complete 8 MHz
	366000, // Complete 8 MHz
	367000, // Complete 8 MHz
	368000, // Complete 8 MHz
	369000, // Complete 8 MHz
	370000, // Complete 8 MHz
	371000, // Complete 8 MHz
	372000, // Complete 8 MHz
	373000, // Complete 8 MHz
	374000, // Complete 8 MHz
	375000, // Complete 8 MHz
	376000, // Complete 8 MHz
	377000, // Complete 8 MHz
	378000, // Complete 8 MHz
	379000, // Complete 8 MHz
	380000, // Complete 8 MHz
	381000, // Complete 8 MHz
	382000, // Complete 8 MHz
	383000, // Complete 8 MHz
	384000, // Complete 8 MHz
	385000, // Complete 8 MHz
	386000, // Complete 8 MHz
	387000, // Complete 8 MHz
	388000, // Complete 8 MHz
	389000, // Complete 8 MHz
	390000, // Complete 8 MHz
	391000, // Complete 8 MHz
	392000, // Complete 8 MHz
	393000, // Complete 8 MHz
	394000, // Complete 8 MHz
	395000, // Complete 8 MHz
	396000, // Complete 8 MHz
	397000, // Complete 8 MHz
	398000, // Complete 8 MHz
	399000, // Complete 8 MHz
	400000, // Complete 8 MHz
	401000, // Complete 8 MHz
	402000, // Complete 8 MHz
	403000, // Complete 8 MHz
	404000, // Complete 8 MHz
	405000, // Complete 8 MHz
	406000, // Complete 8 MHz
	407000, // Complete 8 MHz
	408000, // Complete 8 MHz
	409000, // Complete 8 MHz
	410000, // Complete 8 MHz
	411000, // Complete 8 MHz
	412000, // Complete 8 MHz
	413000, // Complete 8 MHz
	414000, // Complete 8 MHz
	415000, // Complete 8 MHz
	416000, // Complete 8 MHz
	417000, // Complete 8 MHz
	418000, // Complete 8 MHz
	419000, // Complete 8 MHz
	420000, // Complete 8 MHz
	421000, // Complete 8 MHz
	422000, // Complete 8 MHz
	423000, // Complete 8 MHz
	424000, // Complete 8 MHz
	425000, // Complete 8 MHz
	426000, // Complete 8 MHz
	427000, // Complete 8 MHz
	428000, // Complete 8 MHz
	429000, // Complete 8 MHz
	430000, // Complete 8 MHz
	431000, // Complete 8 MHz
	432000, // Complete 8 MHz
	433000, // Complete 8 MHz
	434000, // Complete 8 MHz
	435000, // Complete 8 MHz
	436000, // Complete 8 MHz
	437000, // Complete 8 MHz
	438000, // Complete 8 MHz
	439000, // Complete 8 MHz
	440000, // Complete 8 MHz
	441000, // Complete 8 MHz
	442000, // Complete 8 MHz
	443000, // Complete 8 MHz
	444000, // Complete 8 MHz
	445000, // Complete 8 MHz
	446000, // Complete 8 MHz
	447000, // Complete 8 MHz
	448000, // Complete 8 MHz
	449000, // Complete 8 MHz
	450000, // Complete 8 MHz
	451000, // Complete 8 MHz
	452000, // Complete 8 MHz
	453000, // Complete 8 MHz
	454000, // Complete 8 MHz
	455000, // Complete 8 MHz
	456000, // Complete 8 MHz
	457000, // Complete 8 MHz
	458000, // Complete 8 MHz
	459000, // Complete 8 MHz
	460000, // Complete 8 MHz
	461000, // Complete 8 MHz
	462000, // Complete 8 MHz
	463000, // Complete 8 MHz
	464000, // Complete 8 MHz
	465000, // Complete 8 MHz
	466000, // Complete 8 MHz
	467000, // Complete 8 MHz
	468000, // Complete 8 MHz
	469000, // Complete 8 MHz
	470000, // Complete 8 MHz
	471000, // Complete 8 MHz
	472000, // Complete 8 MHz
	473000, // Complete 8 MHz
	474000, // Complete 8 MHz
	475000, // Complete 8 MHz
	476000, // Complete 8 MHz
	477000, // Complete 8 MHz
	478000, // Complete 8 MHz
	479000, // Complete 8 MHz
	480000, // Complete 8 MHz
	481000, // Complete 8 MHz
	482000, // Complete 8 MHz
	483000, // Complete 8 MHz
	484000, // Complete 8 MHz
	485000, // Complete 8 MHz
	486000, // Complete 8 MHz
	487000, // Complete 8 MHz
	488000, // Complete 8 MHz
	489000, // Complete 8 MHz
	490000, // Complete 8 MHz
	491000, // Complete 8 MHz
	492000, // Complete 8 MHz
	493000, // Complete 8 MHz
	494000, // Complete 8 MHz
	495000, // Complete 8 MHz
	496000, // Complete 8 MHz
	497000, // Complete 8 MHz
	498000, // Complete 8 MHz
	499000, // Complete 8 MHz
	500000, // Complete 8 MHz
	501000, // Complete 8 MHz
	502000, // Complete 8 MHz
	503000, // Complete 8 MHz
	504000, // Complete 8 MHz
	505000, // Complete 8 MHz
	506000, // Complete 8 MHz
	507000, // Complete 8 MHz
	508000, // Complete 8 MHz
	509000, // Complete 8 MHz
	510000, // Complete 8 MHz
	511000, // Complete 8 MHz
	512000, // Complete 8 MHz
	513000, // Complete 8 MHz
	514000, // Complete 8 MHz
	515000, // Complete 8 MHz
	516000, // Complete 8 MHz
	517000, // Complete 8 MHz
	518000, // Complete 8 MHz
	519000, // Complete 8 MHz
	520000, // Complete 8 MHz
	521000, // Complete 8 MHz
	522000, // Complete 8 MHz
	523000, // Complete 8 MHz
	524000, // Complete 8 MHz
	525000, // Complete 8 MHz
	526000, // Complete 8 MHz
	527000, // Complete 8 MHz
	528000, // Complete 8 MHz
	529000, // Complete 8 MHz
	530000, // Complete 8 MHz
	531000, // Complete 8 MHz
	532000, // Complete 8 MHz
	533000, // Complete 8 MHz
	534000, // Complete 8 MHz
	535000, // Complete 8 MHz
	536000, // Complete 8 MHz
	537000, // Complete 8 MHz
	538000, // Complete 8 MHz
	539000, // Complete 8 MHz
	540000, // Complete 8 MHz
	541000, // Complete 8 MHz
	542000, // Complete 8 MHz
	543000, // Complete 8 MHz
	544000, // Complete 8 MHz
	545000, // Complete 8 MHz
	546000, // Complete 8 MHz
	547000, // Complete 8 MHz
	548000, // Complete 8 MHz
	549000, // Complete 8 MHz
	550000, // Complete 8 MHz
	551000, // Complete 8 MHz
	552000, // Complete 8 MHz
	553000, // Complete 8 MHz
	554000, // Complete 8 MHz
	555000, // Complete 8 MHz
	556000, // Complete 8 MHz
	557000, // Complete 8 MHz
	558000, // Complete 8 MHz
	559000, // Complete 8 MHz
	560000, // Complete 8 MHz
	561000, // Complete 8 MHz
	562000, // Complete 8 MHz
	563000, // Complete 8 MHz
	564000, // Complete 8 MHz
	565000, // Complete 8 MHz
	566000, // Complete 8 MHz
	567000, // Complete 8 MHz
	568000, // Complete 8 MHz
	569000, // Complete 8 MHz
	570000, // Complete 8 MHz
	571000, // Complete 8 MHz
	572000, // Complete 8 MHz
	573000, // Complete 8 MHz
	574000, // Complete 8 MHz
	575000, // Complete 8 MHz
	576000, // Complete 8 MHz
	577000, // Complete 8 MHz
	578000, // Complete 8 MHz
	579000, // Complete 8 MHz
	580000, // Complete 8 MHz
	581000, // Complete 8 MHz
	582000, // Complete 8 MHz
	583000, // Complete 8 MHz
	584000, // Complete 8 MHz
	585000, // Complete 8 MHz
	586000, // Complete 8 MHz
	587000, // Complete 8 MHz
	588000, // Complete 8 MHz
	589000, // Complete 8 MHz
	590000, // Complete 8 MHz
	591000, // Complete 8 MHz
	592000, // Complete 8 MHz
	593000, // Complete 8 MHz
	594000, // Complete 8 MHz
	595000, // Complete 8 MHz
	596000, // Complete 8 MHz
	597000, // Complete 8 MHz
	598000, // Complete 8 MHz
	599000, // Complete 8 MHz
	600000, // Complete 8 MHz
	601000, // Complete 8 MHz
	602000, // Complete 8 MHz
	603000, // Complete 8 MHz
	604000, // Complete 8 MHz
	605000, // Complete 8 MHz
	606000, // Complete 8 MHz
	607000, // Complete 8 MHz
	608000, // Complete 8 MHz
	609000, // Complete 8 MHz
	610000, // Complete 8 MHz
	611000, // Complete 8 MHz
	612000, // Complete 8 MHz
	613000, // Complete 8 MHz
	614000, // Complete 8 MHz
	615000, // Complete 8 MHz
	616000, // Complete 8 MHz
	617000, // Complete 8 MHz
	618000, // Complete 8 MHz
	619000, // Complete 8 MHz
	620000, // Complete 8 MHz
	621000, // Complete 8 MHz
	622000, // Complete 8 MHz
	623000, // Complete 8 MHz
	624000, // Complete 8 MHz
	625000, // Complete 8 MHz
	626000, // Complete 8 MHz
	627000, // Complete 8 MHz
	628000, // Complete 8 MHz
	629000, // Complete 8 MHz
	630000, // Complete 8 MHz
	631000, // Complete 8 MHz
	632000, // Complete 8 MHz
	633000, // Complete 8 MHz
	634000, // Complete 8 MHz
	635000, // Complete 8 MHz
	636000, // Complete 8 MHz
	637000, // Complete 8 MHz
	638000, // Complete 8 MHz
	639000, // Complete 8 MHz
	640000, // Complete 8 MHz
	641000, // Complete 8 MHz
	642000, // Complete 8 MHz
	643000, // Complete 8 MHz
	644000, // Complete 8 MHz
	645000, // Complete 8 MHz
	646000, // Complete 8 MHz
	647000, // Complete 8 MHz
	648000, // Complete 8 MHz
	649000, // Complete 8 MHz
	650000, // Complete 8 MHz
	651000, // Complete 8 MHz
	652000, // Complete 8 MHz
	653000, // Complete 8 MHz
	654000, // Complete 8 MHz
	655000, // Complete 8 MHz
	656000, // Complete 8 MHz
	657000, // Complete 8 MHz
	658000, // Complete 8 MHz
	659000, // Complete 8 MHz
	660000, // Complete 8 MHz
	661000, // Complete 8 MHz
	662000, // Complete 8 MHz
	663000, // Complete 8 MHz
	664000, // Complete 8 MHz
	665000, // Complete 8 MHz
	666000, // Complete 8 MHz
	667000, // Complete 8 MHz
	668000, // Complete 8 MHz
	669000, // Complete 8 MHz
	670000, // Complete 8 MHz
	671000, // Complete 8 MHz
	672000, // Complete 8 MHz
	673000, // Complete 8 MHz
	674000, // Complete 8 MHz
	675000, // Complete 8 MHz
	676000, // Complete 8 MHz
	677000, // Complete 8 MHz
	678000, // Complete 8 MHz
	679000, // Complete 8 MHz
	680000, // Complete 8 MHz
	681000, // Complete 8 MHz
	682000, // Complete 8 MHz
	683000, // Complete 8 MHz
	684000, // Complete 8 MHz
	685000, // Complete 8 MHz
	686000, // Complete 8 MHz
	687000, // Complete 8 MHz
	688000, // Complete 8 MHz
	689000, // Complete 8 MHz
	690000, // Complete 8 MHz
	691000, // Complete 8 MHz
	692000, // Complete 8 MHz
	693000, // Complete 8 MHz
	694000, // Complete 8 MHz
	695000, // Complete 8 MHz
	696000, // Complete 8 MHz
	697000, // Complete 8 MHz
	698000, // Complete 8 MHz
	699000, // Complete 8 MHz
	700000, // Complete 8 MHz
	701000, // Complete 8 MHz
	702000, // Complete 8 MHz
	703000, // Complete 8 MHz
	704000, // Complete 8 MHz
	705000, // Complete 8 MHz
	706000, // Complete 8 MHz
	707000, // Complete 8 MHz
	708000, // Complete 8 MHz
	709000, // Complete 8 MHz
	710000, // Complete 8 MHz
	711000, // Complete 8 MHz
	712000, // Complete 8 MHz
	713000, // Complete 8 MHz
	714000, // Complete 8 MHz
	715000, // Complete 8 MHz
	716000, // Complete 8 MHz
	717000, // Complete 8 MHz
	718000, // Complete 8 MHz
	719000, // Complete 8 MHz
	720000, // Complete 8 MHz
	721000, // Complete 8 MHz
	722000, // Complete 8 MHz
	723000, // Complete 8 MHz
	724000, // Complete 8 MHz
	725000, // Complete 8 MHz
	726000, // Complete 8 MHz
	727000, // Complete 8 MHz
	728000, // Complete 8 MHz
	729000, // Complete 8 MHz
	730000, // Complete 8 MHz
	731000, // Complete 8 MHz
	732000, // Complete 8 MHz
	733000, // Complete 8 MHz
	734000, // Complete 8 MHz
	735000, // Complete 8 MHz
	736000, // Complete 8 MHz
	737000, // Complete 8 MHz
	738000, // Complete 8 MHz
	739000, // Complete 8 MHz
	740000, // Complete 8 MHz
	741000, // Complete 8 MHz
	742000, // Complete 8 MHz
	743000, // Complete 8 MHz
	744000, // Complete 8 MHz
	745000, // Complete 8 MHz
	746000, // Complete 8 MHz
	747000, // Complete 8 MHz
	748000, // Complete 8 MHz
	749000, // Complete 8 MHz
	750000, // Complete 8 MHz
	751000, // Complete 8 MHz
	752000, // Complete 8 MHz
	753000, // Complete 8 MHz
	754000, // Complete 8 MHz
	755000, // Complete 8 MHz
	756000, // Complete 8 MHz
	757000, // Complete 8 MHz
	758000, // Complete 8 MHz
	759000, // Complete 8 MHz
	760000, // Complete 8 MHz
	761000, // Complete 8 MHz
	762000, // Complete 8 MHz
	763000, // Complete 8 MHz
	764000, // Complete 8 MHz
	765000, // Complete 8 MHz
	766000, // Complete 8 MHz
	767000, // Complete 8 MHz
	768000, // Complete 8 MHz
	769000, // Complete 8 MHz
	770000, // Complete 8 MHz
	771000, // Complete 8 MHz
	772000, // Complete 8 MHz
	773000, // Complete 8 MHz
	774000, // Complete 8 MHz
	775000, // Complete 8 MHz
	776000, // Complete 8 MHz
	777000, // Complete 8 MHz
	778000, // Complete 8 MHz
	779000, // Complete 8 MHz
	780000, // Complete 8 MHz
	781000, // Complete 8 MHz
	782000, // Complete 8 MHz
	783000, // Complete 8 MHz
	784000, // Complete 8 MHz
	785000, // Complete 8 MHz
	786000, // Complete 8 MHz
	787000, // Complete 8 MHz
	788000, // Complete 8 MHz
	789000, // Complete 8 MHz
	790000, // Complete 8 MHz
	791000, // Complete 8 MHz
	792000, // Complete 8 MHz
	793000, // Complete 8 MHz
	794000, // Complete 8 MHz
	795000, // Complete 8 MHz
	796000, // Complete 8 MHz
	797000, // Complete 8 MHz
	798000, // Complete 8 MHz
	799000, // Complete 8 MHz
	800000, // Complete 8 MHz
	801000, // Complete 8 MHz
	802000, // Complete 8 MHz
	803000, // Complete 8 MHz
	804000, // Complete 8 MHz
	805000, // Complete 8 MHz
	806000, // Complete 8 MHz
	807000, // Complete 8 MHz
	808000, // Complete 8 MHz
	809000, // Complete 8 MHz
	810000, // Complete 8 MHz
	811000, // Complete 8 MHz
	812000, // Complete 8 MHz
	813000, // Complete 8 MHz
	814000, // Complete 8 MHz
	815000, // Complete 8 MHz
	816000, // Complete 8 MHz
	817000, // Complete 8 MHz
	818000, // Complete 8 MHz
	819000, // Complete 8 MHz
	820000, // Complete 8 MHz
	821000, // Complete 8 MHz
	822000, // Complete 8 MHz
	823000, // Complete 8 MHz
	824000, // Complete 8 MHz
	825000, // Complete 8 MHz
	826000, // Complete 8 MHz
	827000, // Complete 8 MHz
	828000, // Complete 8 MHz
	829000, // Complete 8 MHz
	830000, // Complete 8 MHz
	831000, // Complete 8 MHz
	832000, // Complete 8 MHz
	833000, // Complete 8 MHz
	834000, // Complete 8 MHz
	835000, // Complete 8 MHz
	836000, // Complete 8 MHz
	837000, // Complete 8 MHz
	838000, // Complete 8 MHz
	839000, // Complete 8 MHz
	840000, // Complete 8 MHz
	841000, // Complete 8 MHz
	842000, // Complete 8 MHz
	843000, // Complete 8 MHz
	844000, // Complete 8 MHz
	845000, // Complete 8 MHz
	846000, // Complete 8 MHz
	847000, // Complete 8 MHz
	848000, // Complete 8 MHz
	849000, // Complete 8 MHz
	850000, // Complete 8 MHz
	851000, // Complete 8 MHz
	852000, // Complete 8 MHz
	853000, // Complete 8 MHz
	854000, // Complete 8 MHz
	855000, // Complete 8 MHz
	856000, // Complete 8 MHz
	857000, // Complete 8 MHz
	858000, // Complete 8 MHz
	0,		// ******************************* end of Complete 8 MHz
	111000, // Complete 7/8 MHz
	112000, // Complete 7/8 MHz
	113000, // Complete 7/8 MHz
	114000, // Complete 7/8 MHz
	115000, // Complete 7/8 MHz
	116000, // Complete 7/8 MHz
	117000, // Complete 7/8 MHz
	118000, // Complete 7/8 MHz
	119000, // Complete 7/8 MHz
	120000, // Complete 7/8 MHz
	121000, // Complete 7/8 MHz
	122000, // Complete 7/8 MHz
	123000, // Complete 7/8 MHz
	124000, // Complete 7/8 MHz
	125000, // Complete 7/8 MHz
	126000, // Complete 7/8 MHz
	127000, // Complete 7/8 MHz
	128000, // Complete 7/8 MHz
	129000, // Complete 7/8 MHz
	130000, // Complete 7/8 MHz
	131000, // Complete 7/8 MHz
	132000, // Complete 7/8 MHz
	133000, // Complete 7/8 MHz
	134000, // Complete 7/8 MHz
	135000, // Complete 7/8 MHz
	136000, // Complete 7/8 MHz
	137000, // Complete 7/8 MHz
	138000, // Complete 7/8 MHz
	139000, // Complete 7/8 MHz
	140000, // Complete 7/8 MHz
	141000, // Complete 7/8 MHz
	142000, // Complete 7/8 MHz
	143000, // Complete 7/8 MHz
	144000, // Complete 7/8 MHz
	145000, // Complete 7/8 MHz
	146000, // Complete 7/8 MHz
	147000, // Complete 7/8 MHz
	148000, // Complete 7/8 MHz
	149000, // Complete 7/8 MHz
	150000, // Complete 7/8 MHz
	151000, // Complete 7/8 MHz
	152000, // Complete 7/8 MHz
	153000, // Complete 7/8 MHz
	154000, // Complete 7/8 MHz
	155000, // Complete 7/8 MHz
	156000, // Complete 7/8 MHz
	157000, // Complete 7/8 MHz
	158000, // Complete 7/8 MHz
	159000, // Complete 7/8 MHz
	160000, // Complete 7/8 MHz
	161000, // Complete 7/8 MHz
	162000, // Complete 7/8 MHz
	163000, // Complete 7/8 MHz
	164000, // Complete 7/8 MHz
	165000, // Complete 7/8 MHz
	166000, // Complete 7/8 MHz
	167000, // Complete 7/8 MHz
	168000, // Complete 7/8 MHz
	169000, // Complete 7/8 MHz
	170000, // Complete 7/8 MHz
	171000, // Complete 7/8 MHz
	172000, // Complete 7/8 MHz
	173000, // Complete 7/8 MHz
	174000, // Complete 7/8 MHz
	175000, // Complete 7/8 MHz
	176000, // Complete 7/8 MHz
	177000, // Complete 7/8 MHz
	178000, // Complete 7/8 MHz
	179000, // Complete 7/8 MHz
	180000, // Complete 7/8 MHz
	181000, // Complete 7/8 MHz
	182000, // Complete 7/8 MHz
	183000, // Complete 7/8 MHz
	184000, // Complete 7/8 MHz
	185000, // Complete 7/8 MHz
	186000, // Complete 7/8 MHz
	187000, // Complete 7/8 MHz
	188000, // Complete 7/8 MHz
	189000, // Complete 7/8 MHz
	190000, // Complete 7/8 MHz
	191000, // Complete 7/8 MHz
	192000, // Complete 7/8 MHz
	193000, // Complete 7/8 MHz
	194000, // Complete 7/8 MHz
	195000, // Complete 7/8 MHz
	196000, // Complete 7/8 MHz
	197000, // Complete 7/8 MHz
	198000, // Complete 7/8 MHz
	199000, // Complete 7/8 MHz
	200000, // Complete 7/8 MHz
	201000, // Complete 7/8 MHz
	202000, // Complete 7/8 MHz
	203000, // Complete 7/8 MHz
	204000, // Complete 7/8 MHz
	205000, // Complete 7/8 MHz
	206000, // Complete 7/8 MHz
	207000, // Complete 7/8 MHz
	208000, // Complete 7/8 MHz
	209000, // Complete 7/8 MHz
	210000, // Complete 7/8 MHz
	211000, // Complete 7/8 MHz
	212000, // Complete 7/8 MHz
	213000, // Complete 7/8 MHz
	214000, // Complete 7/8 MHz
	215000, // Complete 7/8 MHz
	216000, // Complete 7/8 MHz
	217000, // Complete 7/8 MHz
	218000, // Complete 7/8 MHz
	219000, // Complete 7/8 MHz
	220000, // Complete 7/8 MHz
	221000, // Complete 7/8 MHz
	222000, // Complete 7/8 MHz
	223000, // Complete 7/8 MHz
	224000, // Complete 7/8 MHz
	225000, // Complete 7/8 MHz
	226000, // Complete 7/8 MHz
	227000, // Complete 7/8 MHz
	228000, // Complete 7/8 MHz
	229000, // Complete 7/8 MHz
	230000, // Complete 7/8 MHz
	231000, // Complete 7/8 MHz
	232000, // Complete 7/8 MHz
	233000, // Complete 7/8 MHz
	234000, // Complete 7/8 MHz
	235000, // Complete 7/8 MHz
	236000, // Complete 7/8 MHz
	237000, // Complete 7/8 MHz
	238000, // Complete 7/8 MHz
	239000, // Complete 7/8 MHz
	240000, // Complete 7/8 MHz
	241000, // Complete 7/8 MHz
	242000, // Complete 7/8 MHz
	243000, // Complete 7/8 MHz
	244000, // Complete 7/8 MHz
	245000, // Complete 7/8 MHz
	246000, // Complete 7/8 MHz
	247000, // Complete 7/8 MHz
	248000, // Complete 7/8 MHz
	249000, // Complete 7/8 MHz
	250000, // Complete 7/8 MHz
	251000, // Complete 7/8 MHz
	252000, // Complete 7/8 MHz
	253000, // Complete 7/8 MHz
	254000, // Complete 7/8 MHz
	255000, // Complete 7/8 MHz
	256000, // Complete 7/8 MHz
	257000, // Complete 7/8 MHz
	258000, // Complete 7/8 MHz
	259000, // Complete 7/8 MHz
	260000, // Complete 7/8 MHz
	261000, // Complete 7/8 MHz
	262000, // Complete 7/8 MHz
	263000, // Complete 7/8 MHz
	264000, // Complete 7/8 MHz
	265000, // Complete 7/8 MHz
	266000, // Complete 7/8 MHz
	267000, // Complete 7/8 MHz
	268000, // Complete 7/8 MHz
	269000, // Complete 7/8 MHz
	270000, // Complete 7/8 MHz
	271000, // Complete 7/8 MHz
	272000, // Complete 7/8 MHz
	273000, // Complete 7/8 MHz
	274000, // Complete 7/8 MHz
	275000, // Complete 7/8 MHz
	276000, // Complete 7/8 MHz
	277000, // Complete 7/8 MHz
	278000, // Complete 7/8 MHz
	279000, // Complete 7/8 MHz
	280000, // Complete 7/8 MHz
	281000, // Complete 7/8 MHz
	282000, // Complete 7/8 MHz
	283000, // Complete 7/8 MHz
	284000, // Complete 7/8 MHz
	285000, // Complete 7/8 MHz
	286000, // Complete 7/8 MHz
	287000, // Complete 7/8 MHz
	288000, // Complete 7/8 MHz
	289000, // Complete 7/8 MHz
	290000, // Complete 7/8 MHz
	291000, // Complete 7/8 MHz
	292000, // Complete 7/8 MHz
	293000, // Complete 7/8 MHz
	294000, // Complete 7/8 MHz
	295000, // Complete 7/8 MHz
	296000, // Complete 7/8 MHz
	297000, // Complete 7/8 MHz
	298000, // Complete 7/8 MHz
	299000, // Complete 7/8 MHz
	300000, // Complete 7/8 MHz
	301000, // Complete 7/8 MHz
	302000, // Complete 7/8 MHz
	303000, // Complete 7/8 MHz
	304000, // Complete 7/8 MHz
	305000, // Complete 7/8 MHz
	306000, // Complete 7/8 MHz
	307000, // Complete 7/8 MHz
	308000, // Complete 7/8 MHz
	309000, // Complete 7/8 MHz
	310000, // Complete 7/8 MHz
	311000, // Complete 7/8 MHz
	312000, // Complete 7/8 MHz
	313000, // Complete 7/8 MHz
	314000, // Complete 7/8 MHz
	315000, // Complete 7/8 MHz
	316000, // Complete 7/8 MHz
	317000, // Complete 7/8 MHz
	318000, // Complete 7/8 MHz
	319000, // Complete 7/8 MHz
	320000, // Complete 7/8 MHz
	321000, // Complete 7/8 MHz
	322000, // Complete 7/8 MHz
	323000, // Complete 7/8 MHz
	324000, // Complete 7/8 MHz
	325000, // Complete 7/8 MHz
	326000, // Complete 7/8 MHz
	327000, // Complete 7/8 MHz
	328000, // Complete 7/8 MHz
	329000, // Complete 7/8 MHz
	330000, // Complete 7/8 MHz
	331000, // Complete 7/8 MHz
	332000, // Complete 7/8 MHz
	333000, // Complete 7/8 MHz
	334000, // Complete 7/8 MHz
	335000, // Complete 7/8 MHz
	336000, // Complete 7/8 MHz
	337000, // Complete 7/8 MHz
	338000, // Complete 7/8 MHz
	339000, // Complete 7/8 MHz
	340000, // Complete 7/8 MHz
	341000, // Complete 7/8 MHz
	342000, // Complete 7/8 MHz
	343000, // Complete 7/8 MHz
	344000, // Complete 7/8 MHz
	345000, // Complete 7/8 MHz
	346000, // Complete 7/8 MHz
	347000, // Complete 7/8 MHz
	348000, // Complete 7/8 MHz
	349000, // Complete 7/8 MHz
	350000, // Complete 7/8 MHz
	351000, // Complete 7/8 MHz
	352000, // Complete 7/8 MHz
	353000, // Complete 7/8 MHz
	354000, // Complete 7/8 MHz
	355000, // Complete 7/8 MHz
	356000, // Complete 7/8 MHz
	357000, // Complete 7/8 MHz
	358000, // Complete 7/8 MHz
	359000, // Complete 7/8 MHz
	360000, // Complete 7/8 MHz
	361000, // Complete 7/8 MHz
	362000, // Complete 7/8 MHz
	363000, // Complete 7/8 MHz
	364000, // Complete 7/8 MHz
	365000, // Complete 7/8 MHz
	366000, // Complete 7/8 MHz
	367000, // Complete 7/8 MHz
	368000, // Complete 7/8 MHz
	369000, // Complete 7/8 MHz
	370000, // Complete 7/8 MHz
	371000, // Complete 7/8 MHz
	372000, // Complete 7/8 MHz
	373000, // Complete 7/8 MHz
	374000, // Complete 7/8 MHz
	375000, // Complete 7/8 MHz
	376000, // Complete 7/8 MHz
	377000, // Complete 7/8 MHz
	378000, // Complete 7/8 MHz
	379000, // Complete 7/8 MHz
	380000, // Complete 7/8 MHz
	381000, // Complete 7/8 MHz
	382000, // Complete 7/8 MHz
	383000, // Complete 7/8 MHz
	384000, // Complete 7/8 MHz
	385000, // Complete 7/8 MHz
	386000, // Complete 7/8 MHz
	387000, // Complete 7/8 MHz
	388000, // Complete 7/8 MHz
	389000, // Complete 7/8 MHz
	390000, // Complete 7/8 MHz
	391000, // Complete 7/8 MHz
	392000, // Complete 7/8 MHz
	393000, // Complete 7/8 MHz
	394000, // Complete 7/8 MHz
	395000, // Complete 7/8 MHz
	396000, // Complete 7/8 MHz
	397000, // Complete 7/8 MHz
	398000, // Complete 7/8 MHz
	399000, // Complete 7/8 MHz
	400000, // Complete 7/8 MHz
	401000, // Complete 7/8 MHz
	402000, // Complete 7/8 MHz
	403000, // Complete 7/8 MHz
	404000, // Complete 7/8 MHz
	405000, // Complete 7/8 MHz
	406000, // Complete 7/8 MHz
	407000, // Complete 7/8 MHz
	408000, // Complete 7/8 MHz
	409000, // Complete 7/8 MHz
	410000, // Complete 7/8 MHz
	411000, // Complete 7/8 MHz
	412000, // Complete 7/8 MHz
	413000, // Complete 7/8 MHz
	414000, // Complete 7/8 MHz
	415000, // Complete 7/8 MHz
	416000, // Complete 7/8 MHz
	417000, // Complete 7/8 MHz
	418000, // Complete 7/8 MHz
	419000, // Complete 7/8 MHz
	420000, // Complete 7/8 MHz
	421000, // Complete 7/8 MHz
	422000, // Complete 7/8 MHz
	423000, // Complete 7/8 MHz
	424000, // Complete 7/8 MHz
	425000, // Complete 7/8 MHz
	426000, // Complete 7/8 MHz
	427000, // Complete 7/8 MHz
	428000, // Complete 7/8 MHz
	429000, // Complete 7/8 MHz
	430000, // Complete 7/8 MHz
	431000, // Complete 7/8 MHz
	432000, // Complete 7/8 MHz
	433000, // Complete 7/8 MHz
	434000, // Complete 7/8 MHz
	435000, // Complete 7/8 MHz
	436000, // Complete 7/8 MHz
	437000, // Complete 7/8 MHz
	438000, // Complete 7/8 MHz
	439000, // Complete 7/8 MHz
	440000, // Complete 7/8 MHz
	441000, // Complete 7/8 MHz
	442000, // Complete 7/8 MHz
	443000, // Complete 7/8 MHz
	444000, // Complete 7/8 MHz
	445000, // Complete 7/8 MHz
	446000, // Complete 7/8 MHz
	447000, // Complete 7/8 MHz
	448000, // Complete 7/8 MHz
	449000, // Complete 7/8 MHz
	450000, // Complete 7/8 MHz
	451000, // Complete 7/8 MHz
	452000, // Complete 7/8 MHz
	453000, // Complete 7/8 MHz
	454000, // Complete 7/8 MHz
	455000, // Complete 7/8 MHz
	456000, // Complete 7/8 MHz
	457000, // Complete 7/8 MHz
	458000, // Complete 7/8 MHz
	459000, // Complete 7/8 MHz
	460000, // Complete 7/8 MHz
	461000, // Complete 7/8 MHz
	462000, // Complete 7/8 MHz
	463000, // Complete 7/8 MHz
	464000, // Complete 7/8 MHz
	465000, // Complete 7/8 MHz
	466000, // Complete 7/8 MHz
	467000, // Complete 7/8 MHz
	468000, // Complete 7/8 MHz
	469000, // Complete 7/8 MHz
	470000, // Complete 7/8 MHz
	471000, // Complete 7/8 MHz
	472000, // Complete 7/8 MHz
	473000, // Complete 7/8 MHz
	474000, // Complete 7/8 MHz
	475000, // Complete 7/8 MHz
	476000, // Complete 7/8 MHz
	477000, // Complete 7/8 MHz
	478000, // Complete 7/8 MHz
	479000, // Complete 7/8 MHz
	480000, // Complete 7/8 MHz
	481000, // Complete 7/8 MHz
	482000, // Complete 7/8 MHz
	483000, // Complete 7/8 MHz
	484000, // Complete 7/8 MHz
	485000, // Complete 7/8 MHz
	486000, // Complete 7/8 MHz
	487000, // Complete 7/8 MHz
	488000, // Complete 7/8 MHz
	489000, // Complete 7/8 MHz
	490000, // Complete 7/8 MHz
	491000, // Complete 7/8 MHz
	492000, // Complete 7/8 MHz
	493000, // Complete 7/8 MHz
	494000, // Complete 7/8 MHz
	495000, // Complete 7/8 MHz
	496000, // Complete 7/8 MHz
	497000, // Complete 7/8 MHz
	498000, // Complete 7/8 MHz
	499000, // Complete 7/8 MHz
	500000, // Complete 7/8 MHz
	501000, // Complete 7/8 MHz
	502000, // Complete 7/8 MHz
	503000, // Complete 7/8 MHz
	504000, // Complete 7/8 MHz
	505000, // Complete 7/8 MHz
	506000, // Complete 7/8 MHz
	507000, // Complete 7/8 MHz
	508000, // Complete 7/8 MHz
	509000, // Complete 7/8 MHz
	510000, // Complete 7/8 MHz
	511000, // Complete 7/8 MHz
	512000, // Complete 7/8 MHz
	513000, // Complete 7/8 MHz
	514000, // Complete 7/8 MHz
	515000, // Complete 7/8 MHz
	516000, // Complete 7/8 MHz
	517000, // Complete 7/8 MHz
	518000, // Complete 7/8 MHz
	519000, // Complete 7/8 MHz
	520000, // Complete 7/8 MHz
	521000, // Complete 7/8 MHz
	522000, // Complete 7/8 MHz
	523000, // Complete 7/8 MHz
	524000, // Complete 7/8 MHz
	525000, // Complete 7/8 MHz
	526000, // Complete 7/8 MHz
	527000, // Complete 7/8 MHz
	528000, // Complete 7/8 MHz
	529000, // Complete 7/8 MHz
	530000, // Complete 7/8 MHz
	531000, // Complete 7/8 MHz
	532000, // Complete 7/8 MHz
	533000, // Complete 7/8 MHz
	534000, // Complete 7/8 MHz
	535000, // Complete 7/8 MHz
	536000, // Complete 7/8 MHz
	537000, // Complete 7/8 MHz
	538000, // Complete 7/8 MHz
	539000, // Complete 7/8 MHz
	540000, // Complete 7/8 MHz
	541000, // Complete 7/8 MHz
	542000, // Complete 7/8 MHz
	543000, // Complete 7/8 MHz
	544000, // Complete 7/8 MHz
	545000, // Complete 7/8 MHz
	546000, // Complete 7/8 MHz
	547000, // Complete 7/8 MHz
	548000, // Complete 7/8 MHz
	549000, // Complete 7/8 MHz
	550000, // Complete 7/8 MHz
	551000, // Complete 7/8 MHz
	552000, // Complete 7/8 MHz
	553000, // Complete 7/8 MHz
	554000, // Complete 7/8 MHz
	555000, // Complete 7/8 MHz
	556000, // Complete 7/8 MHz
	557000, // Complete 7/8 MHz
	558000, // Complete 7/8 MHz
	559000, // Complete 7/8 MHz
	560000, // Complete 7/8 MHz
	561000, // Complete 7/8 MHz
	562000, // Complete 7/8 MHz
	563000, // Complete 7/8 MHz
	564000, // Complete 7/8 MHz
	565000, // Complete 7/8 MHz
	566000, // Complete 7/8 MHz
	567000, // Complete 7/8 MHz
	568000, // Complete 7/8 MHz
	569000, // Complete 7/8 MHz
	570000, // Complete 7/8 MHz
	571000, // Complete 7/8 MHz
	572000, // Complete 7/8 MHz
	573000, // Complete 7/8 MHz
	574000, // Complete 7/8 MHz
	575000, // Complete 7/8 MHz
	576000, // Complete 7/8 MHz
	577000, // Complete 7/8 MHz
	578000, // Complete 7/8 MHz
	579000, // Complete 7/8 MHz
	580000, // Complete 7/8 MHz
	581000, // Complete 7/8 MHz
	582000, // Complete 7/8 MHz
	583000, // Complete 7/8 MHz
	584000, // Complete 7/8 MHz
	585000, // Complete 7/8 MHz
	586000, // Complete 7/8 MHz
	587000, // Complete 7/8 MHz
	588000, // Complete 7/8 MHz
	589000, // Complete 7/8 MHz
	590000, // Complete 7/8 MHz
	591000, // Complete 7/8 MHz
	592000, // Complete 7/8 MHz
	593000, // Complete 7/8 MHz
	594000, // Complete 7/8 MHz
	595000, // Complete 7/8 MHz
	596000, // Complete 7/8 MHz
	597000, // Complete 7/8 MHz
	598000, // Complete 7/8 MHz
	599000, // Complete 7/8 MHz
	600000, // Complete 7/8 MHz
	601000, // Complete 7/8 MHz
	602000, // Complete 7/8 MHz
	603000, // Complete 7/8 MHz
	604000, // Complete 7/8 MHz
	605000, // Complete 7/8 MHz
	606000, // Complete 7/8 MHz
	607000, // Complete 7/8 MHz
	608000, // Complete 7/8 MHz
	609000, // Complete 7/8 MHz
	610000, // Complete 7/8 MHz
	611000, // Complete 7/8 MHz
	612000, // Complete 7/8 MHz
	613000, // Complete 7/8 MHz
	614000, // Complete 7/8 MHz
	615000, // Complete 7/8 MHz
	616000, // Complete 7/8 MHz
	617000, // Complete 7/8 MHz
	618000, // Complete 7/8 MHz
	619000, // Complete 7/8 MHz
	620000, // Complete 7/8 MHz
	621000, // Complete 7/8 MHz
	622000, // Complete 7/8 MHz
	623000, // Complete 7/8 MHz
	624000, // Complete 7/8 MHz
	625000, // Complete 7/8 MHz
	626000, // Complete 7/8 MHz
	627000, // Complete 7/8 MHz
	628000, // Complete 7/8 MHz
	629000, // Complete 7/8 MHz
	630000, // Complete 7/8 MHz
	631000, // Complete 7/8 MHz
	632000, // Complete 7/8 MHz
	633000, // Complete 7/8 MHz
	634000, // Complete 7/8 MHz
	635000, // Complete 7/8 MHz
	636000, // Complete 7/8 MHz
	637000, // Complete 7/8 MHz
	638000, // Complete 7/8 MHz
	639000, // Complete 7/8 MHz
	640000, // Complete 7/8 MHz
	641000, // Complete 7/8 MHz
	642000, // Complete 7/8 MHz
	643000, // Complete 7/8 MHz
	644000, // Complete 7/8 MHz
	645000, // Complete 7/8 MHz
	646000, // Complete 7/8 MHz
	647000, // Complete 7/8 MHz
	648000, // Complete 7/8 MHz
	649000, // Complete 7/8 MHz
	650000, // Complete 7/8 MHz
	651000, // Complete 7/8 MHz
	652000, // Complete 7/8 MHz
	653000, // Complete 7/8 MHz
	654000, // Complete 7/8 MHz
	655000, // Complete 7/8 MHz
	656000, // Complete 7/8 MHz
	657000, // Complete 7/8 MHz
	658000, // Complete 7/8 MHz
	659000, // Complete 7/8 MHz
	660000, // Complete 7/8 MHz
	661000, // Complete 7/8 MHz
	662000, // Complete 7/8 MHz
	663000, // Complete 7/8 MHz
	664000, // Complete 7/8 MHz
	665000, // Complete 7/8 MHz
	666000, // Complete 7/8 MHz
	667000, // Complete 7/8 MHz
	668000, // Complete 7/8 MHz
	669000, // Complete 7/8 MHz
	670000, // Complete 7/8 MHz
	671000, // Complete 7/8 MHz
	672000, // Complete 7/8 MHz
	673000, // Complete 7/8 MHz
	674000, // Complete 7/8 MHz
	675000, // Complete 7/8 MHz
	676000, // Complete 7/8 MHz
	677000, // Complete 7/8 MHz
	678000, // Complete 7/8 MHz
	679000, // Complete 7/8 MHz
	680000, // Complete 7/8 MHz
	681000, // Complete 7/8 MHz
	682000, // Complete 7/8 MHz
	683000, // Complete 7/8 MHz
	684000, // Complete 7/8 MHz
	685000, // Complete 7/8 MHz
	686000, // Complete 7/8 MHz
	687000, // Complete 7/8 MHz
	688000, // Complete 7/8 MHz
	689000, // Complete 7/8 MHz
	690000, // Complete 7/8 MHz
	691000, // Complete 7/8 MHz
	692000, // Complete 7/8 MHz
	693000, // Complete 7/8 MHz
	694000, // Complete 7/8 MHz
	695000, // Complete 7/8 MHz
	696000, // Complete 7/8 MHz
	697000, // Complete 7/8 MHz
	698000, // Complete 7/8 MHz
	699000, // Complete 7/8 MHz
	700000, // Complete 7/8 MHz
	701000, // Complete 7/8 MHz
	702000, // Complete 7/8 MHz
	703000, // Complete 7/8 MHz
	704000, // Complete 7/8 MHz
	705000, // Complete 7/8 MHz
	706000, // Complete 7/8 MHz
	707000, // Complete 7/8 MHz
	708000, // Complete 7/8 MHz
	709000, // Complete 7/8 MHz
	710000, // Complete 7/8 MHz
	711000, // Complete 7/8 MHz
	712000, // Complete 7/8 MHz
	713000, // Complete 7/8 MHz
	714000, // Complete 7/8 MHz
	715000, // Complete 7/8 MHz
	716000, // Complete 7/8 MHz
	717000, // Complete 7/8 MHz
	718000, // Complete 7/8 MHz
	719000, // Complete 7/8 MHz
	720000, // Complete 7/8 MHz
	721000, // Complete 7/8 MHz
	722000, // Complete 7/8 MHz
	723000, // Complete 7/8 MHz
	724000, // Complete 7/8 MHz
	725000, // Complete 7/8 MHz
	726000, // Complete 7/8 MHz
	727000, // Complete 7/8 MHz
	728000, // Complete 7/8 MHz
	729000, // Complete 7/8 MHz
	730000, // Complete 7/8 MHz
	731000, // Complete 7/8 MHz
	732000, // Complete 7/8 MHz
	733000, // Complete 7/8 MHz
	734000, // Complete 7/8 MHz
	735000, // Complete 7/8 MHz
	736000, // Complete 7/8 MHz
	737000, // Complete 7/8 MHz
	738000, // Complete 7/8 MHz
	739000, // Complete 7/8 MHz
	740000, // Complete 7/8 MHz
	741000, // Complete 7/8 MHz
	742000, // Complete 7/8 MHz
	743000, // Complete 7/8 MHz
	744000, // Complete 7/8 MHz
	745000, // Complete 7/8 MHz
	746000, // Complete 7/8 MHz
	747000, // Complete 7/8 MHz
	748000, // Complete 7/8 MHz
	749000, // Complete 7/8 MHz
	750000, // Complete 7/8 MHz
	751000, // Complete 7/8 MHz
	752000, // Complete 7/8 MHz
	753000, // Complete 7/8 MHz
	754000, // Complete 7/8 MHz
	755000, // Complete 7/8 MHz
	756000, // Complete 7/8 MHz
	757000, // Complete 7/8 MHz
	758000, // Complete 7/8 MHz
	759000, // Complete 7/8 MHz
	760000, // Complete 7/8 MHz
	761000, // Complete 7/8 MHz
	762000, // Complete 7/8 MHz
	763000, // Complete 7/8 MHz
	764000, // Complete 7/8 MHz
	765000, // Complete 7/8 MHz
	766000, // Complete 7/8 MHz
	767000, // Complete 7/8 MHz
	768000, // Complete 7/8 MHz
	769000, // Complete 7/8 MHz
	770000, // Complete 7/8 MHz
	771000, // Complete 7/8 MHz
	772000, // Complete 7/8 MHz
	773000, // Complete 7/8 MHz
	774000, // Complete 7/8 MHz
	775000, // Complete 7/8 MHz
	776000, // Complete 7/8 MHz
	777000, // Complete 7/8 MHz
	778000, // Complete 7/8 MHz
	779000, // Complete 7/8 MHz
	780000, // Complete 7/8 MHz
	781000, // Complete 7/8 MHz
	782000, // Complete 7/8 MHz
	783000, // Complete 7/8 MHz
	784000, // Complete 7/8 MHz
	785000, // Complete 7/8 MHz
	786000, // Complete 7/8 MHz
	787000, // Complete 7/8 MHz
	788000, // Complete 7/8 MHz
	789000, // Complete 7/8 MHz
	790000, // Complete 7/8 MHz
	791000, // Complete 7/8 MHz
	792000, // Complete 7/8 MHz
	793000, // Complete 7/8 MHz
	794000, // Complete 7/8 MHz
	795000, // Complete 7/8 MHz
	796000, // Complete 7/8 MHz
	797000, // Complete 7/8 MHz
	798000, // Complete 7/8 MHz
	799000, // Complete 7/8 MHz
	800000, // Complete 7/8 MHz
	801000, // Complete 7/8 MHz
	802000, // Complete 7/8 MHz
	803000, // Complete 7/8 MHz
	804000, // Complete 7/8 MHz
	805000, // Complete 7/8 MHz
	806000, // Complete 7/8 MHz
	807000, // Complete 7/8 MHz
	808000, // Complete 7/8 MHz
	809000, // Complete 7/8 MHz
	810000, // Complete 7/8 MHz
	811000, // Complete 7/8 MHz
	812000, // Complete 7/8 MHz
	813000, // Complete 7/8 MHz
	814000, // Complete 7/8 MHz
	815000, // Complete 7/8 MHz
	816000, // Complete 7/8 MHz
	817000, // Complete 7/8 MHz
	818000, // Complete 7/8 MHz
	819000, // Complete 7/8 MHz
	820000, // Complete 7/8 MHz
	821000, // Complete 7/8 MHz
	822000, // Complete 7/8 MHz
	823000, // Complete 7/8 MHz
	824000, // Complete 7/8 MHz
	825000, // Complete 7/8 MHz
	826000, // Complete 7/8 MHz
	827000, // Complete 7/8 MHz
	828000, // Complete 7/8 MHz
	829000, // Complete 7/8 MHz
	830000, // Complete 7/8 MHz
	831000, // Complete 7/8 MHz
	832000, // Complete 7/8 MHz
	833000, // Complete 7/8 MHz
	834000, // Complete 7/8 MHz
	835000, // Complete 7/8 MHz
	836000, // Complete 7/8 MHz
	837000, // Complete 7/8 MHz
	838000, // Complete 7/8 MHz
	839000, // Complete 7/8 MHz
	840000, // Complete 7/8 MHz
	841000, // Complete 7/8 MHz
	842000, // Complete 7/8 MHz
	843000, // Complete 7/8 MHz
	844000, // Complete 7/8 MHz
	845000, // Complete 7/8 MHz
	846000, // Complete 7/8 MHz
	847000, // Complete 7/8 MHz
	848000, // Complete 7/8 MHz
	849000, // Complete 7/8 MHz
	850000, // Complete 7/8 MHz
	851000, // Complete 7/8 MHz
	852000, // Complete 7/8 MHz
	853000, // Complete 7/8 MHz
	854000, // Complete 7/8 MHz
	855000, // Complete 7/8 MHz
	856000, // Complete 7/8 MHz
	857000, // Complete 7/8 MHz
	858000, // Complete 7/8 MHz
	0,		// ******************************* end of Complete 7/8 MHz
	111000, // Complete 500k 7 MHz
	111500, // Complete 500k 7 MHz
	112000, // Complete 500k 7 MHz
	112500, // Complete 500k 7 MHz
	113000, // Complete 500k 7 MHz
	113500, // Complete 500k 7 MHz
	114000, // Complete 500k 7 MHz
	114500, // Complete 500k 7 MHz
	115000, // Complete 500k 7 MHz
	115500, // Complete 500k 7 MHz
	116000, // Complete 500k 7 MHz
	116500, // Complete 500k 7 MHz
	117000, // Complete 500k 7 MHz
	117500, // Complete 500k 7 MHz
	118000, // Complete 500k 7 MHz
	118500, // Complete 500k 7 MHz
	119000, // Complete 500k 7 MHz
	119500, // Complete 500k 7 MHz
	120000, // Complete 500k 7 MHz
	120500, // Complete 500k 7 MHz
	121000, // Complete 500k 7 MHz
	121500, // Complete 500k 7 MHz
	122000, // Complete 500k 7 MHz
	122500, // Complete 500k 7 MHz
	123000, // Complete 500k 7 MHz
	123500, // Complete 500k 7 MHz
	124000, // Complete 500k 7 MHz
	124500, // Complete 500k 7 MHz
	125000, // Complete 500k 7 MHz
	125500, // Complete 500k 7 MHz
	126000, // Complete 500k 7 MHz
	126500, // Complete 500k 7 MHz
	127000, // Complete 500k 7 MHz
	127500, // Complete 500k 7 MHz
	128000, // Complete 500k 7 MHz
	128500, // Complete 500k 7 MHz
	129000, // Complete 500k 7 MHz
	129500, // Complete 500k 7 MHz
	130000, // Complete 500k 7 MHz
	130500, // Complete 500k 7 MHz
	131000, // Complete 500k 7 MHz
	131500, // Complete 500k 7 MHz
	132000, // Complete 500k 7 MHz
	132500, // Complete 500k 7 MHz
	133000, // Complete 500k 7 MHz
	133500, // Complete 500k 7 MHz
	134000, // Complete 500k 7 MHz
	134500, // Complete 500k 7 MHz
	135000, // Complete 500k 7 MHz
	135500, // Complete 500k 7 MHz
	136000, // Complete 500k 7 MHz
	136500, // Complete 500k 7 MHz
	137000, // Complete 500k 7 MHz
	137500, // Complete 500k 7 MHz
	138000, // Complete 500k 7 MHz
	138500, // Complete 500k 7 MHz
	139000, // Complete 500k 7 MHz
	139500, // Complete 500k 7 MHz
	140000, // Complete 500k 7 MHz
	140500, // Complete 500k 7 MHz
	141000, // Complete 500k 7 MHz
	141500, // Complete 500k 7 MHz
	142000, // Complete 500k 7 MHz
	142500, // Complete 500k 7 MHz
	143000, // Complete 500k 7 MHz
	143500, // Complete 500k 7 MHz
	144000, // Complete 500k 7 MHz
	144500, // Complete 500k 7 MHz
	145000, // Complete 500k 7 MHz
	145500, // Complete 500k 7 MHz
	146000, // Complete 500k 7 MHz
	146500, // Complete 500k 7 MHz
	147000, // Complete 500k 7 MHz
	147500, // Complete 500k 7 MHz
	148000, // Complete 500k 7 MHz
	148500, // Complete 500k 7 MHz
	149000, // Complete 500k 7 MHz
	149500, // Complete 500k 7 MHz
	150000, // Complete 500k 7 MHz
	150500, // Complete 500k 7 MHz
	151000, // Complete 500k 7 MHz
	151500, // Complete 500k 7 MHz
	152000, // Complete 500k 7 MHz
	152500, // Complete 500k 7 MHz
	153000, // Complete 500k 7 MHz
	153500, // Complete 500k 7 MHz
	154000, // Complete 500k 7 MHz
	154500, // Complete 500k 7 MHz
	155000, // Complete 500k 7 MHz
	155500, // Complete 500k 7 MHz
	156000, // Complete 500k 7 MHz
	156500, // Complete 500k 7 MHz
	157000, // Complete 500k 7 MHz
	157500, // Complete 500k 7 MHz
	158000, // Complete 500k 7 MHz
	158500, // Complete 500k 7 MHz
	159000, // Complete 500k 7 MHz
	159500, // Complete 500k 7 MHz
	160000, // Complete 500k 7 MHz
	160500, // Complete 500k 7 MHz
	161000, // Complete 500k 7 MHz
	161500, // Complete 500k 7 MHz
	162000, // Complete 500k 7 MHz
	162500, // Complete 500k 7 MHz
	163000, // Complete 500k 7 MHz
	163500, // Complete 500k 7 MHz
	164000, // Complete 500k 7 MHz
	164500, // Complete 500k 7 MHz
	165000, // Complete 500k 7 MHz
	165500, // Complete 500k 7 MHz
	166000, // Complete 500k 7 MHz
	166500, // Complete 500k 7 MHz
	167000, // Complete 500k 7 MHz
	167500, // Complete 500k 7 MHz
	168000, // Complete 500k 7 MHz
	168500, // Complete 500k 7 MHz
	169000, // Complete 500k 7 MHz
	169500, // Complete 500k 7 MHz
	170000, // Complete 500k 7 MHz
	170500, // Complete 500k 7 MHz
	171000, // Complete 500k 7 MHz
	171500, // Complete 500k 7 MHz
	172000, // Complete 500k 7 MHz
	172500, // Complete 500k 7 MHz
	173000, // Complete 500k 7 MHz
	173500, // Complete 500k 7 MHz
	174000, // Complete 500k 7 MHz
	174500, // Complete 500k 7 MHz
	175000, // Complete 500k 7 MHz
	175500, // Complete 500k 7 MHz
	176000, // Complete 500k 7 MHz
	176500, // Complete 500k 7 MHz
	177000, // Complete 500k 7 MHz
	177500, // Complete 500k 7 MHz
	178000, // Complete 500k 7 MHz
	178500, // Complete 500k 7 MHz
	179000, // Complete 500k 7 MHz
	179500, // Complete 500k 7 MHz
	180000, // Complete 500k 7 MHz
	180500, // Complete 500k 7 MHz
	181000, // Complete 500k 7 MHz
	181500, // Complete 500k 7 MHz
	182000, // Complete 500k 7 MHz
	182500, // Complete 500k 7 MHz
	183000, // Complete 500k 7 MHz
	183500, // Complete 500k 7 MHz
	184000, // Complete 500k 7 MHz
	184500, // Complete 500k 7 MHz
	185000, // Complete 500k 7 MHz
	185500, // Complete 500k 7 MHz
	186000, // Complete 500k 7 MHz
	186500, // Complete 500k 7 MHz
	187000, // Complete 500k 7 MHz
	187500, // Complete 500k 7 MHz
	188000, // Complete 500k 7 MHz
	188500, // Complete 500k 7 MHz
	189000, // Complete 500k 7 MHz
	189500, // Complete 500k 7 MHz
	190000, // Complete 500k 7 MHz
	190500, // Complete 500k 7 MHz
	191000, // Complete 500k 7 MHz
	191500, // Complete 500k 7 MHz
	192000, // Complete 500k 7 MHz
	192500, // Complete 500k 7 MHz
	193000, // Complete 500k 7 MHz
	193500, // Complete 500k 7 MHz
	194000, // Complete 500k 7 MHz
	194500, // Complete 500k 7 MHz
	195000, // Complete 500k 7 MHz
	195500, // Complete 500k 7 MHz
	196000, // Complete 500k 7 MHz
	196500, // Complete 500k 7 MHz
	197000, // Complete 500k 7 MHz
	197500, // Complete 500k 7 MHz
	198000, // Complete 500k 7 MHz
	198500, // Complete 500k 7 MHz
	199000, // Complete 500k 7 MHz
	199500, // Complete 500k 7 MHz
	200000, // Complete 500k 7 MHz
	200500, // Complete 500k 7 MHz
	201000, // Complete 500k 7 MHz
	201500, // Complete 500k 7 MHz
	202000, // Complete 500k 7 MHz
	202500, // Complete 500k 7 MHz
	203000, // Complete 500k 7 MHz
	203500, // Complete 500k 7 MHz
	204000, // Complete 500k 7 MHz
	204500, // Complete 500k 7 MHz
	205000, // Complete 500k 7 MHz
	205500, // Complete 500k 7 MHz
	206000, // Complete 500k 7 MHz
	206500, // Complete 500k 7 MHz
	207000, // Complete 500k 7 MHz
	207500, // Complete 500k 7 MHz
	208000, // Complete 500k 7 MHz
	208500, // Complete 500k 7 MHz
	209000, // Complete 500k 7 MHz
	209500, // Complete 500k 7 MHz
	210000, // Complete 500k 7 MHz
	210500, // Complete 500k 7 MHz
	211000, // Complete 500k 7 MHz
	211500, // Complete 500k 7 MHz
	212000, // Complete 500k 7 MHz
	212500, // Complete 500k 7 MHz
	213000, // Complete 500k 7 MHz
	213500, // Complete 500k 7 MHz
	214000, // Complete 500k 7 MHz
	214500, // Complete 500k 7 MHz
	215000, // Complete 500k 7 MHz
	215500, // Complete 500k 7 MHz
	216000, // Complete 500k 7 MHz
	216500, // Complete 500k 7 MHz
	217000, // Complete 500k 7 MHz
	217500, // Complete 500k 7 MHz
	218000, // Complete 500k 7 MHz
	218500, // Complete 500k 7 MHz
	219000, // Complete 500k 7 MHz
	219500, // Complete 500k 7 MHz
	220000, // Complete 500k 7 MHz
	220500, // Complete 500k 7 MHz
	221000, // Complete 500k 7 MHz
	221500, // Complete 500k 7 MHz
	222000, // Complete 500k 7 MHz
	222500, // Complete 500k 7 MHz
	223000, // Complete 500k 7 MHz
	223500, // Complete 500k 7 MHz
	224000, // Complete 500k 7 MHz
	224500, // Complete 500k 7 MHz
	225000, // Complete 500k 7 MHz
	225500, // Complete 500k 7 MHz
	226000, // Complete 500k 7 MHz
	226500, // Complete 500k 7 MHz
	227000, // Complete 500k 7 MHz
	227500, // Complete 500k 7 MHz
	228000, // Complete 500k 7 MHz
	228500, // Complete 500k 7 MHz
	229000, // Complete 500k 7 MHz
	229500, // Complete 500k 7 MHz
	230000, // Complete 500k 7 MHz
	230500, // Complete 500k 7 MHz
	231000, // Complete 500k 7 MHz
	231500, // Complete 500k 7 MHz
	232000, // Complete 500k 7 MHz
	232500, // Complete 500k 7 MHz
	233000, // Complete 500k 7 MHz
	233500, // Complete 500k 7 MHz
	234000, // Complete 500k 7 MHz
	234500, // Complete 500k 7 MHz
	235000, // Complete 500k 7 MHz
	235500, // Complete 500k 7 MHz
	236000, // Complete 500k 7 MHz
	236500, // Complete 500k 7 MHz
	237000, // Complete 500k 7 MHz
	237500, // Complete 500k 7 MHz
	238000, // Complete 500k 7 MHz
	238500, // Complete 500k 7 MHz
	239000, // Complete 500k 7 MHz
	239500, // Complete 500k 7 MHz
	240000, // Complete 500k 7 MHz
	240500, // Complete 500k 7 MHz
	241000, // Complete 500k 7 MHz
	241500, // Complete 500k 7 MHz
	242000, // Complete 500k 7 MHz
	242500, // Complete 500k 7 MHz
	243000, // Complete 500k 7 MHz
	243500, // Complete 500k 7 MHz
	244000, // Complete 500k 7 MHz
	244500, // Complete 500k 7 MHz
	245000, // Complete 500k 7 MHz
	245500, // Complete 500k 7 MHz
	246000, // Complete 500k 7 MHz
	246500, // Complete 500k 7 MHz
	247000, // Complete 500k 7 MHz
	247500, // Complete 500k 7 MHz
	248000, // Complete 500k 7 MHz
	248500, // Complete 500k 7 MHz
	249000, // Complete 500k 7 MHz
	249500, // Complete 500k 7 MHz
	250000, // Complete 500k 7 MHz
	250500, // Complete 500k 7 MHz
	251000, // Complete 500k 7 MHz
	251500, // Complete 500k 7 MHz
	252000, // Complete 500k 7 MHz
	252500, // Complete 500k 7 MHz
	253000, // Complete 500k 7 MHz
	253500, // Complete 500k 7 MHz
	254000, // Complete 500k 7 MHz
	254500, // Complete 500k 7 MHz
	255000, // Complete 500k 7 MHz
	255500, // Complete 500k 7 MHz
	256000, // Complete 500k 7 MHz
	256500, // Complete 500k 7 MHz
	257000, // Complete 500k 7 MHz
	257500, // Complete 500k 7 MHz
	258000, // Complete 500k 7 MHz
	258500, // Complete 500k 7 MHz
	259000, // Complete 500k 7 MHz
	259500, // Complete 500k 7 MHz
	260000, // Complete 500k 7 MHz
	260500, // Complete 500k 7 MHz
	261000, // Complete 500k 7 MHz
	261500, // Complete 500k 7 MHz
	262000, // Complete 500k 7 MHz
	262500, // Complete 500k 7 MHz
	263000, // Complete 500k 7 MHz
	263500, // Complete 500k 7 MHz
	264000, // Complete 500k 7 MHz
	264500, // Complete 500k 7 MHz
	265000, // Complete 500k 7 MHz
	265500, // Complete 500k 7 MHz
	266000, // Complete 500k 7 MHz
	266500, // Complete 500k 7 MHz
	267000, // Complete 500k 7 MHz
	267500, // Complete 500k 7 MHz
	268000, // Complete 500k 7 MHz
	268500, // Complete 500k 7 MHz
	269000, // Complete 500k 7 MHz
	269500, // Complete 500k 7 MHz
	270000, // Complete 500k 7 MHz
	270500, // Complete 500k 7 MHz
	271000, // Complete 500k 7 MHz
	271500, // Complete 500k 7 MHz
	272000, // Complete 500k 7 MHz
	272500, // Complete 500k 7 MHz
	273000, // Complete 500k 7 MHz
	273500, // Complete 500k 7 MHz
	274000, // Complete 500k 7 MHz
	274500, // Complete 500k 7 MHz
	275000, // Complete 500k 7 MHz
	275500, // Complete 500k 7 MHz
	276000, // Complete 500k 7 MHz
	276500, // Complete 500k 7 MHz
	277000, // Complete 500k 7 MHz
	277500, // Complete 500k 7 MHz
	278000, // Complete 500k 7 MHz
	278500, // Complete 500k 7 MHz
	279000, // Complete 500k 7 MHz
	279500, // Complete 500k 7 MHz
	280000, // Complete 500k 7 MHz
	280500, // Complete 500k 7 MHz
	281000, // Complete 500k 7 MHz
	281500, // Complete 500k 7 MHz
	282000, // Complete 500k 7 MHz
	282500, // Complete 500k 7 MHz
	283000, // Complete 500k 7 MHz
	283500, // Complete 500k 7 MHz
	284000, // Complete 500k 7 MHz
	284500, // Complete 500k 7 MHz
	285000, // Complete 500k 7 MHz
	285500, // Complete 500k 7 MHz
	286000, // Complete 500k 7 MHz
	286500, // Complete 500k 7 MHz
	287000, // Complete 500k 7 MHz
	287500, // Complete 500k 7 MHz
	288000, // Complete 500k 7 MHz
	288500, // Complete 500k 7 MHz
	289000, // Complete 500k 7 MHz
	289500, // Complete 500k 7 MHz
	290000, // Complete 500k 7 MHz
	290500, // Complete 500k 7 MHz
	291000, // Complete 500k 7 MHz
	291500, // Complete 500k 7 MHz
	292000, // Complete 500k 7 MHz
	292500, // Complete 500k 7 MHz
	293000, // Complete 500k 7 MHz
	293500, // Complete 500k 7 MHz
	294000, // Complete 500k 7 MHz
	294500, // Complete 500k 7 MHz
	295000, // Complete 500k 7 MHz
	295500, // Complete 500k 7 MHz
	296000, // Complete 500k 7 MHz
	296500, // Complete 500k 7 MHz
	297000, // Complete 500k 7 MHz
	297500, // Complete 500k 7 MHz
	298000, // Complete 500k 7 MHz
	298500, // Complete 500k 7 MHz
	299000, // Complete 500k 7 MHz
	299500, // Complete 500k 7 MHz
	300000, // Complete 500k 7 MHz
	300500, // Complete 500k 7 MHz
	301000, // Complete 500k 7 MHz
	301500, // Complete 500k 7 MHz
	302000, // Complete 500k 7 MHz
	302500, // Complete 500k 7 MHz
	303000, // Complete 500k 7 MHz
	303500, // Complete 500k 7 MHz
	304000, // Complete 500k 7 MHz
	304500, // Complete 500k 7 MHz
	305000, // Complete 500k 7 MHz
	305500, // Complete 500k 7 MHz
	306000, // Complete 500k 7 MHz
	306500, // Complete 500k 7 MHz
	307000, // Complete 500k 7 MHz
	307500, // Complete 500k 7 MHz
	308000, // Complete 500k 7 MHz
	308500, // Complete 500k 7 MHz
	309000, // Complete 500k 7 MHz
	309500, // Complete 500k 7 MHz
	310000, // Complete 500k 7 MHz
	310500, // Complete 500k 7 MHz
	311000, // Complete 500k 7 MHz
	311500, // Complete 500k 7 MHz
	312000, // Complete 500k 7 MHz
	312500, // Complete 500k 7 MHz
	313000, // Complete 500k 7 MHz
	313500, // Complete 500k 7 MHz
	314000, // Complete 500k 7 MHz
	314500, // Complete 500k 7 MHz
	315000, // Complete 500k 7 MHz
	315500, // Complete 500k 7 MHz
	316000, // Complete 500k 7 MHz
	316500, // Complete 500k 7 MHz
	317000, // Complete 500k 7 MHz
	317500, // Complete 500k 7 MHz
	318000, // Complete 500k 7 MHz
	318500, // Complete 500k 7 MHz
	319000, // Complete 500k 7 MHz
	319500, // Complete 500k 7 MHz
	320000, // Complete 500k 7 MHz
	320500, // Complete 500k 7 MHz
	321000, // Complete 500k 7 MHz
	321500, // Complete 500k 7 MHz
	322000, // Complete 500k 7 MHz
	322500, // Complete 500k 7 MHz
	323000, // Complete 500k 7 MHz
	323500, // Complete 500k 7 MHz
	324000, // Complete 500k 7 MHz
	324500, // Complete 500k 7 MHz
	325000, // Complete 500k 7 MHz
	325500, // Complete 500k 7 MHz
	326000, // Complete 500k 7 MHz
	326500, // Complete 500k 7 MHz
	327000, // Complete 500k 7 MHz
	327500, // Complete 500k 7 MHz
	328000, // Complete 500k 7 MHz
	328500, // Complete 500k 7 MHz
	329000, // Complete 500k 7 MHz
	329500, // Complete 500k 7 MHz
	330000, // Complete 500k 7 MHz
	330500, // Complete 500k 7 MHz
	331000, // Complete 500k 7 MHz
	331500, // Complete 500k 7 MHz
	332000, // Complete 500k 7 MHz
	332500, // Complete 500k 7 MHz
	333000, // Complete 500k 7 MHz
	333500, // Complete 500k 7 MHz
	334000, // Complete 500k 7 MHz
	334500, // Complete 500k 7 MHz
	335000, // Complete 500k 7 MHz
	335500, // Complete 500k 7 MHz
	336000, // Complete 500k 7 MHz
	336500, // Complete 500k 7 MHz
	337000, // Complete 500k 7 MHz
	337500, // Complete 500k 7 MHz
	338000, // Complete 500k 7 MHz
	338500, // Complete 500k 7 MHz
	339000, // Complete 500k 7 MHz
	339500, // Complete 500k 7 MHz
	340000, // Complete 500k 7 MHz
	340500, // Complete 500k 7 MHz
	341000, // Complete 500k 7 MHz
	341500, // Complete 500k 7 MHz
	342000, // Complete 500k 7 MHz
	342500, // Complete 500k 7 MHz
	343000, // Complete 500k 7 MHz
	343500, // Complete 500k 7 MHz
	344000, // Complete 500k 7 MHz
	344500, // Complete 500k 7 MHz
	345000, // Complete 500k 7 MHz
	345500, // Complete 500k 7 MHz
	346000, // Complete 500k 7 MHz
	346500, // Complete 500k 7 MHz
	347000, // Complete 500k 7 MHz
	347500, // Complete 500k 7 MHz
	348000, // Complete 500k 7 MHz
	348500, // Complete 500k 7 MHz
	349000, // Complete 500k 7 MHz
	349500, // Complete 500k 7 MHz
	350000, // Complete 500k 7 MHz
	350500, // Complete 500k 7 MHz
	351000, // Complete 500k 7 MHz
	351500, // Complete 500k 7 MHz
	352000, // Complete 500k 7 MHz
	352500, // Complete 500k 7 MHz
	353000, // Complete 500k 7 MHz
	353500, // Complete 500k 7 MHz
	354000, // Complete 500k 7 MHz
	354500, // Complete 500k 7 MHz
	355000, // Complete 500k 7 MHz
	355500, // Complete 500k 7 MHz
	356000, // Complete 500k 7 MHz
	356500, // Complete 500k 7 MHz
	357000, // Complete 500k 7 MHz
	357500, // Complete 500k 7 MHz
	358000, // Complete 500k 7 MHz
	358500, // Complete 500k 7 MHz
	359000, // Complete 500k 7 MHz
	359500, // Complete 500k 7 MHz
	360000, // Complete 500k 7 MHz
	360500, // Complete 500k 7 MHz
	361000, // Complete 500k 7 MHz
	361500, // Complete 500k 7 MHz
	362000, // Complete 500k 7 MHz
	362500, // Complete 500k 7 MHz
	363000, // Complete 500k 7 MHz
	363500, // Complete 500k 7 MHz
	364000, // Complete 500k 7 MHz
	364500, // Complete 500k 7 MHz
	365000, // Complete 500k 7 MHz
	365500, // Complete 500k 7 MHz
	366000, // Complete 500k 7 MHz
	366500, // Complete 500k 7 MHz
	367000, // Complete 500k 7 MHz
	367500, // Complete 500k 7 MHz
	368000, // Complete 500k 7 MHz
	368500, // Complete 500k 7 MHz
	369000, // Complete 500k 7 MHz
	369500, // Complete 500k 7 MHz
	370000, // Complete 500k 7 MHz
	370500, // Complete 500k 7 MHz
	371000, // Complete 500k 7 MHz
	371500, // Complete 500k 7 MHz
	372000, // Complete 500k 7 MHz
	372500, // Complete 500k 7 MHz
	373000, // Complete 500k 7 MHz
	373500, // Complete 500k 7 MHz
	374000, // Complete 500k 7 MHz
	374500, // Complete 500k 7 MHz
	375000, // Complete 500k 7 MHz
	375500, // Complete 500k 7 MHz
	376000, // Complete 500k 7 MHz
	376500, // Complete 500k 7 MHz
	377000, // Complete 500k 7 MHz
	377500, // Complete 500k 7 MHz
	378000, // Complete 500k 7 MHz
	378500, // Complete 500k 7 MHz
	379000, // Complete 500k 7 MHz
	379500, // Complete 500k 7 MHz
	380000, // Complete 500k 7 MHz
	380500, // Complete 500k 7 MHz
	381000, // Complete 500k 7 MHz
	381500, // Complete 500k 7 MHz
	382000, // Complete 500k 7 MHz
	382500, // Complete 500k 7 MHz
	383000, // Complete 500k 7 MHz
	383500, // Complete 500k 7 MHz
	384000, // Complete 500k 7 MHz
	384500, // Complete 500k 7 MHz
	385000, // Complete 500k 7 MHz
	385500, // Complete 500k 7 MHz
	386000, // Complete 500k 7 MHz
	386500, // Complete 500k 7 MHz
	387000, // Complete 500k 7 MHz
	387500, // Complete 500k 7 MHz
	388000, // Complete 500k 7 MHz
	388500, // Complete 500k 7 MHz
	389000, // Complete 500k 7 MHz
	389500, // Complete 500k 7 MHz
	390000, // Complete 500k 7 MHz
	390500, // Complete 500k 7 MHz
	391000, // Complete 500k 7 MHz
	391500, // Complete 500k 7 MHz
	392000, // Complete 500k 7 MHz
	392500, // Complete 500k 7 MHz
	393000, // Complete 500k 7 MHz
	393500, // Complete 500k 7 MHz
	394000, // Complete 500k 7 MHz
	394500, // Complete 500k 7 MHz
	395000, // Complete 500k 7 MHz
	395500, // Complete 500k 7 MHz
	396000, // Complete 500k 7 MHz
	396500, // Complete 500k 7 MHz
	397000, // Complete 500k 7 MHz
	397500, // Complete 500k 7 MHz
	398000, // Complete 500k 7 MHz
	398500, // Complete 500k 7 MHz
	399000, // Complete 500k 7 MHz
	399500, // Complete 500k 7 MHz
	400000, // Complete 500k 7 MHz
	400500, // Complete 500k 7 MHz
	401000, // Complete 500k 7 MHz
	401500, // Complete 500k 7 MHz
	402000, // Complete 500k 7 MHz
	402500, // Complete 500k 7 MHz
	403000, // Complete 500k 7 MHz
	403500, // Complete 500k 7 MHz
	404000, // Complete 500k 7 MHz
	404500, // Complete 500k 7 MHz
	405000, // Complete 500k 7 MHz
	405500, // Complete 500k 7 MHz
	406000, // Complete 500k 7 MHz
	406500, // Complete 500k 7 MHz
	407000, // Complete 500k 7 MHz
	407500, // Complete 500k 7 MHz
	408000, // Complete 500k 7 MHz
	408500, // Complete 500k 7 MHz
	409000, // Complete 500k 7 MHz
	409500, // Complete 500k 7 MHz
	410000, // Complete 500k 7 MHz
	410500, // Complete 500k 7 MHz
	411000, // Complete 500k 7 MHz
	411500, // Complete 500k 7 MHz
	412000, // Complete 500k 7 MHz
	412500, // Complete 500k 7 MHz
	413000, // Complete 500k 7 MHz
	413500, // Complete 500k 7 MHz
	414000, // Complete 500k 7 MHz
	414500, // Complete 500k 7 MHz
	415000, // Complete 500k 7 MHz
	415500, // Complete 500k 7 MHz
	416000, // Complete 500k 7 MHz
	416500, // Complete 500k 7 MHz
	417000, // Complete 500k 7 MHz
	417500, // Complete 500k 7 MHz
	418000, // Complete 500k 7 MHz
	418500, // Complete 500k 7 MHz
	419000, // Complete 500k 7 MHz
	419500, // Complete 500k 7 MHz
	420000, // Complete 500k 7 MHz
	420500, // Complete 500k 7 MHz
	421000, // Complete 500k 7 MHz
	421500, // Complete 500k 7 MHz
	422000, // Complete 500k 7 MHz
	422500, // Complete 500k 7 MHz
	423000, // Complete 500k 7 MHz
	423500, // Complete 500k 7 MHz
	424000, // Complete 500k 7 MHz
	424500, // Complete 500k 7 MHz
	425000, // Complete 500k 7 MHz
	425500, // Complete 500k 7 MHz
	426000, // Complete 500k 7 MHz
	426500, // Complete 500k 7 MHz
	427000, // Complete 500k 7 MHz
	427500, // Complete 500k 7 MHz
	428000, // Complete 500k 7 MHz
	428500, // Complete 500k 7 MHz
	429000, // Complete 500k 7 MHz
	429500, // Complete 500k 7 MHz
	430000, // Complete 500k 7 MHz
	430500, // Complete 500k 7 MHz
	431000, // Complete 500k 7 MHz
	431500, // Complete 500k 7 MHz
	432000, // Complete 500k 7 MHz
	432500, // Complete 500k 7 MHz
	433000, // Complete 500k 7 MHz
	433500, // Complete 500k 7 MHz
	434000, // Complete 500k 7 MHz
	434500, // Complete 500k 7 MHz
	435000, // Complete 500k 7 MHz
	435500, // Complete 500k 7 MHz
	436000, // Complete 500k 7 MHz
	436500, // Complete 500k 7 MHz
	437000, // Complete 500k 7 MHz
	437500, // Complete 500k 7 MHz
	438000, // Complete 500k 7 MHz
	438500, // Complete 500k 7 MHz
	439000, // Complete 500k 7 MHz
	439500, // Complete 500k 7 MHz
	440000, // Complete 500k 7 MHz
	440500, // Complete 500k 7 MHz
	441000, // Complete 500k 7 MHz
	441500, // Complete 500k 7 MHz
	442000, // Complete 500k 7 MHz
	442500, // Complete 500k 7 MHz
	443000, // Complete 500k 7 MHz
	443500, // Complete 500k 7 MHz
	444000, // Complete 500k 7 MHz
	444500, // Complete 500k 7 MHz
	445000, // Complete 500k 7 MHz
	445500, // Complete 500k 7 MHz
	446000, // Complete 500k 7 MHz
	446500, // Complete 500k 7 MHz
	447000, // Complete 500k 7 MHz
	447500, // Complete 500k 7 MHz
	448000, // Complete 500k 7 MHz
	448500, // Complete 500k 7 MHz
	449000, // Complete 500k 7 MHz
	449500, // Complete 500k 7 MHz
	450000, // Complete 500k 7 MHz
	450500, // Complete 500k 7 MHz
	451000, // Complete 500k 7 MHz
	451500, // Complete 500k 7 MHz
	452000, // Complete 500k 7 MHz
	452500, // Complete 500k 7 MHz
	453000, // Complete 500k 7 MHz
	453500, // Complete 500k 7 MHz
	454000, // Complete 500k 7 MHz
	454500, // Complete 500k 7 MHz
	455000, // Complete 500k 7 MHz
	455500, // Complete 500k 7 MHz
	456000, // Complete 500k 7 MHz
	456500, // Complete 500k 7 MHz
	457000, // Complete 500k 7 MHz
	457500, // Complete 500k 7 MHz
	458000, // Complete 500k 7 MHz
	458500, // Complete 500k 7 MHz
	459000, // Complete 500k 7 MHz
	459500, // Complete 500k 7 MHz
	460000, // Complete 500k 7 MHz
	460500, // Complete 500k 7 MHz
	461000, // Complete 500k 7 MHz
	461500, // Complete 500k 7 MHz
	462000, // Complete 500k 7 MHz
	462500, // Complete 500k 7 MHz
	463000, // Complete 500k 7 MHz
	463500, // Complete 500k 7 MHz
	464000, // Complete 500k 7 MHz
	464500, // Complete 500k 7 MHz
	465000, // Complete 500k 7 MHz
	465500, // Complete 500k 7 MHz
	466000, // Complete 500k 7 MHz
	466500, // Complete 500k 7 MHz
	467000, // Complete 500k 7 MHz
	467500, // Complete 500k 7 MHz
	468000, // Complete 500k 7 MHz
	468500, // Complete 500k 7 MHz
	469000, // Complete 500k 7 MHz
	469500, // Complete 500k 7 MHz
	470000, // Complete 500k 7 MHz
	470500, // Complete 500k 7 MHz
	471000, // Complete 500k 7 MHz
	471500, // Complete 500k 7 MHz
	472000, // Complete 500k 7 MHz
	472500, // Complete 500k 7 MHz
	473000, // Complete 500k 7 MHz
	473500, // Complete 500k 7 MHz
	474000, // Complete 500k 7 MHz
	474500, // Complete 500k 7 MHz
	475000, // Complete 500k 7 MHz
	475500, // Complete 500k 7 MHz
	476000, // Complete 500k 7 MHz
	476500, // Complete 500k 7 MHz
	477000, // Complete 500k 7 MHz
	477500, // Complete 500k 7 MHz
	478000, // Complete 500k 7 MHz
	478500, // Complete 500k 7 MHz
	479000, // Complete 500k 7 MHz
	479500, // Complete 500k 7 MHz
	480000, // Complete 500k 7 MHz
	480500, // Complete 500k 7 MHz
	481000, // Complete 500k 7 MHz
	481500, // Complete 500k 7 MHz
	482000, // Complete 500k 7 MHz
	482500, // Complete 500k 7 MHz
	483000, // Complete 500k 7 MHz
	483500, // Complete 500k 7 MHz
	484000, // Complete 500k 7 MHz
	484500, // Complete 500k 7 MHz
	485000, // Complete 500k 7 MHz
	485500, // Complete 500k 7 MHz
	486000, // Complete 500k 7 MHz
	486500, // Complete 500k 7 MHz
	487000, // Complete 500k 7 MHz
	487500, // Complete 500k 7 MHz
	488000, // Complete 500k 7 MHz
	488500, // Complete 500k 7 MHz
	489000, // Complete 500k 7 MHz
	489500, // Complete 500k 7 MHz
	490000, // Complete 500k 7 MHz
	490500, // Complete 500k 7 MHz
	491000, // Complete 500k 7 MHz
	491500, // Complete 500k 7 MHz
	492000, // Complete 500k 7 MHz
	492500, // Complete 500k 7 MHz
	493000, // Complete 500k 7 MHz
	493500, // Complete 500k 7 MHz
	494000, // Complete 500k 7 MHz
	494500, // Complete 500k 7 MHz
	495000, // Complete 500k 7 MHz
	495500, // Complete 500k 7 MHz
	496000, // Complete 500k 7 MHz
	496500, // Complete 500k 7 MHz
	497000, // Complete 500k 7 MHz
	497500, // Complete 500k 7 MHz
	498000, // Complete 500k 7 MHz
	498500, // Complete 500k 7 MHz
	499000, // Complete 500k 7 MHz
	499500, // Complete 500k 7 MHz
	500000, // Complete 500k 7 MHz
	500500, // Complete 500k 7 MHz
	501000, // Complete 500k 7 MHz
	501500, // Complete 500k 7 MHz
	502000, // Complete 500k 7 MHz
	502500, // Complete 500k 7 MHz
	503000, // Complete 500k 7 MHz
	503500, // Complete 500k 7 MHz
	504000, // Complete 500k 7 MHz
	504500, // Complete 500k 7 MHz
	505000, // Complete 500k 7 MHz
	505500, // Complete 500k 7 MHz
	506000, // Complete 500k 7 MHz
	506500, // Complete 500k 7 MHz
	507000, // Complete 500k 7 MHz
	507500, // Complete 500k 7 MHz
	508000, // Complete 500k 7 MHz
	508500, // Complete 500k 7 MHz
	509000, // Complete 500k 7 MHz
	509500, // Complete 500k 7 MHz
	510000, // Complete 500k 7 MHz
	510500, // Complete 500k 7 MHz
	511000, // Complete 500k 7 MHz
	511500, // Complete 500k 7 MHz
	512000, // Complete 500k 7 MHz
	512500, // Complete 500k 7 MHz
	513000, // Complete 500k 7 MHz
	513500, // Complete 500k 7 MHz
	514000, // Complete 500k 7 MHz
	514500, // Complete 500k 7 MHz
	515000, // Complete 500k 7 MHz
	515500, // Complete 500k 7 MHz
	516000, // Complete 500k 7 MHz
	516500, // Complete 500k 7 MHz
	517000, // Complete 500k 7 MHz
	517500, // Complete 500k 7 MHz
	518000, // Complete 500k 7 MHz
	518500, // Complete 500k 7 MHz
	519000, // Complete 500k 7 MHz
	519500, // Complete 500k 7 MHz
	520000, // Complete 500k 7 MHz
	520500, // Complete 500k 7 MHz
	521000, // Complete 500k 7 MHz
	521500, // Complete 500k 7 MHz
	522000, // Complete 500k 7 MHz
	522500, // Complete 500k 7 MHz
	523000, // Complete 500k 7 MHz
	523500, // Complete 500k 7 MHz
	524000, // Complete 500k 7 MHz
	524500, // Complete 500k 7 MHz
	525000, // Complete 500k 7 MHz
	525500, // Complete 500k 7 MHz
	526000, // Complete 500k 7 MHz
	526500, // Complete 500k 7 MHz
	527000, // Complete 500k 7 MHz
	527500, // Complete 500k 7 MHz
	528000, // Complete 500k 7 MHz
	528500, // Complete 500k 7 MHz
	529000, // Complete 500k 7 MHz
	529500, // Complete 500k 7 MHz
	530000, // Complete 500k 7 MHz
	530500, // Complete 500k 7 MHz
	531000, // Complete 500k 7 MHz
	531500, // Complete 500k 7 MHz
	532000, // Complete 500k 7 MHz
	532500, // Complete 500k 7 MHz
	533000, // Complete 500k 7 MHz
	533500, // Complete 500k 7 MHz
	534000, // Complete 500k 7 MHz
	534500, // Complete 500k 7 MHz
	535000, // Complete 500k 7 MHz
	535500, // Complete 500k 7 MHz
	536000, // Complete 500k 7 MHz
	536500, // Complete 500k 7 MHz
	537000, // Complete 500k 7 MHz
	537500, // Complete 500k 7 MHz
	538000, // Complete 500k 7 MHz
	538500, // Complete 500k 7 MHz
	539000, // Complete 500k 7 MHz
	539500, // Complete 500k 7 MHz
	540000, // Complete 500k 7 MHz
	540500, // Complete 500k 7 MHz
	541000, // Complete 500k 7 MHz
	541500, // Complete 500k 7 MHz
	542000, // Complete 500k 7 MHz
	542500, // Complete 500k 7 MHz
	543000, // Complete 500k 7 MHz
	543500, // Complete 500k 7 MHz
	544000, // Complete 500k 7 MHz
	544500, // Complete 500k 7 MHz
	545000, // Complete 500k 7 MHz
	545500, // Complete 500k 7 MHz
	546000, // Complete 500k 7 MHz
	546500, // Complete 500k 7 MHz
	547000, // Complete 500k 7 MHz
	547500, // Complete 500k 7 MHz
	548000, // Complete 500k 7 MHz
	548500, // Complete 500k 7 MHz
	549000, // Complete 500k 7 MHz
	549500, // Complete 500k 7 MHz
	550000, // Complete 500k 7 MHz
	550500, // Complete 500k 7 MHz
	551000, // Complete 500k 7 MHz
	551500, // Complete 500k 7 MHz
	552000, // Complete 500k 7 MHz
	552500, // Complete 500k 7 MHz
	553000, // Complete 500k 7 MHz
	553500, // Complete 500k 7 MHz
	554000, // Complete 500k 7 MHz
	554500, // Complete 500k 7 MHz
	555000, // Complete 500k 7 MHz
	555500, // Complete 500k 7 MHz
	556000, // Complete 500k 7 MHz
	556500, // Complete 500k 7 MHz
	557000, // Complete 500k 7 MHz
	557500, // Complete 500k 7 MHz
	558000, // Complete 500k 7 MHz
	558500, // Complete 500k 7 MHz
	559000, // Complete 500k 7 MHz
	559500, // Complete 500k 7 MHz
	560000, // Complete 500k 7 MHz
	560500, // Complete 500k 7 MHz
	561000, // Complete 500k 7 MHz
	561500, // Complete 500k 7 MHz
	562000, // Complete 500k 7 MHz
	562500, // Complete 500k 7 MHz
	563000, // Complete 500k 7 MHz
	563500, // Complete 500k 7 MHz
	564000, // Complete 500k 7 MHz
	564500, // Complete 500k 7 MHz
	565000, // Complete 500k 7 MHz
	565500, // Complete 500k 7 MHz
	566000, // Complete 500k 7 MHz
	566500, // Complete 500k 7 MHz
	567000, // Complete 500k 7 MHz
	567500, // Complete 500k 7 MHz
	568000, // Complete 500k 7 MHz
	568500, // Complete 500k 7 MHz
	569000, // Complete 500k 7 MHz
	569500, // Complete 500k 7 MHz
	570000, // Complete 500k 7 MHz
	570500, // Complete 500k 7 MHz
	571000, // Complete 500k 7 MHz
	571500, // Complete 500k 7 MHz
	572000, // Complete 500k 7 MHz
	572500, // Complete 500k 7 MHz
	573000, // Complete 500k 7 MHz
	573500, // Complete 500k 7 MHz
	574000, // Complete 500k 7 MHz
	574500, // Complete 500k 7 MHz
	575000, // Complete 500k 7 MHz
	575500, // Complete 500k 7 MHz
	576000, // Complete 500k 7 MHz
	576500, // Complete 500k 7 MHz
	577000, // Complete 500k 7 MHz
	577500, // Complete 500k 7 MHz
	578000, // Complete 500k 7 MHz
	578500, // Complete 500k 7 MHz
	579000, // Complete 500k 7 MHz
	579500, // Complete 500k 7 MHz
	580000, // Complete 500k 7 MHz
	580500, // Complete 500k 7 MHz
	581000, // Complete 500k 7 MHz
	581500, // Complete 500k 7 MHz
	582000, // Complete 500k 7 MHz
	582500, // Complete 500k 7 MHz
	583000, // Complete 500k 7 MHz
	583500, // Complete 500k 7 MHz
	584000, // Complete 500k 7 MHz
	584500, // Complete 500k 7 MHz
	585000, // Complete 500k 7 MHz
	585500, // Complete 500k 7 MHz
	586000, // Complete 500k 7 MHz
	586500, // Complete 500k 7 MHz
	587000, // Complete 500k 7 MHz
	587500, // Complete 500k 7 MHz
	588000, // Complete 500k 7 MHz
	588500, // Complete 500k 7 MHz
	589000, // Complete 500k 7 MHz
	589500, // Complete 500k 7 MHz
	590000, // Complete 500k 7 MHz
	590500, // Complete 500k 7 MHz
	591000, // Complete 500k 7 MHz
	591500, // Complete 500k 7 MHz
	592000, // Complete 500k 7 MHz
	592500, // Complete 500k 7 MHz
	593000, // Complete 500k 7 MHz
	593500, // Complete 500k 7 MHz
	594000, // Complete 500k 7 MHz
	594500, // Complete 500k 7 MHz
	595000, // Complete 500k 7 MHz
	595500, // Complete 500k 7 MHz
	596000, // Complete 500k 7 MHz
	596500, // Complete 500k 7 MHz
	597000, // Complete 500k 7 MHz
	597500, // Complete 500k 7 MHz
	598000, // Complete 500k 7 MHz
	598500, // Complete 500k 7 MHz
	599000, // Complete 500k 7 MHz
	599500, // Complete 500k 7 MHz
	600000, // Complete 500k 7 MHz
	600500, // Complete 500k 7 MHz
	601000, // Complete 500k 7 MHz
	601500, // Complete 500k 7 MHz
	602000, // Complete 500k 7 MHz
	602500, // Complete 500k 7 MHz
	603000, // Complete 500k 7 MHz
	603500, // Complete 500k 7 MHz
	604000, // Complete 500k 7 MHz
	604500, // Complete 500k 7 MHz
	605000, // Complete 500k 7 MHz
	605500, // Complete 500k 7 MHz
	606000, // Complete 500k 7 MHz
	606500, // Complete 500k 7 MHz
	607000, // Complete 500k 7 MHz
	607500, // Complete 500k 7 MHz
	608000, // Complete 500k 7 MHz
	608500, // Complete 500k 7 MHz
	609000, // Complete 500k 7 MHz
	609500, // Complete 500k 7 MHz
	610000, // Complete 500k 7 MHz
	610500, // Complete 500k 7 MHz
	611000, // Complete 500k 7 MHz
	611500, // Complete 500k 7 MHz
	612000, // Complete 500k 7 MHz
	612500, // Complete 500k 7 MHz
	613000, // Complete 500k 7 MHz
	613500, // Complete 500k 7 MHz
	614000, // Complete 500k 7 MHz
	614500, // Complete 500k 7 MHz
	615000, // Complete 500k 7 MHz
	615500, // Complete 500k 7 MHz
	616000, // Complete 500k 7 MHz
	616500, // Complete 500k 7 MHz
	617000, // Complete 500k 7 MHz
	617500, // Complete 500k 7 MHz
	618000, // Complete 500k 7 MHz
	618500, // Complete 500k 7 MHz
	619000, // Complete 500k 7 MHz
	619500, // Complete 500k 7 MHz
	620000, // Complete 500k 7 MHz
	620500, // Complete 500k 7 MHz
	621000, // Complete 500k 7 MHz
	621500, // Complete 500k 7 MHz
	622000, // Complete 500k 7 MHz
	622500, // Complete 500k 7 MHz
	623000, // Complete 500k 7 MHz
	623500, // Complete 500k 7 MHz
	624000, // Complete 500k 7 MHz
	624500, // Complete 500k 7 MHz
	625000, // Complete 500k 7 MHz
	625500, // Complete 500k 7 MHz
	626000, // Complete 500k 7 MHz
	626500, // Complete 500k 7 MHz
	627000, // Complete 500k 7 MHz
	627500, // Complete 500k 7 MHz
	628000, // Complete 500k 7 MHz
	628500, // Complete 500k 7 MHz
	629000, // Complete 500k 7 MHz
	629500, // Complete 500k 7 MHz
	630000, // Complete 500k 7 MHz
	630500, // Complete 500k 7 MHz
	631000, // Complete 500k 7 MHz
	631500, // Complete 500k 7 MHz
	632000, // Complete 500k 7 MHz
	632500, // Complete 500k 7 MHz
	633000, // Complete 500k 7 MHz
	633500, // Complete 500k 7 MHz
	634000, // Complete 500k 7 MHz
	634500, // Complete 500k 7 MHz
	635000, // Complete 500k 7 MHz
	635500, // Complete 500k 7 MHz
	636000, // Complete 500k 7 MHz
	636500, // Complete 500k 7 MHz
	637000, // Complete 500k 7 MHz
	637500, // Complete 500k 7 MHz
	638000, // Complete 500k 7 MHz
	638500, // Complete 500k 7 MHz
	639000, // Complete 500k 7 MHz
	639500, // Complete 500k 7 MHz
	640000, // Complete 500k 7 MHz
	640500, // Complete 500k 7 MHz
	641000, // Complete 500k 7 MHz
	641500, // Complete 500k 7 MHz
	642000, // Complete 500k 7 MHz
	642500, // Complete 500k 7 MHz
	643000, // Complete 500k 7 MHz
	643500, // Complete 500k 7 MHz
	644000, // Complete 500k 7 MHz
	644500, // Complete 500k 7 MHz
	645000, // Complete 500k 7 MHz
	645500, // Complete 500k 7 MHz
	646000, // Complete 500k 7 MHz
	646500, // Complete 500k 7 MHz
	647000, // Complete 500k 7 MHz
	647500, // Complete 500k 7 MHz
	648000, // Complete 500k 7 MHz
	648500, // Complete 500k 7 MHz
	649000, // Complete 500k 7 MHz
	649500, // Complete 500k 7 MHz
	650000, // Complete 500k 7 MHz
	650500, // Complete 500k 7 MHz
	651000, // Complete 500k 7 MHz
	651500, // Complete 500k 7 MHz
	652000, // Complete 500k 7 MHz
	652500, // Complete 500k 7 MHz
	653000, // Complete 500k 7 MHz
	653500, // Complete 500k 7 MHz
	654000, // Complete 500k 7 MHz
	654500, // Complete 500k 7 MHz
	655000, // Complete 500k 7 MHz
	655500, // Complete 500k 7 MHz
	656000, // Complete 500k 7 MHz
	656500, // Complete 500k 7 MHz
	657000, // Complete 500k 7 MHz
	657500, // Complete 500k 7 MHz
	658000, // Complete 500k 7 MHz
	658500, // Complete 500k 7 MHz
	659000, // Complete 500k 7 MHz
	659500, // Complete 500k 7 MHz
	660000, // Complete 500k 7 MHz
	660500, // Complete 500k 7 MHz
	661000, // Complete 500k 7 MHz
	661500, // Complete 500k 7 MHz
	662000, // Complete 500k 7 MHz
	662500, // Complete 500k 7 MHz
	663000, // Complete 500k 7 MHz
	663500, // Complete 500k 7 MHz
	664000, // Complete 500k 7 MHz
	664500, // Complete 500k 7 MHz
	665000, // Complete 500k 7 MHz
	665500, // Complete 500k 7 MHz
	666000, // Complete 500k 7 MHz
	666500, // Complete 500k 7 MHz
	667000, // Complete 500k 7 MHz
	667500, // Complete 500k 7 MHz
	668000, // Complete 500k 7 MHz
	668500, // Complete 500k 7 MHz
	669000, // Complete 500k 7 MHz
	669500, // Complete 500k 7 MHz
	670000, // Complete 500k 7 MHz
	670500, // Complete 500k 7 MHz
	671000, // Complete 500k 7 MHz
	671500, // Complete 500k 7 MHz
	672000, // Complete 500k 7 MHz
	672500, // Complete 500k 7 MHz
	673000, // Complete 500k 7 MHz
	673500, // Complete 500k 7 MHz
	674000, // Complete 500k 7 MHz
	674500, // Complete 500k 7 MHz
	675000, // Complete 500k 7 MHz
	675500, // Complete 500k 7 MHz
	676000, // Complete 500k 7 MHz
	676500, // Complete 500k 7 MHz
	677000, // Complete 500k 7 MHz
	677500, // Complete 500k 7 MHz
	678000, // Complete 500k 7 MHz
	678500, // Complete 500k 7 MHz
	679000, // Complete 500k 7 MHz
	679500, // Complete 500k 7 MHz
	680000, // Complete 500k 7 MHz
	680500, // Complete 500k 7 MHz
	681000, // Complete 500k 7 MHz
	681500, // Complete 500k 7 MHz
	682000, // Complete 500k 7 MHz
	682500, // Complete 500k 7 MHz
	683000, // Complete 500k 7 MHz
	683500, // Complete 500k 7 MHz
	684000, // Complete 500k 7 MHz
	684500, // Complete 500k 7 MHz
	685000, // Complete 500k 7 MHz
	685500, // Complete 500k 7 MHz
	686000, // Complete 500k 7 MHz
	686500, // Complete 500k 7 MHz
	687000, // Complete 500k 7 MHz
	687500, // Complete 500k 7 MHz
	688000, // Complete 500k 7 MHz
	688500, // Complete 500k 7 MHz
	689000, // Complete 500k 7 MHz
	689500, // Complete 500k 7 MHz
	690000, // Complete 500k 7 MHz
	690500, // Complete 500k 7 MHz
	691000, // Complete 500k 7 MHz
	691500, // Complete 500k 7 MHz
	692000, // Complete 500k 7 MHz
	692500, // Complete 500k 7 MHz
	693000, // Complete 500k 7 MHz
	693500, // Complete 500k 7 MHz
	694000, // Complete 500k 7 MHz
	694500, // Complete 500k 7 MHz
	695000, // Complete 500k 7 MHz
	695500, // Complete 500k 7 MHz
	696000, // Complete 500k 7 MHz
	696500, // Complete 500k 7 MHz
	697000, // Complete 500k 7 MHz
	697500, // Complete 500k 7 MHz
	698000, // Complete 500k 7 MHz
	698500, // Complete 500k 7 MHz
	699000, // Complete 500k 7 MHz
	699500, // Complete 500k 7 MHz
	700000, // Complete 500k 7 MHz
	700500, // Complete 500k 7 MHz
	701000, // Complete 500k 7 MHz
	701500, // Complete 500k 7 MHz
	702000, // Complete 500k 7 MHz
	702500, // Complete 500k 7 MHz
	703000, // Complete 500k 7 MHz
	703500, // Complete 500k 7 MHz
	704000, // Complete 500k 7 MHz
	704500, // Complete 500k 7 MHz
	705000, // Complete 500k 7 MHz
	705500, // Complete 500k 7 MHz
	706000, // Complete 500k 7 MHz
	706500, // Complete 500k 7 MHz
	707000, // Complete 500k 7 MHz
	707500, // Complete 500k 7 MHz
	708000, // Complete 500k 7 MHz
	708500, // Complete 500k 7 MHz
	709000, // Complete 500k 7 MHz
	709500, // Complete 500k 7 MHz
	710000, // Complete 500k 7 MHz
	710500, // Complete 500k 7 MHz
	711000, // Complete 500k 7 MHz
	711500, // Complete 500k 7 MHz
	712000, // Complete 500k 7 MHz
	712500, // Complete 500k 7 MHz
	713000, // Complete 500k 7 MHz
	713500, // Complete 500k 7 MHz
	714000, // Complete 500k 7 MHz
	714500, // Complete 500k 7 MHz
	715000, // Complete 500k 7 MHz
	715500, // Complete 500k 7 MHz
	716000, // Complete 500k 7 MHz
	716500, // Complete 500k 7 MHz
	717000, // Complete 500k 7 MHz
	717500, // Complete 500k 7 MHz
	718000, // Complete 500k 7 MHz
	718500, // Complete 500k 7 MHz
	719000, // Complete 500k 7 MHz
	719500, // Complete 500k 7 MHz
	720000, // Complete 500k 7 MHz
	720500, // Complete 500k 7 MHz
	721000, // Complete 500k 7 MHz
	721500, // Complete 500k 7 MHz
	722000, // Complete 500k 7 MHz
	722500, // Complete 500k 7 MHz
	723000, // Complete 500k 7 MHz
	723500, // Complete 500k 7 MHz
	724000, // Complete 500k 7 MHz
	724500, // Complete 500k 7 MHz
	725000, // Complete 500k 7 MHz
	725500, // Complete 500k 7 MHz
	726000, // Complete 500k 7 MHz
	726500, // Complete 500k 7 MHz
	727000, // Complete 500k 7 MHz
	727500, // Complete 500k 7 MHz
	728000, // Complete 500k 7 MHz
	728500, // Complete 500k 7 MHz
	729000, // Complete 500k 7 MHz
	729500, // Complete 500k 7 MHz
	730000, // Complete 500k 7 MHz
	730500, // Complete 500k 7 MHz
	731000, // Complete 500k 7 MHz
	731500, // Complete 500k 7 MHz
	732000, // Complete 500k 7 MHz
	732500, // Complete 500k 7 MHz
	733000, // Complete 500k 7 MHz
	733500, // Complete 500k 7 MHz
	734000, // Complete 500k 7 MHz
	734500, // Complete 500k 7 MHz
	735000, // Complete 500k 7 MHz
	735500, // Complete 500k 7 MHz
	736000, // Complete 500k 7 MHz
	736500, // Complete 500k 7 MHz
	737000, // Complete 500k 7 MHz
	737500, // Complete 500k 7 MHz
	738000, // Complete 500k 7 MHz
	738500, // Complete 500k 7 MHz
	739000, // Complete 500k 7 MHz
	739500, // Complete 500k 7 MHz
	740000, // Complete 500k 7 MHz
	740500, // Complete 500k 7 MHz
	741000, // Complete 500k 7 MHz
	741500, // Complete 500k 7 MHz
	742000, // Complete 500k 7 MHz
	742500, // Complete 500k 7 MHz
	743000, // Complete 500k 7 MHz
	743500, // Complete 500k 7 MHz
	744000, // Complete 500k 7 MHz
	744500, // Complete 500k 7 MHz
	745000, // Complete 500k 7 MHz
	745500, // Complete 500k 7 MHz
	746000, // Complete 500k 7 MHz
	746500, // Complete 500k 7 MHz
	747000, // Complete 500k 7 MHz
	747500, // Complete 500k 7 MHz
	748000, // Complete 500k 7 MHz
	748500, // Complete 500k 7 MHz
	749000, // Complete 500k 7 MHz
	749500, // Complete 500k 7 MHz
	750000, // Complete 500k 7 MHz
	750500, // Complete 500k 7 MHz
	751000, // Complete 500k 7 MHz
	751500, // Complete 500k 7 MHz
	752000, // Complete 500k 7 MHz
	752500, // Complete 500k 7 MHz
	753000, // Complete 500k 7 MHz
	753500, // Complete 500k 7 MHz
	754000, // Complete 500k 7 MHz
	754500, // Complete 500k 7 MHz
	755000, // Complete 500k 7 MHz
	755500, // Complete 500k 7 MHz
	756000, // Complete 500k 7 MHz
	756500, // Complete 500k 7 MHz
	757000, // Complete 500k 7 MHz
	757500, // Complete 500k 7 MHz
	758000, // Complete 500k 7 MHz
	758500, // Complete 500k 7 MHz
	759000, // Complete 500k 7 MHz
	759500, // Complete 500k 7 MHz
	760000, // Complete 500k 7 MHz
	760500, // Complete 500k 7 MHz
	761000, // Complete 500k 7 MHz
	761500, // Complete 500k 7 MHz
	762000, // Complete 500k 7 MHz
	762500, // Complete 500k 7 MHz
	763000, // Complete 500k 7 MHz
	763500, // Complete 500k 7 MHz
	764000, // Complete 500k 7 MHz
	764500, // Complete 500k 7 MHz
	765000, // Complete 500k 7 MHz
	765500, // Complete 500k 7 MHz
	766000, // Complete 500k 7 MHz
	766500, // Complete 500k 7 MHz
	767000, // Complete 500k 7 MHz
	767500, // Complete 500k 7 MHz
	768000, // Complete 500k 7 MHz
	768500, // Complete 500k 7 MHz
	769000, // Complete 500k 7 MHz
	769500, // Complete 500k 7 MHz
	770000, // Complete 500k 7 MHz
	770500, // Complete 500k 7 MHz
	771000, // Complete 500k 7 MHz
	771500, // Complete 500k 7 MHz
	772000, // Complete 500k 7 MHz
	772500, // Complete 500k 7 MHz
	773000, // Complete 500k 7 MHz
	773500, // Complete 500k 7 MHz
	774000, // Complete 500k 7 MHz
	774500, // Complete 500k 7 MHz
	775000, // Complete 500k 7 MHz
	775500, // Complete 500k 7 MHz
	776000, // Complete 500k 7 MHz
	776500, // Complete 500k 7 MHz
	777000, // Complete 500k 7 MHz
	777500, // Complete 500k 7 MHz
	778000, // Complete 500k 7 MHz
	778500, // Complete 500k 7 MHz
	779000, // Complete 500k 7 MHz
	779500, // Complete 500k 7 MHz
	780000, // Complete 500k 7 MHz
	780500, // Complete 500k 7 MHz
	781000, // Complete 500k 7 MHz
	781500, // Complete 500k 7 MHz
	782000, // Complete 500k 7 MHz
	782500, // Complete 500k 7 MHz
	783000, // Complete 500k 7 MHz
	783500, // Complete 500k 7 MHz
	784000, // Complete 500k 7 MHz
	784500, // Complete 500k 7 MHz
	785000, // Complete 500k 7 MHz
	785500, // Complete 500k 7 MHz
	786000, // Complete 500k 7 MHz
	786500, // Complete 500k 7 MHz
	787000, // Complete 500k 7 MHz
	787500, // Complete 500k 7 MHz
	788000, // Complete 500k 7 MHz
	788500, // Complete 500k 7 MHz
	789000, // Complete 500k 7 MHz
	789500, // Complete 500k 7 MHz
	790000, // Complete 500k 7 MHz
	790500, // Complete 500k 7 MHz
	791000, // Complete 500k 7 MHz
	791500, // Complete 500k 7 MHz
	792000, // Complete 500k 7 MHz
	792500, // Complete 500k 7 MHz
	793000, // Complete 500k 7 MHz
	793500, // Complete 500k 7 MHz
	794000, // Complete 500k 7 MHz
	794500, // Complete 500k 7 MHz
	795000, // Complete 500k 7 MHz
	795500, // Complete 500k 7 MHz
	796000, // Complete 500k 7 MHz
	796500, // Complete 500k 7 MHz
	797000, // Complete 500k 7 MHz
	797500, // Complete 500k 7 MHz
	798000, // Complete 500k 7 MHz
	798500, // Complete 500k 7 MHz
	799000, // Complete 500k 7 MHz
	799500, // Complete 500k 7 MHz
	800000, // Complete 500k 7 MHz
	800500, // Complete 500k 7 MHz
	801000, // Complete 500k 7 MHz
	801500, // Complete 500k 7 MHz
	802000, // Complete 500k 7 MHz
	802500, // Complete 500k 7 MHz
	803000, // Complete 500k 7 MHz
	803500, // Complete 500k 7 MHz
	804000, // Complete 500k 7 MHz
	804500, // Complete 500k 7 MHz
	805000, // Complete 500k 7 MHz
	805500, // Complete 500k 7 MHz
	806000, // Complete 500k 7 MHz
	806500, // Complete 500k 7 MHz
	807000, // Complete 500k 7 MHz
	807500, // Complete 500k 7 MHz
	808000, // Complete 500k 7 MHz
	808500, // Complete 500k 7 MHz
	809000, // Complete 500k 7 MHz
	809500, // Complete 500k 7 MHz
	810000, // Complete 500k 7 MHz
	810500, // Complete 500k 7 MHz
	811000, // Complete 500k 7 MHz
	811500, // Complete 500k 7 MHz
	812000, // Complete 500k 7 MHz
	812500, // Complete 500k 7 MHz
	813000, // Complete 500k 7 MHz
	813500, // Complete 500k 7 MHz
	814000, // Complete 500k 7 MHz
	814500, // Complete 500k 7 MHz
	815000, // Complete 500k 7 MHz
	815500, // Complete 500k 7 MHz
	816000, // Complete 500k 7 MHz
	816500, // Complete 500k 7 MHz
	817000, // Complete 500k 7 MHz
	817500, // Complete 500k 7 MHz
	818000, // Complete 500k 7 MHz
	818500, // Complete 500k 7 MHz
	819000, // Complete 500k 7 MHz
	819500, // Complete 500k 7 MHz
	820000, // Complete 500k 7 MHz
	820500, // Complete 500k 7 MHz
	821000, // Complete 500k 7 MHz
	821500, // Complete 500k 7 MHz
	822000, // Complete 500k 7 MHz
	822500, // Complete 500k 7 MHz
	823000, // Complete 500k 7 MHz
	823500, // Complete 500k 7 MHz
	824000, // Complete 500k 7 MHz
	824500, // Complete 500k 7 MHz
	825000, // Complete 500k 7 MHz
	825500, // Complete 500k 7 MHz
	826000, // Complete 500k 7 MHz
	826500, // Complete 500k 7 MHz
	827000, // Complete 500k 7 MHz
	827500, // Complete 500k 7 MHz
	828000, // Complete 500k 7 MHz
	828500, // Complete 500k 7 MHz
	829000, // Complete 500k 7 MHz
	829500, // Complete 500k 7 MHz
	830000, // Complete 500k 7 MHz
	830500, // Complete 500k 7 MHz
	831000, // Complete 500k 7 MHz
	831500, // Complete 500k 7 MHz
	832000, // Complete 500k 7 MHz
	832500, // Complete 500k 7 MHz
	833000, // Complete 500k 7 MHz
	833500, // Complete 500k 7 MHz
	834000, // Complete 500k 7 MHz
	834500, // Complete 500k 7 MHz
	835000, // Complete 500k 7 MHz
	835500, // Complete 500k 7 MHz
	836000, // Complete 500k 7 MHz
	836500, // Complete 500k 7 MHz
	837000, // Complete 500k 7 MHz
	837500, // Complete 500k 7 MHz
	838000, // Complete 500k 7 MHz
	838500, // Complete 500k 7 MHz
	839000, // Complete 500k 7 MHz
	839500, // Complete 500k 7 MHz
	840000, // Complete 500k 7 MHz
	840500, // Complete 500k 7 MHz
	841000, // Complete 500k 7 MHz
	841500, // Complete 500k 7 MHz
	842000, // Complete 500k 7 MHz
	842500, // Complete 500k 7 MHz
	843000, // Complete 500k 7 MHz
	843500, // Complete 500k 7 MHz
	844000, // Complete 500k 7 MHz
	844500, // Complete 500k 7 MHz
	845000, // Complete 500k 7 MHz
	845500, // Complete 500k 7 MHz
	846000, // Complete 500k 7 MHz
	846500, // Complete 500k 7 MHz
	847000, // Complete 500k 7 MHz
	847500, // Complete 500k 7 MHz
	848000, // Complete 500k 7 MHz
	848500, // Complete 500k 7 MHz
	849000, // Complete 500k 7 MHz
	849500, // Complete 500k 7 MHz
	850000, // Complete 500k 7 MHz
	850500, // Complete 500k 7 MHz
	851000, // Complete 500k 7 MHz
	851500, // Complete 500k 7 MHz
	852000, // Complete 500k 7 MHz
	852500, // Complete 500k 7 MHz
	853000, // Complete 500k 7 MHz
	853500, // Complete 500k 7 MHz
	854000, // Complete 500k 7 MHz
	854500, // Complete 500k 7 MHz
	855000, // Complete 500k 7 MHz
	855500, // Complete 500k 7 MHz
	856000, // Complete 500k 7 MHz
	856500, // Complete 500k 7 MHz
	857000, // Complete 500k 7 MHz
	857500, // Complete 500k 7 MHz
	858000, // Complete 500k 7 MHz
	858500, // Complete 500k 7 MHz
	0,		// ******************************* end of Complete 500k 7 MHz
	111000, // Complete 500k 8 MHz
	111500, // Complete 500k 8 MHz
	112000, // Complete 500k 8 MHz
	112500, // Complete 500k 8 MHz
	113000, // Complete 500k 8 MHz
	113500, // Complete 500k 8 MHz
	114000, // Complete 500k 8 MHz
	114500, // Complete 500k 8 MHz
	115000, // Complete 500k 8 MHz
	115500, // Complete 500k 8 MHz
	116000, // Complete 500k 8 MHz
	116500, // Complete 500k 8 MHz
	117000, // Complete 500k 8 MHz
	117500, // Complete 500k 8 MHz
	118000, // Complete 500k 8 MHz
	118500, // Complete 500k 8 MHz
	119000, // Complete 500k 8 MHz
	119500, // Complete 500k 8 MHz
	120000, // Complete 500k 8 MHz
	120500, // Complete 500k 8 MHz
	121000, // Complete 500k 8 MHz
	121500, // Complete 500k 8 MHz
	122000, // Complete 500k 8 MHz
	122500, // Complete 500k 8 MHz
	123000, // Complete 500k 8 MHz
	123500, // Complete 500k 8 MHz
	124000, // Complete 500k 8 MHz
	124500, // Complete 500k 8 MHz
	125000, // Complete 500k 8 MHz
	125500, // Complete 500k 8 MHz
	126000, // Complete 500k 8 MHz
	126500, // Complete 500k 8 MHz
	127000, // Complete 500k 8 MHz
	127500, // Complete 500k 8 MHz
	128000, // Complete 500k 8 MHz
	128500, // Complete 500k 8 MHz
	129000, // Complete 500k 8 MHz
	129500, // Complete 500k 8 MHz
	130000, // Complete 500k 8 MHz
	130500, // Complete 500k 8 MHz
	131000, // Complete 500k 8 MHz
	131500, // Complete 500k 8 MHz
	132000, // Complete 500k 8 MHz
	132500, // Complete 500k 8 MHz
	133000, // Complete 500k 8 MHz
	133500, // Complete 500k 8 MHz
	134000, // Complete 500k 8 MHz
	134500, // Complete 500k 8 MHz
	135000, // Complete 500k 8 MHz
	135500, // Complete 500k 8 MHz
	136000, // Complete 500k 8 MHz
	136500, // Complete 500k 8 MHz
	137000, // Complete 500k 8 MHz
	137500, // Complete 500k 8 MHz
	138000, // Complete 500k 8 MHz
	138500, // Complete 500k 8 MHz
	139000, // Complete 500k 8 MHz
	139500, // Complete 500k 8 MHz
	140000, // Complete 500k 8 MHz
	140500, // Complete 500k 8 MHz
	141000, // Complete 500k 8 MHz
	141500, // Complete 500k 8 MHz
	142000, // Complete 500k 8 MHz
	142500, // Complete 500k 8 MHz
	143000, // Complete 500k 8 MHz
	143500, // Complete 500k 8 MHz
	144000, // Complete 500k 8 MHz
	144500, // Complete 500k 8 MHz
	145000, // Complete 500k 8 MHz
	145500, // Complete 500k 8 MHz
	146000, // Complete 500k 8 MHz
	146500, // Complete 500k 8 MHz
	147000, // Complete 500k 8 MHz
	147500, // Complete 500k 8 MHz
	148000, // Complete 500k 8 MHz
	148500, // Complete 500k 8 MHz
	149000, // Complete 500k 8 MHz
	149500, // Complete 500k 8 MHz
	150000, // Complete 500k 8 MHz
	150500, // Complete 500k 8 MHz
	151000, // Complete 500k 8 MHz
	151500, // Complete 500k 8 MHz
	152000, // Complete 500k 8 MHz
	152500, // Complete 500k 8 MHz
	153000, // Complete 500k 8 MHz
	153500, // Complete 500k 8 MHz
	154000, // Complete 500k 8 MHz
	154500, // Complete 500k 8 MHz
	155000, // Complete 500k 8 MHz
	155500, // Complete 500k 8 MHz
	156000, // Complete 500k 8 MHz
	156500, // Complete 500k 8 MHz
	157000, // Complete 500k 8 MHz
	157500, // Complete 500k 8 MHz
	158000, // Complete 500k 8 MHz
	158500, // Complete 500k 8 MHz
	159000, // Complete 500k 8 MHz
	159500, // Complete 500k 8 MHz
	160000, // Complete 500k 8 MHz
	160500, // Complete 500k 8 MHz
	161000, // Complete 500k 8 MHz
	161500, // Complete 500k 8 MHz
	162000, // Complete 500k 8 MHz
	162500, // Complete 500k 8 MHz
	163000, // Complete 500k 8 MHz
	163500, // Complete 500k 8 MHz
	164000, // Complete 500k 8 MHz
	164500, // Complete 500k 8 MHz
	165000, // Complete 500k 8 MHz
	165500, // Complete 500k 8 MHz
	166000, // Complete 500k 8 MHz
	166500, // Complete 500k 8 MHz
	167000, // Complete 500k 8 MHz
	167500, // Complete 500k 8 MHz
	168000, // Complete 500k 8 MHz
	168500, // Complete 500k 8 MHz
	169000, // Complete 500k 8 MHz
	169500, // Complete 500k 8 MHz
	170000, // Complete 500k 8 MHz
	170500, // Complete 500k 8 MHz
	171000, // Complete 500k 8 MHz
	171500, // Complete 500k 8 MHz
	172000, // Complete 500k 8 MHz
	172500, // Complete 500k 8 MHz
	173000, // Complete 500k 8 MHz
	173500, // Complete 500k 8 MHz
	174000, // Complete 500k 8 MHz
	174500, // Complete 500k 8 MHz
	175000, // Complete 500k 8 MHz
	175500, // Complete 500k 8 MHz
	176000, // Complete 500k 8 MHz
	176500, // Complete 500k 8 MHz
	177000, // Complete 500k 8 MHz
	177500, // Complete 500k 8 MHz
	178000, // Complete 500k 8 MHz
	178500, // Complete 500k 8 MHz
	179000, // Complete 500k 8 MHz
	179500, // Complete 500k 8 MHz
	180000, // Complete 500k 8 MHz
	180500, // Complete 500k 8 MHz
	181000, // Complete 500k 8 MHz
	181500, // Complete 500k 8 MHz
	182000, // Complete 500k 8 MHz
	182500, // Complete 500k 8 MHz
	183000, // Complete 500k 8 MHz
	183500, // Complete 500k 8 MHz
	184000, // Complete 500k 8 MHz
	184500, // Complete 500k 8 MHz
	185000, // Complete 500k 8 MHz
	185500, // Complete 500k 8 MHz
	186000, // Complete 500k 8 MHz
	186500, // Complete 500k 8 MHz
	187000, // Complete 500k 8 MHz
	187500, // Complete 500k 8 MHz
	188000, // Complete 500k 8 MHz
	188500, // Complete 500k 8 MHz
	189000, // Complete 500k 8 MHz
	189500, // Complete 500k 8 MHz
	190000, // Complete 500k 8 MHz
	190500, // Complete 500k 8 MHz
	191000, // Complete 500k 8 MHz
	191500, // Complete 500k 8 MHz
	192000, // Complete 500k 8 MHz
	192500, // Complete 500k 8 MHz
	193000, // Complete 500k 8 MHz
	193500, // Complete 500k 8 MHz
	194000, // Complete 500k 8 MHz
	194500, // Complete 500k 8 MHz
	195000, // Complete 500k 8 MHz
	195500, // Complete 500k 8 MHz
	196000, // Complete 500k 8 MHz
	196500, // Complete 500k 8 MHz
	197000, // Complete 500k 8 MHz
	197500, // Complete 500k 8 MHz
	198000, // Complete 500k 8 MHz
	198500, // Complete 500k 8 MHz
	199000, // Complete 500k 8 MHz
	199500, // Complete 500k 8 MHz
	200000, // Complete 500k 8 MHz
	200500, // Complete 500k 8 MHz
	201000, // Complete 500k 8 MHz
	201500, // Complete 500k 8 MHz
	202000, // Complete 500k 8 MHz
	202500, // Complete 500k 8 MHz
	203000, // Complete 500k 8 MHz
	203500, // Complete 500k 8 MHz
	204000, // Complete 500k 8 MHz
	204500, // Complete 500k 8 MHz
	205000, // Complete 500k 8 MHz
	205500, // Complete 500k 8 MHz
	206000, // Complete 500k 8 MHz
	206500, // Complete 500k 8 MHz
	207000, // Complete 500k 8 MHz
	207500, // Complete 500k 8 MHz
	208000, // Complete 500k 8 MHz
	208500, // Complete 500k 8 MHz
	209000, // Complete 500k 8 MHz
	209500, // Complete 500k 8 MHz
	210000, // Complete 500k 8 MHz
	210500, // Complete 500k 8 MHz
	211000, // Complete 500k 8 MHz
	211500, // Complete 500k 8 MHz
	212000, // Complete 500k 8 MHz
	212500, // Complete 500k 8 MHz
	213000, // Complete 500k 8 MHz
	213500, // Complete 500k 8 MHz
	214000, // Complete 500k 8 MHz
	214500, // Complete 500k 8 MHz
	215000, // Complete 500k 8 MHz
	215500, // Complete 500k 8 MHz
	216000, // Complete 500k 8 MHz
	216500, // Complete 500k 8 MHz
	217000, // Complete 500k 8 MHz
	217500, // Complete 500k 8 MHz
	218000, // Complete 500k 8 MHz
	218500, // Complete 500k 8 MHz
	219000, // Complete 500k 8 MHz
	219500, // Complete 500k 8 MHz
	220000, // Complete 500k 8 MHz
	220500, // Complete 500k 8 MHz
	221000, // Complete 500k 8 MHz
	221500, // Complete 500k 8 MHz
	222000, // Complete 500k 8 MHz
	222500, // Complete 500k 8 MHz
	223000, // Complete 500k 8 MHz
	223500, // Complete 500k 8 MHz
	224000, // Complete 500k 8 MHz
	224500, // Complete 500k 8 MHz
	225000, // Complete 500k 8 MHz
	225500, // Complete 500k 8 MHz
	226000, // Complete 500k 8 MHz
	226500, // Complete 500k 8 MHz
	227000, // Complete 500k 8 MHz
	227500, // Complete 500k 8 MHz
	228000, // Complete 500k 8 MHz
	228500, // Complete 500k 8 MHz
	229000, // Complete 500k 8 MHz
	229500, // Complete 500k 8 MHz
	230000, // Complete 500k 8 MHz
	230500, // Complete 500k 8 MHz
	231000, // Complete 500k 8 MHz
	231500, // Complete 500k 8 MHz
	232000, // Complete 500k 8 MHz
	232500, // Complete 500k 8 MHz
	233000, // Complete 500k 8 MHz
	233500, // Complete 500k 8 MHz
	234000, // Complete 500k 8 MHz
	234500, // Complete 500k 8 MHz
	235000, // Complete 500k 8 MHz
	235500, // Complete 500k 8 MHz
	236000, // Complete 500k 8 MHz
	236500, // Complete 500k 8 MHz
	237000, // Complete 500k 8 MHz
	237500, // Complete 500k 8 MHz
	238000, // Complete 500k 8 MHz
	238500, // Complete 500k 8 MHz
	239000, // Complete 500k 8 MHz
	239500, // Complete 500k 8 MHz
	240000, // Complete 500k 8 MHz
	240500, // Complete 500k 8 MHz
	241000, // Complete 500k 8 MHz
	241500, // Complete 500k 8 MHz
	242000, // Complete 500k 8 MHz
	242500, // Complete 500k 8 MHz
	243000, // Complete 500k 8 MHz
	243500, // Complete 500k 8 MHz
	244000, // Complete 500k 8 MHz
	244500, // Complete 500k 8 MHz
	245000, // Complete 500k 8 MHz
	245500, // Complete 500k 8 MHz
	246000, // Complete 500k 8 MHz
	246500, // Complete 500k 8 MHz
	247000, // Complete 500k 8 MHz
	247500, // Complete 500k 8 MHz
	248000, // Complete 500k 8 MHz
	248500, // Complete 500k 8 MHz
	249000, // Complete 500k 8 MHz
	249500, // Complete 500k 8 MHz
	250000, // Complete 500k 8 MHz
	250500, // Complete 500k 8 MHz
	251000, // Complete 500k 8 MHz
	251500, // Complete 500k 8 MHz
	252000, // Complete 500k 8 MHz
	252500, // Complete 500k 8 MHz
	253000, // Complete 500k 8 MHz
	253500, // Complete 500k 8 MHz
	254000, // Complete 500k 8 MHz
	254500, // Complete 500k 8 MHz
	255000, // Complete 500k 8 MHz
	255500, // Complete 500k 8 MHz
	256000, // Complete 500k 8 MHz
	256500, // Complete 500k 8 MHz
	257000, // Complete 500k 8 MHz
	257500, // Complete 500k 8 MHz
	258000, // Complete 500k 8 MHz
	258500, // Complete 500k 8 MHz
	259000, // Complete 500k 8 MHz
	259500, // Complete 500k 8 MHz
	260000, // Complete 500k 8 MHz
	260500, // Complete 500k 8 MHz
	261000, // Complete 500k 8 MHz
	261500, // Complete 500k 8 MHz
	262000, // Complete 500k 8 MHz
	262500, // Complete 500k 8 MHz
	263000, // Complete 500k 8 MHz
	263500, // Complete 500k 8 MHz
	264000, // Complete 500k 8 MHz
	264500, // Complete 500k 8 MHz
	265000, // Complete 500k 8 MHz
	265500, // Complete 500k 8 MHz
	266000, // Complete 500k 8 MHz
	266500, // Complete 500k 8 MHz
	267000, // Complete 500k 8 MHz
	267500, // Complete 500k 8 MHz
	268000, // Complete 500k 8 MHz
	268500, // Complete 500k 8 MHz
	269000, // Complete 500k 8 MHz
	269500, // Complete 500k 8 MHz
	270000, // Complete 500k 8 MHz
	270500, // Complete 500k 8 MHz
	271000, // Complete 500k 8 MHz
	271500, // Complete 500k 8 MHz
	272000, // Complete 500k 8 MHz
	272500, // Complete 500k 8 MHz
	273000, // Complete 500k 8 MHz
	273500, // Complete 500k 8 MHz
	274000, // Complete 500k 8 MHz
	274500, // Complete 500k 8 MHz
	275000, // Complete 500k 8 MHz
	275500, // Complete 500k 8 MHz
	276000, // Complete 500k 8 MHz
	276500, // Complete 500k 8 MHz
	277000, // Complete 500k 8 MHz
	277500, // Complete 500k 8 MHz
	278000, // Complete 500k 8 MHz
	278500, // Complete 500k 8 MHz
	279000, // Complete 500k 8 MHz
	279500, // Complete 500k 8 MHz
	280000, // Complete 500k 8 MHz
	280500, // Complete 500k 8 MHz
	281000, // Complete 500k 8 MHz
	281500, // Complete 500k 8 MHz
	282000, // Complete 500k 8 MHz
	282500, // Complete 500k 8 MHz
	283000, // Complete 500k 8 MHz
	283500, // Complete 500k 8 MHz
	284000, // Complete 500k 8 MHz
	284500, // Complete 500k 8 MHz
	285000, // Complete 500k 8 MHz
	285500, // Complete 500k 8 MHz
	286000, // Complete 500k 8 MHz
	286500, // Complete 500k 8 MHz
	287000, // Complete 500k 8 MHz
	287500, // Complete 500k 8 MHz
	288000, // Complete 500k 8 MHz
	288500, // Complete 500k 8 MHz
	289000, // Complete 500k 8 MHz
	289500, // Complete 500k 8 MHz
	290000, // Complete 500k 8 MHz
	290500, // Complete 500k 8 MHz
	291000, // Complete 500k 8 MHz
	291500, // Complete 500k 8 MHz
	292000, // Complete 500k 8 MHz
	292500, // Complete 500k 8 MHz
	293000, // Complete 500k 8 MHz
	293500, // Complete 500k 8 MHz
	294000, // Complete 500k 8 MHz
	294500, // Complete 500k 8 MHz
	295000, // Complete 500k 8 MHz
	295500, // Complete 500k 8 MHz
	296000, // Complete 500k 8 MHz
	296500, // Complete 500k 8 MHz
	297000, // Complete 500k 8 MHz
	297500, // Complete 500k 8 MHz
	298000, // Complete 500k 8 MHz
	298500, // Complete 500k 8 MHz
	299000, // Complete 500k 8 MHz
	299500, // Complete 500k 8 MHz
	300000, // Complete 500k 8 MHz
	300500, // Complete 500k 8 MHz
	301000, // Complete 500k 8 MHz
	301500, // Complete 500k 8 MHz
	302000, // Complete 500k 8 MHz
	302500, // Complete 500k 8 MHz
	303000, // Complete 500k 8 MHz
	303500, // Complete 500k 8 MHz
	304000, // Complete 500k 8 MHz
	304500, // Complete 500k 8 MHz
	305000, // Complete 500k 8 MHz
	305500, // Complete 500k 8 MHz
	306000, // Complete 500k 8 MHz
	306500, // Complete 500k 8 MHz
	307000, // Complete 500k 8 MHz
	307500, // Complete 500k 8 MHz
	308000, // Complete 500k 8 MHz
	308500, // Complete 500k 8 MHz
	309000, // Complete 500k 8 MHz
	309500, // Complete 500k 8 MHz
	310000, // Complete 500k 8 MHz
	310500, // Complete 500k 8 MHz
	311000, // Complete 500k 8 MHz
	311500, // Complete 500k 8 MHz
	312000, // Complete 500k 8 MHz
	312500, // Complete 500k 8 MHz
	313000, // Complete 500k 8 MHz
	313500, // Complete 500k 8 MHz
	314000, // Complete 500k 8 MHz
	314500, // Complete 500k 8 MHz
	315000, // Complete 500k 8 MHz
	315500, // Complete 500k 8 MHz
	316000, // Complete 500k 8 MHz
	316500, // Complete 500k 8 MHz
	317000, // Complete 500k 8 MHz
	317500, // Complete 500k 8 MHz
	318000, // Complete 500k 8 MHz
	318500, // Complete 500k 8 MHz
	319000, // Complete 500k 8 MHz
	319500, // Complete 500k 8 MHz
	320000, // Complete 500k 8 MHz
	320500, // Complete 500k 8 MHz
	321000, // Complete 500k 8 MHz
	321500, // Complete 500k 8 MHz
	322000, // Complete 500k 8 MHz
	322500, // Complete 500k 8 MHz
	323000, // Complete 500k 8 MHz
	323500, // Complete 500k 8 MHz
	324000, // Complete 500k 8 MHz
	324500, // Complete 500k 8 MHz
	325000, // Complete 500k 8 MHz
	325500, // Complete 500k 8 MHz
	326000, // Complete 500k 8 MHz
	326500, // Complete 500k 8 MHz
	327000, // Complete 500k 8 MHz
	327500, // Complete 500k 8 MHz
	328000, // Complete 500k 8 MHz
	328500, // Complete 500k 8 MHz
	329000, // Complete 500k 8 MHz
	329500, // Complete 500k 8 MHz
	330000, // Complete 500k 8 MHz
	330500, // Complete 500k 8 MHz
	331000, // Complete 500k 8 MHz
	331500, // Complete 500k 8 MHz
	332000, // Complete 500k 8 MHz
	332500, // Complete 500k 8 MHz
	333000, // Complete 500k 8 MHz
	333500, // Complete 500k 8 MHz
	334000, // Complete 500k 8 MHz
	334500, // Complete 500k 8 MHz
	335000, // Complete 500k 8 MHz
	335500, // Complete 500k 8 MHz
	336000, // Complete 500k 8 MHz
	336500, // Complete 500k 8 MHz
	337000, // Complete 500k 8 MHz
	337500, // Complete 500k 8 MHz
	338000, // Complete 500k 8 MHz
	338500, // Complete 500k 8 MHz
	339000, // Complete 500k 8 MHz
	339500, // Complete 500k 8 MHz
	340000, // Complete 500k 8 MHz
	340500, // Complete 500k 8 MHz
	341000, // Complete 500k 8 MHz
	341500, // Complete 500k 8 MHz
	342000, // Complete 500k 8 MHz
	342500, // Complete 500k 8 MHz
	343000, // Complete 500k 8 MHz
	343500, // Complete 500k 8 MHz
	344000, // Complete 500k 8 MHz
	344500, // Complete 500k 8 MHz
	345000, // Complete 500k 8 MHz
	345500, // Complete 500k 8 MHz
	346000, // Complete 500k 8 MHz
	346500, // Complete 500k 8 MHz
	347000, // Complete 500k 8 MHz
	347500, // Complete 500k 8 MHz
	348000, // Complete 500k 8 MHz
	348500, // Complete 500k 8 MHz
	349000, // Complete 500k 8 MHz
	349500, // Complete 500k 8 MHz
	350000, // Complete 500k 8 MHz
	350500, // Complete 500k 8 MHz
	351000, // Complete 500k 8 MHz
	351500, // Complete 500k 8 MHz
	352000, // Complete 500k 8 MHz
	352500, // Complete 500k 8 MHz
	353000, // Complete 500k 8 MHz
	353500, // Complete 500k 8 MHz
	354000, // Complete 500k 8 MHz
	354500, // Complete 500k 8 MHz
	355000, // Complete 500k 8 MHz
	355500, // Complete 500k 8 MHz
	356000, // Complete 500k 8 MHz
	356500, // Complete 500k 8 MHz
	357000, // Complete 500k 8 MHz
	357500, // Complete 500k 8 MHz
	358000, // Complete 500k 8 MHz
	358500, // Complete 500k 8 MHz
	359000, // Complete 500k 8 MHz
	359500, // Complete 500k 8 MHz
	360000, // Complete 500k 8 MHz
	360500, // Complete 500k 8 MHz
	361000, // Complete 500k 8 MHz
	361500, // Complete 500k 8 MHz
	362000, // Complete 500k 8 MHz
	362500, // Complete 500k 8 MHz
	363000, // Complete 500k 8 MHz
	363500, // Complete 500k 8 MHz
	364000, // Complete 500k 8 MHz
	364500, // Complete 500k 8 MHz
	365000, // Complete 500k 8 MHz
	365500, // Complete 500k 8 MHz
	366000, // Complete 500k 8 MHz
	366500, // Complete 500k 8 MHz
	367000, // Complete 500k 8 MHz
	367500, // Complete 500k 8 MHz
	368000, // Complete 500k 8 MHz
	368500, // Complete 500k 8 MHz
	369000, // Complete 500k 8 MHz
	369500, // Complete 500k 8 MHz
	370000, // Complete 500k 8 MHz
	370500, // Complete 500k 8 MHz
	371000, // Complete 500k 8 MHz
	371500, // Complete 500k 8 MHz
	372000, // Complete 500k 8 MHz
	372500, // Complete 500k 8 MHz
	373000, // Complete 500k 8 MHz
	373500, // Complete 500k 8 MHz
	374000, // Complete 500k 8 MHz
	374500, // Complete 500k 8 MHz
	375000, // Complete 500k 8 MHz
	375500, // Complete 500k 8 MHz
	376000, // Complete 500k 8 MHz
	376500, // Complete 500k 8 MHz
	377000, // Complete 500k 8 MHz
	377500, // Complete 500k 8 MHz
	378000, // Complete 500k 8 MHz
	378500, // Complete 500k 8 MHz
	379000, // Complete 500k 8 MHz
	379500, // Complete 500k 8 MHz
	380000, // Complete 500k 8 MHz
	380500, // Complete 500k 8 MHz
	381000, // Complete 500k 8 MHz
	381500, // Complete 500k 8 MHz
	382000, // Complete 500k 8 MHz
	382500, // Complete 500k 8 MHz
	383000, // Complete 500k 8 MHz
	383500, // Complete 500k 8 MHz
	384000, // Complete 500k 8 MHz
	384500, // Complete 500k 8 MHz
	385000, // Complete 500k 8 MHz
	385500, // Complete 500k 8 MHz
	386000, // Complete 500k 8 MHz
	386500, // Complete 500k 8 MHz
	387000, // Complete 500k 8 MHz
	387500, // Complete 500k 8 MHz
	388000, // Complete 500k 8 MHz
	388500, // Complete 500k 8 MHz
	389000, // Complete 500k 8 MHz
	389500, // Complete 500k 8 MHz
	390000, // Complete 500k 8 MHz
	390500, // Complete 500k 8 MHz
	391000, // Complete 500k 8 MHz
	391500, // Complete 500k 8 MHz
	392000, // Complete 500k 8 MHz
	392500, // Complete 500k 8 MHz
	393000, // Complete 500k 8 MHz
	393500, // Complete 500k 8 MHz
	394000, // Complete 500k 8 MHz
	394500, // Complete 500k 8 MHz
	395000, // Complete 500k 8 MHz
	395500, // Complete 500k 8 MHz
	396000, // Complete 500k 8 MHz
	396500, // Complete 500k 8 MHz
	397000, // Complete 500k 8 MHz
	397500, // Complete 500k 8 MHz
	398000, // Complete 500k 8 MHz
	398500, // Complete 500k 8 MHz
	399000, // Complete 500k 8 MHz
	399500, // Complete 500k 8 MHz
	400000, // Complete 500k 8 MHz
	400500, // Complete 500k 8 MHz
	401000, // Complete 500k 8 MHz
	401500, // Complete 500k 8 MHz
	402000, // Complete 500k 8 MHz
	402500, // Complete 500k 8 MHz
	403000, // Complete 500k 8 MHz
	403500, // Complete 500k 8 MHz
	404000, // Complete 500k 8 MHz
	404500, // Complete 500k 8 MHz
	405000, // Complete 500k 8 MHz
	405500, // Complete 500k 8 MHz
	406000, // Complete 500k 8 MHz
	406500, // Complete 500k 8 MHz
	407000, // Complete 500k 8 MHz
	407500, // Complete 500k 8 MHz
	408000, // Complete 500k 8 MHz
	408500, // Complete 500k 8 MHz
	409000, // Complete 500k 8 MHz
	409500, // Complete 500k 8 MHz
	410000, // Complete 500k 8 MHz
	410500, // Complete 500k 8 MHz
	411000, // Complete 500k 8 MHz
	411500, // Complete 500k 8 MHz
	412000, // Complete 500k 8 MHz
	412500, // Complete 500k 8 MHz
	413000, // Complete 500k 8 MHz
	413500, // Complete 500k 8 MHz
	414000, // Complete 500k 8 MHz
	414500, // Complete 500k 8 MHz
	415000, // Complete 500k 8 MHz
	415500, // Complete 500k 8 MHz
	416000, // Complete 500k 8 MHz
	416500, // Complete 500k 8 MHz
	417000, // Complete 500k 8 MHz
	417500, // Complete 500k 8 MHz
	418000, // Complete 500k 8 MHz
	418500, // Complete 500k 8 MHz
	419000, // Complete 500k 8 MHz
	419500, // Complete 500k 8 MHz
	420000, // Complete 500k 8 MHz
	420500, // Complete 500k 8 MHz
	421000, // Complete 500k 8 MHz
	421500, // Complete 500k 8 MHz
	422000, // Complete 500k 8 MHz
	422500, // Complete 500k 8 MHz
	423000, // Complete 500k 8 MHz
	423500, // Complete 500k 8 MHz
	424000, // Complete 500k 8 MHz
	424500, // Complete 500k 8 MHz
	425000, // Complete 500k 8 MHz
	425500, // Complete 500k 8 MHz
	426000, // Complete 500k 8 MHz
	426500, // Complete 500k 8 MHz
	427000, // Complete 500k 8 MHz
	427500, // Complete 500k 8 MHz
	428000, // Complete 500k 8 MHz
	428500, // Complete 500k 8 MHz
	429000, // Complete 500k 8 MHz
	429500, // Complete 500k 8 MHz
	430000, // Complete 500k 8 MHz
	430500, // Complete 500k 8 MHz
	431000, // Complete 500k 8 MHz
	431500, // Complete 500k 8 MHz
	432000, // Complete 500k 8 MHz
	432500, // Complete 500k 8 MHz
	433000, // Complete 500k 8 MHz
	433500, // Complete 500k 8 MHz
	434000, // Complete 500k 8 MHz
	434500, // Complete 500k 8 MHz
	435000, // Complete 500k 8 MHz
	435500, // Complete 500k 8 MHz
	436000, // Complete 500k 8 MHz
	436500, // Complete 500k 8 MHz
	437000, // Complete 500k 8 MHz
	437500, // Complete 500k 8 MHz
	438000, // Complete 500k 8 MHz
	438500, // Complete 500k 8 MHz
	439000, // Complete 500k 8 MHz
	439500, // Complete 500k 8 MHz
	440000, // Complete 500k 8 MHz
	440500, // Complete 500k 8 MHz
	441000, // Complete 500k 8 MHz
	441500, // Complete 500k 8 MHz
	442000, // Complete 500k 8 MHz
	442500, // Complete 500k 8 MHz
	443000, // Complete 500k 8 MHz
	443500, // Complete 500k 8 MHz
	444000, // Complete 500k 8 MHz
	444500, // Complete 500k 8 MHz
	445000, // Complete 500k 8 MHz
	445500, // Complete 500k 8 MHz
	446000, // Complete 500k 8 MHz
	446500, // Complete 500k 8 MHz
	447000, // Complete 500k 8 MHz
	447500, // Complete 500k 8 MHz
	448000, // Complete 500k 8 MHz
	448500, // Complete 500k 8 MHz
	449000, // Complete 500k 8 MHz
	449500, // Complete 500k 8 MHz
	450000, // Complete 500k 8 MHz
	450500, // Complete 500k 8 MHz
	451000, // Complete 500k 8 MHz
	451500, // Complete 500k 8 MHz
	452000, // Complete 500k 8 MHz
	452500, // Complete 500k 8 MHz
	453000, // Complete 500k 8 MHz
	453500, // Complete 500k 8 MHz
	454000, // Complete 500k 8 MHz
	454500, // Complete 500k 8 MHz
	455000, // Complete 500k 8 MHz
	455500, // Complete 500k 8 MHz
	456000, // Complete 500k 8 MHz
	456500, // Complete 500k 8 MHz
	457000, // Complete 500k 8 MHz
	457500, // Complete 500k 8 MHz
	458000, // Complete 500k 8 MHz
	458500, // Complete 500k 8 MHz
	459000, // Complete 500k 8 MHz
	459500, // Complete 500k 8 MHz
	460000, // Complete 500k 8 MHz
	460500, // Complete 500k 8 MHz
	461000, // Complete 500k 8 MHz
	461500, // Complete 500k 8 MHz
	462000, // Complete 500k 8 MHz
	462500, // Complete 500k 8 MHz
	463000, // Complete 500k 8 MHz
	463500, // Complete 500k 8 MHz
	464000, // Complete 500k 8 MHz
	464500, // Complete 500k 8 MHz
	465000, // Complete 500k 8 MHz
	465500, // Complete 500k 8 MHz
	466000, // Complete 500k 8 MHz
	466500, // Complete 500k 8 MHz
	467000, // Complete 500k 8 MHz
	467500, // Complete 500k 8 MHz
	468000, // Complete 500k 8 MHz
	468500, // Complete 500k 8 MHz
	469000, // Complete 500k 8 MHz
	469500, // Complete 500k 8 MHz
	470000, // Complete 500k 8 MHz
	470500, // Complete 500k 8 MHz
	471000, // Complete 500k 8 MHz
	471500, // Complete 500k 8 MHz
	472000, // Complete 500k 8 MHz
	472500, // Complete 500k 8 MHz
	473000, // Complete 500k 8 MHz
	473500, // Complete 500k 8 MHz
	474000, // Complete 500k 8 MHz
	474500, // Complete 500k 8 MHz
	475000, // Complete 500k 8 MHz
	475500, // Complete 500k 8 MHz
	476000, // Complete 500k 8 MHz
	476500, // Complete 500k 8 MHz
	477000, // Complete 500k 8 MHz
	477500, // Complete 500k 8 MHz
	478000, // Complete 500k 8 MHz
	478500, // Complete 500k 8 MHz
	479000, // Complete 500k 8 MHz
	479500, // Complete 500k 8 MHz
	480000, // Complete 500k 8 MHz
	480500, // Complete 500k 8 MHz
	481000, // Complete 500k 8 MHz
	481500, // Complete 500k 8 MHz
	482000, // Complete 500k 8 MHz
	482500, // Complete 500k 8 MHz
	483000, // Complete 500k 8 MHz
	483500, // Complete 500k 8 MHz
	484000, // Complete 500k 8 MHz
	484500, // Complete 500k 8 MHz
	485000, // Complete 500k 8 MHz
	485500, // Complete 500k 8 MHz
	486000, // Complete 500k 8 MHz
	486500, // Complete 500k 8 MHz
	487000, // Complete 500k 8 MHz
	487500, // Complete 500k 8 MHz
	488000, // Complete 500k 8 MHz
	488500, // Complete 500k 8 MHz
	489000, // Complete 500k 8 MHz
	489500, // Complete 500k 8 MHz
	490000, // Complete 500k 8 MHz
	490500, // Complete 500k 8 MHz
	491000, // Complete 500k 8 MHz
	491500, // Complete 500k 8 MHz
	492000, // Complete 500k 8 MHz
	492500, // Complete 500k 8 MHz
	493000, // Complete 500k 8 MHz
	493500, // Complete 500k 8 MHz
	494000, // Complete 500k 8 MHz
	494500, // Complete 500k 8 MHz
	495000, // Complete 500k 8 MHz
	495500, // Complete 500k 8 MHz
	496000, // Complete 500k 8 MHz
	496500, // Complete 500k 8 MHz
	497000, // Complete 500k 8 MHz
	497500, // Complete 500k 8 MHz
	498000, // Complete 500k 8 MHz
	498500, // Complete 500k 8 MHz
	499000, // Complete 500k 8 MHz
	499500, // Complete 500k 8 MHz
	500000, // Complete 500k 8 MHz
	500500, // Complete 500k 8 MHz
	501000, // Complete 500k 8 MHz
	501500, // Complete 500k 8 MHz
	502000, // Complete 500k 8 MHz
	502500, // Complete 500k 8 MHz
	503000, // Complete 500k 8 MHz
	503500, // Complete 500k 8 MHz
	504000, // Complete 500k 8 MHz
	504500, // Complete 500k 8 MHz
	505000, // Complete 500k 8 MHz
	505500, // Complete 500k 8 MHz
	506000, // Complete 500k 8 MHz
	506500, // Complete 500k 8 MHz
	507000, // Complete 500k 8 MHz
	507500, // Complete 500k 8 MHz
	508000, // Complete 500k 8 MHz
	508500, // Complete 500k 8 MHz
	509000, // Complete 500k 8 MHz
	509500, // Complete 500k 8 MHz
	510000, // Complete 500k 8 MHz
	510500, // Complete 500k 8 MHz
	511000, // Complete 500k 8 MHz
	511500, // Complete 500k 8 MHz
	512000, // Complete 500k 8 MHz
	512500, // Complete 500k 8 MHz
	513000, // Complete 500k 8 MHz
	513500, // Complete 500k 8 MHz
	514000, // Complete 500k 8 MHz
	514500, // Complete 500k 8 MHz
	515000, // Complete 500k 8 MHz
	515500, // Complete 500k 8 MHz
	516000, // Complete 500k 8 MHz
	516500, // Complete 500k 8 MHz
	517000, // Complete 500k 8 MHz
	517500, // Complete 500k 8 MHz
	518000, // Complete 500k 8 MHz
	518500, // Complete 500k 8 MHz
	519000, // Complete 500k 8 MHz
	519500, // Complete 500k 8 MHz
	520000, // Complete 500k 8 MHz
	520500, // Complete 500k 8 MHz
	521000, // Complete 500k 8 MHz
	521500, // Complete 500k 8 MHz
	522000, // Complete 500k 8 MHz
	522500, // Complete 500k 8 MHz
	523000, // Complete 500k 8 MHz
	523500, // Complete 500k 8 MHz
	524000, // Complete 500k 8 MHz
	524500, // Complete 500k 8 MHz
	525000, // Complete 500k 8 MHz
	525500, // Complete 500k 8 MHz
	526000, // Complete 500k 8 MHz
	526500, // Complete 500k 8 MHz
	527000, // Complete 500k 8 MHz
	527500, // Complete 500k 8 MHz
	528000, // Complete 500k 8 MHz
	528500, // Complete 500k 8 MHz
	529000, // Complete 500k 8 MHz
	529500, // Complete 500k 8 MHz
	530000, // Complete 500k 8 MHz
	530500, // Complete 500k 8 MHz
	531000, // Complete 500k 8 MHz
	531500, // Complete 500k 8 MHz
	532000, // Complete 500k 8 MHz
	532500, // Complete 500k 8 MHz
	533000, // Complete 500k 8 MHz
	533500, // Complete 500k 8 MHz
	534000, // Complete 500k 8 MHz
	534500, // Complete 500k 8 MHz
	535000, // Complete 500k 8 MHz
	535500, // Complete 500k 8 MHz
	536000, // Complete 500k 8 MHz
	536500, // Complete 500k 8 MHz
	537000, // Complete 500k 8 MHz
	537500, // Complete 500k 8 MHz
	538000, // Complete 500k 8 MHz
	538500, // Complete 500k 8 MHz
	539000, // Complete 500k 8 MHz
	539500, // Complete 500k 8 MHz
	540000, // Complete 500k 8 MHz
	540500, // Complete 500k 8 MHz
	541000, // Complete 500k 8 MHz
	541500, // Complete 500k 8 MHz
	542000, // Complete 500k 8 MHz
	542500, // Complete 500k 8 MHz
	543000, // Complete 500k 8 MHz
	543500, // Complete 500k 8 MHz
	544000, // Complete 500k 8 MHz
	544500, // Complete 500k 8 MHz
	545000, // Complete 500k 8 MHz
	545500, // Complete 500k 8 MHz
	546000, // Complete 500k 8 MHz
	546500, // Complete 500k 8 MHz
	547000, // Complete 500k 8 MHz
	547500, // Complete 500k 8 MHz
	548000, // Complete 500k 8 MHz
	548500, // Complete 500k 8 MHz
	549000, // Complete 500k 8 MHz
	549500, // Complete 500k 8 MHz
	550000, // Complete 500k 8 MHz
	550500, // Complete 500k 8 MHz
	551000, // Complete 500k 8 MHz
	551500, // Complete 500k 8 MHz
	552000, // Complete 500k 8 MHz
	552500, // Complete 500k 8 MHz
	553000, // Complete 500k 8 MHz
	553500, // Complete 500k 8 MHz
	554000, // Complete 500k 8 MHz
	554500, // Complete 500k 8 MHz
	555000, // Complete 500k 8 MHz
	555500, // Complete 500k 8 MHz
	556000, // Complete 500k 8 MHz
	556500, // Complete 500k 8 MHz
	557000, // Complete 500k 8 MHz
	557500, // Complete 500k 8 MHz
	558000, // Complete 500k 8 MHz
	558500, // Complete 500k 8 MHz
	559000, // Complete 500k 8 MHz
	559500, // Complete 500k 8 MHz
	560000, // Complete 500k 8 MHz
	560500, // Complete 500k 8 MHz
	561000, // Complete 500k 8 MHz
	561500, // Complete 500k 8 MHz
	562000, // Complete 500k 8 MHz
	562500, // Complete 500k 8 MHz
	563000, // Complete 500k 8 MHz
	563500, // Complete 500k 8 MHz
	564000, // Complete 500k 8 MHz
	564500, // Complete 500k 8 MHz
	565000, // Complete 500k 8 MHz
	565500, // Complete 500k 8 MHz
	566000, // Complete 500k 8 MHz
	566500, // Complete 500k 8 MHz
	567000, // Complete 500k 8 MHz
	567500, // Complete 500k 8 MHz
	568000, // Complete 500k 8 MHz
	568500, // Complete 500k 8 MHz
	569000, // Complete 500k 8 MHz
	569500, // Complete 500k 8 MHz
	570000, // Complete 500k 8 MHz
	570500, // Complete 500k 8 MHz
	571000, // Complete 500k 8 MHz
	571500, // Complete 500k 8 MHz
	572000, // Complete 500k 8 MHz
	572500, // Complete 500k 8 MHz
	573000, // Complete 500k 8 MHz
	573500, // Complete 500k 8 MHz
	574000, // Complete 500k 8 MHz
	574500, // Complete 500k 8 MHz
	575000, // Complete 500k 8 MHz
	575500, // Complete 500k 8 MHz
	576000, // Complete 500k 8 MHz
	576500, // Complete 500k 8 MHz
	577000, // Complete 500k 8 MHz
	577500, // Complete 500k 8 MHz
	578000, // Complete 500k 8 MHz
	578500, // Complete 500k 8 MHz
	579000, // Complete 500k 8 MHz
	579500, // Complete 500k 8 MHz
	580000, // Complete 500k 8 MHz
	580500, // Complete 500k 8 MHz
	581000, // Complete 500k 8 MHz
	581500, // Complete 500k 8 MHz
	582000, // Complete 500k 8 MHz
	582500, // Complete 500k 8 MHz
	583000, // Complete 500k 8 MHz
	583500, // Complete 500k 8 MHz
	584000, // Complete 500k 8 MHz
	584500, // Complete 500k 8 MHz
	585000, // Complete 500k 8 MHz
	585500, // Complete 500k 8 MHz
	586000, // Complete 500k 8 MHz
	586500, // Complete 500k 8 MHz
	587000, // Complete 500k 8 MHz
	587500, // Complete 500k 8 MHz
	588000, // Complete 500k 8 MHz
	588500, // Complete 500k 8 MHz
	589000, // Complete 500k 8 MHz
	589500, // Complete 500k 8 MHz
	590000, // Complete 500k 8 MHz
	590500, // Complete 500k 8 MHz
	591000, // Complete 500k 8 MHz
	591500, // Complete 500k 8 MHz
	592000, // Complete 500k 8 MHz
	592500, // Complete 500k 8 MHz
	593000, // Complete 500k 8 MHz
	593500, // Complete 500k 8 MHz
	594000, // Complete 500k 8 MHz
	594500, // Complete 500k 8 MHz
	595000, // Complete 500k 8 MHz
	595500, // Complete 500k 8 MHz
	596000, // Complete 500k 8 MHz
	596500, // Complete 500k 8 MHz
	597000, // Complete 500k 8 MHz
	597500, // Complete 500k 8 MHz
	598000, // Complete 500k 8 MHz
	598500, // Complete 500k 8 MHz
	599000, // Complete 500k 8 MHz
	599500, // Complete 500k 8 MHz
	600000, // Complete 500k 8 MHz
	600500, // Complete 500k 8 MHz
	601000, // Complete 500k 8 MHz
	601500, // Complete 500k 8 MHz
	602000, // Complete 500k 8 MHz
	602500, // Complete 500k 8 MHz
	603000, // Complete 500k 8 MHz
	603500, // Complete 500k 8 MHz
	604000, // Complete 500k 8 MHz
	604500, // Complete 500k 8 MHz
	605000, // Complete 500k 8 MHz
	605500, // Complete 500k 8 MHz
	606000, // Complete 500k 8 MHz
	606500, // Complete 500k 8 MHz
	607000, // Complete 500k 8 MHz
	607500, // Complete 500k 8 MHz
	608000, // Complete 500k 8 MHz
	608500, // Complete 500k 8 MHz
	609000, // Complete 500k 8 MHz
	609500, // Complete 500k 8 MHz
	610000, // Complete 500k 8 MHz
	610500, // Complete 500k 8 MHz
	611000, // Complete 500k 8 MHz
	611500, // Complete 500k 8 MHz
	612000, // Complete 500k 8 MHz
	612500, // Complete 500k 8 MHz
	613000, // Complete 500k 8 MHz
	613500, // Complete 500k 8 MHz
	614000, // Complete 500k 8 MHz
	614500, // Complete 500k 8 MHz
	615000, // Complete 500k 8 MHz
	615500, // Complete 500k 8 MHz
	616000, // Complete 500k 8 MHz
	616500, // Complete 500k 8 MHz
	617000, // Complete 500k 8 MHz
	617500, // Complete 500k 8 MHz
	618000, // Complete 500k 8 MHz
	618500, // Complete 500k 8 MHz
	619000, // Complete 500k 8 MHz
	619500, // Complete 500k 8 MHz
	620000, // Complete 500k 8 MHz
	620500, // Complete 500k 8 MHz
	621000, // Complete 500k 8 MHz
	621500, // Complete 500k 8 MHz
	622000, // Complete 500k 8 MHz
	622500, // Complete 500k 8 MHz
	623000, // Complete 500k 8 MHz
	623500, // Complete 500k 8 MHz
	624000, // Complete 500k 8 MHz
	624500, // Complete 500k 8 MHz
	625000, // Complete 500k 8 MHz
	625500, // Complete 500k 8 MHz
	626000, // Complete 500k 8 MHz
	626500, // Complete 500k 8 MHz
	627000, // Complete 500k 8 MHz
	627500, // Complete 500k 8 MHz
	628000, // Complete 500k 8 MHz
	628500, // Complete 500k 8 MHz
	629000, // Complete 500k 8 MHz
	629500, // Complete 500k 8 MHz
	630000, // Complete 500k 8 MHz
	630500, // Complete 500k 8 MHz
	631000, // Complete 500k 8 MHz
	631500, // Complete 500k 8 MHz
	632000, // Complete 500k 8 MHz
	632500, // Complete 500k 8 MHz
	633000, // Complete 500k 8 MHz
	633500, // Complete 500k 8 MHz
	634000, // Complete 500k 8 MHz
	634500, // Complete 500k 8 MHz
	635000, // Complete 500k 8 MHz
	635500, // Complete 500k 8 MHz
	636000, // Complete 500k 8 MHz
	636500, // Complete 500k 8 MHz
	637000, // Complete 500k 8 MHz
	637500, // Complete 500k 8 MHz
	638000, // Complete 500k 8 MHz
	638500, // Complete 500k 8 MHz
	639000, // Complete 500k 8 MHz
	639500, // Complete 500k 8 MHz
	640000, // Complete 500k 8 MHz
	640500, // Complete 500k 8 MHz
	641000, // Complete 500k 8 MHz
	641500, // Complete 500k 8 MHz
	642000, // Complete 500k 8 MHz
	642500, // Complete 500k 8 MHz
	643000, // Complete 500k 8 MHz
	643500, // Complete 500k 8 MHz
	644000, // Complete 500k 8 MHz
	644500, // Complete 500k 8 MHz
	645000, // Complete 500k 8 MHz
	645500, // Complete 500k 8 MHz
	646000, // Complete 500k 8 MHz
	646500, // Complete 500k 8 MHz
	647000, // Complete 500k 8 MHz
	647500, // Complete 500k 8 MHz
	648000, // Complete 500k 8 MHz
	648500, // Complete 500k 8 MHz
	649000, // Complete 500k 8 MHz
	649500, // Complete 500k 8 MHz
	650000, // Complete 500k 8 MHz
	650500, // Complete 500k 8 MHz
	651000, // Complete 500k 8 MHz
	651500, // Complete 500k 8 MHz
	652000, // Complete 500k 8 MHz
	652500, // Complete 500k 8 MHz
	653000, // Complete 500k 8 MHz
	653500, // Complete 500k 8 MHz
	654000, // Complete 500k 8 MHz
	654500, // Complete 500k 8 MHz
	655000, // Complete 500k 8 MHz
	655500, // Complete 500k 8 MHz
	656000, // Complete 500k 8 MHz
	656500, // Complete 500k 8 MHz
	657000, // Complete 500k 8 MHz
	657500, // Complete 500k 8 MHz
	658000, // Complete 500k 8 MHz
	658500, // Complete 500k 8 MHz
	659000, // Complete 500k 8 MHz
	659500, // Complete 500k 8 MHz
	660000, // Complete 500k 8 MHz
	660500, // Complete 500k 8 MHz
	661000, // Complete 500k 8 MHz
	661500, // Complete 500k 8 MHz
	662000, // Complete 500k 8 MHz
	662500, // Complete 500k 8 MHz
	663000, // Complete 500k 8 MHz
	663500, // Complete 500k 8 MHz
	664000, // Complete 500k 8 MHz
	664500, // Complete 500k 8 MHz
	665000, // Complete 500k 8 MHz
	665500, // Complete 500k 8 MHz
	666000, // Complete 500k 8 MHz
	666500, // Complete 500k 8 MHz
	667000, // Complete 500k 8 MHz
	667500, // Complete 500k 8 MHz
	668000, // Complete 500k 8 MHz
	668500, // Complete 500k 8 MHz
	669000, // Complete 500k 8 MHz
	669500, // Complete 500k 8 MHz
	670000, // Complete 500k 8 MHz
	670500, // Complete 500k 8 MHz
	671000, // Complete 500k 8 MHz
	671500, // Complete 500k 8 MHz
	672000, // Complete 500k 8 MHz
	672500, // Complete 500k 8 MHz
	673000, // Complete 500k 8 MHz
	673500, // Complete 500k 8 MHz
	674000, // Complete 500k 8 MHz
	674500, // Complete 500k 8 MHz
	675000, // Complete 500k 8 MHz
	675500, // Complete 500k 8 MHz
	676000, // Complete 500k 8 MHz
	676500, // Complete 500k 8 MHz
	677000, // Complete 500k 8 MHz
	677500, // Complete 500k 8 MHz
	678000, // Complete 500k 8 MHz
	678500, // Complete 500k 8 MHz
	679000, // Complete 500k 8 MHz
	679500, // Complete 500k 8 MHz
	680000, // Complete 500k 8 MHz
	680500, // Complete 500k 8 MHz
	681000, // Complete 500k 8 MHz
	681500, // Complete 500k 8 MHz
	682000, // Complete 500k 8 MHz
	682500, // Complete 500k 8 MHz
	683000, // Complete 500k 8 MHz
	683500, // Complete 500k 8 MHz
	684000, // Complete 500k 8 MHz
	684500, // Complete 500k 8 MHz
	685000, // Complete 500k 8 MHz
	685500, // Complete 500k 8 MHz
	686000, // Complete 500k 8 MHz
	686500, // Complete 500k 8 MHz
	687000, // Complete 500k 8 MHz
	687500, // Complete 500k 8 MHz
	688000, // Complete 500k 8 MHz
	688500, // Complete 500k 8 MHz
	689000, // Complete 500k 8 MHz
	689500, // Complete 500k 8 MHz
	690000, // Complete 500k 8 MHz
	690500, // Complete 500k 8 MHz
	691000, // Complete 500k 8 MHz
	691500, // Complete 500k 8 MHz
	692000, // Complete 500k 8 MHz
	692500, // Complete 500k 8 MHz
	693000, // Complete 500k 8 MHz
	693500, // Complete 500k 8 MHz
	694000, // Complete 500k 8 MHz
	694500, // Complete 500k 8 MHz
	695000, // Complete 500k 8 MHz
	695500, // Complete 500k 8 MHz
	696000, // Complete 500k 8 MHz
	696500, // Complete 500k 8 MHz
	697000, // Complete 500k 8 MHz
	697500, // Complete 500k 8 MHz
	698000, // Complete 500k 8 MHz
	698500, // Complete 500k 8 MHz
	699000, // Complete 500k 8 MHz
	699500, // Complete 500k 8 MHz
	700000, // Complete 500k 8 MHz
	700500, // Complete 500k 8 MHz
	701000, // Complete 500k 8 MHz
	701500, // Complete 500k 8 MHz
	702000, // Complete 500k 8 MHz
	702500, // Complete 500k 8 MHz
	703000, // Complete 500k 8 MHz
	703500, // Complete 500k 8 MHz
	704000, // Complete 500k 8 MHz
	704500, // Complete 500k 8 MHz
	705000, // Complete 500k 8 MHz
	705500, // Complete 500k 8 MHz
	706000, // Complete 500k 8 MHz
	706500, // Complete 500k 8 MHz
	707000, // Complete 500k 8 MHz
	707500, // Complete 500k 8 MHz
	708000, // Complete 500k 8 MHz
	708500, // Complete 500k 8 MHz
	709000, // Complete 500k 8 MHz
	709500, // Complete 500k 8 MHz
	710000, // Complete 500k 8 MHz
	710500, // Complete 500k 8 MHz
	711000, // Complete 500k 8 MHz
	711500, // Complete 500k 8 MHz
	712000, // Complete 500k 8 MHz
	712500, // Complete 500k 8 MHz
	713000, // Complete 500k 8 MHz
	713500, // Complete 500k 8 MHz
	714000, // Complete 500k 8 MHz
	714500, // Complete 500k 8 MHz
	715000, // Complete 500k 8 MHz
	715500, // Complete 500k 8 MHz
	716000, // Complete 500k 8 MHz
	716500, // Complete 500k 8 MHz
	717000, // Complete 500k 8 MHz
	717500, // Complete 500k 8 MHz
	718000, // Complete 500k 8 MHz
	718500, // Complete 500k 8 MHz
	719000, // Complete 500k 8 MHz
	719500, // Complete 500k 8 MHz
	720000, // Complete 500k 8 MHz
	720500, // Complete 500k 8 MHz
	721000, // Complete 500k 8 MHz
	721500, // Complete 500k 8 MHz
	722000, // Complete 500k 8 MHz
	722500, // Complete 500k 8 MHz
	723000, // Complete 500k 8 MHz
	723500, // Complete 500k 8 MHz
	724000, // Complete 500k 8 MHz
	724500, // Complete 500k 8 MHz
	725000, // Complete 500k 8 MHz
	725500, // Complete 500k 8 MHz
	726000, // Complete 500k 8 MHz
	726500, // Complete 500k 8 MHz
	727000, // Complete 500k 8 MHz
	727500, // Complete 500k 8 MHz
	728000, // Complete 500k 8 MHz
	728500, // Complete 500k 8 MHz
	729000, // Complete 500k 8 MHz
	729500, // Complete 500k 8 MHz
	730000, // Complete 500k 8 MHz
	730500, // Complete 500k 8 MHz
	731000, // Complete 500k 8 MHz
	731500, // Complete 500k 8 MHz
	732000, // Complete 500k 8 MHz
	732500, // Complete 500k 8 MHz
	733000, // Complete 500k 8 MHz
	733500, // Complete 500k 8 MHz
	734000, // Complete 500k 8 MHz
	734500, // Complete 500k 8 MHz
	735000, // Complete 500k 8 MHz
	735500, // Complete 500k 8 MHz
	736000, // Complete 500k 8 MHz
	736500, // Complete 500k 8 MHz
	737000, // Complete 500k 8 MHz
	737500, // Complete 500k 8 MHz
	738000, // Complete 500k 8 MHz
	738500, // Complete 500k 8 MHz
	739000, // Complete 500k 8 MHz
	739500, // Complete 500k 8 MHz
	740000, // Complete 500k 8 MHz
	740500, // Complete 500k 8 MHz
	741000, // Complete 500k 8 MHz
	741500, // Complete 500k 8 MHz
	742000, // Complete 500k 8 MHz
	742500, // Complete 500k 8 MHz
	743000, // Complete 500k 8 MHz
	743500, // Complete 500k 8 MHz
	744000, // Complete 500k 8 MHz
	744500, // Complete 500k 8 MHz
	745000, // Complete 500k 8 MHz
	745500, // Complete 500k 8 MHz
	746000, // Complete 500k 8 MHz
	746500, // Complete 500k 8 MHz
	747000, // Complete 500k 8 MHz
	747500, // Complete 500k 8 MHz
	748000, // Complete 500k 8 MHz
	748500, // Complete 500k 8 MHz
	749000, // Complete 500k 8 MHz
	749500, // Complete 500k 8 MHz
	750000, // Complete 500k 8 MHz
	750500, // Complete 500k 8 MHz
	751000, // Complete 500k 8 MHz
	751500, // Complete 500k 8 MHz
	752000, // Complete 500k 8 MHz
	752500, // Complete 500k 8 MHz
	753000, // Complete 500k 8 MHz
	753500, // Complete 500k 8 MHz
	754000, // Complete 500k 8 MHz
	754500, // Complete 500k 8 MHz
	755000, // Complete 500k 8 MHz
	755500, // Complete 500k 8 MHz
	756000, // Complete 500k 8 MHz
	756500, // Complete 500k 8 MHz
	757000, // Complete 500k 8 MHz
	757500, // Complete 500k 8 MHz
	758000, // Complete 500k 8 MHz
	758500, // Complete 500k 8 MHz
	759000, // Complete 500k 8 MHz
	759500, // Complete 500k 8 MHz
	760000, // Complete 500k 8 MHz
	760500, // Complete 500k 8 MHz
	761000, // Complete 500k 8 MHz
	761500, // Complete 500k 8 MHz
	762000, // Complete 500k 8 MHz
	762500, // Complete 500k 8 MHz
	763000, // Complete 500k 8 MHz
	763500, // Complete 500k 8 MHz
	764000, // Complete 500k 8 MHz
	764500, // Complete 500k 8 MHz
	765000, // Complete 500k 8 MHz
	765500, // Complete 500k 8 MHz
	766000, // Complete 500k 8 MHz
	766500, // Complete 500k 8 MHz
	767000, // Complete 500k 8 MHz
	767500, // Complete 500k 8 MHz
	768000, // Complete 500k 8 MHz
	768500, // Complete 500k 8 MHz
	769000, // Complete 500k 8 MHz
	769500, // Complete 500k 8 MHz
	770000, // Complete 500k 8 MHz
	770500, // Complete 500k 8 MHz
	771000, // Complete 500k 8 MHz
	771500, // Complete 500k 8 MHz
	772000, // Complete 500k 8 MHz
	772500, // Complete 500k 8 MHz
	773000, // Complete 500k 8 MHz
	773500, // Complete 500k 8 MHz
	774000, // Complete 500k 8 MHz
	774500, // Complete 500k 8 MHz
	775000, // Complete 500k 8 MHz
	775500, // Complete 500k 8 MHz
	776000, // Complete 500k 8 MHz
	776500, // Complete 500k 8 MHz
	777000, // Complete 500k 8 MHz
	777500, // Complete 500k 8 MHz
	778000, // Complete 500k 8 MHz
	778500, // Complete 500k 8 MHz
	779000, // Complete 500k 8 MHz
	779500, // Complete 500k 8 MHz
	780000, // Complete 500k 8 MHz
	780500, // Complete 500k 8 MHz
	781000, // Complete 500k 8 MHz
	781500, // Complete 500k 8 MHz
	782000, // Complete 500k 8 MHz
	782500, // Complete 500k 8 MHz
	783000, // Complete 500k 8 MHz
	783500, // Complete 500k 8 MHz
	784000, // Complete 500k 8 MHz
	784500, // Complete 500k 8 MHz
	785000, // Complete 500k 8 MHz
	785500, // Complete 500k 8 MHz
	786000, // Complete 500k 8 MHz
	786500, // Complete 500k 8 MHz
	787000, // Complete 500k 8 MHz
	787500, // Complete 500k 8 MHz
	788000, // Complete 500k 8 MHz
	788500, // Complete 500k 8 MHz
	789000, // Complete 500k 8 MHz
	789500, // Complete 500k 8 MHz
	790000, // Complete 500k 8 MHz
	790500, // Complete 500k 8 MHz
	791000, // Complete 500k 8 MHz
	791500, // Complete 500k 8 MHz
	792000, // Complete 500k 8 MHz
	792500, // Complete 500k 8 MHz
	793000, // Complete 500k 8 MHz
	793500, // Complete 500k 8 MHz
	794000, // Complete 500k 8 MHz
	794500, // Complete 500k 8 MHz
	795000, // Complete 500k 8 MHz
	795500, // Complete 500k 8 MHz
	796000, // Complete 500k 8 MHz
	796500, // Complete 500k 8 MHz
	797000, // Complete 500k 8 MHz
	797500, // Complete 500k 8 MHz
	798000, // Complete 500k 8 MHz
	798500, // Complete 500k 8 MHz
	799000, // Complete 500k 8 MHz
	799500, // Complete 500k 8 MHz
	800000, // Complete 500k 8 MHz
	800500, // Complete 500k 8 MHz
	801000, // Complete 500k 8 MHz
	801500, // Complete 500k 8 MHz
	802000, // Complete 500k 8 MHz
	802500, // Complete 500k 8 MHz
	803000, // Complete 500k 8 MHz
	803500, // Complete 500k 8 MHz
	804000, // Complete 500k 8 MHz
	804500, // Complete 500k 8 MHz
	805000, // Complete 500k 8 MHz
	805500, // Complete 500k 8 MHz
	806000, // Complete 500k 8 MHz
	806500, // Complete 500k 8 MHz
	807000, // Complete 500k 8 MHz
	807500, // Complete 500k 8 MHz
	808000, // Complete 500k 8 MHz
	808500, // Complete 500k 8 MHz
	809000, // Complete 500k 8 MHz
	809500, // Complete 500k 8 MHz
	810000, // Complete 500k 8 MHz
	810500, // Complete 500k 8 MHz
	811000, // Complete 500k 8 MHz
	811500, // Complete 500k 8 MHz
	812000, // Complete 500k 8 MHz
	812500, // Complete 500k 8 MHz
	813000, // Complete 500k 8 MHz
	813500, // Complete 500k 8 MHz
	814000, // Complete 500k 8 MHz
	814500, // Complete 500k 8 MHz
	815000, // Complete 500k 8 MHz
	815500, // Complete 500k 8 MHz
	816000, // Complete 500k 8 MHz
	816500, // Complete 500k 8 MHz
	817000, // Complete 500k 8 MHz
	817500, // Complete 500k 8 MHz
	818000, // Complete 500k 8 MHz
	818500, // Complete 500k 8 MHz
	819000, // Complete 500k 8 MHz
	819500, // Complete 500k 8 MHz
	820000, // Complete 500k 8 MHz
	820500, // Complete 500k 8 MHz
	821000, // Complete 500k 8 MHz
	821500, // Complete 500k 8 MHz
	822000, // Complete 500k 8 MHz
	822500, // Complete 500k 8 MHz
	823000, // Complete 500k 8 MHz
	823500, // Complete 500k 8 MHz
	824000, // Complete 500k 8 MHz
	824500, // Complete 500k 8 MHz
	825000, // Complete 500k 8 MHz
	825500, // Complete 500k 8 MHz
	826000, // Complete 500k 8 MHz
	826500, // Complete 500k 8 MHz
	827000, // Complete 500k 8 MHz
	827500, // Complete 500k 8 MHz
	828000, // Complete 500k 8 MHz
	828500, // Complete 500k 8 MHz
	829000, // Complete 500k 8 MHz
	829500, // Complete 500k 8 MHz
	830000, // Complete 500k 8 MHz
	830500, // Complete 500k 8 MHz
	831000, // Complete 500k 8 MHz
	831500, // Complete 500k 8 MHz
	832000, // Complete 500k 8 MHz
	832500, // Complete 500k 8 MHz
	833000, // Complete 500k 8 MHz
	833500, // Complete 500k 8 MHz
	834000, // Complete 500k 8 MHz
	834500, // Complete 500k 8 MHz
	835000, // Complete 500k 8 MHz
	835500, // Complete 500k 8 MHz
	836000, // Complete 500k 8 MHz
	836500, // Complete 500k 8 MHz
	837000, // Complete 500k 8 MHz
	837500, // Complete 500k 8 MHz
	838000, // Complete 500k 8 MHz
	838500, // Complete 500k 8 MHz
	839000, // Complete 500k 8 MHz
	839500, // Complete 500k 8 MHz
	840000, // Complete 500k 8 MHz
	840500, // Complete 500k 8 MHz
	841000, // Complete 500k 8 MHz
	841500, // Complete 500k 8 MHz
	842000, // Complete 500k 8 MHz
	842500, // Complete 500k 8 MHz
	843000, // Complete 500k 8 MHz
	843500, // Complete 500k 8 MHz
	844000, // Complete 500k 8 MHz
	844500, // Complete 500k 8 MHz
	845000, // Complete 500k 8 MHz
	845500, // Complete 500k 8 MHz
	846000, // Complete 500k 8 MHz
	846500, // Complete 500k 8 MHz
	847000, // Complete 500k 8 MHz
	847500, // Complete 500k 8 MHz
	848000, // Complete 500k 8 MHz
	848500, // Complete 500k 8 MHz
	849000, // Complete 500k 8 MHz
	849500, // Complete 500k 8 MHz
	850000, // Complete 500k 8 MHz
	850500, // Complete 500k 8 MHz
	851000, // Complete 500k 8 MHz
	851500, // Complete 500k 8 MHz
	852000, // Complete 500k 8 MHz
	852500, // Complete 500k 8 MHz
	853000, // Complete 500k 8 MHz
	853500, // Complete 500k 8 MHz
	854000, // Complete 500k 8 MHz
	854500, // Complete 500k 8 MHz
	855000, // Complete 500k 8 MHz
	855500, // Complete 500k 8 MHz
	856000, // Complete 500k 8 MHz
	856500, // Complete 500k 8 MHz
	857000, // Complete 500k 8 MHz
	857500, // Complete 500k 8 MHz
	858000, // Complete 500k 8 MHz
	858500, // Complete 500k 8 MHz
	0,		// ******************************* end of Complete 500k 8 MHz
	111000, // Complete 500k 7/8 MHz
	111500, // Complete 500k 7/8 MHz
	112000, // Complete 500k 7/8 MHz
	112500, // Complete 500k 7/8 MHz
	113000, // Complete 500k 7/8 MHz
	113500, // Complete 500k 7/8 MHz
	114000, // Complete 500k 7/8 MHz
	114500, // Complete 500k 7/8 MHz
	115000, // Complete 500k 7/8 MHz
	115500, // Complete 500k 7/8 MHz
	116000, // Complete 500k 7/8 MHz
	116500, // Complete 500k 7/8 MHz
	117000, // Complete 500k 7/8 MHz
	117500, // Complete 500k 7/8 MHz
	118000, // Complete 500k 7/8 MHz
	118500, // Complete 500k 7/8 MHz
	119000, // Complete 500k 7/8 MHz
	119500, // Complete 500k 7/8 MHz
	120000, // Complete 500k 7/8 MHz
	120500, // Complete 500k 7/8 MHz
	121000, // Complete 500k 7/8 MHz
	121500, // Complete 500k 7/8 MHz
	122000, // Complete 500k 7/8 MHz
	122500, // Complete 500k 7/8 MHz
	123000, // Complete 500k 7/8 MHz
	123500, // Complete 500k 7/8 MHz
	124000, // Complete 500k 7/8 MHz
	124500, // Complete 500k 7/8 MHz
	125000, // Complete 500k 7/8 MHz
	125500, // Complete 500k 7/8 MHz
	126000, // Complete 500k 7/8 MHz
	126500, // Complete 500k 7/8 MHz
	127000, // Complete 500k 7/8 MHz
	127500, // Complete 500k 7/8 MHz
	128000, // Complete 500k 7/8 MHz
	128500, // Complete 500k 7/8 MHz
	129000, // Complete 500k 7/8 MHz
	129500, // Complete 500k 7/8 MHz
	130000, // Complete 500k 7/8 MHz
	130500, // Complete 500k 7/8 MHz
	131000, // Complete 500k 7/8 MHz
	131500, // Complete 500k 7/8 MHz
	132000, // Complete 500k 7/8 MHz
	132500, // Complete 500k 7/8 MHz
	133000, // Complete 500k 7/8 MHz
	133500, // Complete 500k 7/8 MHz
	134000, // Complete 500k 7/8 MHz
	134500, // Complete 500k 7/8 MHz
	135000, // Complete 500k 7/8 MHz
	135500, // Complete 500k 7/8 MHz
	136000, // Complete 500k 7/8 MHz
	136500, // Complete 500k 7/8 MHz
	137000, // Complete 500k 7/8 MHz
	137500, // Complete 500k 7/8 MHz
	138000, // Complete 500k 7/8 MHz
	138500, // Complete 500k 7/8 MHz
	139000, // Complete 500k 7/8 MHz
	139500, // Complete 500k 7/8 MHz
	140000, // Complete 500k 7/8 MHz
	140500, // Complete 500k 7/8 MHz
	141000, // Complete 500k 7/8 MHz
	141500, // Complete 500k 7/8 MHz
	142000, // Complete 500k 7/8 MHz
	142500, // Complete 500k 7/8 MHz
	143000, // Complete 500k 7/8 MHz
	143500, // Complete 500k 7/8 MHz
	144000, // Complete 500k 7/8 MHz
	144500, // Complete 500k 7/8 MHz
	145000, // Complete 500k 7/8 MHz
	145500, // Complete 500k 7/8 MHz
	146000, // Complete 500k 7/8 MHz
	146500, // Complete 500k 7/8 MHz
	147000, // Complete 500k 7/8 MHz
	147500, // Complete 500k 7/8 MHz
	148000, // Complete 500k 7/8 MHz
	148500, // Complete 500k 7/8 MHz
	149000, // Complete 500k 7/8 MHz
	149500, // Complete 500k 7/8 MHz
	150000, // Complete 500k 7/8 MHz
	150500, // Complete 500k 7/8 MHz
	151000, // Complete 500k 7/8 MHz
	151500, // Complete 500k 7/8 MHz
	152000, // Complete 500k 7/8 MHz
	152500, // Complete 500k 7/8 MHz
	153000, // Complete 500k 7/8 MHz
	153500, // Complete 500k 7/8 MHz
	154000, // Complete 500k 7/8 MHz
	154500, // Complete 500k 7/8 MHz
	155000, // Complete 500k 7/8 MHz
	155500, // Complete 500k 7/8 MHz
	156000, // Complete 500k 7/8 MHz
	156500, // Complete 500k 7/8 MHz
	157000, // Complete 500k 7/8 MHz
	157500, // Complete 500k 7/8 MHz
	158000, // Complete 500k 7/8 MHz
	158500, // Complete 500k 7/8 MHz
	159000, // Complete 500k 7/8 MHz
	159500, // Complete 500k 7/8 MHz
	160000, // Complete 500k 7/8 MHz
	160500, // Complete 500k 7/8 MHz
	161000, // Complete 500k 7/8 MHz
	161500, // Complete 500k 7/8 MHz
	162000, // Complete 500k 7/8 MHz
	162500, // Complete 500k 7/8 MHz
	163000, // Complete 500k 7/8 MHz
	163500, // Complete 500k 7/8 MHz
	164000, // Complete 500k 7/8 MHz
	164500, // Complete 500k 7/8 MHz
	165000, // Complete 500k 7/8 MHz
	165500, // Complete 500k 7/8 MHz
	166000, // Complete 500k 7/8 MHz
	166500, // Complete 500k 7/8 MHz
	167000, // Complete 500k 7/8 MHz
	167500, // Complete 500k 7/8 MHz
	168000, // Complete 500k 7/8 MHz
	168500, // Complete 500k 7/8 MHz
	169000, // Complete 500k 7/8 MHz
	169500, // Complete 500k 7/8 MHz
	170000, // Complete 500k 7/8 MHz
	170500, // Complete 500k 7/8 MHz
	171000, // Complete 500k 7/8 MHz
	171500, // Complete 500k 7/8 MHz
	172000, // Complete 500k 7/8 MHz
	172500, // Complete 500k 7/8 MHz
	173000, // Complete 500k 7/8 MHz
	173500, // Complete 500k 7/8 MHz
	174000, // Complete 500k 7/8 MHz
	174500, // Complete 500k 7/8 MHz
	175000, // Complete 500k 7/8 MHz
	175500, // Complete 500k 7/8 MHz
	176000, // Complete 500k 7/8 MHz
	176500, // Complete 500k 7/8 MHz
	177000, // Complete 500k 7/8 MHz
	177500, // Complete 500k 7/8 MHz
	178000, // Complete 500k 7/8 MHz
	178500, // Complete 500k 7/8 MHz
	179000, // Complete 500k 7/8 MHz
	179500, // Complete 500k 7/8 MHz
	180000, // Complete 500k 7/8 MHz
	180500, // Complete 500k 7/8 MHz
	181000, // Complete 500k 7/8 MHz
	181500, // Complete 500k 7/8 MHz
	182000, // Complete 500k 7/8 MHz
	182500, // Complete 500k 7/8 MHz
	183000, // Complete 500k 7/8 MHz
	183500, // Complete 500k 7/8 MHz
	184000, // Complete 500k 7/8 MHz
	184500, // Complete 500k 7/8 MHz
	185000, // Complete 500k 7/8 MHz
	185500, // Complete 500k 7/8 MHz
	186000, // Complete 500k 7/8 MHz
	186500, // Complete 500k 7/8 MHz
	187000, // Complete 500k 7/8 MHz
	187500, // Complete 500k 7/8 MHz
	188000, // Complete 500k 7/8 MHz
	188500, // Complete 500k 7/8 MHz
	189000, // Complete 500k 7/8 MHz
	189500, // Complete 500k 7/8 MHz
	190000, // Complete 500k 7/8 MHz
	190500, // Complete 500k 7/8 MHz
	191000, // Complete 500k 7/8 MHz
	191500, // Complete 500k 7/8 MHz
	192000, // Complete 500k 7/8 MHz
	192500, // Complete 500k 7/8 MHz
	193000, // Complete 500k 7/8 MHz
	193500, // Complete 500k 7/8 MHz
	194000, // Complete 500k 7/8 MHz
	194500, // Complete 500k 7/8 MHz
	195000, // Complete 500k 7/8 MHz
	195500, // Complete 500k 7/8 MHz
	196000, // Complete 500k 7/8 MHz
	196500, // Complete 500k 7/8 MHz
	197000, // Complete 500k 7/8 MHz
	197500, // Complete 500k 7/8 MHz
	198000, // Complete 500k 7/8 MHz
	198500, // Complete 500k 7/8 MHz
	199000, // Complete 500k 7/8 MHz
	199500, // Complete 500k 7/8 MHz
	200000, // Complete 500k 7/8 MHz
	200500, // Complete 500k 7/8 MHz
	201000, // Complete 500k 7/8 MHz
	201500, // Complete 500k 7/8 MHz
	202000, // Complete 500k 7/8 MHz
	202500, // Complete 500k 7/8 MHz
	203000, // Complete 500k 7/8 MHz
	203500, // Complete 500k 7/8 MHz
	204000, // Complete 500k 7/8 MHz
	204500, // Complete 500k 7/8 MHz
	205000, // Complete 500k 7/8 MHz
	205500, // Complete 500k 7/8 MHz
	206000, // Complete 500k 7/8 MHz
	206500, // Complete 500k 7/8 MHz
	207000, // Complete 500k 7/8 MHz
	207500, // Complete 500k 7/8 MHz
	208000, // Complete 500k 7/8 MHz
	208500, // Complete 500k 7/8 MHz
	209000, // Complete 500k 7/8 MHz
	209500, // Complete 500k 7/8 MHz
	210000, // Complete 500k 7/8 MHz
	210500, // Complete 500k 7/8 MHz
	211000, // Complete 500k 7/8 MHz
	211500, // Complete 500k 7/8 MHz
	212000, // Complete 500k 7/8 MHz
	212500, // Complete 500k 7/8 MHz
	213000, // Complete 500k 7/8 MHz
	213500, // Complete 500k 7/8 MHz
	214000, // Complete 500k 7/8 MHz
	214500, // Complete 500k 7/8 MHz
	215000, // Complete 500k 7/8 MHz
	215500, // Complete 500k 7/8 MHz
	216000, // Complete 500k 7/8 MHz
	216500, // Complete 500k 7/8 MHz
	217000, // Complete 500k 7/8 MHz
	217500, // Complete 500k 7/8 MHz
	218000, // Complete 500k 7/8 MHz
	218500, // Complete 500k 7/8 MHz
	219000, // Complete 500k 7/8 MHz
	219500, // Complete 500k 7/8 MHz
	220000, // Complete 500k 7/8 MHz
	220500, // Complete 500k 7/8 MHz
	221000, // Complete 500k 7/8 MHz
	221500, // Complete 500k 7/8 MHz
	222000, // Complete 500k 7/8 MHz
	222500, // Complete 500k 7/8 MHz
	223000, // Complete 500k 7/8 MHz
	223500, // Complete 500k 7/8 MHz
	224000, // Complete 500k 7/8 MHz
	224500, // Complete 500k 7/8 MHz
	225000, // Complete 500k 7/8 MHz
	225500, // Complete 500k 7/8 MHz
	226000, // Complete 500k 7/8 MHz
	226500, // Complete 500k 7/8 MHz
	227000, // Complete 500k 7/8 MHz
	227500, // Complete 500k 7/8 MHz
	228000, // Complete 500k 7/8 MHz
	228500, // Complete 500k 7/8 MHz
	229000, // Complete 500k 7/8 MHz
	229500, // Complete 500k 7/8 MHz
	230000, // Complete 500k 7/8 MHz
	230500, // Complete 500k 7/8 MHz
	231000, // Complete 500k 7/8 MHz
	231500, // Complete 500k 7/8 MHz
	232000, // Complete 500k 7/8 MHz
	232500, // Complete 500k 7/8 MHz
	233000, // Complete 500k 7/8 MHz
	233500, // Complete 500k 7/8 MHz
	234000, // Complete 500k 7/8 MHz
	234500, // Complete 500k 7/8 MHz
	235000, // Complete 500k 7/8 MHz
	235500, // Complete 500k 7/8 MHz
	236000, // Complete 500k 7/8 MHz
	236500, // Complete 500k 7/8 MHz
	237000, // Complete 500k 7/8 MHz
	237500, // Complete 500k 7/8 MHz
	238000, // Complete 500k 7/8 MHz
	238500, // Complete 500k 7/8 MHz
	239000, // Complete 500k 7/8 MHz
	239500, // Complete 500k 7/8 MHz
	240000, // Complete 500k 7/8 MHz
	240500, // Complete 500k 7/8 MHz
	241000, // Complete 500k 7/8 MHz
	241500, // Complete 500k 7/8 MHz
	242000, // Complete 500k 7/8 MHz
	242500, // Complete 500k 7/8 MHz
	243000, // Complete 500k 7/8 MHz
	243500, // Complete 500k 7/8 MHz
	244000, // Complete 500k 7/8 MHz
	244500, // Complete 500k 7/8 MHz
	245000, // Complete 500k 7/8 MHz
	245500, // Complete 500k 7/8 MHz
	246000, // Complete 500k 7/8 MHz
	246500, // Complete 500k 7/8 MHz
	247000, // Complete 500k 7/8 MHz
	247500, // Complete 500k 7/8 MHz
	248000, // Complete 500k 7/8 MHz
	248500, // Complete 500k 7/8 MHz
	249000, // Complete 500k 7/8 MHz
	249500, // Complete 500k 7/8 MHz
	250000, // Complete 500k 7/8 MHz
	250500, // Complete 500k 7/8 MHz
	251000, // Complete 500k 7/8 MHz
	251500, // Complete 500k 7/8 MHz
	252000, // Complete 500k 7/8 MHz
	252500, // Complete 500k 7/8 MHz
	253000, // Complete 500k 7/8 MHz
	253500, // Complete 500k 7/8 MHz
	254000, // Complete 500k 7/8 MHz
	254500, // Complete 500k 7/8 MHz
	255000, // Complete 500k 7/8 MHz
	255500, // Complete 500k 7/8 MHz
	256000, // Complete 500k 7/8 MHz
	256500, // Complete 500k 7/8 MHz
	257000, // Complete 500k 7/8 MHz
	257500, // Complete 500k 7/8 MHz
	258000, // Complete 500k 7/8 MHz
	258500, // Complete 500k 7/8 MHz
	259000, // Complete 500k 7/8 MHz
	259500, // Complete 500k 7/8 MHz
	260000, // Complete 500k 7/8 MHz
	260500, // Complete 500k 7/8 MHz
	261000, // Complete 500k 7/8 MHz
	261500, // Complete 500k 7/8 MHz
	262000, // Complete 500k 7/8 MHz
	262500, // Complete 500k 7/8 MHz
	263000, // Complete 500k 7/8 MHz
	263500, // Complete 500k 7/8 MHz
	264000, // Complete 500k 7/8 MHz
	264500, // Complete 500k 7/8 MHz
	265000, // Complete 500k 7/8 MHz
	265500, // Complete 500k 7/8 MHz
	266000, // Complete 500k 7/8 MHz
	266500, // Complete 500k 7/8 MHz
	267000, // Complete 500k 7/8 MHz
	267500, // Complete 500k 7/8 MHz
	268000, // Complete 500k 7/8 MHz
	268500, // Complete 500k 7/8 MHz
	269000, // Complete 500k 7/8 MHz
	269500, // Complete 500k 7/8 MHz
	270000, // Complete 500k 7/8 MHz
	270500, // Complete 500k 7/8 MHz
	271000, // Complete 500k 7/8 MHz
	271500, // Complete 500k 7/8 MHz
	272000, // Complete 500k 7/8 MHz
	272500, // Complete 500k 7/8 MHz
	273000, // Complete 500k 7/8 MHz
	273500, // Complete 500k 7/8 MHz
	274000, // Complete 500k 7/8 MHz
	274500, // Complete 500k 7/8 MHz
	275000, // Complete 500k 7/8 MHz
	275500, // Complete 500k 7/8 MHz
	276000, // Complete 500k 7/8 MHz
	276500, // Complete 500k 7/8 MHz
	277000, // Complete 500k 7/8 MHz
	277500, // Complete 500k 7/8 MHz
	278000, // Complete 500k 7/8 MHz
	278500, // Complete 500k 7/8 MHz
	279000, // Complete 500k 7/8 MHz
	279500, // Complete 500k 7/8 MHz
	280000, // Complete 500k 7/8 MHz
	280500, // Complete 500k 7/8 MHz
	281000, // Complete 500k 7/8 MHz
	281500, // Complete 500k 7/8 MHz
	282000, // Complete 500k 7/8 MHz
	282500, // Complete 500k 7/8 MHz
	283000, // Complete 500k 7/8 MHz
	283500, // Complete 500k 7/8 MHz
	284000, // Complete 500k 7/8 MHz
	284500, // Complete 500k 7/8 MHz
	285000, // Complete 500k 7/8 MHz
	285500, // Complete 500k 7/8 MHz
	286000, // Complete 500k 7/8 MHz
	286500, // Complete 500k 7/8 MHz
	287000, // Complete 500k 7/8 MHz
	287500, // Complete 500k 7/8 MHz
	288000, // Complete 500k 7/8 MHz
	288500, // Complete 500k 7/8 MHz
	289000, // Complete 500k 7/8 MHz
	289500, // Complete 500k 7/8 MHz
	290000, // Complete 500k 7/8 MHz
	290500, // Complete 500k 7/8 MHz
	291000, // Complete 500k 7/8 MHz
	291500, // Complete 500k 7/8 MHz
	292000, // Complete 500k 7/8 MHz
	292500, // Complete 500k 7/8 MHz
	293000, // Complete 500k 7/8 MHz
	293500, // Complete 500k 7/8 MHz
	294000, // Complete 500k 7/8 MHz
	294500, // Complete 500k 7/8 MHz
	295000, // Complete 500k 7/8 MHz
	295500, // Complete 500k 7/8 MHz
	296000, // Complete 500k 7/8 MHz
	296500, // Complete 500k 7/8 MHz
	297000, // Complete 500k 7/8 MHz
	297500, // Complete 500k 7/8 MHz
	298000, // Complete 500k 7/8 MHz
	298500, // Complete 500k 7/8 MHz
	299000, // Complete 500k 7/8 MHz
	299500, // Complete 500k 7/8 MHz
	300000, // Complete 500k 7/8 MHz
	300500, // Complete 500k 7/8 MHz
	301000, // Complete 500k 7/8 MHz
	301500, // Complete 500k 7/8 MHz
	302000, // Complete 500k 7/8 MHz
	302500, // Complete 500k 7/8 MHz
	303000, // Complete 500k 7/8 MHz
	303500, // Complete 500k 7/8 MHz
	304000, // Complete 500k 7/8 MHz
	304500, // Complete 500k 7/8 MHz
	305000, // Complete 500k 7/8 MHz
	305500, // Complete 500k 7/8 MHz
	306000, // Complete 500k 7/8 MHz
	306500, // Complete 500k 7/8 MHz
	307000, // Complete 500k 7/8 MHz
	307500, // Complete 500k 7/8 MHz
	308000, // Complete 500k 7/8 MHz
	308500, // Complete 500k 7/8 MHz
	309000, // Complete 500k 7/8 MHz
	309500, // Complete 500k 7/8 MHz
	310000, // Complete 500k 7/8 MHz
	310500, // Complete 500k 7/8 MHz
	311000, // Complete 500k 7/8 MHz
	311500, // Complete 500k 7/8 MHz
	312000, // Complete 500k 7/8 MHz
	312500, // Complete 500k 7/8 MHz
	313000, // Complete 500k 7/8 MHz
	313500, // Complete 500k 7/8 MHz
	314000, // Complete 500k 7/8 MHz
	314500, // Complete 500k 7/8 MHz
	315000, // Complete 500k 7/8 MHz
	315500, // Complete 500k 7/8 MHz
	316000, // Complete 500k 7/8 MHz
	316500, // Complete 500k 7/8 MHz
	317000, // Complete 500k 7/8 MHz
	317500, // Complete 500k 7/8 MHz
	318000, // Complete 500k 7/8 MHz
	318500, // Complete 500k 7/8 MHz
	319000, // Complete 500k 7/8 MHz
	319500, // Complete 500k 7/8 MHz
	320000, // Complete 500k 7/8 MHz
	320500, // Complete 500k 7/8 MHz
	321000, // Complete 500k 7/8 MHz
	321500, // Complete 500k 7/8 MHz
	322000, // Complete 500k 7/8 MHz
	322500, // Complete 500k 7/8 MHz
	323000, // Complete 500k 7/8 MHz
	323500, // Complete 500k 7/8 MHz
	324000, // Complete 500k 7/8 MHz
	324500, // Complete 500k 7/8 MHz
	325000, // Complete 500k 7/8 MHz
	325500, // Complete 500k 7/8 MHz
	326000, // Complete 500k 7/8 MHz
	326500, // Complete 500k 7/8 MHz
	327000, // Complete 500k 7/8 MHz
	327500, // Complete 500k 7/8 MHz
	328000, // Complete 500k 7/8 MHz
	328500, // Complete 500k 7/8 MHz
	329000, // Complete 500k 7/8 MHz
	329500, // Complete 500k 7/8 MHz
	330000, // Complete 500k 7/8 MHz
	330500, // Complete 500k 7/8 MHz
	331000, // Complete 500k 7/8 MHz
	331500, // Complete 500k 7/8 MHz
	332000, // Complete 500k 7/8 MHz
	332500, // Complete 500k 7/8 MHz
	333000, // Complete 500k 7/8 MHz
	333500, // Complete 500k 7/8 MHz
	334000, // Complete 500k 7/8 MHz
	334500, // Complete 500k 7/8 MHz
	335000, // Complete 500k 7/8 MHz
	335500, // Complete 500k 7/8 MHz
	336000, // Complete 500k 7/8 MHz
	336500, // Complete 500k 7/8 MHz
	337000, // Complete 500k 7/8 MHz
	337500, // Complete 500k 7/8 MHz
	338000, // Complete 500k 7/8 MHz
	338500, // Complete 500k 7/8 MHz
	339000, // Complete 500k 7/8 MHz
	339500, // Complete 500k 7/8 MHz
	340000, // Complete 500k 7/8 MHz
	340500, // Complete 500k 7/8 MHz
	341000, // Complete 500k 7/8 MHz
	341500, // Complete 500k 7/8 MHz
	342000, // Complete 500k 7/8 MHz
	342500, // Complete 500k 7/8 MHz
	343000, // Complete 500k 7/8 MHz
	343500, // Complete 500k 7/8 MHz
	344000, // Complete 500k 7/8 MHz
	344500, // Complete 500k 7/8 MHz
	345000, // Complete 500k 7/8 MHz
	345500, // Complete 500k 7/8 MHz
	346000, // Complete 500k 7/8 MHz
	346500, // Complete 500k 7/8 MHz
	347000, // Complete 500k 7/8 MHz
	347500, // Complete 500k 7/8 MHz
	348000, // Complete 500k 7/8 MHz
	348500, // Complete 500k 7/8 MHz
	349000, // Complete 500k 7/8 MHz
	349500, // Complete 500k 7/8 MHz
	350000, // Complete 500k 7/8 MHz
	350500, // Complete 500k 7/8 MHz
	351000, // Complete 500k 7/8 MHz
	351500, // Complete 500k 7/8 MHz
	352000, // Complete 500k 7/8 MHz
	352500, // Complete 500k 7/8 MHz
	353000, // Complete 500k 7/8 MHz
	353500, // Complete 500k 7/8 MHz
	354000, // Complete 500k 7/8 MHz
	354500, // Complete 500k 7/8 MHz
	355000, // Complete 500k 7/8 MHz
	355500, // Complete 500k 7/8 MHz
	356000, // Complete 500k 7/8 MHz
	356500, // Complete 500k 7/8 MHz
	357000, // Complete 500k 7/8 MHz
	357500, // Complete 500k 7/8 MHz
	358000, // Complete 500k 7/8 MHz
	358500, // Complete 500k 7/8 MHz
	359000, // Complete 500k 7/8 MHz
	359500, // Complete 500k 7/8 MHz
	360000, // Complete 500k 7/8 MHz
	360500, // Complete 500k 7/8 MHz
	361000, // Complete 500k 7/8 MHz
	361500, // Complete 500k 7/8 MHz
	362000, // Complete 500k 7/8 MHz
	362500, // Complete 500k 7/8 MHz
	363000, // Complete 500k 7/8 MHz
	363500, // Complete 500k 7/8 MHz
	364000, // Complete 500k 7/8 MHz
	364500, // Complete 500k 7/8 MHz
	365000, // Complete 500k 7/8 MHz
	365500, // Complete 500k 7/8 MHz
	366000, // Complete 500k 7/8 MHz
	366500, // Complete 500k 7/8 MHz
	367000, // Complete 500k 7/8 MHz
	367500, // Complete 500k 7/8 MHz
	368000, // Complete 500k 7/8 MHz
	368500, // Complete 500k 7/8 MHz
	369000, // Complete 500k 7/8 MHz
	369500, // Complete 500k 7/8 MHz
	370000, // Complete 500k 7/8 MHz
	370500, // Complete 500k 7/8 MHz
	371000, // Complete 500k 7/8 MHz
	371500, // Complete 500k 7/8 MHz
	372000, // Complete 500k 7/8 MHz
	372500, // Complete 500k 7/8 MHz
	373000, // Complete 500k 7/8 MHz
	373500, // Complete 500k 7/8 MHz
	374000, // Complete 500k 7/8 MHz
	374500, // Complete 500k 7/8 MHz
	375000, // Complete 500k 7/8 MHz
	375500, // Complete 500k 7/8 MHz
	376000, // Complete 500k 7/8 MHz
	376500, // Complete 500k 7/8 MHz
	377000, // Complete 500k 7/8 MHz
	377500, // Complete 500k 7/8 MHz
	378000, // Complete 500k 7/8 MHz
	378500, // Complete 500k 7/8 MHz
	379000, // Complete 500k 7/8 MHz
	379500, // Complete 500k 7/8 MHz
	380000, // Complete 500k 7/8 MHz
	380500, // Complete 500k 7/8 MHz
	381000, // Complete 500k 7/8 MHz
	381500, // Complete 500k 7/8 MHz
	382000, // Complete 500k 7/8 MHz
	382500, // Complete 500k 7/8 MHz
	383000, // Complete 500k 7/8 MHz
	383500, // Complete 500k 7/8 MHz
	384000, // Complete 500k 7/8 MHz
	384500, // Complete 500k 7/8 MHz
	385000, // Complete 500k 7/8 MHz
	385500, // Complete 500k 7/8 MHz
	386000, // Complete 500k 7/8 MHz
	386500, // Complete 500k 7/8 MHz
	387000, // Complete 500k 7/8 MHz
	387500, // Complete 500k 7/8 MHz
	388000, // Complete 500k 7/8 MHz
	388500, // Complete 500k 7/8 MHz
	389000, // Complete 500k 7/8 MHz
	389500, // Complete 500k 7/8 MHz
	390000, // Complete 500k 7/8 MHz
	390500, // Complete 500k 7/8 MHz
	391000, // Complete 500k 7/8 MHz
	391500, // Complete 500k 7/8 MHz
	392000, // Complete 500k 7/8 MHz
	392500, // Complete 500k 7/8 MHz
	393000, // Complete 500k 7/8 MHz
	393500, // Complete 500k 7/8 MHz
	394000, // Complete 500k 7/8 MHz
	394500, // Complete 500k 7/8 MHz
	395000, // Complete 500k 7/8 MHz
	395500, // Complete 500k 7/8 MHz
	396000, // Complete 500k 7/8 MHz
	396500, // Complete 500k 7/8 MHz
	397000, // Complete 500k 7/8 MHz
	397500, // Complete 500k 7/8 MHz
	398000, // Complete 500k 7/8 MHz
	398500, // Complete 500k 7/8 MHz
	399000, // Complete 500k 7/8 MHz
	399500, // Complete 500k 7/8 MHz
	400000, // Complete 500k 7/8 MHz
	400500, // Complete 500k 7/8 MHz
	401000, // Complete 500k 7/8 MHz
	401500, // Complete 500k 7/8 MHz
	402000, // Complete 500k 7/8 MHz
	402500, // Complete 500k 7/8 MHz
	403000, // Complete 500k 7/8 MHz
	403500, // Complete 500k 7/8 MHz
	404000, // Complete 500k 7/8 MHz
	404500, // Complete 500k 7/8 MHz
	405000, // Complete 500k 7/8 MHz
	405500, // Complete 500k 7/8 MHz
	406000, // Complete 500k 7/8 MHz
	406500, // Complete 500k 7/8 MHz
	407000, // Complete 500k 7/8 MHz
	407500, // Complete 500k 7/8 MHz
	408000, // Complete 500k 7/8 MHz
	408500, // Complete 500k 7/8 MHz
	409000, // Complete 500k 7/8 MHz
	409500, // Complete 500k 7/8 MHz
	410000, // Complete 500k 7/8 MHz
	410500, // Complete 500k 7/8 MHz
	411000, // Complete 500k 7/8 MHz
	411500, // Complete 500k 7/8 MHz
	412000, // Complete 500k 7/8 MHz
	412500, // Complete 500k 7/8 MHz
	413000, // Complete 500k 7/8 MHz
	413500, // Complete 500k 7/8 MHz
	414000, // Complete 500k 7/8 MHz
	414500, // Complete 500k 7/8 MHz
	415000, // Complete 500k 7/8 MHz
	415500, // Complete 500k 7/8 MHz
	416000, // Complete 500k 7/8 MHz
	416500, // Complete 500k 7/8 MHz
	417000, // Complete 500k 7/8 MHz
	417500, // Complete 500k 7/8 MHz
	418000, // Complete 500k 7/8 MHz
	418500, // Complete 500k 7/8 MHz
	419000, // Complete 500k 7/8 MHz
	419500, // Complete 500k 7/8 MHz
	420000, // Complete 500k 7/8 MHz
	420500, // Complete 500k 7/8 MHz
	421000, // Complete 500k 7/8 MHz
	421500, // Complete 500k 7/8 MHz
	422000, // Complete 500k 7/8 MHz
	422500, // Complete 500k 7/8 MHz
	423000, // Complete 500k 7/8 MHz
	423500, // Complete 500k 7/8 MHz
	424000, // Complete 500k 7/8 MHz
	424500, // Complete 500k 7/8 MHz
	425000, // Complete 500k 7/8 MHz
	425500, // Complete 500k 7/8 MHz
	426000, // Complete 500k 7/8 MHz
	426500, // Complete 500k 7/8 MHz
	427000, // Complete 500k 7/8 MHz
	427500, // Complete 500k 7/8 MHz
	428000, // Complete 500k 7/8 MHz
	428500, // Complete 500k 7/8 MHz
	429000, // Complete 500k 7/8 MHz
	429500, // Complete 500k 7/8 MHz
	430000, // Complete 500k 7/8 MHz
	430500, // Complete 500k 7/8 MHz
	431000, // Complete 500k 7/8 MHz
	431500, // Complete 500k 7/8 MHz
	432000, // Complete 500k 7/8 MHz
	432500, // Complete 500k 7/8 MHz
	433000, // Complete 500k 7/8 MHz
	433500, // Complete 500k 7/8 MHz
	434000, // Complete 500k 7/8 MHz
	434500, // Complete 500k 7/8 MHz
	435000, // Complete 500k 7/8 MHz
	435500, // Complete 500k 7/8 MHz
	436000, // Complete 500k 7/8 MHz
	436500, // Complete 500k 7/8 MHz
	437000, // Complete 500k 7/8 MHz
	437500, // Complete 500k 7/8 MHz
	438000, // Complete 500k 7/8 MHz
	438500, // Complete 500k 7/8 MHz
	439000, // Complete 500k 7/8 MHz
	439500, // Complete 500k 7/8 MHz
	440000, // Complete 500k 7/8 MHz
	440500, // Complete 500k 7/8 MHz
	441000, // Complete 500k 7/8 MHz
	441500, // Complete 500k 7/8 MHz
	442000, // Complete 500k 7/8 MHz
	442500, // Complete 500k 7/8 MHz
	443000, // Complete 500k 7/8 MHz
	443500, // Complete 500k 7/8 MHz
	444000, // Complete 500k 7/8 MHz
	444500, // Complete 500k 7/8 MHz
	445000, // Complete 500k 7/8 MHz
	445500, // Complete 500k 7/8 MHz
	446000, // Complete 500k 7/8 MHz
	446500, // Complete 500k 7/8 MHz
	447000, // Complete 500k 7/8 MHz
	447500, // Complete 500k 7/8 MHz
	448000, // Complete 500k 7/8 MHz
	448500, // Complete 500k 7/8 MHz
	449000, // Complete 500k 7/8 MHz
	449500, // Complete 500k 7/8 MHz
	450000, // Complete 500k 7/8 MHz
	450500, // Complete 500k 7/8 MHz
	451000, // Complete 500k 7/8 MHz
	451500, // Complete 500k 7/8 MHz
	452000, // Complete 500k 7/8 MHz
	452500, // Complete 500k 7/8 MHz
	453000, // Complete 500k 7/8 MHz
	453500, // Complete 500k 7/8 MHz
	454000, // Complete 500k 7/8 MHz
	454500, // Complete 500k 7/8 MHz
	455000, // Complete 500k 7/8 MHz
	455500, // Complete 500k 7/8 MHz
	456000, // Complete 500k 7/8 MHz
	456500, // Complete 500k 7/8 MHz
	457000, // Complete 500k 7/8 MHz
	457500, // Complete 500k 7/8 MHz
	458000, // Complete 500k 7/8 MHz
	458500, // Complete 500k 7/8 MHz
	459000, // Complete 500k 7/8 MHz
	459500, // Complete 500k 7/8 MHz
	460000, // Complete 500k 7/8 MHz
	460500, // Complete 500k 7/8 MHz
	461000, // Complete 500k 7/8 MHz
	461500, // Complete 500k 7/8 MHz
	462000, // Complete 500k 7/8 MHz
	462500, // Complete 500k 7/8 MHz
	463000, // Complete 500k 7/8 MHz
	463500, // Complete 500k 7/8 MHz
	464000, // Complete 500k 7/8 MHz
	464500, // Complete 500k 7/8 MHz
	465000, // Complete 500k 7/8 MHz
	465500, // Complete 500k 7/8 MHz
	466000, // Complete 500k 7/8 MHz
	466500, // Complete 500k 7/8 MHz
	467000, // Complete 500k 7/8 MHz
	467500, // Complete 500k 7/8 MHz
	468000, // Complete 500k 7/8 MHz
	468500, // Complete 500k 7/8 MHz
	469000, // Complete 500k 7/8 MHz
	469500, // Complete 500k 7/8 MHz
	470000, // Complete 500k 7/8 MHz
	470500, // Complete 500k 7/8 MHz
	471000, // Complete 500k 7/8 MHz
	471500, // Complete 500k 7/8 MHz
	472000, // Complete 500k 7/8 MHz
	472500, // Complete 500k 7/8 MHz
	473000, // Complete 500k 7/8 MHz
	473500, // Complete 500k 7/8 MHz
	474000, // Complete 500k 7/8 MHz
	474500, // Complete 500k 7/8 MHz
	475000, // Complete 500k 7/8 MHz
	475500, // Complete 500k 7/8 MHz
	476000, // Complete 500k 7/8 MHz
	476500, // Complete 500k 7/8 MHz
	477000, // Complete 500k 7/8 MHz
	477500, // Complete 500k 7/8 MHz
	478000, // Complete 500k 7/8 MHz
	478500, // Complete 500k 7/8 MHz
	479000, // Complete 500k 7/8 MHz
	479500, // Complete 500k 7/8 MHz
	480000, // Complete 500k 7/8 MHz
	480500, // Complete 500k 7/8 MHz
	481000, // Complete 500k 7/8 MHz
	481500, // Complete 500k 7/8 MHz
	482000, // Complete 500k 7/8 MHz
	482500, // Complete 500k 7/8 MHz
	483000, // Complete 500k 7/8 MHz
	483500, // Complete 500k 7/8 MHz
	484000, // Complete 500k 7/8 MHz
	484500, // Complete 500k 7/8 MHz
	485000, // Complete 500k 7/8 MHz
	485500, // Complete 500k 7/8 MHz
	486000, // Complete 500k 7/8 MHz
	486500, // Complete 500k 7/8 MHz
	487000, // Complete 500k 7/8 MHz
	487500, // Complete 500k 7/8 MHz
	488000, // Complete 500k 7/8 MHz
	488500, // Complete 500k 7/8 MHz
	489000, // Complete 500k 7/8 MHz
	489500, // Complete 500k 7/8 MHz
	490000, // Complete 500k 7/8 MHz
	490500, // Complete 500k 7/8 MHz
	491000, // Complete 500k 7/8 MHz
	491500, // Complete 500k 7/8 MHz
	492000, // Complete 500k 7/8 MHz
	492500, // Complete 500k 7/8 MHz
	493000, // Complete 500k 7/8 MHz
	493500, // Complete 500k 7/8 MHz
	494000, // Complete 500k 7/8 MHz
	494500, // Complete 500k 7/8 MHz
	495000, // Complete 500k 7/8 MHz
	495500, // Complete 500k 7/8 MHz
	496000, // Complete 500k 7/8 MHz
	496500, // Complete 500k 7/8 MHz
	497000, // Complete 500k 7/8 MHz
	497500, // Complete 500k 7/8 MHz
	498000, // Complete 500k 7/8 MHz
	498500, // Complete 500k 7/8 MHz
	499000, // Complete 500k 7/8 MHz
	499500, // Complete 500k 7/8 MHz
	500000, // Complete 500k 7/8 MHz
	500500, // Complete 500k 7/8 MHz
	501000, // Complete 500k 7/8 MHz
	501500, // Complete 500k 7/8 MHz
	502000, // Complete 500k 7/8 MHz
	502500, // Complete 500k 7/8 MHz
	503000, // Complete 500k 7/8 MHz
	503500, // Complete 500k 7/8 MHz
	504000, // Complete 500k 7/8 MHz
	504500, // Complete 500k 7/8 MHz
	505000, // Complete 500k 7/8 MHz
	505500, // Complete 500k 7/8 MHz
	506000, // Complete 500k 7/8 MHz
	506500, // Complete 500k 7/8 MHz
	507000, // Complete 500k 7/8 MHz
	507500, // Complete 500k 7/8 MHz
	508000, // Complete 500k 7/8 MHz
	508500, // Complete 500k 7/8 MHz
	509000, // Complete 500k 7/8 MHz
	509500, // Complete 500k 7/8 MHz
	510000, // Complete 500k 7/8 MHz
	510500, // Complete 500k 7/8 MHz
	511000, // Complete 500k 7/8 MHz
	511500, // Complete 500k 7/8 MHz
	512000, // Complete 500k 7/8 MHz
	512500, // Complete 500k 7/8 MHz
	513000, // Complete 500k 7/8 MHz
	513500, // Complete 500k 7/8 MHz
	514000, // Complete 500k 7/8 MHz
	514500, // Complete 500k 7/8 MHz
	515000, // Complete 500k 7/8 MHz
	515500, // Complete 500k 7/8 MHz
	516000, // Complete 500k 7/8 MHz
	516500, // Complete 500k 7/8 MHz
	517000, // Complete 500k 7/8 MHz
	517500, // Complete 500k 7/8 MHz
	518000, // Complete 500k 7/8 MHz
	518500, // Complete 500k 7/8 MHz
	519000, // Complete 500k 7/8 MHz
	519500, // Complete 500k 7/8 MHz
	520000, // Complete 500k 7/8 MHz
	520500, // Complete 500k 7/8 MHz
	521000, // Complete 500k 7/8 MHz
	521500, // Complete 500k 7/8 MHz
	522000, // Complete 500k 7/8 MHz
	522500, // Complete 500k 7/8 MHz
	523000, // Complete 500k 7/8 MHz
	523500, // Complete 500k 7/8 MHz
	524000, // Complete 500k 7/8 MHz
	524500, // Complete 500k 7/8 MHz
	525000, // Complete 500k 7/8 MHz
	525500, // Complete 500k 7/8 MHz
	526000, // Complete 500k 7/8 MHz
	526500, // Complete 500k 7/8 MHz
	527000, // Complete 500k 7/8 MHz
	527500, // Complete 500k 7/8 MHz
	528000, // Complete 500k 7/8 MHz
	528500, // Complete 500k 7/8 MHz
	529000, // Complete 500k 7/8 MHz
	529500, // Complete 500k 7/8 MHz
	530000, // Complete 500k 7/8 MHz
	530500, // Complete 500k 7/8 MHz
	531000, // Complete 500k 7/8 MHz
	531500, // Complete 500k 7/8 MHz
	532000, // Complete 500k 7/8 MHz
	532500, // Complete 500k 7/8 MHz
	533000, // Complete 500k 7/8 MHz
	533500, // Complete 500k 7/8 MHz
	534000, // Complete 500k 7/8 MHz
	534500, // Complete 500k 7/8 MHz
	535000, // Complete 500k 7/8 MHz
	535500, // Complete 500k 7/8 MHz
	536000, // Complete 500k 7/8 MHz
	536500, // Complete 500k 7/8 MHz
	537000, // Complete 500k 7/8 MHz
	537500, // Complete 500k 7/8 MHz
	538000, // Complete 500k 7/8 MHz
	538500, // Complete 500k 7/8 MHz
	539000, // Complete 500k 7/8 MHz
	539500, // Complete 500k 7/8 MHz
	540000, // Complete 500k 7/8 MHz
	540500, // Complete 500k 7/8 MHz
	541000, // Complete 500k 7/8 MHz
	541500, // Complete 500k 7/8 MHz
	542000, // Complete 500k 7/8 MHz
	542500, // Complete 500k 7/8 MHz
	543000, // Complete 500k 7/8 MHz
	543500, // Complete 500k 7/8 MHz
	544000, // Complete 500k 7/8 MHz
	544500, // Complete 500k 7/8 MHz
	545000, // Complete 500k 7/8 MHz
	545500, // Complete 500k 7/8 MHz
	546000, // Complete 500k 7/8 MHz
	546500, // Complete 500k 7/8 MHz
	547000, // Complete 500k 7/8 MHz
	547500, // Complete 500k 7/8 MHz
	548000, // Complete 500k 7/8 MHz
	548500, // Complete 500k 7/8 MHz
	549000, // Complete 500k 7/8 MHz
	549500, // Complete 500k 7/8 MHz
	550000, // Complete 500k 7/8 MHz
	550500, // Complete 500k 7/8 MHz
	551000, // Complete 500k 7/8 MHz
	551500, // Complete 500k 7/8 MHz
	552000, // Complete 500k 7/8 MHz
	552500, // Complete 500k 7/8 MHz
	553000, // Complete 500k 7/8 MHz
	553500, // Complete 500k 7/8 MHz
	554000, // Complete 500k 7/8 MHz
	554500, // Complete 500k 7/8 MHz
	555000, // Complete 500k 7/8 MHz
	555500, // Complete 500k 7/8 MHz
	556000, // Complete 500k 7/8 MHz
	556500, // Complete 500k 7/8 MHz
	557000, // Complete 500k 7/8 MHz
	557500, // Complete 500k 7/8 MHz
	558000, // Complete 500k 7/8 MHz
	558500, // Complete 500k 7/8 MHz
	559000, // Complete 500k 7/8 MHz
	559500, // Complete 500k 7/8 MHz
	560000, // Complete 500k 7/8 MHz
	560500, // Complete 500k 7/8 MHz
	561000, // Complete 500k 7/8 MHz
	561500, // Complete 500k 7/8 MHz
	562000, // Complete 500k 7/8 MHz
	562500, // Complete 500k 7/8 MHz
	563000, // Complete 500k 7/8 MHz
	563500, // Complete 500k 7/8 MHz
	564000, // Complete 500k 7/8 MHz
	564500, // Complete 500k 7/8 MHz
	565000, // Complete 500k 7/8 MHz
	565500, // Complete 500k 7/8 MHz
	566000, // Complete 500k 7/8 MHz
	566500, // Complete 500k 7/8 MHz
	567000, // Complete 500k 7/8 MHz
	567500, // Complete 500k 7/8 MHz
	568000, // Complete 500k 7/8 MHz
	568500, // Complete 500k 7/8 MHz
	569000, // Complete 500k 7/8 MHz
	569500, // Complete 500k 7/8 MHz
	570000, // Complete 500k 7/8 MHz
	570500, // Complete 500k 7/8 MHz
	571000, // Complete 500k 7/8 MHz
	571500, // Complete 500k 7/8 MHz
	572000, // Complete 500k 7/8 MHz
	572500, // Complete 500k 7/8 MHz
	573000, // Complete 500k 7/8 MHz
	573500, // Complete 500k 7/8 MHz
	574000, // Complete 500k 7/8 MHz
	574500, // Complete 500k 7/8 MHz
	575000, // Complete 500k 7/8 MHz
	575500, // Complete 500k 7/8 MHz
	576000, // Complete 500k 7/8 MHz
	576500, // Complete 500k 7/8 MHz
	577000, // Complete 500k 7/8 MHz
	577500, // Complete 500k 7/8 MHz
	578000, // Complete 500k 7/8 MHz
	578500, // Complete 500k 7/8 MHz
	579000, // Complete 500k 7/8 MHz
	579500, // Complete 500k 7/8 MHz
	580000, // Complete 500k 7/8 MHz
	580500, // Complete 500k 7/8 MHz
	581000, // Complete 500k 7/8 MHz
	581500, // Complete 500k 7/8 MHz
	582000, // Complete 500k 7/8 MHz
	582500, // Complete 500k 7/8 MHz
	583000, // Complete 500k 7/8 MHz
	583500, // Complete 500k 7/8 MHz
	584000, // Complete 500k 7/8 MHz
	584500, // Complete 500k 7/8 MHz
	585000, // Complete 500k 7/8 MHz
	585500, // Complete 500k 7/8 MHz
	586000, // Complete 500k 7/8 MHz
	586500, // Complete 500k 7/8 MHz
	587000, // Complete 500k 7/8 MHz
	587500, // Complete 500k 7/8 MHz
	588000, // Complete 500k 7/8 MHz
	588500, // Complete 500k 7/8 MHz
	589000, // Complete 500k 7/8 MHz
	589500, // Complete 500k 7/8 MHz
	590000, // Complete 500k 7/8 MHz
	590500, // Complete 500k 7/8 MHz
	591000, // Complete 500k 7/8 MHz
	591500, // Complete 500k 7/8 MHz
	592000, // Complete 500k 7/8 MHz
	592500, // Complete 500k 7/8 MHz
	593000, // Complete 500k 7/8 MHz
	593500, // Complete 500k 7/8 MHz
	594000, // Complete 500k 7/8 MHz
	594500, // Complete 500k 7/8 MHz
	595000, // Complete 500k 7/8 MHz
	595500, // Complete 500k 7/8 MHz
	596000, // Complete 500k 7/8 MHz
	596500, // Complete 500k 7/8 MHz
	597000, // Complete 500k 7/8 MHz
	597500, // Complete 500k 7/8 MHz
	598000, // Complete 500k 7/8 MHz
	598500, // Complete 500k 7/8 MHz
	599000, // Complete 500k 7/8 MHz
	599500, // Complete 500k 7/8 MHz
	600000, // Complete 500k 7/8 MHz
	600500, // Complete 500k 7/8 MHz
	601000, // Complete 500k 7/8 MHz
	601500, // Complete 500k 7/8 MHz
	602000, // Complete 500k 7/8 MHz
	602500, // Complete 500k 7/8 MHz
	603000, // Complete 500k 7/8 MHz
	603500, // Complete 500k 7/8 MHz
	604000, // Complete 500k 7/8 MHz
	604500, // Complete 500k 7/8 MHz
	605000, // Complete 500k 7/8 MHz
	605500, // Complete 500k 7/8 MHz
	606000, // Complete 500k 7/8 MHz
	606500, // Complete 500k 7/8 MHz
	607000, // Complete 500k 7/8 MHz
	607500, // Complete 500k 7/8 MHz
	608000, // Complete 500k 7/8 MHz
	608500, // Complete 500k 7/8 MHz
	609000, // Complete 500k 7/8 MHz
	609500, // Complete 500k 7/8 MHz
	610000, // Complete 500k 7/8 MHz
	610500, // Complete 500k 7/8 MHz
	611000, // Complete 500k 7/8 MHz
	611500, // Complete 500k 7/8 MHz
	612000, // Complete 500k 7/8 MHz
	612500, // Complete 500k 7/8 MHz
	613000, // Complete 500k 7/8 MHz
	613500, // Complete 500k 7/8 MHz
	614000, // Complete 500k 7/8 MHz
	614500, // Complete 500k 7/8 MHz
	615000, // Complete 500k 7/8 MHz
	615500, // Complete 500k 7/8 MHz
	616000, // Complete 500k 7/8 MHz
	616500, // Complete 500k 7/8 MHz
	617000, // Complete 500k 7/8 MHz
	617500, // Complete 500k 7/8 MHz
	618000, // Complete 500k 7/8 MHz
	618500, // Complete 500k 7/8 MHz
	619000, // Complete 500k 7/8 MHz
	619500, // Complete 500k 7/8 MHz
	620000, // Complete 500k 7/8 MHz
	620500, // Complete 500k 7/8 MHz
	621000, // Complete 500k 7/8 MHz
	621500, // Complete 500k 7/8 MHz
	622000, // Complete 500k 7/8 MHz
	622500, // Complete 500k 7/8 MHz
	623000, // Complete 500k 7/8 MHz
	623500, // Complete 500k 7/8 MHz
	624000, // Complete 500k 7/8 MHz
	624500, // Complete 500k 7/8 MHz
	625000, // Complete 500k 7/8 MHz
	625500, // Complete 500k 7/8 MHz
	626000, // Complete 500k 7/8 MHz
	626500, // Complete 500k 7/8 MHz
	627000, // Complete 500k 7/8 MHz
	627500, // Complete 500k 7/8 MHz
	628000, // Complete 500k 7/8 MHz
	628500, // Complete 500k 7/8 MHz
	629000, // Complete 500k 7/8 MHz
	629500, // Complete 500k 7/8 MHz
	630000, // Complete 500k 7/8 MHz
	630500, // Complete 500k 7/8 MHz
	631000, // Complete 500k 7/8 MHz
	631500, // Complete 500k 7/8 MHz
	632000, // Complete 500k 7/8 MHz
	632500, // Complete 500k 7/8 MHz
	633000, // Complete 500k 7/8 MHz
	633500, // Complete 500k 7/8 MHz
	634000, // Complete 500k 7/8 MHz
	634500, // Complete 500k 7/8 MHz
	635000, // Complete 500k 7/8 MHz
	635500, // Complete 500k 7/8 MHz
	636000, // Complete 500k 7/8 MHz
	636500, // Complete 500k 7/8 MHz
	637000, // Complete 500k 7/8 MHz
	637500, // Complete 500k 7/8 MHz
	638000, // Complete 500k 7/8 MHz
	638500, // Complete 500k 7/8 MHz
	639000, // Complete 500k 7/8 MHz
	639500, // Complete 500k 7/8 MHz
	640000, // Complete 500k 7/8 MHz
	640500, // Complete 500k 7/8 MHz
	641000, // Complete 500k 7/8 MHz
	641500, // Complete 500k 7/8 MHz
	642000, // Complete 500k 7/8 MHz
	642500, // Complete 500k 7/8 MHz
	643000, // Complete 500k 7/8 MHz
	643500, // Complete 500k 7/8 MHz
	644000, // Complete 500k 7/8 MHz
	644500, // Complete 500k 7/8 MHz
	645000, // Complete 500k 7/8 MHz
	645500, // Complete 500k 7/8 MHz
	646000, // Complete 500k 7/8 MHz
	646500, // Complete 500k 7/8 MHz
	647000, // Complete 500k 7/8 MHz
	647500, // Complete 500k 7/8 MHz
	648000, // Complete 500k 7/8 MHz
	648500, // Complete 500k 7/8 MHz
	649000, // Complete 500k 7/8 MHz
	649500, // Complete 500k 7/8 MHz
	650000, // Complete 500k 7/8 MHz
	650500, // Complete 500k 7/8 MHz
	651000, // Complete 500k 7/8 MHz
	651500, // Complete 500k 7/8 MHz
	652000, // Complete 500k 7/8 MHz
	652500, // Complete 500k 7/8 MHz
	653000, // Complete 500k 7/8 MHz
	653500, // Complete 500k 7/8 MHz
	654000, // Complete 500k 7/8 MHz
	654500, // Complete 500k 7/8 MHz
	655000, // Complete 500k 7/8 MHz
	655500, // Complete 500k 7/8 MHz
	656000, // Complete 500k 7/8 MHz
	656500, // Complete 500k 7/8 MHz
	657000, // Complete 500k 7/8 MHz
	657500, // Complete 500k 7/8 MHz
	658000, // Complete 500k 7/8 MHz
	658500, // Complete 500k 7/8 MHz
	659000, // Complete 500k 7/8 MHz
	659500, // Complete 500k 7/8 MHz
	660000, // Complete 500k 7/8 MHz
	660500, // Complete 500k 7/8 MHz
	661000, // Complete 500k 7/8 MHz
	661500, // Complete 500k 7/8 MHz
	662000, // Complete 500k 7/8 MHz
	662500, // Complete 500k 7/8 MHz
	663000, // Complete 500k 7/8 MHz
	663500, // Complete 500k 7/8 MHz
	664000, // Complete 500k 7/8 MHz
	664500, // Complete 500k 7/8 MHz
	665000, // Complete 500k 7/8 MHz
	665500, // Complete 500k 7/8 MHz
	666000, // Complete 500k 7/8 MHz
	666500, // Complete 500k 7/8 MHz
	667000, // Complete 500k 7/8 MHz
	667500, // Complete 500k 7/8 MHz
	668000, // Complete 500k 7/8 MHz
	668500, // Complete 500k 7/8 MHz
	669000, // Complete 500k 7/8 MHz
	669500, // Complete 500k 7/8 MHz
	670000, // Complete 500k 7/8 MHz
	670500, // Complete 500k 7/8 MHz
	671000, // Complete 500k 7/8 MHz
	671500, // Complete 500k 7/8 MHz
	672000, // Complete 500k 7/8 MHz
	672500, // Complete 500k 7/8 MHz
	673000, // Complete 500k 7/8 MHz
	673500, // Complete 500k 7/8 MHz
	674000, // Complete 500k 7/8 MHz
	674500, // Complete 500k 7/8 MHz
	675000, // Complete 500k 7/8 MHz
	675500, // Complete 500k 7/8 MHz
	676000, // Complete 500k 7/8 MHz
	676500, // Complete 500k 7/8 MHz
	677000, // Complete 500k 7/8 MHz
	677500, // Complete 500k 7/8 MHz
	678000, // Complete 500k 7/8 MHz
	678500, // Complete 500k 7/8 MHz
	679000, // Complete 500k 7/8 MHz
	679500, // Complete 500k 7/8 MHz
	680000, // Complete 500k 7/8 MHz
	680500, // Complete 500k 7/8 MHz
	681000, // Complete 500k 7/8 MHz
	681500, // Complete 500k 7/8 MHz
	682000, // Complete 500k 7/8 MHz
	682500, // Complete 500k 7/8 MHz
	683000, // Complete 500k 7/8 MHz
	683500, // Complete 500k 7/8 MHz
	684000, // Complete 500k 7/8 MHz
	684500, // Complete 500k 7/8 MHz
	685000, // Complete 500k 7/8 MHz
	685500, // Complete 500k 7/8 MHz
	686000, // Complete 500k 7/8 MHz
	686500, // Complete 500k 7/8 MHz
	687000, // Complete 500k 7/8 MHz
	687500, // Complete 500k 7/8 MHz
	688000, // Complete 500k 7/8 MHz
	688500, // Complete 500k 7/8 MHz
	689000, // Complete 500k 7/8 MHz
	689500, // Complete 500k 7/8 MHz
	690000, // Complete 500k 7/8 MHz
	690500, // Complete 500k 7/8 MHz
	691000, // Complete 500k 7/8 MHz
	691500, // Complete 500k 7/8 MHz
	692000, // Complete 500k 7/8 MHz
	692500, // Complete 500k 7/8 MHz
	693000, // Complete 500k 7/8 MHz
	693500, // Complete 500k 7/8 MHz
	694000, // Complete 500k 7/8 MHz
	694500, // Complete 500k 7/8 MHz
	695000, // Complete 500k 7/8 MHz
	695500, // Complete 500k 7/8 MHz
	696000, // Complete 500k 7/8 MHz
	696500, // Complete 500k 7/8 MHz
	697000, // Complete 500k 7/8 MHz
	697500, // Complete 500k 7/8 MHz
	698000, // Complete 500k 7/8 MHz
	698500, // Complete 500k 7/8 MHz
	699000, // Complete 500k 7/8 MHz
	699500, // Complete 500k 7/8 MHz
	700000, // Complete 500k 7/8 MHz
	700500, // Complete 500k 7/8 MHz
	701000, // Complete 500k 7/8 MHz
	701500, // Complete 500k 7/8 MHz
	702000, // Complete 500k 7/8 MHz
	702500, // Complete 500k 7/8 MHz
	703000, // Complete 500k 7/8 MHz
	703500, // Complete 500k 7/8 MHz
	704000, // Complete 500k 7/8 MHz
	704500, // Complete 500k 7/8 MHz
	705000, // Complete 500k 7/8 MHz
	705500, // Complete 500k 7/8 MHz
	706000, // Complete 500k 7/8 MHz
	706500, // Complete 500k 7/8 MHz
	707000, // Complete 500k 7/8 MHz
	707500, // Complete 500k 7/8 MHz
	708000, // Complete 500k 7/8 MHz
	708500, // Complete 500k 7/8 MHz
	709000, // Complete 500k 7/8 MHz
	709500, // Complete 500k 7/8 MHz
	710000, // Complete 500k 7/8 MHz
	710500, // Complete 500k 7/8 MHz
	711000, // Complete 500k 7/8 MHz
	711500, // Complete 500k 7/8 MHz
	712000, // Complete 500k 7/8 MHz
	712500, // Complete 500k 7/8 MHz
	713000, // Complete 500k 7/8 MHz
	713500, // Complete 500k 7/8 MHz
	714000, // Complete 500k 7/8 MHz
	714500, // Complete 500k 7/8 MHz
	715000, // Complete 500k 7/8 MHz
	715500, // Complete 500k 7/8 MHz
	716000, // Complete 500k 7/8 MHz
	716500, // Complete 500k 7/8 MHz
	717000, // Complete 500k 7/8 MHz
	717500, // Complete 500k 7/8 MHz
	718000, // Complete 500k 7/8 MHz
	718500, // Complete 500k 7/8 MHz
	719000, // Complete 500k 7/8 MHz
	719500, // Complete 500k 7/8 MHz
	720000, // Complete 500k 7/8 MHz
	720500, // Complete 500k 7/8 MHz
	721000, // Complete 500k 7/8 MHz
	721500, // Complete 500k 7/8 MHz
	722000, // Complete 500k 7/8 MHz
	722500, // Complete 500k 7/8 MHz
	723000, // Complete 500k 7/8 MHz
	723500, // Complete 500k 7/8 MHz
	724000, // Complete 500k 7/8 MHz
	724500, // Complete 500k 7/8 MHz
	725000, // Complete 500k 7/8 MHz
	725500, // Complete 500k 7/8 MHz
	726000, // Complete 500k 7/8 MHz
	726500, // Complete 500k 7/8 MHz
	727000, // Complete 500k 7/8 MHz
	727500, // Complete 500k 7/8 MHz
	728000, // Complete 500k 7/8 MHz
	728500, // Complete 500k 7/8 MHz
	729000, // Complete 500k 7/8 MHz
	729500, // Complete 500k 7/8 MHz
	730000, // Complete 500k 7/8 MHz
	730500, // Complete 500k 7/8 MHz
	731000, // Complete 500k 7/8 MHz
	731500, // Complete 500k 7/8 MHz
	732000, // Complete 500k 7/8 MHz
	732500, // Complete 500k 7/8 MHz
	733000, // Complete 500k 7/8 MHz
	733500, // Complete 500k 7/8 MHz
	734000, // Complete 500k 7/8 MHz
	734500, // Complete 500k 7/8 MHz
	735000, // Complete 500k 7/8 MHz
	735500, // Complete 500k 7/8 MHz
	736000, // Complete 500k 7/8 MHz
	736500, // Complete 500k 7/8 MHz
	737000, // Complete 500k 7/8 MHz
	737500, // Complete 500k 7/8 MHz
	738000, // Complete 500k 7/8 MHz
	738500, // Complete 500k 7/8 MHz
	739000, // Complete 500k 7/8 MHz
	739500, // Complete 500k 7/8 MHz
	740000, // Complete 500k 7/8 MHz
	740500, // Complete 500k 7/8 MHz
	741000, // Complete 500k 7/8 MHz
	741500, // Complete 500k 7/8 MHz
	742000, // Complete 500k 7/8 MHz
	742500, // Complete 500k 7/8 MHz
	743000, // Complete 500k 7/8 MHz
	743500, // Complete 500k 7/8 MHz
	744000, // Complete 500k 7/8 MHz
	744500, // Complete 500k 7/8 MHz
	745000, // Complete 500k 7/8 MHz
	745500, // Complete 500k 7/8 MHz
	746000, // Complete 500k 7/8 MHz
	746500, // Complete 500k 7/8 MHz
	747000, // Complete 500k 7/8 MHz
	747500, // Complete 500k 7/8 MHz
	748000, // Complete 500k 7/8 MHz
	748500, // Complete 500k 7/8 MHz
	749000, // Complete 500k 7/8 MHz
	749500, // Complete 500k 7/8 MHz
	750000, // Complete 500k 7/8 MHz
	750500, // Complete 500k 7/8 MHz
	751000, // Complete 500k 7/8 MHz
	751500, // Complete 500k 7/8 MHz
	752000, // Complete 500k 7/8 MHz
	752500, // Complete 500k 7/8 MHz
	753000, // Complete 500k 7/8 MHz
	753500, // Complete 500k 7/8 MHz
	754000, // Complete 500k 7/8 MHz
	754500, // Complete 500k 7/8 MHz
	755000, // Complete 500k 7/8 MHz
	755500, // Complete 500k 7/8 MHz
	756000, // Complete 500k 7/8 MHz
	756500, // Complete 500k 7/8 MHz
	757000, // Complete 500k 7/8 MHz
	757500, // Complete 500k 7/8 MHz
	758000, // Complete 500k 7/8 MHz
	758500, // Complete 500k 7/8 MHz
	759000, // Complete 500k 7/8 MHz
	759500, // Complete 500k 7/8 MHz
	760000, // Complete 500k 7/8 MHz
	760500, // Complete 500k 7/8 MHz
	761000, // Complete 500k 7/8 MHz
	761500, // Complete 500k 7/8 MHz
	762000, // Complete 500k 7/8 MHz
	762500, // Complete 500k 7/8 MHz
	763000, // Complete 500k 7/8 MHz
	763500, // Complete 500k 7/8 MHz
	764000, // Complete 500k 7/8 MHz
	764500, // Complete 500k 7/8 MHz
	765000, // Complete 500k 7/8 MHz
	765500, // Complete 500k 7/8 MHz
	766000, // Complete 500k 7/8 MHz
	766500, // Complete 500k 7/8 MHz
	767000, // Complete 500k 7/8 MHz
	767500, // Complete 500k 7/8 MHz
	768000, // Complete 500k 7/8 MHz
	768500, // Complete 500k 7/8 MHz
	769000, // Complete 500k 7/8 MHz
	769500, // Complete 500k 7/8 MHz
	770000, // Complete 500k 7/8 MHz
	770500, // Complete 500k 7/8 MHz
	771000, // Complete 500k 7/8 MHz
	771500, // Complete 500k 7/8 MHz
	772000, // Complete 500k 7/8 MHz
	772500, // Complete 500k 7/8 MHz
	773000, // Complete 500k 7/8 MHz
	773500, // Complete 500k 7/8 MHz
	774000, // Complete 500k 7/8 MHz
	774500, // Complete 500k 7/8 MHz
	775000, // Complete 500k 7/8 MHz
	775500, // Complete 500k 7/8 MHz
	776000, // Complete 500k 7/8 MHz
	776500, // Complete 500k 7/8 MHz
	777000, // Complete 500k 7/8 MHz
	777500, // Complete 500k 7/8 MHz
	778000, // Complete 500k 7/8 MHz
	778500, // Complete 500k 7/8 MHz
	779000, // Complete 500k 7/8 MHz
	779500, // Complete 500k 7/8 MHz
	780000, // Complete 500k 7/8 MHz
	780500, // Complete 500k 7/8 MHz
	781000, // Complete 500k 7/8 MHz
	781500, // Complete 500k 7/8 MHz
	782000, // Complete 500k 7/8 MHz
	782500, // Complete 500k 7/8 MHz
	783000, // Complete 500k 7/8 MHz
	783500, // Complete 500k 7/8 MHz
	784000, // Complete 500k 7/8 MHz
	784500, // Complete 500k 7/8 MHz
	785000, // Complete 500k 7/8 MHz
	785500, // Complete 500k 7/8 MHz
	786000, // Complete 500k 7/8 MHz
	786500, // Complete 500k 7/8 MHz
	787000, // Complete 500k 7/8 MHz
	787500, // Complete 500k 7/8 MHz
	788000, // Complete 500k 7/8 MHz
	788500, // Complete 500k 7/8 MHz
	789000, // Complete 500k 7/8 MHz
	789500, // Complete 500k 7/8 MHz
	790000, // Complete 500k 7/8 MHz
	790500, // Complete 500k 7/8 MHz
	791000, // Complete 500k 7/8 MHz
	791500, // Complete 500k 7/8 MHz
	792000, // Complete 500k 7/8 MHz
	792500, // Complete 500k 7/8 MHz
	793000, // Complete 500k 7/8 MHz
	793500, // Complete 500k 7/8 MHz
	794000, // Complete 500k 7/8 MHz
	794500, // Complete 500k 7/8 MHz
	795000, // Complete 500k 7/8 MHz
	795500, // Complete 500k 7/8 MHz
	796000, // Complete 500k 7/8 MHz
	796500, // Complete 500k 7/8 MHz
	797000, // Complete 500k 7/8 MHz
	797500, // Complete 500k 7/8 MHz
	798000, // Complete 500k 7/8 MHz
	798500, // Complete 500k 7/8 MHz
	799000, // Complete 500k 7/8 MHz
	799500, // Complete 500k 7/8 MHz
	800000, // Complete 500k 7/8 MHz
	800500, // Complete 500k 7/8 MHz
	801000, // Complete 500k 7/8 MHz
	801500, // Complete 500k 7/8 MHz
	802000, // Complete 500k 7/8 MHz
	802500, // Complete 500k 7/8 MHz
	803000, // Complete 500k 7/8 MHz
	803500, // Complete 500k 7/8 MHz
	804000, // Complete 500k 7/8 MHz
	804500, // Complete 500k 7/8 MHz
	805000, // Complete 500k 7/8 MHz
	805500, // Complete 500k 7/8 MHz
	806000, // Complete 500k 7/8 MHz
	806500, // Complete 500k 7/8 MHz
	807000, // Complete 500k 7/8 MHz
	807500, // Complete 500k 7/8 MHz
	808000, // Complete 500k 7/8 MHz
	808500, // Complete 500k 7/8 MHz
	809000, // Complete 500k 7/8 MHz
	809500, // Complete 500k 7/8 MHz
	810000, // Complete 500k 7/8 MHz
	810500, // Complete 500k 7/8 MHz
	811000, // Complete 500k 7/8 MHz
	811500, // Complete 500k 7/8 MHz
	812000, // Complete 500k 7/8 MHz
	812500, // Complete 500k 7/8 MHz
	813000, // Complete 500k 7/8 MHz
	813500, // Complete 500k 7/8 MHz
	814000, // Complete 500k 7/8 MHz
	814500, // Complete 500k 7/8 MHz
	815000, // Complete 500k 7/8 MHz
	815500, // Complete 500k 7/8 MHz
	816000, // Complete 500k 7/8 MHz
	816500, // Complete 500k 7/8 MHz
	817000, // Complete 500k 7/8 MHz
	817500, // Complete 500k 7/8 MHz
	818000, // Complete 500k 7/8 MHz
	818500, // Complete 500k 7/8 MHz
	819000, // Complete 500k 7/8 MHz
	819500, // Complete 500k 7/8 MHz
	820000, // Complete 500k 7/8 MHz
	820500, // Complete 500k 7/8 MHz
	821000, // Complete 500k 7/8 MHz
	821500, // Complete 500k 7/8 MHz
	822000, // Complete 500k 7/8 MHz
	822500, // Complete 500k 7/8 MHz
	823000, // Complete 500k 7/8 MHz
	823500, // Complete 500k 7/8 MHz
	824000, // Complete 500k 7/8 MHz
	824500, // Complete 500k 7/8 MHz
	825000, // Complete 500k 7/8 MHz
	825500, // Complete 500k 7/8 MHz
	826000, // Complete 500k 7/8 MHz
	826500, // Complete 500k 7/8 MHz
	827000, // Complete 500k 7/8 MHz
	827500, // Complete 500k 7/8 MHz
	828000, // Complete 500k 7/8 MHz
	828500, // Complete 500k 7/8 MHz
	829000, // Complete 500k 7/8 MHz
	829500, // Complete 500k 7/8 MHz
	830000, // Complete 500k 7/8 MHz
	830500, // Complete 500k 7/8 MHz
	831000, // Complete 500k 7/8 MHz
	831500, // Complete 500k 7/8 MHz
	832000, // Complete 500k 7/8 MHz
	832500, // Complete 500k 7/8 MHz
	833000, // Complete 500k 7/8 MHz
	833500, // Complete 500k 7/8 MHz
	834000, // Complete 500k 7/8 MHz
	834500, // Complete 500k 7/8 MHz
	835000, // Complete 500k 7/8 MHz
	835500, // Complete 500k 7/8 MHz
	836000, // Complete 500k 7/8 MHz
	836500, // Complete 500k 7/8 MHz
	837000, // Complete 500k 7/8 MHz
	837500, // Complete 500k 7/8 MHz
	838000, // Complete 500k 7/8 MHz
	838500, // Complete 500k 7/8 MHz
	839000, // Complete 500k 7/8 MHz
	839500, // Complete 500k 7/8 MHz
	840000, // Complete 500k 7/8 MHz
	840500, // Complete 500k 7/8 MHz
	841000, // Complete 500k 7/8 MHz
	841500, // Complete 500k 7/8 MHz
	842000, // Complete 500k 7/8 MHz
	842500, // Complete 500k 7/8 MHz
	843000, // Complete 500k 7/8 MHz
	843500, // Complete 500k 7/8 MHz
	844000, // Complete 500k 7/8 MHz
	844500, // Complete 500k 7/8 MHz
	845000, // Complete 500k 7/8 MHz
	845500, // Complete 500k 7/8 MHz
	846000, // Complete 500k 7/8 MHz
	846500, // Complete 500k 7/8 MHz
	847000, // Complete 500k 7/8 MHz
	847500, // Complete 500k 7/8 MHz
	848000, // Complete 500k 7/8 MHz
	848500, // Complete 500k 7/8 MHz
	849000, // Complete 500k 7/8 MHz
	849500, // Complete 500k 7/8 MHz
	850000, // Complete 500k 7/8 MHz
	850500, // Complete 500k 7/8 MHz
	851000, // Complete 500k 7/8 MHz
	851500, // Complete 500k 7/8 MHz
	852000, // Complete 500k 7/8 MHz
	852500, // Complete 500k 7/8 MHz
	853000, // Complete 500k 7/8 MHz
	853500, // Complete 500k 7/8 MHz
	854000, // Complete 500k 7/8 MHz
	854500, // Complete 500k 7/8 MHz
	855000, // Complete 500k 7/8 MHz
	855500, // Complete 500k 7/8 MHz
	856000, // Complete 500k 7/8 MHz
	856500, // Complete 500k 7/8 MHz
	857000, // Complete 500k 7/8 MHz
	857500, // Complete 500k 7/8 MHz
	858000, // Complete 500k 7/8 MHz
	858500, // Complete 500k 7/8 MHz
	0,		// ******************************* end of Complete 500k 7/8 MHz
	177500, // Europe VHF 7 MHz 5
	184500, // Europe VHF 7 MHz 6
	191500, // Europe VHF 7 MHz 7
	198500, // Europe VHF 7 MHz 8
	205500, // Europe VHF 7 MHz 9
	212500, // Europe VHF 7 MHz 10
	219500, // Europe VHF 7 MHz 11
	226500, // Europe VHF 7 MHz 12
	0,      // ******************************* end of Europe VHF 7MHz
	178000, // Europe VHF 8 MHz 6
	186000, // Europe VHF 8 MHz 7
	194000, // Europe VHF 8 MHz 8
	202000, // Europe VHF 8 MHz 9
	210000, // Europe VHF 8 MHz 10
	218000, // Europe VHF 8 MHz 11
	226000, // Europe VHF 8 MHz 12
	0,      // ******************************* end of Europe VHF 8MHz
	474000, // Europe UHF 21 
	482000, // Europe UHF 22
	490000, // Europe UHF 23
	498000, // Europe UHF 24
	506000, // Europe UHF 25
	514000, // Europe UHF 26
	522000, // Europe UHF 27
	530000, // Europe UHF 28
	538000, // Europe UHF 29
	546000, // Europe UHF 30
	554000, // Europe UHF 31
	562000, // Europe UHF 32
	570000, // Europe UHF 33
	578000, // Europe UHF 34
	586000, // Europe UHF 35
	594000, // Europe UHF 36
	602000, // Europe UHF 37
	610000, // Europe UHF 38
	618000, // Europe UHF 39
	626000, // Europe UHF 40
	634000, // Europe UHF 41
	642000, // Europe UHF 42
	650000, // Europe UHF 43
	658000, // Europe UHF 44
	666000, // Europe UHF 45
	674000, // Europe UHF 46
	682000, // Europe UHF 47
	690000, // Europe UHF 48
	698000, // Europe UHF 49
	706000, // Europe UHF 50
	714000, // Europe UHF 51
	722000, // Europe UHF 52
	730000, // Europe UHF 53
	738000, // Europe UHF 54
	746000, // Europe UHF 55
	754000, // Europe UHF 56
	762000, // Europe UHF 57
	770000, // Europe UHF 58
	778000, // Europe UHF 59
	786000, // Europe UHF 60
	794000, // Europe UHF 61
	802000, // Europe UHF 62
	810000, // Europe UHF 63
	818000, // Europe UHF 64
	826000, // Europe UHF 65
	834000, // Europe UHF 66
	842000, // Europe UHF 67
	850000, // Europe UHF 68
	858000,	// Europe UHF 69
	0,      // ******************************* end of Europe UHF
	210700, // France VHF 5
	218700, // France VHF 7
	0,      // ******************************* end of France VHF
	178000, // Ireland VHF D
	186000, // Ireland VHF E
	194000, // Ireland VHF F
	202000, // Ireland VHF G
	210000, // Ireland VHF H
	218000, // Ireland VHF I
	0,      // ******************************* end of Ireland VHF
	 56000, // Italy VHF A
	 64500, // Italy VHF B
	 84500, // Italy VHF C
	177500, // Italy VHF D
	186000, // Italy VHF E
	194500, // Italy VHF F
	203500, // Italy VHF G
	212500, // Italy VHF H
	219500, // Italy VHF H1
	226500, // Italy VHF H2
	0,      // ******************************* end of Italy VHF
	 57500, // New Zealand 2
	 64500, // New Zealand 3
	177500, // New Zealand 4
	184500, // New Zealand 5
	191500, // New Zealand 6
	198500, // New Zealand 7
	205500, // New Zealand 8
	212500, // New Zealand 9
	219500, // New Zealand 10
	0,      // ******************************* end of New Zealand VHF
	474000, // UK UHF 21 
	482000, // UK UHF 22
	490000, // UK UHF 23
	498000, // UK UHF 24
	506000, // UK UHF 25
	514000, // UK UHF 26
	522000, // UK UHF 27
	530000, // UK UHF 28
	538000, // UK UHF 29
	546000, // UK UHF 30
	554000, // UK UHF 31
	562000, // UK UHF 32
	570000, // UK UHF 33
	578000, // UK UHF 34
	586000, // UK UHF 35
	594000, // UK UHF 36
	602000, // UK UHF 37
	610000, // UK UHF 38
	618000, // UK UHF 39
	626000, // UK UHF 40
	634000, // UK UHF 41
	642000, // UK UHF 42
	650000, // UK UHF 43
	658000, // UK UHF 44
	666000, // UK UHF 45
	674000, // UK UHF 46
	682000, // UK UHF 47
	690000, // UK UHF 48
	698000, // UK UHF 49
	706000, // UK UHF 50
	714000, // UK UHF 51
	722000, // UK UHF 52
	730000, // UK UHF 53
	738000, // UK UHF 54
	746000, // UK UHF 55
	754000, // UK UHF 56
	762000, // UK UHF 57
	770000, // UK UHF 58
	778000, // UK UHF 59
	786000, // UK UHF 60
	794000, // UK UHF 61
	802000, // UK UHF 62
	810000, // UK UHF 63
	818000, // UK UHF 64
	826000, // UK UHF 65
	834000, // UK UHF 66
	842000, // UK UHF 67
	850000, // UK UHF 68
	0,      // ******************************* end of UK UHF
	2031500, // USA BAS A1r
	2043500, // USA BAS A2r
	2055500, // USA BAS A3r
	2067500, // USA BAS A4r
	2079500, // USA BAS A5r
	2091500, // USA BAS A6r
	2103500, // USA BAS A7r
	0,      // ******************************* end of USA BAS
	1999000, // USA BAS 1
	2016500, // USA BAS 2
	2033500, // USA BAS 3
	2050500, // USA BAS 4
	2067500, // USA BAS 5
	2084500, // USA BAS 6
	2101500, // USA BAS 7
	2458500, // USA BAS 8 (not really)
	2475500, // USA BAS 9 (not really)
	2492000, // USA BAS 10 (not really)
	0,      // ******************************* end of USA BAS
	0       // end of table
};
