/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdint>

#include "hardware.h"
#include "network.h"
#include "networkconst.h"

#include "net/apps/mdns.h"

#if defined (ENABLE_NTP_CLIENT)
# include "net/apps/ntpclient.h"
#endif

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"

#include "artnetnode.h"
#include "artnetparams.h"
#include "artnetmsgconst.h"

#include "rdmdeviceresponder.h"
#include "factorydefaults.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"

#include "artnetrdmresponder.h"

#include "pca9685dmxparams.h"
#include "pca9685dmx.h"

#if defined (NODE_SHOWFILE)
# include "showfile.h"
# include "showfileparams.h"
#endif

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "configstore.h"

#include "firmwareversion.h"
#include "software_version.h"

namespace artnetnode {
namespace configstore {
uint32_t DMXPORT_OFFSET = 0;
}  // namespace configstore
}  // namespace artnetnode

void Hardware::RebootHandler() {
	ArtNetNode::Get()->Stop();
}

int main() {
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, CONSOLE_YELLOW);
	Network nw;
	MDNS mDns;
	display.TextStatus(NetworkConst::MSG_NETWORK_STARTED, CONSOLE_GREEN);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	fw.Print("Art-Net 4 PCA9685");

#if defined (ENABLE_NTP_CLIENT)
	NtpClient ntpClient;
	ntpClient.Start();
	ntpClient.Print();
#endif

	PCA9685DmxParams pca9685DmxParams;
	PCA9685Dmx pca9685Dmx;

	pca9685DmxParams.Load();
	pca9685DmxParams.Set(&pca9685Dmx);

	char aDescription[64];
	snprintf(aDescription, sizeof(aDescription) - 1, "PCA9685");

	ArtNetNode node;

	node.SetLongName(aDescription);

	ArtNetParams artnetParams;
	artnetParams.Load();
	artnetParams.Set();

	node.SetRdm(static_cast<uint32_t>(0), true);
	node.SetOutput(pca9685Dmx.GetLightSet());
	node.SetUniverse(0, lightset::PortDir::OUTPUT, artnetParams.GetUniverse(0));

	RDMPersonality *pRDMPersonalities[1] = { new  RDMPersonality(aDescription, pca9685Dmx.GetLightSet())};

	ArtNetRdmResponder rdmResponder(pRDMPersonalities, 1);

	rdmResponder.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	rdmResponder.SetProductDetail(E120_PRODUCT_DETAIL_LED);

	RDMSensorsParams rdmSensorsParams;

	rdmSensorsParams.Load();
	rdmSensorsParams.Set();

	rdmResponder.Init();

	RDMDeviceParams rdmDeviceParams;

	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&rdmResponder);

	rdmResponder.Print();

	node.SetRdmResponder(&rdmResponder);

#if defined (NODE_SHOWFILE)
	ShowFile showFile;

	ShowFileParams showFileParams;
	showFileParams.Load();
	showFileParams.Set();

	showFile.Print();
#endif

	node.Print();
	pca9685Dmx.Print();

	display.SetTitle("Art-Net 4 PCA9685");
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::VERSION);
	display.Set(4, displayudf::Labels::UNIVERSE_PORT_A);
	display.Set(5, displayudf::Labels::DMX_START_ADDRESS);

	DisplayUdfParams displayUdfParams;

	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show();

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, remoteconfig::Output::PWM, node.GetActiveOutputPorts());

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	while (configStore.Flash())
		;

	mDns.Print();

	display.TextStatus(ArtNetMsgConst::START, CONSOLE_YELLOW);

	node.Start();

	display.TextStatus(ArtNetMsgConst::STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.Run();
#if defined (NODE_SHOWFILE)
		showFile.Run();
#endif
		remoteConfig.Run();
		configStore.Flash();
		mDns.Run();
#if defined (ENABLE_NTP_CLIENT)
		ntpClient.Run();
#endif
		display.Run();
		hw.Run();
	}
}
