# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Definitions of builders in the chromium.mac builder group."""

load("//lib/args.star", "args")
load("//lib/branches.star", "branches")
load("//lib/builder_config.star", "builder_config")
load("//lib/builder_health_indicators.star", "health_spec")
load("//lib/builders.star", "cpu", "gardener_rotations", "os", "siso")
load("//lib/ci.star", "ci")
load("//lib/consoles.star", "consoles")
load("//lib/gn_args.star", "gn_args")
load("//lib/xcode.star", "xcode")

ci.defaults.set(
    executable = ci.DEFAULT_EXECUTABLE,
    builder_group = "chromium.mac",
    builder_config_settings = builder_config.ci_settings(
        retry_failed_shards = True,
    ),
    pool = ci.DEFAULT_POOL,
    os = os.MAC_DEFAULT,
    gardener_rotations = gardener_rotations.CHROMIUM,
    tree_closing = True,
    main_console_view = "main",
    execution_timeout = ci.DEFAULT_EXECUTION_TIMEOUT,
    health_spec = health_spec.DEFAULT,
    service_account = ci.DEFAULT_SERVICE_ACCOUNT,
    shadow_service_account = ci.DEFAULT_SHADOW_SERVICE_ACCOUNT,
    siso_enabled = True,
    siso_project = siso.project.DEFAULT_TRUSTED,
    siso_remote_jobs = siso.remote_jobs.DEFAULT,
    thin_tester_cores = 8,
)

consoles.console_view(
    name = "chromium.mac",
    branch_selector = [
        branches.selector.IOS_BRANCHES,
        branches.selector.MAC_BRANCHES,
    ],
    ordering = {
        None: ["release"],
        "release": consoles.ordering(short_names = ["bld"]),
        "debug": consoles.ordering(short_names = ["bld"]),
        "ios|default": consoles.ordering(short_names = ["dev", "sim"]),
    },
)

consoles.console_view(
    name = "sheriff.ios",
    title = "iOS Sheriff Console",
    ordering = {
        "*type*": consoles.ordering(short_names = ["dev", "sim"]),
        None: ["chromium.mac", "chromium.fyi"],
        "chromium.mac": "*type*",
        "chromium.fyi|13": "*type*",
    },
)

def ios_builder(*, name, **kwargs):
    kwargs.setdefault("gardener_rotations", gardener_rotations.IOS)
    kwargs.setdefault("xcode", xcode.xcode_default)
    return ci.builder(name = name, **kwargs)

ci.builder(
    name = "Mac Builder",
    branch_selector = branches.selector.MAC_BRANCHES,
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
            apply_configs = [
                # This is necessary due to child builders running the
                # telemetry_perf_unittests suite.
                "chromium_with_telemetry_dependencies",
                "use_clang_coverage",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
        build_gs_bucket = "chromium-mac-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "gpu_tests",
            "release_builder",
            "remoteexec",
            "minimal_symbols",
            "x64",
            "mac",
        ],
    ),
    cpu = cpu.ARM64,
    console_view_entry = consoles.console_view_entry(
        category = "release",
        short_name = "bld",
    ),
    cq_mirrors_console_view = "mirrors",
    contact_team_email = "bling-engprod@google.com",
    siso_project = siso.project.DEFAULT_TRUSTED,
    siso_remote_jobs = siso.remote_jobs.DEFAULT,
)

ci.builder(
    name = "Mac Builder (dbg)",
    branch_selector = branches.selector.MAC_BRANCHES,
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
        build_gs_bucket = "chromium-mac-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "gpu_tests",
            "debug_builder",
            "remoteexec",
            "x64",
            "mac",
        ],
    ),
    os = os.MAC_ANY,
    cpu = cpu.ARM64,
    console_view_entry = consoles.console_view_entry(
        category = "debug",
        short_name = "bld",
    ),
    cq_mirrors_console_view = "mirrors",
    contact_team_email = "bling-engprod@google.com",
)

ci.builder(
    name = "mac-arm64-on-arm64-rel",

    # TODO(crbug.com/40172659): Expand to more branches when all M1 bots are
    # rosettaless.
    # branch_selector = branches.selector.MAC_BRANCHES,
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "release_builder",
            "remoteexec",
            "mac",
            "arm64",
        ],
    ),
    os = os.MAC_DEFAULT,
    cpu = cpu.ARM64,
    console_view_entry = consoles.console_view_entry(
        category = "release|arm64",
        short_name = "a64",
    ),
    contact_team_email = "bling-engprod@google.com",
)

ci.builder(
    name = "mac-arm64-dbg",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "debug_builder",
            "remoteexec",
            "mac",
            "arm64",
        ],
    ),
    builderless = True,
    os = os.MAC_DEFAULT,
    cpu = cpu.ARM64,
    console_view_entry = consoles.console_view_entry(
        category = "debug|arm64",
        short_name = "bld",
    ),
    contact_team_email = "bling-engprod@google.com",
)

