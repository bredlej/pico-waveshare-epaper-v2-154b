#include <EPaperDevice.hpp>

int main()
{
	using namespace EPaperDevice;
	using namespace Devices;
	
	Device<Waveshare154V2b> device{{7, 4, 6, 5, 8, 9}};
	device.run();
	while (true)
	{

	}
	return 0;
}	