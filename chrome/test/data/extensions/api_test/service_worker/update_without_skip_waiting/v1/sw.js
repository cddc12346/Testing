// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

this.onmessage = function(e) {
  e.ports[0].postMessage('Pong from version 1');
};