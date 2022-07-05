// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (C) 2022 icelimetea, <fr3shtea@outlook.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Linking this library statically or dynamically with other modules is
 *  making a combined work based on this library. Thus, the terms and
 *  conditions of the GNU General Public License cover the whole
 *  combination.
 *
 *  As a special exception, the copyright holders of this library give
 *  you permission to link this library with independent modules to
 *  produce an executable, regardless of the license terms of these
 *  independent modules, and to copy and distribute the resulting
 *  executable under terms of your choice, provided that you also meet,
 *  for each linked independent module, the terms and conditions of the
 *  license of that module. An independent module is a module which is
 *  not derived from or based on this library. If you modify this
 *  library, you may extend this exception to your version of the
 *  library, but you are not obliged to do so. If you do not wish to do
 *  so, delete this exception statement from your version.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

package org.polymc;

import org.polymc.exception.ParseException;
import org.polymc.utils.Parameters;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.logging.Level;
import java.util.logging.Logger;

public final class EntryPoint {

    private static final Logger LOGGER = Logger.getLogger("EntryPoint");

    private final Parameters params = new Parameters();

    public static void main(String[] args) {
        EntryPoint listener = new EntryPoint();

        int retCode = listener.listen();

        if (retCode != 0) {
            LOGGER.info("Exiting with " + retCode);

            System.exit(retCode);
        }
    }

    private Action parseLine(String inData) throws ParseException {
        String[] tokens = inData.split("\\s+", 2);

        if (tokens.length == 0)
            throw new ParseException("Unexpected empty string!");

        switch (tokens[0]) {
            case "launch": {
                return Action.Launch;
            }

            case "abort": {
                return Action.Abort;
            }

            default: {
                if (tokens.length != 2)
                    throw new ParseException("Error while parsing:" + inData);

                params.add(tokens[0], tokens[1]);

                return Action.Proceed;
            }
        }
    }

    public int listen() {
        Action action = Action.Proceed;

        try (BufferedReader reader = new BufferedReader(new InputStreamReader(
                System.in,
                StandardCharsets.UTF_8
        ))) {
            String line;

            while (action == Action.Proceed) {
                if ((line = reader.readLine()) != null) {
                    action = parseLine(line);
                } else {
                    action = Action.Abort;
                }
            }
        } catch (IOException | ParseException e) {
            LOGGER.log(Level.SEVERE, "Launcher ABORT due to exception:", e);

            return 1;
        }

        // Main loop
        if (action == Action.Abort) {
            LOGGER.info("Launch aborted by the launcher.");

            return 1;
        }

        try {
            Launcher launcher =
                    LauncherFactory
                            .getInstance()
                            .createLauncher(params);

            launcher.launch();

            return 0;
        } catch (IllegalArgumentException e) {
            LOGGER.log(Level.SEVERE, "Wrong argument.", e);

            return 1;
        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Exception caught from launcher.", e);

            return 1;
        }
    }

    private enum Action {
        Proceed,
        Launch,
        Abort
    }

}
