/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "ledblink.h"

#include "mdns.h"
#include "mdnsservices.h"

#include "display.h"
#include "displayhandler.h"

#include "oscserver.h"
#include "oscserverparams.h"
#include "oscservermsgconst.h"

#include "pixeldmxconfiguration.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"
#include "ws28xxdmxstartstop.h"
#include "handler.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "flashrom.h"
#include "spiflashstore.h"
#include "storenetwork.h"
#include "storeoscserver.h"
#include "storeremoteconfig.h"
#include "storews28xxdmx.h"

#include "firmwareversion.h"
#include "software_version.h"

void main(void) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	Display display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashRom flashRom;
	SpiFlashStore spiFlashStore;

	fw.Print("OSC Server " "\x1b[32m" "Pixel controller {1x Universe}" "\x1b[37m");
	
	hw.SetLed(hardware::LedStatus::ON);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	StoreOscServer storeOscServer;
	OSCServerParams params(&storeOscServer);
	
	OscServer server;

	if (params.Load()) {
		params.Dump();
		params.Set(&server);
	}

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	StoreNetwork storeNetwork;
	nw.SetNetworkStore(&storeNetwork);
	nw.Init(&storeNetwork);
	nw.Print();

	display.TextStatus(NetworkConst::MSG_MDNS_CONFIG, Display7SegmentMessage::INFO_MDNS_CONFIG, CONSOLE_YELLOW);
	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_OSC, server.GetPortIncoming(), mdns::Protocol::UDP, "type=server");
#if defined (ENABLE_HTTPD)
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=OSC Server");
#endif
	mDns.Print();

	display.TextStatus(OscServerMsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	PixelDmxConfiguration pixelDmxConfiguration;

	StoreWS28xxDmx storeWS28xxDmx;
	WS28xxDmxParams ws28xxparms(&storeWS28xxDmx);

	if (ws28xxparms.Load()) {
		ws28xxparms.Set(&pixelDmxConfiguration);
		ws28xxparms.Dump();
	}

	/*
	 * DMX Footprint = (Channels per Pixel * Groups) <= 512 (1 Universe)
	 * Groups = Led count / Grouping count
	 *
	 * Channels per Pixel * (Led count / Grouping count) <= 512
	 * Channels per Pixel * Led count <= 512 * Grouping count
	 *
	 * Led count <= (512 * Grouping count) / Channels per Pixel
	 */

	uint32_t nLedsPerPixel;
	pixeldmxconfiguration::PortInfo portInfo;
	uint32_t nGroups;
	uint32_t nUniverses;

	pixelDmxConfiguration.Validate(1 , nLedsPerPixel, portInfo, nGroups, nUniverses);
	pixelDmxConfiguration.Dump();

	if (nUniverses > 1) {
		const auto nCount = (512U * pixelDmxConfiguration.GetGroupingCount()) / nLedsPerPixel;
		DEBUG_PRINTF("nCount=%u", nCount);
		pixelDmxConfiguration.SetCount(nCount);
	}

	WS28xxDmx pixelDmx(pixelDmxConfiguration);
	pixelDmx.SetWS28xxDmxStore(&storeWS28xxDmx);

	const auto nTestPattern = static_cast<pixelpatterns::Pattern>(ws28xxparms.GetTestPattern());
	PixelTestPattern pixelTestPattern(nTestPattern, 1);

	PixelDmxStartStop pixelDmxStartStop;
	pixelDmx.SetPixelDmxHandler(&pixelDmxStartStop);

	display.Printf(7, "%s:%d G%d", PixelType::GetType(pixelDmxConfiguration.GetType()), pixelDmxConfiguration.GetCount(), pixelDmxConfiguration.GetGroupingCount());

	server.SetOutput(&pixelDmx);
	server.SetOscServerHandler(new Handler(&pixelDmx));
	
	server.Print();
	pixelDmx.Print();

	for (auto i = 1; i < 7 ; i++) {
		display.ClearLine(i);
	}

	uint8_t nHwTextLength;

	display.Printf(1, "OSC Pixel 1");
	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "In: %d", server.GetPortIncoming());
	display.Printf(5, "Out: %d", server.GetPortOutgoing());

	RemoteConfig remoteConfig(remoteconfig::Node::OSC,  remoteconfig::Output::PIXEL, 1);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.TextStatus(OscServerMsgConst::START, Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	server.Start();

	display.TextStatus(OscServerMsgConst::STARTED, Display7SegmentMessage::INFO_BRIDGE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		server.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
		if (__builtin_expect((PixelTestPattern::GetPattern() != pixelpatterns::Pattern::NONE), 0)) {
			pixelTestPattern.Run();
		}
		mDns.Run();
	}
}
