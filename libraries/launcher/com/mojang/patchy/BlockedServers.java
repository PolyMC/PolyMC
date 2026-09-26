package com.mojang.patchy;

import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.Collection;
import java.util.function.Predicate;

public class BlockedServers implements Predicate<String> {

    public static final Charset HASH_CHARSET = StandardCharsets.ISO_8859_1;
    public BlockedServers(Collection<String> blockedServers) {
    }

    @Override
    public boolean test(String server) {
        return false;
    }
}
