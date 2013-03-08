-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

--- Path manipulation utility functions.

path = {}

path.dirname = function (filename)
    while true do
        if filename == "" or string.sub(filename, -1) == "/" then
            break
        end
        filename = string.sub(filename, 1, -2)
    end
    if filename == "" then
        filename = "."
    end

    return filename
end

path.join = function (dirname, basename)
    if string.sub(dirname, -1) ~= "/" then
        dirname = dirname .. "/"
    end
    return dirname .. basename
end
