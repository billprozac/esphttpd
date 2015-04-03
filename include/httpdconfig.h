
//Define this if you want to be able to use Heatshrink-compressed espfs images.
#define EFS_HEATSHRINK

//Pos of esp fs in flash
#ifndef OTA
#define ESPFS_PART 1
#else
#define ESPFS_PART 4
#define ESPFS_PART2 1
#endif

//If you want, you can define a realm for the authentication system.
//#define HTTP_AUTH_REALM "MyRealm"
