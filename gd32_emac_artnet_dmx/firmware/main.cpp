/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "hardware.h"
#include "network.h"
#include "networkconst.h"

#include "mdns.h"

#if defined (ENABLE_HTTPD)
# include "httpd/httpd.h"
#endif

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"
#include "display_timeout.h"

#include "artnetnode.h"
#include "artnetparams.h"
#include "artnetmsgconst.h"
#include "artnetrdmcontroller.h"

#include "dmxparams.h"
#include "dmxsend.h"
#include "rdmdeviceparams.h"
#include "dmxconfigudp.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "configstore.h"
#include "storeartnet.h"
#include "storedisplayudf.h"
#include "storedmxsend.h"
#include "storenetwork.h"
#include "storerdmdevice.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

static constexpr uint32_t DMXPORT_OFFSET = 0;

void Hardware::RebootHandler() {
	Dmx::Get()->Blackout();
	ArtNetNode::Get()->Stop();
}

void main() {
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);
	StoreNetwork storeNetwork;
	Network nw(&storeNetwork);
	display.TextStatus(NetworkConst::MSG_NETWORK_STARTED, Display7SegmentMessage::INFO_NONE, CONSOLE_GREEN);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	fw.Print("Art-Net 4 DMX/RDM controller {1 Universe}");
	nw.Print();

	display.TextStatus(NetworkConst::MSG_MDNS_CONFIG, Display7SegmentMessage::INFO_MDNS_CONFIG, CONSOLE_YELLOW);

	MDNS mDns;
	mDns.AddServiceRecord(nullptr, mdns::Services::CONFIG, "node=Art-Net 4 DMX/RDM");
#if defined (ENABLE_HTTPD)
	mDns.AddServiceRecord(nullptr, mdns::Services::HTTP);
#endif
	mDns.Print();

#if defined (ENABLE_HTTPD)
	HttpDaemon httpDaemon;
#endif

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	ArtNetNode node;

	StoreArtNet storeArtNet(DMXPORT_OFFSET);
	node.SetArtNetStore(&storeArtNet);

	ArtNetParams artnetParams(&storeArtNet);

	if (artnetParams.Load()) {
		artnetParams.Dump();
		artnetParams.Set(DMXPORT_OFFSET);
	}

	node.SetUniverse(0, artnetParams.GetDirection(0), artnetParams.GetUniverse(0));

	StoreDmxSend storeDmxSend;
	DmxParams dmxparams(&storeDmxSend);

	Dmx dmx;

	if (dmxparams.Load()) {
		dmxparams.Dump();
		dmxparams.Set(&dmx);
	}

	const auto portDirection = (node.GetPortDirection(0) == lightset::PortDir::OUTPUT ? dmx::PortDirection::OUTP : dmx::PortDirection::INP);
	dmx.SetPortDirection(0, portDirection , false);

	DmxSend dmxSend;
	dmxSend.Print();

	node.SetOutput(&dmxSend);

	DmxConfigUdp dmxConfigUdp;

	StoreRDMDevice storeRdmDevice;

	ArtNetRdmController artNetRdmController;

	if (artnetParams.IsRdm()) {
		RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

		if (rdmDeviceParams.Load()) {
			rdmDeviceParams.Set(&artNetRdmController);
			rdmDeviceParams.Dump();
		}

		artNetRdmController.Init();
		artNetRdmController.Print();

		if (node.GetRdm(0) && (node.GetPortDirection(0) == lightset::PortDir::OUTPUT)) {
			display.TextStatus(ArtNetMsgConst::RDM_RUN, Display7SegmentMessage::INFO_RDM_RUN, CONSOLE_YELLOW);
			artNetRdmController.Full(0);
		}

		node.SetRdmHandler(&artNetRdmController);
	}

	node.Print();

	const auto nActivePorts = static_cast<uint32_t>(node.GetActiveInputPorts() + node.GetActiveOutputPorts());

	display.SetTitle("Art-Net 4 %s", artnetParams.GetDirection(0) == lightset::PortDir::INPUT ? "DMX Input" : (artnetParams.IsRdm() ? "RDM" : "DMX Output"));
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::VERSION);
	display.Set(4, displayudf::Labels::UNIVERSE_PORT_A);
	display.Set(5, displayudf::Labels::HOSTNAME);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Dump();
		displayUdfParams.Set(&display);
	}

	display.Show(&node);

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, artnetParams.IsRdm() ? remoteconfig::Output::RDM : remoteconfig::Output::DMX, nActivePorts);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Dump();
		remoteConfigParams.Set(&remoteConfig);
	}

	while (configStore.Flash())
		;

	display.TextStatus(ArtNetMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

	node.Start();

	display.TextStatus(ArtNetMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.Run();
		remoteConfig.Run();
		configStore.Flash();
		if (node.GetActiveOutputPorts() != 0) {
			dmxConfigUdp.Run();
		}
		mDns.Run();
#if defined (ENABLE_HTTPD)
		httpDaemon.Run();
#endif
		display.Run();
		hw.Run();
	}
}
