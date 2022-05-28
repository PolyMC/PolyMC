/*
 * Copyright 2012-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package net.minecraft;

import java.applet.Applet;
import java.applet.AppletStub;
import java.awt.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Map;
import java.util.TreeMap;

public final class Launcher extends Applet implements AppletStub {

    private final Map<String, String> params = new TreeMap<>();

    private final Applet wrappedApplet;

    private boolean active = false;

    public Launcher(Applet applet) {
        this.setLayout(new BorderLayout());

        this.add(applet, "Center");

        this.wrappedApplet = applet;
    }

    public void setParameter(String name, String value)
    {
        params.put(name, value);
    }

    @Override
    public String getParameter(String name) {
        String param = params.get(name);

        if (param != null)
            return param;

        try {
            return super.getParameter(name);
        } catch (Exception ignore) {}

        return null;
    }

    @Override
    public boolean isActive() {
        return active;
    }

    @Override
    public void appletResize(int width, int height) {
        wrappedApplet.resize(width, height);
    }

    @Override
    public void resize(int width, int height) {
        wrappedApplet.resize(width, height);
    }

    @Override
    public void resize(Dimension d) {
        wrappedApplet.resize(d);
    }

    @Override
    public void init() {
        if (wrappedApplet != null)
            wrappedApplet.init();
    }

    @Override
    public void start() {
        wrappedApplet.start();

        active = true;
    }

    @Override
    public void stop() {
        wrappedApplet.stop();

        active = false;
    }

    public void destroy() {
        wrappedApplet.destroy();
    }

    @Override
    public URL getCodeBase() {
        try {
            return new URL("http://www.minecraft.net/game/");
        } catch (MalformedURLException e) {
            e.printStackTrace();
        }

        return null;
    }

    @Override
    public URL getDocumentBase() {
        try {
            // Special case only for Classic versions
            if (wrappedApplet.getClass().getCanonicalName().startsWith("com.mojang"))
                return new URL("http", "www.minecraft.net", 80, "/game/");

            return new URL("http://www.minecraft.net/game/");
        } catch (MalformedURLException e) {
            e.printStackTrace();
        }

        return null;
    }

    @Override
    public void setVisible(boolean b) {
        super.setVisible(b);

        wrappedApplet.setVisible(b);
    }

    public void update(Graphics paramGraphics) {}

    public void paint(Graphics paramGraphics) {}

}
