#!/bin/bash

CXX="$(command -v arm-none-eabi-g++)"
if [[ -z "$CXX" ]]; then
  echo "ERROR: arm-none-eabi-g++ not found in PATH" >&2
  exit 1
fi

get_cxx_system_includes() {
  local cxx="$1"
  "$cxx" -E -x c++ - -v </dev/null 2>&1 \
    | awk '
        $0 ~ /^#include <\.\.\.> search starts here:/ { inside=1; next }
        $0 ~ /^End of search list\./ { inside=0 }
        inside {
          gsub(/^[[:space:]]+|[[:space:]]+$/, "", $0)
          if ($0 != "" && $0 !~ /^\(framework directory\)$/)
            print "-isystem " $0
        }
      ' \
    | tr '\n' ' '
}

SYSINC="$(get_cxx_system_includes "$CXX")"

rm -rf .cache

PROJECT_DIR=$(pwd)

INCLUDE_PATHS=$(find . -name '*.h' \
  -not -path '*ven*' -not -path '*.git*' -not -path '*/build/*' \
  -exec dirname {} \; | sort -u | awk '{
    keep = 1
    for (i in seen) {
     if (index($0, seen[i]) == 1 && length($0) > length(seen[i]) && substr($0, length(seen[i])+1, 1) == "/") {
        keep = 0
        break
      }
      if (index(seen[i], $0) == 1 && length(seen[i]) > length($0) && substr(seen[i], length($0)+1, 1) == "/") {
        delete seen[i]
      }
    }
    if (keep) seen[length(seen)+1]=$0
  } END {
    for (i in seen) print "-I" seen[i]
  }' | tr '\n' ' ')

INCLUDE_PATHS="-I./common/include $INCLUDE_PATHS "
# Add paths to freestanding headers for toolchain
INCLUDE_PATHS="$INCLUDE_PATHS $SYSINC"

DEFINES="-DGD32 -DGD32F10X -DGD32F10X_CL -DGD32F107RC -DBOARD_GD32F107RC -DCONFIG_CLIB_USE_UART0 -DCONFIG_DEBUG_TRACE -DCONFIG_EMAC_HASH_MULTICAST_FILTER -DCONFIG_HTTPD_OPTIMIZE_NONE -DCONFIG_NET_ENABLE_NTP_CLIENT -DCONFIG_STORE_USE_ROM -DCONFIG_TIMER6_HAVE_NO_IRQ_HANDLER -DCONFIG_TIME_USE_TIMER -DDEBUG_BOARD -DDEBUG_GD32_TIMERS -DDISABLE_FS -DDISPLAY_UDF -DENABLE_HTTPD -DENABLE_SSD1311 -DENET_LINK_CHECK_REG_POLL -DHAVE_TIMEOFDAY -DMCU_HAVE_CAU -DMCU_HAVE_HAU -DMCU_HAVE_TRNG -DNDEBUG -DNODE_RDMNET_LLRP_ONLY -DOUTPUT_DMX_NULL -DPHY_TYPE=DP83848 -D_TIME_STAMP_=1788618910 -D_TIME_STAMP_DAY_=5 -D_TIME_STAMP_MONTH_=9 -D_TIME_STAMP_YEAR_=2026"

echo $INCLUDE_PATHS
echo $DEFINES

# Start the JSON array
echo "[" > compile_commands.json

# Find all .cpp files and create a JSON entry for each
find .  -name '*.cpp' | while read file; do
  echo "  {" >> compile_commands.json
  echo "    \"directory\": \"${PROJECT_DIR}\"," >> compile_commands.json
  echo "    \"command\": \"$CXX -std=c++20 -nostartfiles -ffreestanding -nostdlib -nostdinc++ $INCLUDE_PATHS $DEFINES $file\"," >> compile_commands.json
  echo "    \"file\": \"$file\"" >> compile_commands.json
  echo "  }," >> compile_commands.json
done

# Remove last comma and close JSON array
sed -i '' -e '$ s/},/}/' compile_commands.json
echo "]" >> compile_commands.json

jq . compile_commands.json > tmp.json && mv tmp.json compile_commands.json