ci.builder(
    name = "mac-arm64-rel",
    branch_selector = branches.selector.MAC_BRANCHES,
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
            apply_configs = [
                # This is necessary due to child builders running the
                # telemetry_perf_unittests suite.
                "chromium_with_telemetry_dependencies",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "arm64",
            "gpu_tests",
            "release_builder",
            "remoteexec",
            "minimal_symbols",
            "mac",
        ],
    ),
    os = os.MAC_DEFAULT,
    cpu = cpu.ARM64,
    console_view_entry = consoles.console_view_entry(
        category = "release|arm64",
        short_name = "bld",
    ),
    contact_team_email = "bling-engprod@google.com",
)

ci.builder(
    name = "mac-intel-on-arm64-rel",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "release_builder",
            "remoteexec",
            "mac",
            "x64",
        ],
    ),
    os = os.MAC_DEFAULT,
    cpu = cpu.ARM64,
    gardener_rotations = args.ignore_default(None),
    tree_closing = False,
    console_view_entry = consoles.console_view_entry(
        category = "release",
        short_name = "a64",
    ),
)

ci.thin_tester(
    name = "mac11-arm64-rel-tests",
    branch_selector = branches.selector.MAC_BRANCHES,
    triggered_by = ["ci/mac-arm64-rel"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    tree_closing = False,
    console_view_entry = consoles.console_view_entry(
        category = "release|arm64",
        short_name = "11",
    ),
    contact_team_email = "bling-engprod@google.com",
)

ci.thin_tester(
    name = "mac12-arm64-rel-tests",
    branch_selector = branches.selector.MAC_BRANCHES,
    triggered_by = ["ci/mac-arm64-rel"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    tree_closing = False,
    console_view_entry = consoles.console_view_entry(
        category = "release|arm64",
        short_name = "12",
    ),
    contact_team_email = "bling-engprod@google.com",
)

ci.thin_tester(
    name = "mac13-arm64-rel-tests",
    branch_selector = branches.selector.MAC_BRANCHES,
    triggered_by = ["ci/mac-arm64-rel"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    tree_closing = False,
    console_view_entry = consoles.console_view_entry(
        category = "release|arm64",
        short_name = "13",
    ),
    contact_team_email = "bling-engprod@google.com",
)

ci.thin_tester(
    name = "mac-skia-alt-arm64-rel-tests",
    description_html = "Runs web tests with Skia Graphite on Mac ARM machines",
    triggered_by = ["ci/mac-arm64-rel"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    gardener_rotations = args.ignore_default(None),
    tree_closing = False,
    console_view_entry = consoles.console_view_entry(
        category = "release|arm64",
        short_name = "skia-alt",
    ),
    contact_team_email = "chrome-skia-graphite@google.com",
)

ci.thin_tester(
    name = "mac14-arm64-rel-tests",
    branch_selector = branches.selector.MAC_BRANCHES,
    description_html = "Runs MacOS 14 tests on ARM machines",
    triggered_by = ["ci/mac-arm64-rel"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    tree_closing = False,
    console_view_entry = consoles.console_view_entry(
        category = "release|arm64",
        short_name = "14",
    ),
    contact_team_email = "bling-engprod@google.com",
)

ci.thin_tester(
    name = "Mac11 Tests",
    branch_selector = branches.selector.MAC_BRANCHES,
    triggered_by = ["ci/Mac Builder"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac",
        short_name = "11",
    ),
    contact_team_email = "bling-engprod@google.com",
)

ci.thin_tester(
    name = "Mac12 Tests",
    branch_selector = branches.selector.MAC_BRANCHES,
    triggered_by = ["ci/Mac Builder"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac",
        short_name = "12",
    ),
    contact_team_email = "bling-engprod@google.com",
)

ci.thin_tester(
    name = "Mac13 Tests",
    branch_selector = branches.selector.MAC_BRANCHES,
    triggered_by = ["ci/Mac Builder"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac",
        short_name = "13",
    ),
)

ci.thin_tester(
    name = "mac14-tests-dbg",
    branch_selector = branches.selector.MAC_BRANCHES,
    description_html = "Runs Mac 14 tests with debug config.",
    triggered_by = ["ci/Mac Builder (dbg)"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
        build_gs_bucket = "chromium-mac-archive",
    ),
    gardener_rotations = args.ignore_default(None),
    console_view_entry = consoles.console_view_entry(
        category = "debug",
        short_name = "14",
    ),
    cq_mirrors_console_view = "mirrors",
    contact_team_email = "bling-engprod@google.com",
)

ci.thin_tester(
    name = "mac14-tests",
    branch_selector = branches.selector.MAC_BRANCHES,
    description_html = "Runs default MacOS 14 tests on CI.",
    triggered_by = ["ci/Mac Builder"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    # TODO(crbug.com/336530603): Add to rotation when it's stable.
    gardener_rotations = args.ignore_default(None),
    console_view_entry = consoles.console_view_entry(
        category = "mac",
        short_name = "14",
    ),
    contact_team_email = "bling-engprod@google.com",
)

ios_builder(
    # We don't have necessary capacity to run this configuration in CQ, but it
    # is part of the main waterfall
    name = "ios-catalyst",
    description_html = (
        "Builds the open-source version of Chrome for iOS as a Catalyst app " +
        "(i.e. an iOS app for running in a wrapper on macOS). Build-only " +
        "(does not run tests)."
    ),
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "ios",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
                "mac_toolchain",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_bits = 64,
            target_platform = builder_config.target_platform.IOS,
        ),
        build_gs_bucket = "chromium-mac-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "compile_only",
            "debug_static_builder",
            "remoteexec",
            "ios_catalyst",
            "x64",
            "asan",
            "libfuzzer",
            "no_dsyms",
            "no_remoting",
        ],
    ),
    tree_closing = False,
    console_view_entry = [
        consoles.console_view_entry(
            category = "ios|default",
            short_name = "ctl",
        ),
    ],
)

ios_builder(
    name = "ios-device",
    description_html = (
        "Builds the open-source version of Chrome for iOS as a binary for " +
        "running on a real device. Build-only (does not run tests)."
    ),
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "ios",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
                "mac_toolchain",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 64,
            target_platform = builder_config.target_platform.IOS,
        ),
        build_gs_bucket = "chromium-mac-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "compile_only",
            "ios_device",
            "arm64",
            "ios_google_cert",
            "ios_disable_code_signing",
            "release_builder",
            "remoteexec",
        ],
    ),
    cpu = cpu.ARM64,
    console_view_entry = [
        consoles.console_view_entry(
            category = "ios|default",
            short_name = "dev",
        ),
        consoles.console_view_entry(
            branch_selector = branches.selector.MAIN,
            console_view = "sheriff.ios",
            category = "chromium.mac",
            short_name = "dev",
        ),
    ],
)

