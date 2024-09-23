/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "e131.h"
#include "e131bridge.h"
#include "e131params.h"
#include "e131msgconst.h"

#include "dmxparams.h"
#include "dmxsend.h"
#include "dmxconfigudp.h"

#if defined (NODE_RDMNET_LLRP_ONLY)
# include "rdmdeviceparams.h"
# include "rdmnetdevice.h"
# include "rdmnetconst.h"
# include "rdmpersonality.h"
# include "rdm_e120.h"
#endif

#if defined (NODE_SHOWFILE)
# include "showfile.h"
# include "showfileparams.h"
#endif

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "configstore.h"

#include "firmwareversion.h"
#include "software_version.h"

namespace e131bridge {
namespace configstore {
uint32_t DMXPORT_OFFSET = 0;
}  // namespace configstore
}  // namespace e131bridge

void Hardware::RebootHandler() {
	Dmx::Get()->Blackout();
	E131Bridge::Get()->Stop();
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

	fw.Print("sACN E1.31 DMX {1 Universe}");

#if defined (ENABLE_NTP_CLIENT)
	NtpClient ntpClient;
	ntpClient.Start();
	ntpClient.Print();
#endif

	E131Bridge bridge;

	E131Params e131params;
	e131params.Load();
	e131params.Set();

	bool bIsSet;
	bridge.SetUniverse(0, e131params.GetDirection(0), e131params.GetUniverse(0, bIsSet));

	Dmx dmx;

	DmxParams dmxparams;
	dmxparams.Load();
	dmxparams.Set(&dmx);

	uint16_t nUniverse;

	if (bridge.GetUniverse(0, nUniverse, lightset::PortDir::OUTPUT)) {
		dmx.SetPortDirection(0, dmx::PortDirection::OUTP, false);
	} else {
		dmx.SetPortDirection(0, dmx::PortDirection::INP, false);
	}

	DmxSend dmxSend;
	dmxSend.Print();

	bridge.SetOutput(&dmxSend);

	const auto nActivePorts = static_cast<uint32_t>(bridge.GetActiveInputPorts() + bridge.GetActiveOutputPorts());

#if defined (NODE_RDMNET_LLRP_ONLY)
	display.TextStatus(RDMNetConst::MSG_CONFIG, CONSOLE_YELLOW);
	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "sACN DMX %d", nActivePorts);

	char aLabel[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
	const auto nLength = snprintf(aLabel, sizeof(aLabel) - 1, GD32_BOARD_NAME " DMX");

	RDMPersonality *pPersonalities[1] = { new RDMPersonality(aDescription, nullptr) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, static_cast<uint8_t>(nLength));
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	llrpOnlyDevice.Init();

	RDMDeviceParams rdmDeviceParams;

	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&llrpOnlyDevice);

	llrpOnlyDevice.Print();
#endif

#if defined (NODE_SHOWFILE)
	ShowFile showFile;

	ShowFileParams showFileParams;
	showFileParams.Load();
	showFileParams.Set();

	showFile.Print();
#endif

	bridge.Print();

	display.SetTitle("sACN E1.31 DMX %s", e131params.GetDirection(0) == lightset::PortDir::INPUT ? "Input" : "Output");
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::HOSTNAME);
	display.Set(4, displayudf::Labels::VERSION);
	display.Set(5, displayudf::Labels::UNIVERSE_PORT_A);
	display.Set(6, displayudf::Labels::BOARDNAME);

	DisplayUdfParams displayUdfParams;

	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show();

	RemoteConfig remoteConfig(remoteconfig::Node::E131, remoteconfig::Output::DMX, nActivePorts);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	while (configStore.Flash())
		;

	mDns.Print();

	display.TextStatus(E131MsgConst::START, CONSOLE_YELLOW);

	bridge.Start();

	display.TextStatus(E131MsgConst::STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		bridge.Run();
#if defined (NODE_SHOWFILE)
		showFile.Run();
#endif
		remoteConfig.Run();
		configStore.Flash();
		mDns.Run();
#if defined (ENABLE_NTP_CLIENT)
		ntpClient.Run();
#endif
#if defined (NODE_RDMNET_LLRP_ONLY)
		llrpOnlyDevice.Run();
#endif
		display.Run();
		hw.Run();
	}
}
