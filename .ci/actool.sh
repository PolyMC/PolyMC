#!/bin/sh -e

# SPDX-FileCopyrightText: Copyright 2026 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

out=program_info
_svg=org.polymc.PolyMC.Source.svg
_icon="$out"/polymc.icon
app_icon=polymc
_composed="$_icon/Assets/$_svg"
_svg="$out/$_svg"

rm -f "$_composed"
cp "$_svg" "$_composed"

xcrun actool "$_icon" \
    --compile "$out" \
    --platform macosx \
    --minimum-deployment-target 10.15 \
    --app-icon "$app_icon" \
    --output-partial-info-plist /dev/null