ios_builder(
    name = "ios-simulator",
    branch_selector = branches.selector.IOS_BRANCHES,
    description_html = (
        "Builds the open-source version of Chrome for iOS as a simulator " +
        "binary and runs tests. This is what's included on most CQ runs " +
        "(even for CLs that don't explicitly touch an iOS file)."
    ),
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "ios",
            apply_configs = [
                "use_clang_coverage",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
                "mac_toolchain",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_bits = 64,
            target_platform = builder_config.target_platform.IOS,
        ),
        build_gs_bucket = "chromium-mac-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "debug_static_builder",
            "remoteexec",
            "ios_simulator",
            "x64",
            "xctest",
        ],
    ),
    cpu = cpu.ARM64,
    console_view_entry = [
        consoles.console_view_entry(
            category = "ios|default",
            short_name = "sim",
        ),
        consoles.console_view_entry(
            branch_selector = branches.selector.MAIN,
            console_view = "sheriff.ios",
            category = "chromium.mac",
            short_name = "sim",
        ),
    ],
    cq_mirrors_console_view = "mirrors",
)

ios_builder(
    name = "ios-simulator-full-configs",
    branch_selector = branches.selector.IOS_BRANCHES,
    description_html = (
        "Builds the open-source version of Chrome for iOS as a simulator " +
        "binary, and runs tests on a large variety of configurations. These " +
        "configurations are less common (e.g. weird screen sizes, older OS " +
        "versions) and failures are less frequent, so these configs only " +
        "run in the CQ for CLs that actually touch an ios-related file. " +
        "Other CLs may introduce failures, but we handle them reactively as " +
        "they appear on the console."
    ),
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "ios",
            apply_configs = [
                "use_clang_coverage",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
                "mac_toolchain",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_bits = 64,
            target_platform = builder_config.target_platform.IOS,
        ),
        build_gs_bucket = "chromium-mac-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "debug_static_builder",
            "remoteexec",
            "ios_simulator",
            "x64",
            "xctest",
        ],
    ),
    cpu = cpu.ARM64,
    console_view_entry = [
        consoles.console_view_entry(
            category = "ios|default",
            short_name = "ful",
        ),
        consoles.console_view_entry(
            branch_selector = branches.selector.MAIN,
            console_view = "sheriff.ios",
            category = "chromium.mac",
            short_name = "ful",
        ),
    ],
    cq_mirrors_console_view = "mirrors",
)

ios_builder(
    name = "ios-simulator-noncq",
    description_html = (
        "Builds the open-source version of Chrome for iOS as a simulator " +
        "binary. Runs tests that are not included on CQ runs, but that we " +
        "still want tested regularly."
    ),
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "ios",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
                "mac_toolchain",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_bits = 64,
            target_platform = builder_config.target_platform.IOS,
        ),
        build_gs_bucket = "chromium-mac-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "debug_static_builder",
            "remoteexec",
            "ios_simulator",
            "arm64",
            "xctest",
        ],
    ),
    cpu = cpu.ARM64,
    console_view_entry = [
        consoles.console_view_entry(
            category = "ios|default",
            short_name = "non",
        ),
        consoles.console_view_entry(
            branch_selector = branches.selector.MAIN,
            console_view = "sheriff.ios",
            category = "chromium.mac",
            short_name = "non",
        ),
    ],
)