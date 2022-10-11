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
#if defined (ENABLE_HTTPD)
# include "httpd/httpd.h"
#endif

#include "display.h"
#include "displayhandler.h"

#include "oscserver.h"
#include "oscserverparams.h"
#include "oscservermsgconst.h"

#include "dmxparams.h"
#include "dmxsend.h"
#include "dmxconfigudp.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "flashrom.h"
#include "spiflashstore.h"
#include "storedmxsend.h"
#include "storenetwork.h"
#include "storeoscserver.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

void Hardware::RebootHandler() {
	Dmx::Get()->Blackout();
}

void main() {
	Hardware hw;
	Network nw;
	LedBlink lb;
	Display display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashRom flashRom;
	SpiFlashStore spiFlashStore;

	fw.Print("\x1b[32m" "OSC Server DMX controller {1x Universe}" "\x1b[37m");
	
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
#if defined (ENABLE_HTTPD)
	HttpDaemon httpDaemon;
	httpDaemon.Start();
#endif

	display.TextStatus(OscServerMsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	StoreDmxSend storeDmxSend;
	DmxParams dmxparams(&storeDmxSend);

	Dmx dmx;

	if (dmxparams.Load()) {
		dmxparams.Dump();
		dmxparams.Set(&dmx);
	}

	DmxSend dmxSend;

	dmxSend.Print();

	DmxConfigUdp dmxConfigUdp;

	server.SetOutput(&dmxSend);
	server.Print();

	dmxSend.Print();

	for (uint8_t i = 1; i < 7 ; i++) {
		display.ClearLine(i);
	}

	uint8_t nHwTextLength;

	display.Printf(1, "OSC DMX 1");
	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "In: %d", server.GetPortIncoming());
	display.Printf(5, "Out: %d", server.GetPortOutgoing());

	RemoteConfig remoteConfig(remoteconfig::Node::OSC, remoteconfig::Output::DMX, 1);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Dump();
		remoteConfigParams.Set(&remoteConfig);
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
		dmxConfigUdp.Run();
		mDns.Run();
#if defined (ENABLE_HTTPD)
		httpDaemon.Run();
#endif
		display.Run();
		lb.Run();
	}
}
