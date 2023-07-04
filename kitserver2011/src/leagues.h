// camera.h

#define MODID 180
#define NAMELONG L"LEAGUES Module 11.0.0.0"
#define NAMESHORT L"LEAGUES"
#define DEFAULT_DEBUG 0

// LEAGUE Structure
typedef struct _league
{
public:
	DWORD relink;
	char* leaguename;
	char* leaguelogo;
} _league;