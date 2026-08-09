# Backport Analysis for HiGarfield/smartdns

## Summary

This fork (HiGarfield/smartdns) has diverged significantly from upstream (pymumu/smartdns) due to a major code restructuring that occurred in upstream on **April 7, 2025** (commit 451b3ed).

## Successfully Backported Fixes (5 commits)

1. **d4bf46f** - fix 'udp_socket_bio' undeclared issue
2. **56592cb** - build: fix build script issue
3. **efc9d51** - build: fix build script issue  
4. **5aac80b** - fix(security): prevent buffer overflow in domain_rule.c
5. **681c271** - fix: allow set and default empty server name

## Structural Divergence

### Current Fork Structure (Pre-refactoring)
```
src/
├── dns_client.c (monolithic, ~133KB)
├── dns_conf.c (monolithic, ~160KB)
├── dns_server.c (monolithic, ~258KB)
├── fast_ping.c (monolithic, ~58KB)
└── http_parse.c (monolithic, ~12KB)
```

### Upstream Structure (Post-refactoring, April 2025)
```
src/
├── dns_client/ (35 files split)
│   ├── client_http2.c
│   ├── client_http3.c
│   ├── client_quic.c
│   ├── dns_client.c
│   └── ... (31 more files)
├── dns_conf/ (multiple files split)
├── dns_server/ (multiple files split)
├── fast_ping/ (20 files split)
└── http_parse/ (multiple files split)
```

## Identified Fix Commits (67 total)

### Backportable Without Refactoring (5 - Already Done)
- See "Successfully Backported Fixes" above

### Cannot Backport Without Refactoring (62 commits)

All remaining 62 fix commits target the refactored directory structure:
- 48 commits touch `src/dns_client/*` files
- 8 commits touch `src/dns_conf/*` files  
- 6 commits touch `src/dns_server/*` files
- 3 commits touch `src/fast_ping/*` files
- 3 commits touch `src/http_parse/*` files

These fixes address:
- HTTP/2 and HTTP/3 protocol issues
- QUIC connection handling
- Memory safety (UAF, buffer overflows)
- Race conditions  
- Stream management
- Cache handling
- Configuration parsing

## Recommendations

### Option 1: Backport the Refactoring First
1. Cherry-pick or merge the refactoring commits (451b3ed and related)
2. Then backport all 62 remaining fixes

**Pros:** Enables backporting all fixes, maintains compatibility with upstream structure
**Cons:** Large change, may introduce new issues, changes project structure

### Option 2: Manual Adaptation
Manually adapt each fix to the monolithic file structure by:
1. Understanding the fix logic
2. Finding equivalent code in monolithic files
3. Applying changes manually
4. Testing thoroughly

**Pros:** Maintains current structure, selective backporting
**Cons:** Very time-consuming (62 commits), error-prone, loses commit correspondence

### Option 3: Merge/Rebase from Upstream
Merge or rebase from upstream to get all fixes automatically

**Pros:** Gets all fixes plus improvements, maintains upstream compatibility  
**Cons:** May include features (not just fixes), requires conflict resolution

### Option 4: Accept Limitation
Continue with the 5 successfully backported fixes and document the limitation

**Pros:** Low effort, maintains stability
**Cons:** Missing 62 important fixes including security and stability improvements

## Conclusion

The structural divergence makes it impractical to backport most upstream fixes without first adopting the refactored code structure. The most effective path forward depends on the project's goals:

- If maintaining compatibility with upstream is important: **Option 1 or 3**
- If maintaining the current structure is critical: **Option 4** with selective **Option 2** for critical fixes
- If resources are limited: **Option 4**
