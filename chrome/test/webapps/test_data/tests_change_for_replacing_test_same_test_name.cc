// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

IN_PROC_BROWSER_TEST_F(TestName, WAI_StateChangeAChicken_StateChangeADog) {
  // Test contents are generated by script. Please do not modify!
  // See `docs/webapps/why-is-this-test-failing.md` or
  // `docs/webapps/integration-testing-framework` for more info.
  // Gardeners: Disabling this test is supported.
  helper_.StateChangeA(Animal::kChicken);
  helper_.StateChangeA(Animal::kDog);
  helper_.CheckA(Animal::kDog);
}

IN_PROC_BROWSER_TEST_F(
    TestName,
    WAI_StateChangeAChicken_StateChangeADog_StateChangeAChicken) {
  // Test contents are generated by script. Please do not modify!
  // See `docs/webapps/why-is-this-test-failing.md` or
  // `docs/webapps/integration-testing-framework` for more info.
  // Gardeners: Disabling this test is supported.
  helper_.StateChangeA(Animal::kChicken);
  helper_.StateChangeA(Animal::kDog);
  helper_.CheckA(Animal::kDog);
  helper_.StateChangeA(Animal::kChicken);
}