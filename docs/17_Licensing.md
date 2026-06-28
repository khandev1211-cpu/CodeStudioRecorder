# 17 — Licensing

## CodeStudio Recorder — Licensing & Attribution

---

## Overview

This document covers the licensing of CodeStudio Recorder itself and all third-party libraries it includes.

---

## CodeStudio Recorder License

CodeStudio Recorder core is released under the **MIT License**.

```
MIT License

Copyright (c) 2025 CodeStudio Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
```

---

## Third-Party Libraries

| Library | License | Link |
|---|---|---|
| FFmpeg | LGPL 2.1 | https://ffmpeg.org/legal.html |
| SQLite | Public Domain | https://sqlite.org/copyright.html |
| ONNX Runtime | MIT | https://github.com/microsoft/onnxruntime |
| RNNoise | BSD 3-Clause | https://gitlab.xiph.org/xiph/rnnoise |
| Google Test | BSD 3-Clause | https://github.com/google/googletest |
| Flutter | BSD 3-Clause | https://github.com/flutter/flutter |
| Riverpod | MIT | https://github.com/rrousselGit/riverpod |
| go_router | BSD 3-Clause | https://pub.dev/packages/go_router |
| freezed | MIT | https://pub.dev/packages/freezed |
| JetBrains Mono | OFL 1.1 | https://www.jetbrains.com/lp/mono/ |

### LGPL Compliance (FFmpeg)

FFmpeg is linked as a shared library (DLL), not statically. Users can replace the FFmpeg DLLs with their own builds. This satisfies the LGPL requirement of allowing relinking.

The distributed FFmpeg build:
- Does NOT include GPL-licensed components (no libx264 in distribution build — software encoding uses Windows Media Foundation or a separately licensed encoder)
- Is built from source with only LGPL components
- Source code for the FFmpeg build is available at: `[link to FFmpeg build repo]`

---

## Attribution Requirements

All distributions of CodeStudio Recorder must include:
- The MIT license text (included in installer and app About dialog)
- FFmpeg attribution: "This software uses libraries from the FFmpeg project under the LGPLv2.1"
- SQLite attribution: "This product includes software developed by D. Richard Hipp"

---

## Plugin Licensing

Plugins distributed through the Plugin Marketplace must:
- Declare their license in `plugin.json`
- Include their license text in the plugin package
- Not include GPL-licensed code unless the entire plugin is GPL

Plugins are not covered by CodeStudio Recorder's MIT license.

---

*Last updated: 2025 | Module 17 of 19*
