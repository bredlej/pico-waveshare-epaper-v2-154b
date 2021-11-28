#include <EPaperDevice.h>

int main()
{
	using namespace EPaperDevice;
	const Device<Devices::WaveshareE154V2b> device = {7, 4, 6, 5, 8, 9};
	return 0;
}