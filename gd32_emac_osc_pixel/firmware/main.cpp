/**
 * @file main.cpp
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "gd32/hal.h"
#include "gd32/hal_watchdog.h"
#include "hal_boardinfo.h"
#include "display.h"
#include "emac/network.h"
#include "oscserver.h"
#include "json/oscserverparams.h"
#include "oscservermsgconst.h"
#include "firmware/pixeldmx/show.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "pixeldmx.h"
#include "json/pixeldmxparams.h"
#include "handler.h"
#include "remoteconfig.h"
#include "configstore.h"
#include "firmwareversion.h"
#include "software_version.h"
#include "common/utils/utils_flags.h"
#include "configurationstore.h"

namespace hal
{
void RebootHandler()
{
    PixelDmx::Get().Blackout();
}
} // namespace hal

int main() // NOLINT
{
    hal::Init();
    Display display;
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

    fw.Print("OSC Server Pixel controller {1x Universe}");

    OscServer oscserver;

    json::OscServerParams oscserver_params;
    oscserver_params.Load();
    oscserver_params.Set();

    display.TextStatus(OscServerMsgConst::PARAMS, console::Colours::kConsoleYellow);

    PixelDmx pixeldmx;

    json::PixelDmxParams pixeldmx_params;
    pixeldmx_params.Load();
    pixeldmx_params.Set();

    const auto kTestPattern = common::FromValue<pixelpatterns::Pattern>(ConfigStore::Instance().DmxLedGet(&common::store::DmxLed::test_pattern));

    PixelTestPattern pixeltest_pattern(kTestPattern, 1);

    common::firmware::pixeldmx::Show(7);

    oscserver.SetOutput(&pixeldmx);
    oscserver.SetOscServerHandler(new Handler(&pixeldmx));

    oscserver.Print();
    pixeldmx.Print();

    uint8_t text_length;

    display.Printf(1, "OSC Pixel 1");
    display.Write(2, hal::BoardName(text_length));
    display.Printf(3, "IP: " IPSTR " %c", IP2STR(network::GetPrimaryIp()), network::iface::IsDhcpKnown() ? (network::iface::Dhcp() ? 'D' : 'S') : ' ');
    display.Printf(4, "In: %d", oscserver.GetPortIncoming());
    display.Printf(5, "Out: %d", oscserver.GetPortOutgoing());

    RemoteConfig remote_config(remoteconfig::Output::PIXEL, 1);

    display.TextStatus(OscServerMsgConst::START, console::Colours::kConsoleYellow);

    oscserver.Start();

    display.TextStatus(OscServerMsgConst::STARTED, console::Colours::kConsoleGreen);

    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        network::Run();
        pixeltest_pattern.Run();
        hal::Run();
    }
}
