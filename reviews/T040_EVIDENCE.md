# T-040 evidence - network imports in GrimAmp.vst3 (Windows)

CI run 41231742279 (commit b68e49c), artifact `network-imports` (`imports.txt`, `network-imports.txt`;
a copy of the function list is `T040_network_imports_run41231742279.txt`).

The binary imports these functions from the network DLLs the T-040 gate watches:

- `WININET.dll` (13): `InternetOpenW`, `InternetConnectW`, `HttpOpenRequestW`, `HttpSendRequestExW`,
  `HttpEndRequestW`, `HttpQueryInfoW`, `InternetReadFile`, `InternetWriteFile`, `InternetSetOptionW`,
  `InternetSetFilePointer`, `InternetCrackUrlW`, `InternetCloseHandle`, `FtpOpenFileW`
- `WS2_32.dll` (2): `getaddrinfo`, `freeaddrinfo`

This is the import profile of JUCE's `WebInputStream` (WinInet HTTP/FTP client) plus name
resolution, linked in with `juce_core`. Notably absent: `WSAStartup`, `socket`, `connect`, `send`,
`recv`, `bind`, `listen`, so JUCE's raw `Socket` classes are not retained.

What is established
- GrimAmp's own sources use no `juce::URL`, `Socket` or `StreamingSocket` (the static T-040 gate passes,
  now able to fail).
- `JUCE_WEB_BROWSER=0` and `JUCE_USE_CURL=0` are set.

What is NOT established
- That `WebInputStream` is unreachable at run time. The linker retains it, so some retained code
  references it (most likely through `juce::URL::createInputStream`). Proving it unreachable needs a
  call-graph or map-file analysis on Windows that was not done.

Decision recorded as pending (see `reviews/REVIEW_DISPOSITION.md`): the maintainer chose "prove
unreachable" on 2026-10-31. Until that proof or a different decision exists, the Windows T-040 gate
stays failing; it is not weakened.
