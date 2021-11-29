#include <EPaperDevice.h>

int main()
{
	using namespace EPaperDevice;
	using namespace Devices;

	stdio_init_all();
	Device<Waveshare154V2b> device{{7, 4, 6, 5, 8, 9}};
	
	return 0;
}	