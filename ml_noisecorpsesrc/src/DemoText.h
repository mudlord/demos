/*
	DEMOText.h
*/

struct text
{
	float start;
	float duration;
	float x, y;
	float xspd, yspd;
	float xscl, yscl;
	const char *msg;
};

text demotext[] =
{
	/*
	 BEG  DUR  X    Y   XSP  YSP XSX YSX     TXT 
	*/	

	// End scroller
	{2, 20,  20,   0,   0,  42,  0,  0, "WARNING"},
	
	{4, 20,  20,   0,   0,  42,  0,  0, "The following intro may contain"},
	{5, 20,  20,   0,   0,  42,  0,  0, "scenes sceners may find offensive."},
	{6, 20,  20,   0,   0,  42,  0,  0, "If you have a problem, feel free to"},
	{7, 20,  20,   0,   0,  42,  0,  0, "barf into the nearest rubbish bin."},
	{8, 20,  20,   0,   0,  42,  0,  0, "Or complain to your mum."},
	{10, 20,  20,   0,   0,  42,  0,  0, "I don't give a shit!"},

	{68, 20,  20,   0,   0,  42,  0,  0, "noise's corpse"},
	{69, 20,  20,   0,   0,  42,  0,  0, "an intro for flashback 2012"},

	{71, 20,  20,   0,   0,  42,  0,  0, "code/gfx/design:"},
	{72, 20,  20,   0,   0,  42,  0,  0, "mudlord"},

	{74, 20,  20,   0,   0,  42,  0,  0, "music:"},
	{75, 20,  20,   0,   0,  42,  0,  0, "kulor"},

	{76, 20,  20,   0,   0,  42,  0,  0, "greetz:"},
	{77, 20,  20,   0,   0,  42,  0,  0, "Zavie"},
	{78, 20,  20,   0,   0,  42,  0,  0, "SunSpire"},
	{79, 20,  20,   0,   0,  42,  0,  0, "Gargaj"},
	{80, 20,  20,   0,   0,  42,  0,  0, "rez"},
	{81, 20,  20,   0,   0,  42,  0,  0, "people in #vba-m"},
	{82, 20,  20,   0,   0,  42,  0,  0, "people in #zsnes"},

	{85, 20,  20,   0,   0,  42,  0,  0, "Hopefully now noise is dead."},
	{86, 20,  20,   0,   0,  42,  0,  0, "Just like the nyancat.."},
	{87, 20,  20,   0,   0,  42,  0,  0, "And the dodo."},
	{88, 20,  20,   0,   0,  42,  0,  0, "And hopefully the shitty Internet filter."},
	{89, 20,  20,   0,   0,  42,  0,  0, "Anyway, I am done, bai! ^w^"}

	
};

const int numtext = sizeof(demotext)/sizeof(text);